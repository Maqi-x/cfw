#include <apps/snake.h>
#include <fonts.h>
#include <defs.h>

#include <stdlib.h>
#include <assert.h>

typedef struct {
    TTF_Text* hello;
} State;

void SnakeAppInit(Window* win) {
    State* state = malloc(sizeof(State));
    state->hello = TTF_CreateText(tengine, f.h1, "Snake Game\nis TODO!", 0);
    assert(state->hello != NULL);
    TTF_SetTextColor(state->hello, 255, 255, 255, 255);

    win->userData = state;
}

void SnakeAppRender(Window* win, SDL_Renderer* renderer, SDL_FRect contectRect) {
    State* state = win->userData;
    assert(state != NULL);

    SDL_SetRenderDrawColor(renderer, 24, 24, 30, 255);
    SDL_RenderFillRect(renderer, &contectRect);

    TTF_DrawRendererText(
        state->hello,
        contectRect.x + 100,
        contectRect.y + 100);
}

bool SnakeAppHandleEvent(Window* win, const SDL_Event* event, SDL_FPoint localMouse) {
    (void)win, (void)event, (void)localMouse;
    return false;
}

void SnakeAppCleanup(Window* win) {
    State* state = win->userData;
    assert(state != NULL);

    TTF_DestroyText(state->hello);

    free(state);
    win->userData = NULL;
}
