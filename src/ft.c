#include <ft.h>
#include <vector.h>

#include <ctype.h>
#include <stdlib.h>

// while isspace is a macro in almost all libc implementations,
// the C standard also guarantees that it's a function, so you can
// for example, do this:
//   <some ugly type> function_ptr = isspace;
// we can use this trick and define our own version of the macro
// because I always forget to cast to uchar which the standard macro
// requires because it sucks. the entire c stdlib sucks. at least now
// the code is portable (it may be slower though, but who cares? CPUs
// in big 2026 are faster than they need to be anyway)
#undef isspace
#define isspace(c) \
    isspace((uchar)c)

// winapi grade coding
#define max(p, q) (((p) > (q)) ? (p) : (q))

typedef struct {
    TTF_Text* text;
    float x, y;
    uint w, h;
} Part;

VECTOR_DECLARE(Parts, parts, Part);
VECTOR_DEFINE(Parts, parts, Part);

struct FT {
    TTF_TextEngine* engine;

    Style style;
    const TextFragment* fragments;
    usize fragment_count;

    Parts parts;

    uint cw;
    uint ch;
};

void ensure_part_cap(FT* ft, usize need) {
    usize curr = vlen(&ft->parts);
    if (need > curr) parts_resize(&ft->parts, need);
}

static void free_parts(FT* ft) {
    for (Part* part = ft->parts.begin; part < ft->parts.end; ++part) {
        TTF_DestroyText(part->text);
    }
    parts_free(&ft->parts);

    ft->cw = 0;
    ft->ch = 0;
}

static TTF_Font* getfont(const FT* ft, TextKind kind) {
    switch (kind) {
    case TEXT_NORMAL:
        return ft->style.normal;
    case TEXT_H1:
        return ft->style.h1;
    case TEXT_H2:
        return ft->style.h2;
    case TEXT_BOLD:
        return ft->style.bold;
    case TEXT_ITALIC:
        return ft->style.italic;
    }
    unreachable();
}

static bool is_header(TextKind kind) {
    return kind == TEXT_H1 || kind == TEXT_H2;
}

static bool append_part(FT* ft, TTF_Font* font, const char* text, usize len, float x, float y) {
    if (len == 0)
        return true;

    TTF_Text* tt = TTF_CreateText(ft->engine, font, text, len);
    if (tt == NULL) return false;

    TTF_SetTextColor(tt,
        ft->style.text_color.r,
        ft->style.text_color.g,
        ft->style.text_color.b,
        ft->style.text_color.a);

    int w, h;
    if (!TTF_GetTextSize(tt, &w, &h)) {
        TTF_DestroyText(tt);
        return false;
    }

    parts_push(&ft->parts, (Part) {
        .text = tt,
        .x = x, .y = y,
        .w = w, .h = h,
    });

    ft->cw = max(x + w, ft->cw);
    ft->ch = max(y + h, ft->cw);
    return true;
}

static bool finish_line(FT* ft, float* x, float* y, uint* lineheight, bool paragraph_break) {
    uint gap = paragraph_break ? ft->style.paragraph_gap : ft->style.line_gap;

    *x = 0;
    *y += *lineheight + gap;
    *lineheight = 0;

    return true;
}

static bool append_token(
    FT* ft, TTF_Font* font, const char* text, usize len,
    float* x, float* y, uint* lineheight
) {
    int w, h;
    if (!TTF_GetStringSize(font, text, len, &w, &h))
        return false;

    if (ft->style.width > 0)
        if (*x > 0.0f && (*x + w) > ft->style.width)
            finish_line(ft, x, y, lineheight, false);

    if (!append_part(ft, font, text, len, *x, *y))
        return false;

    *x += w;
    *lineheight = max(*lineheight, h);

    return true;
}

static bool update_layout_fragment(
    FT* ft, const TextFragment* frag, float* x, float* y, uint* lineheight
) {
    TTF_Font* font = getfont(ft, frag->kind);
    const char* p = frag->content;

    // we always want a new line BEFORE and AFTER a header
    bool is_blocky = is_header(frag->kind);
    if (is_blocky) {
        if (*x > 0.0f || vlen(&ft->parts) > 0) {
            finish_line(ft, x, y, lineheight, true);
            *y += ft->style.heading_gap;
        }
    }

    while (*p != '\0') {
        if (*p == '\n') {
            finish_line(ft, x, y, lineheight, true);
            p++;
            continue;
        }

        if (isspace(*p)) {
            while (*p != '\0' && *p != '\n' && isspace(*p)) p++;
            if (*x > 0.0f) {
                if (!append_token(ft, font, " ", 1, x, y, lineheight)) {
                    return false;
                }
            }
            continue;
        }

        const char* beg = p;
        while (*p != '\0' && !isspace(*p))
            p++;

        if (!append_token(ft, font, beg, (usize)(p - beg), x, y, lineheight)) {
            return false;
        }
    }

    if (is_blocky) {
        finish_line(ft, x, y, lineheight, true);
    }

    return true;
}

bool update_layout(FT* ft) {
    uint lineheight = 0;
    float x = 0, y = 0;

    free_parts(ft);

    for (usize i = 0; i < ft->fragment_count; i++) {
        if (!update_layout_fragment(ft, &ft->fragments[i], &x, &y, &lineheight)) {
            free_parts(ft);
            return false;
        }
    }

    if (lineheight > 0)
        y += lineheight;

    ft->ch = y;
    return true;
}

FT* ft_create(TTF_TextEngine* engine, const Style* style) {
    FT* ft = calloc(1, sizeof(*ft));
    if (ft == NULL) return NULL;

    ft->engine = engine;
    if (style != NULL) {
        ft->style = *style;
    }

    if (ft->style.line_gap == 0)      ft->style.line_gap = 4;
    if (ft->style.paragraph_gap == 0) ft->style.paragraph_gap = 12;
    if (ft->style.heading_gap == 0)   ft->style.heading_gap = 10;

    return ft;
}

void ft_destroy(FT* ft) {
    free_parts(ft);
    free(ft);
}

bool ft_set_width(FT* ft, uint width) {
    ft->style.width = width;
    if (ft->fragments == NULL)
        return true;

    return update_layout(ft);
}

bool ft_set_fragments(FT* ft, const TextFragment* fragments, usize count) {
    ft->fragments = fragments;
    ft->fragment_count = count;
    return update_layout(ft);
}

void ft_draw(FT* ft, float x, float y) {
    for (Part* part = ft->parts.begin; part < ft->parts.end; ++part) {
        TTF_DrawRendererText(part->text, x + part->x, y + part->y);
    }
}

void ft_get_size(const FT* ft, uint* w, uint* h) {
    if (w != NULL) *w = ft->cw;
    if (h != NULL) *h = ft->ch;
}

void ft_clear(FT* ft) {
    ft->fragments = NULL;
    ft->fragment_count = 0;
    free_parts(ft);
}
