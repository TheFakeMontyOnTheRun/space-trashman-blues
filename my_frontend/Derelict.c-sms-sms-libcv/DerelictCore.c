#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <cv.h>
#include <cvu.h>
#include <assert.h>


#ifndef _CV_COMMON_H
#define _CV_COMMON_H

/* VRAM map
 0x0000 - 0x17ff character pattern table
 0x1800 - 0x1aff image table
 0x2000 - 0x37ff color table
 0x3800 - 0x3bff sprite pattern table
 0x3c00 - 0x3fff sprite attribute table
 */

#define PATTERN        ((const cv_vmemp)0x0000)
#define IMAGE        ((const cv_vmemp)0x1800)
#define COLOR        ((const cv_vmemp)0x2000)
#define SPRITE_PATTERNS ((const cv_vmemp)0x3800)
#define SPRITES        ((const cv_vmemp)0x3c00)

#ifndef COLS
#define COLS 32
#endif

#ifndef ROWS
#define ROWS 24
#endif

typedef unsigned char byte;
typedef signed char sbyte;
typedef unsigned short word;

#ifdef CV_CV
uintptr_t __at(0x6a) font_bitmap_a;
uintptr_t __at(0x6c) font_bitmap_0;
#endif

#ifdef CV_SMS
extern char font_bitmap_a[];
extern char font_bitmap_0[];
#endif

#define COLOR_FGBG(fg,bg) (((fg)<<4)|(bg))
#define COLOR_FG(fg) (((fg)<<4))

#ifndef LOCHAR
#define LOCHAR 0x0
#endif

#define CHAR(ch) (ch-LOCHAR)

#define wait_vsync() __asm__("halt")

extern volatile uint_fast8_t vint_counter;

extern void vint_handler(void);
extern byte reverse_bits(byte n);
extern void flip_sprite_patterns(word dest, const byte* patterns, word len);

extern char cursor_x;
extern char cursor_y;

extern void clrscr();

extern word getimageaddr(byte x, byte y);
extern byte getcharxy(byte x, byte y);
extern void putcharxy(byte x, byte y, byte attr);
extern void putstringxy(byte x, byte y, const char* string);
extern void delay(byte i);
extern byte rndint(byte a, byte b);

extern void memset_safe(void* _dest, char ch, word size);
extern char in_rect(byte x, byte y, byte x0, byte y0, byte w, byte h);
// print 4-digit BCD value
extern void draw_bcd_word(byte x, byte y, word bcd);
// add two 16-bit BCD values
extern word bcd_add(word a, word b);

extern void vdp_setup();
extern void set_shifted_pattern(const byte* src, word dest, byte shift);

extern void copy_default_character_set();

#endif




#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

enum GameStates {
    kNormalGameplay,
    kGoodVictory,
    kBadVictory,
    kGoodGameOver,
    kBadGameOver
};

struct Item;

typedef void ( *ErrorHandlerCallback )(const char* error);

typedef void ( *PickObjectCallback )(struct Item* item);

typedef void ( *DropObjectCallback )(struct Item* item);

typedef void ( *UseObjectCallback )(struct Item* item);

typedef void ( *UseWithObjectCallback )(struct Item* item, struct Item* otherItem);

typedef void ( *LogDelegate )(const char* item);

struct WorldPosition {
    int16_t x;
    int16_t y;
};

struct Item {
    char *description;
    int weight;
    int roomId;
    int active;
    int capacity;
    int pickable;
    uint8_t index;
    char *info;
    struct WorldPosition position;
    UseWithObjectCallback useWithCallback;
    UseObjectCallback useCallback;
    DropObjectCallback dropCallback;
    PickObjectCallback pickCallback;
};

struct ObjectNode {
    struct Item *item;
    struct ObjectNode *next;
};

struct Room {
    char *description;
    char *info;
    int connections[6];
    struct ObjectNode *itemsPresent;
    int sizeX;
    int sizeY;
    int rankRequired;
};

struct WorldPosition* getPlayerPosition();

struct Item* getItem(int index);

struct ObjectNode* getPlayerItems();

void setPlayerPosition(struct WorldPosition* pos);

void addToRoom( const char* roomName, struct Item *itemName );

int isCloseToObject( struct WorldPosition* pos, struct Item* item );

void addObjectToRoom(int roomId, struct Item *itemToAdd);

void dropObjectToRoom(int roomId, struct Item *itemToDrop);

void pickObject(struct Item *itemToPick);

void pickObjectByName(const char *objName);

void dropObjectByName(const char *objName);

void initStation(void);

int getGameStatus();

void setGameStatus(int newStatus);

struct Item *getItemNamed(const char* name);

int getPlayerDirection(void);

void setPlayerDirection(int direction);

void infoAboutItemNamed(const char* itemName);

void moveBy(int direction);

int getPlayerRoom(void);

int hasItemInRoom(const char *roomName, const char *itemName);

int playerHasObject( const char* itemName);

int getPlayerHealth();

void setPlayerHealth(int health);

int isPlayerAtRoom(const char *roomName);

char *getRoomDescription();

struct Room *getRoom(int index);

void useObjectNamed(const char* operand);

void useObjectsTogether(const char* operands);

void showInventory();

extern LogDelegate defaultLogger;

void setLoggerDelegate(LogDelegate newDelegate);

void setErrorHandlerCallback(ErrorHandlerCallback callback);

void walkTo(const char* operands);

void turnLeft(void);

void turnRight(void);

void walkBy(int direction);

int getPlayerRank();

void setPlayerRank(int newRank);

int isPositionAllowed(int x, int y);

#define TOTAL_ROOMS 24
#define TOTAL_ITEMS 64


/* TODO: make accessory method for this */
extern int itemsCount;

///


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
    if (errorHandlerCallback != NULL) {
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
    setPlayerPosition(&pos);
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
        item1 = NULL;
    }
}

