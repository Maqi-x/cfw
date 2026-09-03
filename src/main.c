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

#define TEXT_X 100
#define TEXT_Y 100

// TODO: maybe globals are not a good idea, but i guess it's fine for now
SDL_Window*   window;
SDL_Renderer* renderer;

TTF_TextEngine* tengine;
TTF_Font*       font;
TTF_Text*       text;

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

    TTF_DrawRendererText(text, TEXT_X, TEXT_Y);

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

    window = SDL_CreateWindow("C for Web", 800, 600, 0);
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

    tengine = TTF_CreateRendererTextEngine(renderer);
    if (tengine == NULL) {
        E(TTF_CreateRendererTextEngine);
        goto e5;
    }

    char str[9999];
    char hello[] = "Hello, ";
    for (int i = 0; i < 1000; ++i) {
        strcpy(str + i * (sizeof(hello) - 1), hello);
    }
    str[(sizeof(hello) - 1) * 1000] = '\0';

    text = TTF_CreateText(tengine, font, str, 0);
    if (text == NULL) {
        E(TTF_CreateText);
        goto e6;
    }

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    TTF_SetTextWrapWidth(text, w - TEXT_X * 2);

    return true;

e6: TTF_DestroyRendererTextEngine(tengine);
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
