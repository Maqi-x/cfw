#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <windows.h>

void SnakeAppInit(Window* win);
void SnakeAppCleanup(Window* win);

void SnakeAppRender(Window* win, SDL_Renderer* renderer, SDL_FRect contentRect);
bool SnakeAppHandleEvent(Window* win, const SDL_Event* event, SDL_FPoint localMouse);
