/**
 * Input handling - Step 05.
 *
 * Spec: docs/STEP_05_INPUT.md
 */

#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

typedef struct {
    /* gameplay (held) */
    bool left;
    bool right;
    bool down;
    bool jump;
    bool shoulder_l;
    bool shoulder_r;
    /* navigation (held) */
    bool up;
    /* edge-triggered (true only on the frame the button went down) */
    bool confirm_pressed;  /* A / Cross */
    bool cancel_pressed;   /* B / Circle */
    bool start_pressed;    /* Start */
    bool up_pressed;
    bool down_pressed;
    bool left_pressed;
    bool right_pressed;
    /* raw prev state for edge detection */
    bool _prev_confirm;
    bool _prev_cancel;
    bool _prev_start;
    bool _prev_up;
    bool _prev_down;
    bool _prev_left;
    bool _prev_right;
} Input;

void input_init(void);
void input_cleanup(void);
void input_update(Input* input);
void input_sync(Input* input);

#endif // INPUT_H
