#include "save.h"

#include <pspdisplay.h>
#include <psputility.h>
#include <psputility_savedata.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SAVE_MAGIC 0x42425356u /* BBSV */
#define SAVE_VERSION 2u
#define SAVE_GAME_NAME "BBACK0001"
#define SAVE_NAME "0000"
#define SAVE_FILE_NAME "DATA.BIN"
#define SAVE_ERR_RW_NO_DATA 0x80110327

typedef struct {
    uint32_t magic;
    uint32_t version;
    int unlocked_level;
    bool sound_on;
    bool vibra_on;
    int records[5];
    SaveContinueState continue_state;
    int continue_level_index;
    int continue_lives;
    int continue_levels_done;
    int continue_total_score;
    uint32_t continue_level_elapsed_ms;
    bool continue_one_go;
    PlayerSaveState player;
    uint8_t* tile_map;
    size_t tile_map_size;
    int level_width;
    int level_height;
    int hoops_remaining;
    int object_count;
    int (*object_ag)[2];
    int8_t (*object_s)[2];
} SaveState;

static SaveState g_save;
static bool g_save_initialized = false;
static bool g_save_dirty = false;
static int g_save_last_load_result = 0;
static int g_save_last_save_result = 0;

static void write_u8(uint8_t** p, uint8_t v) {
    (*p)[0] = v;
    *p += 1;
}

static void write_be32(uint8_t** p, uint32_t v) {
    (*p)[0] = (uint8_t)(v >> 24);
    (*p)[1] = (uint8_t)(v >> 16);
    (*p)[2] = (uint8_t)(v >> 8);
    (*p)[3] = (uint8_t)(v);
    *p += 4;
}

static uint8_t read_u8(const uint8_t** p) {
    uint8_t v = (*p)[0];
    *p += 1;
    return v;
}

static uint32_t read_be32(const uint8_t** p) {
    uint32_t v = ((uint32_t)(*p)[0] << 24) |
                 ((uint32_t)(*p)[1] << 16) |
                 ((uint32_t)(*p)[2] << 8) |
                 ((uint32_t)(*p)[3]);
    *p += 4;
    return v;
}

static void save_state_free_runtime(void) {
    free(g_save.tile_map);
    g_save.tile_map = NULL;
    g_save.tile_map_size = 0;
    free(g_save.object_ag);
    g_save.object_ag = NULL;
    free(g_save.object_s);
    g_save.object_s = NULL;
    g_save.object_count = 0;
    g_save.level_width = 0;
    g_save.level_height = 0;
    g_save.hoops_remaining = 0;
}

static void save_state_reset_defaults(void) {
    save_state_free_runtime();
    memset(&g_save.player, 0, sizeof(g_save.player));
    g_save.magic = SAVE_MAGIC;
    g_save.version = SAVE_VERSION;
    g_save.unlocked_level = 1;
    g_save.sound_on = true;
    g_save.vibra_on = true;
    memset(g_save.records, 0, sizeof(g_save.records));
    g_save.continue_state = SAVE_CONTINUE_NONE;
    g_save.continue_level_index = 0;
    g_save.continue_lives = 3;
    g_save.continue_levels_done = 0;
    g_save.continue_total_score = 0;
    g_save.continue_level_elapsed_ms = 0;
    g_save.continue_one_go = false;
}

static size_t save_payload_size(void) {
    size_t size = 0;
    size += 4 + 4 + 4 + 1 + 1 + 2;
    size += 5 * 4;
    size += 1 + 3;
    size += 4 + 4 + 4 + 4 + 4;
    size += 1 + 3;
    size += 30 * 4;
    if (g_save.continue_state == SAVE_CONTINUE_GAME) {
        size += 4 + 4 + 4 + 4;
        size += g_save.tile_map_size;
        size += (size_t)g_save.object_count * (4 + 4 + 1 + 1);
    }
    return size;
}

