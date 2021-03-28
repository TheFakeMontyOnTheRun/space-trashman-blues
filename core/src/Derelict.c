/*
Created by Daniel Monteiro on 2019-07-26.
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "Derelict.h"



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
int accessGrantedToSafe = FALSE;

void writeToLog(const char *errorMsg) {
    if (errorHandlerCallback == NULL) {
        puts("-------");
        puts(errorMsg);
        puts("--!!---");
    } else {
        errorHandlerCallback(errorMsg);
    }
}

struct Item* addItem(char *description,
                     char *info,
                     int weight,
                     int pickable,
                     int positionX,
                     int positionY) {
    
    struct Item* toReturn = &item[itemsCount];
    
    toReturn->index = itemsCount++;
    toReturn->description = description;
    toReturn->info = info;
    toReturn->weight = weight;
    toReturn->pickable = pickable;
    toReturn->position.x = positionX;
    toReturn->position.y = positionY;
    
    return toReturn;
}

struct Room *addRoom(char *description, char *info, int sizeX, int sizeY, int connections[6]) {
    
    struct Room* toReturn = &rooms[roomCount++];
    
    toReturn->description = description;
    toReturn->info = info;
    toReturn->sizeX = sizeX;
    toReturn->sizeY = sizeY;
    
    toReturn->connections[0] = connections[0];
    toReturn->connections[1] = connections[1];
    toReturn->connections[2] = connections[2];
    toReturn->connections[3] = connections[3];
    toReturn->connections[4] = connections[4];
    toReturn->connections[5] = connections[5];
    
    toReturn->itemsPresent = (struct ObjectNode *) calloc(1, sizeof(struct ObjectNode));
    
    return toReturn;
}

LogDelegate defaultLogger = writeToLog;

void setErrorHandlerCallback(ErrorHandlerCallback callback) {
    errorHandlerCallback = callback;
}

struct WorldPosition getPlayerPosition() {
    return characterPositions[playerCharacter];
}

int getPlayerHealth() {
    return playerHealth;
}

void setPlayerHealth(int health) {
    playerHealth = health;
}

void setPlayerPosition(struct WorldPosition pos) {
    characterPositions[playerCharacter] = pos;
}

int isCloseToObject(struct WorldPosition pos, struct Item *item) {
    return (abs(pos.x - item->position.x) + abs(pos.y - item->position.y)) <= 1;
}

int getGameStatus() {
    return gameStatus;
}

void setGameStatus(int newStatus) {
    gameStatus = newStatus;
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

    head->next = (struct ObjectNode *) calloc(1, sizeof(struct ObjectNode));
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
            defaultLogger("You can't move without your magnetic-boots!");
            return;
        }

        if (room == getRoomByName("hangar") && coupling->active && direction == 0) {
            defaultLogger("The magnetic coupling is engaged. The door will not open");
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
        defaultLogger("You can't move without your magnetic-boots!");
    }

    xStr = (char*)operands;
    yStr = strtok(NULL, "\n ");
    x = atoi(xStr);
    y = atoi(yStr);
    pos.x = x;
    pos.y = y;
    setPlayerPosition(pos);
}

void infoAboutItemNamed(const char *itemName) {

    struct ObjectNode *object1 = collectedObject->next;
    struct Room *room = &rooms[playerLocation];
    struct ObjectNode *object2 = room->itemsPresent->next;

    if (itemName == NULL || strlen(itemName) == 0) {
        defaultLogger(room->info);
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

void useCardWithCardWritter(struct Item *item1, struct Item *item2) {
    if (item2 == getItemNamed("card-writter")) {
        struct Item* card = getItemNamed("hacked-keycard");
        addToRoom("computer-core", card);
        card = getItemNamed("low-rank-keycard");
        removeObjectFromList(card, collectedObject);
    } else {
        defaultLogger("No effect");
    }
}

void useBootsWithMagneticCoupling(struct Item *item1, struct Item *item2) {
    struct Item *coupling = getItemNamed("magnetic-coupling");
    if (item2 == coupling ) {
        coupling->active = FALSE;
	defaultLogger("Magnetic lock disengaged");
    } else {
        defaultLogger("No effect");
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

void walkBy(int direction) {

    if (!getItemNamed("magnetic-boots")->active || !playerHasObject("magnetic-boots")) {
        defaultLogger("You can't move without your magnetic-boots!");
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


void updateRankFromKeycards() {
    int rank = 0;
    
    if (playerHasObject("low-rank-keycard")) {
        rank = 1;
    }

    if (playerHasObject("hacked-keycard")) {
        rank = 2;
    }

    if (playerHasObject("high-rank-keycard")) {
        rank = 3;
    }

    if (playerHasObject("root-keycard")) {
        rank = 4;
    }

    setPlayerRank(rank);
}

void keycardPickCallback(struct Item *item) {
    updateRankFromKeycards();
}


void keycardDropCallback(struct Item *item) {
    updateRankFromKeycards();
}

/*
 Good victory - you blew the station and escaped
 Bad victory - you blew the station, but died
 good game over - you escaped, but failed to blow the station
 bad game over - you failed to blow the station and died.
 */
