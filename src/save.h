#ifndef SAVE_H
#define SAVE_H

#include <stdbool.h>
#include <stdint.h>

#include "exit_door.h"
#include "level_loader.h"
#include "player.h"

typedef enum {
    SAVE_CONTINUE_NONE = 0,
    SAVE_CONTINUE_GAME = 1,
    SAVE_CONTINUE_LEVEL_COMPLETE = 2
} SaveContinueState;

int save_init(void);
void save_shutdown(void);
void save_flush(void);

bool save_has_continue(void);
SaveContinueState save_get_continue_state(void);
int save_get_continue_level_index(void);
int save_get_continue_lives(void);
int save_get_continue_levels_done(void);
int save_get_continue_total_score(void);
uint32_t save_get_continue_level_elapsed_ms(void);
bool save_get_continue_one_go(void);

void save_capture_game(int level_index,
                       int total_score,
                       int levels_done,
                       bool one_go_run,
                       uint32_t level_elapsed_ms,
                       const Level* level,
                       const Player* player,
                       const ExitDoorState* door);
void save_capture_level_complete(int level_index,
                                 int lives,
                                 int levels_done,
                                 int total_score,
                                 bool one_go_run,
                                 const Level* level,
                                 const Player* player,
                                 const ExitDoorState* door);
void save_clear_continue(void);
bool save_restore_game(Level* level, Player* player, ExitDoorState* door);

void save_update_unlocked_level(int unlocked_level);
int save_get_unlocked_level(void);
void save_insert_record(int score);
const int* save_get_records(void);

#endif
