/*
Created by Daniel Monteiro on 2019-07-26.
*/

#ifndef DERELICT_DERELICT_H
#define DERELICT_DERELICT_H

#define TRUE 1
#define FALSE 0


struct Item {
  char *description;
  int weight;
  int roomId;
};

struct ObjectNode {
  struct Item *item;
  struct ObjectNode *next;
};

struct Room {
  char *description;
  int connections[4];
  struct ObjectNode *itemsPresent;
};

void addObjectToRoom(int roomId, struct Item *itemToAdd);

void dropObjectToRoom(int roomId, struct Item *itemToDrop);

void pickObject(struct Item *itemToPick);

void pickObjectByName(const char* objName );

void dropObjectByName(const char* objName);

void initStation(void);

void moveBy(int direction);

int getPlayerRoom(void);

#endif
