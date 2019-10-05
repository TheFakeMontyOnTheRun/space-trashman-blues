/*
Created by Daniel Monteiro on 2019-07-26.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "Derelict.h"

struct Room station[10];
struct Item item[2];
struct ObjectNode* collectedObject = NULL;
int playerLocation = 2;
struct Room* room;

void addObjectToRoom( int roomId, struct Item* item ) {
    struct Room *room = &station[roomId];
    struct ObjectNode *node = (struct ObjectNode*) calloc(1, sizeof(struct ObjectNode));
    struct ObjectNode *tmp = room->itemsPresent;

    if ( item->roomId != 0 ) {
        struct ObjectNode* head = station[item->roomId].itemsPresent;
        struct ObjectNode* previousNode = station[item->roomId].itemsPresent;

        while (head != NULL ) {
            if (head->item == item) {
                previousNode->next = head->next;
                free(head);
                goto removed;
            }
            previousNode = head;
            head = head->next;
        }
    }

    removed:

    room->itemsPresent = node;
    node->next = tmp;
    node->item = item;
    item->roomId = roomId;
}

void dropObjectToRoom( int roomId, struct Item* item ) {

    assert(item->roomId == 0);

    {
        struct ObjectNode* head = collectedObject;
        struct ObjectNode* previousNode = collectedObject;

        if ( collectedObject != NULL && collectedObject->item == item ) {
            struct ObjectNode* tmp = collectedObject;
            collectedObject = tmp->next;
            free(tmp);
            goto dropped;
        }

        while (head != NULL ) {
            if (head->item == item) {
                previousNode->next = head->next;
                free(head);
                goto dropped;
            }
            previousNode = head;
            head = head->next;
        }
    }

    dropped:
    addObjectToRoom(roomId, item);
}

void pickObject( struct Item* item) {
    struct ObjectNode *node = (struct ObjectNode*) calloc(1, sizeof(struct ObjectNode));
    struct ObjectNode *tmp = collectedObject;

    if ( item->roomId != 0 ) {

        struct ObjectNode* head = station[item->roomId].itemsPresent;
        struct ObjectNode* previousNode = station[item->roomId].itemsPresent;

        if ( previousNode->item == item ) {
            station[item->roomId].itemsPresent = previousNode->next;
            free(previousNode);
            goto taken;
        }

        while (head != NULL ) {
            if (head->item == item) {
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
    node->item = item;
    item->roomId = 0;
}

void moveBy(int direction){
  printf("input: %d = %d\n", direction, room->connections[direction ] );
  playerLocation = room->connections[direction ];
  room = &station[playerLocation];
}

void initStation() {
    collectedObject = NULL;
    playerLocation = 2;


    memset( &station, 0, 10 * sizeof(struct Room));
    memset( &item, 0, 2 * sizeof(struct Item));
    station[1].description = "Ship";
    station[1].connections[0] = 2;


    station[2].description = "Hangar";
    station[2].connections[2] = 1;
    station[2].connections[0] = 3;

    station[3].description = "Hall 1";
    station[3].connections[2] = 2;
    station[3].connections[0] = 4;
    station[3].connections[1] = 5;

    station[4].description = "Elevator 1";
    station[4].connections[2] = 3;

    station[5].description = "Dorms 1";
    station[5].connections[3] = 3;

    item[0].description = "Treco";
    item[0].weight = 5;

    item[1].description = "Bagulho";
    item[1].weight = 5;


    addObjectToRoom(2, &item[0]);
    addObjectToRoom(2, &item[1]);

    playerLocation = 1;
    room = &station[playerLocation];
}
