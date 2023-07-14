#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"

#include "TMS9918.h"
#include "YM2413.h"
#include "KeyboardUI.h"

char getch(void);

uint8_t updateDirection;

void init(void) {
    initTMS9918();
    initYM2413();
    initKeyboardUI();
    updateDirection = 1;
}

void refreshJustGraphics(void) {
    clearGraphics();
    renderScene();
    graphicsFlush();
}

void backToGraphics(void) {
    clearScreen();
    HUD_initialPaint();
    refreshJustGraphics();
}

uint8_t getKey(void) {
    uint8_t input = getch();

    performAction();

    switch (input) {
        case 30:
            return 'w';
        case 31:
            return 's';
        case 29:
            updateDirection = 1;
            return 'q';
        case 28:
            updateDirection = 1;
            return 'e';
        case 'z':
            return 'a';

        case 'x':
            return 'd';

    }
    return input;
}

void graphicsFlush(void) {
    flush3DBuffer();

    if (updateDirection) {
        updateDirection = 0;
        switch (getPlayerDirection()) {
            case 0:
                writeStrWithLimit(12, 17, "N", 31, 2, 0);
                break;
            case 1:
                writeStrWithLimit(12, 17, "E", 31, 2, 0);
                break;
            case 2:
                writeStrWithLimit(12, 17, "S", 31, 2, 0);
                break;
            case 3:
                writeStrWithLimit(12, 17, "W", 31, 2, 0);
                break;
        }
    }
}