/*
Created by Daniel Monteiro on 2019-07-26.
*/

#ifndef DERELICT_CORE_H
#define DERELICT_CORE_H

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

/**
 * Defines the maximum total of rooms.
 * @see roomCount for the current number of created rooms
 */
#define TOTAL_ROOMS 24
/**
 * Defines the maximum total of items
 * @see itemCount for the number of created items
 */
#define TOTAL_ITEMS 39

/* just a forward declaration, don't bother */
struct Item;

/**
 *
 */
typedef void ( *ErrorHandlerCallback )(const char *error);

/**
 *
 */
typedef void ( *PickObjectCallback )(struct Item *item);

/**
 *
 */
typedef void ( *DropObjectCallback )(struct Item *item);

/**
 *
 */
typedef void ( *UseObjectCallback )(struct Item *item);

/**
 *
 */
typedef void ( *UseWithObjectCallback )(struct Item *item,
                                        struct Item *otherItem);

/**
 *
 */
typedef void ( *LogDelegate )(const char *item);

/**
 *
 */
struct WorldPosition {
    int8_t x;
    int8_t y;
};
/**
 *
 */
struct Item {
    struct WorldPosition position;
    uint8_t roomId: 6;
    uint8_t index: 6;
    uint8_t active: 1;
    uint8_t pickable: 1;
    const char *name;
#ifdef INCLUDE_ITEM_DESCRIPTIONS
    const char *info;
#endif
    UseWithObjectCallback useWithCallback;
    UseObjectCallback useCallback;
    DropObjectCallback dropCallback;
    PickObjectCallback pickCallback;
};
/**
 *
 */
struct ObjectNode {
    uint8_t item;
    struct ObjectNode *next;
};
/**
 *
 */
struct Room {
    const char *name;
#ifdef INCLUDE_ROOM_DESCRIPTIONS
    const char *info;
#endif
    uint8_t connections[6];
    struct ObjectNode *itemsPresent;
    uint8_t chanceOfRandomBattle;
    uint8_t sizeX;
    uint8_t sizeY;
    uint8_t rankRequired;
};
/**
 *
 */
enum EGameStates {
    kNormalGameplay,
    kGoodVictory,
    kBadVictory,
    kGoodGameOver,
    kBadGameOver
};
/**
 *
 */
extern ErrorHandlerCallback errorHandlerCallback;

/**
 * @brief inits the basic internal lists for allowing the inclusion of items and
 * rooms
 */
void initCore(void);

/**
 * @brief force new game state
 *
 * @detail the game state means the real outcome of the play session. Clients
 * are free to halt into an endless loop depending on the set state
 * @param newStatus to be forced into the game
 */
void setGameStatus(enum EGameStates newStatus);

/**
 *
 * @return
 */
enum EGameStates getGameStatus(void);

/**
 *
 * @return
 */
int8_t getPlayerDirection(void);

/**
 *
 * @param direction
 */
void setPlayerDirection(uint8_t direction);

/**
 *
 * @param direction
 */
void moveBy(uint8_t direction);

/**
 *
 * @return
 */
struct WorldPosition *getPlayerPosition(void);

/**
 *
 * @param pos
 */
void setPlayerPosition(struct WorldPosition *pos);

/**
 *
 * @param x
 * @param y
 * @return
 */
uint8_t isPositionAllowed(int8_t x, int8_t y);

/**
 *
 * @param operands
 */
void walkTo(const char *operands);

/**
 *
 */
void turnLeft(void);

/**
 *
 */
void turnRight(void);

/**
 *
 * @param direction
 */
void walkBy(uint8_t direction);

/**
 *
 * @return
 */
uint8_t getPlayerRank(void);

/**
 *
 * @param newRank
 */
void setPlayerRank(uint8_t newRank);

/**
 *
 */
void showInventory(void);

/**
 *
 * @param itemName
 * @return
 */
uint8_t playerHasObject(const char *itemName);

/**
 *
 * @param location
 */
void setPlayerLocation(uint8_t location);

