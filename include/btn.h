#pragma once

#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL.h>

#include <stdbool.h>

typedef struct {
    TTF_Text* text;
    SDL_FRect rect;

    SDL_Color bg;
    bool hasBg;

    bool isHovered;
    bool isClicked;
    bool wasClicked;
} Button;

Button BtnCreate(TTF_Font* font, const char* label);
Button BtnCreateBg(
    TTF_Font* font, const char* label,
    Uint8 r, Uint8 g, Uint8 b, Uint8 a
);

void BtnDestroy(Button* btn);
void BtnFitToText(Button* btn);
void BtnDraw(SDL_Renderer* renderer, const Button* btn);
void BtnDrawOffset(SDL_Renderer* renderer, const Button* btn, SDL_FRect origin);

bool BtnJustClicked(const Button* btn);
bool BtnContains(const Button* btn, SDL_FPoint point);
void BtnHandleEvent(Button* btn, const SDL_Event* event, SDL_FPoint mouse);