void useBootsWithMagneticCoupling(struct Item *item1, struct Item *item2) {
    struct Item *coupling = getItemNamed("magnetic-coupling");
    if (item2 == coupling ) {
        coupling->active = FALSE;
        defaultLogger("Magnetic lock disengaged");
    } else {
        defaultLogger("No effect");
        item1 = NULL;
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
    item = NULL;
}


void keycardDropCallback(struct Item *item) {
    updateRankFromKeycards();
    item = NULL;
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
    item = NULL;
}

void bombControllerActivatedCallback(struct Item *item) {
    bombActivatedCallback(NULL);
    item = NULL;
}

void elevatorGoDownCallback(struct Item *item) {
    
    if (!getItemNamed("comm-terminal-2")->active) {
        defaultLogger("Central computer is offline");
        return;
    }
    
    moveBy(4);
    item = NULL;
}

void elevatorGoUpCallback(struct Item *item) {
    
    if (!getItemNamed("comm-terminal-2")->active) {
        defaultLogger("Central computer is offline");
        return;
    }
    
    moveBy(5);
    item = NULL;
}

void useCloggedFlush(struct Item *item) {
    struct Item* highRankKeycard = getItemNamed("high-rank-keycard");
    if (highRankKeycard->roomId == 0) {
        defaultLogger("Found something among the....stuff...");
        addToRoom("wc", highRankKeycard);
    }
    item = NULL;
}

void useRegularFlush(struct Item *item) {
    defaultLogger("*FLUSH*");
    item = NULL;
}


void cantBeUsedCallback(struct Item *item) {
    defaultLogger("You can't use it like this.");
    item = NULL;
}

void cantBeUsedWithOthersCallback(struct Item *item1, struct Item *item2) {
    defaultLogger("Nothing happens.");
    item1 = item2 = NULL;
}

void useObjectToggleCallback(struct Item *item) {
    item->active = !item->active;
    item = NULL;
}

void useCommWithRank(struct Item *item) {
    
    if (playerRank <= 1) {
        defaultLogger("Insufficient rank to access");
        return;
    }
    defaultLogger("Computer core rebooted");
    item->active = !item->active;
    item = NULL;
}


void useComputerRack(struct Item *item) {
    
    if (accessGrantedToSafe) {
        defaultLogger("Safe unlocked");
        addToRoom("situation-room", getItemNamed("root-keycard"));
        return;
    }
    
    defaultLogger("Safe secured");
    item = NULL;
}

void reactorValveCallback(struct Item *item) {
    gameStatus = kBadVictory;
    item = NULL;
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
    addRoom("lss-daedalus", "The salvage operations vehicle. It's tracked remotely, to prevent escapes. If I try taking outside the predicted path, I will sink into the abyss myself.", 32, 32, connections);
    
    /* 2 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[2] = 1;
    connections[1] = 6;
    connections[0] = 3;
    addRoom("hangar", "The main hangar is rather unremarkable. The only thing you notice is a slight yellow tint of the air, as if a mist slides next to the floor. It's very faint. Your ship's computer tells you this is harmless (as if those readings were worth the trust). Unfortunately, no useful tools around here. Around the corner, there's a escape pod entrance. Apparently, only one pod was launched.", 32, 32, connections);
    
    /* 3 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[3] = 2;
    connections[0] = 4;
    connections[1] = 5;
    addRoom("hall-2", "A well lit hall, with doors. It's the main hub of the vehicle. Despite being right next to the hangar and the control room, it's rather quiet.", 32, 32, connections);
    
    /* 4 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[2] = 3;
    connections[4] = 19;
    connections[5] = 13;
    addRoom("elevator-level-2", "It's rather surprising that this ship has an elevator. This is was typical only of ships with 5 levels or more.", 32, 32, connections)->rankRequired = 1;
    
    /* 5 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[3] = 3;
    connections[1] = 9;
    connections[0] = 7;
    connections[2] = 8;
    addRoom("dorms-1", "Part of the dorms hallway. There are some (busted) control panels for ejecting the pods. Some pieces of cloth and broken plastic on the floor, but nothing really useful.", 32, 32, connections)->rankRequired = 1;
    
    
    /* 6 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[3] = 2;
    addRoom("rls-bohr-2", "A rescue ship. Only for emergencies. Named after some Niels Bohr scientist guy or whatever. Some drops on the carpet and I don't even want know what it is, but I guess I already know. Ick.", 32, 32, connections);
    
    
    /* 7 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[2] = 5;
    addRoom("pod-1", "A living pod. Looks like from one of the oficcers. It's messy, but as if it's occupant would easily find his belongings in there. There are some burn marks on the walls.", 32, 32, connections)->rankRequired = 1;
    
    /* 8 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[0] = 5;
    addRoom("pod-2", "A empty living pod. Looks as if it was never ever used. If can even see some of the factory stickers in it.", 32, 32, connections)->rankRequired = 3;
    
    /* 9 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[1] = 12;
    connections[3] = 5;
    connections[0] = 10;
    connections[2] = 11;
    addRoom("dorms-2", "Anonther part of the dorms hallway. On those, the panels were visibly well. These parts of the quarters were probably the more prestigious ones.", 32, 32, connections);
    
    /* 10 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[2] = 9;
    addRoom("pod-3", "A young woman's pod. You do recognize a few items, but its badly mixed up. It's hard to make the age of girl, but she was young.",
            32, 32, connections)->rankRequired = 3;
    
    /* 11 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[0] = 9;
    addRoom("pod-4", "A the first officer's pod, for sure. It's neat, clean and organized. Not much around. He had a strange fixation on redheads.",
            32, 32, connections)->rankRequired = 4;
    
    /* 12 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[3] = 9;
    addRoom("computer-core", "While it is not the main control.", 32, 32, connections);
    
    /* 13 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[4] = 4;
    connections[2] = 14;
    addRoom("elevator-level-1", "It's rather surprising that this ship has an elevator. This is was typical only of ships with 5 levels or more.", 32, 32, connections)->rankRequired = 1;
    
    /* 14 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[0] = 13;
    connections[1] = 17;
    connections[2] = 16;
    connections[3] = 15;
    addRoom("hall-1", "Not as imponent as the main hall from Level 1, this hall has a busier feel. Here you see objects thrown all over the place, as if someone was in the middle of a day-to-day routine and had to quickly run.", 32, 32, connections)->rankRequired = 1;
    
    /* 15 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[1] = 14;
    addRoom("bridge", "Empty stomach makes no science. Those thinkers were really into fancy stuff. Too bad it all went bad a long time ago.", 32, 32, connections)->rankRequired = 4;
    
    /* 16 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[0] = 14;
    addRoom("situation-room", "Lots of old equiptment.", 32, 32, connections)->rankRequired = 3;
    
    /* 17 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[3] = 14;
    connections[1] = 18;
    addRoom("crew-bunks", "This is where they used to workout to keep their health.", 32, 32, connections)->rankRequired = 1;
    
    /* 18 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[3] = 17;
    addRoom("armory", "...and this is where they would stay clean. Smells like dry sweat. Ick.", 32, 32, connections)->rankRequired = 3;
    
    /* 19 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[2] = 20;
    connections[5] = 4;
    addRoom("elevator-level-3", "It's rather surprising that this ship has an elevator. This is was typical only of ships with 5 levels or more.", 32, 32, connections)->rankRequired = 1;
    
    /* 20 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[0] = 19;
    connections[1] = 21;
    connections[3] = 23;
    addRoom("hall-3", "This was a restricted area, so it's rather sparce. Mostly labs and equipment. A constant hum from the generaters can be heard", 32, 32, connections);
    
    /* 21 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[3] = 20;
    connections[1] = 22;
    addRoom("wc", "0A low-atmosphere-electricity lab. Lots of strange equipment. Looks dangerous.", 32, 32, connections);
    
    /* 22 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[3] = 21;
    addRoom("reactor-core", "A micro-g-hydrostatic lab. Lots of old equipments. There must be something valuable here.", 32, 32, connections)->rankRequired = 4;
    
    /* 23 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[1] = 20;
    addRoom("radar-array", "Looks like this was a chemistry lab. Looks badly destroyed. I was told this was due to space-trash. That's why they got us! On the left wall, there are remnants of a 3D periodic table. If only this was in once piece, it could make some good cash.", 32, 32, connections)->rankRequired = 2;
    
    
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
    
    
    newItem = addItem("hacked-keycard", "Hacked keycard for mid clearance rank.", 0, TRUE, 25, 19);
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
    newItem = addItem("card-writter", "Terminal with card writter, connected to the main computer", 3, FALSE, 26, 20);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("computer-core", newItem);
    
    
    newItem = addItem("high-rank-keycard", "Clearance for high-rank officer.", 0, TRUE, 22, 17);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    newItem->pickCallback = keycardPickCallback;
    newItem->dropCallback = keycardDropCallback;
    
    
    newItem = addItem("computer-node", "A very valuable vintage rare-and-in-working-conditions computer rack!", 138, FALSE, 25, 15);
    newItem->useCallback = useComputerRack;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("situation-room", newItem);
    
    newItem = addItem("computer-terminal", "An offline terminal connected to the computer node", 138, FALSE, 25, 16);
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
    
    
    newItem = addItem("clogged-flush", "Voynich Manuscript - Annottated Translation. Classical edition. It's badly burn't. Can't read it.", 1, FALSE, 23, 17);
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
    
    
    newItem = addItem("reactor-valve-control", "This is the computer node that could be used to eject the rods into the abyss.", 62, FALSE, 25, 15);
    newItem->useCallback = reactorValveCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("reactor-core", newItem);
    
    
    /* Not added directly, will be placed on the restroom after you search the pipe */
    newItem = addItem("root-keycard", "Card for root access.", 0, TRUE, 24, 14);
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




int parseCommand(char *cmd, char *operand) {
    if (!strcmp(cmd, "pick")) {
        pickObjectByName(operand);
    } else if (!strcmp(cmd, "drop")) {
        dropObjectByName(operand);
    } else if (!strcmp(cmd, "move")) {
        
        if (operand != NULL) {
            moveBy(operand[0] - '0');
        } else {
            defaultLogger("Please specify a valid direction");
        }
    } else if (!strcmp(cmd, "use")) {
        useObjectNamed(operand);
        
    } else if (!strcmp(cmd, "w")) {
        walkBy(0);
    } else if (!strcmp(cmd, "s")) {
        walkBy(2);
    } else if (!strcmp(cmd, "a")) {
        walkBy(3);
    } else if (!strcmp(cmd, "d")) {
        walkBy(1);
    } else if (!strcmp(cmd, "q")) {
        turnLeft();
    } else if (!strcmp(cmd, "e")) {
        turnRight();
    } else if (!strcmp(cmd, "walkTo")) {
        walkTo(operand);
    } else if (!strcmp(cmd, "info")) {
        infoAboutItemNamed(operand);
    } else if (!strcmp(cmd, "use-with")) {
        useObjectsTogether(operand);
    } else {
        defaultLogger("Unrecognized command");
        return FALSE;
    }
    
    return TRUE;
}



void setup_mode2() {
    cvu_vmemset(0, 0, 0x4000);
    cv_set_screen_mode(CV_SCREENMODE_BITMAP); // mode 2
    cv_set_image_table(IMAGE);
    cv_set_character_pattern_t(PATTERN|0x1fff); // AND mask
    cv_set_color_table(COLOR|0x1fff); // AND mask
    cv_set_sprite_attribute_table(0x2800);
    {
        byte i=0;
        do {
            cvu_voutb(i, IMAGE+i);
            cvu_voutb(i, IMAGE+0x100+i);
            cvu_voutb(i, IMAGE+0x200+i);
        } while (++i);
    }
}

uint8_t buffer[64 * 64];

void init() {
    setup_mode2();
    cv_set_screen_active(true);
}

uint8_t getKey () {
    struct cv_controller_state state;
    cv_get_controller_state(&state, 0);
    
    if (state.joystick & CV_UP) {
        return 'w';
    }
    
    if (state.joystick & CV_LEFT) {
        return 'a';
    }
    
    
    if (state.joystick & CV_RIGHT) {
        return 'd';
    }
    
    
    if (state.joystick & CV_DOWN) {
        return 's';
    }
    
    return '.';
}

void shutdownGraphics() {
}

void clearGraphics() {
    /*
     cvu_vmemset(0, 0, 0x4000 / 3 );
     
     
     {
     byte i=0;
     do {
     cvu_voutb(i, IMAGE+i);
     } while (++i);
     }*/
}

void graphicsFlush() {
    uint8_t x, y, _x, _y;
    byte b;
    word ofs, _yoff;
    uint8_t *ptr = &buffer[0];
    uint8_t pixel;
    _y = 0;
    for (y = 64; y; --y ) {
        _y++;
        _yoff = ( ((_y >> 3 ) << 8)) + (_y & 7);
        
        _x = 0;
        
        for (x = 0; x < 64;) {
            
            // refactor: _x & 248 -> (_x / 8 ) * 8
            ofs = ( _x & 248 ) + _yoff;
            b = cvu_vinb(PATTERN + ofs);
            
            //1
            pixel = *ptr;
            if ( pixel & 1 ) {
                
                if ( ~(pixel & 2) ) {
                    b |= 128 >> (_x & 7);
                    
                }
                
            } else {
                if ( (pixel & 2) ) {
                    b &= ~(128 >> ( _x     &7));
                    
                }
            }
            *ptr = pixel << 1;
            ptr++;
            _x += 1;
            
            //2
            pixel = *ptr;
            if ( pixel & 1 ) {
                if ( ~(pixel & 2) ) {
                    b |= 128 >> (_x & 7);
                    
                }
            } else {
                if ( (pixel & 2) ) {
                    b &= ~(128 >> ( _x     &7));
                    
                }
            }
            *ptr = pixel << 1;
            ptr++;
            _x += 1;
            
            //3
            pixel = *ptr;
            if ( pixel & 1 ) {
                if ( ~(pixel & 2) ) {
                    b |= 128 >> (_x & 7);
                    
                }
                
            } else {
                if ( (pixel & 2) ) {
                    b &= ~(128 >> ( _x     &7));
                    
                }
            }
            *ptr = pixel << 1;
            ptr++;
            _x += 1;
            
            //4
            pixel = *ptr;
            if ( pixel & 1 ) {
                if ( ~(pixel & 2) ) {
                    b |= 128 >> (_x & 7);
                    
                }
            } else {
                if ( (pixel & 2) ) {
                    b &= ~(128 >> ( _x     &7));
                    
                }
            }
            *ptr = pixel << 1;
            ptr++;
            _x += 1;
            
            x += 4;
            cvu_voutb(b, PATTERN + ofs);
            cvu_voutb(32, COLOR + ofs);
            
            
        }
    }
}

void hLine(uint8_t x0, uint8_t x1, uint8_t y0) {
    uint8_t *ptr;
    uint8_t _x0 = x0;
    uint8_t _x1 = x1;
    
    if (x0 > x1) {
        _x0 = x1;
        _x1 = x0;
    }
    
    ptr = &buffer[(y0 * 64) + _x0];
    for (uint8_t x = _x0; x <= _x1; ++x) {
        {
            ptr++;
            *ptr |= 1;
            
        }
    }
}

void vLine(uint8_t x0, uint8_t y0, uint8_t y1) {
    
    uint8_t *ptr;
    uint8_t _y0 = y0;
    uint8_t _y1 = y1;
    
    if (y0 > y1) {
        _y0 = y1;
        _y1 = y0;
    }
    
    ptr = &buffer[(_y0 * 64) + x0];
    
    for (uint8_t y = _y0; y <= _y1; ++y) {
        
        
        *ptr |= 1;
        ptr += 64;
        
        
        
    }
}

void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    word offset;
    uint8_t *ptr;
    if (x0 == x1) {
        
        uint8_t _y0 = y0;
        uint8_t _y1 = y1;
        
        if (y0 > y1) {
            _y0 = y1;
            _y1 = y0;
        }
        
        
        offset = (_y0 * 64) + x0;
        ptr = &buffer[offset];
        
        for (uint8_t y = _y0; y <= _y1; ++y) {
            
            
            {
                ptr += 64;
                *ptr |= 1;
                
            }
        }
        return;
    }
    
    if (y0 == y1) {
        uint8_t _x0 = x0;
        uint8_t _x1 = x1;
        
        if (x0 > x1) {
            _x0 = x1;
            _x1 = x0;
        }
        
        
        offset = (y0 * 64) + _x0;
        ptr = &buffer[offset];
        for (uint8_t x = _x0; x <= _x1; ++x) {
            {
                ptr++;
                *ptr |= 1;
                
            }
        }
        return;
    }
    
    
    {
        //https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
        
        uint8_t dx = abs(x1 - x0);
        int8_t sx = x0 < x1 ? 1 : -1;
        int16_t dy = -abs(y1 - y0);
        int8_t sy = y0 < y1 ? 1 : -1;
        int16_t err = dx + dy;  /* error value e_xy */
        int16_t e2;
        word offset = (y0 * 64) + x0;
        
        if (sy > 0 ) {
            while (1) {
                
                buffer[offset] |= 1;
                
                /* loop */
                if (x0 == x1 && y0 == y1) return;
                e2 = err << 2;
                
                if (e2 >= dy) {
                    err += dy; /* e_xy+e_x > 0 */
                    x0 += sx;
                    offset += sx;
                }
                
                if (e2 <= dx) {
                    /* e_xy+e_y < 0 */
                    err += dx;
                    y0++;
                    offset += 64;
                }
            }
        } else {
            while (1) {
                
                buffer[offset] |= 1;
                
                /* loop */
                if (x0 == x1 && y0 == y1) return;
                e2 = err << 2;
                
                if (e2 >= dy) {
                    err += dy; /* e_xy+e_x > 0 */
                    x0 += sx;
                    offset += sx;
                }
                
                if (e2 <= dx) {
                    /* e_xy+e_y < 0 */
                    err += dx;
                    y0--;
                    offset -= 64;
                }
            }
        }
        
    }
}


#define WEDGE_TYPE_NEAR_LEFT 4
#define WEDGE_TYPE_NEAR_RIGHT 8

enum DIRECTION {
    DIRECTION_N,
    DIRECTION_E,
    DIRECTION_S,
    DIRECTION_W
};

#define IN_RANGE(V0, V1, V)  ((V0) <= (V) && (V) <= (V1))


void shutdownGraphics();

void clearGraphics();

void writeStr(uint8_t nColumn, uint8_t nLine, char *str, uint8_t fg, uint8_t bg);

uint8_t getKey();

void init();

void graphicsFlush();

void fix_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);

