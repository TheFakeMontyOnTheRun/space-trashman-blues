#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#include "Win32Int.h"
#else

#include <stdint.h>
#include <unistd.h>

#endif

#ifndef NDS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION

#include <OpenGL/gl.h>

#else
#include <GL/gl.h>
#endif
#else
#include <nds.h>
#include <malloc.h>
#include <stdio.h>
#include <nds/arm9/image.h>
#include <nds/arm9/trig_lut.h>
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

#define ANGLE_TURN_THRESHOLD 40
#define ANGLE_TURN_STEP 5

extern int turning;
extern int leanX;
extern int leanY;

FixP_t kCameraYDeltaPlayerDeath;
FixP_t kCameraYSpeedPlayerDeath;
const char *thisMissionName;
int16_t thisMissionNameLen;
int showPromptToAbandonMission = FALSE;
extern size_t biggestOption;
int needToRedrawHUD = FALSE;
const char *AbandonMission_Title = "Abandon game?";
const char *AbandonMission_options[6] = {"Continue", "End game"};
const int AbandonMission_navigation[2] = {-1, kMainMenu};
const int AbandonMission_count = 2;
extern struct GameSnapshot gameSnapshot;

void Crawler_initStateCallback(int32_t tag) {
    int c;

    if (tag == kPlayGame) {
        initStation();
        currentPresentationState = kAppearing;
        timeUntilNextState = kDefaultPresentationStateInterval;
        gameTicks = 0;
        enteredThru = 0;
        memFill(&gameSnapshot, 0, sizeof(struct GameSnapshot));
        memFill(&nativeTextures[0], 0, sizeof(struct Texture) * TOTAL_TEXTURES);
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

    playerHeight = -intToFix(1);
    playerHeightChangeRate = 0;

    thisMissionName = getRoomDescription();
    thisMissionNameLen = (int16_t) (strlen(thisMissionName));

    if (tag == kPlayGame) {
        clearMap(&tileProperties);
        initRoom(getPlayerRoom());
    }
}

void Crawler_initialPaintCallback() {
    /*
    int textPosY = ((YRES_FRAMEBUFFER / 8) / 2) - 1;

    fill(0, 0, XRES_FRAMEBUFFER, YRES_FRAMEBUFFER, getPaletteEntry(0xFF000000), FALSE);

    fill((XRES_FRAMEBUFFER / 2) - (9 * 8) - 1, textPosY * 8, 18 * 8 + 3, 8, getPaletteEntry(0xFFFFFFFF), FALSE);
    drawRect((XRES_FRAMEBUFFER / 2) - (9 * 8) - 1, (textPosY * 8) - 8 - 1, 18 * 8 + 2, 8 + 2, getPaletteEntry(0xFFFFFFFF));

    drawTextAt(((XRES_FRAMEBUFFER / 8) / 2) - 7, textPosY + 1, "Loading", getPaletteEntry(0xFF000000));
    drawTextAt(((XRES_FRAMEBUFFER / 8) / 2) - 7, textPosY, "Please wait...", getPaletteEntry(0xFFFFFFFF));

    needToRedrawHUD = TRUE;
    needsToRedrawVisibleMeshes = TRUE;
     */
}

void Crawler_repaintCallback() {

    visibilityCached = FALSE;
    needsToRedrawVisibleMeshes = TRUE;

    if (showPromptToAbandonMission) {
        int c;
        int optionsHeight = 8 * (AbandonMission_count);
        turnStep = turnTarget;

        fill(0, 0, XRES_FRAMEBUFFER, YRES_FRAMEBUFFER, getPaletteEntry(0xFF000000), TRUE);

        fill(XRES_FRAMEBUFFER - (biggestOption * 8) - 8 - 16, YRES_FRAMEBUFFER - optionsHeight - 8 - 16,
             (biggestOption * 8) + 16, optionsHeight + 16, getPaletteEntry(0xFF000000), TRUE);

        fill(XRES_FRAMEBUFFER - (biggestOption * 8) - 16 - 16, YRES_FRAMEBUFFER - optionsHeight - 16 - 16,
             (biggestOption * 8) + 16, optionsHeight + 16, getPaletteEntry(0xFFFFFFFF), FALSE);

        drawRect(XRES_FRAMEBUFFER - (biggestOption * 8) - 16 - 16,
                 YRES_FRAMEBUFFER - optionsHeight - 16 - 16, (biggestOption * 8) + 16,
                 optionsHeight + 16, getPaletteEntry(0xFF000000));

        if (AbandonMission_Title != NULL) {

            fill((40 - biggestOption - 2 - 2) * 8,
                 ((26 - AbandonMission_count) - 2 - 1 - 2) * 8,
                 (biggestOption + 2) * 8, 8, getPaletteEntry(0xFF000000), FALSE);

            drawTextAt(40 - biggestOption - 2, (26 - AbandonMission_count) - 4,
                       AbandonMission_Title, getPaletteEntry(0xFFFFFFFF));
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
                     (biggestOption * 8) + 16, 8, getPaletteEntry(0xFF000000), FALSE);
            }

            drawTextAt(
                    40 - biggestOption - 2, (26 - AbandonMission_count) + c - 2,
                    &AbandonMission_options[c][0],
                    isCursor ? getPaletteEntry(0xFFAAAAAA) : getPaletteEntry(0xFF000000));
        }
    } else {

        if (currentPresentationState == kRoomTransitioning) {

            struct Vec3 center;
            FixP_t acc;
            FixP_t bias;
            FixP_t scaled;
            float zOffset;

            if (!enableSmoothMovement) {
                currentPresentationState = kWaitingForInput;
                zCameraOffset = xCameraOffset = yCameraOffset = 0;
                needToRedrawHUD = TRUE;
                return;
            }

            xCameraOffset = yCameraOffset = 0;

#ifndef NDS
            enter2D();
            fill(0, 0, XRES_FRAMEBUFFER, YRES_FRAMEBUFFER, getPaletteEntry(0xFF000000), FALSE);
#endif

            enter3D();
            center.mX = center.mY = 0;
            center.mZ = 1;

            bias = intToFix(128);

            acc = (zCameraOffset);
            scaled = Mul(acc, bias);

            zOffset = (fixToInt(scaled) / 128.0f);
            glTranslatef(0, 0.0f, -zOffset);

            glTranslatef(-3, 0.0f, -3);
            drawColumnAt(center, intToFix(3), nativeTextures[1], MASK_FORCE_LEFT, 0, 1);
            glTranslatef(3, 0.0f, 3);

            glTranslatef(3, 0.0f, -3);
            drawColumnAt(center, intToFix(3), nativeTextures[1], MASK_FORCE_RIGHT, 0, 1);
            glTranslatef(-3, 0.0f, 3);

            center.mY = intToFix(4) - zCameraOffset;
            glTranslatef(-1, 0.0f, -3);
            drawBillboardAt(center, nativeTextures[0], intToFix(1), 32);
            glTranslatef(1, 0.0f, 3);

            glTranslatef(1, 0.0f, -3);
            drawBillboardAt(center, nativeTextures[0], intToFix(1), 32);
            glTranslatef(-1, 0.0f, 3);

            center.mY = intToFix(3) - zCameraOffset;
            glTranslatef(-1, 0.0f, -3);
            drawBillboardAt(center, nativeTextures[0], intToFix(1), 32);
            glTranslatef(1, 0.0f, 3);

            glTranslatef(1, 0.0f, -3);
            drawBillboardAt(center, nativeTextures[0], intToFix(1), 32);
            glTranslatef(-1, 0.0f, 3);

            center.mY = intToFix(6) - zCameraOffset;
            glTranslatef(-1, 0.0f, -3);
            drawBillboardAt(center, nativeTextures[0], intToFix(1), 32);
            glTranslatef(1, 0.0f, 3);

            glTranslatef(1, 0.0f, -3);
            drawBillboardAt(center, nativeTextures[0], intToFix(1), 32);

#ifdef NDS
            swiWaitForVBlank();
#endif
            enter2D();

            drawTextAtWithMargin(((XRES / 8) / 2) - (thisMissionNameLen / 2), 1, XRES, thisMissionName,
                                 getPaletteEntry(0xFFFFFFFF));

            zCameraOffset -= Div(intToFix(1), intToFix(32));

            if (zCameraOffset == 0) {
                int chanceForRandomBattle = getRoom(getPlayerRoom())->chanceOfRandomBattle;
                int diceRoll;

                //tmp
                diceRoll = 0xFF;

                if (diceRoll <= chanceForRandomBattle) {
                    currentPresentationState = kEnteringRandomBattle;
                } else {
                    currentPresentationState = kWaitingForInput;
                    needsToRedrawVisibleMeshes = TRUE;
                    gameTicks = 0;
                    needToRedrawHUD = TRUE;
                }
            }
            return;
        }

        if (currentPresentationState == kWaitingForInput) {

            renderTick(30);

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
    }
}

enum EGameMenuState Crawler_tickCallback(enum ECommand cmd, long delta) {
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

                    if (cursorPosition < 0) {
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


        switch (cmd) {
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
        return kMenuStateUnchanged;
    }

    if (timeUntilNextState != kNonExpiringPresentationState) {
        timeUntilNextState -= delta;
    }

    if (currentPresentationState == kWaitingForInput) {
        if (cmd == kCommandFire4) {
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

void Crawler_unloadStateCallback(int32_t newState) {

    if (newState != kBackToGame &&
        newState != kInspectItem &&
        newState != kHackingGame) {
        int c;
        for (c = 0; c < TOTAL_TEXTURES; ++c) {
            if (nativeTextures[c] != NULL) {
                releaseBitmap(nativeTextures[c]->raw);
                disposeMem(nativeTextures[c]);
                nativeTextures[c] = NULL;
            }
        }
    }
}