static void save_pack_player(uint8_t** p, const PlayerSaveState* s) {
    write_be32(p, (uint32_t)s->x_pos);
    write_be32(p, (uint32_t)s->y_pos);
    write_be32(p, (uint32_t)s->x_speed);
    write_be32(p, (uint32_t)s->y_speed);
    write_be32(p, (uint32_t)s->prev_y_speed);
    write_be32(p, (uint32_t)s->sprite_index);
    write_be32(p, (uint32_t)s->is_large);
    write_be32(p, (uint32_t)s->is_inverted);
    write_be32(p, (uint32_t)s->is_popped);
    write_be32(p, (uint32_t)s->gravity_down);
    write_be32(p, (uint32_t)s->is_grounded);
    write_be32(p, (uint32_t)s->has_speed_bonus);
    write_be32(p, (uint32_t)s->has_jump_bonus);
    write_be32(p, (uint32_t)s->has_grav_bonus);
    write_be32(p, (uint32_t)s->stunned);
    write_be32(p, (uint32_t)s->control_mask);
    write_be32(p, (uint32_t)s->bounce_state);
    write_be32(p, (uint32_t)s->timer_a);
    write_be32(p, (uint32_t)s->timer_b);
    write_be32(p, (uint32_t)s->timer_c);
    write_be32(p, (uint32_t)s->state_r);
    write_be32(p, (uint32_t)s->state_a);
    write_be32(p, (uint32_t)s->carrier_object_index);
    write_be32(p, (uint32_t)s->is_dying);
    write_be32(p, (uint32_t)s->spawn_tile_x);
    write_be32(p, (uint32_t)s->spawn_tile_y);
    write_be32(p, (uint32_t)s->spawn_is_large);
    write_be32(p, (uint32_t)s->god_mode);
    write_be32(p, (uint32_t)s->lives);
    write_be32(p, (uint32_t)s->score);
}

static void save_unpack_player(const uint8_t** p, PlayerSaveState* s) {
    s->x_pos = (int)read_be32(p);
    s->y_pos = (int)read_be32(p);
    s->x_speed = (int)read_be32(p);
    s->y_speed = (int)read_be32(p);
    s->prev_y_speed = (int)read_be32(p);
    s->sprite_index = (int)read_be32(p);
    s->is_large = read_be32(p) != 0;
    s->is_inverted = read_be32(p) != 0;
    s->is_popped = read_be32(p) != 0;
    s->gravity_down = read_be32(p) != 0;
    s->is_grounded = read_be32(p) != 0;
    s->has_speed_bonus = read_be32(p) != 0;
    s->has_jump_bonus = read_be32(p) != 0;
    s->has_grav_bonus = read_be32(p) != 0;
    s->stunned = read_be32(p) != 0;
    s->control_mask = (int)read_be32(p);
    s->bounce_state = (int)read_be32(p);
    s->timer_a = (int)read_be32(p);
    s->timer_b = (int)read_be32(p);
    s->timer_c = (int)read_be32(p);
    s->state_r = (int)read_be32(p);
    s->state_a = (int)read_be32(p);
    s->carrier_object_index = (int)read_be32(p);
    s->is_dying = read_be32(p) != 0;
    s->spawn_tile_x = (int)read_be32(p);
    s->spawn_tile_y = (int)read_be32(p);
    s->spawn_is_large = read_be32(p) != 0;
    s->god_mode = read_be32(p) != 0;
    s->lives = (int)read_be32(p);
    s->score = (int)read_be32(p);
}

static bool save_store_data_buffer(const void* data_buf, size_t data_size);

