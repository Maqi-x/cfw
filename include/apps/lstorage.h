#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <windows.h>
#include <ft.h>

#define LSTORAGE_WIDTH  480.0f
#define LSTORAGE_HEIGHT 580.0f

used_but_the_compiler_is_stupid_and_thinks_that_it_is_unused
static TextFragment LStorageDesc[] = {
    T_H2("Local Storage Demo\n"),
    T_BOLD("How it works?\n\n"),
    T("Emscripten exposes many ways of interacting with browser APIs, but the most universal ones are "),
    T_CODE("EM_ASM"), T(", and "), T_CODE("EM_JS"), T(".\n"), T_BOLD("They allow us to execute JS code directly from C. "),
    T("The difference between them is that "), T_CODE("EM_JS"), T(" returns a value directly but there is only a small fixed set "),
    T("of allowed return types and passing arguments from C is possible but limited. "), T_BOLD("It is good for quick one liners and simple scripts."),
    T("\n\n"),
    T_CODE("EM_JS"), T(", on the other hand allows us to define a typed C function but write its body in JS. "), T_BOLD("It supports "),
    T_BOLD("passing arguments and returning complex values natively. "), T("Also, inside the JS body we have access to many useful helper "),
    T("functions for interacting with C, for example "), T_CODE("_malloc"), T(" or functions for converting cstrings to JS strings."),
    T("\n\n\n"),
    T("For this demo we use "), T_CODE("EM_JS"), T(" because in this case it's cleaner and more practical. You can see the source code "),
    T("in the "), T_BOLD("web.c file in our GitHub repository", .href = "https://github.com/Maqi-x/cfw/blob/main/src/web.c"), T(". "),
    T("The file contains the glue code between C and JS for localStorage APIs."),
    T("\n\n\n\n"),
    T_BOLD("TODOs:\n"),
    T(" - Show a message box after clicking the Save button\n"),
    T(" - Show error message when trying to use Save/Load on native target\n"),
};

void LStorageAppInit(Window* win);
void LStorageAppCleanup(Window* win);

void LStorageAppRender(Window* win, SDL_Renderer* renderer, SDL_FRect contentRect);
bool LStorageAppHandleEvent(Window* win, const SDL_Event* event, SDL_FPoint localMouse);

void LStorageAppChangeFocus(Window* win, bool focused);
CursorKind LStorageAppGetCursorKind(Window* win, SDL_FPoint localMouse);
