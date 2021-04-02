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

extern const char *mainText;

const char *CreditsScreen_options[1] = {"Back"};

int32_t CreditsScreen_nextStateNavigation[1] = {
        kMainMenu,
};

int16_t CreditsScreen_optionsCount = 1;
extern char textBuffer[40 * 25];
struct Bitmap *monty;
struct Bitmap *belle;

int32_t CreditsScreen_initStateCallback(int32_t tag, void *data) {
    size_t fileSize = sizeOfFile("Credits.txt");
    FILE *fileInput = openBinaryFileFromPath("Credits.txt");

    cursorPosition = 0;
    currentPresentationState = kAppearing;
    timeUntilNextState = 500;
    memset (&textBuffer[0], ' ', 40 * 25);

    currentBackgroundBitmap = loadBitmap("pattern.img");

    mainText = &textBuffer[0];
    memset (&textBuffer[0], 0, (40 * 25));
    assert (fread(&textBuffer[0], fileSize, 1, fileInput));
    fclose(fileInput);

    CreditsScreen_optionsCount = 1;

    monty = loadBitmap("monty.img");

    belle = loadBitmap("belle.img");

    dirtyLineY0 = 0;
    dirtyLineY1 = 200;
    return 0;
}

void CreditsScreen_initialPaintCallback(void) {
    if (currentBackgroundBitmap != NULL) {
        drawRepeatBitmap(0, 32, 320, 200, currentBackgroundBitmap);
    }
}

void CreditsScreen_repaintCallback(void) {
    int lines = countLines();
    int c = 0;
    int16_t optionsHeight = 8 * (CreditsScreen_optionsCount);
    size_t len = strlen(CreditsScreen_options[0]);

    if (currentPresentationState == kAppearing) {

        int invertedProgression = ((256 - (timeUntilNextState)) / 32) * 32;
        int lerpoSixtyFooooooouuuuuuur =
                lerpInt(0, 64, invertedProgression, 256);
        int lerping32 = 32 - (lerpoSixtyFooooooouuuuuuur / 2);
        int lerp320 = lerpInt(0, 320, invertedProgression, 256);
        int lerpLines = lerpInt(0, (lines + 3) * 8, invertedProgression, 256);
        int lerpLen = lerpInt(0, len * 8, invertedProgression, 256);
        int lerpOptionsHeight =
                lerpInt(0, optionsHeight, invertedProgression, 256);
        if (timeUntilNextState > 256) {
            return;
        }

        drawRect(160 - lerp320 / 2, (((lines + 3) * 8) / 2) - lerpLines / 2,
                 lerp320, lerpLines, 0);

        drawRect(8 + lerping32, 128 + lerping32, lerpoSixtyFooooooouuuuuuur,
                 lerpoSixtyFooooooouuuuuuur, 0);

        drawRect(152 + lerping32, 128 + lerping32, lerpoSixtyFooooooouuuuuuur,
                 lerpoSixtyFooooooouuuuuuur, 0);

        drawRect(320 - 16 - (len * 8) - 16 + (len * 8 / 2) - lerpLen / 2,
                 200 - optionsHeight - 16 - 16 + optionsHeight / 2
                 - lerpOptionsHeight / 2,
                 lerpLen + 16, lerpOptionsHeight + 16, 0);

        return;
    }

    fill(0, (lines + 3) * 8, 320, 8, 0, TRUE);
    fill(8 + 8, 128 + 8, 64, 64, 0, TRUE);
    fill(152 + 8, 128 + 8, 64, 64, 0, TRUE);

    if (mainText != NULL) {

        fill(0, 0, 320, (lines + 3) * 8, 15, FALSE);

        drawRect(0, 0, 320, (lines + 3) * 8, 0);
        fill(0, 0, 320, 8, 0, FALSE);
        drawTextAt(2, 1, "Credits", 4);
        drawTextAt(1, 3, mainText,
#ifdef AGA5BPP
                7
#else
                   0
#endif
        );
    }

    fill(8, 128, 64, 64, 7, FALSE);
    drawBitmap(8, 128, monty, TRUE);
    drawRect(8, 128, 64, 64, 0);

    fill(8, 128, 64, 8, 0, FALSE);
    drawTextAt(3, 17, "Monty", 4);

    fill(152, 128, 64, 64, 7, FALSE);
    drawBitmap(152, 128, belle, TRUE);
    drawRect(152, 128, 64, 64, 0);

    fill(152, 128, 64, 8, 0, FALSE);
    drawTextAt(21, 17, "Belle", 4);

    fill(320 - (len * 8) - 8 - 16, 200 - optionsHeight - 8 - 16,
         (len * 8) + 16, optionsHeight + 16, 0, TRUE);

    fill(320 - (len * 8) - 16 - 16, 200 - optionsHeight - 16 - 16,
         (len * 8) + 16, optionsHeight + 16, 15, FALSE);

    drawRect(320 - (len * 8) - 16 - 16, 200 - optionsHeight - 16 - 16,
             (len * 8) + 16, optionsHeight + 16, 0);

    fill((40 - len - 2 - 2) * 8,
         ((26 - CreditsScreen_optionsCount) - 2 - 1 - 2) * 8,
         (len + 2) * 8, 8, 0, FALSE);

    for (c = 0; c < CreditsScreen_optionsCount; ++c) {

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

        drawTextAt(40 - len - 2, (26 - CreditsScreen_optionsCount) + c - 2,
                   &CreditsScreen_options[c][0],


#ifdef AGA5BPP
                isCursor ? 4 : 7
#else
                   isCursor ? 4 : 0
#endif
                   );
    }
}

int32_t CreditsScreen_tickCallback(int32_t tag, void *data) {

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

                if (cursorPosition >= CreditsScreen_optionsCount) {
                    cursorPosition = CreditsScreen_optionsCount - 1;
                }

                break;
            case kCommandDown:
                cursorPosition =
                        (uint8_t) ((cursorPosition + 1) % CreditsScreen_optionsCount);
                break;
            case kCommandFire1:
            case kCommandFire2:
            case kCommandFire3:

                nextNavigationSelection =
                        CreditsScreen_nextStateNavigation[cursorPosition];
                currentPresentationState = kConfirmInputBlink1;
                break;
        }
    }

    return -1;
}

void CreditsScreen_unloadStateCallback() {
    if (currentBackgroundBitmap != NULL) {
        releaseBitmap(currentBackgroundBitmap);
        currentBackgroundBitmap = NULL;

        releaseBitmap(monty);
        releaseBitmap(belle);

        monty = NULL;
        belle = NULL;
    }
}
