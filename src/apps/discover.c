#include <apps/discover.h>
#include <apps.h>
#include <fonts.h>
#include <defs.h>
#include <utils.h>

#include <stdlib.h>
#include <assert.h>

#define TEXT_COLOR 255, 255, 255, 255
#define BG_COLOR   24, 24, 30, 255

#define ICON_SIZE 85.0f
#define ICON_PADDING 40.0f

#define TITLE_OFFSET 5.0f
#define HEADER_MARGIN 55.0f

#define TOTAL_ICON_SIZE \
    (ICON_SIZE + ICON_PADDING)

#define APPS_IN_SECTION 3

typedef struct {
    App app;
    SDL_Texture* tex;
    TTF_Text* title;
    SDL_FRect rect;
    bool hovered;
    bool hasApp;
} AppEntry;

struct AppInfo {
    App app;
    const char* path;
};

struct AppInfo games[APPS_IN_SECTION] = {
    { APP_SNAKE, "assets/snake.png" },
    { 0 }, { 0 }, // TODO
};

struct AppInfo demos[APPS_IN_SECTION] = {
    { APP_SOUNDBOARD, "assets/soundboard.png" },
    { 0 }, { 0 }, // TODO
};

typedef struct {
    struct {
        TTF_Text* games;
        TTF_Text* demos;
    } t;

    SDL_FPoint games_pos, demos_pos;
    AppEntry games[APPS_IN_SECTION];
    AppEntry demos[APPS_IN_SECTION];
} State;

static void InitIcon(AppEntry* entry, App app, const char* path, SDL_FRect rect) {
    *entry = (AppEntry) {
        .app  = app,
        .rect = rect,
        .hovered = false,
        .hasApp = (path != NULL),
    };

    entry->tex = entry->hasApp ? LoadTexPNG(path) : NULL;
    if (entry->hasApp) {
        entry->title = TTF_CreateText(tengine, f.normal, GetAppTitle(app), 0);
        assert(entry->title != NULL);
        TTF_SetTextColor(entry->title, TEXT_COLOR);
    } else {
        entry->title = NULL;
    }
}

void DiscoverAppInit(Window* win) {
    State* state = malloc(sizeof(State));
    assert(state != NULL);

    state->t.games = TTF_CreateText(tengine, f.h1, "Games", 0);
    assert(state->t.games != NULL);
    TTF_SetTextColor(state->t.games, TEXT_COLOR);

    state->t.demos = TTF_CreateText(tengine, f.h1, "Demos", 0);
    assert(state->t.demos != NULL);
    TTF_SetTextColor(state->t.demos, TEXT_COLOR);

    int tw, th;
    // The width of both texts is almost identical, so this works just fine.
    TTF_GetTextSize(state->t.games, &tw, &th);

    state->games_pos = (SDL_FPoint){ (DISCOVER_WIDTH - tw) / 2.0f, 10.0f };
    state->demos_pos = (SDL_FPoint){ (DISCOVER_WIDTH - tw) / 2.0f, DISCOVER_HEIGHT / 2.0f };

    float sx = (DISCOVER_WIDTH - (APPS_IN_SECTION * ICON_SIZE + 2 * ICON_PADDING)) / 2;
    for (uint i = 0; i < APPS_IN_SECTION; ++i) {
        // i have no idea how to deduplicate this.
        InitIcon(
            &state->games[i], games[i].app, games[i].path, (SDL_FRect) {
            .x = sx + i * TOTAL_ICON_SIZE,
            .y = state->games_pos.y + HEADER_MARGIN,
            .w = ICON_SIZE, .h = ICON_SIZE,
        });
        InitIcon(&state->demos[i], demos[i].app, demos[i].path, (SDL_FRect) {
            .x = sx + i * TOTAL_ICON_SIZE,
            .y = state->demos_pos.y + HEADER_MARGIN,
            .w = ICON_SIZE, .h = ICON_SIZE,
        });
    }

    win->userData = state;
}

