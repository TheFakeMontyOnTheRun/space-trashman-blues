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
extern int cameraRotation;
extern uint8_t enteredFrom;
extern uint8_t playerLocation;

enum EGameMenuState Crawler_tickCallback(enum ECommand cmd, long data) {}

void Crawler_repaintCallback(void) {
    uint8_t prevX;
    uint8_t prevZ;
    struct WorldPosition *pos;
    uint8_t previousLocation = playerLocation;
    uint8_t newCell = 0;
    renderScene();
    graphicsFlush();

    prevX = cameraX;
    prevZ = cameraZ;

    switch (getInput()) {

#ifndef GAMEPAD
        case 'l':
            shutdownGraphics();
            exit(0);
        case '7':
            nextItemInHand();
            HUD_refresh();
            break;

        case '4':
            nextItemInRoom();
            HUD_refresh();
            break;

        case '8':
            useItemInHand();
            updateMapItems();
            HUD_refresh();
            break;

        case '5':
            interactWithItemInRoom();
            updateMapItems();
            HUD_refresh();
            break;

        case '9':
            pickItem();
            HUD_refresh();
            break;

        case '6':
            dropItem();
            HUD_refresh();
            break;
#endif
        case 'q':
            turnLeft();
            break;

        case 'e':
            turnRight();
            break;

        case 'a':
            walkBy(3);
            break;
        case 'd':
            walkBy(1);
            break;
        case 's':
            walkBy(2);
            break;
        case 'w':
            walkBy(0);
            break;
    }
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
            startRoomTransitionAnimation();
#endif
        }
        setPlayerDirection(cameraRotation = (newCell - '0'));
    } else {
        enteredFrom = 0xFF;
    }
}

void onError(const char *mesg) {
    showMessage(mesg);
}

void logDelegate(const char *mesg) {
    showMessage(mesg);
}

void Crawler_initStateCallback(int32_t tag_unused) {
    enteredFrom = 0;
    cameraRotation = 0;
    init();
    initStation();
    focusedItem = getPlayerItems();
    setErrorHandlerCallback(onError);
    setLoggerDelegate(logDelegate);
    initMap();

}

void Crawler_initialPaintCallback(void) {}

void Crawler_unloadStateCallback(int32_t newState) {}

