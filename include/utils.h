#pragma once

#include <SDL3/SDL.h>
#include <config.h>

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

