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

void myDummyOutputHandler(const char *msg) {}

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

class TestMovement : public ::testing::Test {

public:
    static shared_ptr<ErrorHandlerMock> mockedObj;

    virtual void SetUp() {
        initStation();
        setLoggerDelegate(myDummyOutputHandler);
        mockedObj = shared_ptr<ErrorHandlerMock>(new ErrorHandlerMock());
    }

    virtual void TearDown() {
    }
};

shared_ptr<ErrorHandlerMock> TestMovement::mockedObj;


void myErrorHandler(const char *errorMsg) {
    TestMovement::mockedObj->handleError();
}

TEST_F(TestMovement, canMoveOnlyWhenEnabled) {

    ASSERT_EQ(getPlayerRoom(), 1);
    parseCommand("move", "3");
    ASSERT_EQ(getPlayerRoom(), 1);
    parseCommand("move", "1");
    ASSERT_EQ(getPlayerRoom(), 1);

}

TEST_F(TestMovement, invalidRoomNamesMustYieldTheDummyFirstRoom) {
    ASSERT_EQ(0, getRoomIdByName(NULL));
    ASSERT_EQ(0, getRoomIdByName("Farofinha"));
}

TEST_F(TestMovement, canWalkInsideRooms) {

    char buffer[255];


    strcpy(&buffer[0], "walkTo 2 3");
    char *operator1 = strtok(&buffer[0], "\n ");
    char *operand1 = strtok(NULL, "\n ");

    parseCommand(operator1, operand1);

    auto pos = getPlayerPosition();
    ASSERT_EQ(pos->x, 2);
    ASSERT_EQ(pos->y, 3);

    ASSERT_EQ(getPlayerDirection(), 0);

    parseCommand("w", NULL);
    pos = getPlayerPosition();
    ASSERT_EQ(pos->x, 2);
    ASSERT_EQ(pos->y, 2);

    parseCommand("s", NULL);
    pos = getPlayerPosition();
    ASSERT_EQ(pos->x, 2);
    ASSERT_EQ(pos->y, 3);

    parseCommand("a", NULL);
    pos = getPlayerPosition();
    ASSERT_EQ(pos->x, 1);
    ASSERT_EQ(pos->y, 3);

    parseCommand("d", NULL);
    pos = getPlayerPosition();
    ASSERT_EQ(pos->x, 2);
    ASSERT_EQ(pos->y, 3);


    parseCommand("e", NULL);
    ASSERT_EQ(getPlayerDirection(), kEast);

    setPlayerDirection(kEast);
    ASSERT_EQ(getPlayerDirection(), kEast);

    parseCommand("w", NULL);
    pos = getPlayerPosition();
    ASSERT_EQ(pos->x, 3);
    ASSERT_EQ(pos->y, 3);

    parseCommand("s", NULL);
    pos = getPlayerPosition();
    ASSERT_EQ(pos->x, 2);
    ASSERT_EQ(pos->y, 3);

    parseCommand("a", NULL);
    pos = getPlayerPosition();
    ASSERT_EQ(pos->x, 2);
    ASSERT_EQ(pos->y, 2);

    parseCommand("d", NULL);
    pos = getPlayerPosition();
    ASSERT_EQ(pos->x, 2);
    ASSERT_EQ(pos->y, 3);


    parseCommand("e", NULL);

    ASSERT_EQ(getPlayerDirection(), 2);

    parseCommand("w", NULL);
    pos = getPlayerPosition();
    ASSERT_EQ(pos->x, 2);
    ASSERT_EQ(pos->y, 4);

    parseCommand("s", NULL);
    pos = getPlayerPosition();
    ASSERT_EQ(pos->x, 2);
    ASSERT_EQ(pos->y, 3);

    parseCommand("a", NULL);
    pos = getPlayerPosition();
    ASSERT_EQ(pos->x, 3);
    ASSERT_EQ(pos->y, 3);

    parseCommand("d", NULL);
    pos = getPlayerPosition();
    ASSERT_EQ(pos->x, 2);
    ASSERT_EQ(pos->y, 3);


    parseCommand("e", NULL);

    ASSERT_EQ(getPlayerDirection(), 3);

    parseCommand("w", NULL);
    pos = getPlayerPosition();
    ASSERT_EQ(pos->x, 1);
    ASSERT_EQ(pos->y, 3);

    parseCommand("s", NULL);
    pos = getPlayerPosition();
    ASSERT_EQ(pos->x, 2);
    ASSERT_EQ(pos->y, 3);

    parseCommand("a", NULL);
    pos = getPlayerPosition();
    ASSERT_EQ(pos->x, 2);
    ASSERT_EQ(pos->y, 4);

    parseCommand("d", NULL);
    pos = getPlayerPosition();
    ASSERT_EQ(pos->x, 2);
    ASSERT_EQ(pos->y, 3);


    parseCommand("e", NULL);

    ASSERT_EQ(getPlayerDirection(), 0);

    parseCommand("q", NULL);
    ASSERT_EQ(getPlayerDirection(), 3);
    parseCommand("q", NULL);
    ASSERT_EQ(getPlayerDirection(), 2);
    parseCommand("q", NULL);
    ASSERT_EQ(getPlayerDirection(), 1);
    parseCommand("q", NULL);
    ASSERT_EQ(getPlayerDirection(), 0);
}


