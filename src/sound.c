#include "sound.h"
#include "resource_loader.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Fixed-point wavetable */
#define WAVETABLE_SIZE 1024
#define PHASE_BITS 32
#define PHASE_MASK 0xFFFFFFFF
#define AUDIO_SR 44100

/* Envelope constants (~2-3 ms attack/release at 44.1 kHz) */
#define ATT_Q15  512
#define REL_Q15  512
#define OTT_BUFFER_SIZE 16738

static short g_sine_table[WAVETABLE_SIZE];
static int g_wavetable_initialized = 0;

static void init_wavetable(void) {
    if (g_wavetable_initialized) return;
    for (int i = 0; i < WAVETABLE_SIZE; i++) {
        float angle = 2.0f * (float)M_PI * (float)i / (float)WAVETABLE_SIZE;
        g_sine_table[i] = (short)(sinf(angle) * 32767.0f);
    }
    g_wavetable_initialized = 1;
}

static uint32_t frequency_to_phase_inc(float frequency, int sample_rate) {
    if (frequency <= 0.0f) return 0;
    double phase_inc_float = ((double)frequency / (double)sample_rate) * 4294967296.0;
    return (uint32_t)phase_inc_float;
}

/* ── Global state ─────────────────────────────────────────── */
static struct ott_info_t   g_sounds[SND_COUNT];
static struct ott_player_t g_players[SND_COUNT];
static int                 g_sound_initialized = 0;
static SDL_AudioDeviceID   g_audio_dev = 0;
static int                 g_active_sound = -1;

/* ── OTT parser (exact copies from bounce_zero) ──────────── */

int reverse_tempo(int l)
{
    short int tempo_code[32] = { 25,28,31,35,40,45,50,56,63,70,80,90,100,
                                112,125,140,160,180,200,225,250,285,320,
                                355,400,450,500,565,635,715,800,900 };
    return tempo_code[l];
}

int get_bits(unsigned char *buffer, int *ptr, int *bitptr, int bits)
{
    unsigned int holding;
    int i;

    if (*ptr >= OTT_BUFFER_SIZE - 1) return 0;

    holding = (buffer[*ptr] << 8) + buffer[*ptr + 1];
    i = (holding >> (16 - (bits + (*bitptr)))) & ((1 << bits) - 1);

    *bitptr = *bitptr + bits;
    if (*bitptr > 7) {
        *bitptr = *bitptr - 8;
        (*ptr)++;
    }
    return i;
}

int parse_ringtone(unsigned char *buffer, int ptr, struct ott_info_t *ott_info)
{
    int bitptr;
    int k, t, x;
    int patterns, count;
    int pattern_id;

    bitptr = 0;
    t = 0;
    ott_info->note_count = 0;

    k = get_bits(buffer, &t, &bitptr, 8);
    get_bits(buffer, &t, &bitptr, 8);
    k = get_bits(buffer, &t, &bitptr, 7);
    k = get_bits(buffer, &t, &bitptr, 3);

    if (k != 1 && k != 2) return 0;

    k = get_bits(buffer, &t, &bitptr, 4);

    for (x = 0; x < k; x++)
        ott_info->songname[x] = get_bits(buffer, &t, &bitptr, 8);
    ott_info->songname[x] = 0;

    patterns = get_bits(buffer, &t, &bitptr, 8);

    while (t < ptr && t < OTT_BUFFER_SIZE - 1) {
        if (patterns == 0) break;

        get_bits(buffer, &t, &bitptr, 3);
        get_bits(buffer, &t, &bitptr, 2);
        ott_info->loop = get_bits(buffer, &t, &bitptr, 4);
        count = get_bits(buffer, &t, &bitptr, 8);

        if (count > 100) count = 100;

        for (x = 0; x < count; x++) {
            if (t >= ptr || t >= OTT_BUFFER_SIZE - 1) break;

            k = get_bits(buffer, &t, &bitptr, 3);

            if (k == 0) {
                pattern_id = get_bits(buffer, &t, &bitptr, 2);
                (void)pattern_id;
            } else if (k == 1) {
                if (ott_info->note_count < 1024) {
                    struct ott_note_t *note = &ott_info->notes[ott_info->note_count];
                    note->tone     = get_bits(buffer, &t, &bitptr, 4);
                    note->length   = get_bits(buffer, &t, &bitptr, 3);
                    note->modifier = get_bits(buffer, &t, &bitptr, 2);
                    note->scale    = ott_info->scale;
                    note->style    = ott_info->style;
                    note->bpm      = ott_info->bpm;
                    note->volume   = ott_info->volume;
                    ott_info->note_count++;
                }
            } else if (k == 2) {
                ott_info->scale = get_bits(buffer, &t, &bitptr, 2);
                if (ott_info->scale > 0) ott_info->scale--;
            } else if (k == 3) {
                ott_info->style = get_bits(buffer, &t, &bitptr, 2);
            } else if (k == 4) {
                k = get_bits(buffer, &t, &bitptr, 5);
                ott_info->bpm = reverse_tempo(k);
            } else if (k == 5) {
                ott_info->volume = get_bits(buffer, &t, &bitptr, 4);
            }
        }

        if (t >= ptr) break;
        patterns--;
    }
    return 0;
}

