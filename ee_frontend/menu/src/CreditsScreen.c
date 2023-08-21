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

const char *CreditsScreen_options[1] = {"Back"};

int32_t CreditsScreen_nextStateNavigation[1] = {
        kMainMenu,
};

int16_t CreditsScreen_optionsCount = 1;
extern char textBuffer[40 * 25];
struct Bitmap *monty;

void CreditsScreen_initStateCallback(int32_t tag) {

    struct StaticBuffer textFile;
    cursorPosition = 0;
    currentPresentationState = kAppearing;
    timeUntilNextState = 500;
    memFill (&textBuffer[0], ' ', 40 * 25);
    mainText = &textBuffer[0];
    memFill (&textBuffer[0], 0, (40 * 25));
	textFile = loadBinaryFileFromPath("Help.txt");
    memCopyToFrom(&textBuffer[0], textFile.data, textFile.size);

    CreditsScreen_optionsCount = 1;

    monty = loadBitmap("monty.img");
    disposeDiskBuffer(textFile);
}

void CreditsScreen_initialPaintCallback(void) {
}

void CreditsScreen_repaintCallback(void) {
    int lines = countLines();
    int c;
    int optionsHeight = 8 * (CreditsScreen_optionsCount);
    size_t len = strlen(CreditsScreen_options[0]);

	fill(0, 0, 319, 199, getPaletteEntry(0xFF6cb1a3), 0);

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
                 lerp320, lerpLines, getPaletteEntry(0xFF000000));

        drawRect(8 + lerping32, 128 + lerping32, lerpoSixtyFooooooouuuuuuur,
                 lerpoSixtyFooooooouuuuuuur, getPaletteEntry(0xFF000000));

        drawRect(80 + lerping32, 128 + lerping32, lerpoSixtyFooooooouuuuuuur,
                 lerpoSixtyFooooooouuuuuuur, getPaletteEntry(0xFF000000));

        drawRect(152 + lerping32, 128 + lerping32, lerpoSixtyFooooooouuuuuuur,
                 lerpoSixtyFooooooouuuuuuur, getPaletteEntry(0xFF000000));

        drawRect(320 - 16 - (len * 8) - 16 + (len * 8 / 2) - lerpLen / 2,
                 200 - optionsHeight - 16 - 16 + optionsHeight / 2
                 - lerpOptionsHeight / 2,
                 lerpLen + 16, lerpOptionsHeight + 16, getPaletteEntry(0xFF000000));

        return;
    }

    fill(0, (lines + 3) * 8, 320, 8, getPaletteEntry(0xFF000000), TRUE);
    fill(8 + 8, 128 + 8, 64, 64, getPaletteEntry(0xFF000000), TRUE);
    fill(80 + 8, 128 + 8, 64, 64, getPaletteEntry(0xFF000000), TRUE);
    fill(152 + 8, 128 + 8, 64, 64, getPaletteEntry(0xFF000000), TRUE);

    if (mainText != NULL) {

        fill(0, 0, 320, (lines + 3) * 8, getPaletteEntry(0xFFFFFFFF), FALSE);

        drawRect(0, 0, 320, (lines + 3) * 8, getPaletteEntry(0xFF000000));
        fill(0, 0, 320, 8, getPaletteEntry(0xFF000000), FALSE);
        drawTextAt(2, 1, "Credits", getPaletteEntry(0xFFFFFFFF));
        drawTextAt(1, 3, mainText, getPaletteEntry(0xFF000000));
    }

    fill(8, 128, 64, 64, getPaletteEntry(0xFFFFFFFF), FALSE);

	drawBitmap(8, 128, monty, TRUE);

    drawRect(8, 128, 64, 64, getPaletteEntry(0xFF000000));

    fill(8, 128, 64, 8, getPaletteEntry(0xFF000000), FALSE);
    drawTextAt(3, 17, "Monty", getPaletteEntry(0xFFFFFFFF));

    fill(80, 128, 64, 64, getPaletteEntry(0xFFFFFFFF), FALSE);
    drawRect(80, 128, 64, 64, getPaletteEntry(0xFF000000));

    fill(80, 128, 64, 8, getPaletteEntry(0xFF000000), FALSE);

    fill(152, 128, 64, 64, getPaletteEntry(0xFFFFFFFF), FALSE);
    drawRect(152, 128, 64, 64, getPaletteEntry(0xFF000000));

    fill(152, 128, 64, 8, getPaletteEntry(0xFF000000), FALSE);

    fill(320 - (len * 8) - 8 - 16, 200 - optionsHeight - 8 - 16,
         (len * 8) + 16, optionsHeight + 16, getPaletteEntry(0xFF000000), TRUE);

    fill(320 - (len * 8) - 16 - 16, 200 - optionsHeight - 16 - 16,
         (len * 8) + 16, optionsHeight + 16, getPaletteEntry(0xFFFFFFFF), FALSE);

    drawRect(320 - (len * 8) - 16 - 16, 200 - optionsHeight - 16 - 16,
             (len * 8) + 16, optionsHeight + 16, getPaletteEntry(0xFF000000));

    fill((40 - len - 2 - 2) * 8,
         ((26 - CreditsScreen_optionsCount) - 2 - 1 - 2) * 8,
         (len + 2) * 8, 8, getPaletteEntry(0xFF000000), FALSE);

    for (c = 0; c < CreditsScreen_optionsCount; ++c) {

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

        drawTextAt(40 - len - 2, (26 - CreditsScreen_optionsCount) + c - 2,
                   &CreditsScreen_options[c][0], isCursor ? getPaletteEntry(0xFFFFFFFF) : getPaletteEntry(0xFF000000));
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
			case kCommandBack:
                nextNavigationSelection =
                        CreditsScreen_nextStateNavigation[cursorPosition];
                currentPresentationState = kConfirmInputBlink1;
                break;
        }
    }

    return kResumeCurrentState;
}

void CreditsScreen_unloadStateCallback(int32_t newState) {
}