void hLine(uint8_t x0, uint8_t x1, uint8_t y);

void vLine(uint8_t x0, uint8_t y0, uint8_t y1);

int8_t stencilHigh[64];

int8_t cameraX = 33;
int8_t cameraZ = 22;
int8_t cameraRotation = 0;
uint8_t running = 1;

struct Projection {
    uint8_t px;
    uint8_t py;
    int16_t dx;
};


struct Pattern {
    uint8_t ceiling: 4;
    uint8_t elementsMask: 4;
    uint8_t geometryType;
    uint8_t block;
};

const struct Projection projections[36] =
{
    //                                   Z
    {0,  96, -64}, // 0
    {0,  64, -32}, // 1
    {11, 53, -21}, // 2
    {16, 48,  -16}, // 3
    {19, 45,  -13}, // 4
    {21, 43,  -11}, // 5
    {23, 41,  -9},  // 6
    {24, 40,  -8},  // 7
    {25, 39,  -7},  // 8
    {26, 38,  -6},  // 9
    {26, 38,  -6},  // 10
    {27, 37,  -5},  // 11
    {27, 37,  -5}, // 12
    {27, 37,  -5}, // 13
    {28, 36,  -4}, // 14
    {28, 36,  -4}, // 15
    {28, 36,  -4}, // 16
    {28, 36,  -4}, // 17
    {29, 35,  -3}, // 18
    {29, 35,  -3}, // 19
    {29, 35,  -3}, // 20
    {29, 35,  -3}, // 21
    {29, 35,  -3},  // 22
    {29, 35,  -3},  // 23
    {29, 35,  -3},  // 24
    {30, 34,  -2},  // 25
    {30, 34,  -2},  // 26
    {30, 34,  -2},  // 27
    {30, 34,  -2}, // 28
    {30, 34,  -2}, // 29
    {30, 34,  -2}, // 30
    {30, 34,  -2}, // 31
    {30, 34,  -2}, // 32
    {30, 34,  -2}, // 33
    {30, 34,  -2}, // 34
    {30, 34,  -2}, // 35
};

