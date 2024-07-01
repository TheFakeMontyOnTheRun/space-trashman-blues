#ifndef ENGINE_H
#define ENGINE_H

typedef void ( *InitStateCallback )(enum EGameMenuState tag);

typedef void ( *RepaintCallback )(void);

typedef enum EGameMenuState ( *TickCallback )(enum ECommand, void* data);

typedef void ( *UnloadStateCallback )(enum EGameMenuState newState);

#define kNonExpiringPresentationState 0xFFFF
#define kDefaultPresentationStateInterval 2000
#define kMenuStateUnchanged -1

uint8_t menuTick(long ms);

void MainMenu_initStateCallback(enum EGameMenuState tag);

void MainMenu_repaintCallback(void);

enum EGameMenuState MainMenu_tickCallback(enum ECommand, void* data);

void MainMenu_unloadStateCallback(enum EGameMenuState newState);

void Crawler_initStateCallback(enum EGameMenuState tag);

void Crawler_repaintCallback(void);

enum EGameMenuState Crawler_tickCallback(enum ECommand, void* data);

void Crawler_unloadStateCallback(enum EGameMenuState newState);

void HelpScreen_initStateCallback(enum EGameMenuState tag);

void HelpScreen_repaintCallback(void);

enum EGameMenuState HelpScreen_tickCallback(enum ECommand, void* data);

void HelpScreen_unloadStateCallback(enum EGameMenuState newState);

void CreditsScreen_initStateCallback(enum EGameMenuState tag);

void CreditsScreen_repaintCallback(void);

enum EGameMenuState CreditsScreen_tickCallback(enum ECommand, void* data);

void CreditsScreen_unloadStateCallback(enum EGameMenuState newState);

void GameMenu_initStateCallback(enum EGameMenuState tag);

void GameMenu_repaintCallback(void);

enum EGameMenuState GameMenu_tickCallback(enum ECommand, void* data);

void GameMenu_unloadStateCallback(enum EGameMenuState newState);

void HackingScreen_initStateCallback(enum EGameMenuState tag);

void HackingScreen_repaintCallback(void);

enum EGameMenuState HackingScreen_tickCallback(enum ECommand, void* data);

void HackingScreen_unloadStateCallback(enum EGameMenuState newState);

void enterState(enum EGameMenuState State);

int loopTick(enum ECommand cmd);

void renderTick(long ms);

void redrawHUD(void);

void shutdownHW(void);

enum ECommand getInput(void);

void handleSystemEvents(void);

#define MENU_ITEM_TIME_TO_BECOME_ACTIVE_MS 200

extern InitStateCallback initStateCallback;
extern RepaintCallback repaintCallback;
extern TickCallback tickCallback;
extern UnloadStateCallback unloadStateCallback;
extern uint8_t isRunning;
extern int8_t cursorPosition;
extern long timeUntilNextState;
extern enum EPresentationState currentPresentationState;
extern enum EGameMenuState nextNavigationSelection;
extern enum EGameMenuState menuStateToReturn;
extern enum EGameMenuState currentGameMenuState;
extern uint8_t firstFrameOnCurrentState;

#endif /* ENGINE_H */
