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
#include "CPackedFileReader.h"
#include "CRenderer.h"
#include "CTile3DProperties.h"
#include "FixP.h"
#include "LoadBitmap.h"
#include "MapWithCharKey.h"
#include "VisibilityStrategy.h"
#include "UI.h"

const char *BattleScreen_options[2] = {"Attack", "Defend"};

struct Bitmap *enemySprite;

enum EGameMenuState BattleScreen_nextStateNavigation[2] = {
        kAttackOnBattle, kDefendOnBattle
};

int16_t Battlecreen_optionsCount = 2;


void BattleScreen_initStateCallback(int32_t tag) {
    dirtyLineY0 = 0;
    dirtyLineY1 = 200;
    cursorPosition = 1;
    currentPresentationState = kAppearing;
    timeUntilNextState = 500;
    currentBackgroundBitmap = loadBitmap("pattern.img");
    enemySprite = loadBitmap("enemy.img");
    enableSmoothMovement = FALSE;
}

void BattleScreen_initialPaintCallback(void) {
    fill(0, 0, XRES, 200, 128, FALSE );
}

void BattleScreen_repaintCallback(void) {

    int c = 0;
    size_t len = 7;
    int optionsHeight = 8 * (Battlecreen_optionsCount);

    drawTextAt( 15, 13, "Mutant crab", 128);
    redrawHUD();

    drawBitmap( 32, 32, enemySprite, TRUE);

    if (currentPresentationState == kAppearing) {


        int invertedProgression = ((256 - (timeUntilNextState)) / 32) * 32;
        int lerpPositionX = lerpInt( 20, 1, invertedProgression, 256);
        int lerpPositionWidth = lerpInt( 0, 22, invertedProgression, 256);
        int lerpPositionY = lerpInt( 20, 15, invertedProgression, 256);
        int lerpPositionHeight = lerpInt( 0, 10, invertedProgression, 256);


        if (timeUntilNextState > 256) {
            return;
        }


        drawRect(lerpPositionX * 8,
                 lerpPositionY * 8,
                 lerpPositionWidth * 8,
                 lerpPositionHeight * 8, 0);

        return;
    }


    drawWindow(1, 15, 22, 10, "Actions");

    for (c = 0; c < Battlecreen_optionsCount; ++c) {

        int isCursor = (cursorPosition == c)
                       && ((currentPresentationState == kConfirmInputBlink1)
                           || (currentPresentationState == kConfirmInputBlink3)
                           || (currentPresentationState == kConfirmInputBlink5)
                           || (currentPresentationState == kWaitingForInput));

        if (isCursor) {
            fill(8,
                 (200 - optionsHeight) + (c * 8) - 8 - 8, (len * 8) + 16, 8,
                 0, FALSE);
        }

        drawTextAt( 2, (26 - Battlecreen_optionsCount) + c - 2,
                   &BattleScreen_options[c][0], isCursor ? 255 : 0);
    }
}

enum EGameMenuState BattleScreen_tickCallback(enum ECommand cmd, long delta) {

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
            case kCommandLeft:
            case kCommandUp:
                if (cursorPosition > 0 ) {
                    cursorPosition--;
                }
                turnTarget = turnStep;
                break;
            case kCommandRight:
            case kCommandDown:
                if (cursorPosition < (Battlecreen_optionsCount - 1) ) {
                    cursorPosition++;
                }
                turnTarget = turnStep;
                break;


            case kCommandBack:
                break;
            case kCommandFire1:
                break;

            default:
                break;
        }
    }

    return kResumeCurrentState;
}

void BattleScreen_unloadStateCallback() {
    if (currentBackgroundBitmap != NULL) {
        releaseBitmap(currentBackgroundBitmap);
        currentBackgroundBitmap = NULL;
    }
}
