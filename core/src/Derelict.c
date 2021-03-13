/*
Created by Daniel Monteiro on 2019-07-26.
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "Derelict.h"

#define TOTAL_ROOMS 24
#define TOTAL_ITEMS 30

int roomCount = 1; /* there's an implicit dummy first */
struct Room station[TOTAL_ROOMS];
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
    
    struct Room* toReturn = &station[roomCount++];
    
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
        removeObjectFromList(itemToRemove, station[itemToRemove->roomId].itemsPresent);
        itemToRemove->roomId = 0;
    }
}


void addObjectToRoom(int roomId, struct Item *itemToAdd) {
    struct Room *roomToAddObject = &station[roomId];
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
    struct Room *room = &station[playerLocation];
    if (direction >= 0 && direction <= 5 && room->connections[direction] != 0) {

        if (station[room->connections[direction]].rankRequired > playerRank) {
            defaultLogger("Insuficient rank to enter room");
            return;
        }

        if (!getItemNamed("magnetic-boots")->active || !playerHasObject("magnetic-boots")) {
            defaultLogger("You can't move without your magnetic-boots!");
            return;
        }

        playerLocation = room->connections[direction];
        room = &station[playerLocation];
        
        for (c = 0; c < 6; ++c ) {
            if (room->connections[c] == previousLocation) {
                direction = c;
            }
        }

        switch (direction) {
            case 2:
                characterPositions[playerCharacter].x = station[playerLocation].sizeX / 2;
                characterPositions[playerCharacter].y = station[playerLocation].sizeY - 1;
                break;

            case 3:
                characterPositions[playerCharacter].x = 0;
                characterPositions[playerCharacter].y = station[playerLocation].sizeY / 2;
                break;

            case 0:
                characterPositions[playerCharacter].x = station[playerLocation].sizeX / 2;
                characterPositions[playerCharacter].y = 0;
                break;

            case 1:
                characterPositions[playerCharacter].x = station[playerLocation].sizeX - 1;
                characterPositions[playerCharacter].y = station[playerLocation].sizeY / 2;
                break;
        }

    } else {
        defaultLogger("Please specify a valid direction");
    }
}

