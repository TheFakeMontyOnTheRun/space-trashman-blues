#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#include <unistd.h>
#endif

#include "FixP.h"
#include "Enums.h"
#include "Vec.h"
#include "CActor.h"
#include "MapWithCharKey.h"
#include "Common.h"
#include "Vec.h"
#include "LoadBitmap.h"
#include "Core.h"
#include "Engine.h"
#include "MapWithCharKey.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"
#include "VisibilityStrategy.h"
#include "SoundSystem.h"
#include "Core.h"
#include "Derelict.h"
#include "Dungeon.h"

FixP_t kCameraYDeltaPlayerDeath;
FixP_t kCameraYSpeedPlayerDeath;
const char *thisMissionName;
int16_t thisMissionNameLen;
int showPromptToAbandonMission = FALSE;
extern size_t biggestOption;
int needToRedrawHUD = FALSE;
const char *AbandonMission_Title = "Abandon game?";
const char *AbandonMission_options[6] = {"Continue", "End game"};
int AbandonMission_navigation[2] = {-1, kMainMenu};
int AbandonMission_count = 2;
extern struct GameSnapshot gameSnapshot;

void Crawler_initStateCallback(int32_t tag) {
    int c;
    dirtyLineY0 = 0;
    dirtyLineY1 = YRES_FRAMEBUFFER;

    if (tag == kPlayGame) {
        initStation();
        currentPresentationState = kAppearing;
        timeUntilNextState = kDefaultPresentationStateInterval;
        gameTicks = 0;
        enteredThru = 0;
        memset(&gameSnapshot, 0, sizeof(struct GameSnapshot));
    } else {
        currentPresentationState = kWaitingForInput;
        timeUntilNextState = 0;
    }

    kCameraYDeltaPlayerDeath = Div(intToFix(9), intToFix(10));

    kCameraYSpeedPlayerDeath = Div(intToFix(1), intToFix(10));
    showPromptToAbandonMission = FALSE;

    biggestOption = strlen(AbandonMission_Title);

    for (c = 0; c < AbandonMission_count; ++c) {
        size_t len = strlen(AbandonMission_options[c]);

        if (len > biggestOption) {
            biggestOption = len;
        }
    }

    playerHeight = 0;
    playerHeightTarget = kCameraYDeltaPlayerDeath;
    playerHeightChangeRate = 0;

    thisMissionName = getRoomDescription();
    thisMissionNameLen = (int16_t) (strlen(thisMissionName));

    currentBackgroundBitmap = loadBitmap("pattern.img");


    if (tag == kPlayGame) {
        initRoom(getPlayerRoom());
    }
}

void Crawler_initialPaintCallback() {
    fill(0, 0, XRES_FRAMEBUFFER, YRES_FRAMEBUFFER, 0, FALSE);

    fill(11 * 8 - 1, 12 * 8, 18 * 8 + 3, 8, 255, FALSE);
    drawRect(11 * 8 - 1, 11 * 8 - 1, 18 * 8 + 2, 8 + 2, 255);

    drawTextAt(13, 13, "Loading", 0);
    drawTextAt(13, 12, "Please wait...", 255);

    needToRedrawHUD = TRUE;
    needsToRedrawVisibleMeshes = TRUE;
    flipRenderer();
}

