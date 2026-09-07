#include <apps.h>

// the dispatch boilerplate.
#include <apps/discover.h>
#include <apps/snake.h>
#include <apps/soundboard.h>
#include <apps/calculator.h>

void InitApp(Window* win) {
    switch (win->app) {
    case APP_DISCOVER:
        return DiscoverAppInit(win);
    case APP_SNAKE:
        return SnakeAppInit(win);
    case APP_SOUNDBOARD:
        return SoundboardAppInit(win);
    case APP_CALCULATOR:
        return CalcAppInit(win);
    }
    unreachable();
}

void CleanupApp(Window* win) {
    switch (win->app) {
    case APP_DISCOVER:
        return DiscoverAppCleanup(win);
    case APP_SNAKE:
        return SnakeAppCleanup(win);
    case APP_SOUNDBOARD:
        return SoundboardAppCleanup(win);
    case APP_CALCULATOR:
        return CalcAppCleanup(win);
    }
    unreachable();
}

void RenderApp(Window* win, SDL_Renderer* renderer, SDL_FRect content_rect) {
    switch (win->app) {
    case APP_DISCOVER:
        return DiscoverAppRender(win, renderer, content_rect);
    case APP_SNAKE:
        return SnakeAppRender(win, renderer, content_rect);
    case APP_SOUNDBOARD:
        return SoundboardAppRender(win, renderer, content_rect);
    case APP_CALCULATOR:
        return CalcAppRender(win, renderer, content_rect);
    }
    unreachable();
}

bool HandleAppEvent(Window* win, const SDL_Event* event, SDL_FPoint local_mouse) {
    switch (win->app) {
    case APP_DISCOVER:
        return DiscoverAppHandleEvent(win, event, local_mouse);
    case APP_SNAKE:
        return SnakeAppHandleEvent(win, event, local_mouse);
    case APP_SOUNDBOARD:
        return SoundboardAppHandleEvent(win, event, local_mouse);
    case APP_CALCULATOR:
        return CalcAppHandleEvent(win, event, local_mouse);
    }
    unreachable();
}

bool AppWantsPointerCursor(Window* win, SDL_FPoint local_mouse) {
    switch (win->app) {
    case APP_DISCOVER:
        return DiscoverAppWantsPointerCursor(win, local_mouse);
    case APP_CALCULATOR:
        return CalcAppWantsPointerCursor(win, local_mouse);
    default:
        return false;
    }
}

const char* GetAppTitle(App app) {
    switch (app) {
    case APP_DISCOVER:
        return "Discover";
    case APP_SNAKE:
        return "Snake";
    case APP_SOUNDBOARD:
        return "Soundboard";
    case APP_CALCULATOR:
        return "Calculator";
    }
    unreachable();
}

void GetAppSize(App app, float* w, float* h) {
    switch (app) {
    case APP_DISCOVER:
        if (w) *w = DISCOVER_WIDTH;
        if (h) *h = DISCOVER_HEIGHT;
        break;
    case APP_CALCULATOR:
        if (w) *w = CALC_WIDTH;
        if (h) *h = CALC_HEIGHT;
        break;
    default:
        if (w) *w = 400.0f;
        if (h) *h = 300.0f;
        break;
    }
}
