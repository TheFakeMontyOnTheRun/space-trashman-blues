//
// Created by Daniel Monteiro on 2019-07-26.
//
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

extern "C" {
#include "Derelict.h"
#include "Parser.h"
}

using testing::Eq;

class ErrorHandlerInterface {
public:
  virtual ~ErrorHandlerInterface() {}
  virtual int handleError() = 0;
};

class ErrorHandlerMock : public ErrorHandlerInterface {
public:
  virtual ~ErrorHandlerMock() {}
  MOCK_METHOD0(handleError, int());
};

class TestInventoryManipulation : public ::testing::Test {

public:
  static std::shared_ptr<ErrorHandlerMock> mockedObj;

  virtual void SetUp() {
    initStation();
    mockedObj = std::make_shared<ErrorHandlerMock>();
  }

  virtual void TearDown() {
    mockedObj = nullptr;
  }
};

std::shared_ptr<ErrorHandlerMock> TestInventoryManipulation::mockedObj;

extern struct ObjectNode *collectedObject;

static void myErrorHandler(const char* errorMsg) {
  TestInventoryManipulation::mockedObj->handleError();
}

TEST_F(TestInventoryManipulation, checkingForInvalidObjectsInRoomWillCauseError) {

	setErrorHandlerCallback(myErrorHandler);

	EXPECT_CALL(*mockedObj, handleError());
	ASSERT_FALSE(hasItemInRoom("hangar", ""));

	EXPECT_CALL(*mockedObj, handleError());
	ASSERT_FALSE(hasItemInRoom("hangar", NULL));

	//this is not supposed to cause an error, but simply return false. Objects can have any name.
	EXPECT_CALL( *mockedObj, handleError()).Times(0);
	ASSERT_FALSE(hasItemInRoom("hangar", "farofinha_fofa"));
}

TEST_F(TestInventoryManipulation, checkingInvalidRoomForObjectsWillCauseError) {

  	setErrorHandlerCallback(myErrorHandler);

	EXPECT_CALL(*mockedObj, handleError());
	ASSERT_FALSE(hasItemInRoom("", "key"));

	EXPECT_CALL(*mockedObj, handleError());
	ASSERT_FALSE(hasItemInRoom(NULL, "key"));


	//differently from objects, we know very well our rooms. This will cause an error
	EXPECT_CALL(*mockedObj, handleError());
	ASSERT_FALSE(hasItemInRoom("teu_cu", "key"));
}

TEST_F(TestInventoryManipulation, objectsDroppedInRoomStayThere) {
	ASSERT_TRUE(collectedObject->next == nullptr);

	ASSERT_TRUE(isPlayerAtRoom("uss-daedalus"));
	ASSERT_FALSE(hasItemInRoom("uss-daedalus", "key"));
	parseCommand("move", "0");
	ASSERT_TRUE(isPlayerAtRoom("hangar"));
	ASSERT_TRUE(hasItemInRoom("hangar", "key"));
	parseCommand("pick", "key");
	ASSERT_FALSE(hasItemInRoom("hangar", "key"));

	parseCommand("move", "2");
	ASSERT_TRUE(isPlayerAtRoom("uss-daedalus"));
	ASSERT_FALSE(hasItemInRoom("uss-daedalus", "key"));
	parseCommand("drop", "key");
	ASSERT_TRUE(hasItemInRoom("uss-daedalus", "key"));
	ASSERT_TRUE(collectedObject->next == NULL);

	ASSERT_FALSE(hasItemInRoom("hangar", "key"));
	ASSERT_TRUE(hasItemInRoom("uss-daedalus", "key"));
}

TEST_F(TestInventoryManipulation, canPickObjects) {
	ASSERT_TRUE(collectedObject->next == nullptr);
	ASSERT_TRUE(isPlayerAtRoom("uss-daedalus"));
	parseCommand("move", "0");
	ASSERT_TRUE(isPlayerAtRoom("hangar"));
	struct Item *item = getRoom(getPlayerRoom())->itemsPresent->item;
	ASSERT_TRUE(hasItemInRoom("hangar", "key"));
	parseCommand("pick", "key");
	ASSERT_FALSE(hasItemInRoom("hangar", "key"));
	ASSERT_TRUE(collectedObject->item == item);
	parseCommand("drop", "key");
	ASSERT_TRUE(collectedObject->next == NULL);
	ASSERT_TRUE(hasItemInRoom("hangar", "key"));
}

TEST_F(TestInventoryManipulation, objectsCanOnlyExistInOneRoom) {

  struct Item item;
  memset(&item, 0, sizeof(struct Item));

  item.description = "farofinha";

  initStation();

  addObjectToRoom(1, &item);
  ASSERT_TRUE(hasItemInRoom("uss-daedalus", "farofinha"));
  ASSERT_FALSE(hasItemInRoom("hangar", "farofinha"));
  
  addObjectToRoom(2, &item);
  ASSERT_TRUE(hasItemInRoom("hangar", "farofinha"));
  ASSERT_FALSE(hasItemInRoom("uss-daedalus", "farofinha"));
  
  parseCommand("move", "0");
  parseCommand("pick", "farofinha");
  ASSERT_FALSE(hasItemInRoom("uss-daedalus", "farofinha"));
  ASSERT_FALSE(hasItemInRoom("hangar", "farofinha"));

  parseCommand("move", "2");
  parseCommand("drop", "farofinha");
  ASSERT_TRUE(hasItemInRoom("uss-daedalus", "farofinha"));
  ASSERT_FALSE(hasItemInRoom("hangar", "farofinha"));
}
