/*
 Created by Daniel Monteiro on 03/10/2019.
*/

#ifdef WIN32
#include "Win32Int.h"
#else

#include <stdint.h>
#include <unistd.h>

#endif

#include "Common.h"
#include "Enums.h"
#include "Globals.h"
#include "Core.h"
#include "Engine.h"

enum EGameMenuState currentGameMenuState = kResumeCurrentState;
enum EGameMenuState menuStateToReturn = kResumeCurrentState;
int cursorPosition = 0;
enum EGameMenuState nextNavigationSelection = kResumeCurrentState;
long timeUntilNextState = MENU_ITEM_TIME_TO_BECOME_ACTIVE_MS;
enum EPresentationState currentPresentationState;
size_t biggestOption;
const char *mainText = NULL;
int isRunning = TRUE;
int enable3DRendering = TRUE;
int currentSelectedItem = 0;
int needsToRedrawHUD = 1;

