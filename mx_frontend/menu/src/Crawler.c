#include <string.h>

#include <stdint.h>
#include <unistd.h>

#include "FixP.h"
#include "Enums.h"
#include "Vec.h"
#include "CActor.h"
#include "MapWithCharKey.h"
#include "Common.h"
#include "Vec.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "MapWithCharKey.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"
#include "VisibilityStrategy.h"
#include "SoundSystem.h"

#include "Derelict.h"

FixP_t kCameraYDeltaPlayerDeath;

FixP_t kCameraYSpeedPlayerDeath;

int loopTick(enum ECommand cmd);

void initRoom(int room);

void renderTick(long ms);

int getPlayerLocation();

char *thisMissionName;
int16_t thisMissionNameLen;
int showPromptToAbandonMission = FALSE;
extern size_t biggestOption;

const char *AbandonMission_Title = "Abandon mission?";
const char *AbandonMission_options[6] = {"Continue", "End game"};
int AbandonMission_navigation[2] = {-1, kMainMenu};
int AbandonMission_count = 2;

int32_t Crawler_initStateCallback(int32_t tag) {
    int c = 0;

    if (tag == kPlayGame) {
        initStation();
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

    currentPresentationState = kAppearing;
    timeUntilNextState = kDefaultPresentationStateInterval;
    currentBackgroundBitmap = loadBitmap("pattern.img");
    

    if (tag == kPlayGame) {
        initRoom(getPlayerRoom());
    }

    return 0;
}

void Crawler_initialPaintCallback() {
    drawRepeatBitmap(0, 32, 320, 200, currentBackgroundBitmap);


    fill(11 * 8, 12 * 8, 18 * 8, 8, 255, FALSE);
    fill(11 * 8, 11 * 8, 18 * 8, 8, 0, FALSE);

    drawRect(11 * 8, 11 * 8, 18 * 8, 16, 0);

    drawTextAt(13, 13, "Decoding mission", 0);
    drawTextAt(13, 12, "Please wait...", 255);


    needsToRedrawVisibleMeshes = TRUE;
    flipRenderer();
}

void Crawler_repaintCallback() {

    if (needsToRedrawVisibleMeshes && (currentPresentationState == kAppearing)) {
        drawRepeatBitmap(0, 32, 320, 200, currentBackgroundBitmap);

        drawTextAt(2, 18, "Mission Status", 255);

        fill(256, 8, 64, 120, 255, FALSE);
        fill(256, 0, 64, 8, 0, FALSE);
        drawTextAt(34, 1, "Map", 255);
    }

    if (showPromptToAbandonMission) {
        int c = 0;
        int optionsHeight = 8 * (AbandonMission_count);

        drawRepeatBitmap(0, 32, 320, 200, currentBackgroundBitmap);

        fill(0, 0, 320, 200, 0, TRUE);

        fill(320 - (biggestOption * 8) - 8 - 16, 200 - optionsHeight - 8 - 16,
             (biggestOption * 8) + 16, optionsHeight + 16, 0, TRUE);

        fill(320 - (biggestOption * 8) - 16 - 16, 200 - optionsHeight - 16 - 16,
             (biggestOption * 8) + 16, optionsHeight + 16, 255, FALSE);

        drawRect(320 - (biggestOption * 8) - 16 - 16,
                 200 - optionsHeight - 16 - 16, (biggestOption * 8) + 16,
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
                fill(320 - (biggestOption * 8) - 16 - 8 - 8,
                     (200 - optionsHeight) + (c * 8) - 8 - 8,
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
            center.mX = center.mY = 0;
            center.mZ = intToFix(1);
            fill(0, 0, 256, 200, 0, 0);
            drawTextAt(16 - (thisMissionNameLen / 2), 1, thisMissionName, 255);
            drawBillboardAt( center, &nativeTextures[0]->rotations[0][0], intToFix(1), 32);
            zCameraOffset -= Div(intToFix(1), intToFix(16));
            if (zCameraOffset == 0 ) {
                currentPresentationState = kWaitingForInput;
            }
            return;
        }
        
        
        renderTick(30);

        if (currentPresentationState == kAppearing) {
            drawTextAt(16 - (thisMissionNameLen / 2), 10, thisMissionName, 255);
        }
    }
}

int32_t Crawler_tickCallback(int32_t tag, void *data) {
    enum ECommand cmd = (enum ECommand) (tag);
    long delta = *((int *) (data));
    int returnCode;

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
                case kCommandFire1:
                case kCommandFire2:
                case kCommandFire3:

                    if (cursorPosition == 0) {
                        showPromptToAbandonMission = FALSE;
                        needsToRedrawVisibleMeshes = TRUE;
                        currentPresentationState = kAppearing;
                        return -1;
                    }
                    timeUntilNextState = 0;
                    nextNavigationSelection = AbandonMission_navigation[cursorPosition];
                    currentPresentationState = kConfirmInputBlink1;
                    break;
            }
        }

        return -1;
    }

    if (cmd == kCommandBack) {
        showPromptToAbandonMission = TRUE;
        timeUntilNextState = 0;
        return kMenuStateUnchanged;
    }

    if (timeUntilNextState != kNonExpiringPresentationState) {
        timeUntilNextState -= delta;
    }

    if (currentPresentationState == kWaitingForInput) {
        returnCode = loopTick(cmd);

        switch (returnCode) {
            case kCrawlerGameOver: {
                playerHeightChangeRate = kCameraYSpeedPlayerDeath;
                currentPresentationState = kFade;
                timeUntilNextState = kDefaultPresentationStateInterval;
            }
                break;
        }

        return -1;
    }

    if (timeUntilNextState <= 0) {

        switch (currentPresentationState) {
            case kAppearing:
                currentPresentationState = kWaitingForInput;
                timeUntilNextState = kNonExpiringPresentationState;
                break;
            case kFade:
                return -1;
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
