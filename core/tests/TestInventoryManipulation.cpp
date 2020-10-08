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

TEST(TestInventoryManipulation, canPickObjects) {
	initStation();
	ASSERT_TRUE(collectedObject == nullptr);
	ASSERT_TRUE(isPlayerAtRoom("uss-daedalus"));
	parseCommand("move", "0");
	ASSERT_TRUE(isPlayerAtRoom("hangar"));
	struct Item *item = getRoom(getPlayerRoom())->itemsPresent->item;
	ASSERT_TRUE(hasItemInRoom("hangar", "key"));
	parseCommand("pick", "key");
	ASSERT_FALSE(hasItemInRoom("hangar", "key"));
	ASSERT_TRUE(collectedObject->item == item);
	parseCommand("drop", "key");
	ASSERT_TRUE(collectedObject == NULL);
	ASSERT_TRUE(hasItemInRoom("hangar", "key"));
}