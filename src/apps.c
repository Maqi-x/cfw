#include <apps.h>

// the dispatch boilerplate.
#include <apps/calculator.h>
#include <apps/soundboard.h>
#include <apps/brainfuck.h>
#include <apps/discover.h>
#include <apps/snake.h>
#include <apps/lstorage.h>

void InitApp(Window* win) {
    switch (win->app) {
    case APP_DISCOVER:
        return DiscoverAppInit(win);
    case APP_SOUNDBOARD:
        return SoundboardAppInit(win);
    case APP_CALCULATOR:
        return CalcAppInit(win);
    case APP_BRAINFUCK:
        return BfAppInit(win);
    case APP_SNAKE:
        return SnakeAppInit(win);
    case APP_LSTORAGE:
        return LStorageAppInit(win);
    }
    unreachable();
}

void CleanupApp(Window* win) {
    switch (win->app) {
    case APP_DISCOVER:
        return DiscoverAppCleanup(win);
    case APP_SOUNDBOARD:
        return SoundboardAppCleanup(win);
    case APP_CALCULATOR:
        return CalcAppCleanup(win);
    case APP_BRAINFUCK:
        return BfAppCleanup(win);
    case APP_SNAKE:
        return SnakeAppCleanup(win);
    case APP_LSTORAGE:
        return LStorageAppCleanup(win);
    }
    unreachable();
}

void RenderApp(Window* win, SDL_Renderer* renderer, SDL_FRect contentRect) {
    switch (win->app) {
    case APP_DISCOVER:
        return DiscoverAppRender(win, renderer, contentRect);
    case APP_SOUNDBOARD:
        return SoundboardAppRender(win, renderer, contentRect);
    case APP_CALCULATOR:
        return CalcAppRender(win, renderer, contentRect);
    case APP_BRAINFUCK:
        return BfAppRender(win, renderer, contentRect);
    case APP_SNAKE:
        return SnakeAppRender(win, renderer, contentRect);
    case APP_LSTORAGE:
        return LStorageAppRender(win, renderer, contentRect);
    }
    unreachable();
}

bool HandleAppEvent(Window* win, const SDL_Event* event, SDL_FPoint localMouse) {
    switch (win->app) {
    case APP_DISCOVER:
        return DiscoverAppHandleEvent(win, event, localMouse);
    case APP_SOUNDBOARD:
        return SoundboardAppHandleEvent(win, event, localMouse);
    case APP_CALCULATOR:
        return CalcAppHandleEvent(win, event, localMouse);
    case APP_BRAINFUCK:
        return BfAppHandleEvent(win, event, localMouse);
    case APP_SNAKE:
        return SnakeAppHandleEvent(win, event, localMouse);
    case APP_LSTORAGE:
        return LStorageAppHandleEvent(win, event, localMouse);
    }
    unreachable();
}

bool AppWantsPointerCursor(Window* win, SDL_FPoint localMouse) {
    switch (win->app) {
    case APP_DISCOVER:
        return DiscoverAppWantsPointerCursor(win, localMouse);
    case APP_CALCULATOR:
        return CalcAppWantsPointerCursor(win, localMouse);
    case APP_SOUNDBOARD:
        return SoundboardAppWantsPointerCursor(win, localMouse);
    case APP_BRAINFUCK:
        return BfAppWantsPointerCursor(win, localMouse);
    case APP_LSTORAGE:
        return LStorageAppWantsPointerCursor(win, localMouse);
    default:
        return false;
    }
}

bool AppWantsTextCursor(Window* win, SDL_FPoint localMouse) {
    switch (win->app) {
    case APP_LSTORAGE:
        return LStorageAppWantsTextCursor(win, localMouse);
    default:
        return false;
    }
}

void ChangeAppFocus(Window* win, bool focused) {
    switch (win->app) {
    case APP_BRAINFUCK:
        return BfAppChangeFocus(win, focused);
    case APP_LSTORAGE:
        return LStorageAppChangeFocus(win, focused);
    default:
        return;
    }
}

const char* GetAppTitle(App app) {
    switch (app) {
    case APP_DISCOVER:
        return "Discover";
    case APP_SOUNDBOARD:
        return "Soundboard";
    case APP_CALCULATOR:
        return "Calculator";
    case APP_BRAINFUCK:
        return "Brainfuck";
    case APP_SNAKE:
        return "Snake";
    case APP_LSTORAGE:
        return "Local Storage";
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
    case APP_SOUNDBOARD:
        if (w) *w = SOUNDBOARD_WIDTH;
        if (h) *h = SOUNDBOARD_HEIGHT;
        break;
    case APP_BRAINFUCK:
        if (w) *w = BF_WIDTH;
        if (h) *h = BF_HEIGHT;
        break;
    case APP_LSTORAGE:
        if (w) *w = LSTORAGE_WIDTH;
        if (h) *h = LSTORAGE_HEIGHT;
        break;
    default:
        if (w) *w = 400.0f;
        if (h) *h = 300.0f;
        break;
    }
}
