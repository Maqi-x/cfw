#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <windows.h>

#define SOUNDBOARD_HEIGHT 440.0f
#define SOUNDBOARD_WIDTH  685.0f

void SoundboardAppInit(Window* win);
void SoundboardAppCleanup(Window* win);

void SoundboardAppRender(Window* win, SDL_Renderer* renderer, SDL_FRect contentRect);
bool SoundboardAppHandleEvent(Window* win, const SDL_Event* event, SDL_FPoint localMouse);
bool SoundboardAppWantsPointerCursor(Window* win, SDL_FPoint localMouse);
