#include <apps/soundboard.h>
#include <fonts.h>
#include <defs.h>

#include <stdlib.h>
#include <assert.h>

typedef struct {
    TTF_Text* hello;
} State;

void SoundboardAppInit(Window* win) {
    State* state = malloc(sizeof(State));
    state->hello = TTF_CreateText(tengine, f.h1, "Soundboard App\n is currently TODO!", 0);
    assert(state->hello != NULL);
    TTF_SetTextColor(state->hello, 255, 255, 255, 255);

    win->userData = state;
}

void SoundboardAppRender(Window* win, SDL_Renderer* renderer, SDL_FRect content_rect) {
    State* state = win->userData;
    assert(state != NULL);

    SDL_SetRenderDrawColor(renderer, 24, 24, 30, 255);
    SDL_RenderFillRect(renderer, &content_rect);

    TTF_DrawRendererText(
        state->hello,
        content_rect.x + 10,
        content_rect.y + 50);
}

bool SoundboardAppHandleEvent(Window* win, const SDL_Event* event, SDL_FPoint local_mouse) {
    (void)win, (void)event, (void)local_mouse;
    return false;
}

void SoundboardAppCleanup(Window* win) {
    State* state = win->userData;
    assert(state != NULL);

    TTF_DestroyText(state->hello);

    free(state);
    win->userData = NULL;
}
