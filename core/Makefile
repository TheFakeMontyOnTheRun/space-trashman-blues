GTEST_DIR = gtest
GMOCK_DIR = gmock/googlemock

CXXFLAGS = -Iinclude -I../common/include -O3 -g -c -std=c++0x -Wno-narrowing   \
		-DHAS_STDIO -DMOVE_TO_OBJECT_POSITION_WHEN_PICKING -DMORE_OBJECTS \
		-DGTEST_LANG_CXX11=0 \
		-I$(GTEST_DIR) \
		-I$(GTEST_DIR)/include \
		-I$(GMOCK_DIR) \
		-I$(GMOCK_DIR)/include

CFLAGS = -Iinclude -I../common/include -O3 -g -c  -DCLI_BUILD -std=c90 -DHAS_STDIO -DMOVE_TO_OBJECT_POSITION_WHEN_PICKING -DMORE_OBJECTS

TESTOBJS = tests/TestMovement.o \
		tests/TestInventoryManipulation.o \
		tests/TestObjectManipulation.o \
		tests/TestVictoryConditionsAndEndings.o \
		tests/TestParser.o \
		tests/TestDerelict.o \
		$(GTEST_DIR)/src/gtest-all.o \
		$(GMOCK_DIR)/src/gmock-all.o \
		$(GMOCK_DIR)/src/gmock_main.o

OBJS = src/Derelict.o src/Parser.o src/Core.o

MAIN_GAME_OBJ = src/main.o

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

report:
	./build-with-coverage.sh
	lcov --no-external --capture --directory src --output-file lcov-output.info && genhtml lcov-output.info --output-directory coverage && rm lcov-output.info