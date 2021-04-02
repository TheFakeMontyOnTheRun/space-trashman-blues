#ifndef ENGINE_H
#define ENGINE_H

typedef int32_t ( *InitStateCallback )(int32_t tag, void *data);

typedef void ( *InitialPaintCallback )();

typedef void ( *RepaintCallback )();

typedef int32_t ( *TickCallback )(int32_t tag, void *data);

typedef void ( *UnloadStateCallback )();

#define kNonExpiringPresentationState 0xFFFF
#define kDefaultPresentationStateInterval 2000
#define kMenuStateUnchanged -1


extern InitStateCallback initStateCallback;
extern InitialPaintCallback initialPaintCallback;
extern RepaintCallback repaintCallback;
extern TickCallback tickCallback;
extern UnloadStateCallback unloadStateCallback;
extern int isRunning;

extern long timeUntilNextState;
extern enum EPresentationState currentPresentationState;
extern struct Bitmap *currentBackgroundBitmap;

extern uint8_t cursorPosition;
extern int32_t nextNavigationSelection;
extern int32_t menuStateToReturn;

extern uint32_t stateTick;
extern uint32_t globalTick;

int menuTick(long ms);

int32_t MainMenu_initStateCallback(int32_t tag, void *data);

void MainMenu_initialPaintCallback();

void MainMenu_repaintCallback();

int32_t MainMenu_tickCallback(int32_t tag, void *data);

void MainMenu_unloadStateCallback();

int32_t HelpScreen_initStateCallback(int32_t tag, void *data);

void HelpScreen_initialPaintCallback();

void HelpScreen_repaintCallback();

int32_t HelpScreen_tickCallback(int32_t tag, void *data);

void HelpScreen_unloadStateCallback();

int32_t Interrogation_initStateCallback(int32_t tag, void *data);

void Interrogation_initialPaintCallback();

void Interrogation_repaintCallback();

int32_t Interrogation_tickCallback(int32_t tag, void *data);

void Interrogation_unloadStateCallback();

int32_t CreditsScreen_initStateCallback(int32_t tag, void *data);

void CreditsScreen_initialPaintCallback();

void CreditsScreen_repaintCallback();

int32_t CreditsScreen_tickCallback(int32_t tag, void *data);

void CreditsScreen_unloadStateCallback();

int countLines();

void enterState(enum EGameMenuState State);

#define MENU_ITEM_TIME_TO_BECOME_ACTIVE_MS 200

/* 84ms * 6 blinks == ~500ms */
#define MENU_ITEM_TIME_TO_BLINK_MS 84


void startup();

unsigned long getMilliseconds();

#endif
