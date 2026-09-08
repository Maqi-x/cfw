#include <apps/calculator.h>

#include <utils.h>
#include <fonts.h>
#include <eval.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

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

#define DISPLAY_HEIGHT   160.0f
#define DISPLAY_PAD_X    16.0f
#define DISPLAY_PAD_Y    12.0f
#define DISPLAY_MAX_ROWS 3

// C does not allow floating point math in constant integer expressions
// even if the result is casted to int and can be evaluated fully at compile
// time. I don't know why. Don't ask me. This expression represents:
//   (22.5 * DISPLAY_MAX_ROWS)
// 22.5 represents number of digits that fit on a single display line.
// Rounding down is actually intentional.
#define BUFSIZE ((225 * DISPLAY_MAX_ROWS) / 10)

#define DISPLAY_TEXT_WIDTH  (CALC_WIDTH - 32.0f)
#define DISPLAY_TEXT_HEIGHT (DISPLAY_HEIGHT - 24.0f)

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
    usize length;
    TTF_Text* text;

    bool isError;
    bool isResult;
    bool overflow;

    uint hoveredButton;
} State;

void CalcAppInit(Window* win) {
    State* state = malloc(sizeof(State));
    assert(state != NULL);

    memset(state->buttons, 0, sizeof(state->buttons));
    state->isError  = false;
    state->isResult = false;
    state->overflow = false;

    state->hoveredButton = -1;
    state->buffer[0] = '0';
    state->length = 1;

    state->text = TTF_CreateText(tengine, f.h1, state->buffer, state->length);
    assert(state->text != NULL);

    for (uint i = 0; i < NUM_BUTTONS; ++i) {
        uint row = i / COLUMNS;
        uint col = i % COLUMNS;

        TTF_Text* tlabel = TTF_CreateText(tengine, f.h1, &defs[i].label, 1);
        assert(tlabel != NULL);
        TTF_SetTextColor(tlabel, 255, 255, 255, 255);

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
    assert(state != NULL);

    TTF_DestroyText(state->text);
    for (uint i = 0; i < NUM_BUTTONS; ++i) {
        TTF_DestroyText(state->buttons[i].tlabel);
    }
    free(win->userData);
    win->userData = NULL;
}

static SDL_Color Brighter(SDL_Color bg) {
    bg.r = MIN(bg.r + 30, 255);
    bg.g = MIN(bg.g + 30, 255);
    bg.b = MIN(bg.b + 30, 255);
    return bg;
}

static bool FitsDisplay(TTF_Font* font, const char* text, usize len) {
    int w, h;
    TTF_GetStringSize(font, text, len, &w, &h);
    return w <= DISPLAY_TEXT_WIDTH && h <= DISPLAY_TEXT_HEIGHT;
}

static int GetButtonAtPoint(const State* state, SDL_FPoint point) {
    for (uint i = 0; i < NUM_BUTTONS; ++i) {
        if (SDL_PointInRectFloat(&point, &state->buttons[i].rect)) {
            return (int)i;
        }
    }
    return NOHOVER;
}

// pascal grade coding
#define LineEnding '\n'

static usize SplitRows(const char* src, usize len, char* dst, TTF_Font* font) {
    if (len <= 1) {
        memcpy(dst, src, len);
        return len;
    }

    usize srcOff = 0, dstOff = 0;
    for (uint r = 0; r < DISPLAY_MAX_ROWS; ++r) {
        if (srcOff == len) break;

        const char* rest = src + srcOff;
        usize restLen = len - srcOff;

        int w, h;
        TTF_GetStringSize(font, rest, restLen, &w, &h);

        bool wrap =
            (w > DISPLAY_TEXT_WIDTH) &&
            (r < DISPLAY_MAX_ROWS - 1);

        usize split = restLen;
        if (wrap) {
            for (split = restLen - 1; split > 1; --split) {
                TTF_GetStringSize(font, src + srcOff, split, &w, &h);
                if (w <= DISPLAY_TEXT_WIDTH) break;
            }
        }

        memcpy(dst + dstOff, src + srcOff, split);
        dstOff += split, srcOff += split;

        if (wrap) dst[dstOff++] = LineEnding;
    }

    return dstOff;
}

static void UpdateDisplayText(State* state) {
    TTF_Font* font = f.h1;
    const char* text = state->buffer;
    usize len = state->length;

    char display[BUFSIZE + 2];
    if (!FitsDisplay(f.h1, state->buffer, state->length)) {
        font = f.h2;

        len = SplitRows(
            state->buffer, state->length,
            display, font
        );

        text = display;
    }

    if (state->isError)
        TTF_SetTextColor(state->text, 191, 42, 42, 255);
    else
        TTF_SetTextColor(state->text, 255, 255, 255, 255);

    TTF_SetTextFont(state->text, font);
    TTF_SetTextString(state->text, text, len);
}

static void SetBuffer(State* state, const char* value) {
    usize len = strlen(value);
    assert(len < sizeof(state->buffer));
    memcpy(state->buffer, value, len + 1);
    state->length = len;
    UpdateDisplayText(state);
}

static bool isOp(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
}

static void AppendBufferChar(State* state, char c) {
    if (state->isResult) {
        if (state->isError || !isOp(c)) {
            state->length = 0;
        }

        state->isError = false;
        state->isResult = false;
    }

    if (state->length == 1 && state->buffer[0] == '0') {
        state->buffer[0] = c;
        state->buffer[1] = '\0';
    } else if (state->length + 1 < sizeof(state->buffer)) {
        state->buffer[state->length] = c;
        state->buffer[state->length + 1] = '\0';
        state->length++;
    } else {
        return;
    }

    UpdateDisplayText(state);
}

static void HandleEvalError(State* state, EvalCode code) {
    state->isError = true;
    switch (code ){
    case EVAL_ERR_INV_SYNTAX:
        return SetBuffer(state, "INVALID SYNTAX");
    case EVAL_ERR_UNEXP_CHAR:
        return SetBuffer(state, "UNEXPECTED CHAR");
    case EVAL_ERR_DIV_BY_ZERO:
        return SetBuffer(state, "DIVISION BY ZERO");
    case EVAL_ERR_NOT_A_NUM:
        return SetBuffer(state, "NOT A NUMBER");
    case EVAL_OK:
        // to make the compiler happy
        unreachable();
    }
}

static void HandleInputSymbol(State* state, char c) {
    switch (c) {
    case '=': {
        state->isResult = true;

        EvalOutput eo = eval(state->buffer, state->length);
        if (eo.ecode != EVAL_OK)
            return HandleEvalError(state, eo.ecode);

        int length =
            snprintf(state->buffer, BUFSIZE, "%g", eo.result);

        if (length > BUFSIZE) {
            state->isError = true;
            SetBuffer(state, "OVERFLOW");
            return;
        }

        state->length = length;
        UpdateDisplayText(state);
        break;
    }
    case 'C':
        return SetBuffer(state, "0");
    default:
        return AppendBufferChar(state, c);
    }
}

static bool HandleKeyEvent(State* state, SDL_Keycode key, SDL_Keymod mod) {
    bool shift = (mod & SDL_KMOD_SHIFT) != 0;

    switch (key) {
    case SDLK_0:
        if (shift) HandleInputSymbol(state, ')');
        else       HandleInputSymbol(state, '0');
        return true;

    case SDLK_1: if (!shift) { HandleInputSymbol(state, '1'); return true; } break;
    case SDLK_2: if (!shift) { HandleInputSymbol(state, '2'); return true; } break;
    case SDLK_3: if (!shift) { HandleInputSymbol(state, '3'); return true; } break;
    case SDLK_4: if (!shift) { HandleInputSymbol(state, '4'); return true; } break;
    case SDLK_5: if (!shift) { HandleInputSymbol(state, '5'); return true; } break;
    case SDLK_6:
        if (shift) HandleInputSymbol(state, '^');
        else       HandleInputSymbol(state, '6');
        return true;
    case SDLK_7: if (!shift) { HandleInputSymbol(state, '7'); return true; } break;
    case SDLK_8:
        if (shift) HandleInputSymbol(state, '*');
        else       HandleInputSymbol(state, '8');
        return true;
    case SDLK_9:
        if (shift) HandleInputSymbol(state, '(');
        else       HandleInputSymbol(state, '9');
        return true;

    case SDLK_KP_0: HandleInputSymbol(state, '0'); return true;
    case SDLK_KP_1: HandleInputSymbol(state, '1'); return true;
    case SDLK_KP_2: HandleInputSymbol(state, '2'); return true;
    case SDLK_KP_3: HandleInputSymbol(state, '3'); return true;
    case SDLK_KP_4: HandleInputSymbol(state, '4'); return true;
    case SDLK_KP_5: HandleInputSymbol(state, '5'); return true;
    case SDLK_KP_6: HandleInputSymbol(state, '6'); return true;
    case SDLK_KP_7: HandleInputSymbol(state, '7'); return true;
    case SDLK_KP_8: HandleInputSymbol(state, '8'); return true;
    case SDLK_KP_9: HandleInputSymbol(state, '9'); return true;

    case SDLK_EQUALS:
        if (shift) HandleInputSymbol(state, '+');
        else       HandleInputSymbol(state, '=');
        return true;

    case SDLK_PLUS:     case SDLK_KP_PLUS:     HandleInputSymbol(state, '+'); return true;
    case SDLK_MINUS:    case SDLK_KP_MINUS:    HandleInputSymbol(state, '-'); return true;
    case SDLK_ASTERISK: case SDLK_KP_MULTIPLY: HandleInputSymbol(state, '*'); return true;
    case SDLK_SLASH:    case SDLK_KP_DIVIDE:   HandleInputSymbol(state, '/'); return true;
    case SDLK_PERIOD:   case SDLK_KP_PERIOD:   HandleInputSymbol(state, '.'); return true;

    case SDLK_KP_EQUALS: case SDLK_RETURN: case SDLK_KP_ENTER:
        HandleInputSymbol(state, '='); return true;

    case SDLK_LEFTPAREN:  HandleInputSymbol(state, '('); return true;
    case SDLK_RIGHTPAREN: HandleInputSymbol(state, ')'); return true;
    case SDLK_CARET:      HandleInputSymbol(state, '^'); return true;
    case SDLK_C: case SDLK_ESCAPE: HandleInputSymbol(state, 'C'); return true;

    default:
        return false;
    }

    return false;
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

    int tw = 0, th = 0;
    TTF_GetTextSize(state->text, &tw, &th);

    float tx = content_rect.x + content_rect.w - DISPLAY_PAD_X - tw;
    float ty = content_rect.y + (DISPLAY_HEIGHT - th) / 2.0f;

    tx = MIN(tx, content_rect.x + DISPLAY_PAD_X);
    ty = MIN(ty, content_rect.y + DISPLAY_PAD_Y);

    TTF_DrawRendererText(state->text, tx, ty);

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
            bg = Brighter(bg);

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
        state->hoveredButton = GetButtonAtPoint(state, local_mouse);
    } else if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event->button.button == SDL_BUTTON_LEFT) {
            int btn = GetButtonAtPoint(state, local_mouse);
            if (btn != NOHOVER) {
                HandleInputSymbol(state, state->buttons[btn].label);
                return true;
            }
        }
    } else if (event->type == SDL_EVENT_KEY_DOWN && event->key.down) {
        return HandleKeyEvent(state, event->key.key, event->key.mod);
    }

    return false;
}

bool CalcAppWantsPointerCursor(Window* win, SDL_FPoint local_mouse) {
    State* state = win->userData;
    return state->hoveredButton != NOHOVER;
}
