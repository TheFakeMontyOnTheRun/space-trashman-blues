cmake_minimum_required(VERSION 3.12)
project(derelict-core)

set(CMAKE_C_STANDARD 90)
set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -W -Wall -Wextra -pedantic -DHAS_STDIO -DMOVE_TO_OBJECT_POSITION_WHEM_PICKING")

if (APPLE)
	add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
	add_definitions(-D__GLIBCXX__)
endif (APPLE)


enable_testing()

add_subdirectory(googletest)

include_directories(include)
include_directories(googletest/googletest/include)
include_directories(googletest/googlemock/include)

set(TEST_SOURCE_FILES
		tests/TestMovement.cpp
		tests/TestObjectManipulation.cpp
		tests/TestVictoryConditionsAndEndings.cpp
		tests/TestInventoryManipulation.cpp)

set(SOURCE_FILES
		src/Derelict.c
		src/Core.c
		src/Parser.c
		)

add_executable(derelict-core src/main.c ${SOURCE_FILES} )

add_executable(derelict_test  ${TEST_SOURCE_FILES} ${SOURCE_FILES} )

target_link_libraries(derelict_test gtest gmock gmock_main )