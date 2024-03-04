#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sms.h>
#include <stdio.h>

#include "Enums.h"
#include "Core.h"
#include "Derelict.h"
#include "Renderer.h"
#include "GamepadUI.h"
#include "TMS9918.h"
#include "SN76489.h"
#include "SoundSystem.h"

#define COOLDOWN_MAX 0x1F

/*  Required since we have our own memory allocator abstraction */
uint16_t heap = 0;

uint8_t cooldown;

extern uint8_t firstFrameOnCurrentState;
extern uint8_t waitForKey;

uint8_t updateDirection;

extern uint8_t cursorPosition;

enum ESoundDriver soundDriver = kNoSound;

extern enum EGameMenuState currentGameMenuState;

void initHW(void) {
    initGamepadUI();
    initTMS9918();
    initSN76489();
    cooldown = COOLDOWN_MAX;
    updateDirection = 1;
    needs3dRefresh = 0;
}

void handleSystemEvents(void) {}

enum ECommand getInput(void) {
    int key = read_joypad1();

    if (cooldown) {
        cooldown--;
    }

    if (key & JOY_UP && !cooldown) {
        cooldown = COOLDOWN_MAX;
        return kCommandUp;
    }

    if (key & JOY_LEFT && !cooldown) {
        cooldown = COOLDOWN_MAX;
        if (key & JOY_FIREB) {
            return kCommandStrafeLeft;
        } else {
            updateDirection = 1;
            return kCommandLeft;
        }
    }

    if (key & JOY_RIGHT && !cooldown) {
        cooldown = COOLDOWN_MAX;
        if (key & JOY_FIREB) {
            return kCommandStrafeRight;
        } else {
            updateDirection = 1;
            return kCommandRight;
        }
    }

    if (key & JOY_DOWN && !cooldown) {
        cooldown = COOLDOWN_MAX;
        return kCommandDown;
    }

    if ((key & JOY_FIREA) && !cooldown ) {
        if (currentGameMenuState == kPlayGame) {
            playSound(3);
            cooldown = COOLDOWN_MAX;

            if (waitForKey) {
                waitForKey = 0;
                firstFrameOnCurrentState = 1;
                needs3dRefresh = 1;
                return kCommandNone;
            }

            return performActionJoypad();
        } else {
            return kCommandFire1;
        }
    }

    if ((key & JOY_FIREB) && !cooldown ) {
        if (currentGameMenuState == kPlayGame) {
            cursorPosition = (cursorPosition + 1);
            playSound(2);
            if (cursorPosition >= 6) {
                cursorPosition = 0;
            }

            HUD_initialPaint();
            cooldown = COOLDOWN_MAX;
            return kCommandNone;
        } else {
            return kCommandFire1;
        }
    }

    return kCommandNone;
}

void graphicsFlush(void) {
    if (needs3dRefresh) {
        needs3dRefresh = 0;
        flush3DBuffer();
        if (updateDirection) {
            char direction[8] = {'N', 0, 'E', 0, 'S', 0, 'W', 0};
            updateDirection = 0;
            writeStrWithLimit(12, 17, &direction[getPlayerDirection() * 2], 31, 2, 0);
        }
    }
}