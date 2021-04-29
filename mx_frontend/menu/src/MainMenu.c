#include <string.h>

#include <stdint.h>
#include <unistd.h>

#include "FixP.h"
#include "Vec.h"
#include "Enums.h"
#include "CActor.h"
#include "MapWithCharKey.h"
#include "Common.h"
#include "Vec.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"
#include "VisibilityStrategy.h"
#include "UI.h"
#include "SoundSystem.h"

#if !defined(ANDROID) && !defined(__EMSCRIPTEN__)
const char *MainMenu_options[4] = {
        "Play game", "Credits", "Help", "Quit"};

int32_t MainMenu_nextStateNavigation[4] = {
        kPlayGame, kCredits, kHelp,
        kQuit};

const int kMainMenuOptionsCount = 4;
#else
const char *MainMenu_options[3] = {
        "Play game", "Credits", "Help"};

int32_t MainMenu_nextStateNavigation[3] = {
        kPlayGame, kCredits, kHelp};

const int kMainMenuOptionsCount = 3;
#endif

extern size_t biggestOption;

struct Bitmap *logoBitmap;
struct Bitmap *logo2Bitmap;

void MainMenu_initStateCallback(int32_t tag) {
    int c;
    cursorPosition = 0;

    if (currentBackgroundBitmap != NULL) {
        releaseBitmap(currentBackgroundBitmap);
    }

    currentBackgroundBitmap = loadBitmap("pattern.img");
    logoBitmap = loadBitmap("title.img");
    logo2Bitmap = loadBitmap("logo.img");
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
    playSound(7);
}

void MainMenu_initialPaintCallback() {

    //drawRepeatBitmap(0, 32, 320, 200, currentBackgroundBitmap);
    fill(0, 0, 319, 199, getPaletteEntry(0xFF6cb1a3), 0);
}

void MainMenu_repaintCallback(void) {
    int16_t c;

    uint8_t optionsHeight = 8 * kMainMenuOptionsCount;

    if (currentPresentationState == kAppearing) {
        int invertedProgression = ((256 - (timeUntilNextState)) / 32) * 32;
        int movementX =
                lerpInt(0, (biggestOption * 8), invertedProgression, 256);
        int movementY = lerpInt(0, optionsHeight, invertedProgression, 256);
        int sizeX = lerpInt(0, (biggestOption * 8), invertedProgression, 256);
        int sizeY = lerpInt(0, optionsHeight + 8, invertedProgression, 256);

        if (timeUntilNextState > 256) {
            return;
        }


        drawRect(320 - movementX - 8 - 24 - ((biggestOption * 8) / 2)
                 + (sizeX / 2),
                 200 - movementY - 8 - 16 - 8 - ((optionsHeight + 8) / 2)
                 + (sizeY / 2),
                 sizeX, sizeY, 0);
        return;
    }

    drawBitmap(0, 0, logoBitmap, 0);

    drawBitmap(118, 45, logo2Bitmap, 1);

    drawWindow(40 - biggestOption - 3, 25 - 4 - (optionsHeight / 8), biggestOption + 2, (optionsHeight / 8) + 2,
               "Episode 0");

    for (c = 0; c < kMainMenuOptionsCount; ++c) {

        int isCursor = (cursorPosition == c)
                       && ((currentPresentationState == kConfirmInputBlink1)
                           || (currentPresentationState == kConfirmInputBlink3)
                           || (currentPresentationState == kConfirmInputBlink5)
                           || (currentPresentationState == kWaitingForInput));

        if (isCursor) {
            fill((uint16_t) (320 - (biggestOption * 8)) - 8 - 24,
                 (200 - optionsHeight) + (c * 8) - 24,
                 (biggestOption * 8) + 16, 8, 0, FALSE);
        }

        drawTextAt(40 - biggestOption + 1 - 3,
                   (26 - kMainMenuOptionsCount) + c - 3,
                   &MainMenu_options[c][0], isCursor ? 200 : 0);
    }

}

enum EPresentationState MainMenu_tickCallback(enum ECommand cmd, long delta) {

    timeUntilNextState -= delta;

    if (timeUntilNextState <= 0) {

        switch (currentPresentationState) {
            case kAppearing:
                timeUntilNextState = 250;
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
            case kCommandUp:
                playSound(MENU_SELECTION_CHANGE_SOUND);
                cursorPosition = cursorPosition - 1;

                if (cursorPosition > (kMainMenuOptionsCount - 1)) {
                    cursorPosition = (kMainMenuOptionsCount - 1);
                }
                break;
            case kCommandDown:
                playSound(MENU_SELECTION_CHANGE_SOUND);
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

    return kResumeCurrentState;
}

void MainMenu_unloadStateCallback() {
    releaseBitmap(logoBitmap);
    releaseBitmap(currentBackgroundBitmap);
    currentBackgroundBitmap = NULL;
}
