/**
 * Tileset loader implementation (/res/if*).
 */

#include "tileset_loader.h"
#include "resource_loader.h"

#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>

static SDL_Texture* load_png_texture_from_mem(SDL_Renderer* renderer, const uint8_t* data, size_t size) {
    if (!renderer || !data || size == 0) return NULL;

    SDL_RWops* rw = SDL_RWFromConstMem(data, (int)size);
    if (!rw) return NULL;

    SDL_Surface* surface = IMG_LoadTyped_RW(rw, 1, "PNG");
    if (!surface) return NULL;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

Tileset* tileset_load(SDL_Renderer* renderer, const char* if0_path, const char* theme_path) {
    if (!renderer || !if0_path) {
        fprintf(stderr, "tileset_load: invalid args\n");
        return NULL;
    }

    ResourceContainer* base = resource_load(if0_path);
    if (!base) return NULL;

    ResourceContainer* theme = NULL;
    if (theme_path) {
        theme = resource_load(theme_path);
    }

    Tileset* tileset = (Tileset*)calloc(1, sizeof(Tileset));
    if (!tileset) {
        fprintf(stderr, "tileset_load: calloc failed\n");
        if (theme) resource_free(theme);
        resource_free(base);
        return NULL;
    }

    int base_count = (int)base->count;
    int theme_count = theme ? (int)theme->count : 0;
    tileset->count = base_count + theme_count;
    tileset->textures = (SDL_Texture**)calloc((size_t)tileset->count, sizeof(SDL_Texture*));
    if (!tileset->textures) {
        fprintf(stderr, "tileset_load: calloc failed for textures\n");
        free(tileset);
        if (theme) resource_free(theme);
        resource_free(base);
        return NULL;
    }

    for (int i = 0; i < base_count; i++) {
        size_t size = 0;
        const uint8_t* data = resource_get_element(base, i, &size);
        tileset->textures[i] = load_png_texture_from_mem(renderer, data, size);
    }
    resource_free(base);

    if (theme) {
        for (int i = 0; i < theme_count; i++) {
            size_t size = 0;
            const uint8_t* data = resource_get_element(theme, i, &size);
            tileset->textures[base_count + i] = load_png_texture_from_mem(renderer, data, size);
        }
        resource_free(theme);
    }

    return tileset;
}

void tileset_free(Tileset* tileset) {
    if (!tileset) return;
    if (tileset->textures) {
        for (int i = 0; i < tileset->count; i++) {
            if (tileset->textures[i]) SDL_DestroyTexture(tileset->textures[i]);
        }
        free(tileset->textures);
    }
    free(tileset);
}

SDL_Texture* tileset_get(const Tileset* tileset, int image_index) {
    if (!tileset || !tileset->textures) return NULL;
    if (image_index < 0 || image_index >= tileset->count) return NULL;
    return tileset->textures[image_index];
}

