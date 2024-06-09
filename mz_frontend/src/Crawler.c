/*
   Created by monty on 01-10-2023.
*/
#include <stdlib.h>
#include <stdint.h>

#include "Common.h"
#include "Enums.h"
#include "UI.h"
#include "Renderer.h"
#include "Core.h"
#include "Derelict.h"
#include "map.h"
#include "Engine.h"
#include "map.h"
#include "MapWithCharKey.h"
#include "FixP.h"
#include "CTile3DProperties.h"


int8_t map[32][32];

extern int8_t stencilHigh[XRES];
extern struct ObjectNode *focusedItem;
extern struct ObjectNode *roomItem;
extern uint8_t accessGrantedToSafe;
extern int8_t cameraX;
extern int8_t cameraZ;
extern int8_t cameraRotation;
extern uint8_t enteredFrom;
extern uint8_t playerLocation;
extern struct MapWithCharKey tileProperties;

uint8_t needsToRedrawVisibleMeshes;

#ifdef SUPPORTS_ROOM_TRANSITION_ANIMATION
uint8_t roomTransitionAnimationStep = 0;
#endif

void HUD_refresh(void) {
    if (!needsToRedrawHUD) {
        return;
    }

    needsToRedrawHUD = 0;

    drawWindow(0,
               (128 / 8),
               (XRES_FRAMEBUFFER / 8) / 2,
               (YRES_FRAMEBUFFER / 8) - 17,
               "Status");

    drawTextAt(1, YRES_TEXT - 7, "In room:", 2);
    drawTextAt(1, YRES_TEXT - 4, "In hand:", 2);

    struct Item *item;

    /* Display "In room" item */
    if (roomItem != NULL) {
        item = getItem(roomItem->item);
        if (item->active) {
            drawTextAt(1, YRES_TEXT - 6, "*", 2);
        }
        drawTextAtWithMarginWithFiltering(2, YRES_TEXT - 6, (XRES_FRAMEBUFFER) / 2, item->name,  2, ' ');
    } else {
        drawTextAt(2, YRES_TEXT - 6, "Nothing", 2);
    }

    /* Display "In hand" item */
    if (focusedItem != NULL) {
        item = getItem(focusedItem->item);
        if (item->active) {
            drawTextAt(1, YRES_TEXT - 3, "*", 1);
        }
        drawTextAtWithMarginWithFiltering(2, YRES_TEXT - 3, (XRES_FRAMEBUFFER) / 2, item->name,  2, ' ');
    } else {
        drawTextAt(2, YRES_TEXT - 3, "Nothing", 2);
    }
}

enum EGameMenuState Crawler_tickCallback(enum ECommand cmd, void* data) {
    (void)data;
    uint8_t prevX;
    uint8_t prevZ;
    struct WorldPosition *pos;
    uint8_t previousLocation = playerLocation;
    uint8_t newCell = 0;

#ifdef SUPPORTS_ROOM_TRANSITION_ANIMATION
    if (roomTransitionAnimationStep) {
        needsToRedrawVisibleMeshes = 1;
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
    needsToRedrawVisibleMeshes = 1;

    if (!waitForKey) {
        needsToRedrawHUD = 1;
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
            return kResumeCurrentState;
    }
    needsToRedrawVisibleMeshes = 1;
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

    struct CTile3DProperties *prop =
            (struct CTile3DProperties *) getFromMap(&tileProperties, newCell);


    if (prop->mBlockMovement) {
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
        redrawMap = needsToRedrawHUD = 1;
        HUD_initialPaint();
    }

    HUD_refresh();
    drawMap();

    if (!needsToRedrawVisibleMeshes) {
        return;
    }
#ifdef SUPPORTS_ROOM_TRANSITION_ANIMATION
    if (roomTransitionAnimationStep) {
        roomTransitionAnimationStep = 0;
        for (uint16_t y = 32; y >= 2; --y) {
            clearGraphics();

            vLine(y, y, 95 + (32 - y), 1);
            vLine(95 + (32 - y), y, 95 + (32 - y), 1);

            for (uint16_t x = y; x < (95 + (32 - y)); ++x) {
                graphicsPut(x, y, 7);
                graphicsPut(x, 95 + (32 - y), 7);

                //door opening
                vLine(x, y, 95 - 3 * (32 - y), 7);
            }
        }
    } else
#endif
    {
        renderScene();
    }
}

void Crawler_initStateCallback(enum EGameMenuState tag) {
    (void)tag;

    enteredFrom = 0;
    cameraRotation = 0;
    initStation();
    focusedItem = getPlayerItems();
    setErrorHandlerCallback(showMessage);
    setLoggerDelegate(showMessage);
    initMap();
    needsToRedrawVisibleMeshes = 1;
}

void Crawler_unloadStateCallback(enum EGameMenuState newState) {
    (void)newState;
    needsToRedrawVisibleMeshes = 0;
}

