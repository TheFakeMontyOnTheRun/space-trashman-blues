#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Derelict.h"
#include "Parser.h"

extern struct Room station[10];
extern struct Item item[2];
extern struct ObjectNode *collectedObject;
extern int playerLocation;
extern struct Room *room;

int main() {
  initStation();

  while (1) {
    char operator[255];
    char operand[255];
    struct ObjectNode *head = NULL;

    puts("You are at:");
    puts(room->description);

    head = collectedObject;
    puts("Objects you have:");

    while (head != NULL) {
      puts(head->item->description);
      head = head->next;
    }

    head = room->itemsPresent;
    puts("Objects in room:");

    while (head != NULL) {
      puts(head->item->description);
      head = head->next;
    }


    scanf("%s %s", &operator[0], &operand[0]);
    parseCommand(&operator[0], &operand[0]);

  }
  return 0;
}
