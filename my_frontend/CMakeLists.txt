cmake_minimum_required(VERSION 3.11)
project(derelict8)

set(CMAKE_C_STANDARD 90)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/..q/cmake-sdl2")

include_directories( src ../core/src ../core/include ../common/include
		${SDL2_INCLUDE_DIR}
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSDLSW -DSUPPORTS_ROOM_TRANSITION_ANIMATION -DSUPPORTS_HACKING_MINIGAME -DTRACE_OBJECTS_OVER_FLOOR -DCAN_PICK_OBJECT_AT_ANY_DISTANCE -DRES128X128")

find_package(SDL2 REQUIRED)

add_executable(derelict8
		src/demo.c
		src/sdl2.c
		../core/src/Derelict.c
		../core/src/Core.c
		src/Engine3D.h
		../common/src/PackedFileReader.c
		../common/src/Common.c
		src/HackingMinigame.c
		src/HackingMinigame.h)

include(FindPkgConfig)

target_include_directories(derelict8 PUBLIC ${SDL2_INCLUDE_DIR})
target_link_libraries(derelict8 LINK_PUBLIC ${SDL2_LIBRARY} m)