const struct Pattern patterns[16] = {
    {7, 3, 0, 0}, //0
    {7, 3, 0, 1}, // 1
    {0, 0, 0, 0}, // 2
    {0, 3, 0, 0}, //3
    {0, 3, 4, 0}, //4
    {0, 3, 8, 0}, //5
    {3, 3, 0, 0}, //6
    {7, 3, 0, 0}, //7
    {0, 1, 0, 0}, // 8
    {0, 3, 0, 0}, // 9
};

int8_t map[32][32] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 7, 7, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 2, 7, 7, 5, 2, 4, 7, 7, 7, 7, 7, 5, 3, 6, 6, 3, 4, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 5, 2, 1},
    {1, 2, 7, 7, 7, 7, 7, 7, 0, 0, 0, 7, 7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 2, 1},
    {1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1},
    {1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1},
    {1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1},
    {1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 3, 7, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1},
    {1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1},
    {1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1},
    {1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 0, 0, 0, 7, 2, 1},
    {1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 3, 7, 0, 0, 0, 7, 2, 1},
    {1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 0, 0, 7, 2, 1, 1},
    {1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1, 1},
    {1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 2, 1},
    {1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 4, 2, 1},
    {1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 3, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 3, 2, 1},
    {1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 7, 1},
    {1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 7, 1},
    {1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 3, 2, 1},
    {1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 5, 2, 1},
    {1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 2, 1},
    {1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1},
    {1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1},
    {1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1},
    {1, 2, 7, 0, 0, 0, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1},
    {1, 2, 7, 0, 0, 0, 7, 3, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 3, 7, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1},
    {1, 2, 7, 0, 0, 0, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1},
    {1, 2, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 2, 1},
    {1, 2, 5, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 4, 2, 1},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

