#include <apps/lstorage.h>

#include <editbox.h>
#include <fonts.h>
#include <defs.h>
#include <utils.h>
#include <btn.h>

#include <web.h>

#include <stdlib.h>
#include <assert.h>

#define BG_COLOR   24, 24, 30, 255
#define EDIT_BG    14, 14, 20, 255
#define TEXT_COLOR 240, 240, 240, 255

#define PAD     16.0f
#define BTN_GAP 12.0f

#define NUM_BTNS 2
#define BTN_NONE NUM_BTNS

typedef enum {
    BTN_SAVE = 0,
    BTN_LOAD = 1,
} BtnIndex;

typedef struct {
    EditBox* edit;
    SDL_FRect editRect;

    Button saveBtn;
    Button loadBtn;
} State;

static void LayoutButtons(State* state) {
    BtnFitToText(&state->saveBtn);
    BtnFitToText(&state->loadBtn);

    SDL_FRect* saveRect = &state->saveBtn.rect;
    SDL_FRect* loadRect = &state->loadBtn.rect;

    float totalWidth = saveRect->w + BTN_GAP + loadRect->w;
    float sx = (LSTORAGE_WIDTH - totalWidth) / 2.0f;
    float sy = LSTORAGE_HEIGHT - PAD - saveRect->h;

    saveRect->x = sx;
    saveRect->y = sy;

    loadRect->x = sx + saveRect->w + BTN_GAP;
    loadRect->y = sy;
}

// this is needed to keep the edit box position synchronized with
// current window location so it actually moves with the window
static void SyncEditBoxLocation(Window* win, State* state) {
    state->edit->rect = OffsetRect(GetWindowContentRect(win), state->editRect);
}

#define KEY "lstorage-app-content"

static void Save(State* state) {
#if WEB
    const char* content = state->edit->text->text;
    LocalStorageSet(KEY, content, 0);
    // TODO: maybe show some message box or any form of
    //       feedback that the action actually succeeded
#else
    // TODO: show some error message or something.
    //       or... maybe we should implement saving to a file,
    //       but what's the point to be honest, this project
    //       is designed to be a website
#endif
}

static bool Load(State* state) {
#if WEB
    char* text;
    usize len = LocalStorageGet(KEY, &text);

    // I'm not sure if that's how i'm supposed to do this but I don't
    // see any public api for that and it seems to work just fine
    TTF_SetTextString(state->edit->text, text, len);

    return len != 0;
#else
    // TODO: also show error message
    return false;
#endif
}

static BtnIndex ButtonAtPoint(const State* state, SDL_FPoint localMouse) {
    if (BtnContains(&state->saveBtn, localMouse)) return BTN_SAVE;
    if (BtnContains(&state->loadBtn, localMouse)) return BTN_LOAD;
    return BTN_NONE;
}

static bool PointInEditRect(const State* state, SDL_FPoint localMouse) {
    return SDL_PointInRectFloat(&localMouse, &state->editRect);
}

void LStorageAppInit(Window* win) {
    State* state = malloc(sizeof(State));
    assert(state != NULL);

    state->saveBtn = BtnCreateBg(f.bold, "Save", BG_COLOR);
    state->loadBtn = BtnCreateBg(f.bold, "Load", BG_COLOR);
    LayoutButtons(state);

    float editBottom = state->saveBtn.rect.y - PAD;
    state->editRect = (SDL_FRect) {
        .x = PAD,
        .y = PAD,
        .w = LSTORAGE_WIDTH - 2 * PAD,
        .h = editBottom - PAD * 10,
    };

    SDL_FRect editAbs = GetWindowContentRect(win);

    state->edit = EditBox_Create(window, renderer, tengine, f.normal, &editAbs);
    assert(state->edit != NULL);

    TTF_SetTextColor(state->edit->text, TEXT_COLOR);
    if (!Load(state)) {
        TTF_SetTextString(state->edit->text, "Type something...", 0);
    }

    win->userData = state;
}

void LStorageAppCleanup(Window* win) {
    State* state = win->userData;
    assert(state != NULL);

    EditBox_Destroy(state->edit);
    BtnDestroy(&state->saveBtn);
    BtnDestroy(&state->loadBtn);

    free(state);
    win->userData = NULL;
}

void LStorageAppRender(Window* win, SDL_Renderer* renderer, SDL_FRect contentRect) {
    State* state = win->userData;
    assert(state != NULL);

    SDL_SetRenderDrawColor(renderer, BG_COLOR);
    SDL_RenderFillRect(renderer, &contentRect);

    SyncEditBoxLocation(win, state);
    EditBox_Draw(state->edit);

    BtnDrawOffset(renderer, &state->saveBtn, contentRect);
    BtnDrawOffset(renderer, &state->loadBtn, contentRect);
}

bool LStorageAppHandleEvent(Window* win, const SDL_Event* event, SDL_FPoint localMouse) {
    State* state = win->userData;
    assert(state != NULL);

    if (event->type == SDL_EVENT_MOUSE_MOTION) {
        state->saveBtn.hovered = BtnContains(&state->saveBtn, localMouse);
        state->loadBtn.hovered = BtnContains(&state->loadBtn, localMouse);
    }

    bool click = event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button == SDL_BUTTON_LEFT;
    if (click) {
        BtnIndex hit = ButtonAtPoint(state, localMouse);
        if (hit != BTN_NONE) {
            EditBox_SetFocus(state->edit, false);

            switch (hit) {
            case BTN_SAVE: Save(state); return true;
            case BTN_LOAD: Load(state); return true;
            default:       unreachable();
            }
        }
    }

    return EditBox_HandleEvent(state->edit, event);
}

bool LStorageAppWantsPointerCursor(Window* win, SDL_FPoint localMouse) {
    State* state = win->userData;
    assert(state != NULL);
    return ButtonAtPoint(state, localMouse) != BTN_NONE;
}

bool LStorageAppWantsTextCursor(Window* win, SDL_FPoint localMouse) {
    State* state = win->userData;
    assert(state != NULL);

    return ButtonAtPoint(state, localMouse) == BTN_NONE
        && PointInEditRect(state, localMouse);
}

void LStorageAppChangeFocus(Window* win, bool focused) {
    State* state = win->userData;
    assert(state != NULL);

    if (!focused) {
        EditBox_SetFocus(state->edit, false);
        state->saveBtn.hovered = false;
        state->loadBtn.hovered = false;
    }
}
