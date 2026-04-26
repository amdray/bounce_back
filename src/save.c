#include "save.h"

#include "endian_utils.h"

#include <pspdisplay.h>
#include <psputility.h>
#include <psputility_savedata.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SAVE_MAGIC 0x42425356u /* BBSV */
#define SAVE_VERSION 0u
#define SAVE_GAME_NAME "BBACK0001"
#define SAVE_NAME "0000"
#define SAVE_FILE_NAME "DATA.BIN"
#define SAVE_ERR_RW_NO_DATA 0x80110327

typedef struct {
    uint16_t row;
    uint16_t col;
    uint8_t tile;
} SavedTileEntry;

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
    int current_level_score;
    int door_i;
    bool door_open;
    PlayerRmsState player;
    int hoops_remaining;
    SavedTileEntry* modified_tiles;
    int modified_tile_count;
    int object_count;
    int (*object_ag)[2];
    int8_t (*object_s)[2];
} SaveState;

static SaveState g_save;
static bool g_save_initialized = false;
static bool g_save_dirty = false;
static int g_save_last_load_result = 0;
static int g_save_last_save_result = 0;
static size_t g_save_last_loaded_size = 0;

static bool save_unpack_fail(const char* reason, size_t offset, size_t size) {
    (void)reason; (void)offset; (void)size;
    return false;
}

static void write_u8(uint8_t** p, uint8_t v) {
    (*p)[0] = v;
    *p += 1;
}

