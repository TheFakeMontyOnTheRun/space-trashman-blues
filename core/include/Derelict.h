/*
Created by Daniel Monteiro on 2019-07-26.
*/

#ifndef DERELICT_DERELICT_H
#define DERELICT_DERELICT_H

#define TRUE 1
#define FALSE 0

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

struct WorldPosition getPlayerPosition();

struct Item* getItem(int index);

struct ObjectNode* getPlayerItems();

void setPlayerPosition(struct WorldPosition pos);

void addToRoom( const char* roomName, struct Item *itemName );

int isCloseToObject( struct WorldPosition pos, struct Item* item );

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
#endif
