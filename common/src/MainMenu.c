#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#endif

#include <stdlib.h>
#include "Enums.h"
#include "UI.h"
#include "SoundSystem.h"
#include "Engine.h"

#ifdef EMIT_QUIT_OPTION
const char *MainMenu_options[4] = {
        "Play game", "Credits", "Help", "Quit"};

const enum EGameMenuState MainMenu_nextStateNavigation[4] = {
        kPlayGame, kCredits, kHelp,
        kQuit};

const int kMainMenuOptionsCount = 4;
#else
const char *MainMenu_options[3] = {
        "Play game", "Credits", "Help"};

const enum EGameMenuState MainMenu_nextStateNavigation[3] = {
        kPlayGame, kCredits, kHelp};

const int kMainMenuOptionsCount = 3;
#endif

static const uint8_t splashGraphics[] = {
        6, 108, 103, 83,
        40, 63,
        42, 40,
        54, 39,
        54, 65,
        42, 66,
        40, 63,
        7, 85, 0, 0,
        32, 102,
        24, 112,
        21, 139,
        59, 139,
        59, 97,
        41, 94,
        24, 92,
        8, 43, 0, 0,
        35, 115,
        42, 116,
        59, 116,
        59, 139,
        32, 139,
        35, 132,
        35, 115,
        37, 90,
        10, 170, 0, 0,
        36, 64,
        35, 91,
        42, 105,
        57, 99,
        59, 95,
        59, 106,
        42, 110,
        35, 96,
        35, 91,
        36, 64,
        7, 85, 68, 0,
        35, 91,
        38, 74,
        59, 73,
        59, 96,
        57, 99,
        42, 105,
        35, 91,
        8, 170, 0, 0,
        59, 38,
        51, 34,
        38, 42,
        35, 65,
        39, 78,
        50, 81,
        59, 74,
        59, 38,
        9, 212, 170, 0,
        36, 71,
        35, 91,
        42, 105,
        57, 99,
        59, 95,
        58, 74,
        50, 80,
        39, 77,
        36, 67,
        8, 43, 34, 0,
        54, 88,
        55, 92,
        56, 95,
        51, 104,
        46, 99,
        45, 97,
        50, 91,
        55, 85,
        8, 85, 68, 0,
        41, 78,
        41, 85,
        43, 97,
        47, 98,
        54, 92,
        57, 75,
        50, 81,
        41, 78,
        6, 147, 172, 147,
        39, 43,
        45, 40,
        58, 45,
        58, 39,
        49, 35,
        39, 43,
        6, 170, 212, 0,
        36, 64,
        39, 75,
        40, 51,
        39, 49,
        38, 43,
        36, 64,
        6, 255, 221, 85,
        50, 78,
        53, 53,
        56, 52,
        59, 47,
        59, 72,
        50, 78,
        7, 170, 212, 0,
        39, 104,
        37, 100,
        31, 101,
        26, 115,
        29, 109,
        39, 104,
        52, 93,
        0 };

void MainMenu_initStateCallback(enum EGameMenuState tag) {
    cursorPosition = 0;
    playSound(MAIN_MENU_THEME);
}

void MainMenu_repaintCallback(void) {
    if (firstFrameOnCurrentState) {
        clearScreen();
        drawGraphic(splashGraphics);
    }

    drawWindowWithOptions(
            (XRES_FRAMEBUFFER / 8) - (int) 9 - 3,
            (YRES_FRAMEBUFFER / 8) - 3 - kMainMenuOptionsCount,
            9 + 2,
            kMainMenuOptionsCount + 2,
            "Episode 0",
            MainMenu_options,
            kMainMenuOptionsCount,
            cursorPosition);
}

enum EGameMenuState MainMenu_tickCallback(enum ECommand cmd, long delta) {
    return handleCursor(&MainMenu_nextStateNavigation[0], kMainMenuOptionsCount, cmd, kResumeCurrentState);
}

void MainMenu_unloadStateCallback(enum EGameMenuState newState) {
}
