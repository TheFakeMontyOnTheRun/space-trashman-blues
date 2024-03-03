/*
   Created by monty on 01-10-2023.
*/
#include <stdlib.h>
#include <stdint.h>
#include "Enums.h"
#include "UI.h"
#include "Renderer.h"


#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "Core.h"
#include "Derelict.h"
#include "Enums.h"
#include "Renderer.h"
#include "map.h"

int8_t map[32][32];

#ifdef SUPPORTS_HACKING_MINIGAME

#include "HackingMinigame.h"
#include "Engine.h"

#endif

#ifdef MSDOS
#include "Common.h"
#endif

#ifndef EMBEDDED_DATA
#include "PackedFileReader.h"
#endif

extern int8_t stencilHigh[XRES];
extern struct ObjectNode *focusedItem;
extern struct ObjectNode *roomItem;
extern uint8_t accessGrantedToSafe;
extern int8_t cameraX;
extern int8_t cameraZ;
extern int8_t cameraRotation;
extern uint8_t enteredFrom;
extern uint8_t playerLocation;

uint8_t needs3dRefresh;

#ifdef SUPPORTS_ROOM_TRANSITION_ANIMATION
uint8_t roomTransitionAnimationStep = 0;
#endif


enum EGameMenuState Crawler_tickCallback(enum ECommand cmd, long data) {
    uint8_t prevX;
    uint8_t prevZ;
    struct WorldPosition *pos;
    uint8_t previousLocation = playerLocation;
    uint8_t newCell = 0;

#ifdef SUPPORTS_ROOM_TRANSITION_ANIMATION
    if (roomTransitionAnimationStep) {
        needs3dRefresh = 1;
        return kResumeCurrentState;
    }
#endif

    prevX = cameraX;
    prevZ = cameraZ;

    switch (cmd) {
        case kCommandFire5:
            nextItemInHand();
            break;

        case kCommandFire6:
            nextItemInRoom();
            break;

        case kCommandFire1:
            useItemInHand();
            break;

        case kCommandFire2:
            interactWithItemInRoom();
            break;

        case kCommandFire3:
            pickItem();
            break;

        case kCommandFire4:
            dropItem();
            break;
        default:
            goto handle_directions;
    }
    updateMapItems();
    needs3dRefresh = 1;

    if (!waitForKey) {
        HUD_refresh();
    }

    return kResumeCurrentState;
handle_directions:
    switch (cmd) {
        case kCommandLeft:
            turnLeft();
            break;

        case kCommandRight:
            turnRight();
            break;

        case kCommandStrafeLeft:
            walkBy(3);
            break;
        case kCommandStrafeRight:
            walkBy(1);
            break;
        case kCommandDown:
            walkBy(2);
            break;
        case kCommandUp:
            walkBy(0);
            break;
        case kCommandNone:
            needs3dRefresh = 0;
            return kResumeCurrentState;
    }
    needs3dRefresh = 1;
    cameraRotation = getPlayerDirection();
    pos = getPlayerPosition();

    cameraX = pos->x;
    cameraZ = pos->y;

    switch (cameraRotation) {
        case 0:
            newCell = map[cameraZ - 2][cameraX];
            break;
        case 1:
            newCell = map[cameraZ][cameraX + 2];
            break;
        case 2:
            newCell = map[cameraZ + 2][cameraX];
            break;
        case 3:
            newCell = map[cameraZ][cameraX - 2];
            break;
    }

    newCell = newCell & 127;

    if (patterns[newCell - RLE_THRESHOLD].blockMovement) {
        pos->x = cameraX = prevX;
        pos->y = cameraZ = prevZ;
        setPlayerPosition(pos);
    }

    /* unlike MX, we are signaling from the origin into the new room. MX allows for the movement and then searches where
     * did the player came from - hence the "opposite direction" there */

    if (newCell > ('0' - 1) && newCell < ('3' + 1)) {
        enteredFrom = newCell - '0';
        moveBy(enteredFrom);
    }

    if (playerLocation != previousLocation) {
        initMap();

        if (newCell == NEUTRAL_CELL) {
            newCell = '0';
#ifdef SUPPORTS_ROOM_TRANSITION_ANIMATION
        } else {
            roomTransitionAnimationStep = 30;
#endif
        }
        setPlayerDirection(cameraRotation = (newCell - '0'));
    } else {
        enteredFrom = 0xFF;
    }
    return kResumeCurrentState;
}

void Crawler_repaintCallback(void) {

    if (firstFrameOnCurrentState) {
        clearScreen();
        HUD_initialPaint();
    }

    if (!needs3dRefresh) {
        return;
    }
#ifdef SUPPORTS_ROOM_TRANSITION_ANIMATION
    if (roomTransitionAnimationStep) {
        uint8_t y = roomTransitionAnimationStep--;
        uint8_t x;
        uint8_t val = 95 + (MAP_SIZE_Y - y);

        if (roomTransitionAnimationStep == 0) {
            HUD_initialPaint();
            clearGraphics();
            renderScene();
            return;
        }

        vLine(y, y, val, 1);
        vLine(val, y, val, 1);

        for (x = y; x < val; ++x) {
            graphicsPut(x, y);
            graphicsPut(x, val);

            /* door opening */
            graphicsPut(x, 95 - 3 * (MAP_SIZE_Y - y));
        }
    } else
#endif
    {
        renderScene();
    }
}

void Crawler_initStateCallback(enum EGameMenuState tag_unused) {
    enteredFrom = 0;
    cameraRotation = 0;
    initStation();
    focusedItem = getPlayerItems();
    setErrorHandlerCallback(showMessage);
    setLoggerDelegate(showMessage);
    initMap();
    needs3dRefresh = 1;
}

void Crawler_unloadStateCallback(enum EGameMenuState newState) {
    needs3dRefresh = 0;
}

