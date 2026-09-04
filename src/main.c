#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <config.h>
#include <md4c.h>

#include <stdbool.h>
#include <stdlib.h>

#ifdef __EMSCRIPTEN__
    #define WEB true
    #include <emscripten/html5.h>
    #include <emscripten.h>
#else
    #define WEB false
#endif

#define E(F) \
    SDL_Log(#F " failed in " __FILE__ " at " STRINGIFY(__LINE__) ": %s\n", SDL_GetError())

// TODO: maybe globals are not a good idea, but i guess it's fine for now
int w, h;

SDL_Window*   window;
SDL_Renderer* renderer;

TTF_TextEngine* tengine;

struct {
    TTF_Font
        *normal,
        *bold,
        *italic,
        *h1,
        *h2;
} f;

FT* title;
FT* body;

typedef struct {
    TTF_Font** out;
    bool italic;

    uint size;
    TTF_FontStyleFlags style;
} FontSpec;

static const char* getpath(const FontSpec* spec) {
    return spec->italic ? "assets/nunito/NunitoSans-Italic.ttf" : "assets/nunito/NunitoSans.ttf";
}

static bool loadfont(TTF_Font** out, const FontSpec* spec) {
    *out = TTF_OpenFont(getpath(spec), spec->size);
    if (*out == NULL) {
        return false;
    }

    if (spec->style != 0) {
        TTF_SetFontStyle(*out, spec->style);
    }
    return true;
}

static void closefont(TTF_Font** font) {
    if (*font != NULL) {
        TTF_CloseFont(*font);
        *font = NULL;
    }
}

static void unload_fonts() {
    closefont(&f.h2);
    closefont(&f.h1);
    closefont(&f.italic);
    closefont(&f.bold);
    closefont(&f.normal);
}

static bool load_fonts() {
    const FontSpec fonts[] = {
        { .out = &f.normal, .italic = false, .size = 23, .style = 0              },
        { .out = &f.bold,   .italic = false, .size = 23, .style = TTF_STYLE_BOLD },
        { .out = &f.italic, .italic = true,  .size = 23, .style = 0              },
        { .out = &f.h1,     .italic = false, .size = 34, .style = TTF_STYLE_BOLD },
        { .out = &f.h2,     .italic = false, .size = 28, .style = TTF_STYLE_BOLD },
    };

    for (usize i = 0; i < sizeof(fonts) / sizeof(FontSpec); ++i) {
        if (!loadfont(fonts[i].out, &fonts[i])) {
            unload_fonts();
            return false;
        }
    }

    return true;
}

bool running = true;

static void update_layout() {
    SDL_GetWindowSize(window, &w, &h);
    ft_set_width(body, w - MAINTEXT_X * 2);
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

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 7, 7, 7, 255);
    SDL_RenderClear(renderer);

    uint title_w;
    ft_get_size(title, &title_w, NULL);

    ft_draw(title, ((float)w / 2.0f) - ((float)title_w / 2.0f), 10.0f);
    ft_draw(body, MAINTEXT_X, MAINTEXT_Y);

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
    w = 1024, h = 720;
#endif

    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE|SDL_WINDOW_HIGH_PIXEL_DENSITY;
    window = SDL_CreateWindow("C for Web", w, h, flags);
    if (window == NULL)
        { E(SDL_CreateWindow); goto e2; }

    renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == NULL)
        { E(SDL_CreateRenderer); goto e3; }

    if (!load_fonts())
        { E(TTF_OpenFont); goto e4; }

    tengine = TTF_CreateRendererTextEngine(renderer);
    if (tengine == NULL)
        { E(TTF_CreateRendererTextEngine); goto e5; }

    Style style = {
        .normal = f.normal, .bold = f.bold, .italic = f.italic,
        .h1 = f.h1, .h2 = f.h2,
        .text_color = text_color,
    };

    title = ft_create(tengine, &style);
    if (title == NULL) { E(ft_create); goto e6; }

    body = ft_create(tengine, &style);
    if (body == NULL) { E(ft_create); goto e7; }

    if (!ft_set_fragments(title, t_title, sizeof(t_title) / sizeof(t_title[0])))
        { E(ft_set_fragments); goto e8; }

    if (!ft_set_fragments(body, t_description, sizeof(t_description) / sizeof(t_description[0])))
        { E(ft_set_fragments); goto e8; }

    update_layout();

    return true;

e8: ft_destroy(body);
e7: ft_destroy(title);
e6: TTF_DestroyRendererTextEngine(tengine);
e5: unload_fonts();
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
