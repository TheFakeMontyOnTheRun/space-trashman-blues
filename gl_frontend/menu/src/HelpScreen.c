#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "Enums.h"
#include "FixP.h"
#include "Vec.h"
#include "Core.h"
#include "Engine.h"
#include "CActor.h"
#include "Common.h"
#include "PackedFileReader.h"
#include "LoadBitmap.h"
#include "CRenderer.h"
#include "CTile3DProperties.h"
#include "FixP.h"
#include "MapWithCharKey.h"
#include "VisibilityStrategy.h"
#include "UI.h"

const char *HelpScreen_options[1] = {"Back"};

enum EGameMenuState HelpScreen_nextStateNavigation[1] = {
        kMainMenu,
};

int16_t HelpScreen_optionsCount = 1;
extern char textBuffer[40 * 25];

void HelpScreen_initStateCallback(int32_t tag) {
    struct StaticBuffer textFile = loadBinaryFileFromPath("Help.txt");

    cursorPosition = 0;
    currentPresentationState = kAppearing;
    timeUntilNextState = 500;
    memset (&textBuffer[0], ' ', 40 * 25);

    currentBackgroundBitmap = loadBitmap("pattern.img");

    mainText = &textBuffer[0];
    memset (&textBuffer[0], 0, (40 * 25));
    memcpy(&textBuffer[0], textFile.data, textFile.size);

    HelpScreen_optionsCount = 1;
}

void HelpScreen_initialPaintCallback(void) {
}

void HelpScreen_repaintCallback(void) {
    int c;
    int lines;
    size_t len = strlen(HelpScreen_options[0]);
    int optionsHeight = 8 * (HelpScreen_optionsCount);

    lines = countLines();
    
    if (currentBackgroundBitmap != NULL) {
        drawRepeatBitmap(0, 0, 320, 200, currentBackgroundBitmap);
    }

    if (currentPresentationState == kAppearing) {

        int invertedProgression = ((256 - (timeUntilNextState)) / 32) * 32;
        int lerp320 = lerpInt(0, 319, invertedProgression, 256);
        int lerpLines = lerpInt(0, (lines + 3) * 8, invertedProgression, 256);
        int lerpLen8 = lerpInt(0, (len * 8), invertedProgression, 256);
        int lerpOptionsHeight =
                lerpInt(0, optionsHeight, invertedProgression, 256);

        if (timeUntilNextState > 256) {
            return;
        }

        drawRect(160 - lerp320 / 2, ((lines + 3) * 8) / 2 - lerpLines / 2,
                 lerp320, lerpLines, 0);

        drawRect(320 - (len * 8) - 16 - 16 + (len * 8) / 2 - lerpLen8 / 2,
                 200 - optionsHeight - 16 - 16 + optionsHeight / 2
                 - lerpOptionsHeight / 2,
                 lerpLen8 + 16, lerpOptionsHeight + 16, 0);

        return;
    }

    if (mainText != NULL) {
        drawTextWindow(1, 1, 40, lines + 3, "Help", mainText);
    }

    drawWindow(40 - len - 3, 25 - (optionsHeight / 8) - 3, len + 2, (optionsHeight / 8) + 2, "");

    for (c = 0; c < HelpScreen_optionsCount; ++c) {

        int isCursor = (cursorPosition == c)
                       && ((currentPresentationState == kConfirmInputBlink1)
                           || (currentPresentationState == kConfirmInputBlink3)
                           || (currentPresentationState == kConfirmInputBlink5)
                           || (currentPresentationState == kWaitingForInput));

        if (isCursor) {
            fill(320 - (len * 8) - 16 - 8 - 8,
                 (200 - optionsHeight) + (c * 8) - 8 - 8, (len * 8) + 16, 8,
                 getPaletteEntry(0xFF000000), FALSE);
        }

        drawTextAt(40 - len - 2, (26 - HelpScreen_optionsCount) + c - 2,
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

                if (cursorPosition < 0 ) {
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

void HelpScreen_unloadStateCallback() {
    if (currentBackgroundBitmap != NULL) {
        releaseBitmap(currentBackgroundBitmap);
        currentBackgroundBitmap = NULL;
    }
}