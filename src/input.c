/**
 * Input handling implementation - Step 05.
 */

#include "input.h"

#include <SDL2/SDL.h>

static SDL_GameController* s_controller = NULL;

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
    if (!input || !s_controller) return;

    /* held buttons */
    input->left      = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT)  != 0;
    input->right     = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) != 0;
    input->down      = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN)  != 0;
    input->up        = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_DPAD_UP)    != 0;
    input->jump      = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_A)          != 0;
    input->shoulder_l = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER)  != 0;
    input->shoulder_r = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) != 0;

    /* raw current for edge detection */
    bool cur_confirm = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_A)     != 0;
    bool cur_cancel  = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_B)     != 0;
    bool cur_start   = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_START) != 0;
    bool cur_up      = input->up;
    bool cur_down    = input->down;
    bool cur_left    = input->left;
    bool cur_right   = input->right;

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

