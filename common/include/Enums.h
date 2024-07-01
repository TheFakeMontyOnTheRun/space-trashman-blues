#ifndef ENUMS_H
#define ENUMS_H

#ifdef __MWERKS__
#define EMIT_QUIT_OPTION
#endif

enum EDirection {
    kNorth, kEast, kSouth, kWest
};

enum CrawlerState {
    kCrawlerGameOver = -1,
    kCrawlerQuit = 0,
    kCrawlerGameInProgress = 1,
    kCrawlerGameFinished = 2
};

enum ECommand {
    kCommandNone,
    kCommandUp,
    kCommandRight,
    kCommandDown,
    kCommandLeft,
    kCommandFire1,
    kCommandFire2,
    kCommandFire3,
    kCommandFire4,
    kCommandFire5,
    kCommandFire6,
    kCommandBack,
#ifdef EMIT_QUIT_OPTION    
    kCommandQuit,
#endif
    kCommandStrafeLeft,
    kCommandStrafeRight
};

enum EGameMenuState {
    kShutdown = -2,
    kResumeCurrentState = -1,
    kMainMenu,
    kPlayGame,
    kHelp,
    kCredits,
#ifdef EMIT_QUIT_OPTION
    kQuit,
#endif
    kBackToGame,
    kEndGame,
#ifdef SUPPORTS_HACKING_MINIGAME
    kHackingGame,
#endif
    kGoodVictoryEpilogue,
    kBadVictoryEpilogue,
    kGoodGameOverEpilogue,
    kBadGameOverEpilogue,
    kPrologue,
    kEpilogue
};

enum EPresentationState {
    kWaitingForInput,
    kRoomTransitioning
};

enum ESoundDriver {
    kNoSound, kPcSpeaker, kOpl2Lpt, kAdlib, kSN76489, kAY38910
};

#define MENU_SELECTION_CHANGE_SOUND 1
#define STATE_CHANGE_SOUND 2
#define MAIN_MENU_THEME 3
#endif
