#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <config.h>
#include <fonts.h>
#include <windows.h>
#include <apps.h>

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

SDL_Texture* githubIcon = NULL;
bool githubHovered = false;

TTF_Text* demosText = NULL;
bool demosHovered = false;

// It's ugly, but I have no idea how to do this without macros
// If I define this as a function
//   static SDL_FRect githubRect() { return ...; }
// Then I can't just do this:
//   &githubRect();
// It only works with compound literals, I guess that's just
// how this 54 year old language works, and the preprocessor
// also exists for a reason. Let's use it then!
#define GITHUB_RECT ((SDL_FRect){ (float)w - 50, 10, 40, 40 })

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

static SDL_FRect GetGamesDemosButtonRect() {
    int tw = 0, th = 0;
    TTF_GetTextSize(demosText, &tw, &th);

    float bw = tw + 28.0f;
    float bh = th + 12.0f;

    return (SDL_FRect) {
        .w = bw, .h = bh,
        .x = w - MAINTEXT_X - bw,
        .y = CONTENT_Y - scroll.curr,
    };
}

static void DrawGamesDemosButton() {
    if (demosText == NULL) return;
    SDL_FRect btn = GetGamesDemosButtonRect();

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, demosHovered ? 25 : 0);
    SDL_RenderFillRect(renderer, &btn);

    if (demosHovered) {
        SDL_SetRenderDrawColor(renderer, 186, 195, 207, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 225, 230, 237, 255);
    }
    SDL_RenderRect(renderer, &btn);

    int tw = 0, th = 0;
    TTF_GetTextSize(demosText, &tw, &th);

    float tx = btn.x + (btn.w - tw) / 2.0f;
    float ty = btn.y + (btn.h - th) / 2.0f;
    TTF_DrawRendererText(demosText, tx, ty);
}

static void DrawTopbar() {
    uint title_w;
    FTGetSize(title, &title_w, NULL);

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_FRect topbar = { 0, 0, (float)w, (float)MAINTEXT_Y };
    SDL_RenderFillRect(renderer, &topbar);

    FTDraw(title, ((float)w / 2.0f) - ((float)title_w / 2.0f), 10.0f);

    SDL_SetTextureColorMod(githubIcon,
        githubHovered ? 170 : 255,
        githubHovered ? 170 : 255,
        githubHovered ? 170 : 255);
    SDL_RenderTexture(renderer, githubIcon, NULL, &GITHUB_RECT);
}

