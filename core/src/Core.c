/*
Created by Daniel Monteiro on 2019-07-26.
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#include <unistd.h>
#endif

#ifndef DONT_INCLUDE
#include "Core.h"
#endif


int roomCount = 1; /* there's an implicit dummy first */
struct Room rooms[TOTAL_ROOMS];
int itemsCount = 0;
struct Item item[TOTAL_ITEMS];
struct ObjectNode *collectedObject = NULL;
int playerLocation = 1;
int playerDirection;
int playerRank;
int gameStatus;
int playerHealth = 100;
struct WorldPosition *characterPositions;
int charactersCount = 1;
int playerCharacter;
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
//We assume the error handler was already set. If you don't have stdio, you probably know what you're doing
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
    toReturn->description = description;
    toReturn->info = info;
    toReturn->pickable = pickable;
    toReturn->position.x = positionX;
    toReturn->position.y = positionY;
    
    return toReturn;
}

struct Room *addRoom(
        char *description,
#ifdef INCLUDE_ROOM_DESCRIPTIONS
        char *info,
#endif
                int sizeX, int sizeY, int connections[6]) {
    
    struct Room* toReturn = &rooms[roomCount++];
    toReturn->description = description;
#ifdef INCLUDE_ROOM_DESCRIPTIONS
    toReturn->info = info;
#endif
    toReturn->sizeX = sizeX;
    toReturn->sizeY = sizeY;
    
    toReturn->connections[0] = connections[0];
    toReturn->connections[1] = connections[1];
    toReturn->connections[2] = connections[2];
    toReturn->connections[3] = connections[3];
    toReturn->connections[4] = connections[4];
    toReturn->connections[5] = connections[5];
    
    //add list head to make manipulations easier
    toReturn->itemsPresent = (struct ObjectNode *) malloc(sizeof(struct ObjectNode));
    memset(toReturn->itemsPresent, 0, sizeof(struct ObjectNode) );

    return toReturn;
}

LogDelegate defaultLogger = writeToLog;

void setErrorHandlerCallback(ErrorHandlerCallback callback) {
    errorHandlerCallback = callback;
}

struct WorldPosition *getPlayerPosition() {
    return &characterPositions[playerCharacter];
}

int getPlayerHealth() {
    return playerHealth;
}

void setPlayerHealth(int health) {
    playerHealth = health;
}

void setPlayerPosition(struct WorldPosition* pos) {
    characterPositions[playerCharacter].x = pos->x;
    characterPositions[playerCharacter].y = pos->y;
}

int isCloseToObject(struct WorldPosition* pos, struct Item *item) {
    return (abs(pos->x - item->position.x) + abs(pos->y - item->position.y)) <= 1;
}

enum EGameStates getGameStatus() {
    return gameStatus;
}

struct ObjectNode *getPlayerItems() {
    return collectedObject->next;
}

struct Item *getItemNamed(const char* name) {
    int c = 0;
    
    for (c = 0; c < itemsCount; ++c ) {
        if (!strcmp(item[c].description, name)) {
            return &item[c];
        }
    }
    
    return NULL;
}


struct Room *getRoomByName(const char* name) {
    int c = 0;
    
    for (c = 1; c < roomCount; ++c ) {
        if (!strcmp(rooms[c].description, name)) {
            return &rooms[c];
        }
    }
    
    return NULL;
}

void addObjectToList(struct Item *itemToAdd, struct ObjectNode *listHead) {
    struct ObjectNode *head = listHead;

    while (head->next != NULL) {
        if (head->item == itemToAdd) {
            /* Object already belongs to the list! */
            return;
        }

        head = head->next;
    }

    head->next = (struct ObjectNode *) malloc(sizeof(struct ObjectNode));
    memset(head->next, 0, sizeof(struct ObjectNode));
    head->next->item = itemToAdd;
}

