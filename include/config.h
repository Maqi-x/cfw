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
    T_BOLD("No. It is not. "),
    T("As you might have already noticed, you can't select text on this site. This is because the browser doesn't know that there's text.\n"),
    T("Because we're rendering everything ourselves to a "), T_CODE("<canvas>"), T(" element, all the browser sees is a grid of pixels. "),
    T("This means that all the browser features like translators, Find in page, right-click menu, print, or the best extension ever - "),
    T_ITALIC("Microsoft to Microslop", .href = "https://addons.mozilla.org/en-US/firefox/addon/microsoft-to-microslop/"),
    T(" won't work. Even basic features like scrolling or clickable links are implemented manually in this page's source code.\n"),
    T_BOLD("If you want to learn more about how this actually works you can check out "), T_BOLD("our GitHub repository", .href = "https://github.com/Maqi-x/cfw"),
    T(".\n\n\nI won't cover everything in this section because it would take too long. There are plenty of other issues, but... there are also "),
    T_ITALIC("some"), T(" benefits. "), T_BOLD("WebAssembly is faster"), T(" than JavaScript. Even though modern JS engines are optimized "),
    T("they are still slower. Because of dynamic typing, JS can't just be fully AOT-compiled to efficient machine code so instead JS engines rely "),
    T("on a mix of interpretation and Just-in-Time Compilation. "), T_BOLD("Dynamic typing is the source of all evil again."), T(" Who would have thought?\n"),
    T("Jokes aside. Yes it is faster, which does not mean that it's practical for pages whose entire job is to display text on the screen. "),
    T("But this project was never meant to be practical. I'm doing it mostly for fun and because I can. Also, I hate JS, but I hate CSS even more.\n"),
    T("However, this doesn't mean that WebAssembly is useless! This stack is actually really useful if you're using it for right things, for example, for "),
    T_BOLD("video games"), T(", Wasm allows us to write high-performance code that runs directly in the browser. It's also really easy to port existing "),
    T("SDL games to work with Emscripten, or target both native and web platforms."),
    T("\n\n\n\n"),

    T_H2("Sources\n"),
    T_BOLD("SDL3 For Emscripten: "), T("wiki.libsdl.org/SDL3/README-emscripten\n", .href = "https://wiki.libsdl.org/SDL3/README-emscripten"),
    T_BOLD("Emscripten Documentation: "), T("emscripten.org/docs\n", .href = "https://emscripten.org/docs/index.html"),
    T_BOLD("WebAssembly Official Site: "), T("webassembly.org\n", .href = "https://webassembly.org"),
    T_BOLD("WebAssembly Documentation: "), T("developer.mozilla.org/en-US/docs/WebAssembly\n", .href = "https://developer.mozilla.org/en-US/docs/WebAssembly"),
    T("\n\n\n\n\n\n"),
};

static const SDL_Color
    text_color = { 255, 255, 255, 255 },
    link_color = { 53,  163, 219, 255 };

#define MAINTEXT_X 40
#define MAINTEXT_Y 60
#define TOPBAR_PADDING 20

#define SCROLL_SPEED 20.0f
#define SCROLL_WHEEL_STEP 45.0f

#define CONTENT_Y (MAINTEXT_Y + TOPBAR_PADDING)

#define GITHUB_LINK "https://github.com/Maqi-x/cfw.git"
