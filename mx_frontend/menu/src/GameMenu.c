#include <stdint.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>

#include "Enums.h"
#include "FixP.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "FixP.h"
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
#include "UI.h"
#include "Globals.h"
#include "SoundSystem.h"

#define TEXT_BUFFER_SIZE 40 * 25

const char **GameMenu_options;
const int32_t *GameMenu_nextStateNavigation;
const char *GameMenu_StateTitle;
struct Bitmap *featuredBitmap = NULL;
void goTo(int location);
extern int playerLocation;


const char *GameMenu_Main_options[4] = {"Read Dossiers", "Investigate",
									   "Travel", "End game"};

int32_t GameMenu_Main_nextStateNavigation[4] = {
		kDossiersMenu,
		kInvestigateMenu, kTravelMenu, kEndGame};

const char *GameMenu_Dossier_options[9] = {"Sofia", "Ricardo", "Lola",
										  "Pau", "Lina", "Elias",
										  "Carmen", "Jean", "Back"};

int32_t GameMenu_Travel_nextStateNavigation[9] = {
		kTravelPorto, kTravelLisbon, kTravelMadrid,
		kTravelBarcelona, kTravelFrankfurt, kTravelHamburg,
		kTravelLuxembourg, kTravelBrussels, kGameMenu};

const char *GameMenu_Travel_options[9] = {
		"Porto", "Lisbon", "Madrid", "Barcelona", "Frankfurt",
		"Hamburg", "Luxembourg", "Brussels", "Back"};

int32_t GameMenu_Dossier_nextStateNavigation[9] = {
		kReadDossier_Sofia, kReadDossier_Ricardo, kReadDossier_Lola,
		kReadDossier_Pau, kReadDossier_Lina, kReadDossier_Elias,
		kReadDossier_Carmen, kReadDossier_Jean, kGameMenu};


int32_t GameMenu_EndGame_nextStateNavigation[2] = {kGameMenu, kMainMenu};

const char *GameMenu_EndGame_options[2] = {"No", "Yes"};

const char *GameMenu_ReadDossier_options[1] = {"Back"};

int32_t GameMenu_ReadDossier_nextStateNavigation[1] = {
		kDossiersMenu,
};

const char *GameMenu_Story_options[1] = {"Continue"};

int32_t GameMenu_Story_nextStateNavigation[1] = {kMainMenu};

int16_t GameMenu_optionsCount = 4;
extern size_t biggestOption;
extern char textBuffer[TEXT_BUFFER_SIZE];
int32_t GameMenu_substate;
int drawFilter = FALSE;


void getDisplayStatusText(char* buffer, size_t length) {
	sprintf(buffer, "%s", "farofinha");
}

void getDossierText(int slot, char* buffer, size_t length) {
	sprintf(buffer, "%s %d", "farofinha fofa", slot);
}


void getLocationName(int slot, char* buffer, size_t length) {
	sprintf(buffer, "%s %d", "farofinha fofa", slot);
}

int getPlayerLocation() {
    return 12;// playerLocation;
}

int getTurn() {
	return 1;
}

void goTo(int slot ) {
	playerLocation = slot;
}


