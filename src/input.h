/**
 * Input handling - Step 05.
 *
 * Spec: docs/STEP_05_INPUT.md
 */

#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

typedef struct {
    bool left;
    bool right;
    bool down;
    bool jump;
} Input;

void input_init(void);
void input_cleanup(void);
void input_update(Input* input);

#endif // INPUT_H

