// THIS IS NOT WINAPI. I should've named this header somehow differently.
#include <windows.h>

#include <fonts.h>
#include <apps.h>

#include <stdlib.h>
#include <string.h>

#define TITLEBAR_COLOR_FOCUS   21, 21, 28, 255
#define TITLEBAR_COLOR_NOFOCUS 13, 13, 18, 255

#define TITLE_COLOR_FOCUS   240, 240, 240, 255
#define TITLE_COLOR_NOFOCUS 140, 140, 145, 255

#define BORDER_COLOR_FOCUS   65, 65, 80, 255
#define BORDER_COLOR_NOFOCUS 40, 40, 48, 255

#define CLOSE_HOVER_COLOR 215, 50, 50, 255
#define X_MARK_COLOR      240, 240, 240, 255

#define SetColor(IS_FOCUSED, NAME)                        \
    if (IS_FOCUSED) {                                     \
        SDL_SetRenderDrawColor(renderer, NAME##_FOCUS);   \
    } else {                                              \
        SDL_SetRenderDrawColor(renderer, NAME##_NOFOCUS); \
    }

#define SetTextColor(IS_FOCUSED, TEXT, NAME)    \
    if (IS_FOCUSED) {                           \
        TTF_SetTextColor(TEXT, NAME##_FOCUS);   \
    } else {                                    \
        TTF_SetTextColor(TEXT, NAME##_NOFOCUS); \
    }

#define MAX_WINDOWS 32

#define WITEREX(NAME, IT, ...)                     \
    do {                                           \
        for (uint IT = 0; IT < numWindows; ++IT) { \
            Window* NAME = windows[IT];            \
            if (NAME != NULL)                      \
                __VA_ARGS__                        \
        }                                          \
    } while (0)

#define WITER(NAME, ...) WITEREX(NAME, _WITER_it, __VA_ARGS__)

#define WTOP() \
    ((numWindows > 0) ? windows[numWindows - 1] : NULL)

static Window* windows[MAX_WINDOWS];
static uint numWindows = 0;

// cool windows xp like effect
static float cascadeOffset;

static SDL_FRect GetTitlebarRect(const Window* win) {
    return (SDL_FRect) {
        .x = win->rect.x,
        .y = win->rect.y,
        .w = win->rect.w,
        .h = TITLEBAR_HEIGHT,
    };
}
static SDL_FRect GetCloseBtnRect(const Window* win) {
    return (SDL_FRect) {
        .x = win->rect.x + win->rect.w - TITLEBAR_HEIGHT,
        .y = win->rect.y,
        .w = TITLEBAR_HEIGHT,
        .h = TITLEBAR_HEIGHT
    };
}
static SDL_FRect GetContentRect(const Window* win) {
    return (SDL_FRect) {
        .x = win->rect.x,
        .y = win->rect.y + TITLEBAR_HEIGHT,
        .w = win->rect.w,
        .h = win->rect.h,
    };
}
static SDL_FRect GetTotalWindowRect(const Window* win) {
    return (SDL_FRect) {
        .x = win->rect.x,
        .y = win->rect.y,
        .w = win->rect.w,
        .h = win->rect.h + TITLEBAR_HEIGHT,
    };
}

static bool TryHandleAppEvent(Window* win, const SDL_Event* event, SDL_FPoint mouse) {
    SDL_FRect contentRect = GetContentRect(win);
    if (SDL_PointInRectFloat(&mouse, &contentRect)) {
        SDL_FPoint local = {
            mouse.x - contentRect.x,
            mouse.y - contentRect.y
        };

        return HandleAppEvent(win, event, local);
    }
    return false;
}

void InitWindows() {
    memset(windows, 0, sizeof(windows));
    numWindows = 0;
    cascadeOffset = 0.0f;
}

void DeinitWindows() {
    WITER(win, {
        CleanupApp(win);
        if (win->titleText != NULL) {
            TTF_DestroyText(win->titleText);
        }
        free(win);
    });
}

Window* WindowCreate(App app) {
    Window* win = calloc(1, sizeof(Window));
    if (win == NULL) return NULL;

    win->app = app;
    win->title = GetAppTitle(app);

    float aw = 400.0f, ah = 300.0f;
    GetAppSize(app, &aw, &ah);

    SDL_FPoint start = {
        .x = 120.0f + cascadeOffset,
        .y = 100.0f + cascadeOffset,
    };

    cascadeOffset += 25.0f;
    if (cascadeOffset > 150.0f) {
        cascadeOffset = 0.0f;
    }

    win->rect = (SDL_FRect) {
        .x = start.x,
        .y = start.y,
        .w = aw, .h = ah
    };

    win->isFocused = true;
    WITER(other,
        // (win is still not added to the
        //  array so this does not affect it)
        other->isFocused = false;
    );

    win->titleText = TTF_CreateText(tengine, f.bold, win->title, strlen(win->title));
    if (win->titleText != NULL) {
        TTF_SetTextColor(win->titleText, 240, 240, 240, 255);
    }

    InitApp(win);

    if (numWindows < MAX_WINDOWS) {
        windows[numWindows++] = win;
        return win;
    } else {
        // TODO: actually show some error message
        CleanupApp(win);
        free(win);
        return NULL;
    }
}

static int GetWindowIndex(Window* win) {
    for (uint i = 0; i < numWindows; ++i) {
        if (windows[i] == win) return i;
    }
    return -1;
}

void WindowDestroy(Window* win) {
    int foundIdx = GetWindowIndex(win);
    if (foundIdx == -1) return;

    for (int i = foundIdx; i < numWindows - 1; ++i) {
        windows[i] = windows[i+1];
    }
    numWindows--;

    CleanupApp(win);
    if (win->titleText != NULL) {
        TTF_DestroyText(win->titleText);
    }
    free(win);

    Window* top = WTOP();
    if (top != NULL) top->isFocused = true;
}

void WindowBringToFront(Window* win) {
    if (win == NULL || numWindows == 0) return;

    int foundIdx = GetWindowIndex(win);
    if (foundIdx != -1) {
        for (int i = foundIdx; i < numWindows - 1; ++i) {
            windows[i] = windows[i+1];
        }
        windows[numWindows - 1] = win;
    }

    WITER(w,
        w->isFocused = (w == win);
    );
}

void WindowFocus(Window* win) {
    WindowBringToFront(win);
}

void RenderWindows(SDL_Renderer* renderer) {
    float mx = 0.0f, my = 0.0f;
    SDL_GetMouseState(&mx, &my);
    SDL_FPoint mouse = { mx, my };

    WITEREX(win, it, {
        SDL_FRect
            totalRect   = GetTotalWindowRect(win),
            titleRect   = GetTitlebarRect(win),
            closeRect   = GetCloseBtnRect(win),
            contentRect = GetContentRect(win);

        // i wish there were some function like SDL_RectFromFRect
        SDL_Rect contentClip = {
            (uint)contentRect.x, (uint)contentRect.y,
            (uint)contentRect.w, (uint)contentRect.h,
        };

        // "app" content //
        SDL_SetRenderClipRect(renderer, &contentClip);
        RenderApp(win, renderer, contentRect);

        // dark overlay for non focused windows so its
        // actually visible which window is the active one
        if (!win->isFocused) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
            SDL_RenderFillRect(renderer, &contentRect);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }

        SDL_SetRenderClipRect(renderer, NULL);

        // separator line //
        SDL_SetRenderDrawColor(renderer, 49, 50, 61, 255);
        SDL_RenderLine(renderer, titleRect.x, titleRect.y + titleRect.h, titleRect.x + titleRect.w, titleRect.y + titleRect.h);

        // title bar //
        SetColor(win->isFocused, TITLEBAR_COLOR);
        SDL_RenderFillRect(renderer, &titleRect);

        // title text //
        int tw = 0, th = 0;
        TTF_GetTextSize(win->titleText, &tw, &th);

        float tx = titleRect.x + 12.0f;
        float ty = titleRect.y + (titleRect.h - th) / 2.0f;

        SetTextColor(win->isFocused, win->titleText, TITLE_COLOR);
        TTF_DrawRendererText(win->titleText, tx, ty);

        // close button //
        bool closeHovered = SDL_PointInRectFloat(&mouse, &closeRect);
        if (closeHovered) {
            SDL_SetRenderDrawColor(renderer, CLOSE_HOVER_COLOR);
            SDL_RenderFillRect(renderer, &closeRect);
        }

        float cx = closeRect.x + closeRect.w / 2.0f;
        float cy = closeRect.y + closeRect.h / 2.0f;
        float d = 4.5f;

        // X //
        SDL_SetRenderDrawColor(renderer, X_MARK_COLOR);
        SDL_RenderLine(renderer, cx - d + 0.5f, cy - d, cx + d + 0.5f, cy + d);
        SDL_RenderLine(renderer, cx + d + 0.5f, cy - d, cx - d + 0.5f, cy + d);

        SetColor(win->isFocused, BORDER_COLOR);
        SDL_RenderRect(renderer, &totalRect);
    });
}

static bool HandleDownLMB(const SDL_Event* event, SDL_FPoint mouse) {
    for (int i = numWindows - 1; i >= 0; --i) {
        Window* win = windows[i];
        if (win == NULL) continue;

        SDL_FRect totalRect = GetTotalWindowRect(win);
        if (SDL_PointInRectFloat(&mouse, &totalRect)) {
            WindowBringToFront(win);

            SDL_FRect closeRect = GetCloseBtnRect(win);
            if (SDL_PointInRectFloat(&mouse, &closeRect)) {
                WindowDestroy(win);
                return true;
            }

            SDL_FRect titleRect = GetTitlebarRect(win);
            if (SDL_PointInRectFloat(&mouse, &titleRect)) {
                win->isDragging = true;
                win->dragOffset = (SDL_FPoint) {
                    mouse.x - win->rect.x,
                    mouse.y - win->rect.y,
                };
                return true;
            }

            TryHandleAppEvent(win, event, mouse);
            return true;
        }
    }

    return false;
}

static bool HandleUpLMB(const SDL_Event* event, SDL_FPoint mouse) {
    bool wasDraggingBefore = false;
    WITER(win, {
        if (win->isDragging) {
            win->isDragging = false;
            wasDraggingBefore = true;
        }
    });

    Window* top = WTOP();
    if (top != NULL)
        if (TryHandleAppEvent(top, event, mouse))
            return true;

    if (wasDraggingBefore || IsMouseOverWindow(mouse)) {
        return true;
    }

    return false;
}

static bool HandleMouseMotion(const SDL_Event* event, SDL_FPoint mouse) {
    WITER(win, {
        if (win->isDragging) {
            win->rect.x = mouse.x - win->dragOffset.x;
            win->rect.y = mouse.y - win->dragOffset.y;
            return true;
        }
    });

    Window* top = WTOP();
    if (top != NULL)
        TryHandleAppEvent(top, event, mouse);

    return IsMouseOverWindow(mouse);
}

bool HandleWindowEvent(const SDL_Event* event, SDL_FPoint mouse) {
    if (numWindows == 0) return false;

    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event->button.button == SDL_BUTTON_LEFT) {
            return HandleDownLMB(event, mouse);
        }
    } else if (event->type == SDL_EVENT_MOUSE_BUTTON_UP) {
        if (event->button.button == SDL_BUTTON_LEFT) {
            return HandleUpLMB(event, mouse);
        }
    } else if (event->type == SDL_EVENT_MOUSE_MOTION) {
        return HandleMouseMotion(event, mouse);
    } else if (event->type == SDL_EVENT_MOUSE_WHEEL) {
        if (IsMouseOverWindow(mouse)) {
            Window* top = WTOP();
            if (top != NULL)
                TryHandleAppEvent(top, event, mouse);

            return true;
        }
    }

    return false;
}

bool IsMouseOverWindow(SDL_FPoint mouse) {
    WITER(win, {
        SDL_FRect totalRect = GetTotalWindowRect(win);
        if (SDL_PointInRectFloat(&mouse, &totalRect)) {
            return true;
        }
    });
    return false;
}

bool WindowWantsPointerCursor(SDL_FPoint mouse) {
    if (numWindows == 0) return false;

    WITER(win, {
        SDL_FRect totalRect = GetTotalWindowRect(win);
        if (SDL_PointInRectFloat(&mouse, &totalRect)) {
            SDL_FRect closeRect = GetCloseBtnRect(win);
            return SDL_PointInRectFloat(&mouse, &closeRect);
        }
    });
    return false;
}