/* ── New: parse OTT from memory buffer (replaces FILE*-based parse_ott) */
int parse_ott_from_mem(const uint8_t *data, size_t size, struct ott_info_t *ott_info)
{
    unsigned char buffer[OTT_BUFFER_SIZE];

    memset(ott_info, 0, sizeof(struct ott_info_t));
    memset(buffer, 0, sizeof(buffer));

    int len = (int)size;
    if (len > OTT_BUFFER_SIZE) len = OTT_BUFFER_SIZE;
    memcpy(buffer, data, (size_t)len);

    parse_ringtone(buffer, len, ott_info);
    return 0;
}

/* ── Frequency / duration helpers (exact copies) ─────────── */

static double tone_freqs[13] = {
    0, 261.625, 277.175, 293.675, 311.125, 329.625, 349.225,
    370, 392, 415.3, 440, 466.15, 493.883
};

float ott_tone_to_frequency(int tone, int scale) {
    if (tone <= 0 || tone > 12) return 0.0f;
    return (float)(tone_freqs[tone] * (1 << scale));
}

float ott_length_to_duration(int length, int bpm) {
    if (bpm <= 0) bpm = 120;
    if (length < 0 || length > 7) length = 2;
    float seconds_per_beat = 60.0f / (float)bpm;
    float note_fraction = 4.0f / (float)(1 << length);
    return seconds_per_beat * note_fraction;
}

/* ── Player init / start / stop ──────────────────────────── */

void ott_player_init(struct ott_player_t *player, struct ott_info_t *ott_info) {
    player->ott_info      = ott_info;
    player->current_note  = 0;
    player->note_time     = 0.0f;
    player->note_duration = 0.0f;
    player->frequency     = 0.0f;
    player->phase         = 0;
    player->phase_inc     = 0;
    player->env_q15       = 0;
    player->is_playing    = 0;
    player->allow_loop    = 0;
    player->sample_rate   = AUDIO_SR;
}

static void ott_player_start_unlocked(struct ott_player_t *player, int allow_loop) {
    player->current_note = 0;
    player->note_time    = 0.0f;
    player->phase        = 0;
    player->phase_inc    = 0;
    player->allow_loop   = allow_loop;

    if (player->ott_info->note_count > 0) {
        struct ott_note_t *note = &player->ott_info->notes[0];
        player->frequency     = ott_tone_to_frequency(note->tone, note->scale);
        player->note_duration = ott_length_to_duration(note->length, note->bpm);
        player->phase_inc     = frequency_to_phase_inc(player->frequency, player->sample_rate);
    }

    player->is_playing = 1;
}

static void ott_player_finish_unlocked(struct ott_player_t *player) {
    player->is_playing = 0;
    player->allow_loop = 0;
    player->current_note = 0;
    player->note_time = 0.0f;
    player->note_duration = 0.0f;
    player->frequency = 0.0f;
    /* Keep phase/phase_inc/env for a short release tail to avoid a click. */
}

void ott_player_start(struct ott_player_t *player) {
    SDL_LockAudioDevice(g_audio_dev);
    ott_player_start_unlocked(player, 0);
    SDL_UnlockAudioDevice(g_audio_dev);
}

void ott_player_stop(struct ott_player_t *player) {
    player->is_playing = 0;
    player->allow_loop = 0;
    player->current_note = 0;
    player->note_time = 0.0f;
    player->note_duration = 0.0f;
    player->frequency = 0.0f;
    player->phase = 0;
    player->phase_inc = 0;
    player->env_q15 = 0;
}

/* ── Sample generation (exact copy) ──────────────────────── */

