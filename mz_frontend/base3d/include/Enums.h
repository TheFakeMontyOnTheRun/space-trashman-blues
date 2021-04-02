#ifndef ENUMS_H
#define ENUMS_H


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
    kMainMenu,
    kHelp,
    kCredits,
    kQuit,
    kInterrogate_Sofia
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
    kFade
};

enum ESoundDriver {
    kNoSound, kPcSpeaker, kOpl2Lpt, kAdlib, kTandy, kCovox
};

#define MENU_SELECTION_CHANGE_SOUND 0
#define STATE_CHANGE_SOUND 1
#define INFORMATION_ACQUIRED_SOUND 2
#define FAILED_TO_GET_INFORMATION_SOUND 3
#define PLAYER_GOT_DETECTED_SOUND 4
#define PLAYER_FIRING_GUN 5
#define ENEMY_FIRING_GUN 6
#define PLAYER_GET_HURT_SOUND 7
#endif
