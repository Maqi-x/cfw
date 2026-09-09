#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <windows.h>

void SoundboardAppInit(Window* win);
void SoundboardAppCleanup(Window* win);

void SoundboardAppRender(Window* win, SDL_Renderer* renderer, SDL_FRect contentRect);
bool SoundboardAppHandleEvent(Window* win, const SDL_Event* event, SDL_FPoint localMouse);