int8_t max(int8_t x1, int8_t x2) {
    return x1 > x2 ? x1 : x2;
}

int8_t min(int8_t x1, int8_t x2) {
    return x1 < x2 ? x1 : x2;
}


uint8_t drawWedge(int8_t x0, int8_t y0, int8_t z0, int8_t dX, int8_t dY, int8_t dZ, uint8_t elementMask, uint8_t type) {
    
    int16_t z1;
    int16_t z0px;
    int16_t z1px;
    int16_t z0py;
    
    int16_t z1py;
    int16_t z0dx;
    int16_t z1dx;
    
    int16_t px0z0;
    int16_t py0z0;
    int16_t py0z1;
    
    int16_t py1z0;
    int16_t py1z1;
    
    int16_t px1z1;
    
    if (z0 >= 32) {
        return 0;
    }
    
    z1 = z0 + dZ;
    
    if (z0 <= 2) {
        return 0;
    }
    
    if (z1 <= 2) {
        return 0;
    }
    
    if (z1 >= 32) {
        return 0;
    }
    
    
    if (type == WEDGE_TYPE_NEAR_LEFT) {
        z0px = (projections[z0].px);
        z1px = (projections[z1].px);
        z0dx = ((projections[z0].dx));
        z1dx = ((projections[z1].dx));
        
        px0z0 = z0px - (x0 * z0dx);
        px1z1 = z1px - ((dX + x0) * z1dx);
        
        z1py = (projections[z1].py);
        z0py = (projections[z0].py);
        
        py0z0 = z0py + ((y0) * z0dx);
        py0z1 = z1py + ((y0) * z1dx);
        
        py1z0 = z0py + ((y0 + dY) * z0dx);
        py1z1 = z1py + ((y0 + dY) * z1dx);
        
    } else {
        z0px = (projections[z1].px);
        z1px = (projections[z0].px);
        z0dx = ((projections[z1].dx));
        z1dx = ((projections[z0].dx));
        
        px0z0 = z0px - ((x0) * z0dx);
        px1z1 = z1px - ((x0 + dX) * z1dx); //extra operations to avoid overflow
        
        z1py = (projections[z0].py);
        z0py = (projections[z1].py);
        
        py0z0 = z0py + ((y0) * z0dx);
        py0z1 = z1py + ((y0) * z1dx);
        
        py1z0 = z0py + ((y0 + dY) * z0dx);
        py1z1 = z1py + ((y0 + dY) * z1dx);
    }
    
    if (px1z1 < 0 || px0z0 > 63) {
        return 0;
    }
    
#ifdef DEBUG_WIREFRAME
    fix_line( px0z0, py0z0, px1z0, py0z0, 4);
    fix_line( px0z0, py0z0, px0z0, py1z0, 4);
    fix_line( px1z0, py0z0, px1z0, py1z0, 4);
    fix_line( px0z0, py1z0, px1z0, py1z0, 4);
    
    fix_line( px0z1, py0z1, px1z1, py0z1, 4);
    
    fix_line( px0z0, py0z0, px0z1, py0z1, 4);
    fix_line( px1z0, py0z0, px1z1, py0z1, 4);
    return;
#endif
    
    {
        int16_t x0, x1;
        
        
        if (elementMask & 2) {
            if (IN_RANGE(0, 63, px0z0) && stencilHigh[px0z0] < py0z0) {
                vLine(px0z0, py0z0, max(py1z0, stencilHigh[px0z0]));
            }
        }
        
        if (elementMask & 1) {
            if (IN_RANGE(0, 63, px1z1) && py0z1 > stencilHigh[px1z1]) {
                vLine(px1z1, py0z1, max(py1z1, stencilHigh[px1z1]));
            }
        }
        
        
        /* The upper segment */
        x0 = px0z0;
        x1 = px1z1;
        
        if (x0 != x1) {
            int16_t upperY0 = py1z0;
            int16_t upperY1 = py1z1;
            int16_t upperDx = abs(x1 - x0);
            int16_t upperSx = x0 < x1 ? 1 : -1;
            int16_t upperDy = -abs(upperY1 - upperY0);
            int16_t upperSy = upperY0 < upperY1 ? 1 : -1;
            int16_t upperErr = upperDx + upperDy;  /* error value e_xy */
            int16_t upperErr2;
            int16_t lowerY0 = py0z0;
            int16_t lowerY1 = py0z1;
            int16_t lowerDx = abs(x1 - x0);
            int16_t lowerSx = x0 < x1 ? 1 : -1;
            int16_t lowerDy = -abs(lowerY1 - lowerY0);
            int16_t lowerSy = lowerY0 < lowerY1 ? 1 : -1;
            int16_t lowerErr = lowerDx + lowerDy;  /* error value e_xy */
            int16_t lowerErr2 = 0;
            
            while ((x0 != x1 && (upperY0 != upperY1 || lowerY0 != lowerY1))) {
                
                if (IN_RANGE(0, 63, x0)) {
                    if (stencilHigh[x0] <= upperY0) {
                        buffer[(upperY0 * 64) + x0] |= 1;
                    }
                    
                    if (stencilHigh[x0] < lowerY0) {
                        stencilHigh[x0] = lowerY0;
                    }
                }
                
                /* loop */
                upperErr2 = upperErr * 2;
                
                if (upperErr2 >= upperDy || lowerErr2 >= lowerDy) {
                    upperErr += upperDy; /* e_xy+e_x > 0 */
                    lowerErr += lowerDy; /* e_xy+e_x > 0 */
                    x0 += lowerSx;
                }
                
                if (x0 >= 64) {
                    return 0;
                }
                
                if (upperErr2 <= upperDx) {
                    /* e_xy+e_y < 0 */
                    upperErr += upperDx;
                    upperY0 += upperSy;
                }
                
                /* loop */
                lowerErr2 = lowerErr * 2;
                
                if (lowerErr2 <= lowerDx) {
                    /* e_xy+e_y < 0 */
                    lowerErr += lowerDx;
                    lowerY0 += lowerSy;
                }
            }
        }
    }
    
    return 1;
}

