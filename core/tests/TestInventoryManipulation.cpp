//
// Created by Daniel Monteiro on 2019-07-26.
//
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#ifdef __APPLE__
#if __clang_major__ > 4

#include <memory>

using std::shared_ptr;
#else
#include <tr1/memory>
using std::tr1::shared_ptr;
#endif
#else
#include <tr1/memory>
using std::tr1::shared_ptr;
#endif

extern "C" {
#include "Enums.h"
#include "Core.h"
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
    static shared_ptr<ErrorHandlerMock> mockedObj;

    virtual void SetUp() {
        initStation();
        mockedObj = shared_ptr<ErrorHandlerMock>(new ErrorHandlerMock);
    }

    virtual void TearDown() {
    }
};

shared_ptr<ErrorHandlerMock> TestInventoryManipulation::mockedObj;

static void myErrorHandler(const char *errorMsg) {
    TestInventoryManipulation::mockedObj->handleError();
}

TEST_F(TestInventoryManipulation, checkingForInvalidObjectsInRoomWillCauseError) {

    setErrorHandlerCallback(myErrorHandler);

    EXPECT_CALL(*mockedObj, handleError());
    ASSERT_FALSE(hasItemInRoom("hangar", ""));

    EXPECT_CALL(*mockedObj, handleError());
    ASSERT_FALSE(hasItemInRoom("hangar", NULL));

    // this is not supposed to cause an error, but simply return false.
    // Objects can have any name.
    EXPECT_CALL(*mockedObj, handleError()).Times(0);
    ASSERT_FALSE(hasItemInRoom("hangar", "farofinha_fofa"));
}


TEST_F(TestInventoryManipulation, droppingAnInvalidObjectWillResultInAnError) {

    setErrorHandlerCallback(myErrorHandler);

    EXPECT_CALL(*mockedObj, handleError());
    parseCommand("drop", "farofinha");
}

TEST_F(TestInventoryManipulation, checkingInvalidRoomForObjectsWillCauseError) {

    setErrorHandlerCallback(myErrorHandler);

    EXPECT_CALL(*mockedObj, handleError());
    ASSERT_FALSE(hasItemInRoom("", "low-rank-keycard"));

    EXPECT_CALL(*mockedObj, handleError());
    ASSERT_FALSE(hasItemInRoom(NULL, "low-rank-keycard"));


    // differently from objects, we know very well our rooms.
    // This will cause an error
    EXPECT_CALL(*mockedObj, handleError());
    ASSERT_FALSE(hasItemInRoom("teu_cu", "low-rank-keycard"));
}

TEST_F(TestInventoryManipulation, objectsDroppedInRoomStayThere) {
    addToRoom("lss-daedalus", getItemNamed("low-rank-keycard"));
    ASSERT_TRUE(isPlayerAtRoom("lss-daedalus"));
    ASSERT_TRUE(hasItemInRoom("lss-daedalus", "low-rank-keycard"));
    parseCommand("pick", "low-rank-keycard");
    ASSERT_FALSE(hasItemInRoom("lss-daedalus", "low-rank-keycard"));

    parseCommand("move", "0");
    ASSERT_TRUE(isPlayerAtRoom("hangar"));
    ASSERT_FALSE(hasItemInRoom("lss-daedalus", "low-rank-keycard"));
    parseCommand("drop", "low-rank-keycard");
    ASSERT_TRUE(hasItemInRoom("hangar", "low-rank-keycard"));
    ASSERT_FALSE(hasItemInRoom("lss-daedalus", "low-rank-keycard"));
}

TEST_F(TestInventoryManipulation, itemsWillNotBeIncludedInListTwice) {
    struct ObjectNode *list = getPlayerItems();
    struct Item *item1 = getItemNamed("root-keycard");
    struct Item *item2 = getItemNamed("emp-controller");
    addObjectToList(item1, list);
    addObjectToList(item2, list);
    addObjectToList(item1, list);
    ASSERT_FALSE(objectNodes[item1->index].next == NULL);
    ASSERT_TRUE(objectNodes[item2->index].next == NULL);
}

TEST_F(TestInventoryManipulation, canPickObjects) {
    ASSERT_TRUE(isPlayerAtRoom("lss-daedalus"));
    addToRoom("lss-daedalus", getItemNamed("low-rank-keycard"));
    struct Item *item = getItem(getRoom(getPlayerRoom())->itemsPresent->item);

    ASSERT_TRUE(hasItemInRoom("lss-daedalus", "low-rank-keycard"));
    parseCommand("pick", "low-rank-keycard");
    ASSERT_FALSE(hasItemInRoom("lss-daedalus", "low-rank-keycard"));
    ASSERT_TRUE(playerHasObject("low-rank-keycard"));

    parseCommand("drop", "low-rank-keycard");
    ASSERT_TRUE(hasItemInRoom("lss-daedalus", "low-rank-keycard"));
}

TEST_F(TestInventoryManipulation, cantAddInvalidObjectToRoom) {
    setErrorHandlerCallback(myErrorHandler);
    addToRoom("lss-daedalus", NULL);
}


TEST_F(TestInventoryManipulation, cantAddObjectsToInvalidRoom) {
    struct Item *emp = getItemNamed("emp-controller");

    setErrorHandlerCallback(myErrorHandler);

    addToRoom("", emp);
    ASSERT_TRUE(hasItemInRoom("lss-daedalus", "emp-controller"));

    addToRoom("farofinha", emp);
    ASSERT_TRUE(hasItemInRoom("lss-daedalus", "emp-controller"));

    addToRoom(NULL, emp);
    ASSERT_TRUE(hasItemInRoom("lss-daedalus", "emp-controller"));
}

TEST_F(TestInventoryManipulation, objectsCanOnlyExistInOneRoom) {
    struct Item *item;

    initStation();

    item = addItem("farofinha",
#ifdef INCLUDE_ITEM_DESCRIPTIONS
            "",
#endif
#ifdef ITEMS_HAVE_WEIGHT
            0,
#endif
                   TRUE, 15, 19);
    addToRoom("lss-daedalus", item);

    ASSERT_TRUE(hasItemInRoom("lss-daedalus", "farofinha"));
    ASSERT_FALSE(hasItemInRoom("hangar", "farofinha"));

    addObjectToRoom(2, item);
    ASSERT_TRUE(hasItemInRoom("hangar", "farofinha"));
    ASSERT_FALSE(hasItemInRoom("lss-daedalus", "farofinha"));

    parseCommand("move", "0");
    parseCommand("pick", "farofinha");
    ASSERT_FALSE(hasItemInRoom("lss-daedalus", "farofinha"));
    ASSERT_FALSE(hasItemInRoom("hangar", "farofinha"));

    parseCommand("move", "2");
    parseCommand("drop", "farofinha");
    ASSERT_TRUE(hasItemInRoom("lss-daedalus", "farofinha"));
    ASSERT_FALSE(hasItemInRoom("hangar", "farofinha"));
}
