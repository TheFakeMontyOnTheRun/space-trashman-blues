#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef AMIGA
#include "AmigaInt.h"
#else
#ifdef WIN32
#include "Win32Int.h"
#else

#include <stdint.h>
#include <unistd.h>

#endif
#endif

#include "Enums.h"
#include "Common.h"
#include "Engine.h"
#include "SoundSystem.h"

InitStateCallback initStateCallback = NULL;
InitialPaintCallback initialPaintCallback = NULL;
RepaintCallback repaintCallback = NULL;
TickCallback tickCallback = NULL;
UnloadStateCallback unloadStateCallback = NULL;

enum EGameMenuState currentGameMenuState = kResumeCurrentState;
enum EGameMenuState menuStateToReturn = kResumeCurrentState;
int cursorPosition = 0;
enum EGameMenuState nextNavigationSelection = kResumeCurrentState;
long timeUntilNextState = MENU_ITEM_TIME_TO_BECOME_ACTIVE_MS;
enum EPresentationState currentPresentationState;
size_t biggestOption;
int isRunning = TRUE;
const char *mainText = NULL;
int enable3DRendering = TRUE;
int needsToRedrawHUD = 1;

void enterState(enum EGameMenuState newState) {

    stopSounds();

    if (unloadStateCallback != NULL) {
        unloadStateCallback(newState);
    }

    timeUntilNextState = MENU_ITEM_TIME_TO_BECOME_ACTIVE_MS;
    currentPresentationState = kAppearing;
    cursorPosition = 0;
    nextNavigationSelection = -1;
    
    switch (newState) {
        default:
        case kMainMenu:
            initStateCallback = MainMenu_initStateCallback;
            initialPaintCallback = MainMenu_initialPaintCallback;
            repaintCallback = MainMenu_repaintCallback;
            tickCallback = MainMenu_tickCallback;
            unloadStateCallback = MainMenu_unloadStateCallback;
            break;
        case kGoodGameOverEpilogue:
        case kBadGameOverEpilogue:
        case kGoodVictoryEpilogue:
        case kBadVictoryEpilogue:
            initStateCallback = GameMenu_initStateCallback;
            initialPaintCallback = GameMenu_initialPaintCallback;
            repaintCallback = GameMenu_repaintCallback;
            tickCallback = GameMenu_tickCallback;
            unloadStateCallback = GameMenu_unloadStateCallback;
            break;
        case kHelp:
            initStateCallback = HelpScreen_initStateCallback;
            initialPaintCallback = HelpScreen_initialPaintCallback;
            repaintCallback = HelpScreen_repaintCallback;
            tickCallback = HelpScreen_tickCallback;
            unloadStateCallback = HelpScreen_unloadStateCallback;
            break;
        case kCredits:
            initStateCallback = CreditsScreen_initStateCallback;
            initialPaintCallback = CreditsScreen_initialPaintCallback;
            repaintCallback = CreditsScreen_repaintCallback;
            tickCallback = CreditsScreen_tickCallback;
            unloadStateCallback = CreditsScreen_unloadStateCallback;
            break;
        case kPlayGame:
        case kBackToGame:
            menuStateToReturn = kMainMenu;
            initStateCallback = Crawler_initStateCallback;
            initialPaintCallback = Crawler_initialPaintCallback;
            repaintCallback = Crawler_repaintCallback;
            tickCallback = Crawler_tickCallback;
            unloadStateCallback = Crawler_unloadStateCallback;
            break;
        case kInspectItem: {
            initStateCallback = GameMenu_initStateCallback;
            initialPaintCallback = GameMenu_initialPaintCallback;
            repaintCallback = GameMenu_repaintCallback;
            tickCallback = GameMenu_tickCallback;
            unloadStateCallback = GameMenu_unloadStateCallback;
        }
            break;
        case kHackingGame: {
            initStateCallback = HackingScreen_initStateCallback;
            initialPaintCallback = HackingScreen_initialPaintCallback;
            repaintCallback = HackingScreen_repaintCallback;
            tickCallback = HackingScreen_tickCallback;
            unloadStateCallback = HackingScreen_unloadStateCallback;
        }
            break;
        case kQuit:
            isRunning = FALSE;
            break;
    }

    currentGameMenuState = newState;
    initStateCallback(newState);
    initialPaintCallback();
}

int menuTick(long delta_time) {

    enum ECommand input;
    enum EGameMenuState newState;

    handleSystemEvents();

    if (soundDriver != kNoSound) {
        soundTick();
    }

    input = getInput();

    newState = tickCallback(input, delta_time);

    if (input == kCommandQuit) {
        isRunning = FALSE;
        return FALSE;
    }

    if (newState != currentGameMenuState && newState != kResumeCurrentState) {
        playSound(STATE_CHANGE_SOUND);
        enterState(newState);
    }

    repaintCallback();

    return isRunning;
}
