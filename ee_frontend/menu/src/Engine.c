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

#include "FixP.h"
#include "Vec.h"
#include "Enums.h"
#include "CActor.h"
#include "MapWithCharKey.h"
#include "Common.h"
#include "LoadBitmap.h"
#include "Core.h"
#include "Engine.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"
#include "VisibilityStrategy.h"
#include "PackedFileReader.h"


#include "SoundSystem.h"

char textBuffer[40 * 25];

InitStateCallback initStateCallback = NULL;
InitialPaintCallback initialPaintCallback = NULL;
RepaintCallback repaintCallback = NULL;
TickCallback tickCallback = NULL;
UnloadStateCallback unloadStateCallback = NULL;

int countLines() {
    size_t len = strlen(mainText);
    int lines = 2;    /* initial line + final line must be accounted for */
    int charsInLine = 0;
    size_t c;
    for (c = 0; c < len; ++c) {
        if (mainText[c] == '\n') {
            lines++;
            charsInLine = 0;
        } else {
            charsInLine++;
        }
    }

    return lines - 1;
}

void enterState(enum EGameMenuState newState) {

    stopSounds();
    
    if (unloadStateCallback != NULL) {
        unloadStateCallback();
    }

    timeUntilNextState = MENU_ITEM_TIME_TO_BECOME_ACTIVE_MS;
    currentPresentationState = kAppearing;
    currentBackgroundBitmap = NULL;
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
        case kRandomBattle: {
            initStateCallback = BattleScreen_initStateCallback;
            initialPaintCallback = BattleScreen_initialPaintCallback;
            repaintCallback = BattleScreen_repaintCallback;
            tickCallback = BattleScreen_tickCallback;
            unloadStateCallback = BattleScreen_unloadStateCallback;
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

enum ESoundDriver soundDriver = kNoSound;

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

#ifdef __EMSCRIPTEN__
void mainLoop () {
  menuTick ( 50 );
}
#endif