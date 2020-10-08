GTEST_DIR="googletest/googletest"
GMOCK_DIR="googletest/googlemock"
find . | grep gcno | xargs rm
find . | grep gcda | xargs rm
make clean
make CFLAGS='-Iinclude -g -c -fprofile-arcs -ftest-coverage' CC='gcc' CXX='g++' CXXFLAGS='-Iinclude -g -std=c++14 -c -fprofile-arcs -ftest-coverage -I$(GTEST_DIR)/include -I$(GTEST_DIR) -I$(GTEST_DIR)/include/gtest -I$(GTEST_DIR)/include/gtest/internal -I$(GMOCK_DIR)/include -I$(GMOCK_DIR) -I$(GMOCK_DIR)/include/gtest -I$(GMOCK_DIR)/include/gtest/internal' LDFLAGS='-lncurses -lpthread --coverage' unittests
./unittests
find googletest | grep gcno | xargs rm
find googletest | grep gcda | xargs rm
find tests | grep gcda | xargs rm
lcov --no-external --capture --directory . --output-file lcov-output.info
genhtml lcov-output.info --output-directory coverage
rm lcov-output.info