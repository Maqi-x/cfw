#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <stdbool.h>
#include <defs.h>

typedef enum {
    TEXT_NORMAL,

    TEXT_H1,
    TEXT_H2,

    TEXT_BOLD,
    TEXT_ITALIC,
} TextKind;

typedef struct {
    TextKind kind;
    const char* content;
    const char* href;
} TextFragment;

#define T(S, ...)        ((TextFragment) { .kind = TEXT_NORMAL, .content = (S), __VA_ARGS__ })
#define T_BOLD(S, ...)   ((TextFragment) { .kind = TEXT_BOLD,   .content = (S), __VA_ARGS__ })
#define T_ITALIC(S, ...) ((TextFragment) { .kind = TEXT_ITALIC, .content = (S), __VA_ARGS__ })
#define T_H1(S, ...)     ((TextFragment) { .kind = TEXT_H1,     .content = (S), __VA_ARGS__ })
#define T_H2(S, ...)     ((TextFragment) { .kind = TEXT_H2,     .content = (S), __VA_ARGS__ })

// TODO: monospace font etc.
#define T_CODE T_ITALIC

typedef struct FT FT;

typedef struct {
    TTF_Font* normal;
    TTF_Font* bold;
    TTF_Font* italic;

    TTF_Font* h1;
    TTF_Font* h2;

    SDL_Color text_color;
    SDL_Color link_color;

    uint width;
    uint line_gap;
    uint paragraph_gap;
    uint heading_gap;
} Style;

FT* CreateFT(TTF_TextEngine* engine, const Style* style);
void DestroyFT(FT* ft);

bool FTSetWidth(FT* ft, uint width);
bool FTSetFragments(FT* ft, const TextFragment* fragments, usize count);

void FTDraw(FT* ft, float x, float y);
const char* FTGetLinkAt(const FT* ft, float x, float y);
void FTGetSize(const FT* ft, uint* w, uint* h);
void FTClear(FT* ft);
