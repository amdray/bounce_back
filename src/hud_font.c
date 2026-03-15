#include "hud_font.h"

#include "font_atlas.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    SDL_Texture* tex;
} HudPage;

typedef struct {
    const FontAtlas* atlas;
    HudPage* pages;
    int page_count;
} HudAtlasRuntime;

static HudAtlasRuntime g_rt[3];

static int slot_from_height(int h) {
    if (h == 23) return 2;
    if (h == 12) return 1;
    return 0;
}

static int utf8_decode_to_codepoint(const char* str, int* bytes_read) {
    unsigned char c = (unsigned char)str[0];
    *bytes_read = 1;

    if (c < 0x80) return c;

    if ((c & 0xE0) == 0xC0) {
        unsigned char c2 = (unsigned char)str[1];
        if (c2 == '\0' || (c2 & 0xC0) != 0x80) return 0x20;
        *bytes_read = 2;
        return ((c & 0x1F) << 6) | (c2 & 0x3F);
    }

    if ((c & 0xF0) == 0xE0) {
        unsigned char c2 = (unsigned char)str[1];
        unsigned char c3 = (unsigned char)str[2];
        if (c2 == '\0' || c3 == '\0') return 0x20;
        if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80) return 0x20;
        *bytes_read = 3;
        return ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
    }

    if ((c & 0xF8) == 0xF0) {
        unsigned char c2 = (unsigned char)str[1];
        unsigned char c3 = (unsigned char)str[2];
        unsigned char c4 = (unsigned char)str[3];
        if (c2 == '\0' || c3 == '\0' || c4 == '\0') return 0x20;
        if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80 || (c4 & 0xC0) != 0x80) return 0x20;
        *bytes_read = 4;
        return ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
    }

    return 0x20;
}

static SDL_Texture* create_texture_from_t4(SDL_Renderer* renderer, const texture_t* src) {
    if (!renderer || !src || !src->data || src->width <= 0 || src->height <= 0) return NULL;

    int px_count = src->width * src->height;
    uint32_t* rgba = (uint32_t*)malloc((size_t)px_count * sizeof(uint32_t));
    if (!rgba) return NULL;

    /* SDL_PIXELFORMAT_RGBA8888: bits 31-24=R, 23-16=G, 15-8=B, 7-0=A */
    const uint32_t px_opaque_white = 0xFFFFFFFFu; /* R=255 G=255 B=255 A=255 */
    const uint32_t px_transparent  = 0x00000000u; /* fully transparent        */

    const uint8_t* packed = (const uint8_t*)src->data;
    for (int i = 0; i < px_count; i += 2) {
        uint8_t b = packed[i >> 1];
        uint8_t idx0 = (uint8_t)(b & 0x0F);
        uint8_t idx1 = (uint8_t)((b >> 4) & 0x0F);
        rgba[i] = (idx0 != 0) ? px_opaque_white : px_transparent;
        if (i + 1 < px_count) {
            rgba[i + 1] = (idx1 != 0) ? px_opaque_white : px_transparent;
        }
    }

    SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, src->width, src->height);
    if (!tex) {
        free(rgba);
        return NULL;
    }

    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    if (SDL_UpdateTexture(tex, NULL, rgba, src->width * (int)sizeof(uint32_t)) != 0) {
        SDL_DestroyTexture(tex);
        tex = NULL;
    }

    free(rgba);
    return tex;
}

static int build_atlas(SDL_Renderer* renderer, int font_height) {
    int slot = slot_from_height(font_height);
    HudAtlasRuntime* rt = &g_rt[slot];

    rt->atlas = font_atlas_get(font_height);
    if (!rt->atlas) return -1;

    rt->page_count = rt->atlas->page_count;
    rt->pages = (HudPage*)calloc((size_t)rt->page_count, sizeof(HudPage));
    if (!rt->pages) return -1;

    for (int i = 0; i < rt->page_count; i++) {
        rt->pages[i].tex = create_texture_from_t4(renderer, &rt->atlas->pages[i]);
        if (!rt->pages[i].tex) return -1;
    }

    return 0;
}

int hud_font_init(SDL_Renderer* renderer) {
    memset(g_rt, 0, sizeof(g_rt));
    if (build_atlas(renderer, 9) != 0) return -1;
    if (build_atlas(renderer, 12) != 0) return -1;
    if (build_atlas(renderer, 23) != 0) return -1;
    return 0;
}

void hud_font_shutdown(void) {
    for (int s = 0; s < 3; s++) {
        HudAtlasRuntime* rt = &g_rt[s];
        if (rt->pages) {
            for (int i = 0; i < rt->page_count; i++) {
                if (rt->pages[i].tex) SDL_DestroyTexture(rt->pages[i].tex);
            }
            free(rt->pages);
        }
        rt->pages = NULL;
        rt->atlas = NULL;
        rt->page_count = 0;
    }
}

int hud_font_measure_text(const char* text, int font_height) {
    if (!text) return 0;
    const FontAtlas* atlas = font_atlas_get(font_height);
    if (!atlas) return 0;

    int width = 0;
    int i = 0;
    while (text[i] != '\0') {
        int bytes_read = 1;
        int cp = utf8_decode_to_codepoint(&text[i], &bytes_read);
        const FontGlyph* g = font_atlas_lookup(atlas, (u32)cp);
        if (g) width += g->w;
        i += bytes_read;
    }
    return width;
}

void hud_font_draw_text(SDL_Renderer* renderer, int x, int y, const char* text, SDL_Color color, int font_height) {
    if (!renderer || !text) return;

    HudAtlasRuntime* rt = &g_rt[slot_from_height(font_height)];
    if (!rt->atlas || !rt->pages) return;

    int cur_x = x;
    int i = 0;
    int page = -1;
    while (text[i] != '\0') {
        int bytes_read = 1;
        int cp = utf8_decode_to_codepoint(&text[i], &bytes_read);
        const FontGlyph* g = font_atlas_lookup(rt->atlas, (u32)cp);
        if (!g || g->w == 0 || g->h == 0) {
            i += bytes_read;
            continue;
        }

        if ((int)g->page != page) {
            page = (int)g->page;
            if (page < 0 || page >= rt->page_count) {
                i += bytes_read;
                continue;
            }
            SDL_SetTextureColorMod(rt->pages[page].tex, color.r, color.g, color.b);
            SDL_SetTextureAlphaMod(rt->pages[page].tex, color.a);
        }

        SDL_Rect src = { g->x, g->y, g->w, g->h };
        SDL_Rect dst = { cur_x, y, g->w, g->h };
        SDL_RenderCopy(renderer, rt->pages[page].tex, &src, &dst);

        cur_x += g->w;
        i += bytes_read;
    }
}
