//
// Created by Daniel Monteiro on 2019-07-26.
//

#include <gmock/gmock.h>
#include <gtest/gtest.h>

extern "C" {
#include "Derelict.h"
#include "Parser.h"
}

extern struct Room *room;
extern struct ObjectNode *collectedObject;

using testing::Eq;

TEST(TestInventoryManipulation, canPickObjects) {
  initStation();
  ASSERT_TRUE( collectedObject == nullptr );
  ASSERT_EQ( getPlayerRoom(), 1 );
  parseCommand( "move", "0" );
  struct Item *item = room->itemsPresent->item;
  parseCommand( "pick", "key" );
  ASSERT_TRUE( collectedObject->item == item );
  parseCommand( "drop", "key" );
  ASSERT_TRUE( collectedObject == NULL );
}