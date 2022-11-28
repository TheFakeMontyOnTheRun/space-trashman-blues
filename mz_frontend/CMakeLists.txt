cmake_minimum_required(VERSION 3.11)
project(derelict16)

set(CMAKE_C_STANDARD 11)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-sdl2")

include_directories( src ../core/src ../core/include ../common/include Engine3D
		${SDL2_INCLUDE_DIR}
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSDLSW -DLEAN_BUILD -DRES128X128 -DSUPPORTS_HACKING_MINIGAME -DTRACE_OBJECTS_OVER_FLOOR -DCAN_PICK_OBJECT_AT_ANY_DISTANCE")

find_package(SDL2 REQUIRED)

add_executable(derelict16
		Engine3D/demo.c
		Engine3D/HackingMinigame.c
		../common/src/Common.c
		SDLVersion/sdl2.c
		../core/src/Derelict.c
		../core/src/Core.c
		../common/src/PackedFileReader.c
		)

include(FindPkgConfig)

target_include_directories(derelict16 PUBLIC ${SDL2_INCLUDE_DIR})
target_link_libraries(derelict16 LINK_PUBLIC ${SDL2_LIBRARY} m)

