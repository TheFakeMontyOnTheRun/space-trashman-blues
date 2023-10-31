#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "Enums.h"
#include "Core.h"
#include "Derelict.h"
#include "Renderer.h"

#include "TMS9918.h"
#include "AY-3-8910.h"
#include "KeyboardUI.h"

char getch(void);

uint8_t updateDirection;

enum ESoundDriver soundDriver = kNoSound;

/*  Required since we have our own memory allocator abstraction */
uint16_t heap = 0;


void initHW(void) {
    initTMS9918();
    initAY38910();
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

void handleSystemEvents(void) {}

enum ECommand getInput(void) {
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