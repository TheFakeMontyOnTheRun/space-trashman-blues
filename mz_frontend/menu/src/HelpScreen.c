#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "Enums.h"
#include "FixP.h"
#include "Engine.h"
#include "Common.h"
#include "CPackedFileReader.h"
#include "CRenderer.h"
#include "FixP.h"
#include "LoadBitmap.h"
#include "UI.h"

extern const char *mainText;

const char *HelpScreen_options[1] = {"Back"};

int32_t HelpScreen_nextStateNavigation[1] = {
        kMainMenu,
};

int16_t HelpScreen_optionsCount = 1;
extern char textBuffer[40 * 25];

int32_t HelpScreen_initStateCallback(int32_t tag, void *data) {
    size_t fileSize = sizeOfFile("Help.txt");
    FILE *fileInput = openBinaryFileFromPath("Help.txt");

    cursorPosition = 0;
    currentPresentationState = kAppearing;
    timeUntilNextState = 500;
    memset (&textBuffer[0], ' ', 40 * 25);

    currentBackgroundBitmap = loadBitmap("pattern.img");

    mainText = &textBuffer[0];
    memset (&textBuffer[0], 0, (40 * 25));
    assert (fread(&textBuffer[0], fileSize, 1, fileInput));
    fclose(fileInput);

    HelpScreen_optionsCount = 1;
    dirtyLineY0 = 0;
    dirtyLineY1 = 200;
    return 0;
}

extern int countLines();

void HelpScreen_initialPaintCallback(void) {

    if (currentBackgroundBitmap != NULL) {
        drawRepeatBitmap(0, 32, 320, 200, currentBackgroundBitmap);
    }
}

void HelpScreen_repaintCallback(void) {
    int c = 0;
    int lines;
    size_t len = strlen(HelpScreen_options[0]);
    int16_t optionsHeight = 8 * (HelpScreen_optionsCount);

    lines = countLines();

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
#ifdef AGA5BPP
                    7
#else
                 0
#endif
                 , FALSE);
        }

        drawTextAt(40 - len - 2, (26 - HelpScreen_optionsCount) + c - 2,
                   &HelpScreen_options[c][0],

#ifdef AGA5BPP
                isCursor ? 0 : 7
#else
                   isCursor ? 8 : 0
#endif

                   );
    }
}

int32_t HelpScreen_tickCallback(int32_t tag, void *data) {

    long delta = *((long *) data);

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
            case kCommandBack:
                return kMainMenu;
            case kCommandUp:
                cursorPosition = (cursorPosition - 1);

                if (cursorPosition >= HelpScreen_optionsCount) {
                    cursorPosition = HelpScreen_optionsCount - 1;
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

    return -1;
}

void HelpScreen_unloadStateCallback() {
    if (currentBackgroundBitmap != NULL) {
        releaseBitmap(currentBackgroundBitmap);
        currentBackgroundBitmap = NULL;
    }
}
