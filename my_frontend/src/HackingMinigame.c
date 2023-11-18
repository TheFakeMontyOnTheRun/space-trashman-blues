/*
   Created by Daniel Monteiro on 2021-11-01.
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "Enums.h"
#include "UI.h"
#include "Core.h"
#include "Derelict.h"
#include "Renderer.h"
#include "HackingMinigame.h"
#include "HackingMinigameRules.h"

extern int8_t cursorPosition;

const char *functionNames[5] = {
        "???",
        "writeB",
        "snprintf",
        "hackLogin",
        "LoadDBReg"
};

void HackingScreen_initStateCallback(int32_t tag) {
    cursorPosition = 1;
    needs3dRefresh = 0;

    initHackingMinigame();
}

void HackingScreen_repaintCallback(void) {
    uint8_t pin;
	uint8_t c;
    uint8_t holdingDisk;

    drawTextAt(1, 1, "Stack trace:", 1);

    drawTextAt((12 * 0), 11, cursorPosition == 0 ? "[CPU0]" : " CPU0 ", 1);
    drawTextAt((12 * 1), 11, cursorPosition == 1 ? "[CPU1]" : " CPU1 ", 1);
    drawTextAt((12 * 2), 11, cursorPosition == 2 ? "[CPU2]" : " CPU2 ", 1);

    for (c = 0; c < 31; ++c ) {
        drawTextAt(c, 10, "-", 1);
    }

    for (pin = 0; pin < 3; ++pin) {
        uint8_t disk;

        for (disk = 0; disk < 5; ++disk) {

            uint8_t diskIndex = getPositionForPin(pin, disk);

            char *funcName = (disk >= getDisksForPin(pin)) ? NULL : functionNames[diskIndex];

            if (pin != 0) {
                drawTextAt(10 * (pin), 4 + (4 - disk), "I", 1);
            }

            if (funcName) {
                drawTextAt(10 * (pin) + (pin == 0 ? 0 : 1), 4 + (4 - disk), funcName, 1);
            }
        }
    }

    drawTextAt(1, 2, "Pointer:", 1);

    holdingDisk = getHoldingDisk();

    if (holdingDisk != 0xFF) {
        drawTextAt(19, 2, functionNames[holdingDisk], 1);
    } else {
        drawTextAt(19, 2, "NULL", 1);
    }
}

void HackingScreen_initialPaintCallback(void) {

}

void HackingScreen_unloadStateCallback(int32_t newState) {

}

enum EGameMenuState HackingScreen_tickCallback(enum ECommand cmd, long data) {
    
    uint8_t holdingDisk = getHoldingDisk();

    if (isHackingMinigameCompleted()) {
        grantAccessToSafe();
    }

    switch (cmd) {
        case 'q':
        case 'a':
            if (cursorPosition > 0) {
                cursorPosition--;
            }
            break;
        case 'e':
        case 'd':
            if (cursorPosition < 2) {
                cursorPosition++;
            }
            break;
        case 's':
            return -1;
        case 'w':
            clearScreen();
            if (holdingDisk == 0xFF) {
                pickDisk(cursorPosition);
            } else {
                dropDisk(cursorPosition);
            }
            break;

        default:
            break;
    }

    return kResumeCurrentState;
}
