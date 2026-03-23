/**
 * Input handling implementation - Step 05.
 */

#include "input.h"

#include <SDL2/SDL.h>

static SDL_GameController* s_controller = NULL;

static void input_read_current(Input* input,
                               bool* cur_confirm,
                               bool* cur_cancel,
                               bool* cur_start,
                               bool* cur_up,
                               bool* cur_down,
                               bool* cur_left,
                               bool* cur_right) {
    input->left       = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT) != 0;
    input->right      = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) != 0;
    input->down       = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN) != 0;
    input->up         = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_DPAD_UP) != 0;
    input->jump       = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_A) != 0;
    input->shoulder_l = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER) != 0;
    input->shoulder_r = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) != 0;

    *cur_confirm = input->jump;
    *cur_cancel = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_B) != 0;
    *cur_start = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_START) != 0;
    *cur_up = input->up;
    *cur_down = input->down;
    *cur_left = input->left;
    *cur_right = input->right;
}

void input_init(void) {
    if (SDL_NumJoysticks() > 0) {
        s_controller = SDL_GameControllerOpen(0);
    }
}

void input_cleanup(void) {
    if (s_controller) {
        SDL_GameControllerClose(s_controller);
        s_controller = NULL;
    }
}

void input_update(Input* input) {
    bool cur_confirm, cur_cancel, cur_start, cur_up, cur_down, cur_left, cur_right;

    if (!input || !s_controller) return;
    input_read_current(input, &cur_confirm, &cur_cancel, &cur_start,
                       &cur_up, &cur_down, &cur_left, &cur_right);

    input->confirm_pressed = cur_confirm && !input->_prev_confirm;
    input->cancel_pressed  = cur_cancel  && !input->_prev_cancel;
    input->start_pressed   = cur_start   && !input->_prev_start;
    input->up_pressed      = cur_up      && !input->_prev_up;
    input->down_pressed    = cur_down    && !input->_prev_down;
    input->left_pressed    = cur_left    && !input->_prev_left;
    input->right_pressed   = cur_right   && !input->_prev_right;

    input->_prev_confirm = cur_confirm;
    input->_prev_cancel  = cur_cancel;
    input->_prev_start   = cur_start;
    input->_prev_up      = cur_up;
    input->_prev_down    = cur_down;
    input->_prev_left    = cur_left;
    input->_prev_right   = cur_right;
}

void input_sync(Input* input) {
    bool cur_confirm, cur_cancel, cur_start, cur_up, cur_down, cur_left, cur_right;

    if (!input || !s_controller) return;
    input_read_current(input, &cur_confirm, &cur_cancel, &cur_start,
                       &cur_up, &cur_down, &cur_left, &cur_right);

    input->confirm_pressed = false;
    input->cancel_pressed = false;
    input->start_pressed = false;
    input->up_pressed = false;
    input->down_pressed = false;
    input->left_pressed = false;
    input->right_pressed = false;

    input->_prev_confirm = cur_confirm;
    input->_prev_cancel = cur_cancel;
    input->_prev_start = cur_start;
    input->_prev_up = cur_up;
    input->_prev_down = cur_down;
    input->_prev_left = cur_left;
    input->_prev_right = cur_right;
}
