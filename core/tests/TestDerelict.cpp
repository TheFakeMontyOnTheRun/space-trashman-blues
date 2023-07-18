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
#include "Common.h"
}

extern struct ObjectNode *collectedObject;
extern uint8_t accessGrantedToSafe;
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

	setPlayerRank(3);
	getItemNamed("comm-terminal-2")->active = TRUE;
	setPlayerLocation(elevatorLevel3->connections[5]);
	parseCommand("use", "elevator-level2-go-down");
	ASSERT_EQ(getRoom(getPlayerRoom()), elevatorLevel3);
}

TEST_F(TestDerelict, usingElevatorToGoDownWillNotMoveIfPlayerDoesntHaveEnoughRank) {
	struct Room* elevatorLevel3 = getRoomByName("elevator-level-3");

	getItemNamed("comm-terminal-2")->active = TRUE;
	setPlayerLocation(elevatorLevel3->connections[5]);
	parseCommand("use", "elevator-level2-go-down");
	ASSERT_NE(getRoom(getPlayerRoom()), elevatorLevel3);
}

TEST_F(TestDerelict, usingElevatorToGoDownWillNotMoveIfCommTerminalIsOffline) {
	struct Room* elevatorLevel3 = getRoomByName("elevator-level-3");

	setPlayerRank(3);
	setPlayerLocation(elevatorLevel3->connections[5]);
	parseCommand("use", "elevator-level2-go-down");
	ASSERT_NE(getRoom(getPlayerRoom()), elevatorLevel3);
}

TEST_F(TestDerelict, usingElevatorToGoDownWillNotMovePastTheLimit) {
	struct Room* elevatorLevel2 = getRoomByName("elevator-level-2");
	struct Room* elevatorLevel3 = getRoomByName("elevator-level-3");

	setPlayerRank(3);
	getItemNamed("comm-terminal-3")->active = TRUE;
	setPlayerLocation(elevatorLevel2->connections[4]);
	parseCommand("use", "elevator-level3-go-down");
	ASSERT_EQ(getRoom(getPlayerRoom()), elevatorLevel3);
}

TEST_F(TestDerelict, usingElevatorToGoUpWillMovePlayerUp) {
	struct Room* elevatorLevel1 = getRoomByName("elevator-level-1");
	setPlayerRank(3);
	getItemNamed("comm-terminal-2")->active = TRUE;
	setPlayerLocation(elevatorLevel1->connections[4]);
	parseCommand("use", "elevator-level2-go-up");
	ASSERT_EQ(getRoom(getPlayerRoom()), elevatorLevel1);
}

TEST_F(TestDerelict, usingElevatorToGoUpWillNotMovePastTheLimit) {
	struct Room* elevatorLevel1 = getRoomByName("elevator-level-1");
	struct Room* elevatorLevel2 = getRoomByName("elevator-level-2");

	setPlayerRank(3);
	getItemNamed("comm-terminal-1")->active = TRUE;
	setPlayerLocation(elevatorLevel2->connections[5]);
	parseCommand("use", "elevator-level1-go-up");
	ASSERT_EQ(getRoom(getPlayerRoom()), elevatorLevel1);
}

TEST_F(TestDerelict, usingElevatorToGoUpWillNotMoveIfPlayerDoesntHaveEnoughRank) {
	struct Room* elevatorLevel1 = getRoomByName("elevator-level-1");

	getItemNamed("comm-terminal-2")->active = TRUE;
	setPlayerLocation(elevatorLevel1->connections[4]);
	parseCommand("use", "elevator-level2-go-up");
	ASSERT_NE(getRoom(getPlayerRoom()), elevatorLevel1);
}

TEST_F(TestDerelict, usingElevatorToGoUpWillNotMoveIfCommTerminalIsOffline) {
	struct Room* elevatorLevel1 = getRoomByName("elevator-level-1");

	setPlayerRank(3);
	setPlayerLocation(elevatorLevel1->connections[4]);
	parseCommand("use", "elevator-level2-go-up");
	ASSERT_NE(getRoom(getPlayerRoom()), elevatorLevel1);
}

TEST_F(TestDerelict, usingElevatorToGoUpWillMoveObjectsAlongWithPlayer) {
	struct Room* elevatorLevel1 = getRoomByName("elevator-level-1");
	setPlayerRank(3);
	getItemNamed("comm-terminal-2")->active = TRUE;
	parseCommand("pick", "emp-bomb");
	setPlayerLocation(elevatorLevel1->connections[4]);
	parseCommand("drop", "emp-bomb");
	parseCommand("use", "elevator-level2-go-up");
	ASSERT_EQ(getRoom(getPlayerRoom()), elevatorLevel1);
	ASSERT_TRUE(hasItemInRoom("elevator-level-1", "emp-bomb"));
}

TEST_F(TestDerelict, usingElevatorToGoDownWillMoveObjectsAlongWithPlayer) {
	struct Room* elevatorLevel3 = getRoomByName("elevator-level-3");

	setPlayerRank(3);
	getItemNamed("comm-terminal-2")->active = TRUE;
	parseCommand("pick", "emp-bomb");
	setPlayerLocation(getRoomIdByName("elevator-level-2"));
	parseCommand("drop", "emp-bomb");
	parseCommand("use", "elevator-level2-go-down");
	ASSERT_EQ(getRoom(getPlayerRoom()), elevatorLevel3);
	ASSERT_TRUE(hasItemInRoom("elevator-level-3", "emp-bomb"));
}

