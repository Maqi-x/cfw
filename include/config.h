#pragma once

#include <ft.h>

static const TextFragment tTitle[] = {
    T_H1("C For Web"),
};

static const TextFragment tDescription[] = {
    T_H1("Webdev that's actually fun.\n"),
    T("\n\n\n\n"),

    T_BOLD("This website is written entirely in C.\n"),
    T_H2("...But, how is this possible?\n"),

    T("It's simple! Back in the day, browsers only ran "), T_BOLD("JavaScript"), T(", a language that nobody likes "),
    T_ITALIC("(depending on who you ask, but the truth is that it's poorly designed)"), T(".\n"),

    T("But now, we have a cool thing called "), T_BOLD("WebAssembly"), T(". It's a low-level language somewhat similar to LLVM IR, "),
    T("which your browser compiles to machine code under the hood and executes in a sandbox. We can compile C to WebAssembly using "),
    T_BOLD("the Emscripten project"), T(", a complete toolchain targeting Wasm, which also provides ports of many essential libraries. "),
    T("This allows us to use "), T_BOLD("SDL3"), T(" on the World Wide Web!\n"),
    T("Of course, we can't just draw directly to the browser window, so we use the HTML "), T_CODE("<canvas>"), T(" element instead. "),
    T("It allows us to draw arbitrary pixels on the page natively, and the Emscripten SDL3 port works with this approach perfectly.\n"),
    T("\n"),

    T_H2("Is it practical? The limitations\n"),
    T_BOLD("No."), T_ITALIC(" (This section will be expanded in the future, there are many limitations and other issues.)\n"),
    T("\n\n\n\n\n\n"),

    T_ITALIC("This page is in early development.\n"),
    T_H2("TODOs:\n"),
    T(" - Clickable links\n"),
    T(" - Selecting text and right-click menu\n"),
    T(" - Other stuff.\n"),
};

static const SDL_Color
    text_color = { 255, 255, 255, 255 };

#define MAINTEXT_X 40
#define MAINTEXT_Y 60
#define TOPBAR_PADDING 20

#define SCROLL_SPEED 20.0f
#define SCROLL_WHEEL_STEP 45.0f

