//
// Created by Daniel Monteiro on 2019-07-26.
//

#include <gtest/gtest.h>

extern "C" {
#include "Core.h"
#include "Derelict.h"
#include "Parser.h"
}

extern struct ObjectNode *collectedObject;

using testing::Eq;

void usableCallback(struct Item* item) {
  item->description = "used";
}

void useWithCallback(struct Item* item1, struct Item* item2 ) {
  if (!strcmp(item2->description, "farofinha")) {
    item1->description = "used-twofold";
  }
}

TEST(TestObjectManipulation, canUseObjectsTogether) {

  struct Item item[4];
  memset(&item[0], 0, 4 * sizeof(struct Item));

  item[0].description = "usableWith";
  item[0].useWithCallback = useWithCallback;
  item[0].pickable = TRUE;
  item[1].description = "farofinha";
  item[1].pickable = TRUE;
  item[2].description = "pamonha";
  item[2].pickable = TRUE;
  item[3].description = "cocada";
  item[3].pickable = TRUE;

  initStation();

  addObjectToRoom(1, &item[0]);
  addObjectToRoom(1, &item[1]);
  addObjectToRoom(1, &item[2]);
  addObjectToRoom(1, &item[3]);

  ASSERT_TRUE(hasItemInRoom("lss-daedalus", "usableWith"));
  ASSERT_TRUE(hasItemInRoom("lss-daedalus", "farofinha"));
  ASSERT_TRUE(isPlayerAtRoom("lss-daedalus"));
  parseCommand("pick", "usableWith");
  parseCommand("pick", "cocada");
  ASSERT_FALSE(hasItemInRoom("hangar", "usableWith"));

  //needed to setup the tokenizer
  char buffer[255];
  strcpy(&buffer[0], "use-with usableWith farofinha");
  char *operator1 = strtok( &buffer[0], "\n " );
  char *operand1 = strtok( NULL, "\n ");

  parseCommand(operator1, operand1);

  parseCommand("drop", "used-twofold");

  ASSERT_TRUE(hasItemInRoom("lss-daedalus", "used-twofold"));
}

TEST(TestObjectManipulation, canUseObjects) {

  struct Item item[2];
  memset(&item, 0, 2 * sizeof(struct Item));

  item[0].description = "usable";
  item[0].pickable = TRUE;
  item[0].useCallback = usableCallback;

  item[1].description = "artificial";
  item[1].pickable = TRUE;

  initStation();
  addObjectToRoom(2, &item[0]);  
  addObjectToRoom(1, &item[0]);
  addObjectToRoom(1, &item[1]);

  ASSERT_TRUE(hasItemInRoom("lss-daedalus", "usable"));
  ASSERT_TRUE(isPlayerAtRoom("lss-daedalus"));
  parseCommand("pick", "usable");
  parseCommand("pick", "artificial");
  ASSERT_FALSE(hasItemInRoom("hangar", "usable"));
  parseCommand("use", "usable");
  parseCommand("drop", "used");

  ASSERT_TRUE(hasItemInRoom("lss-daedalus", "used"));
}

TEST(TestObjectManipulation, cantPickUnpickableObjects) {

  struct Item item;
  memset(&item, 0, sizeof(struct Item));

  item.description = "unpickable";
  item.pickable = FALSE;
  item.useCallback = usableCallback;

  initStation();
  addObjectToRoom(1, &item);


  ASSERT_TRUE(hasItemInRoom("lss-daedalus", "unpickable"));
  ASSERT_TRUE(isPlayerAtRoom("lss-daedalus"));
  parseCommand("pick", "unpickable");
  ASSERT_TRUE(hasItemInRoom("lss-daedalus", "unpickable"));
  item.pickable = TRUE; 
  parseCommand("pick", "unpickable");
  ASSERT_FALSE(hasItemInRoom("lss-daedalus", "unpickable"));
}
