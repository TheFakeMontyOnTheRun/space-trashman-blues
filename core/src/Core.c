/*
Created by Daniel Monteiro on 2019-07-26.
*/

#ifndef DONT_INCLUDE

#ifndef SMD
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#endif

#ifndef CPC_PLATFORM
#ifndef NGB
#include <unistd.h>
#endif
#endif

#else
#include <genesis.h>
#endif

#include "Core.h"
#endif


int roomCount = 1; /* there's an implicit dummy first */
struct Room rooms[TOTAL_ROOMS];
int itemsCount = 0;
struct Item item[TOTAL_ITEMS];
struct ObjectNode objectNodes[TOTAL_ITEMS];
struct ObjectNode collectedObjectHead;
struct ObjectNode roomObjectHeads[TOTAL_ROOMS];
struct ObjectNode *collectedObject = NULL;
int playerLocation = 1;
int playerDirection;
int playerRank;
int gameStatus;
int playerHealth = 100;
struct WorldPosition playerPosition;
ErrorHandlerCallback errorHandlerCallback = NULL;

void writeToLog(const char *errorMsg) {
#ifdef HAS_STDIO
    if (errorHandlerCallback == NULL) {
        puts("-------");
        puts(errorMsg);
        puts("--!!---");
    } else {
        errorHandlerCallback(errorMsg);
    }
#else
    /* We assume the error handler was already set. If you don't have stdio, you probably know what you're doing */
    errorHandlerCallback(errorMsg);
#endif
}

struct Item* addItem(char *description,
                     char *info,
#ifdef ITEMS_HAVE_WEIGHT
                     int weight,
#endif
                     int pickable,
                     int positionX,
                     int positionY) {

    struct Item* toReturn = &item[itemsCount];

    toReturn->index = itemsCount++;
    toReturn->name = description;
    toReturn->info = info;
    toReturn->pickable = pickable;
    toReturn->position.x = positionX;
    toReturn->position.y = positionY;

    return toReturn;
}

struct Room *addRoom(
        char *name,
#ifdef INCLUDE_ROOM_DESCRIPTIONS
        char *info,
#endif
                int sizeX, int sizeY, int chanceOfRandomBattle, int connections[6]) {

    struct Room* toReturn = &rooms[roomCount];
    toReturn->name = name;
#ifdef INCLUDE_ROOM_DESCRIPTIONS
    toReturn->info = info;
#endif
    toReturn->sizeX = sizeX;
    toReturn->sizeY = sizeY;
    toReturn->chanceOfRandomBattle = chanceOfRandomBattle;
    toReturn->connections[0] = connections[0];
    toReturn->connections[1] = connections[1];
    toReturn->connections[2] = connections[2];
    toReturn->connections[3] = connections[3];
    toReturn->connections[4] = connections[4];
    toReturn->connections[5] = connections[5];

    /* add list head to make manipulations easier */
    toReturn->itemsPresent = &roomObjectHeads[roomCount++];
    memset(toReturn->itemsPresent, 0, sizeof(struct ObjectNode) );

    return toReturn;
}

LogDelegate defaultLogger = writeToLog;

void setErrorHandlerCallback(ErrorHandlerCallback callback) {
    errorHandlerCallback = callback;
}

struct WorldPosition *getPlayerPosition(void) {
    return &playerPosition;
}

int getPlayerHealth(void) {
    return playerHealth;
}

void setPlayerHealth(int health) {
    playerHealth = health;
}

void setPlayerPosition(struct WorldPosition* pos) {
    playerPosition.x = pos->x;
    playerPosition.y = pos->y;
}

int isCloseToObject(struct WorldPosition* pos, struct Item *item) {
    return (abs(pos->x - item->position.x) + abs(pos->y - item->position.y)) <= 1;
}

enum EGameStates getGameStatus(void) {
    return gameStatus;
}

struct ObjectNode *getPlayerItems(void) {
    return collectedObject->next;
}

