#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <stdbool.h>
#include <stdlib.h>

#ifdef __EMSCRIPTEN__
    #define WEB true
    #include <emscripten.h>
#else
    #define WEB false
#endif

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

#define E(F) \
    SDL_Log(#F " failed in " __FILE__ " at " STRINGIFY(__LINE__) ": %s\n", SDL_GetError())

// TODO: maybe globals are not a good idea, but i guess it's fine for now
SDL_Window* window;
SDL_Renderer* renderer;
TTF_Font* font;
SDL_Surface* text_surf;
SDL_Texture* text_tex;

bool running = true;

static void mainloop() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            running = false;
        }
    }

    SDL_SetRenderDrawColor(renderer, 34, 35, 42, 255);
    SDL_RenderClear(renderer);


    SDL_FRect dst_rect = {
        50.0f, 50.0f, (float)text_surf->w, (float)text_surf->h
    };
    SDL_RenderTexture(renderer, text_tex, NULL, &dst_rect);

    SDL_RenderPresent(renderer);

    if (!running) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

    #if WEB
        emscripten_cancel_main_loop();
    #else
        exit(0);
    #endif
    }
}

bool init() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        E(SDL_Init);
        goto e0;
    }
    if (!TTF_Init()) {
        E(TTF_Init);
        goto e1;
    }

    window = SDL_CreateWindow("Minimal SDL3 Window", 800, 600, 0);
    if (window == NULL) {
        E(SDL_CreateWindow);
        goto e2;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == NULL) {
        E(SDL_CreateRenderer);
        goto e3;
    }

    font = TTF_OpenFont("assets/nunito/NunitoSans.ttf", 24.0f);
    if (font == NULL) {
        E(TTF_OpenFont);
        goto e4;
    }

    const SDL_Color text_color = { 255, 255, 255 };
    text_surf = TTF_RenderText_Blended(font, "Hello mars!", 0, text_color);
    if (text_surf == NULL) {
        E(TTF_RenderText_Blended);
        goto e5;
    }

    text_tex = SDL_CreateTextureFromSurface(renderer, text_surf);
    if (text_tex == NULL) {
        E(SDL_CreateTextureFromSurface);
        goto e6;
    }

    return true;

e6: SDL_DestroySurface(text_surf);
e5: TTF_CloseFont(font);
e4: SDL_DestroyRenderer(renderer);
e3: SDL_DestroyWindow(window);
e2: TTF_Quit();
e1: SDL_Quit();
e0: return false;
}

int main(int argc, char *argv[]) {
    if (!init())
        return 1;

#if WEB
    // see https://wiki.libsdl.org/SDL3/README-emscripten
    emscripten_set_main_loop(mainloop, 0, 1);
#else
    while (true) { mainloop(); }
#endif

    return 0;
}
