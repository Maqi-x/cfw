#include <btn.h>

#include <utils.h>
#include <assert.h>

#define PAD_X 28.0f
#define PAD_Y 12.0f

#define TEXT_COLOR 240, 240, 240, 255

#define BORDER_HOVER 186, 195, 207, 255
#define BORDER_IDLE  225, 230, 237, 255

#define CLICK_FILL_ALPHA 30
#define HOVER_FILL_ALPHA 25

#define CLICKOFF 2.1f

Button BtnCreate(TTF_Font* font, const char* label) {
    Button btn = { 0 };

    btn.text = TTF_CreateText(tengine, font, label, 0);
    assert(btn.text != NULL);
    TTF_SetTextColor(btn.text, TEXT_COLOR);

    BtnFitToText(&btn);
    return btn;
}

Button BtnCreateBg(
    TTF_Font* font, const char* label,
    Uint8 r, Uint8 g, Uint8 b, Uint8 a
) {
    Button btn = BtnCreate(font, label);

    btn.hasBg = true;
    btn.bg = (SDL_Color) { r, g, b, a };

    return btn;
}

void BtnDestroy(Button* btn) {
    if (btn == NULL) return;
    if (btn->text != NULL) {
        TTF_DestroyText(btn->text);
        btn->text = NULL;
    }
}

void BtnFitToText(Button* btn) {
    assert(btn != NULL && btn->text != NULL);

    int tw = 0, th = 0;
    TTF_GetTextSize(btn->text, &tw, &th);

    btn->rect.w = (float)tw + PAD_X;
    btn->rect.h = (float)th + PAD_Y;
}

static Uint8 ProcessColor(const Button* btn, Uint8 value) {
    if (btn->isClicked) return MoreBrighterE(value);
    if (btn->isHovered) return BrighterE(value);
    return value;
}

void BtnDrawOffset(SDL_Renderer* renderer, const Button* btn, SDL_FRect origin) {
    assert(btn != NULL && btn->text != NULL);

    SDL_FRect r = OffsetRect(origin, btn->rect);
    if (btn->isClicked) {
        r.h -= CLICKOFF;
        r.w -= CLICKOFF;
        r.x += CLICKOFF/2;
        r.y += CLICKOFF/2;
    }

    if (btn->hasBg) {
        SDL_SetRenderDrawColor(renderer,
            ProcessColor(btn, btn->bg.r),
            ProcessColor(btn, btn->bg.g),
            ProcessColor(btn, btn->bg.b),
            btn->bg.a);

        SDL_RenderFillRect(renderer, &r);
    } else {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255,
            btn->isClicked
                ? CLICK_FILL_ALPHA
                : btn->isHovered
                    ? HOVER_FILL_ALPHA
                    : 0);

        SDL_RenderFillRect(renderer, &r);
    }

    if (btn->isHovered) {
        SDL_SetRenderDrawColor(renderer, BORDER_HOVER);
    } else {
        SDL_SetRenderDrawColor(renderer, BORDER_IDLE);
    }
    SDL_RenderRect(renderer, &r);

    int tw = 0, th = 0;
    TTF_GetTextSize(btn->text, &tw, &th);

    float tx = r.x + (r.w - (float)tw) / 2.0f;
    float ty = r.y + (r.h - (float)th) / 2.0f;
    TTF_DrawRendererText(btn->text, tx, ty);
}

void BtnDraw(SDL_Renderer* renderer, const Button* btn) {
    BtnDrawOffset(renderer, btn, (SDL_FRect){ 0 });
}

bool BtnJustClicked(const Button* btn) {
    return btn->isClicked && !btn->wasClicked;
}

bool BtnContains(const Button* btn, SDL_FPoint point) {
    assert(btn != NULL);
    return SDL_PointInRectFloat(&point, &btn->rect);
}

void BtnHandleEvent(Button* btn, const SDL_Event* event, SDL_FPoint mouse) {
    btn->wasClicked = btn->isClicked;
    switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_UP:
        btn->isClicked = false;
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (event->button.button == SDL_BUTTON_LEFT) {
            btn->isClicked = BtnContains(btn, mouse);
        }
        fallthrough; // i think it's the first time i actually
                     // want switch fallthrough, wow.
    case SDL_EVENT_MOUSE_MOTION:
        btn->isHovered = BtnContains(btn, mouse);
        break;
    }
}