struct Item *getItemNamed(const char* name) {
    int c = 0;

    for (c = 0; c < itemsCount; ++c ) {
        if (!strcmp(item[c].name, name)) {
            return &item[c];
        }
    }

    return NULL;
}


struct Room *getRoomByName(const char* name) {
    int c = 0;

    for (c = 1; c < roomCount; ++c ) {
        if (!strcmp(rooms[c].name, name)) {
            return &rooms[c];
        }
    }

    return NULL;
}

void addObjectToList(struct Item *itemToAdd, struct ObjectNode *listHead) {
    struct ObjectNode *head = listHead;

    while (head->next != NULL) {
        if (getItem(head->item) == itemToAdd) {
            /* Object already belongs to the list! */
            return;
        }

        head = head->next;
    }

    head->next = &objectNodes[itemToAdd->index];
    memset(head->next, 0, sizeof(struct ObjectNode));
    head->next->item = itemToAdd->index;
}

void removeObjectFromList(struct Item *itemToRemove, struct ObjectNode *listHead) {
    struct ObjectNode *head = listHead->next;
    struct ObjectNode *prev = listHead;

    while (head != NULL) {
        if (head->item == itemToRemove->index) {
            prev->next = head->next;
            return;
        }

        prev = head;
        head = head->next;
    }
    /* Object doesn't belongs to the list! */
}

void removeObjectFromRoom(struct Item *itemToRemove) {
    if (itemToRemove->roomId != 0) {
        removeObjectFromList(itemToRemove, rooms[itemToRemove->roomId].itemsPresent);
        itemToRemove->roomId = 0;
    }
}


void addObjectToRoom(int roomId, struct Item *itemToAdd) {
    struct Room *roomToAddObject = &rooms[roomId];
    removeObjectFromRoom(itemToAdd);
    addObjectToList(itemToAdd, roomToAddObject->itemsPresent);
    itemToAdd->roomId = roomId;
}

void dropObjectToRoom(int roomId, struct Item *itemToDrop) {
#ifdef CLI_BUILD
    if (itemToDrop->roomId != 0) {
        defaultLogger("Object not present to drop");
    }
#endif

    removeObjectFromList(itemToDrop, collectedObject);
    addObjectToRoom(roomId, itemToDrop);

    if (itemToDrop->dropCallback != NULL) {
        itemToDrop->dropCallback(itemToDrop);
    }
}

void pickObject(struct Item *itemToPick) {
#ifndef CAN_PICK_OBJECT_AT_ANY_DISTANCE
    if (!isCloseToObject(getPlayerPosition(), itemToPick)) {
        return;
    }
#endif

    if (!itemToPick->pickable) {
        defaultLogger("Can't pick it up");
        return;
    }

    removeObjectFromRoom(itemToPick);
    addObjectToList(itemToPick, collectedObject);

    if (itemToPick->pickCallback != NULL) {
        itemToPick->pickCallback(itemToPick);
    }
}

int getPlayerRank(void) {
    return playerRank;
}

void setPlayerRank(int newRank) {
    playerRank = newRank;
}