void removeObjectFromList(struct Item *itemToRemove, struct ObjectNode *listHead) {
    struct ObjectNode *head = listHead->next;
    struct ObjectNode *prev = listHead;

    while (head != NULL) {
        if (head->item == itemToRemove) {
            prev->next = head->next;
            free(head);
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

    if (itemToDrop->roomId != 0) {
        defaultLogger("Object not present to drop");
    }

    removeObjectFromList(itemToDrop, collectedObject);
    addObjectToRoom(roomId, itemToDrop);

    if (itemToDrop->dropCallback != NULL) {
        itemToDrop->dropCallback(itemToDrop);
    }
}

void pickObject(struct Item *itemToPick) {

    if (!isCloseToObject(getPlayerPosition(), itemToPick)) {
        return;
    }

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

int getPlayerRank() {
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
            defaultLogger("Insuficient rank to enter room");
            return;
        }

        if (!getItemNamed("magnetic-boots")->active || !playerHasObject("magnetic-boots")) {
            defaultLogger("You can't move without your\nmagnetic-boots!");
            return;
        }

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
                characterPositions[playerCharacter].x = rooms[playerLocation].sizeX / 2;
                characterPositions[playerCharacter].y = rooms[playerLocation].sizeY - 1;
                break;

            case 3:
                characterPositions[playerCharacter].x = 0;
                characterPositions[playerCharacter].y = rooms[playerLocation].sizeY / 2;
                break;

            case 0:
                characterPositions[playerCharacter].x = rooms[playerLocation].sizeX / 2;
                characterPositions[playerCharacter].y = 0;
                break;

            case 1:
                characterPositions[playerCharacter].x = rooms[playerLocation].sizeX - 1;
                characterPositions[playerCharacter].y = rooms[playerLocation].sizeY / 2;
                break;
        }

    } else {
        defaultLogger("Please specify a valid direction");
    }
}

void pickObjectByName(const char *objName) {
    struct Room *room = &rooms[playerLocation];
    struct ObjectNode *itemToPick = room->itemsPresent->next;

    while (itemToPick != NULL) {
        if (!strcmp(itemToPick->item->description, objName)) {
#ifdef MOVE_TO_OBJECT_POSITION_WHEM_PICKING
            characterPositions[playerCharacter] = itemToPick->item->position;
#endif
            pickObject(itemToPick->item);
            return;
        }
        itemToPick = itemToPick->next;
    }
}

void dropObjectByName(const char *objName) {
    struct ObjectNode *itemToPick = collectedObject->next;

    while (itemToPick != NULL) {
        if (!strcmp(itemToPick->item->description, objName)) {
            dropObjectToRoom(playerLocation, itemToPick->item);
            return;
        }
        itemToPick = itemToPick->next;
    }
}

int hasItemInRoom(const char *roomName, const char *itemName) {
    int r = 0;

    if (roomName == NULL || itemName == NULL || strlen(roomName) == 0 || strlen(itemName) == 0) {
        defaultLogger("Either the object name or the room name are null. Check your stuff");
        return 0;
    }

    for (r = 1; r < TOTAL_ROOMS; ++r) {
        char *desc = rooms[r].description;

        if (desc != NULL && !strcmp(desc, roomName)) {
            struct ObjectNode *itemToPick = rooms[r].itemsPresent->next;

            while (itemToPick != NULL) {
                if (!strcmp(itemToPick->item->description, itemName)) {
                    return 1;
                }
                itemToPick = itemToPick->next;
            }
            return 0;
        }
    }
    defaultLogger("It was not possible to determine if object is in room");
    return 0;
}

int playerHasObject(const char *itemName) {
    struct ObjectNode *itemToPick = collectedObject->next;

    while (itemToPick != NULL) {
        if (!strcmp(itemToPick->item->description, itemName)) {
            return 1;
        }
        itemToPick = itemToPick->next;
    }
    return 0;
}


int isPlayerAtRoom(const char *roomName) {
    struct Room *room = &rooms[playerLocation];
    char *name = room->description;
    int returnValue = !strcmp(name, roomName);
    return returnValue;
}