int32_t GameMenu_initStateCallback(int32_t tag) {
	int c;
	GameMenu_StateTitle = NULL;
	cursorPosition = 0;
	GameMenu_substate = tag;
	currentPresentationState = kAppearing;
	timeUntilNextState = 500;
	memset (&textBuffer[0], ' ', 40 * 25);
	drawFilter = FALSE;

	switch (tag) {

		case kEndGame:
			timeUntilNextState = 0;
			drawFilter = TRUE;
			GameMenu_StateTitle = "End session?";
			getDisplayStatusText(&textBuffer[0], 40 * 10);
			mainText = &textBuffer[0];
			currentBackgroundBitmap = loadBitmap("pattern.img");
			GameMenu_optionsCount = 2;
			GameMenu_options = &GameMenu_EndGame_options[0];
			GameMenu_nextStateNavigation = &GameMenu_EndGame_nextStateNavigation[0];
			break;
		case kPlayGame:
		case kGameMenu:
			GameMenu_StateTitle = "Investigation";
			getDisplayStatusText(&textBuffer[0], 40 * 10);
			mainText = &textBuffer[0];
			currentBackgroundBitmap = loadBitmap("pattern.img");
			GameMenu_optionsCount = 4;
			GameMenu_options = &GameMenu_Main_options[0];
			GameMenu_nextStateNavigation = &GameMenu_Main_nextStateNavigation[0];
			break;

		case kDossiersMenu:
			GameMenu_StateTitle = "Dossiers";
			getDisplayStatusText(&textBuffer[0], 40 * 10);
			mainText = &textBuffer[0];
			currentBackgroundBitmap = loadBitmap("pattern.img");
			GameMenu_optionsCount = 9;
			GameMenu_options = &GameMenu_Dossier_options[0];
			GameMenu_nextStateNavigation = &GameMenu_Dossier_nextStateNavigation[0];
			break;


		case kTravelMenu:
			GameMenu_StateTitle = "Travel";
			getDisplayStatusText(&textBuffer[0], 40 * 10);
			mainText = &textBuffer[0];
			currentBackgroundBitmap = loadBitmap("pattern.img");
			GameMenu_optionsCount = 9;
			GameMenu_options = &GameMenu_Travel_options[0];
			GameMenu_nextStateNavigation = &GameMenu_Travel_nextStateNavigation[0];
			break;

		case kVictory:
			sprintf (textBuffer, "Victory!\n\n\n\n\n\n");
			mainText = &textBuffer[0];

			GameMenu_StateTitle = "Victory";
			currentBackgroundBitmap = loadBitmap("pattern.img");

			GameMenu_optionsCount = 1;
			GameMenu_options = &GameMenu_Story_options[0];
			GameMenu_nextStateNavigation = &GameMenu_Story_nextStateNavigation[0];
			break;

		case kGameOver:
			sprintf (textBuffer, "You're dead!\n\n\n\n\n\n");
			mainText = &textBuffer[0];

			GameMenu_StateTitle = "Game Over";
			currentBackgroundBitmap = loadBitmap("pattern.img");

			GameMenu_optionsCount = 1;
			GameMenu_options = &GameMenu_Story_options[0];
			GameMenu_nextStateNavigation = &GameMenu_Story_nextStateNavigation[0];
			break;

		case kPrologue:
			sprintf (textBuffer, "Out of prison");
			mainText = &textBuffer[0];

			GameMenu_StateTitle = "Everything's changed...but still feels the same.";
			currentBackgroundBitmap = loadBitmap("pattern.img");

			GameMenu_optionsCount = 1;
			GameMenu_options = &GameMenu_Story_options[0];
			GameMenu_nextStateNavigation = &GameMenu_Story_nextStateNavigation[0];
			break;

		case kReadDossier_Sofia:
		case kReadDossier_Ricardo:
		case kReadDossier_Lola:
		case kReadDossier_Pau:
		case kReadDossier_Lina:
		case kReadDossier_Elias:
		case kReadDossier_Carmen:
		case kReadDossier_Jean: {
			GameMenu_StateTitle = "Dossier";
			currentBackgroundBitmap = loadBitmap("pattern.img");

			mainText = &textBuffer[0];
			getDossierText(tag - kReadDossier_Sofia, &textBuffer[0], 40 * 25);

			GameMenu_optionsCount = 1;
			GameMenu_options = &GameMenu_ReadDossier_options[0];
			GameMenu_nextStateNavigation =
					&GameMenu_ReadDossier_nextStateNavigation[0];
		}
	}

	biggestOption = strlen(GameMenu_StateTitle);

	for (c = 0; c < GameMenu_optionsCount; ++c) {
		size_t len = strlen(GameMenu_options[c]);

		if (len > biggestOption) {
			biggestOption = len;
		}
	}

	return 0;
}

void GameMenu_initialPaintCallback(void) {

	if (currentBackgroundBitmap != NULL) {
		drawRepeatBitmap(0, 32, 320, 200, currentBackgroundBitmap);
	}

	featuredBitmap = NULL;
}