void moveBy(int direction) {
    int c;
    int previousLocation = playerLocation;
    struct Room *room = &rooms[playerLocation];
    if (direction >= 0 && direction <= 5 && room->connections[direction] != 0) {
        struct Item *coupling = getItemNamed("magnetic-coupling");
        room = &rooms[playerLocation];

        if (rooms[room->connections[direction]].rankRequired > playerRank) {
            defaultLogger("Insufficient rank to enter room");
            return;
        }

#ifndef CPC_PLATFORM
        if (!getItemNamed("magnetic-boots")->active || !playerHasObject("magnetic-boots")) {
            defaultLogger("You can't move without your\nmagnetic-boots!");
            return;
        }
#endif

        if (room == getRoomByName("hangar") && coupling->active && direction == 0) {
            defaultLogger("The magnetic coupling is\nengaged. The door won't open.");
            return;
        }


        playerLocation = room->connections[direction];
        room = &rooms[playerLocation];

        for (c = 0; c < 6; ++c ) {
            if (room->connections[c] == previousLocation) {
                direction = c;
            }
        }

        switch (direction) {
            case 2:
                playerPosition.x = rooms[playerLocation].sizeX / 2;
                playerPosition.y = rooms[playerLocation].sizeY - 1;
                break;

            case 3:
                playerPosition.x = 0;
                playerPosition.y = rooms[playerLocation].sizeY / 2;
                break;

            case 0:
                playerPosition.x = rooms[playerLocation].sizeX / 2;
                playerPosition.y = 0;
                break;

            case 1:
                playerPosition.x = rooms[playerLocation].sizeX - 1;
                playerPosition.y = rooms[playerLocation].sizeY / 2;
                break;
        }
#ifdef CLI_BUILD
    } else {
        defaultLogger("Please specify a valid direction");
#endif
    }
}

void pickObjectByName(const char *objName) {
    struct Room *room = &rooms[playerLocation];
    struct ObjectNode *itemToPick = room->itemsPresent->next;

    while (itemToPick != NULL) {
        if (!strcmp(getItem(itemToPick->item)->name, objName)) {
#ifdef MOVE_TO_OBJECT_POSITION_WHEM_PICKING
            playerPosition = getItem(itemToPick->item)->position;
#endif
            pickObject(getItem(itemToPick->item));
            return;
        }
        itemToPick = itemToPick->next;
    }
}

void dropObjectByName(const char *objName) {
    struct ObjectNode *itemToPick = collectedObject->next;



    while (itemToPick != NULL) {
        if (!strcmp(getItem(itemToPick->item)->name, objName)) {
            dropObjectToRoom(playerLocation, getItem(itemToPick->item));
            return;
        }
        itemToPick = itemToPick->next;
    }

#ifdef CLI_BUILD
        errorHandlerCallback("Unable to locate object");
#endif

}

int hasItemInRoom(const char *roomName, const char *itemName) {
    struct ObjectNode *itemToPick;

#ifdef CLI_BUILD
    struct Room* room;

    if (roomName == NULL || itemName == NULL || strlen(roomName) == 0 || strlen(itemName) == 0) {
        defaultLogger("Either the object name or the room name are null. Check your stuff");
        return 0;
    }
#endif

#ifdef CLI_BUILD
    room = getRoomByName(roomName);

    if (room != NULL) {
        itemToPick = room->itemsPresent->next;
    } else {
        errorHandlerCallback("Invalid room name");
        return 0;
    }
#else
    itemToPick = getRoomByName(roomName)->itemsPresent->next;
#endif

    while (itemToPick != NULL) {
        struct Item *pick = getItem(itemToPick->item);
        if (!strcmp(pick->name, itemName)) {
            return 1;
        }
        itemToPick = itemToPick->next;
    }

    return 0;
}

int playerHasObject(const char *itemName) {
    struct ObjectNode *itemToPick = collectedObject->next;

    while (itemToPick != NULL) {
        if (!strcmp(getItem(itemToPick->item)->name, itemName)) {
            return 1;
        }
        itemToPick = itemToPick->next;
    }
    return 0;
}


int isPlayerAtRoom(const char *roomName) {
    struct Room *room = &rooms[playerLocation];
    char *name = room->name;
    int returnValue = !strcmp(name, roomName);
    return returnValue;
}

char *getRoomDescription() {
    struct Room *room = &rooms[playerLocation];
    return room->name;
}

struct Room *getRoom(int index) {
    return &rooms[index];
}

struct Item *getItem(int index) {
    return &item[index];
}

int getPlayerRoom(void) { return playerLocation; }

