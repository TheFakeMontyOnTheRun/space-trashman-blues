#!/bin/sh

make CFLAGS='-Iinclude -I../common/include -fprofile-arcs -ftest-coverage -DMORE_OBJECTS -DCLI_BUILD -DMOVE_TO_OBJECT_POSITION_WHEN_PICKING' CC='gcc' CXX='g++' CXXFLAGS='-Iinclude -I../common/include -std=c++14 -c -Wno-writable-strings -fprofile-arcs -ftest-coverage -I$(GTEST_DIR)/include -I$(GTEST_DIR) -I$(GMOCK_DIR) -I$(GMOCK_DIR)/include -DMOVE_TO_OBJECT_POSITION_WHEN_PICKING' LDFLAGS='-lpthread --coverage' check