static void write_be16(uint8_t** p, uint16_t v) {
    (*p)[0] = (uint8_t)(v >> 8);
    (*p)[1] = (uint8_t)(v);
    *p += 2;
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

static uint16_t read_be16(const uint8_t** p) {
    uint16_t v = (uint16_t)(((uint16_t)(*p)[0] << 8) | (uint16_t)(*p)[1]);
    *p += 2;
    return v;
}

static void save_state_free_runtime(void) {
    free(g_save.modified_tiles);
    g_save.modified_tiles = NULL;
    g_save.modified_tile_count = 0;
    free(g_save.object_ag);
    g_save.object_ag = NULL;
    free(g_save.object_s);
    g_save.object_s = NULL;
    g_save.object_count = 0;
    g_save.hoops_remaining = 0;
    g_save.current_level_score = 0;
    g_save.door_i = 0;
    g_save.door_open = false;
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

static bool save_should_capture_tile(uint8_t tile_id) {
    switch (tile_id) {
        case 7:
        case 8:
        case 12:
        case 30:
        case 34:
        case 35:
        case 93:
        case 94:
        case 95:
        case 96:
        case 97:
        case 98:
        case 99:
        case 100:
        case 101:
        case 102:
        case 103:
        case 104:
            return true;
        default:
            return false;
    }
}

static bool save_should_restore_tile(uint8_t tile_id) {
    if (tile_id == 35) return false;
    return save_should_capture_tile(tile_id);
}

static int save_count_modified_tiles(const Level* level) {
    int count = 0;

    if (!level || !level->tile_map) return 0;
    for (int row = 0; row < (int)level->height; row++) {
        for (int col = 0; col < (int)level->width; col++) {
            uint8_t tile = level->tile_map[(size_t)row * (size_t)level->width + (size_t)col];
            if (save_should_capture_tile(tile & 0x7F)) {
                count++;
            }
        }
    }
    return count;
}

static void save_capture_modified_tiles(const Level* level, SavedTileEntry* entries) {
    int index = 0;

    if (!level || !level->tile_map || !entries) return;
    for (int row = 0; row < (int)level->height; row++) {
        for (int col = 0; col < (int)level->width; col++) {
            uint8_t tile = level->tile_map[(size_t)row * (size_t)level->width + (size_t)col];
            if (!save_should_capture_tile(tile & 0x7F)) continue;
            entries[index].row = (uint16_t)row;
            entries[index].col = (uint16_t)col;
            entries[index].tile = tile;
            index++;
        }
    }
}

static const SavedTileEntry* save_find_modified_tile(const SaveState* save, int row, int col) {
    if (!save || !save->modified_tiles) return NULL;
    for (int i = 0; i < save->modified_tile_count; i++) {
        const SavedTileEntry* entry = &save->modified_tiles[i];
        if ((int)entry->row == row && (int)entry->col == col) {
            return entry;
        }
    }
    return NULL;
}

static void save_capture_runtime_snapshot(const Level* level,
                                          const Player* player,
                                          const ExitDoorState* door) {
    if (!level || !player || !level->tile_map) return;

    player_export_rms_state(player, &g_save.player);
    g_save.current_level_score = player->score;
    g_save.door_i = door ? door->I : 0;
    g_save.door_open = door ? door->open : false;
    g_save.hoops_remaining = level->hoops_remaining;
    g_save.modified_tile_count = save_count_modified_tiles(level);
    if (g_save.modified_tile_count > 0) {
        g_save.modified_tiles = (SavedTileEntry*)calloc((size_t)g_save.modified_tile_count, sizeof(SavedTileEntry));
        if (g_save.modified_tiles) {
            save_capture_modified_tiles(level, g_save.modified_tiles);
        } else {
            g_save.modified_tile_count = 0;
        }
    }

    g_save.object_count = level->objects.count;
    if (g_save.object_count > 0) {
        g_save.object_ag = (int(*)[2])calloc((size_t)g_save.object_count, sizeof(int[2]));
        g_save.object_s = (int8_t(*)[2])calloc((size_t)g_save.object_count, sizeof(int8_t[2]));
        if (g_save.object_ag && g_save.object_s) {
            memcpy(g_save.object_ag, level->objects.ag, (size_t)g_save.object_count * sizeof(int[2]));
            memcpy(g_save.object_s, level->objects.s, (size_t)g_save.object_count * sizeof(int8_t[2]));
        } else {
            free(g_save.object_ag);
            free(g_save.object_s);
            g_save.object_ag = NULL;
            g_save.object_s = NULL;
            g_save.object_count = 0;
        }
    }
}

static size_t save_payload_size(void) {
    size_t size = 0;
    size += 4 + 4 + 4 + 1 + 1 + 2;
    size += 5 * 4;
    size += 1 + 3;
    size += 4 + 4 + 4 + 4 + 4;
    size += 1 + 3;
    size += 10 * 4; /* D i n H s h G z b B */
    size += 5;      /* x F I p q */
    size += 2;      /* a r */
    size += 4 + 4;  /* A g */
    if (g_save.continue_state == SAVE_CONTINUE_GAME) {
        size += 4 + 4 + 1 + 4 + 1 + 4;
        size += (size_t)g_save.modified_tile_count * 5;
        size += (size_t)g_save.object_count * (4 + 4 + 1 + 1);
    }
    return size;
}

static void save_pack_player(uint8_t** p, const PlayerRmsState* s) {
    write_be32(p, (uint32_t)s->x_pos);
    write_be32(p, (uint32_t)s->y_pos);
    write_be32(p, (uint32_t)s->spawn_tile_y);
    write_be32(p, (uint32_t)s->spawn_tile_x);
    write_be32(p, (uint32_t)s->x_speed);
    write_be32(p, (uint32_t)s->y_speed);
    write_be32(p, (uint32_t)s->bounce_state);
    write_be32(p, (uint32_t)s->prev_y_speed);
    write_be32(p, (uint32_t)s->stone_timer);
    write_be32(p, (uint32_t)s->timer_b);
    write_u8(p, s->is_grounded ? 1 : 0);
    write_u8(p, s->is_stone ? 1 : 0);
    write_u8(p, s->is_inverted ? 1 : 0);
    write_u8(p, s->gravity_down ? 1 : 0);
    write_u8(p, s->stunned ? 1 : 0);
    write_u8(p, (uint8_t)s->state_a);
    write_u8(p, (uint8_t)s->state_r);
    write_be32(p, (uint32_t)s->timer_a);
    write_be32(p, (uint32_t)s->sprite_index);
}

static void save_unpack_player(const uint8_t** p, PlayerRmsState* s) {
    memset(s, 0, sizeof(*s));
    s->x_pos = (int)endian_read_be32_advance(p);
    s->y_pos = (int)endian_read_be32_advance(p);
    s->spawn_tile_y = (int)endian_read_be32_advance(p);
    s->spawn_tile_x = (int)endian_read_be32_advance(p);
    s->x_speed = (int)endian_read_be32_advance(p);
    s->y_speed = (int)endian_read_be32_advance(p);
    s->bounce_state = (int)endian_read_be32_advance(p);
    s->prev_y_speed = (int)endian_read_be32_advance(p);
    s->stone_timer = (int)endian_read_be32_advance(p);
    s->timer_b = (int)endian_read_be32_advance(p);
    s->is_grounded = read_u8(p) != 0;
    s->is_stone = read_u8(p) != 0;
    s->is_inverted = read_u8(p) != 0;
    s->gravity_down = read_u8(p) != 0;
    s->stunned = read_u8(p) != 0;
    s->state_a = (int8_t)read_u8(p);
    s->state_r = (int8_t)read_u8(p);
    s->timer_a = (int)endian_read_be32_advance(p);
    s->sprite_index = (int)endian_read_be32_advance(p);
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
        write_be32(&p, (uint32_t)g_save.current_level_score);
        write_be32(&p, (uint32_t)g_save.door_i);
        write_u8(&p, g_save.door_open ? 1 : 0);
        write_be32(&p, (uint32_t)g_save.hoops_remaining);
        write_u8(&p, (uint8_t)g_save.modified_tile_count);
        for (int i = 0; i < g_save.modified_tile_count; i++) {
            write_be16(&p, g_save.modified_tiles[i].row);
            write_be16(&p, g_save.modified_tiles[i].col);
            write_u8(&p, g_save.modified_tiles[i].tile);
        }
        write_be32(&p, (uint32_t)g_save.object_count);
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

    if (!buf || size < 4 + 4) return save_unpack_fail("header_too_small", 0, size);

    save_state_free_runtime();
    magic = endian_read_be32_advance(&p);
    version = endian_read_be32_advance(&p);
    if (magic != SAVE_MAGIC) return save_unpack_fail("bad_magic", (size_t)(p - buf), size);

    g_save.magic = magic;
    g_save.version = version;
    g_save.unlocked_level = (int)endian_read_be32_advance(&p);
    g_save.sound_on = read_u8(&p) != 0;
    g_save.vibra_on = read_u8(&p) != 0;
    (void)read_u8(&p);
    (void)read_u8(&p);
    for (int i = 0; i < 5; i++) {
        g_save.records[i] = (int)endian_read_be32_advance(&p);
    }
    g_save.continue_state = (SaveContinueState)read_u8(&p);
    (void)read_u8(&p);
    (void)read_u8(&p);
    (void)read_u8(&p);
    g_save.continue_level_index = (int)endian_read_be32_advance(&p);
    g_save.continue_lives = (int)endian_read_be32_advance(&p);
    g_save.continue_levels_done = (int)endian_read_be32_advance(&p);
    g_save.continue_total_score = (int)endian_read_be32_advance(&p);
    g_save.continue_level_elapsed_ms = endian_read_be32_advance(&p);
    g_save.continue_one_go = read_u8(&p) != 0;
    (void)read_u8(&p);
    (void)read_u8(&p);
    (void)read_u8(&p);
    save_unpack_player(&p, &g_save.player);

    if (g_save.continue_state == SAVE_CONTINUE_GAME) {
        g_save.current_level_score = (int)endian_read_be32_advance(&p);
        g_save.door_i = (int)endian_read_be32_advance(&p);
        g_save.door_open = read_u8(&p) != 0;
        g_save.hoops_remaining = (int)endian_read_be32_advance(&p);
        g_save.modified_tile_count = (int)read_u8(&p);
        if (g_save.modified_tile_count < 0) return save_unpack_fail("negative_modified_tile_count", (size_t)(p - buf), size);
        if (g_save.modified_tile_count > 0) {
            if (p + (size_t)g_save.modified_tile_count * 5 > end) {
                return save_unpack_fail("modified_tiles_overflow", (size_t)(p - buf), size);
            }
            g_save.modified_tiles = (SavedTileEntry*)calloc((size_t)g_save.modified_tile_count, sizeof(SavedTileEntry));
            if (!g_save.modified_tiles) return save_unpack_fail("modified_tiles_alloc", (size_t)(p - buf), size);
            for (int i = 0; i < g_save.modified_tile_count; i++) {
                g_save.modified_tiles[i].row = read_be16(&p);
                g_save.modified_tiles[i].col = read_be16(&p);
                g_save.modified_tiles[i].tile = read_u8(&p);
            }
        }
        g_save.object_count = (int)endian_read_be32_advance(&p);
        if (g_save.object_count < 0) return save_unpack_fail("negative_object_count", (size_t)(p - buf), size);
        if (g_save.object_count > 0) {
            g_save.object_ag = (int(*)[2])calloc((size_t)g_save.object_count, sizeof(int[2]));
            g_save.object_s = (int8_t(*)[2])calloc((size_t)g_save.object_count, sizeof(int8_t[2]));
            if (!g_save.object_ag || !g_save.object_s) return save_unpack_fail("object_arrays_alloc", (size_t)(p - buf), size);
        }
        for (int i = 0; i < g_save.object_count; i++) {
            if (p + 10 > end) return save_unpack_fail("object_data_overflow", (size_t)(p - buf), size);
            g_save.object_ag[i][0] = (int)endian_read_be32_advance(&p);
            g_save.object_ag[i][1] = (int)endian_read_be32_advance(&p);
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
        g_save_last_loaded_size = (dialog.base.result == 0) ? (size_t)dialog.dataSize : 0;
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
        size_t loaded_size = g_save_last_loaded_size;
        if (loaded_size == 0 || loaded_size > buf_size) {
            loaded_size = buf_size;
        }
        if (!save_unpack_blob(buf, loaded_size)) {
            save_state_reset_defaults();
            g_save_initialized = true;
            g_save_dirty = true;
            save_flush();
            g_save_initialized = false;
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
                       const Player* player,
                       const ExitDoorState* door) {
    if (!g_save_initialized || !level || !player || !level->tile_map) return;

    save_state_free_runtime();
    g_save.continue_state = SAVE_CONTINUE_GAME;
    g_save.continue_level_index = level_index;
    g_save.continue_lives = player->lives;
    g_save.continue_levels_done = levels_done;
    g_save.continue_total_score = total_score;
    g_save.continue_level_elapsed_ms = level_elapsed_ms;
    g_save.continue_one_go = one_go_run;
    save_capture_runtime_snapshot(level, player, door);

    g_save_dirty = true;
}

void save_capture_level_complete(int level_index,
                                 int lives,
                                 int levels_done,
                                 int total_score,
                                 bool one_go_run,
                                 const Level* level,
                                 const Player* player,
                                 const ExitDoorState* door) {
    if (!g_save_initialized) return;
    save_state_free_runtime();
    g_save.continue_state = SAVE_CONTINUE_LEVEL_COMPLETE;
    g_save.continue_level_index = level_index;
    g_save.continue_lives = lives;
    g_save.continue_levels_done = levels_done;
    g_save.continue_total_score = total_score;
    g_save.continue_level_elapsed_ms = 0;
    g_save.continue_one_go = one_go_run;
    if (level && player) {
        save_capture_runtime_snapshot(level, player, door);
    } else {
        memset(&g_save.player, 0, sizeof(g_save.player));
    }
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

bool save_restore_game(Level* level, Player* player, ExitDoorState* door) {
    if (!level || !player || !door) return false;
    if (g_save.continue_state != SAVE_CONTINUE_GAME) return false;
    if (level->objects.count != g_save.object_count) return false;
    if (!level->tile_map) return false;

    for (int row = 0; row < (int)level->height; row++) {
        for (int col = 0; col < (int)level->width; col++) {
            size_t index = (size_t)row * (size_t)level->width + (size_t)col;
            uint8_t tile = level->tile_map[index];
            uint8_t tile_id = tile & 0x7F;

            if (!save_should_restore_tile(tile_id)) continue;

            const SavedTileEntry* entry = save_find_modified_tile(&g_save, row, col);
            level->tile_map[index] = entry ? entry->tile : (uint8_t)(tile & 0x80);
        }
    }
    level->hoops_remaining = g_save.hoops_remaining;
    if (g_save.object_count > 0) {
        memcpy(level->objects.ag, g_save.object_ag, (size_t)g_save.object_count * sizeof(int[2]));
        memcpy(level->objects.s, g_save.object_s, (size_t)g_save.object_count * sizeof(int8_t[2]));
    }
    door->I = g_save.door_i;
    door->open = g_save.door_open;
    if (!player_import_rms_state(player, level, &g_save.player)) return false;
    player->lives = g_save.continue_lives;
    player->score = g_save.current_level_score;
    return true;
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
