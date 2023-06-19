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
#include "Core.h"
#include "Engine.h"
#include "CActor.h"
#include "Common.h"
#include "PackedFileReader.h"
#include "CRenderer.h"
#include "CTile3DProperties.h"
#include "FixP.h"
#include "LoadBitmap.h"
#include "MapWithCharKey.h"
#include "VisibilityStrategy.h"
#include "UI.h"

const char *CreditsScreen_options[1] = {"Back"};

int32_t CreditsScreen_nextStateNavigation[1] = {
        kMainMenu,
};

const int16_t CreditsScreen_optionsCount = 1;
extern char *textBuffer;
struct Bitmap *monty;

void CreditsScreen_initStateCallback(int32_t tag) {

    struct StaticBuffer textFile;
    cursorPosition = 0;
    currentPresentationState = kAppearing;
    timeUntilNextState = 500;
    memFill (textBuffer, ' ', 40 * 25);

    mainText = textBuffer;
    memFill (textBuffer, 0, (40 * 25));
	textFile = loadBinaryFileFromPath("Credits.txt");
    memCopyToFrom(textBuffer, (void*)textFile.data, textFile.size);
    disposeDiskBuffer(textFile);


    monty = loadBitmap("monty.img");
}

void CreditsScreen_initialPaintCallback(void) {
    dirtyLineY0 = 0;
    dirtyLineY1 = YRES_FRAMEBUFFER;
    fill(0, 0, (XRES_FRAMEBUFFER-1), (YRES_FRAMEBUFFER-1), getPaletteEntry(0xFF6cb1a3), 0);
}

void CreditsScreen_repaintCallback(void) {
    int lines = countLines();
    int c;
    int optionsHeight = 8 * (CreditsScreen_optionsCount);
    size_t len = max( strlen("Options"), strlen(CreditsScreen_options[0]));

    if (currentPresentationState == kAppearing) {

        int invertedProgression = ((256 - (timeUntilNextState)) / 32) * 32;
        int lerpoSixtyFooooooouuuuuuur =
                lerpInt(0, 64, invertedProgression, 256);
        int lerping32 = 32 - (lerpoSixtyFooooooouuuuuuur / 2);
        int lerp320 = lerpInt(0, XRES_FRAMEBUFFER, invertedProgression, 256);
        int lerpLines = lerpInt(0, (lines + 3) * 8, invertedProgression, 256);
        int lerpLen = lerpInt(0, len * 8, invertedProgression, 256);
        int lerpOptionsHeight =
                lerpInt(0, optionsHeight, invertedProgression, 256);
        if (timeUntilNextState > 256) {
            return;
        }

        drawRect(160 - lerp320 / 2, (((lines + 3) * 8) / 2) - lerpLines / 2,
                 lerp320, lerpLines, 0);

        drawRect(8 + lerping32, (YRES_FRAMEBUFFER - 72) + lerping32, lerpoSixtyFooooooouuuuuuur,
                 lerpoSixtyFooooooouuuuuuur, 0);

        drawRect(XRES_FRAMEBUFFER - 16 - (len * 8) - 16 + (len * 8 / 2) - lerpLen / 2,
				 YRES_FRAMEBUFFER - optionsHeight - 16 - 16 + optionsHeight / 2
                 - lerpOptionsHeight / 2,
                 lerpLen + 16, lerpOptionsHeight + 16, 0);

        return;
    }

    fill(0, (lines + 3) * 8, XRES_FRAMEBUFFER, 8, 0, TRUE);
    fill(8 + 8, (YRES_FRAMEBUFFER - 72) + 8, 64, 64, 0, TRUE);

    if (mainText != NULL) {

        fill(0, 0, XRES_FRAMEBUFFER, (lines + 3) * 8, 255, FALSE);

        drawRect(0, 0, XRES_FRAMEBUFFER, (lines + 3) * 8, 0);
        fill(0, 0, XRES_FRAMEBUFFER, 8, 0, FALSE);
        drawTextAt(2, 1, "Credits", 255);
        drawTextAt(1, 3, mainText, 0);
    }

    drawImageWindow(2,  (YRES_FRAMEBUFFER - 72) / 8, monty->width / 8, (monty->height / 8) + 1, "Monty", monty);

    drawWindow(
            (XRES_FRAMEBUFFER / 8) - (int) len - 3,
            (YRES_FRAMEBUFFER / 8) - 3 - (optionsHeight / 8),
            len + 2, (optionsHeight / 8) + 2,
            "Options");


    for (c = 0; c < CreditsScreen_optionsCount; ++c) {

        int isCursor = (cursorPosition == c)
                       && ((currentPresentationState == kConfirmInputBlink1)
                           || (currentPresentationState == kConfirmInputBlink3)
                           || (currentPresentationState == kConfirmInputBlink5)
                           || (currentPresentationState == kWaitingForInput));

        if (isCursor) {
            fill(XRES_FRAMEBUFFER - (len * 8) - 16 - 8 - 8,
                 (YRES_FRAMEBUFFER - optionsHeight) + (c * 8) - 8 - 8, (len * 8) + 16, 8,
                 0, FALSE);
        }

        drawTextAt(40 - len - 2, (26 - CreditsScreen_optionsCount) + c - 2,
                   &CreditsScreen_options[c][0], isCursor ? 255 : 0);
    }
}

enum EGameMenuState CreditsScreen_tickCallback(enum ECommand cmd, long delta) {

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
                        (enum EPresentationState) (((int) currentPresentationState) + 1);
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

                if (cursorPosition >= CreditsScreen_optionsCount) {
                    cursorPosition = CreditsScreen_optionsCount - 1;
                }

                if (cursorPosition < 0 ) {
                    cursorPosition = 0;
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

    return kResumeCurrentState;
}

void CreditsScreen_unloadStateCallback() {
    releaseBitmap(monty);
    monty = NULL;
}
