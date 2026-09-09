#include <apps/soundboard.h>

#include <fonts.h>
#include <defs.h>
#include <utils.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define LEFT_BG    14,  14,  20,  255
#define TEXT_COLOR 255, 255, 255, 255

#define ROW_SEP    48,  48,  58,  255
#define ROW_BG_1   28,  28,  38,  255
#define ROW_BG_2   LEFT_BG

#define ICON_NORMAL  255, 255, 255
#define ICON_HOVER   200, 200, 200
#define ICON_PRESSED 142, 142, 142

#define LEFT_PANEL_WIDTH 240

#define PAD           14.0f
#define PLAY_SIZE     36.0f
#define TITLE_GAP     8.0f
#define NAME_GAP      10.0f
#define PRESS_SHRINK  4.0f

#define DUCK_WIDTH    200.0f
#define DUCK_PAD      28.0f
#define DUCK_SRC_W    297.0f
#define DUCK_SRC_H    360.0f

typedef struct {
    const char* name;
    const char* path;
} SoundDef;

static const SoundDef honorableMention = {
    "Honorable mention", "assets/sound/duck-toy.wav",
};

static const SoundDef soundEffects[] = {
    { "1. Quack",       "assets/sound/quack.wav"       },
    { "2. Hahahaha",    "assets/sound/hahahaha.wav"    },
    { "3. Punch",       "assets/sound/punch.wav"       },
    { "4. Silly laugh", "assets/sound/silly-laugh.wav" },
    { "5. Auughhh",     "assets/sound/auughhh.wav"     },
};

#define NUM_SOUNDS \
    (sizeof(soundEffects) / sizeof(SoundDef))

#define NUM_SLOTS (NUM_SOUNDS + 1)

// same thing as in discover.c:99
#define FNUM_SOUNDS \
    (float)(uint)NUM_SOUNDS

#define NOHOVER NUM_SLOTS

typedef enum {
    STOPPED,
    PLAYING,
    PAUSED,
} SlotPlayback;

typedef struct {
    Uint8* wav;
    Uint32 wavLen;
    SDL_AudioSpec spec;
    SDL_AudioStream* stream;

    TTF_Text* nameText;

    SDL_FRect playRect;
    SlotPlayback playback;
} SoundSlot;

typedef struct {
    SoundSlot honorable;
    SoundSlot sounds[NUM_SOUNDS];

    TTF_Text* honorableTitle;
    SDL_FPoint titlePos;
    SDL_Texture* duck;
    SDL_FRect duckRect;

    SDL_Texture* playIcon;
    SDL_Texture* pauseIcon;

    float leftWidth;
    float rightOff;
    float rowHeight;

    uint hovered;
    uint pressed;
} State;

static SoundSlot* SlotAt(State* state, uint idx) {
    if (idx == 0) return &state->honorable;
    if (idx <= NUM_SOUNDS) return &state->sounds[idx - 1];
    return NULL;
}

static bool InitSlot(SoundSlot* slot, const SoundDef* def) {
    memset(slot, 0, sizeof(SoundSlot));

    if (!SDL_LoadWAV(def->path, &slot->spec, &slot->wav, &slot->wavLen)) {
        SDL_Log("Failed to load %s: %s", def->path, SDL_GetError());
        return false;
    }

    slot->stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &slot->spec, NULL, NULL);

    if (slot->stream == NULL) {
        SDL_Log("Failed to open stream for %s: %s", def->path, SDL_GetError());
        SDL_free(slot->wav);
        slot->wav = NULL;
        return false;
    }

    slot->nameText = TTF_CreateText(tengine, f.normal, def->name, 0);
    assert(slot->nameText != NULL);
    TTF_SetTextColor(slot->nameText, TEXT_COLOR);

    return true;
}

static void CleanupSlot(SoundSlot* slot) {
    if (slot->stream) SDL_DestroyAudioStream(slot->stream);
    if (slot->wav) SDL_free(slot->wav);
    if (slot->nameText) TTF_DestroyText(slot->nameText);
    memset(slot, 0, sizeof(*slot));
}

static void SyncPlaying(SoundSlot* slot) {
    if (slot->playback != PLAYING || slot->stream == NULL)
        return;
    if (SDL_GetAudioStreamQueued(slot->stream) == 0)
        slot->playback = STOPPED;
}