/**
 *
 * @return
 */
uint8_t getPlayerRoom(void);

/**
 *
 * @param roomName
 * @return
 */
uint8_t isPlayerAtRoom(const char *roomName);

/**
 *
 * @param description
 * @param pickable
 * @param positionX
 * @param positionY
 * @return
 */
struct Item *addItem(const char *description,
#ifdef INCLUDE_ITEM_DESCRIPTIONS
        const char *info,
#endif
#ifdef ITEMS_HAVE_WEIGHT
        uint8_t weight,
#endif
                     uint8_t pickable,
                     int8_t positionX,
                     int8_t positionY);

/**
 *
 * @return
 */
struct ObjectNode *getPlayerItems(void);

/**
 *
 * @param index
 * @return
 */
struct Item *getItem(uint8_t index);

/**
 *
 * @param pos
 * @param _item
 * @return
 */
uint8_t isCloseToObject(struct WorldPosition *pos, struct Item *_item);

/**
 *
 * @param itemToPick
 */
void pickObject(struct Item *itemToPick);

/**
 *
 * @param objName
 */
void pickObjectByName(const char *objName);

/**
 *
 * @param objName
 */
void dropObjectByName(const char *objName);

/**
 *
 * @param roomName
 * @param itemName
 * @return
 */
uint8_t hasItemInRoom(const char *roomName, const char *itemName);

/**
 *
 * @param roomName
 * @param itemName
 */
void addToRoom(const char *roomName, struct Item *itemName);

/**
 *
 * @param description
 * @param sizeX
 * @param sizeY
 * @param chanceOfRandomBattle
 * @param connections
 * @return
 */
struct Room *addRoom(
        const char *description,
#ifdef INCLUDE_ROOM_DESCRIPTIONS
        const char *info,
#endif
        uint8_t sizeX, uint8_t sizeY, uint8_t chanceOfRandomBattle,
        const int8_t connections[6]);

/**
 *
 * @param roomId
 * @param itemToAdd
 */
void addObjectToRoom(uint8_t roomId, struct Item *itemToAdd);

/**
 *
 * @param roomId
 * @param itemToDrop
 */
void dropObjectToRoom(uint8_t roomId, struct Item *itemToDrop);

/**
 *
 * @param name
 * @return
 */
struct Item *getItemNamed(const char *name);

#ifdef INCLUDE_ITEM_DESCRIPTIONS
/**
 *
 */
void infoAboutItemNamed(const char* itemName);
#endif

/**
 *
 * @return
 */
const char *getRoomDescription(void);

/**
 *
 * @param index
 * @return
 */
struct Room *getRoom(uint8_t index);

/**
 *
 * @param name
 * @return
 */
uint8_t getRoomIdByName(const char *name);

/**
 *
 * @param name
 * @return
 */
struct Room *getRoomByName(const char *name);

/**
 *
 * @param itemToRemove
 */
void removeObjectFromRoom(struct Item *itemToRemove);

/**
 *
 * @param operand
 */
void useObjectNamed(const char *operand);

/**
 *
 * @param operands
 */
void useObjectsTogether(const char *operands);

/**
 *
 * @param newDelegate
 */
void setLoggerDelegate(LogDelegate newDelegate);

/**
 *
 * @param callback
 */
void setErrorHandlerCallback(ErrorHandlerCallback callback);

/**
 *
 * @param itemToAdd
 * @param listHead
 */
void addObjectToList(struct Item *itemToAdd, struct ObjectNode *listHead);

/**
 *
 * @param itemToRemove
 * @param listHead
 */
void
removeObjectFromList(struct Item *itemToRemove, struct ObjectNode *listHead);

/**
 *
 * @param listHead
 * @return
 */
uint8_t listIsEmpty(struct ObjectNode *listHead);

/**
 * TODO: make accessory method for this
 */
extern uint8_t itemsCount;
/**
 *
 */
extern LogDelegate defaultLogger;
/**
 *
 */
extern struct ObjectNode objectNodes[TOTAL_ITEMS];
#endif
