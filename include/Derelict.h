/*
Created by Daniel Monteiro on 2019-07-26.
*/

#ifndef DERELICT_DERELICT_H
#define DERELICT_DERELICT_H

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

void addObjectToRoom(int roomId, struct Item *item);

void dropObjectToRoom(int roomId, struct Item *item);

void pickObject(struct Item *item);

void initStation();

void moveBy(int direction);

#endif
