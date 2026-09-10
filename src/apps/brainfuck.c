#include <apps/snake.h>
#include <fonts.h>
#include <defs.h>

#include <stdlib.h>
#include <assert.h>

typedef struct {
    TTF_Text* hello;
} State;

void BfAppInit(Window* win) {
    State* state = malloc(sizeof(State));
    state->hello = TTF_CreateText(tengine, f.h1, "Brainfuck\nis currently TODO!", 0);
    assert(state->hello != NULL);
    TTF_SetTextColor(state->hello, 255, 255, 255, 255);

    win->userData = state;
}

void BfAppRender(Window* win, SDL_Renderer* renderer, SDL_FRect contentRect) {
    State* state = win->userData;
    assert(state != NULL);

    SDL_SetRenderDrawColor(renderer, 24, 24, 30, 255);
    SDL_RenderFillRect(renderer, &contentRect);

    TTF_DrawRendererText(
        state->hello,
        contentRect.x + 100,
        contentRect.y + 100);
}

bool BfAppHandleEvent(Window* win, const SDL_Event* event, SDL_FPoint localMouse) {
    (void)win, (void)event, (void)localMouse;
    return false;
}

void BfAppCleanup(Window* win) {
    State* state = win->userData;
    assert(state != NULL);

    TTF_DestroyText(state->hello);

    free(state);
    win->userData = NULL;
}

bool BfAppWantsPointerCursor(Window* win, SDL_FPoint localMouse) {
    (void)win, (void)localMouse;
    return false;
}

void BfAppChangeFocus(Window* win, bool focused) {
    (void)win, (void)focused;
}
