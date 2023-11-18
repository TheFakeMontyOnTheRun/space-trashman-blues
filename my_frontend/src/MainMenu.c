/*
   Created by monty on 01-10-2023.
*/
#include <stdlib.h>
#include <stdint.h>
#include "Enums.h"
#include "UI.h"
#include "Renderer.h"

const uint8_t shapes[] = {
        7,
        28, 103,
        16, 140,
        54, 140,
        55, 107,
        38, 111,
        32, 100,
        32, 100,
        7,
        30, 116,
        32, 109,
        40, 117,
        54, 117,
        54, 140,
        28, 140,
        28, 140,
        10,
        32, 68,
        31, 92,
        38, 106,
        53, 99,
        54, 96,
        54, 107,
        38, 111,
        31, 97,
        31, 92,
        31, 87,
        8,
        54, 39,
        44, 38,
        34, 43,
        31, 66,
        35, 79,
        46, 82,
        54, 74,
        54, 74,
        6,
        37, 79,
        39, 98,
        43, 99,
        53, 76,
        46, 81,
        46, 81,
        0 };

#if !defined(ANDROID) && !defined(__EMSCRIPTEN__) && !defined(AGS) && !defined(N64) && !defined(NDS)
const char *MainMenu_options[4] = {
        "Play game",
        "Credits",
        "Help",
        "Quit"};

const enum EGameMenuState MainMenu_nextStateNavigation[4] = {
        kPlayGame, kCredits, kHelp,
        kQuit};

const uint8_t kMainMenuOptionsCount = 4;
#else
const char *MainMenu_options[3] = {
        "Play game", "Credits", "Help"};

const int32_t MainMenu_nextStateNavigation[3] = {
        kPlayGame, kCredits, kHelp};

const uint8_t kMainMenuOptionsCount = 3;
#endif

extern size_t biggestOption;
extern int8_t cursorPosition;

void MainMenu_initStateCallback(enum EGameMenuState tag) {
    cursorPosition = 0;
    biggestOption = 9;
}

void MainMenu_initialPaintCallback() {
    clearScreen();
}

void MainMenu_repaintCallback(void) {
    drawGraphic(shapes);

    drawWindowWithOptions(
            (XRES_FRAMEBUFFER / 8) - (int) biggestOption - 3,
            (YRES_FRAMEBUFFER / 8) - 3 - kMainMenuOptionsCount,
            biggestOption + 2,
            kMainMenuOptionsCount + 2,
            "Episode 0",
            MainMenu_options,
            kMainMenuOptionsCount,
            cursorPosition);
}

enum EGameMenuState MainMenu_tickCallback(enum ECommand cmd, long delta) {
    return handleCursor(MainMenu_nextStateNavigation, kMainMenuOptionsCount, cmd, kResumeCurrentState);
}

void MainMenu_unloadStateCallback(enum EGameMenuState newState) {
}
