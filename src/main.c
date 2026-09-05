#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <config.h>
#include <fonts.h>

#include <stdbool.h>
#include <stdlib.h>
#include <tgmath.h>

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
uint64_t lastTicks;

struct {
    float curr;
    float target;
} scroll;

SDL_Window*   window;
SDL_Renderer* renderer;

struct {
    SDL_Cursor *arrow,
               *pointer;
} cursor;

TTF_TextEngine* tengine;
FT *title, *body;

bool running = true;

static void clamp(float* value, float max) {
    if (*value > max) *value = max;
    if (*value < 0)   *value = 0;
}

static void EnsureScrollInBounds() {
    uint bw, bh;
    FTGetSize(body, &bw, &bh);

    float maxScroll = (float)bh - (h - CONTENT_Y);
    if (maxScroll < 0) maxScroll = 0;

    clamp(&scroll.target, maxScroll);
    clamp(&scroll.curr,   maxScroll);
}

static void UpdateLayout() {
    SDL_GetWindowSize(window, &w, &h);
    FTSetWidth(body, w - MAINTEXT_X * 2);
    EnsureScrollInBounds();
}

static void UpdateScroll(float dt) {
    float factor = 1.0f - exp(-SCROLL_SPEED * dt);
    scroll.curr += (scroll.target - scroll.curr) * factor;
}

static void DrawTopbar() {
    uint title_w;
    FTGetSize(title, &title_w, NULL);

    FTDraw(title, ((float)w / 2.0f) - ((float)title_w / 2.0f), 10.0f);

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_FRect topbar = { 0, 0, (float)w, (float)MAINTEXT_Y };
    SDL_RenderFillRect(renderer, &topbar);

    FTDraw(title, ((float)w / 2.0f) - ((float)title_w / 2.0f), 10.0f);
}

static void DrawBodytext() {
    SDL_Rect clip = { 0, MAINTEXT_Y, w, h - MAINTEXT_Y };
    SDL_SetRenderClipRect(renderer, &clip);

    FTDraw(body, MAINTEXT_X, CONTENT_Y - scroll.curr);
    SDL_SetRenderClipRect(renderer, NULL);
}

static void MainLoop();

// i don't know how this works but for whatever reason it's needed
// without this scroll feels laggy etc. i don't know. ai wrote this.
#if WEB
static void RestartWebLoop(void *arg) {
    (void)arg;
    lastTicks = SDL_GetTicksNS();
    emscripten_set_main_loop(MainLoop, 0, 0);
}
static void CheckAndKickCompositor(float dt) {
    static bool compositor_kicked = false;
    static int frame_counter = 0;
    if (compositor_kicked) return;

    frame_counter++;
    if (frame_counter == 10) {
        if (dt > 0.012f) {
            compositor_kicked = true;
            emscripten_cancel_main_loop();
            emscripten_async_call(RestartWebLoop, NULL, 100);
        }
    }
}
#endif

static void HandleMouseEvents(SDL_Event* event, float mx, float my) {
    float x = mx - MAINTEXT_X;
    float y = my - CONTENT_Y + scroll.curr;

    const char* url = FTGetLinkAt(body, x, y);
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (url == NULL) return;
        (void)SDL_OpenURL(url);
    } else {
        SDL_SetCursor(url != NULL ? cursor.pointer : cursor.arrow);
    }
}

static void MainLoop() {
    uint64_t now = SDL_GetTicksNS();
    float dt = (float)((now - lastTicks) / 1000000000.0);
    lastTicks = now;
    if (dt > 0.1f) dt = 0.1f;

#if WEB
    CheckAndKickCompositor(dt);
#endif

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            running = false;
        } else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            UpdateLayout();
        } else if (event.type == SDL_EVENT_MOUSE_WHEEL) {
            scroll.target -= event.wheel.y * SCROLL_WHEEL_STEP;
            EnsureScrollInBounds();
        } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            HandleMouseEvents(&event, event.button.x, event.button.y);
        } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
            HandleMouseEvents(&event, event.motion.x, event.motion.y);
        }
    }

    UpdateScroll(dt);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 7, 7, 7, 255);
    SDL_RenderClear(renderer);

    DrawTopbar();
    DrawBodytext();

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

    lastTicks = SDL_GetTicksNS();

    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE|SDL_WINDOW_HIGH_PIXEL_DENSITY;
    window = SDL_CreateWindow("C for Web", w, h, flags);
    if (window == NULL)
        { E(SDL_CreateWindow); goto e2; }

    renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == NULL)
        { E(SDL_CreateRenderer); goto e3; }

    if (!LoadFonts())
        { E(TTF_OpenFont); goto e4; }

    tengine = TTF_CreateRendererTextEngine(renderer);
    if (tengine == NULL)
        { E(TTF_CreateRendererTextEngine); goto e5; }

    Style style = {
        .normal = f.normal, .bold = f.bold, .italic = f.italic,
        .text_color = text_color, .link_color = link_color,
        .h1 = f.h1, .h2 = f.h2,
    };

    title = CreateFT(tengine, &style);
    if (title == NULL) { E(CreateFT); goto e6; }

    body = CreateFT(tengine, &style);
    if (body == NULL) { E(CreateFT); goto e7; }

    if (!FTSetFragments(title, tTitle, sizeof(tTitle) / sizeof(tTitle[0])))
        { E(FTSetFragments); goto e8; }

    if (!FTSetFragments(body, tDescription, sizeof(tDescription) / sizeof(tDescription[0])))
        { E(FTSetFragments); goto e8; }

    cursor.arrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
    if (cursor.arrow == NULL)
        { E(SDL_CreateSystemCursor); goto e8; }

    cursor.pointer = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
    if (cursor.pointer == NULL)
        { E(SDL_CreateSystemCursor); goto e9; }

    UpdateLayout();
    return true;

e9: SDL_DestroyCursor(cursor.arrow);
e8: DestroyFT(body);
e7: DestroyFT(title);
e6: TTF_DestroyRendererTextEngine(tengine);
e5: UnloadFonts();
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
    emscripten_set_main_loop(MainLoop, 0, 1);
#else
    while (true) { MainLoop(); }
#endif

    return 0;
}
