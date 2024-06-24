/*
   Created by Daniel Monteiro on 2021-11-01.
*/

#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#endif
#include <string.h>

#include "Common.h"
#include "Enums.h"
#include "UI.h"
#include "Derelict.h"
#include "Renderer.h"
#include "HackingMinigame.h"
#include "HackingMinigameRules.h"
#include "Engine.h"

#ifdef PAGE_FLIP_ANIMATION
int wasSmoothMovementPreviouslyEnabled;
#endif

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
        drawTextAt(1, 1, "Stack trace:", getPaletteEntry(0xFF999999));

        drawTextAt((12 * cursorPosition), 11, ">    <", getPaletteEntry(0xFF999999));

        drawLine(0, 80, XRES_FRAMEBUFFER - 1, 80, 2);

        for (pin = 0; pin < 3; ++pin) {
            uint8_t disk;

            /* hack to save on some 30 bytes in the ROM size*/
            drawTextAtWithMarginWithFiltering((12 * pin) + 1, 11, XRES_FRAMEBUFFER, "CPU-", getPaletteEntry(0xFF999999), '0' + pin);

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
                            funcName, getPaletteEntry(0xFF999999));
                }
            }
        }

        drawTextAt(1, 2, "Pointer:", getPaletteEntry(0xFF999999));

        holdingDisk = getHoldingDisk();

        if (holdingDisk != 0xFF) {
            drawTextAt(19, 2, functionNames[holdingDisk], getPaletteEntry(0xFF999999));
        } else {
            drawTextAt(19, 2, "NULL", getPaletteEntry(0xFF999999));
        }
    }
}

void HackingScreen_unloadStateCallback(enum EGameMenuState newState) {
    (void)newState;
#ifdef PAGE_FLIP_ANIMATION
    enableSmoothMovement = wasSmoothMovementPreviouslyEnabled;
#endif
}

enum EGameMenuState HackingScreen_tickCallback(enum ECommand cmd, void* data) {
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