static bool save_pack_blob(uint8_t** out_buf, size_t* out_size) {
    uint8_t* buf;
    uint8_t* p;
    size_t size = save_payload_size();

    if (!out_buf || !out_size) return false;
    buf = (uint8_t*)malloc(size);
    if (!buf) return false;

    p = buf;
    write_be32(&p, g_save.magic);
    write_be32(&p, g_save.version);
    write_be32(&p, (uint32_t)g_save.unlocked_level);
    write_u8(&p, g_save.sound_on ? 1 : 0);
    write_u8(&p, g_save.vibra_on ? 1 : 0);
    write_u8(&p, 0);
    write_u8(&p, 0);
    for (int i = 0; i < 5; i++) {
        write_be32(&p, (uint32_t)g_save.records[i]);
    }
    write_u8(&p, (uint8_t)g_save.continue_state);
    write_u8(&p, 0);
    write_u8(&p, 0);
    write_u8(&p, 0);
    write_be32(&p, (uint32_t)g_save.continue_level_index);
    write_be32(&p, (uint32_t)g_save.continue_lives);
    write_be32(&p, (uint32_t)g_save.continue_levels_done);
    write_be32(&p, (uint32_t)g_save.continue_total_score);
    write_be32(&p, g_save.continue_level_elapsed_ms);
    write_u8(&p, g_save.continue_one_go ? 1 : 0);
    write_u8(&p, 0);
    write_u8(&p, 0);
    write_u8(&p, 0);
    save_pack_player(&p, &g_save.player);

    if (g_save.continue_state == SAVE_CONTINUE_GAME) {
        write_be32(&p, (uint32_t)g_save.level_width);
        write_be32(&p, (uint32_t)g_save.level_height);
        write_be32(&p, (uint32_t)g_save.hoops_remaining);
        write_be32(&p, (uint32_t)g_save.object_count);
        if (g_save.tile_map_size > 0 && g_save.tile_map) {
            memcpy(p, g_save.tile_map, g_save.tile_map_size);
            p += g_save.tile_map_size;
        }
        for (int i = 0; i < g_save.object_count; i++) {
            write_be32(&p, (uint32_t)g_save.object_ag[i][0]);
            write_be32(&p, (uint32_t)g_save.object_ag[i][1]);
            write_u8(&p, (uint8_t)g_save.object_s[i][0]);
            write_u8(&p, (uint8_t)g_save.object_s[i][1]);
        }
    }

    *out_buf = buf;
    *out_size = size;
    return true;
}

static bool save_unpack_blob(const uint8_t* buf, size_t size) {
    const uint8_t* p = buf;
    const uint8_t* end = buf + size;
    uint32_t magic;
    uint32_t version;

    if (!buf || size < 4 + 4) return false;

    save_state_free_runtime();
    magic = read_be32(&p);
    version = read_be32(&p);
    if (magic != SAVE_MAGIC || version != SAVE_VERSION) return false;

    g_save.magic = magic;
    g_save.version = version;
    g_save.unlocked_level = (int)read_be32(&p);
    g_save.sound_on = read_u8(&p) != 0;
    g_save.vibra_on = read_u8(&p) != 0;
    (void)read_u8(&p);
    (void)read_u8(&p);
    for (int i = 0; i < 5; i++) {
        g_save.records[i] = (int)read_be32(&p);
    }
    g_save.continue_state = (SaveContinueState)read_u8(&p);
    (void)read_u8(&p);
    (void)read_u8(&p);
    (void)read_u8(&p);
    g_save.continue_level_index = (int)read_be32(&p);
    g_save.continue_lives = (int)read_be32(&p);
    g_save.continue_levels_done = (int)read_be32(&p);
    g_save.continue_total_score = (int)read_be32(&p);
    g_save.continue_level_elapsed_ms = read_be32(&p);
    if (version >= 2) {
        g_save.continue_one_go = read_u8(&p) != 0;
        (void)read_u8(&p);
        (void)read_u8(&p);
        (void)read_u8(&p);
    } else {
        g_save.continue_one_go = false;
    }
    save_unpack_player(&p, &g_save.player);

    if (g_save.continue_state == SAVE_CONTINUE_GAME) {
        g_save.level_width = (int)read_be32(&p);
        g_save.level_height = (int)read_be32(&p);
        g_save.hoops_remaining = (int)read_be32(&p);
        g_save.object_count = (int)read_be32(&p);
        if (g_save.level_width <= 0 || g_save.level_height <= 0 || g_save.object_count < 0) return false;
        g_save.tile_map_size = (size_t)g_save.level_width * (size_t)g_save.level_height;
        if (p + g_save.tile_map_size > end) return false;
        g_save.tile_map = (uint8_t*)malloc(g_save.tile_map_size);
        if (!g_save.tile_map) return false;
        memcpy(g_save.tile_map, p, g_save.tile_map_size);
        p += g_save.tile_map_size;
        if (g_save.object_count > 0) {
            g_save.object_ag = (int(*)[2])calloc((size_t)g_save.object_count, sizeof(int[2]));
            g_save.object_s = (int8_t(*)[2])calloc((size_t)g_save.object_count, sizeof(int8_t[2]));
            if (!g_save.object_ag || !g_save.object_s) return false;
        }
        for (int i = 0; i < g_save.object_count; i++) {
            if (p + 10 > end) return false;
            g_save.object_ag[i][0] = (int)read_be32(&p);
            g_save.object_ag[i][1] = (int)read_be32(&p);
            g_save.object_s[i][0] = (int8_t)read_u8(&p);
            g_save.object_s[i][1] = (int8_t)read_u8(&p);
        }
    }

    return true;
}