void useObjectNamed(const char *operand) {
    struct ObjectNode *itemToPick = collectedObject->next;

    while (itemToPick != NULL) {
        struct Item* item = getItem(itemToPick->item);
        if (!strcmp(item->name, operand)) {
            if (item->useCallback != NULL) {
                item->useCallback(item);
            }
            return;
        }
        itemToPick = itemToPick->next;
    }

    itemToPick = getRoom(playerLocation)->itemsPresent->next;

    while (itemToPick != NULL) {
        struct Item* item = getItem(itemToPick->item);
        if (!strcmp(item->name, operand)) {
            if (item->useCallback != NULL) {
                item->useCallback(item);
            }
            return;
        }
        itemToPick = itemToPick->next;
    }
}

#ifdef CLI_BUILD
void walkTo(const char *operands) {
    struct WorldPosition pos;
    char *xStr;
    char *yStr;
    int x;
    int y;

    if (playerLocation != 1 && (!item[5].active || !playerHasObject("magnetic-boots"))) {
        defaultLogger("You can't move without your\nmagnetic-boots!");
    }

    xStr = (char*)operands;
    yStr = strtok(NULL, "\n ");
    x = atoi(xStr);
    y = atoi(yStr);
    pos.x = x;
    pos.y = y;
    setPlayerPosition(&pos);
}

void infoAboutItemNamed(const char *itemName) {

    struct ObjectNode *object1 = collectedObject->next;
    struct Room *room = &rooms[playerLocation];
    struct ObjectNode *object2 = room->itemsPresent->next;

    if (itemName == NULL || strlen(itemName) == 0) {
#ifdef INCLUDE_ROOM_DESCRIPTIONS
        defaultLogger(room->info);
#endif
        return;
    }

    while (object1 != NULL) {
        struct Item* item = getItem(object1->item);
        assert(item->name != NULL);

        if (!strcmp(item->name, itemName)) {
            defaultLogger(item->info);
            return;
        }
        object1 = object1->next;
    }

    while (object2 != NULL) {
        struct Item* item = getItem(object2->item);
        assert(item->name != NULL);

        if (!strcmp(item->name, itemName)) {
            defaultLogger(item->info);
            return;
        }
        object2 = object2->next;
    }

    defaultLogger("No such item could be found");
}

void useObjectsTogether(const char *operands) {

    struct ObjectNode *object1 = collectedObject->next;
    struct Room *room = &rooms[playerLocation];
    struct ObjectNode *object2 = room->itemsPresent->next;

    char *operand1 = (char*)operands;
    char *operand2 = strtok(NULL, "\n ");

    if (!playerHasObject(operand1)) {
        defaultLogger("You do not have this object");
        return;
    }

    if (!hasItemInRoom(getRoom(playerLocation)->name, operand2)) {
        defaultLogger("That object is not present in the room");
        return;
    }

    while (object1 != NULL) {
        struct Item* item = getItem(object1->item);
        assert(item->name != NULL);

        if (!strcmp(item->name, operand1)) {
            goto got_first_object;
        }
        object1 = object1->next;
    }

    got_first_object:
    while (object2 != NULL) {
        struct Item* item = getItem(object2->item);
        assert(item->name != NULL);

        if (!strcmp(item->name, operand2)) {
            goto got_second_object;
        }
        object2 = object2->next;
    }

    got_second_object:
    if (object1 != NULL){
        struct Item* item = getItem(object1->item);

        if (item != NULL && item->useWithCallback != NULL && object2 != NULL) {
            item->useWithCallback(item, getItem(object2->item));
        }
    }
}
#endif

void turnLeft(void) {
    playerDirection--;

    while (playerDirection < 0) {
        playerDirection += 4;
    }
}

void turnRight(void) {
    playerDirection++;

    playerDirection = playerDirection & 3;
}

void setPlayerLocation(int location) {
    playerLocation = location;
}

