cmake_minimum_required(VERSION 3.11)
project(madame_y)

set(CMAKE_C_STANDARD 90)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_sdl2")

include_directories( src ../core/src ../core/include
		${SDL2_INCLUDE_DIR}
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSDLSW -std=c90 -W -ansi -g -pedantic -Wall")

find_package(SDL2 REQUIRED)

add_executable(madame_y src/demo.c src/sdl2.c
		../core/src/Derelict.c
		../core/src/Parser.c

		)

include(FindPkgConfig)

target_include_directories(madame_y PUBLIC ${SDL2_INCLUDE_DIR})
target_link_libraries(madame_y LINK_PUBLIC ${SDL2_LIBRARY} m)

