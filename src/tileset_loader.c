/**
 * Tileset loader implementation (/res/if*).
 */

#include "tileset_loader.h"
#include "resource_loader.h"
#include "texture_loader.h"
#include <stdio.h>
#include <stdlib.h>

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
        tileset->textures[i] = texture_loader_png_from_memory(renderer, data, size);
    }
    resource_free(base);

    if (theme) {
        for (int i = 0; i < theme_count; i++) {
            size_t size = 0;
            const uint8_t* data = resource_get_element(theme, i, &size);
            tileset->textures[base_count + i] = texture_loader_png_from_memory(renderer, data, size);
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
