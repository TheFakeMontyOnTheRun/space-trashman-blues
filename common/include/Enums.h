#ifndef ENUMS_H
#define ENUMS_H


enum EDirection {
    kNorth, kEast, kSouth, kWest
};

enum CrawlerState {
    kCrawlerGameOver = -1,
    kCrawlerQuit = 0,
    kCrawlerGameInProgress = 1
};

enum ECommand {
    kCommandNone,
    kCommandUp,
    kCommandRight,
    kCommandDown,
    kCommandLeft,
    kCommandFire1,
    kCommandFire2,
    kCommandBack,
    kCommandQuit,
    kCommandStrafeLeft,
    kCommandStrafeRight,
    kCommandFire3,
    kCommandFire4
};

enum EGameMenuState {
    kShutdown = -2,
    kResumeCurrentState = -1,
    kMainMenu,
    kPlayGame,
    kHelp,
    kCredits,
    kQuit,
    kBackToGame,
    kEndGame,
    kInspectItem,
    kHackingGame,
    kGoodVictoryEpilogue,
    kBadVictoryEpilogue,
    kGoodGameOverEpilogue,
    kBadGameOverEpilogue,
    kPrologue,
    kEpilogue
};

enum EPresentationState {
    kAppearing,
    kWaitingForInput,
    kConfirmInputBlink1,
    kConfirmInputBlink2,
    kConfirmInputBlink3,
    kConfirmInputBlink4,
    kConfirmInputBlink5,
    kConfirmInputBlink6,
    kFade,
    kRoomTransitioning,
    kEnteringRandomBattle
};

enum ESoundDriver {
    kNoSound, kPcSpeaker, kOpl2Lpt, kAdlib
};

#define MENU_SELECTION_CHANGE_SOUND 0
#define STATE_CHANGE_SOUND 1
#define MAIN_MENU_THEME 7
#endif