TEST_F(TestDerelict, keycardsCanElevatePlayerRankIfItsHigherThanCurrent) {
    ASSERT_EQ(getPlayerRoom(), 1);
    ASSERT_EQ(getPlayerRank(), 0);
    addToRoom( "lss-daedalus", getItemNamed("low-rank-keycard"));
    addToRoom( "lss-daedalus", getItemNamed("high-rank-keycard"));
    addToRoom( "lss-daedalus", getItemNamed("root-keycard"));
    addToRoom( "lss-daedalus", getItemNamed("hacked-keycard"));

    parseCommand("pick", "low-rank-keycard");
    ASSERT_EQ(getPlayerRank(), 1);
    parseCommand("drop", "low-rank-keycard");
    ASSERT_EQ(getPlayerRank(), 0);


    parseCommand("pick", "root-keycard");

    ASSERT_EQ(getPlayerRank(), 4);

    parseCommand("pick", "low-rank-keycard");
    ASSERT_EQ(getPlayerRank(), 4);

    parseCommand("drop", "low-rank-keycard");
    ASSERT_EQ(getPlayerRank(), 4);

    parseCommand("pick", "high-rank-keycard");
    ASSERT_EQ(getPlayerRank(), 4);

    parseCommand("drop", "root-keycard");
    ASSERT_EQ(getPlayerRank(), 3);

    parseCommand("drop", "high-rank-keycard");
    ASSERT_EQ(getPlayerRank(), 0);

    parseCommand("pick", "hacked-keycard");

    ASSERT_EQ(getPlayerRank(), 2);
}

TEST_F(TestDerelict, canOnlyUseCommWithHigherRanks) {
    setPlayerLocation(getRoomIdByName("hall-2"));
    addToRoom( "hall-2", getItemNamed("hacked-keycard"));

    parseCommand("use", "comm-terminal-2");
    ASSERT_FALSE(getItemNamed("comm-terminal-2")->active);

    parseCommand("pick", "low-rank-keycard");
    parseCommand("use", "comm-terminal-2");
    ASSERT_FALSE(getItemNamed("comm-terminal-2")->active);

    parseCommand("pick", "hacked-keycard");
    parseCommand("use", "comm-terminal-2");
    ASSERT_TRUE(getItemNamed("comm-terminal-2")->active);
}


TEST_F(TestDerelict, cantUnlockSafeWithOfflineComputerNodes) {
    setPlayerLocation(getRoomIdByName("situation-room"));

    parseCommand("use", "digital-safe");
    ASSERT_FALSE(hasItemInRoom("situation-room", "root-keycard"));

    accessGrantedToSafe = TRUE;

    parseCommand("use", "digital-safe");
    ASSERT_FALSE(hasItemInRoom("situation-room", "root-keycard"));

}

TEST_F(TestDerelict, canUnlockSafeToCollectTheRootCard) {
    setPlayerLocation(getRoomIdByName("situation-room"));

    getItemNamed("comm-terminal-1")->active = 1;
    getItemNamed("comm-terminal-2")->active = 1;
    getItemNamed("comm-terminal-3")->active = 1;

    parseCommand("use", "digital-safe");
    ASSERT_FALSE(hasItemInRoom("situation-room", "root-keycard"));

    accessGrantedToSafe = TRUE;

    parseCommand("use", "digital-safe");
    ASSERT_TRUE(hasItemInRoom("situation-room", "root-keycard"));

}

TEST_F(TestDerelict, usingTheReactorCoreWillCauseMeltdown) {
    setPlayerLocation(getRoomIdByName("reactor-core"));
    ASSERT_EQ(kNormalGameplay, getGameStatus());
    parseCommand("use", "reactor-valve-control");
    ASSERT_EQ(kBadVictory, getGameStatus());
}

TEST_F(TestDerelict, canToggleCommTerminal1) {
    setPlayerLocation(getRoomIdByName("hall-1"));
    setPlayerRank(5);
    ASSERT_FALSE(getItemNamed("comm-terminal-1")->active);
    parseCommand("use", "comm-terminal-1");
    ASSERT_TRUE(getItemNamed("comm-terminal-1")->active);
}

TEST_F(TestDerelict, cantToggleMagneticBoots) {
    ASSERT_TRUE(getItemNamed("magnetic-boots")->active);
    parseCommand("use", "magnetic-boots");
    ASSERT_TRUE(getItemNamed("magnetic-boots")->active);
}

TEST_F(TestDerelict, cantToggleCommTerminal1ByUsingWithOthers) {
    setPlayerLocation(getRoomIdByName("hall-1"));
    addToRoom( "hall-1", getItemNamed("hacked-keycard"));
    parseCommand("pick", "hacked-keycard");

    ASSERT_FALSE(getItemNamed("hacked-keycard")->active);

    char buffer[255];
    strcpy(&buffer[0], "use-with hacked-keycard comm-terminal-1");
    char *operator1 = strtok( &buffer[0], "\n " );
    char *operand1 = strtok( NULL, "\n ");
    parseCommand(operator1, operand1);
    ASSERT_FALSE(getItemNamed("hacked-keycard")->active);
}

TEST_F(TestDerelict, cantUseKeyCard) {
    setPlayerLocation(getRoomIdByName("hall-1"));
    addToRoom( "hall-1", getItemNamed("hacked-keycard"));
    parseCommand("pick", "hacked-keycard");

    ASSERT_FALSE(getItemNamed("hacked-keycard")->active);
    parseCommand("use", "hacked-keycard");
    ASSERT_FALSE(getItemNamed("hacked-keycard")->active);
}
