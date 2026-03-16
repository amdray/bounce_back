/**
 * game_assets.c — one-time load of shared res/ic textures.
 */

#include "game_assets.h"
#include "resource_loader.h"

#include <SDL2/SDL_image.h>
#include <string.h>

/* ── internal helper ───────────────────────────────────────────────────── */

static SDL_Texture* decode_ic_entry(SDL_Renderer* renderer,
                                    ResourceContainer* rc, int index)
{
    size_t sz = 0;
    const uint8_t* data = resource_get_element(rc, (uint16_t)index, &sz);
    if (!data || sz == 0) return NULL;

    SDL_RWops* rw = SDL_RWFromConstMem(data, (int)sz);
    if (!rw) return NULL;

    SDL_Surface* surf = IMG_Load_RW(rw, 1);
    if (!surf) return NULL;

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return tex;
}

/* ── public API ────────────────────────────────────────────────────────── */

int game_assets_init(GameAssets* assets, SDL_Renderer* renderer, FILE* log)
{
    if (!assets || !renderer) return -1;
    memset(assets, 0, sizeof(*assets));

    ResourceContainer* ic = resource_load("res/ic");
    if (!ic) {
        if (log) fprintf(log, "game_assets_init: failed to load res/ic\n");
        return -1;
    }

    assets->ic.lives_strip = decode_ic_entry(renderer, ic, 1);
    assets->ic.ring_icon   = decode_ic_entry(renderer, ic, 2);
    for (int i = 0; i < 4; i++)
        assets->ic.hoop[i] = decode_ic_entry(renderer, ic, 3 + i);
    assets->ic.door        = decode_ic_entry(renderer, ic, 7);
    for (int i = 0; i < 4; i++)
        assets->ic.object_sprite[i] = decode_ic_entry(renderer, ic, 8 + i);

    /* The raw container is only needed for decoding; release it now. */
    resource_free(ic);

    /* Validate mandatory textures — hoop, door, and moving object sprites are required
     * for correct gameplay rendering.  HUD textures (lives_strip, ring_icon)
     * are optional: the draw functions already guard for NULL. */
    for (int i = 0; i < 4; i++) {
        if (!assets->ic.hoop[i]) {
            if (log) fprintf(log, "game_assets_init: missing ic[%d] (hoop)\n", 3 + i);
            game_assets_shutdown(assets);
            return -1;
        }
        if (!assets->ic.object_sprite[i]) {
            if (log) fprintf(log, "game_assets_init: missing ic[%d] (object sprite)\n", 8 + i);
            game_assets_shutdown(assets);
            return -1;
        }
    }
    if (!assets->ic.door) {
        if (log) fprintf(log, "game_assets_init: missing ic[7] (door)\n");
        game_assets_shutdown(assets);
        return -1;
    }

    if (log) fprintf(log, "game_assets_init: res/ic decoded (11 textures)\n");
    return 0;
}

void game_assets_shutdown(GameAssets* assets)
{
    if (!assets) return;

    IcAssets* ic = &assets->ic;

    if (ic->lives_strip) { SDL_DestroyTexture(ic->lives_strip); ic->lives_strip = NULL; }
    if (ic->ring_icon)   { SDL_DestroyTexture(ic->ring_icon);   ic->ring_icon   = NULL; }
    for (int i = 0; i < 4; i++) {
        if (ic->hoop[i])          { SDL_DestroyTexture(ic->hoop[i]);          ic->hoop[i] = NULL; }
        if (ic->object_sprite[i]) { SDL_DestroyTexture(ic->object_sprite[i]); ic->object_sprite[i] = NULL; }
    }
    if (ic->door) { SDL_DestroyTexture(ic->door); ic->door = NULL; }
}
