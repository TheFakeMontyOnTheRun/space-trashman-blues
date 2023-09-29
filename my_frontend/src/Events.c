#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "Core.h"
#include "Derelict.h"
#include "Renderer.h"
#include "map.h"
#include "map-data.h"

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

/*  Required since we have our own memory allocator abstraction */
uint16_t heap = 0;

extern int8_t stencilHigh[XRES];

struct ObjectNode *focusedItem = NULL;
struct ObjectNode *roomItem = NULL;

extern uint8_t accessGrantedToSafe;

int8_t cameraX = 33;
int8_t cameraZ = 22;
int8_t cameraRotation = 0;
uint8_t running = 1;

uint8_t enteredFrom = 0xFF;

extern uint8_t playerLocation;

void pickItem(void) {
    struct Room *room = getRoom(getPlayerRoom());

    if (roomItem && roomItem->item) {
        struct Item *itemToPick = getItem(roomItem->item);
        if (itemToPick != NULL) {

            if (!strcmp(itemToPick->name, "digital-safe")) {

#ifdef SUPPORTS_HACKING_MINIGAME
                runHackingMinigame();
#else
                accessGrantedToSafe = TRUE;
#endif
                return;
            }

            if (itemToPick->pickable) {

                uint8_t pattern = map[itemToPick->position.y][itemToPick->position.x];
                map[itemToPick->position.y][itemToPick->position.x] = pattern & 127;

                pickObject(itemToPick);
                focusedItem = roomItem;
                roomItem = room->itemsPresent->next;

            } else {
                useObjectNamed(itemToPick->name);
            }
        }
    }
}

void dropItem(void) {

    struct Item *item = NULL;

    if (focusedItem != NULL) {
        item = getItem(focusedItem->item);
    }

    if (item != NULL) {
        uint8_t pattern;
        struct WorldPosition *pos = getPlayerPosition();

        dropObjectToRoom(getPlayerRoom(), item);

        focusedItem = getPlayerItems();

        roomItem = &objectNodes[item->index];

        switch (cameraRotation) {
            case 0:
                item->position.x = pos->x;
                item->position.y = pos->y - 3;
                break;

            case 1:
                item->position.x = pos->x + 3;
                item->position.y = pos->y;
                break;

            case 2:
                item->position.x = pos->x;
                item->position.y = pos->y + 3;
                break;

            case 3:
                item->position.x = pos->x - 3;
                item->position.y = pos->y;
                break;
        }
        pattern = map[item->position.y][item->position.x];
        map[item->position.y][item->position.x] = pattern | 128;
    }
}

void nextItemInRoom(void) {
    struct Room *room = getRoom(getPlayerRoom());

    if (roomItem == NULL) {
        return;
    }

    roomItem = roomItem->next;

    if (!roomItem) {
        roomItem = room->itemsPresent;
    }

    if (roomItem->item == 0) {
        roomItem = roomItem->next;
    }
}

void interactWithItemInRoom(void) {
    struct Item *item = NULL;
    struct Item *itemToPick = NULL;

    if (roomItem != NULL) {
        itemToPick = getItem(roomItem->item);
        if (itemToPick != NULL) {
            if (focusedItem != NULL) {
                item = getItem(focusedItem->item);
                if (item != NULL && item->useWithCallback) {
                    item->useWithCallback(item, itemToPick);
                }
            }
        }
    }
}

void useItemInHand(void) {
    useObjectNamed(getItem(focusedItem->item)->name);
}

void nextItemInHand(void) {
    focusedItem = focusedItem->next;

    if (!focusedItem) {
        focusedItem = getPlayerItems();
    }
}

void initMap(void) {
    uint8_t x, y, c;
    const uint8_t *head;
    const uint8_t *headEnd;
    uint8_t current = NEUTRAL_CELL;

    uint16_t offsetOnDataStrip = 0;
    int16_t repetitions = -1;

    memset(stencilHigh, 0, XRES);

#ifdef EMBEDDED_DATA
    for (c = 0; c < playerLocation; ++c) {
        offsetOnDataStrip += dataPositions[c];
    }

    head = &data[offsetOnDataStrip];

    /* the last location */
    if (dataPositions[playerLocation + 1] == 0) {
        size_t extra = sizeof(data) - 1;
        headEnd = &data[0] + extra;
    } else {
        headEnd = head + (dataPositions[playerLocation]);
    }
#else
    struct StaticBuffer datafile = loadBinaryFileFromPath(playerLocation);
    head = datafile.data;
#endif
    /* first item in the list is always a dummy */
    roomItem = getRoom(playerLocation)->itemsPresent->next;

#ifdef OPTIMIZATION_BLOCK_CELL
    memset(map, BLOCK_CELL, MAP_SIZE_X * MAP_SIZE_Y);
#else
    memset(map, NEUTRAL_CELL, MAP_SIZE_X * MAP_SIZE_Y);
#endif
    for (y = 0; y < MAP_SIZE_Y; ++y) {
        for (x = 0; x < MAP_SIZE_X; ++x) {

#ifdef RLE_COMPRESSED_MAPS
            if (head == headEnd) {
                goto done_loading;
            }

            if (repetitions < 1) {
                repetitions = *head;

                if (repetitions >= RLE_THRESHOLD) {
                    ++head;
                    current = repetitions;
                    repetitions = 0;
                } else {
                    ++head;
                    current = *head;
                    ++head;
                    repetitions--;
                }
            } else {
                repetitions--;
            }
#else
            current = *head;
#endif

            if ((current == 's' && enteredFrom == 0) ||
                (current == 'w' && enteredFrom == 1) ||
                (current == 'n' && enteredFrom == 2) ||
                (current == 'e' && enteredFrom == 3)) {

                struct WorldPosition newPos;
                cameraX = x;
                cameraZ = y;
                newPos.x = x;
                newPos.y = y;
                setPlayerPosition(&newPos);
                enteredFrom = 0xFF;
                current = NEUTRAL_CELL;
            }

            map[y][x] = current;
#ifndef EMBEDDED_DATA
            ++head;
#endif
        }
#ifndef EMBEDDED_DATA
        ++head; /*  line break */
#endif
    }

    done_loading:
#ifndef EMBEDDED_DATA
    disposeDiskBuffer(datafile);
#endif

    updateMapItems();
    HUD_initialPaint();
}

void tickRenderer(void) {
    uint8_t prevX;
    uint8_t prevZ;
    struct WorldPosition *pos;
    uint8_t previousLocation = playerLocation;
    uint8_t newCell = 0;
    renderScene();
    graphicsFlush();

    prevX = cameraX;
    prevZ = cameraZ;

    switch (getKey()) {

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

void initState() {
    running = 1;
    enteredFrom = 0;
    cameraRotation = 0;
    init();
    initStation();
    focusedItem = getPlayerItems();
    setErrorHandlerCallback(onError);
    setLoggerDelegate(logDelegate);
}