void Crawler_repaintCallback() {



    if (showPromptToAbandonMission) {
        int c;
        int optionsHeight = 8 * (AbandonMission_count);
        turnStep = turnTarget;
        drawRepeatBitmap(0, 0, XRES_FRAMEBUFFER, YRES_FRAMEBUFFER, currentBackgroundBitmap);

        fill(0, 0, XRES_FRAMEBUFFER, YRES_FRAMEBUFFER, 0, TRUE);

        fill(XRES_FRAMEBUFFER - (biggestOption * 8) - 8 - 16, YRES_FRAMEBUFFER - optionsHeight - 8 - 16,
             (biggestOption * 8) + 16, optionsHeight + 16, 0, TRUE);

        fill(XRES_FRAMEBUFFER - (biggestOption * 8) - 16 - 16, YRES_FRAMEBUFFER - optionsHeight - 16 - 16,
             (biggestOption * 8) + 16, optionsHeight + 16, 255, FALSE);

        drawRect(XRES_FRAMEBUFFER - (biggestOption * 8) - 16 - 16,
                 YRES_FRAMEBUFFER - optionsHeight - 16 - 16, (biggestOption * 8) + 16,
                 optionsHeight + 16, 0);

        if (AbandonMission_Title != NULL) {

            fill((40 - biggestOption - 2 - 2) * 8,
                 ((26 - AbandonMission_count) - 2 - 1 - 2) * 8,
                 (biggestOption + 2) * 8, 8, 0, FALSE);

            drawTextAt(40 - biggestOption - 2, (26 - AbandonMission_count) - 4,
                       AbandonMission_Title, 255);
        }

        for (c = 0; c < AbandonMission_count; ++c) {

            int isCursor = (cursorPosition == c)
                           && ((currentPresentationState == kConfirmInputBlink1)
                               || (currentPresentationState == kConfirmInputBlink3)
                               || (currentPresentationState == kConfirmInputBlink5)
                               || (currentPresentationState == kWaitingForInput));

            if (isCursor) {
                fill(XRES_FRAMEBUFFER - (biggestOption * 8) - 16 - 8 - 8,
                     (YRES_FRAMEBUFFER - optionsHeight) + (c * 8) - 8 - 8,
                     (biggestOption * 8) + 16, 8, 0, FALSE);
            }

            drawTextAt(
                    40 - biggestOption - 2, (26 - AbandonMission_count) + c - 2,
                    &AbandonMission_options[c][0],
                    isCursor ? 200 : 0);
        }
    } else {

        if  (currentPresentationState == kRoomTransitioning ) {

            struct Vec3 center;

            if ( !enableSmoothMovement ) {
                currentPresentationState = kWaitingForInput;
                zCameraOffset = xCameraOffset = yCameraOffset = 0;
                needToRedrawHUD = TRUE;
                return;
            }

            xCameraOffset = yCameraOffset = 0;

            fill(0, 0, XRES + 1, YRES, 0, 0);



            center.mY = 0;
            center.mZ = intToFix(3);
            center.mX = -intToFix(3);
            drawColumnAt( center, intToFix(3), nativeTextures[1], MASK_LEFT, 0, 1);



            center.mY = 0;
            center.mX = intToFix(3);
            drawColumnAt( center, intToFix(3), nativeTextures[1], MASK_RIGHT, 0, 1);


            center.mZ = intToFix(2);
            center.mX = -intToFix(1);

            center.mY = intToFix(4) - zCameraOffset;
            drawBillboardAt( center, &nativeTextures[0]->rotations[0][0], intToFix(1), 32);

            center.mX = intToFix(1);
            drawBillboardAt( center, &nativeTextures[0]->rotations[0][0], intToFix(1), 32);

            center.mY = intToFix(2) - zCameraOffset;

            center.mZ = intToFix(3);
            drawCeilingAt( center, nativeTextures[0], 0);

            center.mZ = intToFix(2);

            center.mY = intToFix(3) - zCameraOffset;

            center.mX = -intToFix(1);
            drawBillboardAt( center, &nativeTextures[0]->rotations[0][0], intToFix(1), 32);

            center.mX = intToFix(1);
            drawBillboardAt( center, &nativeTextures[0]->rotations[0][0], intToFix(1), 32);


            center.mY = intToFix(6) - zCameraOffset;

            center.mX = -intToFix(1);
            drawBillboardAt( center, &nativeTextures[0]->rotations[0][0], intToFix(1), 32);

            center.mX = intToFix(1);
            drawBillboardAt( center, &nativeTextures[0]->rotations[0][0], intToFix(1), 32);



            center.mX = -intToFix(1);

            center.mY = intToFix(2) - zCameraOffset;

            center.mZ = intToFix(3);
            drawCeilingAt( center, nativeTextures[0], 0);

            drawTextAtWithMargin(((XRES / 8) / 2) - (thisMissionNameLen / 2), 1, XRES, thisMissionName, 255);

            zCameraOffset -= Div(intToFix(1), intToFix(16));
            if (zCameraOffset == 0 ) {
                int chanceForRandomBattle = getRoom(getPlayerRoom())->chanceOfRandomBattle;
                int diceRoll;

                //tmp
                diceRoll = 0xFF;

                if (diceRoll <= chanceForRandomBattle ) {
                    currentPresentationState = kEnteringRandomBattle;
                } else {
                    currentPresentationState = kWaitingForInput;
                    needToRedrawHUD = TRUE;
                }
            }
            return;
        }

        if (currentPresentationState == kWaitingForInput ) {
            renderTick(30);
        }
    }
}

enum EGameMenuState Crawler_tickCallback(enum ECommand cmd, long delta) {
    int returnCode;

