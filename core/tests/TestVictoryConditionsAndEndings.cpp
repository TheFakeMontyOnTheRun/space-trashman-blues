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

extern struct Item item[25]; 

TEST(TestVictoryCondition, activatingTheRemoteControlWithTheActiveBombWillExplodeEverything) {

  initStation();
  ASSERT_EQ(getPlayerRoom(), 1);

  addObjectToRoom(21, &item[0]);
  addObjectToRoom(1, &item[21]);

  ASSERT_TRUE(hasItemInRoom("lab-1", "time-bomb"));
  ASSERT_FALSE(hasItemInRoom("lab-1", "metal-mending"));

  ASSERT_EQ(kNormalGameplay, getGameStatus());
  item[1].useCallback(NULL);
  ASSERT_EQ(kGoodVictory, getGameStatus());
}



TEST(TestVictoryCondition, placingTheBombOnLab1WithoutTheMendingWillExplodeStationWhileAtNotAtShipWillCauseYouToGoDownWithIt) {

  initStation();
  parseCommand("move", "0");
  ASSERT_NE(getPlayerRoom(), 1);

  addObjectToRoom(21, &item[0]);
  addObjectToRoom(1, &item[21]);

  ASSERT_TRUE(hasItemInRoom("lab-1", "time-bomb"));
  ASSERT_FALSE(hasItemInRoom("lab-1", "metal-mending"));

  ASSERT_EQ(kNormalGameplay, getGameStatus());
  item[0].useCallback(NULL);
  ASSERT_EQ(kBadVictory, getGameStatus());
}

TEST(TestVictoryCondition, placingTheBombOnLab1WithoutTheMendingWillExplodeStationWhileAtShipWillMeanTheMissionIsAccomplished) {

  initStation();
  ASSERT_EQ(getPlayerRoom(), 1);

  addObjectToRoom(21, &item[0]);
  addObjectToRoom(1, &item[21]);

  ASSERT_TRUE(hasItemInRoom("lab-1", "time-bomb"));
  ASSERT_FALSE(hasItemInRoom("lab-1", "metal-mending"));

  ASSERT_EQ(kNormalGameplay, getGameStatus());
  item[0].useCallback(NULL);
  ASSERT_EQ(kGoodVictory, getGameStatus());
}

TEST(TestVictoryCondition, placingTheBombOnLab1WithTheMendingWillFailToExplodeStationProperlyWithYouAtTheShipSurvivingAndLosingYourLicense) {

  initStation();
  ASSERT_EQ(getPlayerRoom(), 1);

  addObjectToRoom(21, &item[0]);
  ASSERT_TRUE(hasItemInRoom("lab-1", "time-bomb"));
  ASSERT_TRUE(hasItemInRoom("lab-1", "metal-mending"));

  ASSERT_EQ(kNormalGameplay, getGameStatus());
  item[0].useCallback(NULL);
  ASSERT_EQ(kGoodGameOver, getGameStatus());
}

TEST(TestVictoryCondition, placingTheBombOnLab1WithTheMendingWillFailToExplodeStationWithYouOutsideTheShipWillCauseYouToGoDownWithIt) {

  initStation();
  parseCommand("move", "0");
  ASSERT_NE(getPlayerRoom(), 1);

  addObjectToRoom(21, &item[0]);
  ASSERT_TRUE(hasItemInRoom("lab-1", "time-bomb"));
  ASSERT_TRUE(hasItemInRoom("lab-1", "metal-mending"));

  ASSERT_EQ(kNormalGameplay, getGameStatus());
  item[0].useCallback(NULL);
  ASSERT_EQ(kBadGameOver, getGameStatus());
}


