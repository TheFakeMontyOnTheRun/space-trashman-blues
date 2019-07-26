cmake_minimum_required(VERSION 3.14)
project(derelict)

set(CMAKE_C_STANDARD 90)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -W -Wall -Wextra -pedantic")

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)


enable_testing()

add_subdirectory(googletest)

include_directories(include)
include_directories(googletest/googletest/include)
include_directories(googletest/googlemock/include)

add_executable(derelict src/main.c src/Derelict.c)

add_executable(derelict_test tests/TestMovement.cpp)

target_link_libraries(derelict_test gtest gmock gmock_main )