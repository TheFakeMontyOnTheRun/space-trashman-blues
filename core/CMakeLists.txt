cmake_minimum_required(VERSION 3.12)
project(derelict-core)

set(CMAKE_C_STANDARD 90)
set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_C_STANDARD_REQUIRED ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(DERELICT_CORE_FEATURE_FLAGS "-DCLI_BUILD -DHAS_STDIO -DMOVE_TO_OBJECT_POSITION_WHEN_PICKING -DMORE_OBJECTS")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${DERELICT_CORE_FEATURE_FLAGS} -Werror -W -Wall -Wextra -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DERELICT_CORE_FEATURE_FLAGS} -Wno-writable-strings ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGTEST_LANG_CXX11=0")

if (APPLE)
	add_definitions(-D__GLIBCXX__)
endif (APPLE)

enable_testing()

include_directories(include)

include_directories(gtest)
include_directories(gtest/include)

include_directories(gmock/googlemock)
include_directories(gmock/googlemock/include)

set(TEST_SOURCE_FILES
		tests/TestMovement.cpp
		tests/TestObjectManipulation.cpp
		tests/TestVictoryConditionsAndEndings.cpp
		tests/TestInventoryManipulation.cpp
		tests/TestVictoryConditionsAndEndings.cpp

		gtest/src/gtest-all.cc
		gmock/googlemock/src/gmock-all.cc
		gmock/googlemock/src/gmock_main.cc

		)

set(SOURCE_FILES
		src/Derelict.c
		src/Core.c
		src/Parser.c
		)

add_executable(derelict-core src/main.c ${SOURCE_FILES} )

add_executable(derelict_test  ${TEST_SOURCE_FILES} ${SOURCE_FILES} )

target_link_libraries(derelict_test PRIVATE Threads::Threads)