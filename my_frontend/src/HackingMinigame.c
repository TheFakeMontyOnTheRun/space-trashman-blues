/*
   Created by Daniel Monteiro on 2021-11-01.
*/

#include <stdint.h>
#include <string.h>

#include "Common.h"
#include "Enums.h"
#include "UI.h"
#include "Derelict.h"
#include "Renderer.h"
#include "HackingMinigame.h"
#include "HackingMinigameRules.h"
#include "Engine.h"

const char *functionNames[5] = {
        "???",
        "writeB",
        "snprintf",
        "hackLogin",
        "LoadDBReg"
};

void HackingScreen_initStateCallback(enum EGameMenuState tag) {
    (void)tag;
    cursorPosition = 1;
    needsToRedrawVisibleMeshes = 0;

    initHackingMinigame();
}

void HackingScreen_repaintCallback(void) {
    uint8_t pin;
    uint8_t holdingDisk;

    if (firstFrameOnCurrentState) {
        clearScreen();
        needsToRedrawVisibleMeshes = 0;
        drawTextAt(1, 1, "Stack trace:", 1);
        drawTextAt((12 * 0), 11, " CPU0 ", 1);
        drawTextAt((12 * 1), 11, " CPU1 ", 1);
        drawTextAt((12 * 2), 11, " CPU2 ", 1);
    }

    drawTextAt((12 * cursorPosition), 11, ">", 1);
    drawTextAt((12 * cursorPosition) + 5, 11, "<", 1);

    drawLine(0, 80, XRES_FRAMEBUFFER - 1, 80, 2);

    for (pin = 0; pin < 3; ++pin) {
        uint8_t disk;

        if (pin != 0) {
            uint8_t pinX = (10 * (pin) ) * 8;
            drawLine(pinX, 40, pinX, 80, 2);
        }

        for (disk = 0; disk < 5; ++disk) {

            uint8_t diskIndex = getPositionForPin(pin, disk);

            const char *funcName = (disk >= getDisksForPin(pin)) ? NULL
                                                           : functionNames[diskIndex];

            if (funcName) {
                drawTextAt(
                        10 * (pin) + (pin == 0 ? 0 : 1), 4 + (4 - disk),
                        funcName, 1);
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

void HackingScreen_unloadStateCallback(enum EGameMenuState newState) {
    (void)newState;
}

enum EGameMenuState HackingScreen_tickCallback(enum ECommand cmd, long data) {
    (void)data;
    uint8_t holdingDisk = getHoldingDisk();

    if (isHackingMinigameCompleted()) {
        grantAccessToSafe();
    }

    switch (cmd) {
        case kCommandLeft:
            if (cursorPosition > 0) {
                cursorPosition--;
                firstFrameOnCurrentState = 1;
            }
#ifdef PAGE_FLIP_ANIMATION
            turnTarget = turnStep;
#endif
            break;
        case kCommandRight:
            if (cursorPosition < 2) {
                cursorPosition++;
                firstFrameOnCurrentState = 1;
            }
#ifdef PAGE_FLIP_ANIMATION
            turnTarget = turnStep;
#endif
            break;
        case kCommandBack:
        case kCommandDown:
            return kBackToGame;
        case kCommandFire1:
            if (holdingDisk == 0xFF) {
                pickDisk(cursorPosition);
            } else {
                dropDisk(cursorPosition);
            }
            firstFrameOnCurrentState = 1;
            break;

        default:
            break;
    }

    return kResumeCurrentState;
}
