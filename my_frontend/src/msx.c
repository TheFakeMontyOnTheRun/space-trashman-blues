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
#include "UI.h"
#include "KeyboardUI.h"

uint8_t updateDirection;

/* Sadly, I can't include conio.h - otherwise, I would get errors when building on OSX */
int kbhit(void);
int getch(void);
extern uint8_t firstFrameOnCurrentState;
enum ESoundDriver soundDriver = kNoSound;

/*  Required since we have our own memory allocator abstraction */
uint16_t heap = 0;

void initHW(void) {
    initTMS9918();
    initAY38910();
    initKeyboardUI();
    updateDirection = 1;
    needs3dRefresh = 0;
}

void handleSystemEvents(void) {}

enum ECommand getInput(void) {

    if (!kbhit()) {
        return kCommandNone;
    }

    uint8_t input = getch();

    performAction();

    switch (input) {
        case 30:
            return kCommandUp;
        case 31:
            return kCommandDown;
        case 29:
            updateDirection = 1;
            return kCommandLeft;
        case 28:
            updateDirection = 1;
            return kCommandRight;
        case 'c':
            return kCommandStrafeLeft;
        case 'v':
            return kCommandStrafeRight;
        case '1':
            if (waitForKey) {
                waitForKey = 0;
                firstFrameOnCurrentState = 1;
                needs3dRefresh = 1;
                return kCommandNone;
            }
            return kCommandFire1;
        case '2':
            return kCommandFire2;
        case '3':
            return kCommandFire3;
        case '4':
            return kCommandFire4;
        case '5':
            return kCommandFire5;
        case '6':
            return kCommandFire6;

    }
    return input;
}

void graphicsFlush(void) {
    if (needs3dRefresh) {
        flush3DBuffer();

        if (updateDirection) {
            char direction[8] = {'N', 0, 'E', 0, 'S', 0, 'W', 0};
            updateDirection = 0;
            writeStrWithLimit(12, 17, &direction[getPlayerDirection() * 2], 31, 2, 0);
        }
    }
}