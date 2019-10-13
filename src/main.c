#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Derelict.h"
#include "Parser.h"

extern struct ObjectNode *collectedObject;

int main() {
	int d;
	struct Room *currentRoom;
	initStation();

	while (1) {
		char operator[255];
		char operand[255];
		struct ObjectNode *head = NULL;

		puts("------\nYou are at:");
		puts(getRoomDescription());

		head = collectedObject;

		if (head != NULL) {
			puts("\nObjects you have:");
		}

		while (head != NULL) {
			puts(head->item->description);
			head = head->next;
		}

		head = getRoom(getPlayerRoom())->itemsPresent;

		if (head != NULL) {
			puts("\nObjects in room:");
		}

		while (head != NULL) {
			puts(head->item->description);
			head = head->next;
		}

		puts("\nPlaces you can go:");

		currentRoom = getRoom(getPlayerRoom());

		for (d = 0; d < 6; ++d) {

			int connection = currentRoom->connections[d];

			if (connection) {
				char buffer[255];
				struct Room *r;
				r = getRoom(connection);
				snprintf(&buffer[0], 255, "%d - %s", d, r->description);
				puts(buffer);
			}
		}

		puts("");

		scanf("%s %s", &operator[0], &operand[0]);
		parseCommand(&operator[0], &operand[0]);
	}
	return 0;
}