static void save_init_dialog(SceUtilitySavedataParam* dialog, int mode, void* data_buf, size_t data_size) {
    int language = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;

    memset(dialog, 0, sizeof(*dialog));
    dialog->base.size = sizeof(*dialog);
    if (sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &language) == 0) {
        dialog->base.language = language;
    } else {
        dialog->base.language = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
    }
    dialog->base.buttonSwap = PSP_UTILITY_ACCEPT_CROSS;
    dialog->base.graphicsThread = 0x11;
    dialog->base.accessThread = 0x13;
    dialog->base.fontThread = 0x12;
    dialog->base.soundThread = 0x10;
    dialog->mode = mode;
    dialog->overwrite = 1;

    strncpy(dialog->gameName, SAVE_GAME_NAME, sizeof(dialog->gameName) - 1);
    strncpy(dialog->saveName, SAVE_NAME, sizeof(dialog->saveName) - 1);
    strncpy(dialog->fileName, SAVE_FILE_NAME, sizeof(dialog->fileName) - 1);

    dialog->dataBuf = data_buf;
    dialog->dataBufSize = (SceSize)data_size;
    dialog->dataSize = (mode == SCE_UTILITY_SAVEDATA_READDATA) ? 0 : (SceSize)data_size;

    strncpy(dialog->sfoParam.title, "Bounce Back", sizeof(dialog->sfoParam.title) - 1);
    strncpy(dialog->sfoParam.savedataTitle, "Bounce Back", sizeof(dialog->sfoParam.savedataTitle) - 1);
    strncpy(dialog->sfoParam.detail, "Continue and records.", sizeof(dialog->sfoParam.detail) - 1);
    dialog->sfoParam.parentalLevel = 1;
}

static bool save_do_utility(int mode, void* data_buf, size_t data_size) {
    SceUtilitySavedataParam dialog;
    int init_rc;

    if (!data_buf || data_size == 0) return false;
    save_init_dialog(&dialog, mode, data_buf, data_size);
    init_rc = sceUtilitySavedataInitStart(&dialog);
    if (init_rc < 0) return false;

    for (;;) {
        int status = sceUtilitySavedataGetStatus();
        if (status == PSP_UTILITY_DIALOG_INIT || status == PSP_UTILITY_DIALOG_VISIBLE) {
            sceUtilitySavedataUpdate(1);
        } else if (status == PSP_UTILITY_DIALOG_FINISHED || status == PSP_UTILITY_DIALOG_QUIT) {
            sceUtilitySavedataShutdownStart();
        } else if (status == PSP_UTILITY_DIALOG_NONE) {
            break;
        }
        sceDisplayWaitVblankStart();
    }

    if (mode == SCE_UTILITY_SAVEDATA_READDATA) {
        g_save_last_load_result = dialog.base.result;
    } else {
        g_save_last_save_result = dialog.base.result;
    }
    return dialog.base.result == 0;
}

