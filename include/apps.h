#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <windows.h>

void InitApp(Window* win);
void CleanupApp(Window* win);

void RenderApp(Window* win, SDL_Renderer* renderer, SDL_FRect contentRect);
bool HandleAppEvent(Window* win, const SDL_Event* event, SDL_FPoint localMouse);

CursorKind AppGetCursorKind(Window* win, SDL_FPoint localMouse);
void ChangeAppFocus(Window* win, bool focused);

const char* GetAppTitle(App app);
void GetAppSize(App app, float* w, float* h);
