/*
Created by Daniel Monteiro on 2019-07-26.
*/

#ifndef DERELICT_CORE_H
#define DERELICT_CORE_H

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define TOTAL_ROOMS 24
#define TOTAL_ITEMS 39

struct Item;

typedef void ( *ErrorHandlerCallback )(const char* error);

typedef void ( *PickObjectCallback )(struct Item* item);

typedef void ( *DropObjectCallback )(struct Item* item);

typedef void ( *UseObjectCallback )(struct Item* item);

typedef void ( *UseWithObjectCallback )(struct Item* item, struct Item* otherItem);

typedef void ( *LogDelegate )(const char* item);

struct WorldPosition {
  int8_t x;
  int8_t y;
};

struct Item {
  struct WorldPosition position;
  uint8_t roomId : 6;
  uint8_t index : 6;
  uint8_t active : 1;
  uint8_t pickable : 1;
  const char *name;
#ifdef INCLUDE_ITEM_DESCRIPTIONS
  const char *info;
#endif
  UseWithObjectCallback useWithCallback;
  UseObjectCallback useCallback;
  DropObjectCallback dropCallback;
  PickObjectCallback pickCallback;
};

struct ObjectNode {
  uint8_t item;
  struct ObjectNode *next;
};

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

enum EGameStates {
	kNormalGameplay,
	kGoodVictory,
	kBadVictory,
	kGoodGameOver,
	kBadGameOver
};

extern ErrorHandlerCallback errorHandlerCallback;

void initCore(void);

void setGameStatus(enum EGameStates newStatus);

enum EGameStates  getGameStatus(void);

uint8_t getPlayerDirection(void);

void setPlayerDirection(uint8_t direction);

void moveBy(uint8_t direction);

struct WorldPosition* getPlayerPosition(void);

void setPlayerPosition(struct WorldPosition* pos);

uint8_t isPositionAllowed(int8_t x, int8_t y);

void walkTo(const char* operands);

void turnLeft(void);

void turnRight(void);

void walkBy(uint8_t direction);

uint8_t getPlayerRank(void);

void setPlayerRank(uint8_t newRank);

void showInventory(void);

uint8_t playerHasObject( const char* itemName);

void setPlayerLocation(uint8_t location);

uint8_t getPlayerRoom(void);

uint8_t isPlayerAtRoom(const char *roomName);

struct Item* addItem(const char *description,
#ifdef INCLUDE_ITEM_DESCRIPTIONS
                     const char *info,
#endif
#ifdef ITEMS_HAVE_WEIGHT
		uint8_t weight,
#endif
					 uint8_t pickable,
					 int8_t positionX,
					 int8_t positionY);

struct ObjectNode* getPlayerItems(void);

struct Item* getItem(uint8_t index);

uint8_t isCloseToObject( struct WorldPosition* pos, struct Item* _item );

void pickObject(struct Item *itemToPick);

void pickObjectByName(const char *objName);

void dropObjectByName(const char *objName);

uint8_t hasItemInRoom(const char *roomName, const char *itemName);

void addToRoom( const char* roomName, struct Item *itemName );

struct Room *addRoom(
		const char *description,
#ifdef INCLUDE_ROOM_DESCRIPTIONS
		const char *info,
#endif
		uint8_t sizeX, uint8_t sizeY, uint8_t chanceOfRandomBattle, const int8_t connections[6]);

void addObjectToRoom(uint8_t roomId, struct Item *itemToAdd);

void dropObjectToRoom(uint8_t roomId, struct Item *itemToDrop);


struct Item *getItemNamed(const char* name);

#ifdef INCLUDE_ITEM_DESCRIPTIONS
void infoAboutItemNamed(const char* itemName);
#endif

const char *getRoomDescription(void);

struct Room *getRoom(uint8_t index);

struct Room *getRoomByName(const char *name);

void removeObjectFromRoom(struct Item *itemToRemove);

void useObjectNamed(const char* operand);

void useObjectsTogether(const char* operands);

void setLoggerDelegate(LogDelegate newDelegate);

void setErrorHandlerCallback(ErrorHandlerCallback callback);

void removeObjectFromList(struct Item *itemToRemove, struct ObjectNode *listHead);

uint8_t listIsEmpty(struct ObjectNode *listHead);

/* TODO: make accessory method for this */
extern uint8_t itemsCount;

extern LogDelegate defaultLogger;

extern struct ObjectNode objectNodes[TOTAL_ITEMS];
#endif
