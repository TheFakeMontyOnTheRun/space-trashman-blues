#ifndef ENGINE_H
#define ENGINE_H

typedef int32_t ( *InitStateCallback )(int32_t tag);

typedef void ( *InitialPaintCallback )(void);

typedef void ( *RepaintCallback )(void);

typedef int32_t ( *TickCallback )(int32_t tag, void *data);

typedef void ( *UnloadStateCallback )(void);

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

extern int cursorPosition;
extern int32_t nextNavigationSelection;
extern int32_t menuStateToReturn;

int menuTick(long ms);

int32_t MainMenu_initStateCallback(int32_t tag);

void MainMenu_initialPaintCallback(void);

void MainMenu_repaintCallback(void);

int32_t MainMenu_tickCallback(int32_t tag, void *data);

void MainMenu_unloadStateCallback(void);

int32_t Crawler_initStateCallback(int32_t tag);

void Crawler_initialPaintCallback(void);

void Crawler_repaintCallback(void);

int32_t Crawler_tickCallback(int32_t tag, void *data);

void Crawler_unloadStateCallback(void);

int32_t HelpScreen_initStateCallback(int32_t tag);

void HelpScreen_initialPaintCallback(void);

void HelpScreen_repaintCallback(void);

int32_t HelpScreen_tickCallback(int32_t tag, void *data);

void HelpScreen_unloadStateCallback(void);

int32_t CreditsScreen_initStateCallback(int32_t tag);

void CreditsScreen_initialPaintCallback(void);

void CreditsScreen_repaintCallback(void);

int32_t CreditsScreen_tickCallback(int32_t tag, void *data);

void CreditsScreen_unloadStateCallback(void);

int32_t GameMenu_initStateCallback(int32_t tag);

void GameMenu_initialPaintCallback(void);

void GameMenu_repaintCallback(void);

int32_t GameMenu_tickCallback(int32_t tag, void *data);

void GameMenu_unloadStateCallback(void);

int countLines(void);
void enterState( enum EGameMenuState State );
#define MENU_ITEM_TIME_TO_BECOME_ACTIVE_MS 200

/* 84ms * 6 blinks == ~500ms */
#define MENU_ITEM_TIME_TO_BLINK_MS 84
#endif
