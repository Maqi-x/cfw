#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <windows.h>

void InitApp(Window* win);
void CleanupApp(Window* win);

void RenderApp(Window* win, SDL_Renderer* renderer, SDL_FRect contectRect);
bool HandleAppEvent(Window* win, const SDL_Event* event, SDL_FPoint localMouse);
bool AppWantsPointerCursor(Window* win, SDL_FPoint localMouse);

const char* GetAppTitle(App app);
void GetAppSize(App app, float* w, float* h);
