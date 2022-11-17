GTEST_DIR = googletest
GMOCK_DIR = googlemock

CXXFLAGS = -Iinclude -O3 -g -c -std=c++0x -Wno-narrowing   \
		-I$(GTEST_DIR)/include \
		-I$(GTEST_DIR) \
		-I$(GTEST_DIR)/include/gtest \
		-I$(GTEST_DIR)/include/gtest/internal \
		-I$(GMOCK_DIR)/include \
		-I$(GMOCK_DIR) \
		-I$(GMOCK_DIR)/include/gtest \
		-I$(GMOCK_DIR)/include/gtest/internal -DHAS_STDIO -DMOVE_TO_OBJECT_POSITION_WHEM_PICKING -DMORE_OBJECTS \
		-DGTEST_LANG_CXX11=0

CFLAGS = -Iinclude -O3 -g -c  -DCLI_BUILD -std=c90 -DHAS_STDIO -DMOVE_TO_OBJECT_POSITION_WHEM_PICKING -DMORE_OBJECTS



TESTOBJS = tests/TestMovement.o \
		tests/TestInventoryManipulation.o \
		tests/TestObjectManipulation.o \
		tests/TestVictoryConditionsAndEndings.o \
		tests/TestParser.o \
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

check: clean $(TESTTARGET)
	./unittests

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TESTTARGET) $(MAIN_GAME_OBJ) $(TESTOBJS) $(TARGET)
	rm -rf coverage
	find . | grep ~ | xargs rm -f
	find . | grep gcno | xargs rm -f
	find . | grep gcda | xargs rm -f
