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
	parseCommand("move", "0");
	ASSERT_EQ(getPlayerRoom(), 2);
	parseCommand("move", "3");
	ASSERT_EQ(getPlayerRoom(), 2);
	parseCommand("move", "1");
	ASSERT_EQ(getPlayerRoom(), 2);

}

TEST_F(TestMovement, canWalkInsideRooms) {

  char buffer[255];
  strcpy(&buffer[0], "walkTo 2 3");
  char *operator1 = strtok( &buffer[0], "\n " );
  char *operand1 = strtok( NULL, "\n ");

  parseCommand(operator1, operand1);

  auto pos = getPlayerPosition();
  ASSERT_EQ(pos.x, 2);
  ASSERT_EQ(pos.y, 3);
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
