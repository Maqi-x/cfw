#include <fonts.h>
#include <defs.h>

Fonts f;

typedef struct {
    TTF_Font** out;
    bool italic;

    uint size;
    TTF_FontStyleFlags style;
} FontSpec;

static const char* GetPath(const FontSpec* spec) {
    return spec->italic ? "assets/nunito/NunitoSans-Italic.ttf" : "assets/nunito/NunitoSans.ttf";
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
        { .out = &f.normal, .italic = false, .size = 23, .style = 0              },
        { .out = &f.bold,   .italic = false, .size = 23, .style = TTF_STYLE_BOLD },
        { .out = &f.italic, .italic = true,  .size = 23, .style = 0              },
        { .out = &f.h1,     .italic = false, .size = 34, .style = TTF_STYLE_BOLD },
        { .out = &f.h2,     .italic = false, .size = 28, .style = TTF_STYLE_BOLD },
    };

    for (usize i = 0; i < sizeof(fonts) / sizeof(FontSpec); ++i) {
        if (!LoadFont(fonts[i].out, &fonts[i])) {
            UnloadFonts();
            return false;
        }
    }

    return true;
}
