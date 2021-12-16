/*
Created by Daniel Monteiro on 2019-07-26.
*/

#ifndef DERELICT_CORE_H
#define DERELICT_CORE_H

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

enum EGameStates {
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
  int8_t x;
  int8_t y;
};

struct Item {
  struct WorldPosition position;
  uint8_t roomId : 6;
  uint8_t index : 6;
  uint8_t active : 1;
  uint8_t pickable : 1;
  char *name;
  char *info;
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
  char *name;
#ifdef INCLUDE_ROOM_DESCRIPTIONS
    char *info;
#endif
  uint8_t connections[6];
  struct ObjectNode *itemsPresent;
  uint8_t chanceOfRandomBattle;
  uint8_t sizeX;
  uint8_t sizeY;
  uint8_t rankRequired;
};

struct Room *addRoom(
        char *description,
#ifdef INCLUDE_ROOM_DESCRIPTIONS
        char *info,
#endif
        int sizeX, int sizeY, int chanceOfRandomBattle, int connections[6]);

struct Item* addItem(char *description,
                     char *info,
#ifdef ITEMS_HAVE_WEIGHT
        int weight,
#endif
                     int pickable,
                     int positionX,
                     int positionY);

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

enum EGameStates  getGameStatus(void);

struct Item *getItemNamed(const char* name);

int getPlayerDirection(void);

void setPlayerDirection(int direction);

void infoAboutItemNamed(const char* itemName);

void moveBy(int direction);

int getPlayerRoom(void);

void setPlayerLocation(int location);

int hasItemInRoom(const char *roomName, const char *itemName);

int playerHasObject( const char* itemName);

int getPlayerHealth(void);

void setPlayerHealth(int health);

int isPlayerAtRoom(const char *roomName);

char *getRoomDescription(void);

struct Room *getRoom(int index);

void useObjectNamed(const char* operand);

void removeObjectFromList(struct Item *itemToRemove, struct ObjectNode *listHead);

void removeObjectFromRoom(struct Item *itemToRemove);

void useObjectsTogether(const char* operands);

void showInventory(void);

extern LogDelegate defaultLogger;

void setLoggerDelegate(LogDelegate newDelegate);

void setErrorHandlerCallback(ErrorHandlerCallback callback);

void walkTo(const char* operands);

void turnLeft(void);

void turnRight(void);

void walkBy(int direction);

int getPlayerRank(void);

void setPlayerRank(int newRank);

int isPositionAllowed(int x, int y);

uint8_t listIsEmpty(struct ObjectNode *listHead);

void initCore(void);

void setGameStatus(enum EGameStates newStatus);

void useObjectNamed(const char *operand);

#define TOTAL_ROOMS 24
#define TOTAL_ITEMS 39


/* TODO: make accessory method for this */
extern int itemsCount;
extern struct ObjectNode objectNodes[TOTAL_ITEMS];
#endif
