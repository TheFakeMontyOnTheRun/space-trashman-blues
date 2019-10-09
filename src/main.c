#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Derelict.h"
#include "Parser.h"

extern struct ObjectNode *collectedObject;

int main() {
  initStation();

  while (1) {
    char operator[255];
    char operand[255];
    struct ObjectNode *head = NULL;

    puts("You are at:");
    puts(getRoomDescription());

    head = collectedObject;
    puts("Objects you have:");

    while (head != NULL) {
      puts(head->item->description);
      head = head->next;
    }

    head = station[getPlayerRoom()].itemsPresent;
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