uint8_t drawObjectCube(int8_t x0, int8_t y0, int8_t z0, int8_t dX, int8_t dY, int8_t dZ) {
    
    int8_t z1;
    uint8_t z0px;
    uint8_t z0py;
    uint8_t z1px;
    uint8_t z1py;
    int8_t z0dx;
    int8_t z1dx;
    
    int16_t px0z0;
    int8_t py0z0;
    int16_t px1z0;
    int8_t py1z0;
    int16_t px0z1;
    int8_t py0z1;
    int16_t px1z1;
    
    
    if (z0 >= 32) {
        return 0;
    }
    
    z1 = z0 + dZ;
    
    if (z1 >= 32) {
        return 0;
    }
    
    
    z0px = (projections[z0].px);
    z1px = (projections[z1].px);
    z0dx = ((projections[z0].dx));
    z1dx = ((projections[z1].dx));
    
    px0z0 = z0px - ((x0) * z0dx);
    px0z1 = z1px - ((x0) * z1dx);
    
    px1z0 = px0z0 - (dX * z0dx);
    px1z1 = px0z1 - (dX * z1dx);
    
    z1py = (projections[z1].py);
    z0py = (projections[z0].py);
    
    py0z0 = z0py + ((y0) * z0dx);
    py1z0 = py0z0 + (dY * z0dx);
    py0z1 = z1py + ((y0) * z1dx);
    
    if (px1z0 < 0 || px0z0 > 63) {
        return 0;
    }
    
    
    drawLine( px0z0, py0z0, px1z0, py0z0);
    drawLine( px0z0, py0z0, px0z0, py1z0);
    drawLine( px1z0, py0z0, px1z0, py1z0);
    drawLine( px0z0, py1z0, px1z0, py1z0);
    drawLine( px0z1, py0z1, px1z1, py0z1);
    drawLine( px0z0, py0z0, px0z1, py0z1);
    drawLine( px1z0, py0z0, px1z1, py0z1);
}

