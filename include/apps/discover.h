#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <windows.h>

#define DISCOVER_WIDTH 460.0f
#define DISCOVER_HEIGHT 300.0f

void DiscoverAppInit(Window* win);
void DiscoverAppCleanup(Window* win);

void DiscoverAppRender(Window* win, SDL_Renderer* renderer, SDL_FRect content_rect);
bool DiscoverAppHandleEvent(Window* win, const SDL_Event* event, SDL_FPoint local_mouse);
