//
// Created by Daniel Monteiro on 2019-07-26.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <assert.h>

extern "C" {
#include "Core.h"
#include "Derelict.h"
#include "Parser.h"
}

extern struct ObjectNode *collectedObject;

using testing::Eq;

extern struct Item item[TOTAL_ITEMS];
extern struct Room rooms[TOTAL_ROOMS];

int getRoomIdByName(const char* name) {

  int c = 1;
  for (; c < TOTAL_ROOMS; ++c) {
    if (!strcmp(rooms[c].name, name )) {
      return c;
    }
  }
  assert(FALSE);
  return 0;
}

TEST(TestVictoryCondition, activatingTheRemoteControlWithTheActiveBombWillExplodeEverything) {

  initStation();
  ASSERT_EQ(getPlayerRoom(), 1);
  
  addObjectToRoom(getRoomIdByName("reactor-core"), getItemNamed("emp-bomb"));
  addObjectToRoom(getRoomIdByName("lss-daedalus"), getItemNamed("fuel-rods"));

  ASSERT_TRUE(hasItemInRoom("reactor-core", "emp-bomb"));
  ASSERT_FALSE(hasItemInRoom("reactor-core", "fuel-rods"));

  ASSERT_EQ(kNormalGameplay, getGameStatus());
  item[1].useCallback(NULL);
  ASSERT_EQ(kGoodVictory, getGameStatus());
}



TEST(TestVictoryCondition, placingTheBombOnLab1WithoutTheMendingWillExplodeStationWhileAtNotAtShipWillCauseYouToGoDownWithIt) {

  initStation();

  parseCommand("move", "0");
  ASSERT_FALSE(isPlayerAtRoom("lss-daedalus"));

  addObjectToRoom(getRoomIdByName("reactor-core"), getItemNamed("emp-bomb"));
  addObjectToRoom(getRoomIdByName("lss-daedalus"), getItemNamed("fuel-rods"));
  
  ASSERT_TRUE(hasItemInRoom("reactor-core", "emp-bomb"));
  ASSERT_FALSE(hasItemInRoom("reactor-core", "fuel-rods"));
 
  ASSERT_EQ(kNormalGameplay, getGameStatus());
  getItemNamed("emp-controller")->useCallback(NULL);
  ASSERT_EQ(kBadVictory, getGameStatus());
}

TEST(TestVictoryCondition, placingTheBombOnLab1WithoutTheMendingWillExplodeStationWhileAtShipWillMeanTheMissionIsAccomplished) {

  initStation();

  ASSERT_TRUE(isPlayerAtRoom("lss-daedalus"));

  addObjectToRoom(getRoomIdByName("reactor-core"), getItemNamed("emp-bomb"));
  addObjectToRoom(getRoomIdByName("lss-daedalus"), getItemNamed("fuel-rods"));
  
  ASSERT_TRUE(hasItemInRoom("reactor-core", "emp-bomb"));
  ASSERT_FALSE(hasItemInRoom("reactor-core", "fuel-rods"));

  ASSERT_EQ(kNormalGameplay, getGameStatus());
  getItemNamed("emp-controller")->useCallback(NULL);
  ASSERT_EQ(kGoodVictory, getGameStatus());
  
}

TEST(TestVictoryCondition, placingTheBombOnLab1WithTheMendingWillFailToExplodeStationProperlyWithYouAtTheShipSurvivingAndLosingYourLicense) {

  initStation();

  ASSERT_TRUE(isPlayerAtRoom("lss-daedalus"));

  addObjectToRoom(getRoomIdByName("reactor-core"), getItemNamed("emp-bomb"));

  ASSERT_TRUE(hasItemInRoom("reactor-core", "emp-bomb"));
  ASSERT_TRUE(hasItemInRoom("reactor-core", "fuel-rods"));

  ASSERT_EQ(kNormalGameplay, getGameStatus());
  getItemNamed("emp-controller")->useCallback(NULL);
  ASSERT_EQ(kGoodGameOver, getGameStatus());
}

TEST(TestVictoryCondition, placingTheBombOnLab1WithTheMendingWillFailToExplodeStationWithYouOutsideTheShipWillCauseYouToGoDownWithIt) {

  initStation();
  parseCommand("move", "0");

  ASSERT_FALSE(isPlayerAtRoom("lss-daedalus"));

  addObjectToRoom(getRoomIdByName("reactor-core"), getItemNamed("emp-bomb"));
  
  ASSERT_TRUE(hasItemInRoom("reactor-core", "emp-bomb"));
  ASSERT_TRUE(hasItemInRoom("reactor-core", "fuel-rods"));

  ASSERT_EQ(kNormalGameplay, getGameStatus());
  getItemNamed("emp-controller")->useCallback(NULL);
  ASSERT_EQ(kBadGameOver, getGameStatus());
}
