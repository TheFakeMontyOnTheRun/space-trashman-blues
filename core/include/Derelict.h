/*
Created by Daniel Monteiro on 2019-07-26.
*/

#ifndef DERELICT_DERELICT_H
#define DERELICT_DERELICT_H

#define TRUE 1
#define FALSE 0

struct Item;

typedef void ( *ErrorHandlerCallback )(const char* error);

typedef void ( *PickObjectCallback )(struct Item* item);

typedef void ( *DropObjectCallback )(struct Item* item);

typedef void ( *UseObjectCallback )(struct Item* item);

typedef void ( *UseWithObjectCallback )(struct Item* item, struct Item* otherItem);

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
  int connections[6];
  struct ObjectNode *itemsPresent;
  int sizeX;
  int sizeY;
};

struct WorldPosition getPlayerPosition();

void setPlayerPosition(struct WorldPosition pos);

int isCloseToObject( struct WorldPosition pos, struct Item* item );

void addObjectToRoom(int roomId, struct Item *itemToAdd);

void dropObjectToRoom(int roomId, struct Item *itemToDrop);

void pickObject(struct Item *itemToPick);

void pickObjectByName(const char *objName);

void dropObjectByName(const char *objName);

void initStation(void);

int getPlayerDirection(void);

void moveBy(int direction);

int getPlayerRoom(void);

int hasItemInRoom(const char *roomName, const char *itemName);

int isPlayerAtRoom(const char *roomName);

char *getRoomDescription();

struct Room *getRoom(int index);

void useObjectNamed(const char* operand);

void useObjectsTogether(const char* operands);

void showInventory();

void notifyError(const char* errorMsg);

void setErrorHandlerCallback(ErrorHandlerCallback callback);

void walkTo(const char* operands);

void turnLeft(void);

void turnRight(void);

void walkBy(int direction);
#endif
