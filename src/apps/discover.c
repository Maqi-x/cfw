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
#define ICON_PADDING 60.0f

#define TITLE_OFFSET 5.0f
#define HEADER_MARGIN 55.0f

#define TOTAL_ICON_SIZE \
    (ICON_SIZE + ICON_PADDING)

#define APPS_PER_SECTION 3

typedef struct {
    App app;
    const char* path;
} AppInfo;

typedef struct {
    const char* title;
    AppInfo apps[APPS_PER_SECTION];
} Section;

typedef struct {
    App app;
    SDL_Texture* tex;
    TTF_Text* title;
    SDL_FRect rect;
    bool hovered;
    bool hasApp;
} AppEntry;

static const Section sections[] = {
    {
        .title = "Games",
        .apps = {
            { APP_SNAKE, "assets/icons/snake.png" },
            { 0 }, { 0 }, // TODO
        },
    },
    {
        .title = "Misc",
        .apps = {
            { APP_CALCULATOR, "assets/icons/calculator.png" },
            { APP_BRAINFUCK,  "assets/icons/brainfuck.png"  },
            { 0 }, // TODO
        },
    },
    {
        .title = "Demos",
        .apps ={
            { APP_SOUNDBOARD, "assets/icons/soundboard.png" },
            { APP_LSTORAGE,   "assets/icons/lstorage.png"   },
            { 0 }, // TODO
        },
    },
};

#define NUM_SECTIONS (sizeof(sections) / sizeof(sections[0]))

typedef struct {
    TTF_Text* title;
    SDL_FPoint titlePos;
    AppEntry apps[APPS_PER_SECTION];
} SectionState;

typedef struct {
    SectionState sections[NUM_SECTIONS];
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

    float sx = (DISCOVER_WIDTH - (APPS_PER_SECTION * ICON_SIZE + 2 * ICON_PADDING)) / 2;

    for (uint c = 0; c < NUM_SECTIONS; ++c) {
        state->sections[c].title = TTF_CreateText(tengine, f.h1, sections[c].title, 0);
        assert(state->sections[c].title != NULL);
        TTF_SetTextColor(state->sections[c].title, TEXT_COLOR);

        int tw, th;
        TTF_GetTextSize(state->sections[c].title, &tw, &th);

        // Clang is complaining about integer division being used in a floating point context...
        // This fucking compiler is so stupid that it doesn't even notice that both operands are
        // compile time constants and result of the division is integral. I really wish GCC had an LSP.
        // GCC is simply superior to Clang by any measure and I will never ever change my mind.
        float numSections = (float)(uint)NUM_SECTIONS;

        state->sections[c].titlePos = (SDL_FPoint) {
            (DISCOVER_WIDTH - tw) / 2.0f,
            10.0f + c * (DISCOVER_HEIGHT / numSections)
        };

        for (uint i = 0; i < APPS_PER_SECTION; ++i) {
            InitIcon(
                &state->sections[c].apps[i],
                sections[c].apps[i].app,
                sections[c].apps[i].path,
                (SDL_FRect){
                    .x = sx + i * TOTAL_ICON_SIZE,
                    .y = state->sections[c].titlePos.y + HEADER_MARGIN,
                    .w = ICON_SIZE,
                    .h = ICON_SIZE,
                }
            );
        }
    }

    win->userData = state;
}

static void DrawAppIconAndName(SDL_FRect contentRect, AppEntry* entry) {
    SDL_FRect irect = OffsetRect(contentRect, entry->rect);

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

void DiscoverAppRender(Window* win, SDL_Renderer* renderer, SDL_FRect contentRect) {
    State* state = win->userData;
    assert(state != NULL);

    SDL_SetRenderDrawColor(renderer, BG_COLOR);
    SDL_RenderFillRect(renderer, &contentRect);

    for (uint c = 0; c < NUM_SECTIONS; ++c) {
        SectionState* sec = &state->sections[c];
        TTF_DrawRendererText(sec->title, contentRect.x + sec->titlePos.x, contentRect.y + sec->titlePos.y);

        for (uint i = 0; i < APPS_PER_SECTION; ++i) {
            if (!sec->apps[i].hasApp) continue;
            DrawAppIconAndName(contentRect, &sec->apps[i]);
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

bool DiscoverAppHandleEvent(Window* win, const SDL_Event* event, SDL_FPoint localMouse) {
    State* s = win->userData;

    bool anyHovered = false;
    for (uint c = 0; c < NUM_SECTIONS; ++c) {
        for (uint i = 0; i < APPS_PER_SECTION; ++i) {
            if (HandleIconEvent(&s->sections[c].apps[i], event, win, localMouse))
                return true;

            if (s->sections[c].apps[i].hovered) {
                anyHovered = true;
            }
        }
    }

    return anyHovered;
}

CursorKind DiscoverAppGetCursorKind(Window* win, SDL_FPoint localMouse) {
    State* s = win->userData;

    for (uint c = 0; c < NUM_SECTIONS; ++c) {
        for (uint i = 0; i < APPS_PER_SECTION; ++i) {
            AppEntry* entry = &s->sections[c].apps[i];
            if (entry->hasApp && SDL_PointInRectFloat(&localMouse, &entry->rect)) {
                return CPOINTER;
            }
        }
    }

    return CARROW;
}

void DiscoverAppCleanup(Window* win) {
    State* state = win->userData;
    assert(state != NULL);

    for (uint c = 0; c < NUM_SECTIONS; ++c) {
        if (state->sections[c].title != NULL) TTF_DestroyText(state->sections[c].title);
        for (uint i = 0; i < APPS_PER_SECTION; ++i) {
            if (state->sections[c].apps[i].tex != NULL) SDL_DestroyTexture(state->sections[c].apps[i].tex);
            if (state->sections[c].apps[i].title != NULL) TTF_DestroyText(state->sections[c].apps[i].title);
        }
    }

    free(state);
    win->userData = NULL;
}
