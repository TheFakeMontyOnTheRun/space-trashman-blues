/*
Created by Daniel Monteiro on 2019-07-26.
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "Derelict.h"

#define TOTAL_ROOMS 24
#define TOTAL_ITEMS 26

struct Room station[TOTAL_ROOMS];
struct Item item[TOTAL_ITEMS];
struct ObjectNode *collectedObject = NULL;
int playerLocation = 1;
int playerDirection;
int playerRank;
int gameStatus;
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
  return (abs(pos.x - item->position.x) + abs(pos.y - item->position.y) ) <= 1;
}

int getGameStatus() {
  return gameStatus;
}

void setGameStatus(int newStatus) {
  gameStatus = newStatus;
}

void addObjectToList(struct Item* itemToAdd, struct ObjectNode* listHead) {
  struct ObjectNode *head = listHead;

  while ( head->next != NULL ) {
    if (head->item == itemToAdd) {
      /* Object already belongs to the list! */
      return;
    }

    head = head->next;
  }

  head->next = (struct ObjectNode *) calloc(1, sizeof(struct ObjectNode));  
  head->next->item = itemToAdd;
}

void removeObjectFromList(struct Item* itemToRemove, struct ObjectNode* listHead) {
  struct ObjectNode *head = listHead->next;
  struct ObjectNode *prev = listHead;

  while ( head != NULL ) {
    if (head->item == itemToRemove) {
      prev->next = head->next;
      free(head);
      return;
    }

    prev = head;
    head = head->next;
  }
  /* Object doesn't belongs to the list! */
}


void removeObjectFromRoom(struct Item *itemToRemove) {
  if (itemToRemove->roomId != 0) {
    removeObjectFromList(itemToRemove, station[itemToRemove->roomId].itemsPresent);    
    itemToRemove->roomId = 0;
  }
}


void addObjectToRoom(int roomId, struct Item *itemToAdd) {
  struct Room *roomToAddObject = &station[roomId];
  removeObjectFromRoom(itemToAdd);
  addObjectToList(itemToAdd, roomToAddObject->itemsPresent);
  itemToAdd->roomId = roomId;
}

void dropObjectToRoom(int roomId, struct Item *itemToDrop) {

  if(itemToDrop->roomId != 0) {
    notifyError("Object not present to drop");
  }

  removeObjectFromList(itemToDrop, collectedObject);
  addObjectToRoom(roomId, itemToDrop);

  if (itemToDrop->dropCallback != NULL ) {
    itemToDrop->dropCallback(itemToDrop);
  }
}

void pickObject(struct Item *itemToPick) {
   
  if (!isCloseToObject(getPlayerPosition(), itemToPick)) {
    return;
  }

  if (!itemToPick->pickable) {
    notifyError("Can't pick it up");
    return;
  }
 
  removeObjectFromRoom(itemToPick);
  addObjectToList(itemToPick, collectedObject);

  if (itemToPick->pickCallback != NULL ) {
    itemToPick->pickCallback(itemToPick);
  }
}

int getPlayerRank() {
  return playerRank;
}

void setPlayerRank(int newRank) {
  playerRank = newRank;
}

void moveBy(int direction) {
  struct Room *room = &station[playerLocation];
  if (direction >= 0 && direction <= 5 && room->connections[direction] != 0) {

    if (station[room->connections[direction]].rankRequired > playerRank ) {
      notifyError("Insuficient rank to enter room");
      return;
    }

    if (!item[5].active || !playerHasObject("magnetic-boots")) {
      puts("You can't move without your magnetic-boots!");
    }

    playerLocation = room->connections[direction];
    room = &station[playerLocation];

    switch(direction) {
    case 0:
      playerPosition.x = station[playerLocation].sizeX / 2;
      playerPosition.y = station[playerLocation].sizeY - 1;
      break;

    case 1:
      playerPosition.x = 0;
      playerPosition.y = station[playerLocation].sizeY / 2;
      break;

    case 2:
      playerPosition.x = station[playerLocation].sizeX / 2;
      playerPosition.y = 0;
      break;

    case 3:
      playerPosition.x = station[playerLocation].sizeX - 1;
      playerPosition.y = station[playerLocation].sizeY / 2;
      break;
    }

  } else {
    notifyError("Please specify a valid direction");
  }
}

