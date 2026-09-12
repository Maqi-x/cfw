#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <windows.h>

#define LSTORAGE_WIDTH  480.0f
#define LSTORAGE_HEIGHT 580.0f

void LStorageAppInit(Window* win);
void LStorageAppCleanup(Window* win);

void LStorageAppRender(Window* win, SDL_Renderer* renderer, SDL_FRect contentRect);
bool LStorageAppHandleEvent(Window* win, const SDL_Event* event, SDL_FPoint localMouse);

void LStorageAppChangeFocus(Window* win, bool focused);
CursorKind LStorageAppGetCursorKind(Window* win, SDL_FPoint localMouse);
