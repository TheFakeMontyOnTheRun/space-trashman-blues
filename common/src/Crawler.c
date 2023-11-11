#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#include "Win32Int.h"
#else

#include <stdint.h>
#include <unistd.h>

#endif

#include "FixP.h"
#include "Enums.h"
#include "Vec.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "CActor.h"
#include "Mesh.h"
#include "CRenderer.h"
#include "Dungeon.h"
#include "Common.h"
#include "Core.h"
#include "Derelict.h"
#include "MapWithCharKey.h"
#include "SoundSystem.h"
#include "UI.h"

#define ANGLE_TURN_THRESHOLD 40
#define ANGLE_TURN_STEP 5

int turning = 0;
int leanX = 0;
int leanY = 0;

int showPromptToAbandonMission = FALSE;
int needToRedrawHUD = FALSE;

const char *AbandonMission_Title = "Abandon game?";
const char *AbandonMission_options[6] = {"Continue", "End game"};
const int AbandonMission_navigation[2] = {-1, kMainMenu};
const int AbandonMission_count = 2;

void Crawler_initStateCallback(int32_t tag) {
    int c;

    if (tag == kPlayGame) {
        initStation();
        timeUntilNextState = kDefaultPresentationStateInterval;
        gameTicks = 0;
        enteredThru = 0;
        memFill(&gameSnapshot, 0, sizeof(struct GameSnapshot));
    } else {
        timeUntilNextState = 0;
    }

    showPromptToAbandonMission = FALSE;

    biggestOption = strlen(AbandonMission_Title);

    for (c = 0; c < AbandonMission_count; ++c) {
        size_t len = strlen(AbandonMission_options[c]);

        if (len > biggestOption) {
            biggestOption = len;
        }
    }

    playerHeight = 0;
    playerHeightChangeRate = 0;

    if (tag == kPlayGame) {
        clearMap(&tileProperties);
        initRoom(getPlayerRoom());
    }
}

void Crawler_initialPaintCallback() {
}

void recenterView() {
    if (leanX > 0 && !turning) {
        leanX -= ANGLE_TURN_STEP;
    }

    if (leanX < 0 && !turning) {
        leanX += ANGLE_TURN_STEP;
    }

    if (leanY > 0) {
        leanY -= ANGLE_TURN_STEP;
    }

    if (leanY < 0) {
        leanY += ANGLE_TURN_STEP;
    }

    if (leanX > 0 && turning) {
        if (leanX < ANGLE_TURN_THRESHOLD) {
            leanX += ANGLE_TURN_STEP;
        } else if (leanX == ANGLE_TURN_THRESHOLD) {
            visibilityCached = FALSE;
            mBufferedCommand = kCommandRight;
            leanX = -ANGLE_TURN_THRESHOLD;
            turning = 0;
        }
    }

    if (leanX < 0 && turning) {
        if (leanX > -ANGLE_TURN_THRESHOLD) {
            leanX -= ANGLE_TURN_STEP;
        } else if (leanX == -ANGLE_TURN_THRESHOLD) {
            visibilityCached = FALSE;
            mBufferedCommand = kCommandLeft;
            leanX = ANGLE_TURN_THRESHOLD;
            turning = 0;
        }
    }
}

void Crawler_repaintCallback() {

    visibilityCached = FALSE;
    needsToRedrawVisibleMeshes = TRUE;

    if (showPromptToAbandonMission) {
        int c;
        int optionsHeight = 8 * (AbandonMission_count);
        turnStep = turnTarget;

        /* The dithered filter on top of the 3D rendering*/
        fill(0, 0, XRES_FRAMEBUFFER, YRES_FRAMEBUFFER, getPaletteEntry(0xFF000000), TRUE);

        drawWindow((XRES_FRAMEBUFFER / 8) - biggestOption - 3,
                   (YRES_FRAMEBUFFER / 8) - AbandonMission_count - 3,
                   (biggestOption) + 2,
                   AbandonMission_count + 2,
                   AbandonMission_Title);

        for (c = 0; c < AbandonMission_count; ++c) {

            int isCursor = (cursorPosition == c)
                           && ((currentPresentationState == kWaitingForInput));

            if (isCursor) {
                fill(XRES_FRAMEBUFFER - (biggestOption * 8) - 16 - 8 - 8,
                     (YRES_FRAMEBUFFER - optionsHeight) + (c * 8) - 8 - 8,
                     (biggestOption * 8) + 16, 8, getPaletteEntry(0xFF000000), FALSE);
            }

            drawTextAt(
                    40 - biggestOption - 2, (26 - AbandonMission_count) + c - 2,
                    &AbandonMission_options[c][0],
                    isCursor ? getPaletteEntry(0xFFAAAAAA) : getPaletteEntry(0xFF000000));
        }
    } else {

        if (currentPresentationState == kRoomTransitioning) {
            renderRoomTransition();
        } else if (currentPresentationState == kWaitingForInput) {

            renderTick(30);

#ifndef PLAYSTATION2
            recenterView();
#endif
        }
    }
}

enum EGameMenuState Crawler_tickCallback(enum ECommand cmd, long delta) {
    int returnCode;

    if (showPromptToAbandonMission) {

        switch (cmd) {
            case kCommandBack:
                return kMainMenu;
            case kCommandUp:
                playSound(MENU_SELECTION_CHANGE_SOUND);
                --cursorPosition;
                break;
            case kCommandDown:
                playSound(MENU_SELECTION_CHANGE_SOUND);
                ++cursorPosition;
                break;
            case kCommandFire1:
            case kCommandFire2:
            case kCommandFire3:

                if (cursorPosition == 0) {
                    showPromptToAbandonMission = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
                    return kResumeCurrentState;
                }
                timeUntilNextState = 0;
                return AbandonMission_navigation[cursorPosition];
        }

        if (cursorPosition >= AbandonMission_count) {
            cursorPosition = AbandonMission_count - 1;
        }

        if (cursorPosition < 0) {
            cursorPosition = 0;
        }

        return kResumeCurrentState;
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
        /* Not sure why this is here? */
        if (cmd == kCommandFire4) {
            needsToRedrawHUD = TRUE;
        }

        returnCode = loopTick(cmd);

        return kResumeCurrentState;
    }

    return kMenuStateUnchanged;
}

void Crawler_unloadStateCallback(int32_t newState) {

    if (newState != kBackToGame &&
        newState != kInspectItem &&
        newState != kHackingGame) {
        clearTextures();
        clearTileProperties();
    }
}
