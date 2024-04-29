#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#include <unistd.h>
#endif


#include "Common.h"
#include "FixP.h"
#include "Enums.h"
#include "Vec.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "CActor.h"
#include "Mesh.h"
#include "Renderer.h"
#include "Dungeon.h"
#include "Core.h"
#include "Derelict.h"
#include "MapWithCharKey.h"
#include "SoundSystem.h"
#include "UI.h"

#ifndef SHORT_VIEW_ANGLE
#define ANGLE_TURN_THRESHOLD 45
#define ANGLE_TURN_STEP 5
#else
#define ANGLE_TURN_THRESHOLD 5
#define ANGLE_TURN_STEP 1
#endif

int turning = 0;
int leanX = 0;
int leanY = 0;

int showPromptToAbandonMission = FALSE;
int needsToRedrawHUD = FALSE;

const char *AbandonMission_Title = "Abandon game?";
const char *AbandonMission_options[6] = {"Continue", "End game"};
const enum EGameMenuState AbandonMission_navigation[2] = {kResumeCurrentState, kMainMenu};
const int AbandonMission_count = 2;

void Crawler_initStateCallback(enum EGameMenuState tag) {
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

void recenterView(void) {
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

void Crawler_repaintCallback(void) {

    needsToRedrawVisibleMeshes = TRUE;

    if (showPromptToAbandonMission) {
        int optionsHeight = 8 * (AbandonMission_count);
        turnStep = turnTarget;

        /* The dithered filter on top of the 3D rendering*/
        fillRect(0, 0, XRES_FRAMEBUFFER, YRES_FRAMEBUFFER, getPaletteEntry(0xFF000000), TRUE);

        drawWindowWithOptions((XRES_FRAMEBUFFER / 8) - biggestOption - 3,
                              ((YRES_FRAMEBUFFER / 8) + 1) - (optionsHeight / 8) - 3,
                              biggestOption + 2,
                              (optionsHeight / 8) + 2,
                              AbandonMission_Title,
                              AbandonMission_options,
                              AbandonMission_count,
                              cursorPosition);
    } else {

        if (currentPresentationState == kRoomTransitioning) {
            renderRoomTransition();
        } else if (currentPresentationState == kWaitingForInput) {

            renderTick(30);

            recenterView();
        }
    }
}

enum EGameMenuState Crawler_tickCallback(enum ECommand cmd, long delta) {

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
        return kResumeCurrentState;
    }

    if (timeUntilNextState != kNonExpiringPresentationState) {
        timeUntilNextState -= delta;
    }

    if (currentPresentationState == kWaitingForInput) {
        /* Not sure why this is here? */
        if (cmd == kCommandFire4) {
            needsToRedrawHUD = TRUE;
        }

        loopTick(cmd);

        return kResumeCurrentState;
    }

    return kResumeCurrentState;
}

void Crawler_unloadStateCallback(enum EGameMenuState newState) {

    if (newState != kBackToGame &&
        newState != kHackingGame) {
        clearTextures();
        clearTileProperties();
    }
}
