#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <windows.h>

#define CALC_WIDTH 450.0f
#define CALC_HEIGHT 600.0f

void CalcAppInit(Window* win);
void CalcAppCleanup(Window* win);

void CalcAppRender(Window* win, SDL_Renderer* renderer, SDL_FRect content_rect);
bool CalcAppHandleEvent(Window* win, const SDL_Event* event, SDL_FPoint local_mouse);
bool CalcAppWantsPointerCursor(Window* win, SDL_FPoint local_mouse);

