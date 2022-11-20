#!/bin/sh

GTEST_DIR="gtest"
GMOCK_DIR="gmock/googlemock"
find . | grep gcno | xargs rm
find . | grep gcda | xargs rm
make CFLAGS='-Iinclude -g -c -fprofile-arcs -ftest-coverage -DMORE_OBJECTS -DCLI_BUILD -DHAS_STDIO -DMOVE_TO_OBJECT_POSITION_WHEM_PICKING' CC='gcc' CXX='g++' CXXFLAGS='-Iinclude -g -std=c++14 -c -Wno-writable-strings -fprofile-arcs -ftest-coverage -I$(GTEST_DIR)/include -I$(GTEST_DIR) -I$(GMOCK_DIR) -I$(GMOCK_DIR)/include -DHAS_STDIO -DMOVE_TO_OBJECT_POSITION_WHEM_PICKING' LDFLAGS='-lncurses -lpthread --coverage' check
find gtest | grep gcno | xargs rm
find gmock | grep gcda | xargs rm
find tests | grep gcda | xargs rm
lcov --no-external --capture --directory . --output-file lcov-output.info
genhtml lcov-output.info --output-directory coverage
rm lcov-output.info
