#pragma once

#include <SDL3_ttf/SDL_ttf.h>

typedef struct {
    TTF_Font
        *normal,
        *bold,
        *italic,
        *h1,
        *h2;
} Fonts;

extern Fonts f;

void CloseFont(TTF_Font** font);
void UnloadFonts();
bool LoadFonts();
