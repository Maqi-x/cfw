#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <windows.h>

#define BF_HEIGHT 440.0f
#define BF_WIDTH  685.0f

void BfAppInit(Window* win);
void BfAppCleanup(Window* win);

void BfAppRender(Window* win, SDL_Renderer* renderer, SDL_FRect contentRect);
bool BfAppHandleEvent(Window* win, const SDL_Event* event, SDL_FPoint localMouse);

bool BfAppWantsPointerCursor(Window* win, SDL_FPoint localMouse);
void BfAppChangeFocus(Window* win, bool focused);
