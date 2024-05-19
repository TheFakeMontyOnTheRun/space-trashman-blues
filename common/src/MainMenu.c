#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#endif

#include "Common.h"
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

const uint8_t kMainMenuOptionsCount = 4;
#else
const char *MainMenu_options[3] = {
        "Play game", "Credits", "Help"};

const enum EGameMenuState MainMenu_nextStateNavigation[3] = {
        kPlayGame, kCredits, kHelp};

const uint8_t kMainMenuOptionsCount = 3;
#endif

static const uint8_t splashGraphics[] = {
        7,
#ifndef MONOCHROME_VECTORS
        85, 0, 0,
#endif
        64, 76,
        64, 76,
        46, 111,
        46, 111,
        106, 111,
        106, 71,
        78, 69,
        6,
#ifndef MONOCHROME_VECTORS
        43, 0, 0,
#endif
        68, 88,
        68, 88,
        106, 90,
        106, 90,
        106, 111,
        64, 111,
        10,
#ifndef MONOCHROME_VECTORS
        170, 0, 0,
#endif
        70, 40,
        69, 66,
        80, 79,
        104, 73,
        106, 70,
        106, 80,
        80, 84,
        68, 71,
        69, 66,
        70, 40,
        9,
#ifndef MONOCHROME_VECTORS
        170, 0, 0,
#endif
        106, 15,
        106, 15,
        73, 19,
        73, 19,
        69, 41,
        75, 53,
        92, 56,
        106, 49,
        106, 15,
        9,
#ifndef MONOCHROME_VECTORS
        212, 170, 0,
#endif
        70, 47,
        69, 65,
        80, 79,
        103, 73,
        106, 70,
        106, 49,
        92, 55,
        75, 53,
        70, 43,
        9,
#ifndef MONOCHROME_VECTORS
        43, 34, 0,
#endif
        99, 63,
        99, 63,
        102, 70,
        102, 70,
        102, 70,
        85, 74,
        85, 74,
        85, 71,
        92, 66,
        10,
#ifndef MONOCHROME_VECTORS
        85, 68, 0,
#endif
        79, 54,
        79, 54,
        82, 72,
        82, 72,
        88, 73,
        88, 73,
        103, 51,
        103, 51,
        92, 56,
        79, 54,
        8,
#ifndef MONOCHROME_VECTORS
        147, 172, 147,
#endif
        74, 20,
        74, 20,
        106, 22,
        106, 22,
        106, 16,
        106, 16,
        74, 20,
        74, 20,
        7,
#ifndef MONOCHROME_VECTORS
        170, 212, 0,
#endif
        70, 40,
        76, 51,
        77, 28,
        77, 28,
        74, 21,
        74, 21,
        70, 40,
        7,
#ifndef MONOCHROME_VECTORS
        255, 221, 85,
#endif
        93, 53,
        97, 30,
        97, 30,
        106, 24,
        106, 24,
        106, 48,
        93, 53,
        4,
#ifndef MONOCHROME_VECTORS
        170, 212, 0,
#endif
        75, 78,
        70, 74,
        64, 77,
        62, 83,
        5,
#ifndef MONOCHROME_VECTORS
        0, 0, 0,
#endif
        72, 92,
        72, 92,
        70, 111,
        70, 111,
        106, 111,
        5,
#ifndef MONOCHROME_VECTORS
        128, 128, 0,
#endif
        78, 111,
        78, 111,
        103, 111,
        103, 111,
        79, 111,
        4,
#ifndef MONOCHROME_VECTORS
        0, 0, 255,
#endif
        82, 111,
        82, 111,
        89, 111,
        89, 111,
        3,
#ifndef MONOCHROME_VECTORS
        0, 0, (70757494 & 255),
#endif
        80, 84,
        75, 53,
        68, 71,
        5,
#ifndef MONOCHROME_VECTORS
        43, 0, 0,
#endif
        66, 84,
        66, 84,
        106, 86,
        106, 86,
        106, 84,
        3,
#ifndef MONOCHROME_VECTORS
        0, 0, 0,
#endif
        80, 84,
        92, 56,
        106, 80,
        0};
void MainMenu_initStateCallback(enum EGameMenuState tag) {
    (void)tag;
    cursorPosition = 0;
}

void MainMenu_repaintCallback(void) {
    if (firstFrameOnCurrentState) {
        clearScreen();
        drawGraphic(splashGraphics);
        playSound(MAIN_MENU_THEME);
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

enum EGameMenuState MainMenu_tickCallback(enum ECommand cmd, void *data) {
    (void)data;
    return handleCursor(&MainMenu_nextStateNavigation[0], kMainMenuOptionsCount, cmd, kResumeCurrentState);
}

void MainMenu_unloadStateCallback(enum EGameMenuState newState) {
    (void)newState;
}