    if (kEnteringRandomBattle == currentPresentationState ) {
        return kRandomBattle;
    }

    if (showPromptToAbandonMission) {

        timeUntilNextState -= delta;

        if (timeUntilNextState <= 0) {

            switch (currentPresentationState) {
                case kAppearing:
                    timeUntilNextState = 500;
                    currentPresentationState = kWaitingForInput;
                    break;
                case kWaitingForInput:
                    break;
                case kRoomTransitioning:
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
                    playSound(MENU_SELECTION_CHANGE_SOUND);
                    cursorPosition = (cursorPosition - 1);

                    if (cursorPosition >= AbandonMission_count) {
                        cursorPosition = AbandonMission_count - 1;
                    }

                    if (cursorPosition < 0 ) {
                        cursorPosition = 0;
                    }

                    break;
                case kCommandDown:
                    playSound(MENU_SELECTION_CHANGE_SOUND);
                    cursorPosition =
                            (uint8_t) ((cursorPosition + 1) % AbandonMission_count);

                    break;
                case kCommandFire1:
                case kCommandFire2:
                case kCommandFire3:

                    if (cursorPosition == 0) {
                        showPromptToAbandonMission = FALSE;
                        needsToRedrawVisibleMeshes = TRUE;
                        currentPresentationState = kAppearing;
                        return kResumeCurrentState;
                    }
                    timeUntilNextState = 0;
                    nextNavigationSelection = AbandonMission_navigation[cursorPosition];
                    currentPresentationState = kConfirmInputBlink1;
                    break;
            }

            return kResumeCurrentState;
        }


        switch(cmd) {
            case kCommandUp:
                playSound(MENU_SELECTION_CHANGE_SOUND);
                cursorPosition = (cursorPosition - 1);

                if (cursorPosition >= AbandonMission_count) {
                    cursorPosition = AbandonMission_count - 1;
                }
                break;
            case kCommandDown:
                playSound(MENU_SELECTION_CHANGE_SOUND);
                cursorPosition =
                        (uint8_t) ((cursorPosition + 1) % AbandonMission_count);

                break;
            case kCommandBack:
                showPromptToAbandonMission = TRUE;
                break;

            case kCommandFire1:
            case kCommandFire2:
            case kCommandFire3:

                if (cursorPosition == 0) {
                    showPromptToAbandonMission = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
                    currentPresentationState = kAppearing;
                    return kResumeCurrentState;
                }
                timeUntilNextState = 0;
                nextNavigationSelection = AbandonMission_navigation[cursorPosition];
                currentPresentationState = kConfirmInputBlink1;
                break;
        }

        return kResumeCurrentState;
    }

    if (cmd == kCommandBack) {
        showPromptToAbandonMission = TRUE;
        timeUntilNextState = 0;
        dirtyLineY0 = 0;
        dirtyLineY1 = YRES_FRAMEBUFFER;
        return kMenuStateUnchanged;
    }

    if (timeUntilNextState != kNonExpiringPresentationState) {
        timeUntilNextState -= delta;
    }

    if (currentPresentationState == kWaitingForInput) {
        if (cmd == kCommandFire4) {
            dirtyLineY0 = 0;
            dirtyLineY1 = YRES_FRAMEBUFFER;
            needToRedrawHUD = TRUE;
        }

        returnCode = loopTick(cmd);


        if (returnCode == kCrawlerGameOver) {
            playerHeightChangeRate = kCameraYSpeedPlayerDeath;
            currentPresentationState = kFade;
            timeUntilNextState = kDefaultPresentationStateInterval;
        }

        return kResumeCurrentState;
    }

    if (timeUntilNextState <= 0) {

        switch (currentPresentationState) {
            case kAppearing:
                currentPresentationState = kWaitingForInput;
                timeUntilNextState = kNonExpiringPresentationState;
                break;
            case kFade:
                return kResumeCurrentState;
            case kWaitingForInput:
                return kMenuStateUnchanged;
            case kConfirmInputBlink1:
            case kConfirmInputBlink2:
            case kConfirmInputBlink3:
            case kConfirmInputBlink4:
            case kConfirmInputBlink5:
            case kConfirmInputBlink6:
                break;
        }

        needsToRedrawVisibleMeshes = TRUE;
    }

    return kMenuStateUnchanged;
}

void Crawler_unloadStateCallback() {
    if (currentBackgroundBitmap != NULL) {
        releaseBitmap(currentBackgroundBitmap);
        currentBackgroundBitmap = NULL;
    }
}