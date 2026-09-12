#include <apps/brainfuck.h>
#include <fonts.h>
#include <defs.h>

#include <stdlib.h>
#include <assert.h>

#include <editbox.h>
#include <fonts.h>
#include <defs.h>
#include <utils.h>
#include <btn.h>

#include <web.h>

#include <stdlib.h>
#include <assert.h>

#define BG_COLOR   22,  22,  30,  255
#define EDIT_BG    14,  14,  20,  255
#define TEXT_COLOR 240, 240, 240, 255

#define PAD     16.0f
#define BTN_GAP 12.0f

#define NUM_BTNS 2
#define BTN_NONE NUM_BTNS

typedef struct {
    EditBox* edit;
    SDL_FRect editRect;

    Button runBtn;
    Button clearBtn;
} State;

static void LayoutButtons(State* state) {
    BtnFitToText(&state->runBtn);
    BtnFitToText(&state->clearBtn);

    SDL_FRect* saveRect = &state->runBtn.rect;
    SDL_FRect* loadRect = &state->clearBtn.rect;

    float totalWidth = saveRect->w + BTN_GAP + loadRect->w;
    float sx = (BF_WIDTH - totalWidth) / 2.0f;
    float sy = BF_HEIGHT - PAD - saveRect->h;

    saveRect->x = sx;
    saveRect->y = sy;

    loadRect->x = sx + saveRect->w + BTN_GAP;
    loadRect->y = sy;
}

static void SyncEditBoxLocation(Window* win, State* state) {
    state->edit->rect = OffsetRect(GetWindowContentRect(win), state->editRect);
}

static bool PointInEditRect(const State* state, SDL_FPoint localMouse) {
    return SDL_PointInRectFloat(&localMouse, &state->editRect);
}

static void Run(State* state) {
    unreachable();
}

#define PRETTY_MUCH_EMPTY_STRING ""
static void Clear(State* state) {
    TTF_SetTextString(state->edit->text, PRETTY_MUCH_EMPTY_STRING, 0);
}

void BfAppInit(Window* win) {
    State* state = malloc(sizeof(State));
    assert(state != NULL);

    state->runBtn = BtnCreateBg(f.bold, "Run", BG_COLOR);
    state->clearBtn = BtnCreateBg(f.bold, "Clear", BG_COLOR);
    LayoutButtons(state);

    float editBottom = state->runBtn.rect.y - PAD;
    state->editRect = (SDL_FRect) {
        .x = PAD,
        .y = PAD,
        .w = BF_WIDTH - 2 * PAD,
        .h = editBottom - PAD * 10,
    };

    SDL_FRect editAbs = GetWindowContentRect(win);

    state->edit = EditBox_Create(window, renderer, tengine, f.normal, &editAbs);
    assert(state->edit != NULL);

    TTF_SetTextColor(state->edit->text, TEXT_COLOR);

    win->userData = state;
}

void BfAppCleanup(Window* win) {
    State* state = win->userData;
    assert(state != NULL);

    EditBox_Destroy(state->edit);
    BtnDestroy(&state->runBtn);
    BtnDestroy(&state->clearBtn);

    free(state);
    win->userData = NULL;
}

void BfAppRender(Window* win, SDL_Renderer* renderer, SDL_FRect contentRect) {
    State* state = win->userData;
    assert(state != NULL);

    SDL_SetRenderDrawColor(renderer, BG_COLOR);
    SDL_RenderFillRect(renderer, &contentRect);

    SyncEditBoxLocation(win, state);
    EditBox_Draw(state->edit);

    BtnDrawOffset(renderer, &state->runBtn, contentRect);
    BtnDrawOffset(renderer, &state->clearBtn, contentRect);
}

bool BfAppHandleEvent(Window* win, const SDL_Event* event, SDL_FPoint localMouse) {
    State* state = win->userData;
    assert(state != NULL);

    BtnHandleEvent(&state->runBtn, event, localMouse);
    BtnHandleEvent(&state->clearBtn, event, localMouse);

    if (BtnJustClicked(&state->runBtn))   Run(state);
    if (BtnJustClicked(&state->clearBtn)) Clear(state);

    return EditBox_HandleEvent(state->edit, event);
}

bool BfAppWantsPointerCursor(Window* win, SDL_FPoint localMouse) {
    State* state = win->userData;
    assert(state != NULL);

    return state->runBtn.isHovered || state->clearBtn.isHovered;
}

bool BfAppWantsTextCursor(Window* win, SDL_FPoint localMouse) {
    State* state = win->userData;
    assert(state != NULL);

    return (!state->runBtn.isHovered && !state->clearBtn.isHovered)
        && PointInEditRect(state, localMouse);
}

void BfAppChangeFocus(Window* win, bool focused) {
    State* state = win->userData;
    assert(state != NULL);

    if (!focused) {
        EditBox_SetFocus(state->edit, false);
        state->runBtn.isHovered = false;
        state->clearBtn.isHovered = false;
    }
}