static void ToggleSlot(SoundSlot* slot) {
    if (slot->stream == NULL) return;

    if (slot->playback == PLAYING) {
        SDL_PauseAudioStreamDevice(slot->stream);
        slot->playback = PAUSED;
        return;
    }

    if (slot->playback == PAUSED) {
        SDL_ResumeAudioStreamDevice(slot->stream);
        slot->playback = PLAYING;
        return;
    }

    SDL_ClearAudioStream(slot->stream);
    SDL_PutAudioStreamData(slot->stream, slot->wav, (int)slot->wavLen);
    SDL_FlushAudioStream(slot->stream);
    SDL_ResumeAudioStreamDevice(slot->stream);
    slot->playback = PLAYING;
}

static uint SoundAtPoint(const State* state, SDL_FPoint point) {
    if (SDL_PointInRectFloat(&point, &state->honorable.playRect))
        return 0;

    for (uint i = 0; i < NUM_SOUNDS; ++i) {
        if (SDL_PointInRectFloat(&point, &state->sounds[i].playRect)) {
            return i + 1;
        }
    }
    return NOHOVER;
}

static void Layout(State* state) {
    state->leftWidth = LEFT_PANEL_WIDTH;
    state->rightOff = state->leftWidth;

    state->rowHeight = SOUNDBOARD_HEIGHT / FNUM_SOUNDS;

    int titleWidth, titleHeight;
    TTF_GetTextSize(state->honorableTitle, &titleWidth, &titleHeight);

    state->titlePos = (SDL_FPoint) {
        .x = (state->leftWidth - titleWidth) * 0.5f,
        .y = PAD,
    };

    float leftCenter  = state->leftWidth * 0.5f;
    float titleBottom = state->titlePos.y + titleHeight + TITLE_GAP;

    state->honorable.playRect = (SDL_FRect) {
        .x = leftCenter - PLAY_SIZE * 0.5f,
        .y = titleBottom,
        .w = PLAY_SIZE,
        .h = PLAY_SIZE,
    };

    float duckTop = state->honorable.playRect.y + PLAY_SIZE + DUCK_PAD;

    float duckWidth  = DUCK_WIDTH;
    float duckHeight = duckWidth * (DUCK_SRC_H / DUCK_SRC_W);

    state->duckRect = (SDL_FRect ){
        .x = leftCenter - duckWidth * 0.5f,
        .y = duckTop,
        .w = duckWidth,
        .h = duckHeight,
    };

    float rightInnerWidth = SOUNDBOARD_WIDTH - state->rightOff;
    for (uint i = 0; i < NUM_SOUNDS; ++i) {
        state->sounds[i].playRect = (SDL_FRect) {
            .x = state->rightOff + rightInnerWidth - PAD - PLAY_SIZE,
            .y = i * state->rowHeight + (state->rowHeight - PLAY_SIZE) * 0.5f,
            .w = PLAY_SIZE,
            .h = PLAY_SIZE,
        };
    }
}

void SoundboardAppInit(Window* win) {
    State* state = calloc(1, sizeof(State));
    assert(state != NULL);

    state->hovered = NOHOVER;
    state->pressed = NOHOVER;

    state->honorableTitle = TTF_CreateText(tengine, f.bold, "Honorable\n  mention", 0);
    assert(state->honorableTitle != NULL);
    TTF_SetTextColor(state->honorableTitle, TEXT_COLOR);

    InitSlot(&state->honorable, &honorableMention);
    for (uint i = 0; i < NUM_SOUNDS; ++i)
        InitSlot(&state->sounds[i], &soundEffects[i]);

    state->duck      = LoadTexPNG("assets/images/toy-duck.png");
    state->playIcon  = LoadTexPNG("assets/icons/play.png");
    state->pauseIcon = LoadTexPNG("assets/icons/pause.png");

    assert(state->duck      != NULL);
    assert(state->playIcon  != NULL);
    assert(state->pauseIcon != NULL);

    Layout(state);
    win->userData = state;
}

void SoundboardAppCleanup(Window* win) {
    State* state = win->userData;
    assert(state != NULL);

    CleanupSlot(&state->honorable);
    for (uint i = 0; i < NUM_SOUNDS; ++i)
        CleanupSlot(&state->sounds[i]);

    TTF_DestroyText(state->honorableTitle);
    SDL_DestroyTexture(state->duck);
    SDL_DestroyTexture(state->playIcon);
    SDL_DestroyTexture(state->pauseIcon);

    free(state);
    win->userData = NULL;
}

static void SetIconsColorMod(State* state, Uint8 r, Uint8 g, Uint8 b) {
    SDL_SetTextureColorMod(state->playIcon,  r, g, b);
    SDL_SetTextureColorMod(state->pauseIcon, r, g, b);
}