static bool save_load_data(uint8_t* data_buf, size_t data_size) {
    return save_do_utility(SCE_UTILITY_SAVEDATA_READDATA, data_buf, data_size);
}

static bool save_store_data_buffer(const void* data_buf, size_t data_size) {
    bool ok = save_do_utility(SCE_UTILITY_SAVEDATA_WRITEDATA, (void*)data_buf, data_size);
    if (!ok && g_save_last_save_result == (int)SAVE_ERR_RW_NO_DATA) {
        ok = save_do_utility(SCE_UTILITY_SAVEDATA_MAKEDATA, (void*)data_buf, data_size);
    }
    return ok;
}

int save_init(void) {
    uint8_t* buf = NULL;
    size_t buf_size = 64 * 1024;

    if (g_save_initialized) return 1;
    save_state_reset_defaults();

    buf = (uint8_t*)malloc(buf_size);
    if (!buf) return 0;

    if (save_load_data(buf, buf_size)) {
        if (!save_unpack_blob(buf, buf_size)) {
            save_state_reset_defaults();
        }
        g_save_initialized = true;
        free(buf);
        return 1;
    }

    if (g_save_last_load_result == (int)SAVE_ERR_RW_NO_DATA) {
        g_save_initialized = true;
        g_save_dirty = true;
        save_flush();
        free(buf);
        return 1;
    }

    free(buf);
    save_state_reset_defaults();
    return 0;
}

void save_shutdown(void) {
    if (!g_save_initialized) return;
    save_flush();
    g_save_initialized = false;
    save_state_free_runtime();
}

void save_flush(void) {
    uint8_t* buf = NULL;
    size_t size = 0;

    if (!g_save_initialized || !g_save_dirty) return;
    if (!save_pack_blob(&buf, &size)) return;
    if (save_store_data_buffer(buf, size)) {
        g_save_dirty = false;
    }
    free(buf);
}

bool save_has_continue(void) {
    return g_save.continue_state != SAVE_CONTINUE_NONE;
}

SaveContinueState save_get_continue_state(void) {
    return g_save.continue_state;
}

int save_get_continue_level_index(void) {
    return g_save.continue_level_index;
}

int save_get_continue_lives(void) {
    return g_save.continue_lives;
}

int save_get_continue_levels_done(void) {
    return g_save.continue_levels_done;
}

int save_get_continue_total_score(void) {
    return g_save.continue_total_score;
}

uint32_t save_get_continue_level_elapsed_ms(void) {
    return g_save.continue_level_elapsed_ms;
}

bool save_get_continue_one_go(void) {
    return g_save.continue_one_go;
}

