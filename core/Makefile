CXXFLAGS = -Iinclude -O3 -g -c -std=c++14  -ferror-limit=1 -Wno-narrowing   \
		-I$(GTEST_DIR)/include \
		-I$(GTEST_DIR) \
		-I$(GTEST_DIR)/include/gtest \
		-I$(GTEST_DIR)/include/gtest/internal \
		-I$(GMOCK_DIR)/include \
		-I$(GMOCK_DIR) \
		-I$(GMOCK_DIR)/include/gtest \
		-I$(GMOCK_DIR)/include/gtest/internal -DHAS_STDIO -DMOVE_TO_OBJECT_POSITION_WHEM_PICKING

CFLAGS = -Iinclude -O3 -g -c -std=c89 -ansi  -ferror-limit=1 -DHAS_STDIO -DMOVE_TO_OBJECT_POSITION_WHEM_PICKING

CXX = clang++
CC = clang

GTEST_DIR = googletest/googletest
GMOCK_DIR = googletest/googlemock

TESTOBJS = Tests/TestMovement.o \
		Tests/TestInventoryManipulation.o \
		Tests/TestObjectManipulation.o \
		Tests/TestVictoryConditionsAndEndings.o \
		Tests/TestParser.o \
		$(GTEST_DIR)/src/gtest-all.o \
		$(GMOCK_DIR)/src/gmock-all.o \
		$(GMOCK_DIR)/src/gmock_main.o

OBJS = src/Derelict.o src/Parser.o src/Core.o

MAIN_GAME_OBJ = src/main.o

LDFLAGS = -lncurses
TESTLDFLAGS = -lpthread
TARGET = blues
TESTTARGET = unittests

$(TARGET):	$(OBJS) $(MAIN_GAME_OBJ)
	$(CXX) -o $(TARGET) $(OBJS) $(MAIN_GAME_OBJ) $(LDFLAGS)

$(TESTTARGET): $(OBJS) $(TESTOBJS)
	$(CXX) -o $(TESTTARGET) -g $(OBJS) $(TESTOBJS) $(TESTLDFLAGS) $(LDFLAGS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TESTTARGET) $(MAIN_GAME_OBJ) $(TESTOBJS) $(TARGET)
	rm -rf coverage
	find . | grep ~ | xargs rm -f
	find . | grep gcno | xargs rm -f
	find . | grep gcda | xargs rm -f