uint8_t drawCubeAt(int8_t x0, int8_t y0, int8_t z0, int8_t dX, int8_t dY, int8_t dZ, uint8_t elementMask) {
    
    int8_t z1;
    uint8_t z0px;
    uint8_t z0py;
    uint8_t z1px;
    uint8_t z1py;
    int8_t z0dx;
    int8_t z1dx;
    
    int16_t px0z0;
    int8_t py0z0;
    int16_t px1z0;
    int8_t py1z0;
    int16_t px0z1;
    int8_t py0z1;
    int16_t px1z1;
    
    uint8_t drawContour;
    
    if (z0 >= 32) {
        return 0;
    }
    
    z1 = z0 + dZ;
    
    if (z1 >= 32) {
        return 0;
    }
    
    
    z0px = (projections[z0].px);
    z1px = (projections[z1].px);
    z0dx = ((projections[z0].dx));
    z1dx = ((projections[z1].dx));
    
    px0z0 = z0px - ((x0) * z0dx);
    px0z1 = z1px - ((x0) * z1dx);
    
    px1z0 = px0z0 - (dX * z0dx);
    px1z1 = px0z1 - (dX * z1dx);
    
    z1py = (projections[z1].py);
    z0py = (projections[z0].py);
    
    py0z0 = z0py + ((y0) * z0dx);
    py1z0 = py0z0 + (dY * z0dx);
    py0z1 = z1py + ((y0) * z1dx);
    
    if (px1z0 < 0 || px0z0 > 63) {
        return 0;
    }
    
    drawContour = (dY);
    
#ifdef DEBUG_WIREFRAME
    fix_line( px0z0, py0z0, px1z0, py0z0, 4);
    fix_line( px0z0, py0z0, px0z0, py1z0, 4);
    fix_line( px1z0, py0z0, px1z0, py1z0, 4);
    fix_line( px0z0, py1z0, px1z0, py1z0, 4);
    
    fix_line( px0z1, py0z1, px1z1, py0z1, 4);
    
    fix_line( px0z0, py0z0, px0z1, py0z1, 4);
    fix_line( px1z0, py0z0, px1z1, py0z1, 4);
    return;
#endif
    
    {
        int16_t x, x0, x1;
        
        if (drawContour) {
            if (elementMask & 2) {
                if (IN_RANGE(0, 63, px0z0) && stencilHigh[px0z0] < py0z0) {
                    vLine(px0z0, py0z0, stencilHigh[px0z0]);
                }
                
                if (IN_RANGE(0, 63, px1z0) && stencilHigh[px1z0] < py0z0) {
                    vLine(px1z0, py0z0, stencilHigh[px1z0]);
                }
            }
            
            if (elementMask & 1) {
                if (IN_RANGE(0, 63, px0z1) && px0z1 < px0z0 && py0z1 > stencilHigh[px0z1]) {
                    vLine(px0z1, py0z1, stencilHigh[px0z1]);
                }
                
                if (IN_RANGE(0, 63, px1z1) && px1z1 > px1z0 && py0z1 > stencilHigh[px1z1]) {
                    vLine(px1z1, py0z1, stencilHigh[px1z1]);
                }
            }
        }
        
        /* Draw the horizontal outlines of z0 and z1 */
        
        if (py0z0 > py0z1) {
            /* Ceiling is lower than camera */
            for (x = px0z0; x <= px1z0; ++x) {
                if (IN_RANGE(0, 63, x) && stencilHigh[x] < py0z0) {
                    if (drawContour) {
                        buffer[(stencilHigh[x] * 64) + x] |= 1;
                    }
                    stencilHigh[x] = py0z0;
                }
            }
        } else if (drawContour) {
            /* Ceiling is higher than the camera*/
            /* Let's just draw the nearer segment */
            for (x = px0z0; x <= px1z0; ++x) {
                if (IN_RANGE(0, 63, x) && stencilHigh[x] < py0z0) {
                    buffer[(stencilHigh[x] * 64) + x] |= 1;
                }
            }
        }
        
        
        /* The left segment */
        x0 = px0z0;
        x1 = px0z1;
        
        if (x0 != x1) {
            int16_t y0 = py0z0;
            int16_t y1 = py0z1;
            int16_t dx = abs(x1 - x0);
            int16_t sx = x0 < x1 ? 1 : -1;
            int16_t dy = -abs(y1 - y0);
            int16_t sy = y0 < y1 ? 1 : -1;
            int16_t err = dx + dy;  /* error value e_xy */
            int16_t e2;
            
            while ((x0 != x1 || y0 != y1)) {
                
                if (IN_RANGE(0, 63, x0)) {
                    if (stencilHigh[x0] < y0) {
                        if (drawContour) {
                            buffer[(stencilHigh[x0] * 64) + x0] |= 1;
                        }
                        stencilHigh[x0] = y0;
                    }
                }
                
                /* loop */
                e2 = err << 2;
                
                if (e2 >= dy) {
                    err += dy; /* e_xy+e_x > 0 */
                    x0 += sx;
                }
                
                if (x0 >= 64) {
                    goto right_stroke;
                }
                
                if (e2 <= dx) {
                    /* e_xy+e_y < 0 */
                    err += dx;
                    y0 += sy;
                }
            }
        }
        
    right_stroke:
        
        /* The right segment */
        x0 = px1z0;
        x1 = px1z1;
        
        if (x0 != x1) {
            int16_t y0 = py0z0;
            int16_t y1 = py0z1;
            int16_t dx = abs(x1 - x0);
            int16_t sx = x0 < x1 ? 1 : -1;
            int16_t dy = -abs(y1 - y0);
            int16_t sy = y0 < y1 ? 1 : -1;
            int16_t err = dx + dy;  /* error value e_xy */
            int16_t e2;
            
            while ((x0 != x1 || y0 != y1)) {
                
                if (IN_RANGE(0, 63, x0) && stencilHigh[x0] < y0) {
                    if (drawContour) {
                        buffer[(stencilHigh[x0] * 64) + x0] |= 1;
                    }
                    stencilHigh[x0] = y0;
                }
                
                /* loop */
                e2 = err << 2;
                
                if (e2 >= dy) {
                    err += dy; /* e_xy+e_x > 0 */
                    x0 += sx;
                }
                
                if (x0 >= 64) {
                    goto final_stroke;
                }
                
                if (e2 <= dx) {
                    /* e_xy+e_y < 0 */
                    err += dx;
                    y0 += sy;
                }
            }
        }
        
    final_stroke:
        if (py0z0 <= py0z1) {
            /* Ceiling is higher than the camera*/
            /* Draw the last segment */
            
            if (drawContour) {
                for (x = px0z1; x <= px1z1; ++x) {
                    if (IN_RANGE(0, 63, x) && stencilHigh[x] < py0z1) {
                        stencilHigh[x] = py0z1;
                    }
                }
            } else {
                for (x = px0z1; x <= px1z1; ++x) {
                    if (IN_RANGE(0, 63, x) && stencilHigh[x] < py0z1) {
                        stencilHigh[x] = py0z1;
                    }
                }
            }
        }
    }
    
    return 1;
}


uint8_t drawPattern(uint8_t _pattern, uint8_t x0, uint8_t x1, uint8_t y) {
    uint8_t pattern = _pattern & 0x7F;
    int8_t diff = patterns[0].ceiling - patterns[pattern].ceiling;
    uint8_t type = patterns[pattern].geometryType;
    
    if (patterns[pattern].block) {
        return 0;
    }
    
    if (_pattern & 128 ) {
        drawObjectCube( x0, 0, y, 1, 1, 1);
    }
    
    
    if (type == 0) {
        return drawCubeAt(x0, patterns[pattern].ceiling - 1, y, x1 - x0,
                          diff, 1, patterns[pattern].elementsMask);
        
    } else {
        switch (cameraRotation) {
            case DIRECTION_W:
            case DIRECTION_E:
                if (type == WEDGE_TYPE_NEAR_LEFT) {
                    type = WEDGE_TYPE_NEAR_RIGHT;
                } else {
                    type = WEDGE_TYPE_NEAR_LEFT;
                }
                break;
                
        }
        
        return drawWedge(x0, patterns[pattern].ceiling - 1, y, x1 - x0,
                         diff, 1, patterns[pattern].elementsMask, type);
    }
}