static void DrawBodytext() {
    SDL_Rect clip = { 0, MAINTEXT_Y, w, h - MAINTEXT_Y };
    SDL_SetRenderClipRect(renderer, &clip);

    FTDraw(body, MAINTEXT_X, CONTENT_Y - scroll.curr);
    DrawGamesDemosButton();

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

static void HandleMouseEvents(SDL_Event* event, SDL_FPoint mouse) {
    bool onGithubBtn = SDL_PointInRectFloat(&mouse, &GITHUB_RECT) && mouse.y <= (float)MAINTEXT_Y;
    githubHovered = onGithubBtn;

    SDL_FRect btn = GetGamesDemosButtonRect();
    bool onDemosBtn = SDL_PointInRectFloat(&mouse, &btn) && mouse.y > (float)MAINTEXT_Y && !IsMouseOverWindow(mouse);
    demosHovered = onDemosBtn;

    float x = mouse.x - MAINTEXT_X;
    float y = mouse.y - CONTENT_Y + scroll.curr;

    const char* url = (!IsMouseOverWindow(mouse) && mouse.y > (float)MAINTEXT_Y) ? FTGetLinkAt(body, x, y) : NULL;

    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (onGithubBtn)
            return (void)SDL_OpenURL(GITHUB_LINK);

        if (onDemosBtn) {
            WindowCreate(APP_DISCOVER);
            return;
        }

        if (url == NULL) return;
        (void)SDL_OpenURL(url);
    } else {
        if (WindowWantsPointerCursor(mouse) || onGithubBtn || onDemosBtn || url != NULL) {
            SDL_SetCursor(cursor.pointer);
        } else {
            SDL_SetCursor(cursor.arrow);
        }
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
        } else {
            SDL_FPoint mouse;
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
                mouse = (SDL_FPoint) { event.button.x, event.button.y };
            } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
                mouse = (SDL_FPoint) { event.motion.x, event.motion.y };
            } else {
                float mx, my;
                SDL_GetMouseState(&mx, &my);
                mouse = (SDL_FPoint) { mx, my };
            }

            if (HandleWindowEvent(&event, mouse)) {
                if (event.type == SDL_EVENT_MOUSE_MOTION) {
                    HandleMouseEvents(&event, mouse);
                }
                continue;
            }

            if (event.type == SDL_EVENT_MOUSE_WHEEL) {
                scroll.target -= event.wheel.y * SCROLL_WHEEL_STEP;
                EnsureScrollInBounds();
            } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                HandleMouseEvents(&event, mouse);
            } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
                HandleMouseEvents(&event, mouse);
            }
        }
    }

    UpdateScroll(dt);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 7, 7, 7, 255);
    SDL_RenderClear(renderer);

    DrawBodytext();
    DrawTopbar();
    RenderWindows(renderer);

    SDL_RenderPresent(renderer);

    if (!running) {
        if (demosText != NULL) {
            TTF_DestroyText(demosText);
        }
        DeinitWindows();
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

static SDL_Texture* LoadTexture(const char* file) {
    SDL_Surface* surf = SDL_LoadPNG(file);
    if (surf == NULL) {
        SDL_Log("Failed to load %s: %s", file, SDL_GetError());
        return NULL;
    }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_DestroySurface(surf);

    if (tex == NULL) {
        SDL_Log("Failed to create texture from %s: %s", file, SDL_GetError());
        return NULL;
    }

    return tex;
}

bool init() {
    if (!SDL_Init(SDL_INIT_VIDEO))
        { E(SDL_Init); goto e0; }
    if (!TTF_Init())
        { E(TTF_Init); goto e1; }

#if WEB
    emscripten_get_canvas_element_size("#canvas", &w, &h);
#else
    // does anyone even use 900p in big 2026?
    w = 1600, h = 900;
#endif

    lastTicks = SDL_GetTicksNS();

    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE;
    window = SDL_CreateWindow("C for Web", w, h, flags);
    if (window == NULL)
        { E(SDL_CreateWindow); goto e2; }

    renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == NULL)
        { E(SDL_CreateRenderer); goto e3; }

    if (!LoadFonts())
        { E(TTF_OpenFont); goto e4; }

    tengine = TTF_CreateRendererTextEngine(renderer);
    if (tengine == NULL) E(TTF_CreateRendererTextEngine);

    InitWindows();

    demosText = TTF_CreateText(tengine, f.bold, "Games and demos", 15);
    if (demosText == NULL) E(TTF_CreateText);
    TTF_SetTextColor(demosText, 240, 240, 240, 255);

    Style style = {
        .normal = f.normal, .bold = f.bold, .italic = f.italic,
        .text_color = text_color, .link_color = link_color,
        .h1 = f.h1, .h2 = f.h2,
    };

    title = CreateFT(tengine, &style);
    if (title == NULL) E(CreateFT);

    body = CreateFT(tengine, &style);
    if (body == NULL) E(CreateFT);

    if (!FTSetFragments(title, tTitle, sizeof(tTitle) / sizeof(tTitle[0])))
        E(FTSetFragments);

    if (!FTSetFragments(body, tDescription, sizeof(tDescription) / sizeof(tDescription[0])))
        E(FTSetFragments);

    cursor.arrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
    if (cursor.arrow == NULL) E(SDL_CreateSystemCursor);

    cursor.pointer = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
    if (cursor.pointer == NULL) E(SDL_CreateSystemCursor);

    githubIcon = LoadTexture("assets/github.png");
    if (githubIcon == NULL) E(SDL_LoadPNG);

    UpdateLayout();
    return true;

// other things are not important enough for me to care
// about freeing their resources
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
