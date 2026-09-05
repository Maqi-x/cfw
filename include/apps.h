#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <windows.h>
#include <apps/discover.h>

void InitApp(Window* win);
void CleanupApp(Window* win);

void RenderApp(Window* win, SDL_Renderer* renderer, SDL_FRect content_rect);
bool HandleAppEvent(Window* win, const SDL_Event* event, SDL_FPoint local_mouse);

const char* GetAppTitle(App app);
void GetAppSize(App app, float* w, float* h);
