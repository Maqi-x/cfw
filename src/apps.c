#include <apps.h>

// the dispatch boilerplate.

void InitApp(Window* win) {
    switch (win->app) {
    case APP_DISCOVER:
        return DiscoverAppInit(win);
    }
    unreachable();
}

void CleanupApp(Window* win) {
    switch (win->app) {
    case APP_DISCOVER:
        return DiscoverAppCleanup(win);
    }
    unreachable();
}

void RenderApp(Window* win, SDL_Renderer* renderer, SDL_FRect content_rect) {
    switch (win->app) {
    case APP_DISCOVER:
        return DiscoverAppRender(win, renderer, content_rect);
    }
    unreachable();
}

bool HandleAppEvent(Window* win, const SDL_Event* event, SDL_FPoint local_mouse) {
    switch (win->app) {
    case APP_DISCOVER:
        return DiscoverAppHandleEvent(win, event, local_mouse);
    }
    unreachable();
}

const char* GetAppTitle(App app) {
    switch (app) {
    case APP_DISCOVER:
        return "Games and Demos";
    }
    unreachable();
}

void GetAppSize(App app, float* w, float* h) {
    switch (app) {
    case APP_DISCOVER:
        if (w) *w = DISCOVER_WIDTH;
        if (h) *h = DISCOVER_HEIGHT;
        break;
    default:
        if (w) *w = 400.0f;
        if (h) *h = 300.0f;
        break;
    }
}
