#ifdef WIN32
#include "Win32Int.h"
#else
#ifndef SMD
#include <stdint.h>
#include <stdio.h>
#else
#include <genesis.h>
typedef unsigned long size_t;
#endif
#endif

#include <stddef.h>
#include <string.h>

#include "Common.h"
#include "FixP.h"
#include "Core.h"
#include "Enums.h"
#include "Renderer.h"
#include "Engine.h"
#include "PackedFileReader.h"
#include "CTile3DProperties.h"

extern int8_t map[32][32];

extern int8_t stencilHigh[XRES];

struct ObjectNode *focusedItem = NULL;
struct ObjectNode *roomItem = NULL;

extern uint8_t accessGrantedToSafe;

extern struct MapWithCharKey tileProperties;
extern struct MapWithCharKey customMeshes;


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
    int x, y;
    const uint8_t *head;
    char buffer[32];
    uint16_t current;

    roomItem = getRoom(getPlayerRoom())->itemsPresent->next;

    sprintf(&buffer[0], "map%d.txt", getPlayerRoom());

    struct StaticBuffer datafile = loadBinaryFileFromPath(&buffer[0]);
    head = datafile.data;

    for (y = 0; y < 32; ++y) {
        for (x = 0; x < 32; ++x) {

            current = *head;


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
                current = '.';
            }

            map[y][x] = current;
            ++head;
        }
        ++head; // line break
    }

    disposeDiskBuffer(datafile);

    sprintf(&buffer[0], "props%d.bin", getPlayerRoom());
    loadPropertyList(&buffer[0], &tileProperties, &customMeshes);

    //updateMapItems();
    HUD_initialPaint();
}
