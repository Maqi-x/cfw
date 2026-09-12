#pragma once

#include <SDL3/SDL.h>
#include <config.h>

// winapi grade coding
#define MAX(p, q) (((p) > (q)) ? (p) : (q))
#define MIN(p, q) (((p) < (q)) ? (p) : (q))

static inline SDL_Texture* LoadTexPNG(const char* file) {
    SDL_Surface* surf = SDL_LoadPNG(file);
    if (surf == NULL) {
        SDL_Log("Failed to load %s: %s", file, SDL_GetError());
        return NULL;
    }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_DestroySurface(surf);

    if (tex == NULL) {
        SDL_Log("Failed to create texture from %s: %s", file, SDL_GetError());
        return NULL;
    }

    return tex;
}

static inline SDL_FRect OffsetRect(SDL_FRect content, SDL_FRect local) {
    return (SDL_FRect) {
        .x = content.x + local.x,
        .y = content.y + local.y,
        .w = local.w,
        .h = local.h,
    };
}

static inline Uint8 BrighterE(Uint8 element) {
    return MIN(element + 30, 255);
}

static inline Uint8 MoreBrighterE(Uint8 element) {
    return MIN(element + 38, 255);
}

static inline SDL_Color Brighter(SDL_Color bg) {
    bg.r = BrighterE(bg.r);
    bg.g = BrighterE(bg.g);
    bg.b = BrighterE(bg.b);
    return bg;
}
