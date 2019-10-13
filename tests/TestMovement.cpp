//
// Created by Daniel Monteiro on 2019-07-26.
//

#include <gmock/gmock.h>
#include <gtest/gtest.h>

extern "C" {
#include "Derelict.h"
#include "Parser.h"
}

using testing::Eq;

TEST(TestMovement, canMoveOnlyWhenEnabled) {
  initStation();

  ASSERT_EQ( getPlayerRoom(), 1 );
  parseCommand("move", "0");
  ASSERT_EQ( getPlayerRoom(), 2 );
  parseCommand("move", "3");
  ASSERT_EQ( getPlayerRoom(), 2 );
  parseCommand("move", "1");
  ASSERT_EQ( getPlayerRoom(), 2 );

}