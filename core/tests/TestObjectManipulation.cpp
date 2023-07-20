//
// Created by Daniel Monteiro on 2019-07-26.
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

class TestObjectManipulation : public ::testing::Test {

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

shared_ptr<ErrorHandlerMock> TestObjectManipulation::mockedObj;


static void myErrorHandler(const char* errorMsg) {
	TestObjectManipulation::mockedObj->handleError();
}

void usableCallback(struct Item* item) {
	item->name = "used";
}

void usablePickCallback(struct Item* item) {
	if (!strcmp(item->name, "pick")) {
		item->name = "not-pick";
	} else {
		item->name = "pick";
	}
}

void useWithCallback(struct Item* item1, struct Item* item2 ) {
	if (!strcmp(item2->name, "farofinha")) {
		item1->name = "used-twofold";
	}
}

TEST_F(TestObjectManipulation, canUseObjectsTogether) {

	struct Item *item;

	item = addItem("usableWith",
#ifdef INCLUDE_ITEM_DESCRIPTIONS
			"",
#endif
#ifdef ITEMS_HAVE_WEIGHT
			0,
#endif
				   TRUE, 15, 19);
	item->useCallback = usableCallback;
	addToRoom("lss-daedalus", item);


	item = addItem("farofinha",
#ifdef INCLUDE_ITEM_DESCRIPTIONS
			"",
#endif
#ifdef ITEMS_HAVE_WEIGHT
			0,
#endif
				   TRUE, 15, 19);
	addToRoom("lss-daedalus", item);


	item = addItem("pamonha",
#ifdef INCLUDE_ITEM_DESCRIPTIONS
			"",
#endif
#ifdef ITEMS_HAVE_WEIGHT
			0,
#endif
				   TRUE, 15, 19);
	addToRoom("lss-daedalus", item);

	item = addItem("cocada",
#ifdef INCLUDE_ITEM_DESCRIPTIONS
			"",
#endif
#ifdef ITEMS_HAVE_WEIGHT
			0,
#endif
				   TRUE, 15, 19);
	addToRoom("lss-daedalus", item);

	ASSERT_TRUE(hasItemInRoom("lss-daedalus", "usableWith"));
	ASSERT_TRUE(hasItemInRoom("lss-daedalus", "farofinha"));
	ASSERT_TRUE(isPlayerAtRoom("lss-daedalus"));
	parseCommand("pick", "usableWith");
	parseCommand("pick", "cocada");
	ASSERT_FALSE(hasItemInRoom("hangar", "usableWith"));

	//needed to setup the tokenizer
	char buffer[255];
	strcpy(&buffer[0], "use-with usableWith farofinha");
	char *operator1 = strtok( &buffer[0], "\n " );
	char *operand1 = strtok( NULL, "\n ");

	parseCommand(operator1, operand1);
}

TEST_F(TestObjectManipulation, canUseObjects) {

	struct Item *item;

	item = addItem("usable",
#ifdef INCLUDE_ITEM_DESCRIPTIONS
			"",
#endif
#ifdef ITEMS_HAVE_WEIGHT
			0,
#endif
				   TRUE, 15, 19);
	item->useCallback = usableCallback;

	addToRoom("lss-daedalus", item);


	item = addItem("artificial",
#ifdef INCLUDE_ITEM_DESCRIPTIONS
			"",
#endif
#ifdef ITEMS_HAVE_WEIGHT
			0,
#endif
				   TRUE, 15, 19);
	addToRoom("lss-daedalus", item);

	ASSERT_TRUE(hasItemInRoom("lss-daedalus", "usable"));
	ASSERT_TRUE(isPlayerAtRoom("lss-daedalus"));
	parseCommand("pick", "usable");
	parseCommand("pick", "artificial");
	ASSERT_FALSE(hasItemInRoom("hangar", "usable"));
	parseCommand("use", "usable");
	parseCommand("drop", "used");

	ASSERT_TRUE(hasItemInRoom("lss-daedalus", "used"));
}

TEST_F(TestObjectManipulation, cantPickObjectsTwice) {

	struct Item *item;

	item = addItem("pickable",
#ifdef INCLUDE_ITEM_DESCRIPTIONS
			"",
#endif
#ifdef ITEMS_HAVE_WEIGHT
			0,
#endif
				   TRUE, 15, 19);
	item->pickCallback = usablePickCallback;

	addToRoom("lss-daedalus", item);

	parseCommand("pick", "pickable");
	ASSERT_TRUE(!strcmp(item->name, "pick"));
	parseCommand("pick", "pick");
	/* the name won't be changed by the callback */
	ASSERT_TRUE(!strcmp(item->name, "pick"));
}

TEST_F(TestObjectManipulation, usingObjectsYouDontHaveWillUseThoseInTheRoom) {

	struct Item *item;

	item = addItem("usable",
#ifdef INCLUDE_ITEM_DESCRIPTIONS
			"",
#endif
#ifdef ITEMS_HAVE_WEIGHT
			0,
#endif
				   FALSE, 15, 19);
	item->useCallback = usableCallback;

	addToRoom("lss-daedalus", item);

	parseCommand("use", "usable");

	ASSERT_TRUE(hasItemInRoom("lss-daedalus", "used"));
}

TEST_F(TestObjectManipulation, askingForInvalidObjectsYieldsNull) {
	ASSERT_TRUE(getItemNamed("farofinha") == NULL);
}

TEST_F(TestObjectManipulation, cantUseObjectsInOtherRoom) {

	struct Item *item;

	item = addItem("usable",
#ifdef INCLUDE_ITEM_DESCRIPTIONS
			"",
#endif
#ifdef ITEMS_HAVE_WEIGHT
			0,
#endif
				   FALSE, 15, 19);
	item->useCallback = usableCallback;

	addToRoom("hangar", item);

	parseCommand("use", "usable");

	ASSERT_FALSE(hasItemInRoom("lss-daedalus", "used"));
}

TEST_F(TestObjectManipulation, cantPickUnpickableObjects) {

	struct Item *item;

	item = addItem("unpickable",
#ifdef INCLUDE_ITEM_DESCRIPTIONS
			"",
#endif
#ifdef ITEMS_HAVE_WEIGHT
			0,
#endif
				   FALSE, 15, 19);
	item->useCallback = usableCallback;

	addToRoom("lss-daedalus", item);


	ASSERT_TRUE(hasItemInRoom("lss-daedalus", "unpickable"));
	ASSERT_TRUE(isPlayerAtRoom("lss-daedalus"));
	parseCommand("pick", "unpickable");
	ASSERT_TRUE(hasItemInRoom("lss-daedalus", "unpickable"));
	item->pickable = TRUE;
	parseCommand("pick", "unpickable");
	ASSERT_FALSE(hasItemInRoom("lss-daedalus", "unpickable"));
}