void renderScene() {
    uint8_t lastPattern, lastIndex;
    
    switch (cameraRotation) {
        case DIRECTION_N: {
            int8_t y;
            int8_t limit = max(cameraZ - 19, 0);
            for (y = min(cameraZ - 3, 31); y >= limit; --y) {
                int8_t x;
                int8_t const *mapY = &map[y][0];
                int8_t const *mapXY;
                int8_t minX = cameraX + 5 + ((cameraZ - 3) - y);
                int8_t maxX = 0;
                
                if (minX > (cameraX + 16)) {
                    minX = cameraX + 16;
                }
                
                lastIndex = cameraX;
                lastPattern = *(mapY + lastIndex);
                
                mapXY = &map[y][lastIndex];
                for (x = lastIndex; x < minX - 1; ++x) {
                    uint8_t pattern;
                    
                    pattern = *mapXY;
                    
                    if (pattern != lastPattern) {
                        if (lastPattern != 0) {
                            if (!drawPattern(lastPattern, lastIndex - cameraX, x - cameraX, cameraZ - y)) {
                                x = minX - 1;
                            }
                            lastIndex = x;
                        }
                        lastPattern = pattern;
                    }
                    
                    ++mapXY;
                }
                if (lastPattern != 0) {
                    drawPattern(lastPattern, lastIndex - cameraX, x - cameraX, cameraZ - y);
                }
                
                lastIndex = cameraX - 1;
                
                if (!cameraX) {
                    lastIndex = 0;
                }
                
                lastPattern = *(mapY + lastIndex);
                
                mapXY = &map[y][lastIndex];
                
                maxX = max(cameraX - 3 - ((cameraZ - 3) - y), 0);
                
                for (x = lastIndex; x >= maxX + 1; --x) {
                    uint8_t pattern;
                    pattern = *mapXY;
                    
                    if (pattern != lastPattern) {
                        if (lastPattern != 0) {
                            
                            if (!drawPattern(lastPattern, x + 1 - cameraX, lastIndex + 1 - cameraX, cameraZ - y)) {
                                x = maxX + 1;
                            }
                            
                            lastIndex = x;
                        }
                        lastPattern = pattern;
                    }
                    
                    --mapXY;
                }
                if (lastPattern != 0) {
                    drawPattern(lastPattern, x + 1 - cameraX, lastIndex + 1 - cameraX, cameraZ - y);
                }
            }
        }
            break;
            
        case DIRECTION_E: {
            int8_t x;
            for (x = min(cameraX - 3, 31); x <= min(cameraX + 13, 31); ++x) {
                int8_t y;
                
                for (y = cameraZ; y <= min(cameraZ + (x - cameraX), 31); ++y) {
                    drawPattern(map[y][x], y - cameraZ + 3, y + 1 - cameraZ + 3, x - cameraX + 3);
                }
                
                for (y = max(cameraZ - 1, 0); y >= max(cameraZ - (x - cameraX), 0); --y) {
                    drawPattern(map[y][x], y - cameraZ + 3, y + 1 - cameraZ + 3, x - cameraX + 3);
                }
                
            }
        }
            break;
            
        case DIRECTION_S: {
            int8_t y;
            for (y = min(cameraZ + 3, 31); y <= min(cameraZ + 19, 31); ++y) {
                int8_t x;
                for (x = cameraX; x <= min(cameraX + (y - (cameraZ + 3)), 31); ++x) {
                    drawPattern(map[y][x], cameraX - x, cameraX - x + 1, y - cameraZ);
                }
                
                for (x = max(cameraX - 1, 0); x >= max(cameraX - (y - (cameraZ + 3)), 0); --x) {
                    drawPattern(map[y][x], cameraX - x, cameraX - x + 1, y - cameraZ);
                }
            }
        }
            break;
            
        case DIRECTION_W: {
            int8_t x;
            for (x = max(cameraX, 0); x >= max(cameraX - 16, 0); --x) {
                int8_t y;
                for (y = cameraZ; y <= min(cameraZ - (x - (cameraX)), 31); ++y) {
                    drawPattern(map[y][x], y - cameraZ + 3, y + 1 - cameraZ + 3, cameraX - x + 1);
                }
                
                for (y = max(cameraZ - 1, 0); y >= max(cameraZ + (x - (cameraX)), 0); --y) {
                    drawPattern(map[y][x], y - cameraZ + 3, y + 1 - cameraZ + 3, cameraX - x + 1);
                }
            }
        }
            break;
    }
    
    for (uint8_t x = 0; x < 64; ++x) {
        buffer[(stencilHigh[x] * 64) + x] |= 1;
    }
}

void tickRenderer() {
    uint8_t prevX;
    uint8_t prevZ;
    
    
    renderScene();
    
    
    
    graphicsFlush();
    memset(stencilHigh, 0, 64);
    
    prevX = cameraX;
    prevZ = cameraZ;
    
waitkey:
    switch (getKey()) {
        case 'q':
            cameraRotation--;
            if (cameraRotation < 0) {
                cameraRotation = 3;
            }
            break;
        case 'e':
            cameraRotation = (cameraRotation + 1) & 3;
            break;
        case 'l':
            running = 0;
            break;
            
        case 'a':
            cameraX -= 2;
            break;
        case 'd':
            cameraX += 2;
            break;
            
            
        case 's':
            switch (cameraRotation) {
                case 0:
                    cameraZ += 2;
                    break;
                case 1:
                    cameraX -= 2;
                    break;
                case 2:
                    cameraZ -= 2;
                    break;
                case 3:
                    cameraX += 2;
                    break;
            }
            
            
            break;
        case 'w':
            switch (cameraRotation) {
                case 0:
                    cameraZ -= 2;
                    break;
                case 1:
                    cameraX += 2;
                    break;
                case 2:
                    cameraZ += 2;
                    break;
                case 3:
                    cameraX -= 2;
                    break;
            }
            break;
            
            
        default:
            goto waitkey;
    }
    
    if (cameraZ >= 32) {
        cameraZ = 31;
    }
    
    if (cameraX >= 32) {
        cameraX = 31;
    }
    
    if (cameraZ < 0) {
        cameraZ = 0;
    }
    
    if (cameraX < 0) {
        cameraX = 0;
    }
    
    if (patterns[map[cameraZ - 2][cameraX]].ceiling < 2) {
        cameraX = prevX;
        cameraZ = prevZ;
    }
}

void putchar(char dummy) {
    dummy = dummy;
}

int demoMain() {
    /*
     struct ObjectNode* head = getRoom(getPlayerRoom())->itemsPresent->next;
     
     while (head != NULL) {
     map[head->item->position.y][head->item->position.x] |= 128;
     head = head->next;
     }
     */
    running = 1;
    cameraX = 15;
    cameraZ = 15;
    cameraRotation = 0;
    init();
    
    memset(stencilHigh, 0, 64);
    
    do {
        tickRenderer();
    } while (running);
    
    shutdownGraphics();
    
    return 0;
}


#ifdef __MAIN__

void main() {
    
    initStation();
    demoMain();
}
#endif

