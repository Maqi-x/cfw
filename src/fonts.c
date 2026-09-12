#include <fonts.h>
#include <defs.h>

Fonts f;

typedef enum {
    NORMAL,
    ITALIC,
    MONO,
} FontKind;

typedef struct {
    TTF_Font** out;
    FontKind kind;

    uint size;
    TTF_FontStyleFlags style;
} FontSpec;

static const char* GetPath(const FontSpec* spec) {
    switch (spec->kind) {
    case NORMAL: return "assets/fonts/nunito/NunitoSans.ttf";
    case ITALIC: return "assets/fonts/nunito/NunitoSans-Italic.ttf";
    case MONO:   return "assets/fonts/JetBrainsMono-Thin.ttf";
    }
    unreachable();
}

static bool LoadFont(TTF_Font** out, const FontSpec* spec) {
    *out = TTF_OpenFont(GetPath(spec), spec->size);
    if (*out == NULL) {
        return false;
    }

    if (spec->style != 0) {
        TTF_SetFontStyle(*out, spec->style);
    }
    return true;
}

void CloseFont(TTF_Font** font) {
    if (*font != NULL) {
        TTF_CloseFont(*font);
        *font = NULL;
    }
}

void UnloadFonts() {
    CloseFont(&f.h2);
    CloseFont(&f.h1);
    CloseFont(&f.italic);
    CloseFont(&f.bold);
    CloseFont(&f.normal);
}

bool LoadFonts() {
    const FontSpec fonts[] = {
        { .out = &f.normal, .kind = NORMAL, .size = 23, .style = 0              },
        { .out = &f.bold,   .kind = NORMAL, .size = 23, .style = TTF_STYLE_BOLD },
        { .out = &f.italic, .kind = ITALIC, .size = 23, .style = 0              },
        { .out = &f.h1,     .kind = NORMAL, .size = 34, .style = TTF_STYLE_BOLD },
        { .out = &f.h2,     .kind = NORMAL, .size = 28, .style = TTF_STYLE_BOLD },
        { .out = &f.code,   .kind = MONO,   .size = 23, .style = 0              },
    };

    for (usize i = 0; i < sizeof(fonts) / sizeof(FontSpec); ++i) {
        if (!LoadFont(fonts[i].out, &fonts[i])) {
            UnloadFonts();
            return false;
        }
    }

    return true;
}
