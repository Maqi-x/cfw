#include <apps/calculator.h>
#include <string.h>
#include <utils.h>
#include <fonts.h>

#include <stdlib.h>
#include <assert.h>

#define BUFSIZE 30

#define C1 ((SDL_Color) { 17, 25,  36  })
#define C2 ((SDL_Color) { 21, 34,  51  })
#define C3 ((SDL_Color) { 23, 108, 235 })
#define C4 ((SDL_Color) { 54, 130, 245 })

typedef struct {
    char label;
    SDL_Color bg;
} ButtonDef;

static const ButtonDef defs[] = {
    { 'C', C4 }, { '0', C1 }, { '.', C1 }, { '+', C3 },
    { '1', C1 }, { '2', C1 }, { '3', C1 }, { '-', C3 },
    { '4', C1 }, { '5', C1 }, { '6', C1 }, { '*', C3 },
    { '7', C1 }, { '8', C1 }, { '9', C1 }, { '/', C3 },
    { '(', C2 }, { ')', C2 }, { '=', C4 }, { '^', C3 },
};

#define COLUMNS 4
#define ROWS    5

#define DISPLAY_HEIGHT 160.0f
#define BTN_AREA_HEIGH (CALC_HEIGHT - DISPLAY_HEIGHT)

#define BUTTON_WIDTH  (CALC_WIDTH / COLUMNS)
#define BUTTON_HEIGHT (BTN_AREA_HEIGH / ROWS)

#define NUM_BUTTONS \
    (COLUMNS * ROWS)

#define NOHOVER NUM_BUTTONS

typedef struct {
    char label;
    TTF_Text* tlabel;
    SDL_FRect rect;
    SDL_Color bg;
} Button;

typedef struct {
    Button buttons[NUM_BUTTONS];
    char buffer[BUFSIZE];
    TTF_Text* text;
    TTF_Font* currentFont;

    bool isResult;
    bool overflow;

    uint hoveredButton;
} State;

void CalcAppInit(Window* win) {
    State* state = malloc(sizeof(State));

    memset(state->buttons, 0, sizeof(state->buttons));
    state->isResult = false;

    state->hoveredButton = -1;
    state->buffer[0] = '0';
    state->buffer[1] = '\0';

    state->currentFont = f.h1;

    state->text = TTF_CreateText(tengine, f.h1, state->buffer, 0);
    assert(state->text != NULL);

    for (uint i = 0; i < NUM_BUTTONS; ++i) {
        uint row = i / COLUMNS;
        uint col = i % COLUMNS;

        TTF_Text* tlabel = TTF_CreateText(tengine, f.h1, &defs[i].label, 1);
        assert(tlabel != NULL);

        state->buttons[i] = (Button) {
            .label = defs[i].label,
            .tlabel = tlabel,
            .bg = defs[i].bg,
            .rect = {
                .x = col * BUTTON_WIDTH,
                .y = DISPLAY_HEIGHT + (row * BUTTON_HEIGHT),
                .w = BUTTON_WIDTH,
                .h = BUTTON_HEIGHT,
            },
        };
    }

    win->userData = state;
}

void CalcAppCleanup(Window* win) {
    State* state = win->userData;
    TTF_DestroyText(state->text);
    free(win->userData);
}

static SDL_Color Darker(SDL_Color bg) {
    bg.r = MIN(bg.r + 30, 255);
    bg.g = MIN(bg.g + 30, 255);
    bg.b = MIN(bg.b + 30, 255);
    return bg;
}

void CalcAppRender(Window* win, SDL_Renderer* renderer, SDL_FRect content_rect) {
    State* state = win->userData;

    /// display ///
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &(SDL_FRect) {
        .x = content_rect.x,
        .y = content_rect.y,
        .w = content_rect.w,
        .h = DISPLAY_HEIGHT,
    });

    /// buttons ///
    for (int i = 0; i < NUM_BUTTONS; ++i) {
        Button* btn = &state->buttons[i];
        SDL_FRect rect = {
            .x = content_rect.x + btn->rect.x,
            .y = content_rect.y + btn->rect.y,
            .w = btn->rect.w, btn->rect.h,
        };

        /// background ///
        SDL_Color bg = btn->bg;
        if (state->hoveredButton == i)
            bg = Darker(bg);

        SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, 255);
        SDL_RenderFillRect(renderer, &rect);

        /// border ///
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderRect(renderer, &rect);

        // label //
        int tw, th;
        TTF_GetTextSize(btn->tlabel, &tw, &th);

        TTF_DrawRendererText(btn->tlabel,
                rect.x + (rect.w - tw) / 2,
                rect.y + (rect.h - th) / 2);
    }
}

bool CalcAppHandleEvent(Window* win, const SDL_Event* event, SDL_FPoint local_mouse) {
    State* state = win->userData;
    if (event->type == SDL_EVENT_MOUSE_MOTION) {
        state->hoveredButton = NOHOVER;
        for (uint i = 0; i < NUM_BUTTONS; ++i) {
            if (SDL_PointInRectFloat(&local_mouse, &state->buttons[i].rect)) {
                state->hoveredButton = i;
                break;
            }
        }
    }
    return false;
}

bool CalcAppWantsPointerCursor(Window* win, SDL_FPoint local_mouse) {
    State* state = win->userData;
    return state->hoveredButton != NOHOVER;
}

