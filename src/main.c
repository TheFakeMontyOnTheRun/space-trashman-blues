#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "Derelict.h"

extern struct Room station[10];
extern struct Item item[2];
extern struct ObjectNode* collectedObject;
extern int playerLocation;
extern struct Room* room;

int main() {
    initStation();


    while ( 1 ) {
        int input;
        struct ObjectNode* head = NULL;

        puts("You are at:");
        puts(room->description);

        head = collectedObject;
        puts("Objects you have:");

        while (head != NULL ) {
            puts(head->item->description);
            head = head->next;
        }

        head = room->itemsPresent;
        puts("Objects in room:");

        while (head != NULL ) {
            puts(head->item->description);
            head = head->next;
        }

        do {
            input = getchar() - '1';
        } while ( (( input  < 0 ) || (3 < input )) && (input != ('a' - '1')) && (input != ('q' - '1')) && (input != ('z' - '1')) );

        if (input == ('a' - '1')) {

          puts("Picking up:");
            puts(item[0].description);
            pickObject(&item[0]);

        } else if (input == ('z' - '1')) {

          puts("Dropping:");
            puts(item[0].description);
            dropObjectToRoom( playerLocation, &item[0]);

        } else if (input == ('q' - '1')) {
            exit(0);
        } else if (room->connections[input ]) {
            moveBy(input);
        }
    }
    return 0;
}
