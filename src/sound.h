#ifndef SOUND_H
#define SOUND_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <SDL2/SDL.h>

#define SONGNAME_LEN 64

/* All 11 OTT sounds from res/s container.
 * Indices match CrystalMidlet.b(n) in the original Java code. */
#define SND_COUNT      11
#define SND_DEATH       0   /* b(0) player death              */
#define SND_HOOP        1   /* b(1) ring/hoop collect          */
#define SND_GEM         2   /* b(2) gem/checkpoint/good items  */
#define SND_JUMP_UP     3   /* b(3) jump-up tile               */
#define SND_JUMP_DOWN   4   /* b(4) jump-down tile (inverted)  */
#define SND_INFLATE     5   /* b(5) inflate/deflate            */
#define SND_POWERUP     6   /* b(6) powerup pickup             */
#define SND_EXIT        7   /* b(7) level exit                 */
#define SND_MENU_SPLASH  8  /* b(8) menu transition/splash     */
#define SND_MENU_SELECT  9  /* b(9) menu select/confirm        */
#define SND_MENU_BACK   10  /* b(10) menu back/cancel          */

struct ott_note_t {
    int tone;
    int length;
    int modifier;
    int scale;
    int style;
    int bpm;
    int volume;
};

struct ott_info_t {
    char songname[SONGNAME_LEN];
    int loop;
    int scale;
    int style;
    int bpm;
    int volume;
    int note_count;
    struct ott_note_t notes[1024];
};

struct ott_player_t {
    struct ott_info_t *ott_info;
    int current_note;
    float note_time;
    float note_duration;
    float frequency;
    uint32_t phase;
    uint32_t phase_inc;
    uint16_t env_q15;
    int is_playing;
    int allow_loop;
    int sample_rate;
};

/* OTT parsing */
int get_bits(unsigned char *buffer, int *ptr, int *bitptr, int bits);
int reverse_tempo(int l);
int parse_ringtone(unsigned char *buffer, int ptr, struct ott_info_t *ott_info);
int parse_ott_from_mem(const uint8_t *data, size_t size, struct ott_info_t *ott_info);

/* Synthesis helpers */
float ott_tone_to_frequency(int tone, int scale);
float ott_length_to_duration(int length, int bpm);
void ott_player_init(struct ott_player_t *player, struct ott_info_t *ott_info);
void ott_player_start(struct ott_player_t *player);
void ott_player_stop(struct ott_player_t *player);

/* High-level game API */
int  sound_init(void);
void sound_shutdown(void);
void sound_play(int index);
void sound_stop_all(void);

#endif
