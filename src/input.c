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

    input->left = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT) != 0;
    input->right = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) != 0;
    input->down = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN) != 0;
    input->jump = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_A) != 0;
    input->shoulder_l = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER) != 0;
    input->shoulder_r = SDL_GameControllerGetButton(s_controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) != 0;
}

