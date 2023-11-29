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

#define COOLDOWN_MAX 0xFF

/*  Required since we have our own memory allocator abstraction */
uint16_t heap = 0;

uint8_t cooldown;

uint8_t updateDirection;

extern uint8_t cursorPosition;

enum ESoundDriver soundDriver = kNoSound;

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

    if (key & JOY_UP) {
        return kCommandUp;
    }

    if (key & JOY_LEFT) {
        if (key & JOY_FIREB) {
            return kCommandStrafeLeft;
        } else {
            updateDirection = 1;
            return kCommandLeft;
        }
    }

    if (key & JOY_RIGHT) {
        if (key & JOY_FIREB) {
            return kCommandStrafeRight;
        } else {
            updateDirection = 1;
            return kCommandRight;
        }
    }

    if (key & JOY_DOWN) {
        return kCommandDown;
    }

    if ((key & JOY_FIREA) /* && !cooldown */) {
        cooldown = COOLDOWN_MAX;
        return performActionJoypad();
    }

    if ((key & JOY_FIREB) /* && !cooldown */ ) {
        cursorPosition = (cursorPosition + 1);

        if (cursorPosition >= 6) {
            cursorPosition = 0;
        }

        HUD_refresh();
        cooldown = COOLDOWN_MAX;
        return kCommandNone;
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