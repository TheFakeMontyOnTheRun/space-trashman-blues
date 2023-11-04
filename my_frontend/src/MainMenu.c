/*
   Created by monty on 01-10-2023.
*/
#include <stdlib.h>
#include <stdint.h>
#include "Enums.h"
#include "UI.h"
#include "Renderer.h"

const uint8_t shapes[] = {
        7, 2,
        28, 103,
        16, 140,
        54, 140,
        55, 107,
        38, 111,
        32, 100,
        32, 100,
        7, 2,
        30, 116,
        32, 109,
        40, 117,
        54, 117,
        54, 140,
        28, 140,
        28, 140,
        10, 2,
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
        8, 2,
        54, 39,
        44, 38,
        34, 43,
        31, 66,
        35, 79,
        46, 82,
        54, 74,
        54, 74,
        6, 2,
        37, 79,
        39, 98,
        43, 99,
        53, 76,
        46, 81,
        46, 81,
        0 };

int drawn = 0;

void MainMenu_initStateCallback(int32_t tag) {
    needs3dRefresh = 0;
}

void MainMenu_initialPaintCallback(void) {
    clearScreen();
}

void MainMenu_repaintCallback(void) {
    if(!drawn) {
        drawn = 1;

        clearScreen();

        drawGraphic(shapes);

#ifndef GAMEPAD
        writeStr(16, 1, "Sub Mare\nImperium:\nDerelict\nby\nDaniel Monteiro\nPress SPACE to\nstart ");
#else
        writeStr(16, 1, "Sub Mare\nImperium:\nDerelict\nby\nDaniel Monteiro\nPress Start!");
#endif
    }
}

enum EGameMenuState MainMenu_tickCallback(enum ECommand cmd, long data) {

    if (cmd != '.') {
        return kPlayGame;
    }

    return kResumeCurrentState;
}

void MainMenu_unloadStateCallback(int32_t newState) {

}
