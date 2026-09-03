#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <stdbool.h>
#include <stdlib.h>

#ifdef __EMSCRIPTEN__
    #define WEB true
    #include <emscripten/html5.h>
    #include <emscripten.h>
#else
    #define WEB false
#endif

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

#define E(F) \
    SDL_Log(#F " failed in " __FILE__ " at " STRINGIFY(__LINE__) ": %s\n", SDL_GetError())

#define DESCRIPTION \
    "Webdev that is actually fun."

#define MAINTEXT_X 40
#define MAINTEXT_Y 100

// TODO: maybe globals are not a good idea, but i guess it's fine for now
int w, h;

SDL_Window*   window;
SDL_Renderer* renderer;

TTF_TextEngine* tengine;

struct {
    TTF_Font
        *header,
        *maintext;
} f;

struct {
    TTF_Text
        *c_for_web,
        *description;
} t;

bool running = true;

static void update_layout() {
    SDL_GetWindowSize(window, &w, &h);
    TTF_SetTextWrapWidth(t.description, w - MAINTEXT_X * 2);
}

static void mainloop() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            running = false;
        } else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            update_layout();
        }
    }

    SDL_SetRenderDrawColor(renderer, 7, 7, 7, 255);
    SDL_RenderClear(renderer);

    int cfw_header_w, cfw_header_h;
    TTF_GetTextSize(t.c_for_web, &cfw_header_w, &cfw_header_h);

    TTF_DrawRendererText(t.c_for_web, ((float)w / 2) - ((float)cfw_header_w / 2), 10);
    TTF_DrawRendererText(t.description, MAINTEXT_X, MAINTEXT_Y);

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
    if (!SDL_Init(SDL_INIT_VIDEO))
        { E(SDL_Init); goto e0; }
    if (!TTF_Init())
        { E(TTF_Init); goto e1; }

#if WEB
    emscripten_get_canvas_element_size("#canvas", &w, &h);
#else
    w = 800, h = 600;
#endif

    window = SDL_CreateWindow("C for Web", w, h, SDL_WINDOW_RESIZABLE);
    if (window == NULL)
        { E(SDL_CreateWindow); goto e2; }

    renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == NULL)
        { E(SDL_CreateRenderer); goto e3; }

    f.header = TTF_OpenFont("assets/nunito/NunitoSans.ttf", 34);
    if (f.header == NULL) { E(TTF_OpenFont); goto e4; }
    TTF_SetFontStyle(f.header, TTF_STYLE_BOLD|TTF_STYLE_UNDERLINE);

    f.maintext = TTF_OpenFont("assets/nunito/NunitoSans.ttf", 24);
    if (f.maintext == NULL) { E(TTF_OpenFont); goto e5; }

    tengine = TTF_CreateRendererTextEngine(renderer);
    if (tengine == NULL)
        { E(TTF_CreateRendererTextEngine); goto e6; }

    t.c_for_web = TTF_CreateText(tengine, f.header, "C For Web", 0);
    if (t.c_for_web == NULL) { E(TTF_CreateText); goto e7; }

    t.description = TTF_CreateText(tengine, f.maintext, DESCRIPTION, 0);
    if (t.description == NULL) { E(TTF_CreateText); goto e8; }

    update_layout();

    return true;

e8: TTF_DestroyText(t.c_for_web);
e7: TTF_DestroyRendererTextEngine(tengine);
e6: TTF_CloseFont(f.maintext);
e5: TTF_CloseFont(f.header);
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