char *getRoomDescription() {
    struct Room *room = &rooms[playerLocation];
    return room->description;
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
        if (!strcmp(itemToPick->item->description, operand)) {
            if (itemToPick->item->useCallback != NULL) {
                itemToPick->item->useCallback(itemToPick->item);
            }
            return;
        }
        itemToPick = itemToPick->next;
    }

    itemToPick = getRoom(playerLocation)->itemsPresent->next;

    while (itemToPick != NULL) {
        if (!strcmp(itemToPick->item->description, operand)) {
            if (itemToPick->item->useCallback != NULL) {
                itemToPick->item->useCallback(itemToPick->item);
            }
            return;
        }
        itemToPick = itemToPick->next;
    }
}

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
        assert(object1->item->description != NULL);

        if (!strcmp(object1->item->description, itemName)) {
            defaultLogger(object1->item->info);
            return;
        }
        object1 = object1->next;
    }

    while (object2 != NULL) {
        assert(object2->item->description != NULL);

        if (!strcmp(object2->item->description, itemName)) {
            defaultLogger(object2->item->info);
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

    if (!hasItemInRoom(getRoom(playerLocation)->description, operand2)) {
        defaultLogger("That object is not present in the room");
        return;
    }

    while (object1 != NULL) {
        assert(object1->item->description != NULL);

        if (!strcmp(object1->item->description, operand1)) {
            goto got_first_object;
        }
        object1 = object1->next;
    }

    got_first_object:
    while (object2 != NULL) {
        assert(object2->item->description != NULL);

        if (!strcmp(object2->item->description, operand2)) {
            goto got_second_object;
        }
        object2 = object2->next;
    }

    got_second_object:
    if (object1->item->useWithCallback != NULL) {
        object1->item->useWithCallback(object1->item, object2->item);
    }
}

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
                    characterPositions[playerCharacter].y--;
                    break;
                case 1:
                    characterPositions[playerCharacter].x++;
                    break;
                case 2:
                    characterPositions[playerCharacter].y++;
                    break;
                case 3:
                    characterPositions[playerCharacter].x--;
                    break;
            }
            break;
        case 1:
            switch (playerDirection) {
                case 0:
                    characterPositions[playerCharacter].x++;
                    break;
                case 1:
                    characterPositions[playerCharacter].y++;
                    break;
                case 2:
                    characterPositions[playerCharacter].x--;
                    break;
                case 3:
                    characterPositions[playerCharacter].y--;
                    break;
            }
            break;
        case 2:
            switch (playerDirection) {
                case 0:
                    characterPositions[playerCharacter].y++;
                    break;
                case 1:
                    characterPositions[playerCharacter].x--;
                    break;
                case 2:
                    characterPositions[playerCharacter].y--;
                    break;
                case 3:
                    characterPositions[playerCharacter].x++;
                    break;
            }
            break;
        case 3:
            switch (playerDirection) {
                case 0:
                    characterPositions[playerCharacter].x--;
                    break;
                case 1:
                    characterPositions[playerCharacter].y--;
                    break;
                case 2:
                    characterPositions[playerCharacter].x++;
                    break;
                case 3:
                    characterPositions[playerCharacter].y++;
                    break;
            }
            break;
    }

    if (characterPositions[playerCharacter].x < 0) {
        if (getRoom(playerLocation)->connections[3]) {
            moveBy(3);
        } else {
            characterPositions[playerCharacter].x = 0;
        }
    }

    if (characterPositions[playerCharacter].y < 0) {
        if (getRoom(playerLocation)->connections[0]) {
            moveBy(0);
        } else {
            characterPositions[playerCharacter].y = 0;
        }
    }


    if (characterPositions[playerCharacter].x >= rooms[playerLocation].sizeX) {
        if (getRoom(playerLocation)->connections[1]) {
            moveBy(1);
        } else {
            characterPositions[playerCharacter].x = rooms[playerLocation].sizeX - 1;
        }
    }

    if (characterPositions[playerCharacter].y >= rooms[playerLocation].sizeY) {
        if (getRoom(playerLocation)->connections[2]) {
            moveBy(2);
        } else {
            characterPositions[playerCharacter].y = rooms[playerLocation].sizeY - 1;
        }
    }
}

int getPlayerDirection(void) {
    return playerDirection;
}

void addToRoom(const char *roomName, struct Item *itemName) {
    int r = 0;

    if (roomName == NULL || itemName == NULL || strlen(roomName) == 0) {
        defaultLogger("Either the object name or the room name are null. Check your stuff");
        return;
    }

    for (r = 1; r < TOTAL_ROOMS; ++r) {
        char *desc = rooms[r].description;

        if (desc != NULL && !strcmp(desc, roomName)) {
            addObjectToRoom(r, itemName);
            return;
        }
    }
    defaultLogger("It was not possible to determine the room to add object");
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

void initCore() {

    //prepare for a single player in the game
    playerCharacter = 0;
    charactersCount = 1;
    characterPositions = (struct WorldPosition*)malloc(charactersCount * sizeof(struct WorldPosition));
    memset(characterPositions, 0, charactersCount * sizeof(struct WorldPosition));
    setErrorHandlerCallback(NULL);

    collectedObject = (struct ObjectNode *) malloc(sizeof(struct ObjectNode));
    memset(collectedObject, 0, sizeof(struct ObjectNode));
    playerLocation = 1;
    itemsCount = 0;
    roomCount = 1; /* there's an implicit dummy first */
    playerHealth = 100;
    playerRank = 0;
    gameStatus = 0;
    playerDirection = 0;
    characterPositions[playerCharacter].x = 15;
    characterPositions[playerCharacter].y = 15;

    memset(&rooms, 0, TOTAL_ROOMS * sizeof(struct Room));
    memset(&item, 0, TOTAL_ITEMS * sizeof(struct Item));
}



