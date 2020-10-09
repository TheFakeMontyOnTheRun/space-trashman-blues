/*
Created by Daniel Monteiro on 2019-07-26.
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "Derelict.h"

#define TOTAL_ROOMS 21
struct Room station[TOTAL_ROOMS];
struct Item item[2];
struct ObjectNode *collectedObject = NULL;
int playerLocation = 1;
struct WorldPosition playerPosition;
ErrorHandlerCallback errorHandlerCallback = NULL;


void notifyError(const char* errorMsg) {
  if (errorHandlerCallback == NULL) {
    puts("-------");
    puts(errorMsg);
    puts("--!!---");
  } else {
    errorHandlerCallback(errorMsg);
  }
} 

void setErrorHandlerCallback(ErrorHandlerCallback callback) {
  errorHandlerCallback = callback;
}

struct WorldPosition getPlayerPosition() {
  return playerPosition;
}

void setPlayerPosition(struct WorldPosition pos) {
  playerPosition = pos;
}

int isCloseToObject( struct WorldPosition pos, struct Item* item ) {
  return (abs(pos.x - item->position.x) + abs(pos.y + item->position.y) ) <= 1;
}

void addObjectToRoom(int roomId, struct Item *itemToAdd) {
	struct Room *roomToAddObject = &station[roomId];
	struct ObjectNode *node =
			(struct ObjectNode *) calloc(1, sizeof(struct ObjectNode));
	struct ObjectNode *tmp = roomToAddObject->itemsPresent;

	if (itemToAdd->roomId != 0) {
		struct ObjectNode *head = station[itemToAdd->roomId].itemsPresent;
		struct ObjectNode *previousNode = station[itemToAdd->roomId].itemsPresent;

		while (head != NULL) {
			if (head->item == itemToAdd) {
				previousNode->next = head->next;
				free(head);
				goto removed;
			}
			previousNode = head;
			head = head->next;
		}
	}

	removed:

	roomToAddObject->itemsPresent = node;
	node->next = tmp;
	node->item = itemToAdd;
	itemToAdd->roomId = roomId;
}

void dropObjectToRoom(int roomId, struct Item *itemToDrop) {

	assert(itemToDrop->roomId == 0);

	{
		struct ObjectNode *head = collectedObject;
		struct ObjectNode *previousNode = collectedObject;

		if (collectedObject != NULL && collectedObject->item == itemToDrop) {
			struct ObjectNode *tmp = collectedObject;
			collectedObject = tmp->next;
			free(tmp);
			goto dropped;
		}

		while (head != NULL) {
			if (head->item == itemToDrop) {
				previousNode->next = head->next;
				free(head);
				goto dropped;
			}
			previousNode = head;
			head = head->next;
		}
	}

	dropped:
	addObjectToRoom(roomId, itemToDrop);
}

void pickObject(struct Item *itemToPick) {
	struct ObjectNode *node =
			(struct ObjectNode *) calloc(1, sizeof(struct ObjectNode));
	struct ObjectNode *tmp = collectedObject;

	if (!isCloseToObject(getPlayerPosition(), itemToPick)) {
	  return;
	}

	if (itemToPick->roomId != 0) {

		struct ObjectNode *head = station[itemToPick->roomId].itemsPresent;
		struct ObjectNode *previousNode = station[itemToPick->roomId].itemsPresent;

		if (previousNode->item == itemToPick) {
			station[itemToPick->roomId].itemsPresent = previousNode->next;
			free(previousNode);
			goto taken;
		}

		while (head != NULL) {
			if (head->item == itemToPick) {
				previousNode->next = head->next;
				free(head);
				goto taken;
			}
			previousNode = head;
			head = head->next;
		}
	}
	taken:
	collectedObject = node;
	node->next = tmp;
	node->item = itemToPick;
	itemToPick->roomId = 0;
}

void moveBy(int direction) {
	struct Room *room = &station[playerLocation];
	if (direction >= 0 && direction <= 5 && room->connections[direction] != 0) {
		playerLocation = room->connections[direction];
		room = &station[playerLocation];
	} else {
	  notifyError("Please specify a valid direction");
	}
}

void pickObjectByName(const char *objName) {
	struct Room *room = &station[playerLocation];
	struct ObjectNode *itemToPick = room->itemsPresent;

	while (itemToPick != NULL) {
		if (!strcmp(itemToPick->item->description, objName)) {
		  playerPosition = itemToPick->item->position;
		  pickObject(itemToPick->item);
		  return;
		}
		itemToPick = itemToPick->next;
	}
}

void dropObjectByName(const char *objName) {
	struct ObjectNode *itemToPick = collectedObject;

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
	  notifyError("Either the object name or the room name are null. Check your stuff");
	  return 0;
	}

	for (r = 1; r < TOTAL_ROOMS; ++r) {
		char *desc = station[r].description;

		if (desc != NULL && !strcmp(desc, roomName)) {
			struct ObjectNode *itemToPick = station[r].itemsPresent;

			while (itemToPick != NULL) {
				if (!strcmp(itemToPick->item->description, itemName)) {
					return 1;
				}
				itemToPick = itemToPick->next;
			}
			return 0;
		}
	}
	notifyError("It was not possible to determine if object is in room");
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

int getPlayerRoom(void) { return playerLocation; }

void useObjectNamed(const char* operand) {
  struct ObjectNode *itemToPick = collectedObject;
  
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

void walkTo(const char* operands) {
  struct WorldPosition pos;
  char *xStr = operands;
  char *yStr = strtok(NULL, "\n " );
  int x = atoi(xStr);
  int y = atoi(yStr);
  pos.x = x;
  pos.y = y;
  setPlayerPosition(pos);
}

void useObjectsTogether(const char* operands){

  struct ObjectNode *object1 = collectedObject;
  struct Room *room = &station[playerLocation];
  struct ObjectNode *object2 = room->itemsPresent;

  char *operand1 = operands;
  char *operand2 = strtok(NULL, "\n " );

  assert(operand1 != NULL);
  assert(operand2 != NULL);

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
  if( object1->item->useWithCallback != NULL) {
    object1->item->useWithCallback(object1->item, object2->item);
  }
}

void initStation(void) {

	setErrorHandlerCallback(NULL);
	collectedObject = NULL;
	playerLocation = 1;

	memset(&station, 0, 21 * sizeof(struct Room));
	memset(&item, 0, 2 * sizeof(struct Item));
    
    
	/*Rooms*/
	station[1].description = "uss-daedalus";
	station[1].connections[0] = 2;

	station[2].description = "hangar";
	station[2].connections[2] = 1;
	station[2].connections[0] = 3;

	station[3].description = "hall-1";
	station[3].connections[2] = 2;
	station[3].connections[0] = 4;
	station[3].connections[1] = 5;

	station[4].description = "elevator-level-1";
	station[4].connections[2] = 3;

	station[5].description = "dorms-1";
	station[5].connections[3] = 3;

	/*Items*/    
    
	item[0].description = "metal-plate";
	item[0].weight = 5;

	item[1].description = "boots";
	item[1].weight = 5;

	item[1].description = "key";
	item[1].weight = 5;

	addObjectToRoom(2, &item[0]);
	addObjectToRoom(2, &item[1]);

	playerLocation = 1;
}