void bombActivatedCallback(struct Item *item) {

    int empOnReactor = hasItemInRoom( "reactor-core", "emp-bomb");
    int playerAtDaedaus = (playerLocation == 1);
    int playerAtSameLocationAsBomb = hasItemInRoom( getRoom( playerLocation)->description, "emp-bomb");
    
    if (empOnReactor) {
        if (playerAtDaedaus) {
            setGameStatus(kGoodVictory);
        } else {
            setGameStatus(kBadVictory);
        }
    } else {
        if (playerAtSameLocationAsBomb) {
            setGameStatus(kBadGameOver);
        } else {
            if (playerAtDaedaus  ) {
                setGameStatus(kGoodGameOver);
            } else {
                setGameStatus(kBadGameOver);
            }
        }
    }
}

void bombControllerActivatedCallback(struct Item *item) {
    bombActivatedCallback(NULL);
}

void elevatorGoDownCallback(struct Item *item) {
    
    if (!getItemNamed("comm-terminal-2")->active) {
        defaultLogger("Central computer is offline");
        return;
    }
    
    moveBy(4);
}

void elevatorGoUpCallback(struct Item *item) {
    
    if (!getItemNamed("comm-terminal-2")->active) {
        defaultLogger("Central computer is offline");
        return;
    }
    
    moveBy(5);
}

void useCloggedFlush(struct Item *item) {
    struct Item* highRankKeycard = getItemNamed("high-rank-keycard");
    if (highRankKeycard->roomId == 0) {
        defaultLogger("Found something among the....stuff...");
        addToRoom("wc", highRankKeycard);
    }
}

void useRegularFlush(struct Item *item) {
    defaultLogger("*FLUSH*");
}


void cantBeUsedCallback(struct Item *item) {
    defaultLogger("You can't use it like this.");
}

void cantBeUsedWithOthersCallback(struct Item *item1, struct Item *item2) {
    defaultLogger("Nothing happens.");
}

void useObjectToggleCallback(struct Item *item) {
    item->active = !item->active;
}

void useCommWithRank(struct Item *item) {
    
    if (playerRank <= 1) {
        defaultLogger("Insufficient rank to access");
        return;
    }
       defaultLogger("Computer core rebooted");
    item->active = !item->active;
}


void useComputerRack(struct Item *item) {
    
    if (accessGrantedToSafe) {
        defaultLogger("Safe unlocked");
        addToRoom("situation-room", getItemNamed("root-keycard"));
        return;
    }
    
    defaultLogger("Safe secured");
}

void reactorValveCallback(struct Item *item) {
    gameStatus = kBadVictory;
}

void setPlayerDirection(int direction) {
    playerDirection = direction;
}