void walkBy(int direction) {

    if (!getItemNamed("magnetic-boots")->active || !playerHasObject("magnetic-boots")) {
        defaultLogger("You can't move without your\nmagnetic-boots!");
        return;
    }

    switch (direction) {
        case 0:
            switch (playerDirection) {
                case 0:
                    playerPosition.y--;
                    break;
                case 1:
                    playerPosition.x++;
                    break;
                case 2:
                    playerPosition.y++;
                    break;
                case 3:
                    playerPosition.x--;
                    break;
            }
            break;
        case 1:
            switch (playerDirection) {
                case 0:
                    playerPosition.x++;
                    break;
                case 1:
                    playerPosition.y++;
                    break;
                case 2:
                    playerPosition.x--;
                    break;
                case 3:
                    playerPosition.y--;
                    break;
            }
            break;
        case 2:
            switch (playerDirection) {
                case 0:
                    playerPosition.y++;
                    break;
                case 1:
                    playerPosition.x--;
                    break;
                case 2:
                    playerPosition.y--;
                    break;
                case 3:
                    playerPosition.x++;
                    break;
            }
            break;
        case 3:
            switch (playerDirection) {
                case 0:
                    playerPosition.x--;
                    break;
                case 1:
                    playerPosition.y--;
                    break;
                case 2:
                    playerPosition.x++;
                    break;
                case 3:
                    playerPosition.y++;
                    break;
            }
            break;
    }

#ifdef CLI_BUILD
    if (playerPosition.x < 0) {
        if (getRoom(playerLocation)->connections[3]) {
            moveBy(3);
        } else {
            playerPosition.x = 0;
        }
    }

    if (playerPosition.y < 0) {
        if (getRoom(playerLocation)->connections[0]) {
            moveBy(0);
        } else {
            playerPosition.y = 0;
        }
    }


    if (playerPosition.x >= rooms[playerLocation].sizeX) {
        if (getRoom(playerLocation)->connections[1]) {
            moveBy(1);
        } else {
            playerPosition.x = rooms[playerLocation].sizeX - 1;
        }
    }

    if (playerPosition.y >= rooms[playerLocation].sizeY) {
        if (getRoom(playerLocation)->connections[2]) {
            moveBy(2);
        } else {
            playerPosition.y = rooms[playerLocation].sizeY - 1;
        }
    }
#endif
}

int getPlayerDirection(void) {
    return playerDirection;
}

void addToRoom(const char *roomName, struct Item *itemName) {
    int r = 0;

#ifdef CLI_BUILD
    if (roomName == NULL || itemName == NULL || strlen(roomName) == 0) {
        defaultLogger("Either the object name or the room name are null. Check your stuff");
        return;
    }
#endif

    for (r = 1; r < TOTAL_ROOMS; ++r) {
        char *desc = rooms[r].name;

        if (desc != NULL && !strcmp(desc, roomName)) {
            addObjectToRoom(r, itemName);
            return;
        }
    }

#ifdef CLI_BUILD
    defaultLogger("It was not possible to determine the room to add object");
#endif
}

void setLoggerDelegate(LogDelegate newDelegate) {
    defaultLogger = newDelegate;
}


void setPlayerDirection(int direction) {
    playerDirection = direction;
}

void setGameStatus(enum EGameStates newStatus) {
    gameStatus = newStatus;
}

void initCore(void) {

  /* prepare for a single player in the game */
    memset(&playerPosition, 0, sizeof(struct WorldPosition));
    setErrorHandlerCallback(NULL);

    collectedObject = &collectedObjectHead;
    memset(collectedObject, 0, sizeof(struct ObjectNode));
    playerLocation = 1;
    itemsCount = 0;
    roomCount = 1; /* there's an implicit dummy first */
    playerHealth = 100;
    playerRank = 0;
    gameStatus = 0;
    playerDirection = 0;
    playerPosition.x = 15;
    playerPosition.y = 15;

    memset(&rooms, 0, TOTAL_ROOMS * sizeof(struct Room));
    memset(&item, 0, TOTAL_ITEMS * sizeof(struct Item));
}



