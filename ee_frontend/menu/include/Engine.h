#ifndef ENGINE_H
#define ENGINE_H

typedef void ( *InitStateCallback )(int32_t tag);

typedef void ( *InitialPaintCallback )(void);

typedef void ( *RepaintCallback )(void);

typedef enum EGameMenuState ( *TickCallback )(enum ECommand, long data);

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
extern enum EGameMenuState nextNavigationSelection;
extern enum EGameMenuState menuStateToReturn;

int menuTick(long ms);

void MainMenu_initStateCallback(int32_t tag);

void MainMenu_initialPaintCallback(void);

void MainMenu_repaintCallback(void);

enum EGameMenuState MainMenu_tickCallback(enum ECommand, long data);

void MainMenu_unloadStateCallback(void);

void Crawler_initStateCallback(int32_t tag);

void Crawler_initialPaintCallback(void);

void Crawler_repaintCallback(void);

enum EGameMenuState Crawler_tickCallback(enum ECommand, long data);

void Crawler_unloadStateCallback(void);

void HelpScreen_initStateCallback(int32_t tag);

void HelpScreen_initialPaintCallback(void);

void HelpScreen_repaintCallback(void);

enum EGameMenuState HelpScreen_tickCallback(enum ECommand, long data);

void HelpScreen_unloadStateCallback(void);

void BattleScreen_initStateCallback(int32_t tag);

void BattleScreen_initialPaintCallback(void);

void BattleScreen_repaintCallback(void);

enum EGameMenuState BattleScreen_tickCallback(enum ECommand, long data);

void BattleScreen_unloadStateCallback(void);

void CreditsScreen_initStateCallback(int32_t tag);

void CreditsScreen_initialPaintCallback(void);

void CreditsScreen_repaintCallback(void);

enum EGameMenuState CreditsScreen_tickCallback(enum ECommand, long data);

void CreditsScreen_unloadStateCallback(void);

void GameMenu_initStateCallback(int32_t tag);

void GameMenu_initialPaintCallback(void);

void GameMenu_repaintCallback(void);

enum EGameMenuState GameMenu_tickCallback(enum ECommand, long data);

void GameMenu_unloadStateCallback(void);

void HackingScreen_initStateCallback(int32_t tag);

void HackingScreen_initialPaintCallback(void);

void HackingScreen_repaintCallback(void);

enum EGameMenuState HackingScreen_tickCallback(enum ECommand, long data);

void HackingScreen_unloadStateCallback(void);

int countLines(void);

void enterState( enum EGameMenuState State );

int loopTick(enum ECommand cmd);

void renderTick(long ms);

void redrawHUD();

void shutdownHW();

extern int isRunning;
extern long timeUntilNextState;
extern enum EPresentationState currentPresentationState;
extern struct Bitmap *currentBackgroundBitmap;
extern enum EGameMenuState nextNavigationSelection;
extern enum EGameMenuState currentGameMenuState;
extern const char *mainText;

int menuTick(long delta_time);

extern struct Texture *itemSprites[TOTAL_ITEMS];

extern int currentSelectedItem;

#define MENU_ITEM_TIME_TO_BECOME_ACTIVE_MS 200
#endif
