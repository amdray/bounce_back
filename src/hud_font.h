#ifndef HUD_FONT_H
#define HUD_FONT_H

#include <SDL2/SDL.h>

int hud_font_init(SDL_Renderer* renderer);
void hud_font_shutdown(void);
int hud_font_measure_text(const char* text, int font_height);
void hud_font_draw_text(SDL_Renderer* renderer, int x, int y, const char* text, SDL_Color color, int font_height);

#endif // HUD_FONT_H