void GameMenu_repaintCallback(void) {
	int c;

	int lines = countLines();
	char photoBuffer[16];
	const char *name;
	int wrappedCursor = ( cursorPosition < 8 ) ? cursorPosition : getPlayerLocation();

	int16_t optionsHeight = 8 * (GameMenu_optionsCount);

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

		drawRect(320 - movementX - 8 - 24 - ((biggestOption * 8) / 2)
				 + (sizeX / 2),
				 200 - movementY - 8 - 16 - 8 - ((optionsHeight + 8) / 2)
				 + (sizeY / 2),
				 sizeX, sizeY, 0);

		return;
	}

	if (GameMenu_substate == kTravelMenu  ) {

		name = GameMenu_Travel_options[wrappedCursor];

		if (featuredBitmap == NULL ) {
			sprintf (photoBuffer, "location%d.img", wrappedCursor);
			featuredBitmap = loadBitmap(photoBuffer);
		}

		drawImageWindow( 5, 7, 16, 17, name, featuredBitmap);

	} else if (mainText != NULL) {

		int textWidth;
		int posX;
		int posY;

		switch (GameMenu_substate) {
			case kReadDossier_Sofia:
			case kReadDossier_Ricardo:
			case kReadDossier_Lola:
			case kReadDossier_Pau:
			case kReadDossier_Lina:
			case kReadDossier_Elias:
			case kReadDossier_Carmen:
			case kReadDossier_Jean: {
				textWidth = 320;
				posX = 0;
				posY = 0;
			}
			break;
			default:
				textWidth = 176;
				posX = 8;
				posY = 8;
		}

		{
			char turnBuffer[64];
			char locationBuffer[64];


			switch (GameMenu_substate) {
				case kReadDossier_Sofia:
				case kReadDossier_Ricardo:
				case kReadDossier_Lola:
				case kReadDossier_Pau:
				case kReadDossier_Lina:
				case kReadDossier_Elias:
				case kReadDossier_Carmen:
				case kReadDossier_Jean: {
					getLocationName(GameMenu_substate - kReadDossier_Sofia, locationBuffer, 64);
					sprintf (turnBuffer, "%s", locationBuffer);
				}
					break;
				default:
					getLocationName(getPlayerLocation(), locationBuffer, 64);
					sprintf (turnBuffer, "Turn %d: %s", getTurn(),
							 locationBuffer);
			}


			drawTextWindow( (posX / 8) + 1, ( posY / 8 ) + 1, textWidth / 8, lines + 4, turnBuffer, mainText );
		}
	}

	if (drawFilter) {
		fill(0, 0, 320, 200, 0, TRUE);
	}

	drawWindow( 40 - biggestOption - 3, 25 - (optionsHeight / 8) - 3, biggestOption + 2, (optionsHeight / 8) + 2, GameMenu_StateTitle );

	for (c = 0; c < GameMenu_optionsCount; ++c) {

		int isCursor = (cursorPosition == c)
					   && ((currentPresentationState == kConfirmInputBlink1)
						   || (currentPresentationState == kConfirmInputBlink3)
						   || (currentPresentationState == kConfirmInputBlink5)
						   || (currentPresentationState == kWaitingForInput));

		int shouldGreyOut =
				(GameMenu_substate == kTravelMenu && (c < 8) );

		if (isCursor) {
			fill(320 - (biggestOption * 8) - 16 - 8 - 8,
				 (200 - optionsHeight) + (c * 8) - 8 - 8,
				 (biggestOption * 8) + 16, 8, 0, FALSE);
		}

		drawTextAt(
				40 - biggestOption - 2, (26 - GameMenu_optionsCount) + c - 2,
				&GameMenu_options[c][0],
				isCursor ? (shouldGreyOut ? 128 : 200) : (shouldGreyOut ? 64 : 0));
	}
}

int32_t GameMenu_tickCallback(int32_t tag, void *data) {

	long delta = *((long *) data);

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

		switch (tag) {
			case kCommandBack:
				return kMainMenu;
			case kCommandUp:
				playSound(MENU_SELECTION_CHANGE_SOUND);
				cursorPosition = (cursorPosition - 1);

				if (cursorPosition >= GameMenu_optionsCount) {
					cursorPosition = GameMenu_optionsCount - 1;
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

				if (GameMenu_substate == kTravelMenu && cursorPosition < 8) {
					goTo(cursorPosition);
				}
				featuredBitmap = NULL;
				nextNavigationSelection = GameMenu_nextStateNavigation[cursorPosition];
				currentPresentationState = kConfirmInputBlink1;
				break;
		}
	}

	return -1;
}

void GameMenu_unloadStateCallback() {
	if (currentBackgroundBitmap != NULL) {
		releaseBitmap(currentBackgroundBitmap);
		currentBackgroundBitmap = NULL;

		if (featuredBitmap) {
			releaseBitmap(featuredBitmap);
			featuredBitmap = NULL;
		}
	}
}
