#ifdef WIN32
#include "Win32Int.h"
#else

#include <stdint.h>
#include <unistd.h>

#endif

#include <stdio.h>
#include <string.h>

#include "Enums.h"
#include "FixP.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "Vec.h"
#include "Common.h"
#include "LoadBitmap.h"
#include "CActor.h"
#include "Mesh.h"
#include "CRenderer.h"
#include "UI.h"
#include "Globals.h"
#include "SoundSystem.h"
#include "Core.h"

const char **GameMenu_options;
const enum EGameMenuState *GameMenu_nextStateNavigation;
const char *GameMenu_StateTitle;
struct Bitmap *featuredBitmap = NULL;

const char *inspectItem_options[1] = {"Back"};

const enum EGameMenuState InspectItem_nextStateNavigation[1] = {
        kBackToGame};

const enum EGameMenuState GameMenu_EndGame_nextStateNavigation[2] = {kInspectItem, kMainMenu};

const char *GameMenu_EndGame_options[2] = {"No", "Yes"};

const char *GameMenu_Story_options[1] = {"Continue"};

const enum EGameMenuState GameMenu_Story_nextStateNavigation[1] = {kMainMenu};

int16_t GameMenu_optionsCount = 2;
int drawFilter = FALSE;

void GameMenu_initStateCallback(int32_t tag) {
    int c;

    GameMenu_StateTitle = NULL;
    cursorPosition = 0;
    currentPresentationState = kAppearing;
    timeUntilNextState = 500;
    memFill(textBuffer, ' ', TEXT_BUFFER_SIZE);
    drawFilter = FALSE;

    switch (tag) {

        case kEndGame:
            timeUntilNextState = 0;
            drawFilter = TRUE;
            GameMenu_StateTitle = "End session?";
            mainText = textBuffer;
            GameMenu_optionsCount = 2;
            GameMenu_options = &GameMenu_EndGame_options[0];
            GameMenu_nextStateNavigation = &GameMenu_EndGame_nextStateNavigation[0];
            break;


        case kGoodVictoryEpilogue:
            sprintf (textBuffer, "Victory! You managed to destroy the\nship and get out alive\n\n\n\n\n\n");
            mainText = textBuffer;

            GameMenu_StateTitle = "Victory";

            GameMenu_optionsCount = 1;
            GameMenu_options = &GameMenu_Story_options[0];
            GameMenu_nextStateNavigation = &GameMenu_Story_nextStateNavigation[0];
            break;

        case kBadVictoryEpilogue:
            sprintf (textBuffer, "Victory! Too bad you didn't survive\nto tell the story\n\n\n\n\n\n");
            mainText = textBuffer;
            GameMenu_StateTitle = "Victory";
            GameMenu_optionsCount = 1;
            GameMenu_options = &GameMenu_Story_options[0];
            GameMenu_nextStateNavigation = &GameMenu_Story_nextStateNavigation[0];
            break;

        case kGoodGameOverEpilogue:
            sprintf (textBuffer,
                     "You failed! While you fled the ship\n"
                     "alive, you failed to prevent the \n"
                     "worst scenario and now EVERYBODY is\n"
                     "dead (and that includes you!)\n\n\n\n\n");
            mainText = textBuffer;
            GameMenu_StateTitle = "Game Over";
            GameMenu_optionsCount = 1;
            GameMenu_options = &GameMenu_Story_options[0];
            GameMenu_nextStateNavigation = &GameMenu_Story_nextStateNavigation[0];
            break;

        case kBadGameOverEpilogue:
            sprintf (textBuffer,
                     "You're dead! And so are millions of\n"
                     "other people on the path of\n"
                     "destruction faulty reactor\n\n\n\n\n\n");
            mainText = textBuffer;
            GameMenu_StateTitle = "Game Over";
            GameMenu_optionsCount = 1;
            GameMenu_options = &GameMenu_Story_options[0];
            GameMenu_nextStateNavigation = &GameMenu_Story_nextStateNavigation[0];
            break;

        case kInspectItem: {
            struct ObjectNode *head;
            int index = 0;

            GameMenu_StateTitle = "CyDeck";

            head = getPlayerItems();

            while (head != NULL && index < currentSelectedItem) {
                ++index;
                head = head->next;
            }

            if (head != NULL && getItem(head->item) != NULL) {
                struct Item *item = getItem(head->item);
                mainText = item->name;
#ifdef INCLUDE_ITEM_DESCRIPTIONS
                strcpy(textBuffer, item->info);
#endif
                GameMenu_optionsCount = 1;
                GameMenu_options = &inspectItem_options[0];
                GameMenu_nextStateNavigation =
                        &InspectItem_nextStateNavigation[0];
            }
        }
    }

    biggestOption = strlen(GameMenu_StateTitle);

    for (c = 0; c < GameMenu_optionsCount; ++c) {
        size_t len = strlen(GameMenu_options[c]);

        if (len > biggestOption) {
            biggestOption = len;
        }
    }

    featuredBitmap = NULL;
}

