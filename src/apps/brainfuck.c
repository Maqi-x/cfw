#include <apps/brainfuck.h>

#include <editbox.h>
#include <fonts.h>
#include <defs.h>
#include <utils.h>
#include <btn.h>
#include <bf.h>

#include <stdlib.h>
#include <assert.h>
#include <tgmath.h>

#define BG_COLOR   18,  18,  20,  255
#define OUT_BG     14,  14,  14,  255

#define TEXT_COLOR  240, 240, 240, 255
#define ERR_COLOR   191, 42,  42,  255
#define EMPTY_COLOR 3,   140, 252, 255

#define PAD     16.0f
#define BTN_GAP 12.0f

#define NUM_BTNS 2
#define BTN_NONE NUM_BTNS

#define AVAILABLE_WIDTH (BF_WIDTH - 3.0f * PAD)
#define INPUT_WIDTH     (AVAILABLE_WIDTH * 0.54f)
#define OUTPUT_WIDTH    (AVAILABLE_WIDTH * 0.46f)
#define OUTPUT_X        (INPUT_WIDTH + 2 * PAD)

#define STEPS_PER_FRAME 5000

typedef struct {
    EditBox* edit;
    SDL_FRect editRect;

    SDL_FRect outputRect;
    TTF_Text* outputText;

    Button runBtn;
    Button clearBtn;

    Instruction prog[PROGRAM_SIZE];
    BfContext ctx;
} State;

static void LayoutButtons(State* state) {
    BtnFitToText(&state->runBtn);
    BtnFitToText(&state->clearBtn);

    SDL_FRect* saveRect = &state->runBtn.rect;
    SDL_FRect* loadRect = &state->clearBtn.rect;

    float totalWidth = saveRect->w + BTN_GAP + loadRect->w;
    float sx = BF_WIDTH - PAD - totalWidth;
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
    TTF_Text* t = state->edit->text;
    if (t->text == NULL || t->text[0] == '\0') {
        state->ctx.running = false;
        TTF_SetTextString(state->outputText, "No input provided!", 0);
        TTF_SetTextColor(state->outputText, EMPTY_COLOR);
        return;
    }

    if (CompileBrainfuck(state->edit->text->text, state->prog)) {
        InitBrainfuck(&state->ctx);
        TTF_SetTextString(state->outputText, "", 0);
        TTF_SetTextColor(state->outputText, TEXT_COLOR);
    } else {
        state->ctx.running = false;
        TTF_SetTextString(state->outputText, "Compilation Error!\nUnmatched delimiters", 0);
        TTF_SetTextColor(state->outputText, ERR_COLOR);
    }
}

#define PRETTY_MUCH_EMPTY_STRING ""
static void Clear(State* state) {
    state->ctx.running = false;
    TTF_SetTextString(state->edit->text, PRETTY_MUCH_EMPTY_STRING, 0);
    state->ctx.outLen = 0;
    TTF_SetTextString(state->outputText, "", 0);
}

void BfAppInit(Window* win) {
    State* state = calloc(1, sizeof(State));
    assert(state != NULL);

    state->runBtn   = BtnCreateBg(f.bold, "Run",   BG_COLOR);
    state->clearBtn = BtnCreateBg(f.bold, "Clear", BG_COLOR);
    LayoutButtons(state);

    state->editRect = (SDL_FRect) {
        .x = PAD,
        .y = PAD,
        .w = INPUT_WIDTH,
        .h = BF_HEIGHT - 2.0f * PAD,
    };

    state->outputRect = (SDL_FRect) {
        .x = OUTPUT_X,
        .y = PAD,
        .w = OUTPUT_WIDTH,
        .h = state->runBtn.rect.y - 2.0f * PAD,
    };

    state->outputText = TTF_CreateText(tengine, f.code, "", 0);
    assert(state->outputText != NULL);

    TTF_SetTextColor(state->outputText, 150, 150, 150, 255);
    TTF_SetTextWrapWidth(state->outputText, state->outputRect.w);

    SDL_FRect editAbs = OffsetRect(GetWindowContentRect(win), state->editRect);

    state->edit = EditBox_Create(window, renderer, tengine, f.code, &editAbs);
    assert(state->edit != NULL);

    // for whatever reason we actually need to do this manually...
    TTF_SetTextWrapWidth(state->edit->text, floor(state->editRect.w));

    TTF_SetTextString(state->edit->text, "[ Put brainfuck code here... ]", 0);
    TTF_SetTextColor(state->edit->text, TEXT_COLOR);

    win->userData = state;
}

void BfAppCleanup(Window* win) {
    State* state = win->userData;
    assert(state != NULL);

    state->ctx.running = false;

    TTF_DestroyText(state->outputText);
    EditBox_Destroy(state->edit);
    BtnDestroy(&state->runBtn);
    BtnDestroy(&state->clearBtn);

    free(state);
    win->userData = NULL;
}

void BfAppRender(Window* win, SDL_Renderer* renderer, SDL_FRect contentRect) {
    State* state = win->userData;
    assert(state != NULL);

    if (state->ctx.running) {
        StepBrainfuck(state->prog, &state->ctx, STEPS_PER_FRAME);
        TTF_SetTextString(state->outputText, state->ctx.outBuf, state->ctx.outLen);
    }

    SDL_SetRenderDrawColor(renderer, BG_COLOR);
    SDL_RenderFillRect(renderer, &contentRect);

    SyncEditBoxLocation(win, state);
    EditBox_Draw(state->edit);

    SDL_FRect absOutputRect = OffsetRect(contentRect, state->outputRect);
    SDL_SetRenderDrawColor(renderer, OUT_BG);
    SDL_RenderFillRect(renderer, &absOutputRect);

    TTF_DrawRendererText(state->outputText, absOutputRect.x + 8.0f, absOutputRect.y + 8.0f);

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

CursorKind BfAppGetCursorKind(Window* win, SDL_FPoint localMouse) {
    State* state = win->userData;
    assert(state != NULL);

    if (state->runBtn.isHovered || state->clearBtn.isHovered)
        return CPOINTER;

    if (PointInEditRect(state, localMouse))
        return CTEXT;

    return CARROW;
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

