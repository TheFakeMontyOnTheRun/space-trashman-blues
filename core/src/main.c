#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "Core.h"
#include "Derelict.h"
#include "Parser.h"
#include "Common.h"

extern struct ObjectNode *collectedObject;

#ifndef USE_CUSTOM_MEMORY_HANDLER
void *allocMem(size_t sizeInBytes, enum MemoryType type, int clearAfterAlloc) {
    /*
    For the general allocator, we're not worried about the type of memory. It all comes from the
    same place.
     */
    void *ptr;

#ifndef N64
    ptr = malloc(sizeInBytes);
#else
    ptr = malloc_uncached(sizeInBytes);
#endif

    if (clearAfterAlloc) {
        memset(ptr, 0, sizeInBytes);
    }

    return ptr;
}

void disposeMem(void* ptr) {
#ifndef N64
    free(ptr);
#else
    free_uncached(ptr);
#endif

}

void memCopyToFrom(void* dst, void* src, size_t sizeInBytes) {
    memcpy(dst, src, sizeInBytes);
}

void memFill(void* dst, uint8_t val, size_t sizeInBytes) {
    memset(dst, val, sizeInBytes);
}
#endif

int main(int argc, char**argv) {
	int d;
	struct Room *currentRoom;
	initStation();

	while (1) {
        	int x, y;
		char buffer[255];
		char *operator;
		char *operand;
		struct WorldPosition playerPos = *getPlayerPosition();
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
			  if (x == getItem(head->item)->position.x && y == getItem(head->item)->position.y) {
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
		  printf("%s%s\n", getItem(head->item)->active ? "[*]" : "[ ]" , getItem(head->item)->name);
		  head = head->next;
		}

		head = getRoom(getPlayerRoom())->itemsPresent->next;

		if (head != NULL) {
			puts("\nObjects in room:");
		}

		while (head != NULL) {
		  printf("%s%s (%d, %d)\n", getItem(head->item)->active ? "[*]" : "[ ]" , getItem(head->item)->name, getItem(head->item)->position.x, getItem(head->item)->position.y);
		  head = head->next;
		}

		puts("\nPlaces you can go:");

		currentRoom = getRoom(getPlayerRoom());

		for (d = 0; d < 6; ++d) {

			int connection = currentRoom->connections[d];

			if (connection) {
				struct Room *r;
				r = getRoom(connection);
				snprintf(&buffer[0], 255, "%d - %s", d, r->name);
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