void GameMenu_initialPaintCallback(void) {
}

void GameMenu_repaintCallback(void) {
    int c;
    int16_t optionsHeight = 8 * (GameMenu_optionsCount);

    fill(0, 0, (XRES_FRAMEBUFFER), (YRES_FRAMEBUFFER), getPaletteEntry(0xFF6cb1a3), FALSE);

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

        drawRect(XRES_FRAMEBUFFER - movementX - 8 - 24 - ((biggestOption * 8) / 2)
                 + (sizeX / 2),
                 YRES_FRAMEBUFFER - movementY - 8 - 16 - 8 - ((optionsHeight + 8) / 2)
                 + (sizeY / 2),
                 sizeX, sizeY, getPaletteEntry(0xFF000000));

        return;
    }

    if (drawFilter) {
        fill(0, 0, XRES_FRAMEBUFFER, YRES_FRAMEBUFFER, getPaletteEntry(0xFF000000), TRUE);
    }

    if (mainText != NULL) {
        drawTextWindow(1, 1, (XRES_FRAMEBUFFER / 8) - 2, (YRES_FRAMEBUFFER / 8) - 5, GameMenu_StateTitle, textBuffer);
    }

    drawWindow((XRES_FRAMEBUFFER / 8) - biggestOption - 3, (YRES_FRAMEBUFFER / 8) - (optionsHeight / 8) - 3,
               biggestOption + 2, (optionsHeight / 8) + 2,
               GameMenu_StateTitle);

    for (c = 0; c < GameMenu_optionsCount; ++c) {

        int isCursor = (cursorPosition == c)
                       && ((currentPresentationState == kConfirmInputBlink1)
                           || (currentPresentationState == kConfirmInputBlink3)
                           || (currentPresentationState == kConfirmInputBlink5)
                           || (currentPresentationState == kWaitingForInput));

        int shouldGreyOut = FALSE;

        if (isCursor) {
            fill(XRES_FRAMEBUFFER - (biggestOption * 8) - 16 - 8 - 8,
                 (YRES_FRAMEBUFFER - optionsHeight) + (c * 8) - 8 - 8,
                 (biggestOption * 8) + 16, 8, getPaletteEntry(0xFF000000), FALSE);
        }

        drawTextAt(
                (XRES_FRAMEBUFFER / 8) - biggestOption - 2,
                (((YRES_FRAMEBUFFER / 8) + 1) - GameMenu_optionsCount) + c - 2,
                &GameMenu_options[c][0],
                isCursor ? (shouldGreyOut ? getPaletteEntry(0xFF000099) : getPaletteEntry(0xFF0000FF)) : (shouldGreyOut
                                                                                                          ? getPaletteEntry(
                                0xFF555555) : getPaletteEntry(0xFF000000)));
    }
}

enum EGameMenuState GameMenu_tickCallback(enum ECommand cmd, long delta) {

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
            case kCommandBack:
                return kMainMenu;
            case kCommandUp:
                playSound(MENU_SELECTION_CHANGE_SOUND);
                cursorPosition = (cursorPosition - 1);

                if (cursorPosition >= GameMenu_optionsCount) {
                    cursorPosition = GameMenu_optionsCount - 1;
                }

                if (cursorPosition < 0) {
                    cursorPosition = 0;
                }

                featuredBitmap = NULL;
                break;
            case kCommandDown:
                playSound(MENU_SELECTION_CHANGE_SOUND);
                cursorPosition =
                        (uint8_t) ((cursorPosition + 1) % GameMenu_optionsCount);

                featuredBitmap = NULL;

                break;
            case kCommandFire1:
            case kCommandFire2:
            case kCommandFire3:

                featuredBitmap = NULL;
                nextNavigationSelection = GameMenu_nextStateNavigation[cursorPosition];
                currentPresentationState = kConfirmInputBlink1;
                break;
        }
    }

    return kResumeCurrentState;
}

void GameMenu_unloadStateCallback(int32_t newState) {
    if (featuredBitmap) {
        releaseBitmap(featuredBitmap);
        featuredBitmap = NULL;
    }
}