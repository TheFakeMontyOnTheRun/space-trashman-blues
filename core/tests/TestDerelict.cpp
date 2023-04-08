//
// Created by Daniel Monteiro on 2023-04-07.
//

#include <memory>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

extern "C" {
#include "Core.h"
#include "Derelict.h"
#include "Parser.h"
}

extern struct ObjectNode *collectedObject;
using namespace std;
using testing::Eq;

static void myDummyOutputHandler(const char* msg) {}

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

class TestDerelict : public ::testing::Test {

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

shared_ptr<ErrorHandlerMock> TestDerelict::mockedObj;

static void myErrorHandler(const char* errorMsg) {
	TestDerelict::mockedObj->handleError();
}

TEST_F(TestDerelict, canUseCardWriterToHackKeycards) {
	struct Item* lowRankCard = getItemNamed("low-rank-keycard");
	struct Item* cardWritter = getItemNamed("card-writer");

	setPlayerLocation(lowRankCard->roomId);
	pickObjectByName("low-rank-keycard");

	ASSERT_FALSE(hasItemInRoom("computer-core", "hacked-keycard"));
	useCardWithCardWriter(lowRankCard, cardWritter);
	ASSERT_TRUE(hasItemInRoom("computer-core", "hacked-keycard"));
}

TEST_F(TestDerelict, cantUseCardWriterToHackOtherThings) {
	struct Item* boots = getItemNamed("magnetic-boots");
	struct Item* cardWritter = getItemNamed("card-writer");

	ASSERT_FALSE(hasItemInRoom("computer-core", "hacked-keycard"));
	useCardWithCardWriter(boots, cardWritter);
	ASSERT_FALSE(hasItemInRoom("computer-core", "hacked-keycard"));
}

TEST_F(TestDerelict, canUseMagneticBootsToDisableMagneticCoupling) {
	struct Item* boots = getItemNamed("magnetic-boots");
	struct Item* coupling = getItemNamed("magnetic-coupling");

	ASSERT_TRUE(coupling->active);
	useBootsWithMagneticCoupling(boots, coupling);
	ASSERT_FALSE(coupling->active);
}

TEST_F(TestDerelict, usingOtherObjectsWithMagneticCoupleAchievesNothing) {
	struct Item* helmet = getItemNamed("helmet");
	struct Item* coupling = getItemNamed("magnetic-coupling");

	ASSERT_TRUE(coupling->active);
	useBootsWithMagneticCoupling(helmet, coupling);
	ASSERT_TRUE(coupling->active);
}

TEST_F(TestDerelict, usingTheCloggedFlushWillProduceTheHighRankKeycard) {
	struct Item* highRankCard = getItemNamed("high-rank-keycard");
	struct Item* cloggedFlush = getItemNamed("clogged-flush");

	ASSERT_FALSE(hasItemInRoom("wc", "high-rank-keycard"));
	useCloggedFlush(NULL);
	ASSERT_TRUE(hasItemInRoom("wc", "high-rank-keycard"));
}

TEST_F(TestDerelict, usingTheRegularFlushWillNotProduceTheHighRankKeycard) {
	struct Item* highRankCard = getItemNamed("high-rank-keycard");
	struct Item* cloggedFlush = getItemNamed("clogged-flush");

	ASSERT_FALSE(hasItemInRoom("wc", "high-rank-keycard"));
	useRegularFlush(NULL);
	ASSERT_FALSE(hasItemInRoom("wc", "high-rank-keycard"));
}

TEST_F(TestDerelict, usingElevatorToGoDownWillMovePlayerDown) {
	struct Room* elevatorLevel3 = getRoomByName("elevator-level-3");

	getItemNamed("comm-terminal-2")->active = TRUE;
	setPlayerLocation(elevatorLevel3->connections[5]);
	parseCommand("use", "elevator-level2-go-down");
	ASSERT_EQ(getRoom(getPlayerRoom()), elevatorLevel3);
}


TEST_F(TestDerelict, usingElevatorToGoUpWillMovePlayerUp) {
	struct Room* elevatorLevel1 = getRoomByName("elevator-level-1");

	getItemNamed("comm-terminal-2")->active = TRUE;
	setPlayerLocation(elevatorLevel1->connections[4]);
	parseCommand("use", "elevator-level2-go-up");
	ASSERT_EQ(getRoom(getPlayerRoom()), elevatorLevel1);


}