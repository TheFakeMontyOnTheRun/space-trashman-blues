#ifdef WIN32
#include "Win32Int.h"
#else

#include <stdint.h>
#include <unistd.h>

#endif

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "Enums.h"
#include "FixP.h"
#include "Vec.h"
#include "Engine.h"
#include "Common.h"
#include "PackedFileReader.h"
#include "LoadBitmap.h"
#include "CActor.h"
#include "Mesh.h"
#include "CRenderer.h"
#include "UI.h"

const char *HelpScreen_options[1] = {"Back"};

const enum EGameMenuState HelpScreen_nextStateNavigation[1] = {
        kMainMenu,
};

const int16_t HelpScreen_optionsCount = 1;

void HelpScreen_initStateCallback(int32_t tag) {
#ifndef AGS
    struct StaticBuffer textFile = loadBinaryFileFromPath("Help.txt");
#else
    struct StaticBuffer textFile = loadBinaryFileFromPath("HelpAGS.txt");
#endif
    
    cursorPosition = 0;
    currentPresentationState = kAppearing;
    timeUntilNextState = 500;
    memFill(textBuffer, ' ', TEXT_BUFFER_SIZE);

    mainText = textBuffer;
    memFill(textBuffer, 0, TEXT_BUFFER_SIZE);
    memCopyToFrom(textBuffer, (void *) textFile.data, textFile.size);
    disposeDiskBuffer(textFile);
}

void HelpScreen_initialPaintCallback(void) {
}

void HelpScreen_repaintCallback(void) {
    int c;
    int lines;
    size_t len = strlen(HelpScreen_options[0]);
    int optionsHeight = 8 * (HelpScreen_optionsCount);

    lines = 18;

    fill(0, 0, 319, 199, getPaletteEntry(0xFF6cb1a3), FALSE);

    if (currentPresentationState == kAppearing) {

        int invertedProgression = ((256 - (timeUntilNextState)) / 32) * 32;
        int lerp320 = lerpInt(0, (XRES_FRAMEBUFFER - 1), invertedProgression, 256);
        int lerpLines = lerpInt(0, (lines + 3) * 8, invertedProgression, 256);
        int lerpLen8 = lerpInt(0, (len * 8), invertedProgression, 256);
        int lerpOptionsHeight =
                lerpInt(0, optionsHeight, invertedProgression, 256);

        if (timeUntilNextState > 256) {
            return;
        }

        drawRect((XRES_FRAMEBUFFER / 2) - lerp320 / 2, ((lines + 3) * 8) / 2 - lerpLines / 2,
                 lerp320, lerpLines, getPaletteEntry(0xFF000000));

        drawRect(XRES_FRAMEBUFFER - (len * 8) - 16 - 16 + (len * 8) / 2 - lerpLen8 / 2,
                 YRES_FRAMEBUFFER - optionsHeight - 16 - 16 + optionsHeight / 2
                 - lerpOptionsHeight / 2,
                 lerpLen8 + 16, lerpOptionsHeight + 16, getPaletteEntry(0xFF000000));

        return;
    }

    if (mainText != NULL) {
        drawTextWindow(1, 1, (XRES_FRAMEBUFFER / 8) - 2, lines + 3, "Help", mainText);
    }

    drawWindow((XRES_FRAMEBUFFER / 8) - len - 3, ((YRES_FRAMEBUFFER / 8) + 1) - (optionsHeight / 8) - 3, len + 2,
               (optionsHeight / 8) + 2, "");

    for (c = 0; c < HelpScreen_optionsCount; ++c) {

        int isCursor = (cursorPosition == c)
                       && ((currentPresentationState == kConfirmInputBlink1)
                           || (currentPresentationState == kConfirmInputBlink3)
                           || (currentPresentationState == kConfirmInputBlink5)
                           || (currentPresentationState == kWaitingForInput));

        if (isCursor) {
            fill(XRES_FRAMEBUFFER - (len * 8) - 16 - 8 - 8,
                 (YRES_FRAMEBUFFER - optionsHeight) + (c * 8) - (8 * 1),
                 (len * 8) + 16,
                 8,
                 getPaletteEntry(0xFF000000), FALSE);
        }

        drawTextAt((XRES_FRAMEBUFFER / 8) - len - 2, (((YRES_FRAMEBUFFER / 8) + 1) - HelpScreen_optionsCount) + c - 1,
                   &HelpScreen_options[c][0], isCursor ? getPaletteEntry(0xFFFFFFFF) : getPaletteEntry(0xFF000000));
    }
}

enum EGameMenuState HelpScreen_tickCallback(enum ECommand cmd, long delta) {

    timeUntilNextState -= delta;

    if (timeUntilNextState <= 0) {

        switch (currentPresentationState) {
            case kAppearing:
                timeUntilNextState = 500;
                currentPresentationState = kWaitingForInput;
                break;
            case kWaitingForInput:
                break;
            case kConfirmInputBlink1:
            case kConfirmInputBlink2:
            case kConfirmInputBlink3:
            case kConfirmInputBlink4:
            case kConfirmInputBlink5:
            case kConfirmInputBlink6:
                timeUntilNextState = 250;
                currentPresentationState =
                        (enum EPresentationState) ((int) currentPresentationState + 1);
                break;
            case kFade:
                return nextNavigationSelection;
        }
    }

    if (currentPresentationState == kWaitingForInput) {

        switch (cmd) {
            case kCommandBack:
                return kMainMenu;
            case kCommandUp:
                cursorPosition = (cursorPosition - 1);

                if (cursorPosition >= HelpScreen_optionsCount) {
                    cursorPosition = HelpScreen_optionsCount - 1;
                }

                if (cursorPosition < 0) {
                    cursorPosition = 0;
                }

                break;
            case kCommandDown:
                cursorPosition =
                        (uint8_t) ((cursorPosition + 1) % HelpScreen_optionsCount);
                break;
            case kCommandFire1:
            case kCommandFire2:
            case kCommandFire3:

                nextNavigationSelection =
                        HelpScreen_nextStateNavigation[cursorPosition];
                currentPresentationState = kConfirmInputBlink1;
                break;
        }
    }

    return kResumeCurrentState;
}

void HelpScreen_unloadStateCallback(int32_t newState) {
}