static void DrawPlayButton(
    State* state, SDL_Renderer* renderer,
    SDL_FRect contentRect, SoundSlot* slot, uint idx
) {
    SyncPlaying(slot);

    if (state->pressed == idx)
        SetIconsColorMod(state, ICON_PRESSED);
    else if (state->hovered == idx)
        SetIconsColorMod(state, ICON_HOVER);
    else
        SetIconsColorMod(state, ICON_NORMAL);

    SDL_Texture* icon =
        slot->playback == PLAYING
            ? state->pauseIcon
            : state->playIcon;

    SDL_FRect dst = OffsetRect(contentRect, slot->playRect);
    SDL_RenderTexture(renderer, icon, NULL, &dst);
}

static void DrawSoundRow(
    State* state, SDL_Renderer* renderer,
    SDL_FRect contentRect, uint i
) {
    SoundSlot* slot = &state->sounds[i];

    SDL_FRect row = {
        .x = contentRect.x + state->rightOff,
        .y = contentRect.y + i * state->rowHeight,
        .w = SOUNDBOARD_WIDTH - state->rightOff,
        .h = state->rowHeight,
    };

    // doing everything just not to use an if statement...
    i % 2 == 0
    ? SDL_SetRenderDrawColor(renderer, ROW_BG_2)
    : SDL_SetRenderDrawColor(renderer, ROW_BG_1);

    SDL_RenderFillRect(renderer, &row);

    if (i > 0) {
        SDL_SetRenderDrawColor(renderer, ROW_SEP);
        SDL_RenderLine(renderer, row.x, row.y, row.x + row.w, row.y);
    }

    int nw = 0, nh = 0;
    TTF_GetTextSize(slot->nameText, &nw, &nh);

    TTF_DrawRendererText(
        slot->nameText,
        row.x + PAD,
        row.y + (state->rowHeight - nh) * 0.5f);

    DrawPlayButton(state, renderer, contentRect, slot, i + 1);
}

void SoundboardAppRender(Window* win, SDL_Renderer* renderer, SDL_FRect contentRect) {
    State* state = win->userData;
    assert(state != NULL);

    SDL_FRect leftBg = {
        .x = contentRect.x,
        .y = contentRect.y,
        .w = state->leftWidth,
        .h = contentRect.h,
    };

    SDL_SetRenderDrawColor(renderer, LEFT_BG);
    SDL_RenderFillRect(renderer, &leftBg);

    SDL_SetRenderDrawColor(renderer, ROW_SEP);
    SDL_RenderLine(
        renderer,
        contentRect.x + state->rightOff,
        contentRect.y,
        contentRect.x + state->rightOff,
        contentRect.y + contentRect.h);

    TTF_DrawRendererText(
        state->honorableTitle,
        contentRect.x + state->titlePos.x,
        contentRect.y + state->titlePos.y);

    DrawPlayButton(state, renderer, contentRect, &state->honorable, 0);

    SDL_FRect duckDst = OffsetRect(contentRect, state->duckRect);
    SDL_RenderTexture(renderer, state->duck, NULL, &duckDst);

    for (uint i = 0; i < NUM_SOUNDS; ++i)
        DrawSoundRow(state, renderer, contentRect, i);
}

bool SoundboardAppHandleEvent(Window* win, const SDL_Event* event, SDL_FPoint localMouse) {
    State* state = win->userData;
    assert(state != NULL);

    if (event->type == SDL_EVENT_MOUSE_MOTION) {
        state->hovered = SoundAtPoint(state, localMouse);
        if (state->pressed != NOHOVER && state->hovered != state->pressed)
            state->pressed = NOHOVER;
        return state->hovered != NOHOVER;
    }

    bool click =
        event->type == SDL_EVENT_MOUSE_BUTTON_DOWN
     && event->button.button == SDL_BUTTON_LEFT;

    bool unclick =
        event->type == SDL_EVENT_MOUSE_BUTTON_UP
     && event->button.button == SDL_BUTTON_LEFT;

    uint idx = SoundAtPoint(state, localMouse);
    if (click) {
        if (idx != NOHOVER) {
            state->pressed = idx;
            return true;
        }
    } else if (unclick) {
        if (state->pressed != NOHOVER && idx == state->pressed) {
            SoundSlot* slot = SlotAt(state, idx);
            if (slot) ToggleSlot(slot);
        }

        state->pressed = NOHOVER;
        return idx != NOHOVER;
    }

    return false;
}

bool SoundboardAppWantsPointerCursor(Window* win, SDL_FPoint localMouse) {
    State* state = win->userData;
    assert(state != NULL);
    return SoundAtPoint(state, localMouse) != NOHOVER;
}
