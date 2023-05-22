#!/bin/sh

make CFLAGS='-Iinclude -fprofile-arcs -ftest-coverage -DMORE_OBJECTS -DCLI_BUILD -DMOVE_TO_OBJECT_POSITION_WHEN_PICKING' CC='gcc' CXX='g++' CXXFLAGS='-Iinclude -std=c++14 -c -Wno-writable-strings -fprofile-arcs -ftest-coverage -I$(GTEST_DIR)/include -I$(GTEST_DIR) -I$(GMOCK_DIR) -I$(GMOCK_DIR)/include -DMOVE_TO_OBJECT_POSITION_WHEN_PICKING' LDFLAGS='-lncurses -lpthread --coverage' check
