// This header is unrelated to the NT Operating System.
#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <stdbool.h>
#include <defs.h>
#include <utils.h>

#define TITLEBAR_HEIGHT 30.0f

typedef enum {
    APP_DISCOVER,
    APP_CALCULATOR,
    APP_SOUNDBOARD,
    APP_BRAINFUCK,
    APP_LSTORAGE,
    APP_SNAKE,
} App;

typedef struct FT FT;

typedef struct Window {
    App app;

    const char* title;
    TTF_Text* titleText;

    bool isFocused;
    SDL_FRect rect;

    bool isDragging;
    SDL_FPoint dragOffset;

    bool showingInfo;
    FT* infoFt;
    Scroll infoScroll;

    void* userData;
} Window;

void InitWindows();
void DeinitWindows();

Window* WindowCreate(App app);
void WindowReplaceApp(Window* win, App newApp);
void WindowDestroy(Window* win);
void WindowBringToFront(Window* win);
void WindowFocus(Window* win);

SDL_FRect GetWindowContentRect(Window* window);

void RenderWindows(SDL_Renderer* renderer, float dt);
bool HandleWindowEvent(const SDL_Event* event, SDL_FPoint mouse);

bool IsMouseOverWindow(SDL_FPoint mouse);
CursorKind WindowsGetCursorKind(SDL_FPoint mouse);
