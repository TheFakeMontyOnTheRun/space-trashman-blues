GTEST_DIR="googletest/googletest"
GMOCK_DIR="googletest/googlemock"
find . | grep gcno | xargs rm
find . | grep gcda | xargs rm
make clean
make CXX='g++' CXXFLAGS='-Iinclude -g -c -std=c++14 -fprofile-arcs -ftest-coverage -I$(GTEST_DIR)/include -I$(GTEST_DIR) -I$(GTEST_DIR)/include/gtest -I$(GTEST_DIR)/include/gtest/internal -I$(GMOCK_DIR)/include -I$(GMOCK_DIR) -I$(GMOCK_DIR)/include/gtest -I$(GMOCK_DIR)/include/gtest/internal' LDFLAGS='-lncurses -lpthread --coverage' unittests

./unittests
#gcov lib/googletest/googlemock/src/gmock_main.gcno
lcov --capture --directory . --output-file lcov-output.info
genhtml lcov-output.info --output-directory coverage
rm lcov-output.info