void pickObjectByName(const char *objName) {
    struct Room *room = &station[playerLocation];
    struct ObjectNode *itemToPick = room->itemsPresent->next;

    while (itemToPick != NULL) {
        if (!strcmp(itemToPick->item->description, objName)) {
            characterPositions[playerCharacter] = itemToPick->item->position;
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
        char *desc = station[r].description;

        if (desc != NULL && !strcmp(desc, roomName)) {
            struct ObjectNode *itemToPick = station[r].itemsPresent->next;

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
    struct Room *room = &station[playerLocation];
    char *name = room->description;
    int returnValue = !strcmp(name, roomName);
    return returnValue;
}

char *getRoomDescription() {
    struct Room *room = &station[playerLocation];
    return room->description;
}

struct Room *getRoom(int index) {
    return &station[index];
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

    if (playerLocation != 1 && (!item[5].active || !playerHasObject("magnetic-boots"))) {
        defaultLogger("You can't move without your magnetic-boots!");
    }

    struct WorldPosition pos;
    char *xStr = operands;
    char *yStr = strtok(NULL, "\n ");
    int x = atoi(xStr);
    int y = atoi(yStr);
    pos.x = x;
    pos.y = y;
    setPlayerPosition(pos);
}

void infoAboutItemNamed(const char *itemName) {

    struct ObjectNode *object1 = collectedObject->next;
    struct Room *room = &station[playerLocation];
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
    struct Room *room = &station[playerLocation];
    struct ObjectNode *object2 = room->itemsPresent->next;

    char *operand1 = operands;
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

void useBlowtorchWithCallback(struct Item *item1, struct Item *item2) {
    item2->pickable = TRUE;
}

void useBootsWithCallback(struct Item *item1, struct Item *item2) {
    if (item2 == &item[17]) {
        addToRoom("restroom", &item[25]);
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


    if (characterPositions[playerCharacter].x >= station[playerLocation].sizeX) {
        if (getRoom(playerLocation)->connections[1]) {
            moveBy(1);
        } else {
            characterPositions[playerCharacter].x = station[playerLocation].sizeX - 1;
        }
    }

    if (characterPositions[playerCharacter].y >= station[playerLocation].sizeY) {
        if (getRoom(playerLocation)->connections[2]) {
            moveBy(2);
        } else {
            characterPositions[playerCharacter].y = station[playerLocation].sizeY - 1;
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
        char *desc = station[r].description;

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

    struct ObjectNode *itemToPick = collectedObject->next;
    int newRank = 0;
    while (itemToPick != NULL) {
        int rank = 0;

        if (itemToPick->item == &item[7]) {
            rank = 1;
        }

        if (itemToPick->item == &item[18]) {
            rank = 2;
        }

        if (itemToPick->item == &item[25]) {
            rank = 3;
        }

        if (newRank < rank) {
            newRank = rank;
        }

        itemToPick = itemToPick->next;
    }

    setPlayerRank(newRank);
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

    int blewTheStation = (!hasItemInRoom("lab-1", "metal-mending") && hasItemInRoom("lab-1", "time-bomb"));
    int playerAtShip = (playerLocation == 1);
    int playerAtSameLocationAsBomb = hasItemInRoom(getRoom(playerLocation)->description, "time-bomb");

    if (blewTheStation) {
        if (playerAtShip) {
            setGameStatus(kGoodVictory);
        } else {
            setGameStatus(kBadVictory);
        }
    } else {

        if (playerAtSameLocationAsBomb) {
            setGameStatus(kBadGameOver);
        } else {
            if (playerAtShip) {
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
    moveBy(4);
}

void elevatorGoUpCallback(struct Item *item) {
    moveBy(5);
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
    
    memset(&station, 0, TOTAL_ROOMS * sizeof(struct Room));
    memset(&item, 0, TOTAL_ITEMS * sizeof(struct Item));

    
    /*Rooms*/
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[0] = 2;
    addRoom("lss-daedalus", "My trusty old scrap ship. Built it myself. Still leaks fuel like a drunken horse, but it's mine, damn it! Well, at least until some of fines I have to pay results in repo men knocking my door.", 64, 64, connections);
    
    
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[2] = 1;
    connections[1] = 6;
    connections[0] = 3;
    addRoom("hangar", "The station main hangar is rather unremarkable. The only thing you notice is a slight yellow tint of the air, as if a mist slides next to the floor. It's very faint. Your ship's computer tells you this is harmless (as if those readings were worth the trust). Unfortunately, no useful tools around here. Around the corner, near the escape pod entrance, there is deactivated ship reactor.", 64, 64, connections);

    
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[2] = 2;
    connections[0] = 4;
    connections[1] = 5;
    addRoom("hall-1", "A well lit hall, with doors. It's the main hub of the station. Despite being right next to the hangar and the control room, it's rather quiet.", 64, 64, connections)
    ->rankRequired = 1;

    
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[2] = 3;
    connections[4] = 13;
    addRoom("elevator-level-1", "Going down? The elevator no longer works. It seems to be stuck in level 3. You have to navegate the shaft by yourself.", 64, 64, connections);
    
    
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[3] = 3;
    connections[1] = 9;
    connections[0] = 7;
    connections[2] = 8;
    addRoom("dorms-1", "Part of the dorms hallway. There are some (busted) control panels for ejecting the pods. Some pieces of cloth and broken plastic on the floor, but nothing really useful or valuable.", 64, 64, connections);

    
    
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[3] = 2;
    addRoom("rls-bohr-2", "A rescue lander ship. Only for emergencies. Named after some Niels Bohr scientist guy or whatever. Some drops on the carpet and I don't even want know what it is, but I guess I already know. Ick.", 64, 5, connections);

    
    
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[2] = 5;
    addRoom("pod-1", "A male living pod. Looks like from one of the scientists. It's messy, but as if it's occupant would easily find his belongings in there. There are a few cracks in the glass already.", 64, 64, connections)->rankRequired = 2;
    
    
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[0] = 5;
    addRoom("pod-2", "A empty living pod. Looks as if it was never ever used. If can even see some of the factory stickers in it.", 64, 64, connections)->rankRequired = 2;
    
    
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[1] = 12;
    connections[3] = 5;
    connections[0] = 10;
    connections[2] = 11;
    addRoom("dorms-2", "Anonther part of the dorms hallway. On those, the panels were visibly well. There is a skylight. These parts of the quarters were probably the luxury ones.", 64, 64, connections);
    
    
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[2] = 9;
    addRoom("pod-3", "A young woman's pod. You do recognize a few items, but its badly mixed up. It's hard to make the age of girl, but she was young.",
            64, 64, connections)->rankRequired = 2;
    

    memset(&connections[0], 0, 6 * sizeof(int));
    connections[0] = 9;
    addRoom("pod-4", "A scientists pod, for sure. It's neat, clean and organized. Not much around. He had a strange fixation on redheads.",
            64, 64, connections)->rankRequired = 2;
    
    
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[3] = 9;
    addRoom("lounge", "Here, it seems like a relaxation place. You gaze at the stars and the planet. Very nice.", 64, 5, connections);

    
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[5] = 4;
    connections[4] = 19;
    connections[2] = 14;
    addRoom("elevator-level-2", "Going up or down? Looking down, you can clearly see the elevator cabin in level 3.", 64, 64, connections);
    
    
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[0] = 13;
    connections[1] = 17;
    connections[2] = 16;
    connections[3] = 15;
    addRoom("hall-2", "Not as imponent as the main hall from Level 1, this hall has a busier feel. Here you see objects thrown all over the place, as if someone was in the middle of a day-to-day routine and had to quickly run.", 64, 64, connections);
    
    
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[1] = 14;
    addRoom("dinner-room", "Empty stomach makes no science. Those thinkers were really into fancy stuff. Too bad it all went bad a long time ago.", 64, 64, connections);
    

    memset(&connections[0], 0, 6 * sizeof(int));
    connections[0] = 14;
    addRoom("control-room", "Lots of old equiptment.", 64, 64, connections)->rankRequired = 2;

    
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[3] = 14;
    connections[1] = 18;
    addRoom("gymnasium", "This is where they used to workout to keep their health.", 64, 64, connections);
    

    memset(&connections[0], 0, 6 * sizeof(int));
    connections[3] = 17;
    addRoom("restroom", "...and this is where they would stay clean. Smells like dry sweat. Ick.", 64, 64, connections);

    
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[2] = 20;
    connections[5] = 13;
    addRoom("elevator-level-3", "Going up? Fortunately, the escape hatch is open and this allows for access. The cabin itself is unremarkable.", 64, 64, connections);
    

    memset(&connections[0], 0, 6 * sizeof(int));
    connections[0] = 19;
    connections[1] = 21;
    connections[2] = 22;
    connections[3] = 23;
    addRoom("hall-3", "This was a restricted area, so it's rather sparce. Mostly labs and equipment. A constant hum from the generaters can be heard", 64, 64, connections);

    
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[3] = 20;
    addRoom("lab-1", "A micro-g-hydrostatic lab. Lots of old equipments. There must be something valuable here.", 64, 64, connections)->rankRequired = 3;


    memset(&connections[0], 0, 6 * sizeof(int));
    connections[0] = 20;
    addRoom("lab-2", "A low-atmosphere-electricity lab. Lots of strange equipment. Looks dangerous.", 64, 64, connections)->rankRequired = 3;
    

    memset(&connections[0], 0, 6 * sizeof(int));
    connections[1] = 20;
    addRoom("lab-3", "Looks like this was a chemistry lab. Looks badly destroyed. I was told this was due to space-trash. That's why they got us! On the left wall, there are remnants of a 3D periodic table. If only this was in once piece, it could make some good cash.", 64, 64, connections)->rankRequired = 3;


    /*Items*/

    /* LSS-Daedalus */
    newItem = addItem("time-bomb", "time-programmable Halogen bomb.", 5, TRUE, 9, 14);
    addToRoom("lss-daedalus", newItem);
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    newItem->useCallback = bombActivatedCallback;

    
    newItem = addItem("time-bomb-controller",
                      "The remote controller allows you to instantly detonate the bomb from very far (empirical evidence tells it works from as far as 0.5AU).",
                      0, TRUE, 10, 14);
    addToRoom("lss-daedalus", newItem);
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    newItem->useCallback = bombControllerActivatedCallback;
    
    
    newItem = addItem("blowtorch", "precision vintage-but-rather-well-kept metal cutter", 8, TRUE, 11, 14);
    addToRoom("lss-daedalus", newItem);
    newItem->useWithCallback = useBlowtorchWithCallback;
    newItem->useCallback = cantBeUsedCallback;

    
    newItem = addItem("ship-ignition", "token needed to ignite the ship's computer and thrusters", 0, TRUE, 12, 14);
    addToRoom("lss-daedalus", newItem);
    newItem->useWithCallback = cantBeUsedWithOthersCallback;


    newItem = addItem("plasma-gun", "A mostly harmless gun. Useful for heating surfaces and light defense.", 1, TRUE, 13, 14);
    addToRoom("lss-daedalus", newItem);
    

    newItem = addItem("magnetic-boots",
                      "boots with strong electro-magnets. Ideal for walking on low-gravity situations - as long as the surface in question is metallic (like most of the surfaces here).",
                      2, TRUE, 15, 15);
    
    newItem->active = TRUE;
    
    pickObject(newItem);
    
    newItem->useWithCallback = useBootsWithCallback;
    newItem->useCallback = useObjectToggleCallback;
    

    newItem = addItem("helmet", "Atmosphere-contained helmet for safety.", 2, TRUE, 15, 14);
    addToRoom("lss-daedalus", newItem);
    newItem->useCallback = useObjectToggleCallback;

    
    newItem = addItem("low-rank-keycard", "Clearance for low rank.", 0, TRUE, 16, 14);
    addToRoom("lss-daedalus", newItem);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    newItem->pickCallback = keycardPickCallback;
    newItem->dropCallback = keycardDropCallback;


    /* Hangar */
    
    newItem = addItem("gold-pipe",
                      "This seems valuable. One can wonder why it is used here and what would be the consequences of taking it out? Whaever. It looks like paying for dinner.",
                      17, FALSE, 17, 16);
    addToRoom("hangar", newItem);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    

    /* Comm terminals*/
    newItem = addItem("comm-terminal", "Offline comm terminal for communicating with the other levels.", 200, FALSE, 17, 16);
    addToRoom("lss-daedalus", newItem);
    newItem->useCallback = useObjectToggleCallback;
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("hall-1", newItem);

    
    newItem = addItem("comm-terminal", "Offline comm terminal for communicating with the other levels.", 200, FALSE, 17, 16);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("hall-2", newItem);
    
    
    newItem = addItem("comm-terminal", "Offline comm terminal for communicating with the other levels.", 200, FALSE, 17, 16);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("hall-3", newItem);

    /* Diaries */
    newItem = addItem("white-diary", "...And so, I realized that when you apply a presure in MicroG...", 0, TRUE, 5, 3);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("pod-1", newItem);

    
    newItem = addItem("blue-diary",
                      "...Look in the mirror e try laughing. It's going to be hard first, but once you get used to it, it is easy to leave the room with a smile in your face.",
                      0, TRUE, 8, 4);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("pod-3", newItem);

    
    newItem = addItem("black-diary",
                      "...We meet every night in the empty lab, to make out. I asked her for a access key and hid it in the tubes - probably a good place to hide it. Those are inactive for years! If they catch me, I'm scr...", 0, TRUE, 2, 2);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("pod-4", newItem);

    
    newItem = addItem("yellow-book", "Today, I lost card during dinner. Gonna ask for a new one. Specially disconcerting since it gives access to all bunks. Nobody knows, so nothing to worry, I guess...", 0, TRUE, 4, 3);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("dinner-room", newItem);

    
    newItem = addItem("log-book", "A side note is written: That metal mending we put on Lab 1. last week mending was a real rush job. Any stronger bump and it might...", 1, TRUE, 10, 10);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("lab-2", newItem);

    /* Misc */
    newItem = addItem("plastic-pipe", "Just a regular pipe, taking something somewhere.", 3, FALSE, 20, 40);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("restroom", newItem);

    
    newItem = addItem("high-rank-keycard", "Clearance for high-rank officer.", 0, TRUE, 7, 6);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    newItem->pickCallback = keycardPickCallback;
    newItem->dropCallback = keycardDropCallback;
    addToRoom("dinner-room", newItem);

    
    newItem = addItem("computer-rack", "A very valuable vintage rare-and-in-working-conditions computer rack!", 138, FALSE, 1, 1);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("control-room", newItem);

    
    newItem = addItem("journal",
                      "...and so you guys could just join in and see whats going on. I hope it is not too instrusive of me. To that, she just gave me a cold stare and...",
                      0, TRUE, 17, 6);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("control-room", newItem);
    

    /* Has to be removed for the bomb to be properly planted */
    newItem = addItem("metal-mending", "A piece of metal that might be valuable.", 74, FALSE, 7, 6);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("lab-1", newItem);

    
    newItem = addItem("scientific-treatise", "Voynich Manuscript - Annottated Translation. Classical edition. It's badly burn't. Can't read it.", 1, TRUE, 1, 1);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("lab-2", newItem);
    
    newItem = addItem("electric-experiment", "All these equipment looks the same. Doesn't look valuable for me.", 209, FALSE, 1, 1);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("lab-2", newItem);

    
    newItem = addItem("chemical-experiment", "All these equipment looks the same. Doesn't look valuable for me.", 62, TRUE, 1, 1);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("lab-3", newItem);

    
    /* Not added directly, will be placed on the restroom after you search the pipe */
    newItem = addItem("root-keycard", "Card for root access.", 0, TRUE, 21, 41);
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
