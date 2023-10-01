#ifndef ENGINE_H
#define ENGINE_H

typedef void ( *InitStateCallback )(int32_t tag);

typedef void ( *InitialPaintCallback )(void);

typedef void ( *RepaintCallback )(void);

typedef enum EGameMenuState ( *TickCallback )(enum ECommand, long data);

typedef void ( *UnloadStateCallback )(int32_t newState);

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

extern int cursorPosition;
extern enum EGameMenuState nextNavigationSelection;
extern enum EGameMenuState menuStateToReturn;

int menuTick(long ms);

void MainMenu_initStateCallback(int32_t tag);

void MainMenu_initialPaintCallback(void);

void MainMenu_repaintCallback(void);

enum EGameMenuState MainMenu_tickCallback(enum ECommand, long data);

void MainMenu_unloadStateCallback(int32_t newState);

void Crawler_initStateCallback(int32_t tag);

void Crawler_initialPaintCallback(void);

void Crawler_repaintCallback(void);

enum EGameMenuState Crawler_tickCallback(enum ECommand, long data);

void Crawler_unloadStateCallback(int32_t newState);

void HelpScreen_initStateCallback(int32_t tag);

void HelpScreen_initialPaintCallback(void);

void HelpScreen_repaintCallback(void);

enum EGameMenuState HelpScreen_tickCallback(enum ECommand, long data);

void HelpScreen_unloadStateCallback(int32_t newState);

void CreditsScreen_initStateCallback(int32_t tag);

void CreditsScreen_initialPaintCallback(void);

void CreditsScreen_repaintCallback(void);

enum EGameMenuState CreditsScreen_tickCallback(enum ECommand, long data);

void CreditsScreen_unloadStateCallback(int32_t newState);

void GameMenu_initStateCallback(int32_t tag);

void GameMenu_initialPaintCallback(void);

void GameMenu_repaintCallback(void);

enum EGameMenuState GameMenu_tickCallback(enum ECommand, long data);

void GameMenu_unloadStateCallback(int32_t newState);

void HackingScreen_initStateCallback(int32_t tag);

void HackingScreen_initialPaintCallback(void);

void HackingScreen_repaintCallback(void);

enum EGameMenuState HackingScreen_tickCallback(enum ECommand, long data);

void HackingScreen_unloadStateCallback(int32_t newState);

int countLines(void);

void enterState(enum EGameMenuState State);

int loopTick(enum ECommand cmd);

void renderTick(long ms);

void redrawHUD(void);

void shutdownHW(void);

extern int isRunning;
extern long timeUntilNextState;
extern enum EPresentationState currentPresentationState;
extern enum EGameMenuState nextNavigationSelection;
extern enum EGameMenuState currentGameMenuState;
extern const char *mainText;
extern char *textBuffer;

int menuTick(long delta_time);

#define MENU_ITEM_TIME_TO_BECOME_ACTIVE_MS 200
#define TEXT_BUFFER_SIZE (40 * 25)

#endif /* ENGINE_H */
