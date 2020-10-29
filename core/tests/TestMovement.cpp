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




class TestMovement : public ::testing::Test {

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

std::shared_ptr<ErrorHandlerMock> TestMovement::mockedObj;


void myErrorHandler(const char* errorMsg) {
  TestMovement::mockedObj->handleError();
}


TEST_F(TestMovement, canMoveOnlyWhenEnabled) {

	ASSERT_EQ(getPlayerRoom(), 1);
	parseCommand("move", "3");
	ASSERT_EQ(getPlayerRoom(), 1);
	parseCommand("move", "1");
	ASSERT_EQ(getPlayerRoom(), 1);

}

TEST_F(TestMovement, canWalkInsideRooms) {

  char buffer[255];


  parseCommand("pick", "magnetic-boots");
  parseCommand("use", "magnetic-boots");

  strcpy(&buffer[0], "walkTo 2 3");
  char *operator1 = strtok( &buffer[0], "\n " );
  char *operand1 = strtok( NULL, "\n ");

  parseCommand(operator1, operand1);

  auto pos = getPlayerPosition();
  ASSERT_EQ(pos.x, 2);
  ASSERT_EQ(pos.y, 3);

  ASSERT_EQ(getPlayerDirection(), 0);

  parseCommand("w", NULL);
  pos = getPlayerPosition();
  ASSERT_EQ(pos.x, 2);
  ASSERT_EQ(pos.y, 2);

  parseCommand("s", NULL);
  pos = getPlayerPosition();
  ASSERT_EQ(pos.x, 2);
  ASSERT_EQ(pos.y, 3);

  parseCommand("a", NULL);
  pos = getPlayerPosition();
  ASSERT_EQ(pos.x, 1);
  ASSERT_EQ(pos.y, 3);

  parseCommand("d", NULL);
  pos = getPlayerPosition();
  ASSERT_EQ(pos.x, 2);
  ASSERT_EQ(pos.y, 3);



  parseCommand("e", NULL);

  ASSERT_EQ(getPlayerDirection(), 1);

  parseCommand("w", NULL);
  pos = getPlayerPosition();
  ASSERT_EQ(pos.x, 3);
  ASSERT_EQ(pos.y, 3);

  parseCommand("s", NULL);
  pos = getPlayerPosition();
  ASSERT_EQ(pos.x, 2);
  ASSERT_EQ(pos.y, 3);

  parseCommand("a", NULL);
  pos = getPlayerPosition();
  ASSERT_EQ(pos.x, 2);
  ASSERT_EQ(pos.y, 2);

  parseCommand("d", NULL);
  pos = getPlayerPosition();
  ASSERT_EQ(pos.x, 2);
  ASSERT_EQ(pos.y, 3);



  parseCommand("e", NULL);

  ASSERT_EQ(getPlayerDirection(), 2);

  parseCommand("w", NULL);
  pos = getPlayerPosition();
  ASSERT_EQ(pos.x, 2);
  ASSERT_EQ(pos.y, 4);

  parseCommand("s", NULL);
  pos = getPlayerPosition();
  ASSERT_EQ(pos.x, 2);
  ASSERT_EQ(pos.y, 3);

  parseCommand("a", NULL);
  pos = getPlayerPosition();
  ASSERT_EQ(pos.x, 3);
  ASSERT_EQ(pos.y, 3);

  parseCommand("d", NULL);
  pos = getPlayerPosition();
  ASSERT_EQ(pos.x, 2);
  ASSERT_EQ(pos.y, 3);


  parseCommand("e", NULL);

  ASSERT_EQ(getPlayerDirection(), 3);

  parseCommand("w", NULL);
  pos = getPlayerPosition();
  ASSERT_EQ(pos.x, 1);
  ASSERT_EQ(pos.y, 3);

  parseCommand("s", NULL);
  pos = getPlayerPosition();
  ASSERT_EQ(pos.x, 2);
  ASSERT_EQ(pos.y, 3);

  parseCommand("a", NULL);
  pos = getPlayerPosition();
  ASSERT_EQ(pos.x, 2);
  ASSERT_EQ(pos.y, 4);

  parseCommand("d", NULL);
  pos = getPlayerPosition();
  ASSERT_EQ(pos.x, 2);
  ASSERT_EQ(pos.y, 3);


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


TEST_F(TestMovement, canWalkBetweenRooms) {
  char buffer[255];
  char *operator1;
  char *operand1;

  ASSERT_EQ(getPlayerRoom(), 1);

  parseCommand("pick", "low-rank-keycard");
  parseCommand("pick", "magnetic-boots");
  parseCommand("use", "magnetic-boots");
  
  strcpy(&buffer[0], "walkTo 0 0");
  operator1 = strtok( &buffer[0], "\n " );
  operand1 = strtok( NULL, "\n ");
  parseCommand(operator1, operand1);
  ASSERT_EQ(getPlayerRoom(), 1);

  parseCommand("w", NULL);
  ASSERT_EQ(getPlayerRoom(), 2);

  parseCommand("s", NULL);
  ASSERT_EQ(getPlayerRoom(), 1);

  parseCommand("move", "0");
  ASSERT_EQ(getPlayerRoom(), 2);

  parseCommand("move", "0");
  ASSERT_EQ(getPlayerRoom(), 3);

  parseCommand("move", "1");
  ASSERT_EQ(getPlayerRoom(), 5);

  strcpy(&buffer[0], "walkTo 0 0");
  operator1 = strtok( &buffer[0], "\n " );
  operand1 = strtok( NULL, "\n ");

  parseCommand("a", NULL);
  ASSERT_EQ(getPlayerRoom(), 3);

  parseCommand("d", NULL);
  ASSERT_EQ(getPlayerRoom(), 5);

}

TEST_F(TestMovement, roomsCanRequireSpecialRankForAccess) {
  ASSERT_EQ(getPlayerRoom(), 1);
  parseCommand("move", "0");
  ASSERT_EQ(getPlayerRoom(), 2);
  parseCommand("move", "2");

  getRoom(2)->rankRequired = 2;
  parseCommand("move", "0");
  ASSERT_EQ(getPlayerRoom(), 1);

  setPlayerRank(2);

  parseCommand("move", "0");
  ASSERT_EQ(getPlayerRoom(), 2);
}

extern struct Item item[26];

TEST_F(TestMovement, keycardsCanElevatePlayerRankIfItsHigherThanCurrent) {
  ASSERT_EQ(getPlayerRoom(), 1);
  ASSERT_EQ(getPlayerRank(), 0);
  parseCommand("pick", "low-rank-keycard");
  ASSERT_EQ(getPlayerRank(), 1);
  parseCommand("drop", "low-rank-keycard");
  ASSERT_EQ(getPlayerRank(), 0);

  addToRoom( "lss-daedalus", &item[25]);
  addToRoom( "lss-daedalus", &item[18]);

  parseCommand("pick", "root-keycard");

  ASSERT_EQ(getPlayerRank(), 3);

  parseCommand("pick", "low-rank-keycard");
  ASSERT_EQ(getPlayerRank(), 3);

  parseCommand("drop", "low-rank-keycard");
  ASSERT_EQ(getPlayerRank(), 3);

  parseCommand("pick", "high-rank-keycard");
  ASSERT_EQ(getPlayerRank(), 3);

  parseCommand("drop", "root-keycard");
  ASSERT_EQ(getPlayerRank(), 2);

  parseCommand("drop", "high-rank-keycard");
  ASSERT_EQ(getPlayerRank(), 0);
}

TEST_F(TestMovement, walkingTowardsWallsWillBlockMovement) {


  char buffer[255];
  char *operator1;
  char *operand1;

  ASSERT_EQ(getPlayerRoom(), 1);

  parseCommand("pick", "low-rank-keycard");
  
  strcpy(&buffer[0], "walkTo 0 0");
  operator1 = strtok( &buffer[0], "\n " );
  operand1 = strtok( NULL, "\n ");
  parseCommand(operator1, operand1);
  ASSERT_EQ(getPlayerRoom(), 1);

  parseCommand("a", NULL);
  ASSERT_EQ(getPlayerRoom(), 1);

  snprintf(&buffer[0], 255, "walkTo %d 0", getRoom(getPlayerRoom())->sizeX - 1);
  operator1 = strtok( &buffer[0], "\n " );
  operand1 = strtok( NULL, "\n ");
  parseCommand(operator1, operand1);
  ASSERT_EQ(getPlayerRoom(), 1);

  parseCommand("d", NULL);
  ASSERT_EQ(getPlayerRoom(), 1);


  snprintf(&buffer[0], 255, "walkTo 0 %d", getRoom(getPlayerRoom())->sizeY - 1);
  operator1 = strtok( &buffer[0], "\n " );
  operand1 = strtok( NULL, "\n ");
  parseCommand(operator1, operand1);
  ASSERT_EQ(getPlayerRoom(), 1);

  parseCommand("s", NULL);
  ASSERT_EQ(getPlayerRoom(), 1);

  parseCommand("move", "0");
  ASSERT_EQ(getPlayerRoom(), 2);

  parseCommand("move", "0");
  ASSERT_EQ(getPlayerRoom(), 3);

  parseCommand("move", "0");
  ASSERT_EQ(getPlayerRoom(), 4);

  strcpy(&buffer[0], "walkTo 0 0");
  operator1 = strtok( &buffer[0], "\n " );
  operand1 = strtok( NULL, "\n ");

  parseCommand("w", NULL);
  ASSERT_EQ(getPlayerRoom(), 4);
}
