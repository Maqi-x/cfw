#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <windows.h>

#define DISCOVER_WIDTH  460.0f
#define DISCOVER_HEIGHT 565.0f

#define APPS_PER_SECTION 3

typedef struct {
    App app;
    const char* path;
} AppInfo;

typedef struct {
    const char* title;
    AppInfo apps[APPS_PER_SECTION];
} Section;

void DiscoverAppInit(Window* win);
void DiscoverAppCleanup(Window* win);

void DiscoverAppRender(Window* win, SDL_Renderer* renderer, SDL_FRect contectRect);
bool DiscoverAppHandleEvent(Window* win, const SDL_Event* event, SDL_FPoint localMouse);
bool DiscoverAppWantsPointerCursor(Window* win, SDL_FPoint localMouse);
