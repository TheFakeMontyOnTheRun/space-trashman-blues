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
#include "Engine.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"
#include "VisibilityStrategy.h"
#include "CPackedFileReader.h"


#include "SoundSystem.h"

char textBuffer[ 40 * 25 ];

InitStateCallback    initStateCallback	  = NULL;
InitialPaintCallback initialPaintCallback = NULL;
RepaintCallback	     repaintCallback	  = NULL;
TickCallback	     tickCallback	  = NULL;
UnloadStateCallback  unloadStateCallback  = NULL;

extern long		       timeUntilNextState;
extern enum EPresentationState currentPresentationState;
extern const char *	       mainText;
extern int32_t		       currentGameMenuState;

int countLines () {
	size_t len	  = strlen ( mainText );
	int lines	  = 2;	/* initial line + final line must be accounted for */
	int charsInLine = 0;
	int c		  = 0;
	for ( c = 0; c < len; ++c ) {
		if ( mainText[ c ] == '\n' ) {
			lines++;
			charsInLine = 0;
		} else {
			charsInLine++;
		}
	}

	return lines - 1;
}

void enterState ( enum EGameMenuState newState ) {

	if ( unloadStateCallback != NULL ) {
		unloadStateCallback ();
	}

	timeUntilNextState	   = MENU_ITEM_TIME_TO_BECOME_ACTIVE_MS;
	currentPresentationState = kAppearing;
	currentBackgroundBitmap  = NULL;
	cursorPosition	   = 0;
	nextNavigationSelection  = -1;

	switch ( newState ) {
		default:
		case kMainMenu:
			initStateCallback	   = MainMenu_initStateCallback;
			initialPaintCallback = MainMenu_initialPaintCallback;
			repaintCallback	   = MainMenu_repaintCallback;
			tickCallback	   = MainMenu_tickCallback;
			unloadStateCallback  = MainMenu_unloadStateCallback;
			break;
		case kHelp:
			initStateCallback	   = HelpScreen_initStateCallback;
			initialPaintCallback = HelpScreen_initialPaintCallback;
			repaintCallback	   = HelpScreen_repaintCallback;
			tickCallback	   = HelpScreen_tickCallback;
			unloadStateCallback  = HelpScreen_unloadStateCallback;
			break;
		case kCredits:
			initStateCallback	   = CreditsScreen_initStateCallback;
			initialPaintCallback = CreditsScreen_initialPaintCallback;
			repaintCallback	   = CreditsScreen_repaintCallback;
			tickCallback	   = CreditsScreen_tickCallback;
			unloadStateCallback  = CreditsScreen_unloadStateCallback;
			break;
		case kPlayGame:
            menuStateToReturn = kMainMenu;
			initStateCallback	   = Crawler_initStateCallback;
			initialPaintCallback = Crawler_initialPaintCallback;
			repaintCallback	   = Crawler_repaintCallback;
			tickCallback	   = Crawler_tickCallback;
			unloadStateCallback  = Crawler_unloadStateCallback;
			break;
        case kInspectItem: {
            initStateCallback       = GameMenu_initStateCallback;
            initialPaintCallback = GameMenu_initialPaintCallback;
            repaintCallback       = GameMenu_repaintCallback;
            tickCallback       = GameMenu_tickCallback;
            unloadStateCallback  = GameMenu_unloadStateCallback;
		} break;
		case kQuit:
			isRunning = FALSE;
			break;
	}

	currentGameMenuState = newState;
	initStateCallback ( newState );
	initialPaintCallback ();
}

enum ESoundDriver soundDriver = kNoSound;

int menuTick ( long delta_time ) {

	enum ECommand input;
	int32_t	newState;

	handleSystemEvents ();

	if ( soundDriver != kNoSound ) {
		soundTick ();
	}

	input = getInput ();

	newState = tickCallback ( input, &delta_time );

	if ( input == kCommandQuit ) {
		isRunning = FALSE;
		return FALSE;
	}

	if ( newState != currentGameMenuState && newState != -1 ) {
		playSound ( STATE_CHANGE_SOUND );
		enterState ( newState );
	}

	repaintCallback ();
	flipRenderer ();

	return isRunning;
}

#ifdef __EMSCRIPTEN__
void mainLoop () {
  menuTick ( 50 );
}
#endif
