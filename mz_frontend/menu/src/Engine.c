#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "FixP.h"
#include "Enums.h"
#include "Common.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "CRenderer.h"
#include "CPackedFileReader.h"

InitStateCallback initStateCallback = NULL;
InitialPaintCallback initialPaintCallback = NULL;
RepaintCallback repaintCallback = NULL;
TickCallback tickCallback = NULL;
UnloadStateCallback unloadStateCallback = NULL;
uint32_t stateTick = 0;
uint32_t globalTick = 0;


extern long timeUntilNextState;
extern enum EPresentationState currentPresentationState;
extern int32_t currentGameMenuState;

char textBuffer[40 * 25];
extern const char *mainText;

int countLines() {
    int len = strlen(mainText);
    int lines = 2; /* initial line + final line must be accounted for */
    int charsInLine = 0;
    int c = 0;
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
    
    if (unloadStateCallback != NULL) {
        unloadStateCallback();
    }
    
    stateTick = 0;
    
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
            
        case kInterrogate_Sofia:
            menuStateToReturn = kMainMenu;
            initStateCallback = Interrogation_initStateCallback;
            initialPaintCallback = Interrogation_initialPaintCallback;
            repaintCallback = Interrogation_repaintCallback;
            tickCallback = Interrogation_tickCallback;
            unloadStateCallback = Interrogation_unloadStateCallback;
            break;
        case kQuit:
            isRunning = FALSE;
            break;
    }
    
    currentGameMenuState = newState;
    initStateCallback(newState, NULL);
    initialPaintCallback();
}

enum ESoundDriver soundDriver = kNoSound;

int menuTick(long delta_time) {

    enum ECommand input;
    int32_t newState;
    
    globalTick++;
    stateTick++;
    
    handleSystemEvents();

    if (soundDriver != kNoSound) {
 //       soundTick();
    }

    /* protect against machines too fast for their own good. */
    if (delta_time <= 0) {
        delta_time = 1;
    }

    input = getInput();

    newState = tickCallback(input, &delta_time);

    if (input == kCommandQuit) {
        return FALSE;
    }

    if (newState != currentGameMenuState && newState != -1) {
   //     playSound(STATE_CHANGE_SOUND);
        enterState(newState);
    }

    repaintCallback();
    flipRenderer();

    return TRUE;
}

int start_clock, end_clock, prev;

void mainLoop() {
    long now, delta_time;
    enum ECommand input;
    int32_t newState;

    globalTick++;
    stateTick++;

    handleSystemEvents();

    delta_time = 1000;
    
#ifdef AMIGA
#ifdef AGA8BPP
    delta_time = 50;
#else
    delta_time = 50;
#endif
#else
    
    
#ifdef ANDROID
	delta_time = 50;
#endif
#ifdef __EMSCRIPTEN__
    delta_time = 500;
#endif
#endif
	input = getInput();
	newState = tickCallback(input, &delta_time);
    
    if (input == kCommandQuit) {
        isRunning = FALSE;
    }
    
    if (newState != currentGameMenuState && newState != -1) {
        enterState(newState);
    }
    
    repaintCallback();
    flipRenderer();

}
