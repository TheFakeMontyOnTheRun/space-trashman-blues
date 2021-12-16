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
  item->name = "used";
}

void useWithCallback(struct Item* item1, struct Item* item2 ) {
  if (!strcmp(item2->name, "farofinha")) {
    item1->name = "used-twofold";
  }
}

TEST(TestObjectManipulation, canUseObjectsTogether) {

  struct Item *item;

  initStation();


  item = addItem("usableWith", "",
#ifdef ITEMS_HAVE_WEIGHT
          0,
#endif
                 TRUE, 15, 19);
  item->useCallback = usableCallback;
  addToRoom("lss-daedalus", item);


  item = addItem("farofinha", "",
#ifdef ITEMS_HAVE_WEIGHT
          0,
#endif
                 TRUE, 15, 19);
  addToRoom("lss-daedalus", item);


  item = addItem("pamonha", "",
#ifdef ITEMS_HAVE_WEIGHT
          0,
#endif
                 TRUE, 15, 19);
  addToRoom("lss-daedalus", item);

  item = addItem("cocada", "",
#ifdef ITEMS_HAVE_WEIGHT
          0,
#endif
                 TRUE, 15, 19);
  addToRoom("lss-daedalus", item);

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
}

TEST(TestObjectManipulation, canUseObjects) {

    struct Item *item;

    initStation();


    item = addItem("usable", "",
#ifdef ITEMS_HAVE_WEIGHT
            0,
#endif
                   TRUE, 15, 19);
    item->useCallback = usableCallback;

    addToRoom("lss-daedalus", item);


    item = addItem("artificial", "",
#ifdef ITEMS_HAVE_WEIGHT
            0,
#endif
                   TRUE, 15, 19);
    addToRoom("lss-daedalus", item);

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

  struct Item *item;

  initStation();


  item = addItem("unpickable", "",
#ifdef ITEMS_HAVE_WEIGHT
          0,
#endif
                 FALSE, 15, 19);
  item->useCallback = usableCallback;

  addToRoom("lss-daedalus", item);


  ASSERT_TRUE(hasItemInRoom("lss-daedalus", "unpickable"));
  ASSERT_TRUE(isPlayerAtRoom("lss-daedalus"));
  parseCommand("pick", "unpickable");
  ASSERT_TRUE(hasItemInRoom("lss-daedalus", "unpickable"));
  item->pickable = TRUE;
  parseCommand("pick", "unpickable");
  ASSERT_FALSE(hasItemInRoom("lss-daedalus", "unpickable"));
}
