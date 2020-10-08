//
// Created by Daniel Monteiro on 2019-07-26.
//

#include <gtest/gtest.h>

extern "C" {
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

  item[1].description = "farofinha";

  item[2].description = "pamonha";

  item[3].description = "cocada";

  initStation();

  addObjectToRoom(1, &item[0]);
  addObjectToRoom(1, &item[1]);
  addObjectToRoom(1, &item[2]);
  addObjectToRoom(1, &item[3]);

  ASSERT_TRUE(hasItemInRoom("uss-daedalus", "usableWith"));
  ASSERT_TRUE(hasItemInRoom("uss-daedalus", "farofinha"));
  ASSERT_TRUE(isPlayerAtRoom("uss-daedalus"));
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

  ASSERT_TRUE(hasItemInRoom("uss-daedalus", "used-twofold"));
}

TEST(TestObjectManipulation, canUseObjects) {

  struct Item item[2];
  memset(&item, 0, 2 * sizeof(struct Item));

  item[0].description = "usable";
  item[0].weight = 5;
  item[0].useCallback = usableCallback;

  item[1].description = "artificial";
  initStation();
  addObjectToRoom(2, &item[0]);  
  addObjectToRoom(1, &item[0]);
  addObjectToRoom(1, &item[1]);

  ASSERT_TRUE(hasItemInRoom("uss-daedalus", "usable"));
  ASSERT_TRUE(isPlayerAtRoom("uss-daedalus"));
  parseCommand("pick", "usable");
  parseCommand("pick", "artificial");
  ASSERT_FALSE(hasItemInRoom("hangar", "usable"));
  parseCommand("use", "usable");
  parseCommand("drop", "used");

  ASSERT_TRUE(hasItemInRoom("uss-daedalus", "used"));
}