void save_capture_game(int level_index,
                       int total_score,
                       int levels_done,
                       bool one_go_run,
                       uint32_t level_elapsed_ms,
                       const Level* level,
                       const Player* player) {
    if (!g_save_initialized || !level || !player || !level->tile_map) return;

    save_state_free_runtime();
    g_save.continue_state = SAVE_CONTINUE_GAME;
    g_save.continue_level_index = level_index;
    g_save.continue_lives = player->lives;
    g_save.continue_levels_done = levels_done;
    g_save.continue_total_score = total_score;
    g_save.continue_level_elapsed_ms = level_elapsed_ms;
    g_save.continue_one_go = one_go_run;
    player_export_state(player, &g_save.player);

    g_save.level_width = (int)level->width;
    g_save.level_height = (int)level->height;
    g_save.hoops_remaining = level->hoops_remaining;
    g_save.tile_map_size = (size_t)level->width * (size_t)level->height;
    g_save.tile_map = (uint8_t*)malloc(g_save.tile_map_size);
    if (g_save.tile_map && g_save.tile_map_size > 0) {
        memcpy(g_save.tile_map, level->tile_map, g_save.tile_map_size);
    }

    g_save.object_count = level->objects.count;
    if (g_save.object_count > 0) {
        g_save.object_ag = (int(*)[2])calloc((size_t)g_save.object_count, sizeof(int[2]));
        g_save.object_s = (int8_t(*)[2])calloc((size_t)g_save.object_count, sizeof(int8_t[2]));
        if (g_save.object_ag && g_save.object_s) {
            memcpy(g_save.object_ag, level->objects.ag, (size_t)g_save.object_count * sizeof(int[2]));
            memcpy(g_save.object_s, level->objects.s, (size_t)g_save.object_count * sizeof(int8_t[2]));
        }
    }

    g_save_dirty = true;
}

void save_capture_level_complete(int level_index,
                                 int lives,
                                 int levels_done,
                                 int total_score,
                                 bool one_go_run) {
    if (!g_save_initialized) return;
    save_state_free_runtime();
    memset(&g_save.player, 0, sizeof(g_save.player));
    g_save.continue_state = SAVE_CONTINUE_LEVEL_COMPLETE;
    g_save.continue_level_index = level_index;
    g_save.continue_lives = lives;
    g_save.continue_levels_done = levels_done;
    g_save.continue_total_score = total_score;
    g_save.continue_level_elapsed_ms = 0;
    g_save.continue_one_go = one_go_run;
    g_save_dirty = true;
}

void save_clear_continue(void) {
    if (!g_save_initialized) return;
    save_state_free_runtime();
    memset(&g_save.player, 0, sizeof(g_save.player));
    g_save.continue_state = SAVE_CONTINUE_NONE;
    g_save.continue_level_index = 0;
    g_save.continue_lives = 3;
    g_save.continue_levels_done = 0;
    g_save.continue_total_score = 0;
    g_save.continue_level_elapsed_ms = 0;
    g_save.continue_one_go = false;
    g_save_dirty = true;
}

bool save_restore_game(Level* level, Player* player) {
    if (!level || !player) return false;
    if (g_save.continue_state != SAVE_CONTINUE_GAME) return false;
    if ((int)level->width != g_save.level_width || (int)level->height != g_save.level_height) return false;
    if (level->objects.count != g_save.object_count) return false;
    if (!level->tile_map || !g_save.tile_map) return false;

    memcpy(level->tile_map, g_save.tile_map, g_save.tile_map_size);
    level->hoops_remaining = g_save.hoops_remaining;
    if (g_save.object_count > 0) {
        memcpy(level->objects.ag, g_save.object_ag, (size_t)g_save.object_count * sizeof(int[2]));
        memcpy(level->objects.s, g_save.object_s, (size_t)g_save.object_count * sizeof(int8_t[2]));
    }
    return player_import_state(player, &g_save.player);
}

void save_update_unlocked_level(int unlocked_level) {
    if (!g_save_initialized) return;
    if (unlocked_level > g_save.unlocked_level) {
        g_save.unlocked_level = unlocked_level;
        g_save_dirty = true;
    }
}

int save_get_unlocked_level(void) {
    return g_save.unlocked_level;
}

void save_insert_record(int score) {
    if (!g_save_initialized) return;
    for (int i = 0; i < 5; i++) {
        if (score > g_save.records[i]) {
            int old = g_save.records[i];
            g_save.records[i] = score;
            score = old;
            g_save_dirty = true;
        }
    }
}

const int* save_get_records(void) {
    return g_save.records;
}