void pickObjectByName(const char *objName) {
  struct Room *room = &station[playerLocation];
  struct ObjectNode *itemToPick = room->itemsPresent->next;
  
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
  struct ObjectNode *itemToPick = collectedObject->next;
  
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
      struct ObjectNode *itemToPick = station[r].itemsPresent->next;
      
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

int playerHasObject( const char* itemName) {
  struct ObjectNode *itemToPick = collectedObject->next;
  
  while (itemToPick != NULL) {
    if (!strcmp(itemToPick->item->description, itemName)) {
      return 1;
    }
    itemToPick = itemToPick->next;
  }
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
  struct ObjectNode *itemToPick = collectedObject->next;
  
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

  if (playerLocation != 0 && (!item[5].active || !playerHasObject("magnetic-boots"))) {
    puts("You can't move without your magnetic-boots!");
  }

  struct WorldPosition pos;
  char *xStr = operands;
  char *yStr = strtok(NULL, "\n " );
  int x = atoi(xStr);
  int y = atoi(yStr);
  pos.x = x;
  pos.y = y;
  setPlayerPosition(pos);
}

void infoAboutItemNamed(const char* itemName) {

  struct ObjectNode *object1 = collectedObject->next;
  struct Room *room = &station[playerLocation];
  struct ObjectNode *object2 = room->itemsPresent->next;

  if(itemName == NULL || strlen(itemName) == 0) {
    puts(room->info);
    return;
  }

  while (object1 != NULL) {
    assert(object1->item->description != NULL);

    if (!strcmp(object1->item->description, itemName)) {
      puts(object1->item->info);
      return;
    }
    object1 = object1->next;
  }

  while (object2 != NULL) {
    assert(object2->item->description != NULL);
    
    if (!strcmp(object2->item->description, itemName)) {
      puts(object2->item->info);
      return;
    }
    object2 = object2->next;
  }

  notifyError("No such item could be found");
}

void useObjectsTogether(const char* operands){

  struct ObjectNode *object1 = collectedObject->next;
  struct Room *room = &station[playerLocation];
  struct ObjectNode *object2 = room->itemsPresent->next;

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

void useBlowtorchWithCallback(struct Item* item1, struct Item* item2) {
  item2->pickable = TRUE;
}

void useBootsWithCallback(struct Item* item1, struct Item* item2) {
  if (item2 == &item[17]) {
    addToRoom("restroom", &item[25] );
  }
}

void turnLeft(void) {
  playerDirection--;

  while (playerDirection < 0 ) {
    playerDirection += 4;
  }
}

void turnRight(void) {
  playerDirection++;

  playerDirection = playerDirection & 3;
}

void walkBy(int direction) {

  if (playerLocation != 0 && (!item[5].active || !playerHasObject("magnetic-boots"))) {
    puts("You can't move without your magnetic-boots!");
  }

  switch ( direction ) {
  case 0:
    switch( playerDirection) {
    case 0:
      playerPosition.y--;
      break;
    case 1:
      playerPosition.x++;
      break;
    case 2:
      playerPosition.y++;
      break;
    case 3:
      playerPosition.x--;
      break;
    }
    break;
  case 1:
    switch( playerDirection) {
    case 0:
      playerPosition.x++;
      break;
    case 1:
      playerPosition.y++;
      break;
    case 2:
      playerPosition.x--;
      break;
    case 3:
      playerPosition.y--;
      break;
    }
    break;
  case 2:
    switch( playerDirection) {
    case 0:
      playerPosition.y++;
      break;
    case 1:
      playerPosition.x--;
      break;
    case 2:
      playerPosition.y--;
      break;
    case 3:
      playerPosition.x++;
      break;
    }
    break;
  case 3:
    switch( playerDirection) {
    case 0:
      playerPosition.x--;
      break;
    case 1:
      playerPosition.y--;
      break;
    case 2:
      playerPosition.x++;
      break;
    case 3:
      playerPosition.y++;
      break;
    }
    break;
  }

  if (playerPosition.x < 0 ) {
    if (getRoom(playerLocation)->connections[3]) {
      moveBy(3);
    } else {
      playerPosition.x = 0;
    }
  }

  if (playerPosition.y < 0 ) {
    if (getRoom(playerLocation)->connections[0]) {
      moveBy(0);
    } else {
      playerPosition.y = 0;
    }
  }

  
  if (playerPosition.x >= station[playerLocation].sizeX ) {
    if (getRoom(playerLocation)->connections[1]) {
      moveBy(1);
    } else {
      playerPosition.x = station[playerLocation].sizeX - 1;
    }
  }
  
  if (playerPosition.y >= station[playerLocation].sizeY ) {
    if (getRoom(playerLocation)->connections[2]) {
      moveBy(2);
    } else {
      playerPosition.y = station[playerLocation].sizeY - 1;
    }
  }
}

int getPlayerDirection(void) {
  return playerDirection;
}

void addToRoom( const char* roomName, struct Item *itemName ) {
  int r = 0;

  if (roomName == NULL || itemName == NULL || strlen(roomName) == 0 || strlen(itemName) == 0) {
    notifyError("Either the object name or the room name are null. Check your stuff");
    return;
  }
  
  for (r = 1; r < TOTAL_ROOMS; ++r) {
    char *desc = station[r].description;
    
    if (desc != NULL && !strcmp(desc, roomName)) {
      addObjectToRoom(r, itemName );
      return;
    }
  }
  notifyError("It was not possible to determine the room to add object");
}


void updateRankFromKeycards() {

  struct ObjectNode *itemToPick = collectedObject->next;
  int newRank = 0;
  while (itemToPick != NULL) {
    int rank = 0;

    if (itemToPick->item == &item[7]) {
      rank = 1;
    }

    if (itemToPick->item == &item[18]) {
      rank = 2;
    }

    if (itemToPick->item == &item[25]) {
      rank = 3;
    }

    if (newRank < rank ) {
      newRank = rank;
    }

    itemToPick = itemToPick->next;
  }

  setPlayerRank(newRank);
}

void keycardPickCallback(struct Item* item) {
  updateRankFromKeycards();
}


void keycardDropCallback(struct Item* item) {
  updateRankFromKeycards();
}


void bombActivatedCallback(struct Item* item) {

  if (!hasItemInRoom("lab-1", "metal-mending")) {
    if (playerLocation != 1 ) {
      setGameStatus(kBadVictory);    
    } else {
      setGameStatus(kGoodVictory);    
    }
  } else {
    if (playerLocation != 1 ) {
      setGameStatus(kBadGameOver);
    } else {
      setGameStatus(kGoodGameOver);
    }
  }
}

void bombControllerActivatedCallback(struct Item* item) {
  bombActivatedCallback(NULL);
}

void cantBeUsedCallback(struct Item* item) {
  puts("You can't use it like this.");
}

void cantBeUsedWithOthersCallback(struct Item* item1, struct Item* item2) {
  puts("Nothing happens.");
}

void useObjectToggleCallback(struct Item* item) {
  item->active = !item->active;
}

void initStation(void) {

	setErrorHandlerCallback(NULL);
	collectedObject = (struct ObjectNode*)calloc(1, sizeof(struct ObjectNode));
	playerLocation = 1;
	playerRank = 0;
	gameStatus = 0;
	playerDirection = 0;
	memset(&station, 0, TOTAL_ROOMS * sizeof(struct Room));
	memset(&item, 0, TOTAL_ITEMS * sizeof(struct Item));
        
	/*Rooms*/
	station[1].description = "lss-daedalus";
	station[1].info = "My trusty old scrap ship. Built it myself. Still leaks fuel like a drunken horse, but it's mine, damn it! Well, at least until some of fines I have to pay results in repo men knocking my door.";
	station[1].connections[0] = 2;
	station[1].itemsPresent = (struct ObjectNode*)calloc(1, sizeof(struct ObjectNode));
	station[1].sizeX = 30;
	station[1].sizeY = 10;
	playerPosition.x = station[playerLocation].sizeX / 2;
	playerPosition.y = station[playerLocation].sizeY / 2;

	station[2].description = "hangar";
	station[2].info = "The station main hangar is rather unremarkable. The only thing you notice is a slight yellow tint of the air, as if a mist slides next to the floor. It's very faint. Your ship's computer tells you this is harmless (as if those readings were worth the trust). Unfortunately, no useful tools around here. Around the corner, near the escape pod entrance, there is deactivated ship reactor.";
	station[2].connections[2] = 1;
	station[2].connections[1] = 6;
	station[2].connections[0] = 3;
	station[2].itemsPresent = (struct ObjectNode*)calloc(1, sizeof(struct ObjectNode));
	station[2].sizeX = 30;
	station[2].sizeY = 20;

	station[3].description = "hall-1";
	station[3].info = "A well lit hall, with doors. It's the main hub of the station. Despite being right next to the hangar and the control room, it's rather quiet.";
	station[3].connections[2] = 2;
	station[3].connections[0] = 4;
	station[3].connections[1] = 5;
	station[3].rankRequired = 1;
	station[3].itemsPresent = (struct ObjectNode*)calloc(1, sizeof(struct ObjectNode));
	station[3].sizeX = 10;
	station[3].sizeY = 20;

	station[4].description = "elevator-level-1";
	station[4].info = "Going down? The elevator no longer works. It seems to be stuck in level 3. You have to navegate the shaft by yourself";
	station[4].connections[2] = 3;
	station[4].connections[5] = 13;
	station[4].itemsPresent = (struct ObjectNode*)calloc(1, sizeof(struct ObjectNode));
	station[4].sizeX = 5;
	station[4].sizeY = 5;

	station[5].description = "dorms-1";
	station[5].info = "Part of the dorms hallway. There are some (busted) control panels for ejecting the pods. Some pieces of cloth and broken plastic on the floor, but nothing really useful or valuable.";
	station[5].connections[3] = 3;
	station[5].connections[1] = 9;
	station[5].connections[0] = 7;
	station[5].connections[2] = 8;
	station[5].itemsPresent = (struct ObjectNode*)calloc(1, sizeof(struct ObjectNode));
	station[5].sizeX = 20;
	station[5].sizeY = 10;

	station[6].description = "rls-bohr-2";
	station[6].info = "A rescue lander ship. Only for emergencies. Named after some Niels Bohr scientist guy or whatever. Some drops on the carpet and I don't even want know what it is, but I guess I already know. Ick.";
	station[6].connections[3] = 2;
	station[6].itemsPresent = (struct ObjectNode*)calloc(1, sizeof(struct ObjectNode));
	station[6].sizeX = 5;
	station[6].sizeY = 5;

	station[7].description = "pod-1";
	station[7].info = "A male living pod. Looks like from one of the scientists. It's messy, but as if it's occupant would easily find his belongings in there. There are a few cracks in the glass already.";
	station[7].connections[2] = 5;
	station[7].itemsPresent = (struct ObjectNode*)calloc(1, sizeof(struct ObjectNode));
	station[7].sizeX = 7;
	station[7].sizeY = 7;

	station[8].description = "pod-2";
	station[8].info = "A empty living pod. Looks as if it was never ever used. If can even see some of the factory stickers in it.";
	station[8].connections[0] = 5;
	station[8].itemsPresent = (struct ObjectNode*)calloc(1, sizeof(struct ObjectNode));
	station[8].sizeX = 7;
	station[8].sizeY = 7;

	station[9].description = "dorms-2";
	station[9].info = "Anonther part of the dorms hallway. On those, the panels were visibly well. There is a skylight. These parts of the quarters were probably the luxury ones.";
	station[9].connections[1] = 12;
	station[9].connections[3] = 5;
	station[9].connections[0] = 10;
	station[9].connections[2] = 11;
	station[9].itemsPresent = (struct ObjectNode*)calloc(1, sizeof(struct ObjectNode));
	station[9].sizeX = 20;
	station[9].sizeY = 10;

	station[10].description = "pod-3";
	station[10].info = "A young woman's pod. You do recognize a few items, but its badly mixed up. It's hard to make the age of girl, but she was young.";
	station[10].connections[2] = 9;
	station[10].itemsPresent = (struct ObjectNode*)calloc(1, sizeof(struct ObjectNode));
	station[10].sizeX = 7;
	station[10].sizeY = 7;

	station[11].description = "pod-4";
	station[11].info = "A scientists pod, for sure. It's neat, clean and organized. Not much around. He had a strange fixation on redheads.";
	station[11].connections[0] = 9;
	station[11].itemsPresent = (struct ObjectNode*)calloc(1, sizeof(struct ObjectNode));
	station[11].sizeX = 7;
	station[11].sizeY = 7;

	station[12].description = "lounge";
	station[12].info = "Here, it seems like a relaxation place. You gaze at the stars and the planet. Very nice.";
	station[12].connections[3] = 9;
	station[12].itemsPresent = (struct ObjectNode*)calloc(1, sizeof(struct ObjectNode));
	station[12].sizeX = 10;
	station[12].sizeY = 16;

	station[13].description = "elevator-level-2";
	station[13].info = "Going up or down? Looking down, you can clearly see the elevator cabin in level 3.";
	station[13].connections[5] = 4;
	station[13].connections[4] = 19;
	station[13].connections[2] = 14;
	station[13].itemsPresent = (struct ObjectNode*)calloc(1, sizeof(struct ObjectNode));
	station[13].sizeX = 10;
	station[13].sizeY = 16;

	station[14].description = "hall-2";
	station[14].info = "Not as imponent as the main hall from Level 1, this hall has a busier feel. Here you see objects thrown all over the place, as if someone was in the middle of a day-to-day routine and had to quickly run.";
	station[14].connections[0] = 13;
	station[14].connections[1] = 17;
	station[14].connections[2] = 16;
	station[14].connections[3] = 15;
	station[14].itemsPresent = (struct ObjectNode*)calloc(1, sizeof(struct ObjectNode));
	station[14].sizeX = 10;
	station[14].sizeY = 16;

	station[15].description = "dinner-room";
	station[15].info = "Empty stomach makes no science. Those thinkers were really into fancy stuff. Too bad it all went bad a long time ago.";
	station[15].connections[1] = 14;
	station[15].itemsPresent = (struct ObjectNode*)calloc(1, sizeof(struct ObjectNode));
	station[15].sizeX = 10;
	station[15].sizeY = 16;

	station[16].description = "control-room";
	station[16].info = "Lots of old equiptment.";
	station[16].connections[0] = 14;
	station[16].itemsPresent = (struct ObjectNode*)calloc(1, sizeof(struct ObjectNode));
	station[16].sizeX = 10;
	station[16].sizeY = 16;

	station[17].description = "gymnasium";
	station[17].info = "This is where they used to workout to keep their health";
	station[17].connections[3] = 14;
	station[17].connections[1] = 18;
	station[17].itemsPresent = (struct ObjectNode*)calloc(1, sizeof(struct ObjectNode));
	station[17].sizeX = 10;
	station[17].sizeY = 16;

	station[18].description = "restroom";
	station[18].info = "...and this is where they would stay clean. Smells like dry sweat. Ick.";
	station[18].connections[3] = 17;
	station[18].itemsPresent = (struct ObjectNode*)calloc(1, sizeof(struct ObjectNode));
	station[18].sizeX = 10;
	station[18].sizeY = 16;

	station[19].description = "elevator-level-3";
	station[19].info = "Going up? Fortunately, the escape hatch is open and this allows for access. The cabin itself is unremarkable.";
	station[19].connections[2] = 20;
	station[19].connections[4] = 13;
	station[19].itemsPresent = (struct ObjectNode*)calloc(1, sizeof(struct ObjectNode));
	station[19].sizeX = 5;
	station[19].sizeY = 5;

	station[20].description = "hall-3";
	station[20].info = "This was a restricted area, so it's rather sparce. Mostly labs and equipment. A constant hum from the generaters can be heard";
	station[20].connections[0] = 19;
	station[20].connections[1] = 21;
	station[20].connections[2] = 22;
	station[20].connections[3] = 23;
	station[20].itemsPresent = (struct ObjectNode*)calloc(1, sizeof(struct ObjectNode));
	station[20].sizeX = 5;
	station[20].sizeY = 5;

	station[21].description = "lab-1";
	station[21].connections[3] = 20;
	station[21].info = "A micro-g-hydrostatic lab. Lots of old equipments. There must be something valuable here.";
	station[21].itemsPresent = (struct ObjectNode*)calloc(1, sizeof(struct ObjectNode));
	station[21].sizeX = 5;
	station[21].sizeY = 5;

	station[22].description = "lab-2";
	station[22].connections[0] = 20;
	station[22].info = "A low-atmosphere-electricity lab. Lots of strange equipment. Looks dangerous.";
	station[22].itemsPresent = (struct ObjectNode*)calloc(1, sizeof(struct ObjectNode));
	station[22].sizeX = 5;
	station[22].sizeY = 5;

	station[23].description = "lab-3";
	station[23].connections[1] = 20;
	station[23].info = "Looks like this was a chemistry lab. Looks badly destroyed. I was told this was due to space-trash. That's why they got us! On the left wall, there are remnants of a 3D periodic table. If only this was in once piece, it could make some good cash.";
	station[23].itemsPresent = (struct ObjectNode*)calloc(1, sizeof(struct ObjectNode));
	station[23].sizeX = 5;
	station[23].sizeY = 5;

	playerLocation = 1;
	/*Items*/    
    
	/* LSS-Daedalus */

	item[0].description = "time-bomb";
	item[0].pickable = TRUE;
	item[0].info = "time-programmable Halogen bomb.";
	item[0].useWithCallback = cantBeUsedWithOthersCallback;
	item[0].weight = 5;
	item[0].position.x = 5;
	item[0].position.y = 4;
	item[0].useCallback = bombActivatedCallback;
	addToRoom("lss-daedalus", &item[0]);

	item[1].description = "time-bomb-controller";
	item[1].info = "The remote controller allows you to instantly detonate the bomb from very far (empirical evidence tells it works from as far as 0.5AU).";
	item[1].weight = 0;
	item[1].useWithCallback = cantBeUsedWithOthersCallback;
	item[1].pickable = TRUE;
	item[1].position.x = 3;
	item[1].position.y = 2;
	item[1].useCallback = bombControllerActivatedCallback;
	addToRoom("lss-daedalus", &item[1]);

	item[2].description = "blowtorch";
	item[2].info = "precision vintage-but-rather-well-kept metal cutter";
	item[2].weight = 8;
	item[2].useWithCallback = useBlowtorchWithCallback;
	item[2].useCallback = cantBeUsedCallback;
	item[2].pickable = TRUE;
	item[2].position.x = 10;
	item[2].position.y = 9;
	addToRoom("lss-daedalus", &item[2]);

	item[3].description = "ship-ignition";
	item[3].weight = 0;
	item[3].useWithCallback = cantBeUsedWithOthersCallback;
	item[3].info = "token needed to ignite the ship's computer and thrusters";
	item[3].pickable = TRUE;
	item[3].position.x = 7;
	item[3].position.y = 6;
	addToRoom("lss-daedalus", &item[3]);

	item[4].description = "plasma-gun";
	item[4].info = "A mostly harmless gun. Useful for heating surfaces and light defense.";
	item[4].weight = 1;
	item[4].pickable = TRUE;
	item[4].position.x = 8;
	item[4].position.y = 6;
	addToRoom("lss-daedalus", &item[4]);

	item[5].description = "magnetic-boots";
	item[5].weight = 2;
	item[5].info = "boots with strong electro-magnets. Ideal for walking on low-gravity situations - as long as the surface in question is metallic (like most of the surfaces here).";
	item[5].useWithCallback = useBootsWithCallback;
	item[5].useCallback = useObjectToggleCallback;
	item[5].pickable = TRUE;
	item[5].position.x = 9;
	item[5].position.y = 6;
	addToRoom("lss-daedalus", &item[5]);

	item[6].description = "helmet";
	item[6].weight = 2;
	item[6].info = "makes sure the air entering your gear is the best. Or how that was, 50 years ago, when this was manufactured.";
	item[6].info = "Atmosphere-contained helmet for safety.";
	item[6].pickable = TRUE;
	item[6].useCallback = useObjectToggleCallback;
	item[6].position.x = 10;
	item[6].position.y = 6;
	addToRoom("lss-daedalus", &item[6]);

	item[7].description = "low-rank-keycard";
	item[7].info = "clearance for low rank";
	item[7].weight = 0;
	item[7].pickable = TRUE;
	item[7].position.x = 11;
	item[7].useCallback = cantBeUsedCallback;
	item[7].useWithCallback = cantBeUsedWithOthersCallback;
	item[7].position.y = 6;
	item[7].pickCallback = keycardPickCallback;
	item[7].dropCallback = keycardDropCallback;
	addToRoom("lss-daedalus", &item[7]);

	/* Hangar */

	item[8].description = "gold-pipe";
	item[8].info = "This seems valuable. One can wonder why it is used here and what would be the consequences of taking it out? Whaever. It looks like paying for dinner.";
	item[8].weight = 17;
	item[8].useCallback = cantBeUsedCallback;
	item[8].useWithCallback = cantBeUsedWithOthersCallback;
	item[8].pickable = FALSE;
	item[8].position.x = 17;
	item[8].position.y = 16;
	addToRoom("hangar", &item[8]);

	/* Comm terminals*/
	item[9].description = "comm-terminal";
	item[9].info = "Offline comm terminal for communicating with the other levels";
	item[9].useCallback = cantBeUsedCallback;
	item[9].useWithCallback = cantBeUsedWithOthersCallback;
	item[9].weight = 200;
	item[9].pickable = FALSE;
	item[9].position.x = 17;
	item[9].position.y = 16;
	addToRoom("hall-1", &item[9]);

	item[10].description = "comm-terminal";
	item[10].info = "Offline comm terminal for communicating with the other levels";
	item[10].useCallback = cantBeUsedCallback;
	item[10].useWithCallback = cantBeUsedWithOthersCallback;
	item[10].weight = 200;
	item[10].pickable = FALSE;
	item[10].position.x = 17;
	item[10].position.y = 16;
	addToRoom("hall-2", &item[10]);

	item[11].description = "comm-terminal";
	item[11].info = "Offline comm terminal for communicating with the other levels";
	item[11].useCallback = cantBeUsedCallback;
	item[11].useWithCallback = cantBeUsedWithOthersCallback;
	item[11].weight = 200;
	item[11].pickable = FALSE;
	item[11].position.x = 17;
	item[11].position.y = 16;
	addToRoom("hall-3", &item[11]);

	/* Diaries */
	item[12].description = "white-diary";
	item[12].info = "...And so, I realized that when you apply a presure in MicroG...";
	item[12].useCallback = cantBeUsedCallback;
	item[12].useWithCallback = cantBeUsedWithOthersCallback;
	item[12].weight = 0;
	item[12].pickable = TRUE;
	item[12].position.x = 5;
	item[12].position.y = 3;
	addToRoom("pod-1", &item[12]);

	item[13].description = "blue-diary";
	item[13].weight = 0;
	item[13].info = "...Look in the mirror e try laughing. It's going to be hard first, but once you get used to it, it is easy to leave the room with a smile in your face.";
	item[13].useCallback = cantBeUsedCallback;
	item[13].useWithCallback = cantBeUsedWithOthersCallback;
	item[13].pickable = TRUE;
	item[13].position.x = 8;
	item[13].position.y = 4;
	addToRoom("pod-3", &item[13]);

	item[14].description = "black-diary";
	item[14].weight = 0;
	item[14].info = "...We meet every night in the empty lab, to make out. I asked her for a access key and hid it in the tubes - probably a good place to hide it. Those are inactive for years! If they catch me, I'm scr...";
	item[14].useCallback = cantBeUsedCallback;
	item[14].useWithCallback = cantBeUsedWithOthersCallback;
	item[14].pickable = TRUE;
	item[14].position.x = 2;
	item[14].position.y = 2;
	addToRoom("pod-4", &item[14]);

	item[15].description = "yellow-book";
	item[15].info = "Today, I lost card during dinner. Gonna ask for a new one. Specially disconcerting since it gives access to all bunks. Nobody knows, so nothing to worry, I guess...";
	item[15].useCallback = cantBeUsedCallback;
	item[15].useWithCallback = cantBeUsedWithOthersCallback;
	item[15].weight = 0;
	item[15].pickable = TRUE;
	item[15].position.x = 4;
	item[15].position.y = 3;
	addToRoom("dinner-room", &item[15]);

	item[16].description = "log-book";
	item[16].info = "A side note is written: That metal mending we put on Lab 1. last week mending was a real rush job. Any stronger bump and it might ...";
	item[16].useCallback = cantBeUsedCallback;
	item[16].useWithCallback = cantBeUsedWithOthersCallback;
	item[16].weight = 1;
	item[16].pickable = TRUE;
	item[16].position.x = 10;
	item[16].position.y = 10;
	addToRoom("lab-2", &item[16]);

	/* Misc */
	item[17].description = "plastic-pipe";
	item[17].info = "Just a regular pipe, taking something somewhere.";
	item[17].useCallback = cantBeUsedCallback;
	item[17].useWithCallback = cantBeUsedWithOthersCallback;
	item[17].weight = 3;
	item[17].pickable = FALSE;
	item[17].position.x = 17;
	item[17].position.y = 16;
	addToRoom("restroom", &item[17]);

	item[18].description = "high-rank-keycard";
	item[18].info = "Clearance for high-rank officer";
	item[18].useCallback = cantBeUsedCallback;
	item[18].useWithCallback = cantBeUsedWithOthersCallback;
	item[18].weight = 0;
	item[18].pickable = TRUE;
	item[18].position.x = 7;
	item[18].pickCallback = keycardPickCallback;
	item[18].dropCallback = keycardDropCallback;
	item[18].position.y = 6;
	addToRoom("dinner-room", &item[18]);

	item[19].description = "computer-rack";
	item[19].useCallback = cantBeUsedCallback;
	item[19].useWithCallback = cantBeUsedWithOthersCallback;
	item[19].weight = 138;
	item[19].info = "A very valuable vintage rare-and-in-working-conditions computer rack!";
	item[19].pickable = FALSE;
	item[19].position.x = 1;
	item[19].position.y = 1;
	addToRoom("control-room", &item[19]);

	item[20].description = "journal";
	item[20].useCallback = cantBeUsedCallback;
	item[20].useWithCallback = cantBeUsedWithOthersCallback;
	item[20].info = "...and so you guys could just join in and see whats going on. I hope it is not too instrusive of me. To that, she just gave me a cold stare and...";
	item[20].weight = 0;
	item[20].pickable = TRUE;
	item[20].position.x = 17;
	item[20].position.y = 6;
	addToRoom("control-room", &item[20]);

	/* Has to be removed for the bomb to be properly planted */
	item[21].description = "metal-mending";
	item[21].useCallback = cantBeUsedCallback;
	item[21].useWithCallback = cantBeUsedWithOthersCallback;
	item[21].info = "A piece of metal that might be valuable.";
	item[21].weight = 74;
	item[21].pickable = FALSE;
	item[21].position.x = 7;
	item[21].position.y = 6;
	addToRoom("lab-1", &item[21]);

	item[22].description = "scientific-treatise";
	item[22].useCallback = cantBeUsedCallback;
	item[22].useWithCallback = cantBeUsedWithOthersCallback;
	item[22].info = "Voynich Manuscript - Annottated Translation. Classical edition. It's badly burn't. Can't read it.";
	item[22].weight = 1;
	item[22].pickable = TRUE;
	item[22].position.x = 1;
	item[22].position.y = 1;
	addToRoom("lab-2", &item[22]);

	item[23].description = "electric-experiment";
	item[23].info = "All these equipment looks the same. Doesn't look valuable for me.";
	item[23].useCallback = cantBeUsedCallback;
	item[23].useWithCallback = cantBeUsedWithOthersCallback;
	item[23].weight = 209;
	item[23].pickable = FALSE;
	item[23].position.x = 1;
	item[23].position.y = 1;
	addToRoom("lab-2", &item[23]);

	item[24].description = "chemical-experiment";
	item[24].info = "All these equipment looks the same. Doesn't look valuable for me.";
	item[24].useCallback = cantBeUsedCallback;
	item[24].useWithCallback = cantBeUsedWithOthersCallback;
	item[24].weight = 62;
	item[24].pickable = TRUE;
	item[24].position.x = 1;
	item[24].position.y = 1;
	addToRoom("lab-3", &item[24]);

	/* Not added directly, will be placed on the restroom after you search the pipe */
	item[25].description = "root-keycard";
	item[25].info = "Card for root access";
	item[25].useCallback = cantBeUsedCallback;
	item[25].useWithCallback = cantBeUsedWithOthersCallback;
	item[25].weight = 0;
	item[25].pickCallback = keycardPickCallback;
	item[25].dropCallback = keycardDropCallback;
	item[25].pickable = TRUE;
	item[25].position.x = 7;
	item[25].position.y = 6;
}