static short generate_player_sample(struct ott_player_t *player, float sample_length) {
    if (!player || !player->ott_info) return 0;

    if (player->is_playing) {
        if (player->note_time >= player->note_duration) {
            player->current_note++;
            player->note_time = 0.0f;

            if (player->current_note >= player->ott_info->note_count) {
                if (player->allow_loop && player->ott_info->loop) {
                    player->current_note = 0;
                } else {
                    ott_player_finish_unlocked(player);
                }
            }

            if (player->is_playing) {
                struct ott_note_t *note = &player->ott_info->notes[player->current_note];
                player->frequency     = ott_tone_to_frequency(note->tone, note->scale);
                player->note_duration = ott_length_to_duration(note->length, note->bpm);
                player->phase_inc     = frequency_to_phase_inc(player->frequency, player->sample_rate);
            }
        }
    }

    const int osc_on = (player->is_playing && player->phase_inc) || (player->env_q15 > 0);

    short s = 0;
    if (osc_on && player->phase_inc) {
        uint32_t table_index = (player->phase >> (PHASE_BITS - 10)) & (WAVETABLE_SIZE - 1);
        s = g_sine_table[table_index];
        player->phase += player->phase_inc;
    }

    if (player->is_playing && player->phase_inc) {
        player->env_q15 += ((32767 - player->env_q15) * ATT_Q15) >> 15;
    } else {
        player->env_q15 -= (player->env_q15 * REL_Q15) >> 15;
        if (player->env_q15 < 16) player->env_q15 = 0;
    }

    s = (short)(((int)s * (int)player->env_q15) >> 15);

    if (player->is_playing) {
        player->note_time += sample_length;
    }
    return s;
}

/* ── SDL2 audio callback ─────────────────────────────────── */

static void sdl_audio_callback(void *userdata, Uint8 *stream, int len) {
    (void)userdata;
    int16_t *out = (int16_t *)stream;
    int sample_count = len / (int)sizeof(int16_t) / 2; /* stereo */
    const float sample_length = 1.0f / (float)AUDIO_SR;

    for (int i = 0; i < sample_count; i++) {
        int mix = 0;
        int active = 0;
        for (int j = 0; j < SND_COUNT; j++) {
            short s = generate_player_sample(&g_players[j], sample_length);
            mix += (int)s;
            if (g_players[j].is_playing && g_players[j].phase_inc) active++;
        }
        if (active > 1) mix /= active;
        if (mix >  32767) mix =  32767;
        if (mix < -32768) mix = -32768;

        out[i * 2 + 0] = (int16_t)mix;
        out[i * 2 + 1] = (int16_t)mix;
    }
}

/* ── High-level API ──────────────────────────────────────── */

int sound_init(void) {
    if (g_sound_initialized) return 1;

    /* SDL_INIT_AUDIO must already be passed to SDL_Init() by the caller.
     * We only check here to catch mistakes; we do NOT call SDL_InitSubSystem
     * because on the PSP SDL2 backend that can fail after SDL_Init. */
    if (!(SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO)) return 10;

    init_wavetable();

    /* Load all 11 OTT sounds from res/s container */
    ResourceContainer *rc = resource_load("res/s");
    if (!rc) return 2;

    int count = (rc->count < SND_COUNT) ? rc->count : SND_COUNT;
    for (int i = 0; i < count; i++) {
        size_t sz;
        const uint8_t *data = resource_get_element(rc, i, &sz);
        if (data) parse_ott_from_mem(data, sz, &g_sounds[i]);
    }
    resource_free(rc);

    /* Init all players */
    for (int i = 0; i < SND_COUNT; i++)
        ott_player_init(&g_players[i], &g_sounds[i]);

    /* Open SDL audio device */
    SDL_AudioSpec want;
    SDL_memset(&want, 0, sizeof(want));
    want.freq     = AUDIO_SR;
    want.format   = AUDIO_S16SYS;
    want.channels = 2;
    want.samples  = 1024;
    want.callback = sdl_audio_callback;

    g_audio_dev = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
    if (g_audio_dev == 0) {
        return 11;
    }

    SDL_PauseAudioDevice(g_audio_dev, 0);

    g_sound_initialized = 1;
    return 1;
}

void sound_shutdown(void) {
    if (!g_sound_initialized) return;

    SDL_LockAudioDevice(g_audio_dev);
    for (int i = 0; i < SND_COUNT; i++)
        ott_player_stop(&g_players[i]);
    SDL_UnlockAudioDevice(g_audio_dev);

    SDL_CloseAudioDevice(g_audio_dev);
    g_audio_dev = 0;
    g_sound_initialized = 0;
    g_active_sound = -1;
}

void sound_play(int index) {
    if (g_sound_initialized && index >= 0 && index < SND_COUNT) {
        SDL_LockAudioDevice(g_audio_dev);
        if (g_active_sound >= 0 && g_active_sound < SND_COUNT) {
            ott_player_stop(&g_players[g_active_sound]);
        }
        ott_player_start_unlocked(&g_players[index], 0);
        g_active_sound = index;
        SDL_UnlockAudioDevice(g_audio_dev);
    }
}

void sound_stop_all(void) {
    if (!g_sound_initialized) return;
    SDL_LockAudioDevice(g_audio_dev);
    for (int i = 0; i < SND_COUNT; i++) {
        g_players[i].is_playing = 0;
        g_players[i].env_q15   = 0;
        g_players[i].allow_loop = 0;
    }
    g_active_sound = -1;
    SDL_UnlockAudioDevice(g_audio_dev);
}