static void DrawAppIconAndName(SDL_FRect content_rect, AppEntry* entry) {
    SDL_FRect irect = {
        .x = content_rect.x + entry->rect.x,
        .y = content_rect.y + entry->rect.y,
        .w = entry->rect.w,
        .h = entry->rect.h
    };

    if (entry->hovered)
        SDL_SetTextureColorMod(entry->tex, 200, 200, 200);
    else
        SDL_SetTextureColorMod(entry->tex, 255, 255, 255);

    SDL_RenderTexture(renderer, entry->tex, NULL, &irect);

    int tw, th;
    TTF_GetTextSize(entry->title, &tw, &th);

    float tx = irect.x + (irect.w - tw) / 2.0f;
    float ty = irect.y + irect.h + TITLE_OFFSET;
    TTF_DrawRendererText(entry->title, tx, ty);
}

void DiscoverAppRender(Window* win, SDL_Renderer* renderer, SDL_FRect content_rect) {
    State* state = win->userData;
    assert(state != NULL);

    SDL_SetRenderDrawColor(renderer, BG_COLOR);
    SDL_RenderFillRect(renderer, &content_rect);

    TTF_DrawRendererText(state->t.games, content_rect.x + state->games_pos.x, content_rect.y + state->games_pos.y);
    TTF_DrawRendererText(state->t.demos, content_rect.x + state->demos_pos.x, content_rect.y + state->demos_pos.y);

    for (uint i = 0; i < APPS_IN_SECTION; ++i) {
        AppEntry* icons[] = { &state->games[i], &state->demos[i] };

        for (uint j = 0; j < 2; j++) {
            if (!icons[j]->hasApp) continue;
            DrawAppIconAndName(content_rect, icons[j]);
        }
    }
}

static bool HandleIconEvent(AppEntry* entry, const SDL_Event* event, Window* win, SDL_FPoint m) {
    bool inRect = SDL_PointInRectFloat(&m, &entry->rect);
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (entry->hasApp && inRect) {
            WindowReplaceApp(win, entry->app);
            return true;
        }
        return false;
    }

    if (!entry->hasApp || !inRect) {
        entry->hovered = false;
    } else if (event->type == SDL_EVENT_MOUSE_MOTION) {
        entry->hovered = true;
    }
    return false;
}

bool DiscoverAppHandleEvent(Window* win, const SDL_Event* event, SDL_FPoint local_mouse) {
    State* s = win->userData;

    bool anyHovered = false;
    for (uint i = 0; i < APPS_IN_SECTION; ++i) {
        if (HandleIconEvent(&s->games[i], event, win, local_mouse)) return true;
        if (HandleIconEvent(&s->demos[i], event, win, local_mouse)) return true;

        if (s->games[i].hovered || s->demos[i].hovered)
            anyHovered = true;
    }

    return anyHovered;
}

bool DiscoverAppWantsPointerCursor(Window* win, SDL_FPoint local_mouse) {
    State* s = win->userData;
    for (uint i = 0; i < APPS_IN_SECTION; ++i) {
        if (s->games[i].hasApp && SDL_PointInRectFloat(&local_mouse, &s->games[i].rect)) return true;
        if (s->demos[i].hasApp && SDL_PointInRectFloat(&local_mouse, &s->demos[i].rect)) return true;
    }
    return false;
}

void DiscoverAppCleanup(Window* win) {
    State* state = win->userData;
    assert(state != NULL);

    TTF_DestroyText(state->t.games);
    TTF_DestroyText(state->t.demos);
    for (uint i = 0; i < APPS_IN_SECTION; ++i) {
        if (state->games[i].tex != NULL) SDL_DestroyTexture(state->games[i].tex);
        if (state->demos[i].tex != NULL) SDL_DestroyTexture(state->demos[i].tex);
        if (state->games[i].title != NULL) TTF_DestroyText(state->games[i].title);
        if (state->demos[i].title != NULL) TTF_DestroyText(state->demos[i].title);
    }

    free(state);
    win->userData = NULL;
}
