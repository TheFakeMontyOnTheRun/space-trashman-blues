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
        	int x, y;
		char buffer[255];
		char *operator;
		char *operand;
		struct WorldPosition playerPos = getPlayerPosition();
		struct ObjectNode *head = NULL;

		puts("------\nYou are at:");
		puts(getRoomDescription());
		
		for ( y = -1; y <= getRoom(getPlayerRoom())->sizeY; ++y ) {
		  putchar( getRoom(getPlayerRoom())->connections[3] ? '|' : '#');
		  for ( x = 0; x < getRoom(getPlayerRoom())->sizeX; ++x ) {

		    if (y == -1 ) {
		      putchar( getRoom(getPlayerRoom())->connections[0] ? '-' : '=');
		    } else if (y == getRoom(getPlayerRoom())->sizeY) {
		      putchar( getRoom(getPlayerRoom())->connections[2] ? '-' : '=');
		    } else {

		      if (x == playerPos.x && y == playerPos.y) {
			switch(getPlayerDirection()) {
			case 0:
			  putchar('^');
			  break;
			case 1:
			  putchar('>');
			  break;
			case 2:
			  putchar('V');
			  break;
			case 3:
			  putchar('<');
			  break;
			}		   
		      } else {
			int hasObject = 0;
			head = getRoom(getPlayerRoom())->itemsPresent->next;

			while (head != NULL) {
			  if (x == head->item->position.x && y == head->item->position.y) {
			    hasObject = 1;
			  }
			  head = head->next;
			}

			putchar( hasObject ? '*' : '.');
		      }
		    }
		  }
		  puts( getRoom(getPlayerRoom())->connections[1] ? "|" : "#");
		}


		head = collectedObject->next;

		if (head != NULL) {
			puts("\nObjects you have:");
		}

		while (head != NULL) {
			puts(head->item->description);
			head = head->next;
		}

		head = getRoom(getPlayerRoom())->itemsPresent->next;

		if (head != NULL) {
			puts("\nObjects in room:");
		}

		while (head != NULL) {
			printf("%s (%d, %d)\n", head->item->description, head->item->position.x, head->item->position.y);
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

		fgets(&buffer[0], 255, stdin );
		operator = strtok( buffer, "\n " );

		if (operator == NULL ) {
		  continue;
		}

		operand = strtok( NULL, "\n " );
		
		parseCommand(operator, operand);
	}
	return 0;
}
