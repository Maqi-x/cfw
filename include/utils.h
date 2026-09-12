#pragma once

#include <SDL3/SDL.h>
#include <math.h>

#include <config.h>
#include <fonts.h>

// winapi grade coding
#define MAX(p, q) (((p) > (q)) ? (p) : (q))
#define MIN(p, q) (((p) < (q)) ? (p) : (q))

typedef struct {
    float curr;
    float target;
} Scroll;

static inline void ScrollUpdate(Scroll* scroll, float dt) {
    float factor = 1.0f - expf(-SCROLL_SPEED * dt);
    scroll->curr += (scroll->target - scroll->curr) * factor;
}

static inline void Clamp(float* value, float max) {
    if (*value > max) *value = max;
    if (*value < 0)   *value = 0;
}

static inline void ScrollClamp(Scroll* scroll, float maxScroll) {
    if (maxScroll < 0.0f) maxScroll = 0;

    Clamp(&scroll->target, maxScroll);
    Clamp(&scroll->curr,   maxScroll);
}

static inline void ScrollOnWheel(Scroll* scroll, float wheelY) {
    scroll->target -= wheelY * SCROLL_WHEEL_STEP;
}

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

static inline Style* GetStyle() {
    static Style style;
    static bool initialized;

    if (!initialized)
        initialized = true,
        style = (Style) {
            .normal = f.normal, .bold = f.bold, .italic = f.italic,
            .h1 = f.h1, .h2 = f.h2, .code = f.code,

            .textColor = textColor,
            .linkColor = linkColor,
        };

    return &style;
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