void initStation(void) {

    struct Item* newItem;
    int connections[6];
    
    //prepare for a single player in the game
    playerCharacter = 0;
    charactersCount = 1;
    characterPositions = (struct WorldPosition*)calloc(charactersCount, sizeof(struct WorldPosition));
    accessGrantedToSafe = FALSE;
    setErrorHandlerCallback(NULL);
    
    collectedObject = (struct ObjectNode *) calloc(1, sizeof(struct ObjectNode));
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

    
    /*Rooms*/
    /* 1 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[0] = 2;
    addRoom("lss-daedalus", "The salvage operations vehicle. It's tracked remotely, to prevent escapes. If I try taking outside the predicted path, I will sink into the abyss myself.", 64, 64, connections);
    
    /* 2 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[2] = 1;
    connections[1] = 6;
    connections[0] = 3;
    addRoom("hangar", "The main hangar is rather unremarkable. The only thing you notice is a slight yellow tint of the air, as if a mist slides next to the floor. It's very faint. Your ship's computer tells you this is harmless (as if those readings were worth the trust). Unfortunately, no useful tools around here. Around the corner, there's a escape pod entrance. Apparently, only one pod was launched.", 64, 64, connections);

    /* 3 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[3] = 2;
    connections[0] = 4;
    connections[1] = 5;
    addRoom("hall-2", "A well lit hall, with doors. It's the main hub of the vehicle. Despite being right next to the hangar and the control room, it's rather quiet.", 64, 64, connections);

    /* 4 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[2] = 3;
    connections[4] = 19;
    connections[5] = 13;
    addRoom("elevator-level-2", "It's rather surprising that this ship has an elevator. This is was typical only of ships with 5 levels or more.", 64, 64, connections)->rankRequired = 1;
    
    /* 5 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[3] = 3;
    connections[1] = 9;
    connections[0] = 7;
    connections[2] = 8;
    addRoom("dorms-1", "Part of the dorms hallway. There are some (busted) control panels for ejecting the pods. Some pieces of cloth and broken plastic on the floor, but nothing really useful.", 64, 64, connections)->rankRequired = 1;

    
    /* 6 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[3] = 2;
    addRoom("rls-bohr-2", "A rescue ship. Only for emergencies. Named after some Niels Bohr scientist guy or whatever. Some drops on the carpet and I don't even want know what it is, but I guess I already know. Ick.", 64, 5, connections);

    
    /* 7 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[2] = 5;
    addRoom("pod-1", "A living pod. Looks like from one of the oficcers. It's messy, but as if it's occupant would easily find his belongings in there. There are some burn marks on the walls.", 64, 64, connections)->rankRequired = 1;
    
    /* 8 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[0] = 5;
    addRoom("pod-2", "A empty living pod. Looks as if it was never ever used. If can even see some of the factory stickers in it.", 64, 64, connections)->rankRequired = 3;
    
    /* 9 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[1] = 12;
    connections[3] = 5;
    connections[0] = 10;
    connections[2] = 11;
    addRoom("dorms-2", "Anonther part of the dorms hallway. On those, the panels were visibly well. These parts of the quarters were probably the more prestigious ones.", 64, 64, connections);
    
    /* 10 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[2] = 9;
    addRoom("pod-3", "A young woman's pod. You do recognize a few items, but its badly mixed up. It's hard to make the age of girl, but she was young.",
            64, 64, connections)->rankRequired = 3;
    
    /* 11 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[0] = 9;
    addRoom("pod-4", "A the first officer's pod, for sure. It's neat, clean and organized. Not much around. He had a strange fixation on redheads.",
            64, 64, connections)->rankRequired = 4;
    
    /* 12 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[3] = 9;
    addRoom("computer-core", "While it is not the main control.", 64, 64, connections);

    /* 13 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[4] = 4;
    connections[2] = 14;
    addRoom("elevator-level-1", "It's rather surprising that this ship has an elevator. This is was typical only of ships with 5 levels or more.", 64, 64, connections)->rankRequired = 1;
    
    /* 14 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[0] = 13;
    connections[1] = 17;
    connections[2] = 16;
    connections[3] = 15;
    addRoom("hall-1", "Not as imponent as the main hall from Level 1, this hall has a busier feel. Here you see objects thrown all over the place, as if someone was in the middle of a day-to-day routine and had to quickly run.", 64, 64, connections)->rankRequired = 1;
    
    /* 15 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[1] = 14;
    addRoom("bridge", "Empty stomach makes no science. Those thinkers were really into fancy stuff. Too bad it all went bad a long time ago.", 64, 64, connections)->rankRequired = 4;
    
    /* 16 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[0] = 14;
    addRoom("situation-room", "Lots of old equiptment.", 64, 64, connections)->rankRequired = 3;

    /* 17 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[3] = 14;
    connections[1] = 18;
    addRoom("crew-bunks", "This is where they used to workout to keep their health.", 64, 64, connections)->rankRequired = 1;
    
    /* 18 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[3] = 17;
    addRoom("armory", "...and this is where they would stay clean. Smells like dry sweat. Ick.", 64, 64, connections)->rankRequired = 3;

    /* 19 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[2] = 20;
    connections[5] = 4;
    addRoom("elevator-level-3", "It's rather surprising that this ship has an elevator. This is was typical only of ships with 5 levels or more.", 64, 64, connections)->rankRequired = 1;
    
    /* 20 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[0] = 19;
    connections[1] = 21;
    connections[3] = 23;
    addRoom("hall-3", "This was a restricted area, so it's rather sparce. Mostly labs and equipment. A constant hum from the generaters can be heard", 64, 64, connections);

    /* 21 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[3] = 20;
    connections[1] = 22;
    addRoom("wc", "0A low-atmosphere-electricity lab. Lots of strange equipment. Looks dangerous.", 64, 64, connections);
    
    /* 22 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[3] = 21;
    addRoom("reactor-core", "A micro-g-hydrostatic lab. Lots of old equipments. There must be something valuable here.", 64, 64, connections)->rankRequired = 4;
    
    /* 23 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[1] = 20;
    addRoom("radar-array", "Looks like this was a chemistry lab. Looks badly destroyed. I was told this was due to space-trash. That's why they got us! On the left wall, there are remnants of a 3D periodic table. If only this was in once piece, it could make some good cash.", 64, 64, connections)->rankRequired = 2;


    /*Items*/

    /* LSS-Daedalus */
    newItem = addItem("emp-bomb", "time-programmable Halogen bomb.", 5, TRUE, 9, 14);
    addToRoom("lss-daedalus", newItem);
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    newItem->useCallback = bombActivatedCallback;

    
    newItem = addItem("emp-controller",
                      "The remote controller allows you to instantly detonate the bomb from very far (empirical evidence tells it works from as far as 200 nautical miles).",
                      0, TRUE, 10, 14);
    addToRoom("lss-daedalus", newItem);
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    newItem->useCallback = bombControllerActivatedCallback;
    
    newItem = addItem("ship-ignition", "token needed to ignite the ship's computer and thrusters", 0, TRUE, 11, 14);
    addToRoom("lss-daedalus", newItem);
    newItem->useCallback = bombActivatedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;

    newItem = addItem("magnetic-boots",
                      "boots with strong electro-magnets. Ideal for walking underwater - as long as the surface in question is metallic (like most of the surfaces here).",
                      2, TRUE, 15, 15);
    
    newItem->active = TRUE;
    pickObject(newItem);
    newItem->useWithCallback = useBootsWithMagneticCoupling;
    newItem->useCallback = useObjectToggleCallback;
    

    newItem = addItem("helmet", "Atmosphere-contained helmet for safety.", 2, TRUE, 15, 15);
    newItem->active = TRUE;
    pickObject(newItem);
    newItem->useCallback = useObjectToggleCallback;

    
    newItem = addItem("low-rank-keycard", "Clearance for low rank. Oddly, this one is of the rewrittable kind; probably due to a field promotion.", 0, TRUE, 16, 14);
    addToRoom("hall-2", newItem);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = useCardWithCardWritter;
    newItem->pickCallback = keycardPickCallback;
    newItem->dropCallback = keycardDropCallback;
    
    
    newItem = addItem("hacked-keycard", "Hacked keycard for mid clearance rank.", 0, TRUE, 35, 19);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    newItem->pickCallback = keycardPickCallback;
    newItem->dropCallback = keycardDropCallback;


    /* Hangar */
    
    newItem = addItem("magnetic-coupling",
                      "Automatic seal activated by special safety protocols",
                      17, FALSE, 2, 7);
    addToRoom("hangar", newItem);
    newItem->active = TRUE;
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    
    /* Comm terminals*/
    
    newItem = addItem("door-panel", "Special control for ", 200, FALSE, 12, 8);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("hangar", newItem);
    

    /* Comm terminals*/
    newItem = addItem("comm-terminal-1", "Tterminal for communicating with the central computer.", 200, FALSE, 19, 10);
    newItem->useCallback = useObjectToggleCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("hall-1", newItem);

    
    newItem = addItem("comm-terminal-2", "Terminal for communicating with the central computer.", 200, FALSE, 19, 10);
    newItem->useCallback = useCommWithRank;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("hall-2", newItem);
    
    
    newItem = addItem("comm-terminal-3", "Terminal for communicating with the central computer.", 200, FALSE, 19, 10);
    newItem->useCallback = useObjectToggleCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("hall-3", newItem);

    /* Diaries */
    newItem = addItem("white-diary", "Crew is growing demotivated with all the combat exercises and no downtime. Don't know long can I keep the fact that we already lost the war. If anything goes wrong, the situation room will be our last stand.", 0, TRUE, 5, 3);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("pod-2", newItem);

    
    newItem = addItem("blue-diary",
                      "The growing discontent is very noticeable. I don't know flor how long can we keep the situation stable. For safety, I gave the root keycard to first officer Costa.",
                      0, TRUE, 8, 4);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("pod-1", newItem);

    
    newItem = addItem("black-diary",
                      "...We meet every night in the rest room, to make out. I asked her for an access key for the armory - let's see if she keeps her promisse. If they catch me, I'm scr...", 0, TRUE, 2, 2);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("pod-4", newItem);

    
    newItem = addItem("yellow-book", "Situation is hopeless. I must enact the security lockdown protocol and set the reactor to a low activity state. With luck, they will rescue us in the next weeks or so.", 0, TRUE, 4, 3);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("bridge", newItem);

    
    newItem = addItem("log-book", "Power conduit on level 3 was restored without incidents.", 1, TRUE, 10, 10);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("wc", newItem);

    /* Misc */
    newItem = addItem("card-writter", "Terminal with card writter, connected to the main computer", 3, FALSE, 36, 20);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("computer-core", newItem);

    
    newItem = addItem("high-rank-keycard", "Clearance for high-rank officer.", 0, TRUE, 32, 17);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    newItem->pickCallback = keycardPickCallback;
    newItem->dropCallback = keycardDropCallback;
    

    newItem = addItem("computer-node", "A very valuable vintage rare-and-in-working-conditions computer rack!", 138, FALSE, 35, 15);
    newItem->useCallback = useComputerRack;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("situation-room", newItem);
    
    newItem = addItem("computer-terminal", "An offline terminal connected to the computer node", 138, FALSE, 35, 16);
    newItem->useCallback = useComputerRack;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("situation-room", newItem);

    
    newItem = addItem("journal",
                      "...and so you guys could just join in and see whats going on. I hope it is not too instrusive of me. To that, she just gave me a cold stare and...",
                      0, TRUE, 17, 6);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("situation-room", newItem);
    

    /* Has to be removed for the bomb to be properly planted */
    newItem = addItem("metal-mending", "A piece of metal that might be valuable.", 74, FALSE, 7, 6);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("radar-array", newItem);

    
    newItem = addItem("scientific-treatise", "Voynich Manuscript - Annottated Translation. Classical edition. It's badly burn't. Can't read it.", 1, TRUE, 1, 1);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("wc", newItem);
    
    
    newItem = addItem("clogged-flush", "Voynich Manuscript - Annottated Translation. Classical edition. It's badly burn't. Can't read it.", 1, FALSE, 33, 17);
    newItem->useCallback = useCloggedFlush;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("wc", newItem);
    
    newItem = addItem("flush", "Voynich Manuscript - Annottated Translation. Classical edition. It's badly burn't. Can't read it.", 1, FALSE, 29, 17);
    newItem->useCallback = useRegularFlush;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("wc", newItem);

    
    newItem = addItem("fuel-rods", "This is the source of all the trouble. Both now and then. Gotta find a way to eject those into the abyss.", 209, FALSE, 1, 1);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("reactor-core", newItem);

    
    newItem = addItem("reactor-valve-control", "This is the computer node that could be used to eject the rods into the abyss.", 62, FALSE, 35, 15);
    newItem->useCallback = reactorValveCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("reactor-core", newItem);

    
    /* Not added directly, will be placed on the restroom after you search the pipe */
    newItem = addItem("root-keycard", "Card for root access.", 0, TRUE, 34, 14);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    newItem->pickCallback = keycardPickCallback;
    newItem->dropCallback = keycardDropCallback;
    
    /* Elevator controls */
    newItem = addItem("elevator-level1-go-down", "Elevator controls - Go down.", 0, FALSE, 27, 0);
    newItem->useCallback = elevatorGoDownCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("elevator-level-1", newItem);

    newItem = addItem("elevator-level2-go-down", "Elevator controls - Go down.", 0, FALSE, 27, 0);
    newItem->useCallback = elevatorGoDownCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("elevator-level-2", newItem);
    
    newItem = addItem("elevator-level2-go-up", "Elevator controls - Go Up.", 0, FALSE, 28, 0);
    newItem->useCallback = elevatorGoUpCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("elevator-level-2", newItem);
    
    newItem = addItem("elevator-level3-go-up", "Elevator controls - Go Up.", 0, FALSE, 28, 0);
    newItem->useCallback = elevatorGoUpCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("elevator-level-3", newItem);
}