TEST_F(TestMovement, cannotMoveToInvalidDirections) {
    ASSERT_EQ(getPlayerRoom(), 1);

    setErrorHandlerCallback(myErrorHandler);

    EXPECT_CALL(*mockedObj, handleError());
    parseCommand("move", "");
    ASSERT_EQ(getPlayerRoom(), 1);

    EXPECT_CALL(*mockedObj, handleError());
    parseCommand("move", NULL);
    ASSERT_EQ(getPlayerRoom(), 1);

    EXPECT_CALL(*mockedObj, handleError());
    parseCommand("move", "9");
    ASSERT_EQ(getPlayerRoom(), 1);

    EXPECT_CALL(*mockedObj, handleError());
    parseCommand("move", "farofinha");
    ASSERT_EQ(getPlayerRoom(), 1);
}

TEST_F(TestMovement, roomsCanRequireSpecialRankForAccess) {

    setPlayerLocation(3);

    parseCommand("move", "0");
    ASSERT_EQ(getPlayerRoom(), 3);

    parseCommand("pick", "low-rank-keycard");

    parseCommand("move", "1");
    ASSERT_EQ(getPlayerRoom(), 5);
}

extern struct Item item[26];

TEST_F(TestMovement, walkingTowardsWallsWillBlockMovement) {


    char buffer[255];
    char *operator1;
    char *operand1;
    addToRoom("lss-daedalus", getItemNamed("low-rank-keycard"));

    ASSERT_EQ(getPlayerRoom(), 1);

    parseCommand("pick", "low-rank-keycard");

    strcpy(&buffer[0], "walkTo 0 0");
    operator1 = strtok(&buffer[0], "\n ");
    operand1 = strtok(NULL, "\n ");
    parseCommand(operator1, operand1);
    ASSERT_EQ(getPlayerRoom(), 1);

    parseCommand("a", NULL);
    ASSERT_EQ(getPlayerRoom(), 1);

    snprintf(&buffer[0], 255, "walkTo %d 0", getRoom(getPlayerRoom())->sizeX - 1);
    operator1 = strtok(&buffer[0], "\n ");
    operand1 = strtok(NULL, "\n ");
    parseCommand(operator1, operand1);
    ASSERT_EQ(getPlayerRoom(), 1);

    parseCommand("d", NULL);
    ASSERT_EQ(getPlayerRoom(), 1);


    snprintf(&buffer[0], 255, "walkTo 0 %d", getRoom(getPlayerRoom())->sizeY - 1);
    operator1 = strtok(&buffer[0], "\n ");
    operand1 = strtok(NULL, "\n ");
    parseCommand(operator1, operand1);
    ASSERT_EQ(getPlayerRoom(), 1);

    parseCommand("s", NULL);
    ASSERT_EQ(getPlayerRoom(), 1);

    parseCommand("move", "0");
    ASSERT_EQ(getPlayerRoom(), 2);
}
