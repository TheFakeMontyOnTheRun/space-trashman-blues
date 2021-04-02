#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "FixP.h"
#include "Enums.h"
#include "Common.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "CRenderer.h"
#include "UI.h"

#if !defined(ANDROID) && !defined(__EMSCRIPTEN__)
const char *MainMenu_options[4] = {
        "Agent log in", "Credits", "Help", "Quit"};

int32_t MainMenu_nextStateNavigation[4] = {
        kInterrogate_Sofia, kCredits, kHelp,
        kQuit};

const int kMainMenuOptionsCount = 4;
#else
const char *MainMenu_options[3] = {
        "Agent log in", "Credits", "Help"};

int32_t MainMenu_nextStateNavigation[3] = {
        kInterrogate_Sofia, kCredits, kHelp};

const int kMainMenuOptionsCount = 3;
#endif

extern size_t biggestOption;

struct Bitmap *logoBitmap;

int32_t MainMenu_initStateCallback(int32_t tag, void *data) {
    int c;
    cursorPosition = 0;

    if (currentBackgroundBitmap != NULL) {
        releaseBitmap(currentBackgroundBitmap);
    }

    currentBackgroundBitmap = loadBitmap("pattern.img");
    logoBitmap = loadBitmap("mistral.img");
    currentPresentationState = kAppearing;
    timeUntilNextState = 500;

    biggestOption = 0;

    for (c = 0; c < kMainMenuOptionsCount; ++c) {
        const char *MainMenu_option = MainMenu_options[c];

        size_t len = strlen(MainMenu_option);

        if (len > biggestOption) {
            biggestOption = len;
        }
    }
    
    dirtyLineY0 = 0;
    dirtyLineY1 = 200;

    return 0;
}

void MainMenu_initialPaintCallback() {
	drawRepeatBitmap(0, 0, 320, 200, currentBackgroundBitmap);

    drawImageWindow(2, 2, 21, 22, "Invisible affairs", logoBitmap);
}

void MainMenu_repaintCallback(void) {
    int16_t c;

    uint8_t optionsHeight = 8 * kMainMenuOptionsCount;
/*
    if (currentPresentationState == kAppearing) {
        int invertedProgression = ((256 - (timeUntilNextState)) / 32) * 32;
        int size = lerpInt(0, 160, invertedProgression, 256);
        int movementX =
                lerpInt(0, (biggestOption * 8), invertedProgression, 256);
        int movementY = lerpInt(0, optionsHeight, invertedProgression, 256);
        int sizeX = lerpInt(0, (biggestOption * 8), invertedProgression, 256);
        int sizeY = lerpInt(0, optionsHeight + 8, invertedProgression, 256);

        if (timeUntilNextState > 256) {
            return;
        }

        drawRect(8 + 80 - size / 2, 16 + 80 - size / 2, size, size, 0);

        drawRect(320 - movementX - 8 - 24 - ((biggestOption * 8) / 2)
                 + (sizeX / 2),
                 200 - movementY - 8 - 16 - 8 - ((optionsHeight + 8) / 2)
                 + (sizeY / 2),
                 sizeX, sizeY, 0);
        return;
    }
*/

    drawWindow(40 - biggestOption - 3, 25 - 4 - (optionsHeight / 8), biggestOption + 2, (optionsHeight / 8) + 2,
               "Play Game");

    for (c = 0; c < kMainMenuOptionsCount; ++c) {

        int isCursor = (cursorPosition == c)
                       && ((currentPresentationState == kConfirmInputBlink1)
                           || (currentPresentationState == kConfirmInputBlink3)
                           || (currentPresentationState == kConfirmInputBlink5)
                           || (currentPresentationState == kWaitingForInput));

        if (isCursor) {
            fill((uint16_t) (320 - (biggestOption * 8)) - 8 - 24,
                 (200 - optionsHeight) + (c * 8) - 24,
                 (biggestOption * 8) + 16, 8,
#ifdef AGA5BPP
                 7
#else
                 0
#endif
                 , FALSE);
        }

        drawTextAt(40 - biggestOption + 1 - 3,
                   (26 - kMainMenuOptionsCount) + c - 3,
                   &MainMenu_options[c][0], 4);
    }
    
    
    if (stateTick > 5 ) {
        dirtyLineY0 = (200 - optionsHeight) + (0 * 8) - 24;
        dirtyLineY1 = (200 - optionsHeight) + (kMainMenuOptionsCount * 8) - 24;
    }


}

int32_t MainMenu_tickCallback(int32_t tag, void *data) {

    long delta = *((long *) data);

    timeUntilNextState -= delta;

    if (timeUntilNextState <= 0) {

        switch (currentPresentationState) {
            case kAppearing:
                timeUntilNextState = 100;
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
                timeUntilNextState = 100;
                currentPresentationState =
                        (enum EPresentationState) ((int) currentPresentationState + 1);
                break;
            case kFade:
                return nextNavigationSelection;
        }
    }

    if (currentPresentationState == kWaitingForInput) {

        switch (tag) {
            case kCommandUp:
                cursorPosition = cursorPosition - 1;

                if (cursorPosition > (kMainMenuOptionsCount - 1)) {
                    cursorPosition = (kMainMenuOptionsCount - 1);
                }
                break;
            case kCommandDown:
                cursorPosition =
                        (uint8_t) ((cursorPosition + 1) % kMainMenuOptionsCount);
                break;
            case kCommandFire1:
            case kCommandFire2:
            case kCommandFire3:

                nextNavigationSelection =
                        MainMenu_nextStateNavigation[cursorPosition];
                currentPresentationState = kConfirmInputBlink1;
                break;
        }
    }

    return -1;
}

void MainMenu_unloadStateCallback() {
    releaseBitmap(logoBitmap);
    releaseBitmap(currentBackgroundBitmap);
    currentBackgroundBitmap = NULL;
}
