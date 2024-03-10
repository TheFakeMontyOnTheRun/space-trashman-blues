#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#endif

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "Enums.h"
#include "FixP.h"
#include "Vec.h"
#include "Engine.h"
#include "CActor.h"
#include "Mesh.h"
#include "Renderer.h"
#include "UI.h"
#include "Core.h"
#include "Derelict.h"
#include "SoundSystem.h"
#include "HackingMinigameRules.h"

int wasSmoothMovementPreviouslyEnabled;

const char *functionNames[5] = {
        "???",
        "writeB",
        "snprintf",
        "hackLogin",
        "LoadDBReg"
};

void HackingScreen_initStateCallback(enum EGameMenuState tag) {

    /* The middle pin */
    cursorPosition = 1;

    initHackingMinigame();

    wasSmoothMovementPreviouslyEnabled = enableSmoothMovement;
    enableSmoothMovement = FALSE;
}

void HackingScreen_repaintCallback(void) {
    uint8_t isSelected;
    int pin;
    int pinPosition = 0;
    uint8_t holdingDisk;
    drawWindow(1, 1, XRES_FRAMEBUFFER / 8, 15, "Disassembly: CONTROLLER.PRG (stack)");

#ifndef AGS
    drawTextAt(6 + (12 * 0), 11, "CPU0",
               cursorPosition == 0 ? getPaletteEntry(0xFF999999) : getPaletteEntry(0xFF000000));
    drawTextAt(6 + (12 * 1), 11, "CPU1",
               cursorPosition == 1 ? getPaletteEntry(0xFF999999) : getPaletteEntry(0xFF000000));
    drawTextAt(6 + (12 * 2), 11, "CPU2",
               cursorPosition == 2 ? getPaletteEntry(0xFF999999) : getPaletteEntry(0xFF000000));

    for (pin = 0; pin < 3; ++pin)
#else
        pin = cursorPosition;
#endif
    {
        int disk;
        int isCursorOnThisPin = cursorPosition == pin;

#ifndef AGS
        pinPosition = pin;
#else
        pinPosition = 0;

        char buffer[8];
        sprintf(buffer, "CPU%d", cursorPosition);
        drawTextAt( 6 + (12), 11, buffer, 128);
#endif

        for (disk = 0; disk < 5; ++disk) {

            int diskIndex = getPositionForPin(pin, disk);
            const char *funcName = (disk >= getDisksForPin(pin)) ? NULL : functionNames[diskIndex];

            if (isCursorOnThisPin) {
                isSelected = getPaletteEntry(0xFF999999);
            } else if (diskIndex == 3) {
                isSelected = getPaletteEntry(0xFF444444);
            } else {
                isSelected = 0;
            }

            if (isAccessToSafeGranted()) {
                isSelected = getPaletteEntry(0xFF00AA00);
            }

            drawTextAt(13 * (pinPosition) + 1, 4 + (5 - disk), "|", isSelected);

            if (funcName) {
                drawTextAt(13 * (pinPosition) + 2, 4 + (5 - disk), funcName, isSelected);
            }

            drawTextAt(13 * (pinPosition) + 1, 10, "-------------", isSelected);
        }
    }

    drawTextAt(1, 2, "register pointer:", getPaletteEntry(0xFF000000));

    holdingDisk = getHoldingDisk();

    if (holdingDisk != 0xFF) {
        drawTextAt(19, 2, functionNames[holdingDisk], getPaletteEntry(0xFF999999));
    } else {
        drawTextAt(19, 2, "NULL", getPaletteEntry(0xFF999999));
    }
}

enum EGameMenuState HackingScreen_tickCallback(enum ECommand cmd, long delta) {

    uint8_t holdingDisk = getHoldingDisk();

    if (isHackingMinigameCompleted()) {
        grantAccessToSafe();
    }

    switch (cmd) {
        case kCommandLeft:
            if (cursorPosition > 0) {
                cursorPosition--;
            }
            turnTarget = turnStep;
            break;
        case kCommandRight:
            if (cursorPosition < 2) {
                cursorPosition++;
            }
            turnTarget = turnStep;
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
            break;

        default:
            break;
    }

    return kResumeCurrentState;
}

void HackingScreen_unloadStateCallback(enum EGameMenuState newState) {
    enableSmoothMovement = wasSmoothMovementPreviouslyEnabled;
}
