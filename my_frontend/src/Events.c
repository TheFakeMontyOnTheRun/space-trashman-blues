#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "Common.h"
#include "Core.h"
#include "Enums.h"
#include "Renderer.h"
#include "map-data.h"
#include "Engine.h"

#ifdef MSDOS
#include "Common.h"
#endif

#ifndef EMBEDDED_DATA
#include "PackedFileReader.h"
#endif
extern int8_t map[32][32];

extern int8_t stencilHigh[XRES];

struct ObjectNode *focusedItem = NULL;
struct ObjectNode *roomItem = NULL;

extern uint8_t accessGrantedToSafe;

int8_t cameraX = 33;
int8_t cameraZ = 22;
extern int8_t cameraRotation;

uint8_t enteredFrom = 0xFF;

extern uint8_t playerLocation;

void pickItem(void) {
    struct Room *room = getRoom(getPlayerRoom());

    if (roomItem && roomItem->item) {
        struct Item *itemToPick = getItem(roomItem->item);
        if (itemToPick != NULL) {

            if (!strcmp(itemToPick->name, "computer-terminal")) {

#ifdef SUPPORTS_HACKING_MINIGAME
                enterState(kHackingGame);
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
    if (roomItem && focusedItem) {
        struct Item *itemToPick = getItem(roomItem->item);
        struct Item *item = getItem(focusedItem->item);
        if (itemToPick && item && item->useWithCallback) {
            item->useWithCallback(item, itemToPick);
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
    headEnd = head + datafile.size;
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
            if (repetitions < 1) {

                if (head == headEnd) {
                    goto done_loading;
                }

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
            if (head == headEnd) {
                goto done_loading;
            }

            current = *head;
#endif

            if ((current == 's' && enteredFrom == 0) ||
                (current == 'w' && enteredFrom == 1) ||
                (current == 'n' && enteredFrom == 2) ||
                (current == 'e' && enteredFrom == 3)) {

                struct WorldPosition newPos;
                cameraX = newPos.x = x;
                cameraZ = newPos.y = y;
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
}
