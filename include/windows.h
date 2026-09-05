// This header is unrelated to the NT Operating System.
#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <stdbool.h>
#include <defs.h>

#define TITLEBAR_HEIGHT 30.0f

typedef enum {
    APP_DISCOVER,
    // TODO: games, demos and other stuff
} App;

typedef struct Window {
    App app;

    const char* title;
    TTF_Text* titleText;

    bool isFocused;
    SDL_FRect rect;

    bool isDragging;
    SDL_FPoint dragOffset;

    void* userData;
} Window;

void InitWindows();
void DeinitWindows();

Window* WindowCreate(App app);
void WindowDestroy(Window* win);
void WindowBringToFront(Window* win);
void WindowFocus(Window* win);

void RenderWindows(SDL_Renderer* renderer);
bool HandleWindowEvent(const SDL_Event* event, SDL_FPoint mouse);
bool IsMouseOverWindow(SDL_FPoint mouse);
bool WindowWantsPointerCursor(SDL_FPoint mouse);
