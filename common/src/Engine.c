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
RepaintCallback repaintCallback = NULL;
TickCallback tickCallback = NULL;
UnloadStateCallback unloadStateCallback = NULL;

uint8_t firstFrameOnCurrentState = 1;
enum EGameMenuState currentGameMenuState = kResumeCurrentState;
enum EGameMenuState menuStateToReturn = kResumeCurrentState;
enum EGameMenuState nextNavigationSelection = kResumeCurrentState;
enum EPresentationState currentPresentationState;
int8_t cursorPosition = 0;
long timeUntilNextState = MENU_ITEM_TIME_TO_BECOME_ACTIVE_MS;
size_t biggestOption;
uint8_t isRunning = TRUE;
const char *mainText = NULL;

void enterState(enum EGameMenuState newState) {

    stopSounds();

    if (unloadStateCallback != NULL) {
        unloadStateCallback(newState);
    }

    timeUntilNextState = MENU_ITEM_TIME_TO_BECOME_ACTIVE_MS;
    currentPresentationState = kWaitingForInput;
    cursorPosition = 0;
    nextNavigationSelection = -1;
    
    switch (newState) {
        default:
        case kMainMenu:
            initStateCallback = MainMenu_initStateCallback;
            repaintCallback = MainMenu_repaintCallback;
            tickCallback = MainMenu_tickCallback;
            unloadStateCallback = MainMenu_unloadStateCallback;
            break;
        case kGoodGameOverEpilogue:
        case kBadGameOverEpilogue:
        case kGoodVictoryEpilogue:
        case kBadVictoryEpilogue:
            initStateCallback = GameMenu_initStateCallback;
            repaintCallback = GameMenu_repaintCallback;
            tickCallback = GameMenu_tickCallback;
            unloadStateCallback = GameMenu_unloadStateCallback;
            break;
        case kHelp:
            initStateCallback = HelpScreen_initStateCallback;
            repaintCallback = HelpScreen_repaintCallback;
            tickCallback = HelpScreen_tickCallback;
            unloadStateCallback = HelpScreen_unloadStateCallback;
            break;
        case kCredits:
            initStateCallback = CreditsScreen_initStateCallback;
            repaintCallback = CreditsScreen_repaintCallback;
            tickCallback = CreditsScreen_tickCallback;
            unloadStateCallback = CreditsScreen_unloadStateCallback;
            break;
        case kPlayGame:
        case kBackToGame:
            menuStateToReturn = kMainMenu;
            initStateCallback = Crawler_initStateCallback;
            repaintCallback = Crawler_repaintCallback;
            tickCallback = Crawler_tickCallback;
            unloadStateCallback = Crawler_unloadStateCallback;
            break;
        case kHackingGame: {
            initStateCallback = HackingScreen_initStateCallback;
            repaintCallback = HackingScreen_repaintCallback;
            tickCallback = HackingScreen_tickCallback;
            unloadStateCallback = HackingScreen_unloadStateCallback;
        }
            break;
#ifdef EMIT_QUIT_OPTION
        case kQuit:
            isRunning = FALSE;
            break;
#endif
    }

    currentGameMenuState = newState;
    initStateCallback(newState);
    firstFrameOnCurrentState = 1;
}

uint8_t menuTick(long delta_time) {

    enum ECommand input;
    enum EGameMenuState newState;

    handleSystemEvents();

    if (soundDriver != kNoSound) {
        soundTick();
    }

    input = getInput();

    newState = tickCallback(input, delta_time);
    
#ifdef EMIT_QUIT_OPTION
    if (input == kCommandQuit) {
        isRunning = FALSE;
        return FALSE;
    }
#endif
    
    if (newState != currentGameMenuState && newState != kResumeCurrentState) {
        playSound(STATE_CHANGE_SOUND);
        enterState(newState);
    }

    repaintCallback();
    firstFrameOnCurrentState = 0;

    return isRunning;
}
