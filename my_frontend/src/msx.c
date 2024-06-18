#include <stdint.h>

#include "Enums.h"
#include "Core.h"
#include "Renderer.h"

#include "TMS9918.h"
#include "AY-3-8910.h"
#include "UI.h"
#include "KeyboardUI.h"

/* Sadly, I can't include conio.h - otherwise, I would get errors when building on OSX */
int kbhit(void);
int getch(void);
extern uint8_t firstFrameOnCurrentState;
enum ESoundDriver soundDriver = kAY38910;

/*  Required since we have our own memory allocator abstraction */
uint16_t heap = 0;

void initHW(int argc, char **argv) {
    (void)argc;
    (void)argv;
    initTMS9918();
    initAY38910();
    initKeyboardUI();
    needsToRedrawVisibleMeshes = 0;
}

void handleSystemEvents(void) {}

enum ECommand getInput(void) {

    if (!kbhit()) {
        return kCommandNone;
    }

    uint8_t input = getch();

    performAction();

    if (waitForKey) {
        if (input == '2') {
            waitForKey = 0;
            firstFrameOnCurrentState = 1;
            needsToRedrawVisibleMeshes = 1;
            return kCommandNone;
        }
        return kCommandNone;
    }

    switch (input) {
        case 30:
            return kCommandUp;
        case 31:
            return kCommandDown;
        case 29:
            return kCommandLeft;
        case 28:
            return kCommandRight;
        case 'z':
            return kCommandStrafeLeft;
        case 'x':
            return kCommandStrafeRight;
        case '1':
            return kCommandFire1;
        case '2':
            return kCommandFire2;
        case '3':
            return kCommandFire3;
        case '4':
            return kCommandFire4;
        case '5':
            return kCommandFire5;
        case '6':
            return kCommandFire6;
    }
    return input;
}

void startFrame(int x, int y, int width, int height) {
    (void)x;
    (void)y;
    (void)width;
    (void)height;
}

void endFrame(void) {
    if (needsToRedrawVisibleMeshes) {
        flush3DBuffer();
    }
}
