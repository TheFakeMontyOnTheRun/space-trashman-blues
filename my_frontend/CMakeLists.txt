cmake_minimum_required(VERSION 3.11)
project(derelict8)

set(CMAKE_C_STANDARD 99)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-sdl2")

include_directories(src ../core/src ../core/include include ../common/include
        ${SDL2_INCLUDE_DIR}
        )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSDLW -DENDIANESS_AWARE -DEMBEDDED_DATA -DRLE_COMPRESSED_MAPS -DSUPPORTS_ROOM_TRANSITION_ANIMATION -DSUPPORTS_HACKING_MINIGAME -DTRACE_OBJECTS_OVER_FLOOR -DCAN_PICK_OBJECT_AT_ANY_DISTANCE -DRES128X128 -Wall -Wextra -pedantic -Wunused-parameter -DXRES_FRAMEBUFFER=256 -DYRES_FRAMEBUFFER=192 -ggdb3")

find_package(SDL2 REQUIRED)

add_executable(derelict8
        src/RendererTesselation.c
        src/sdl2.c
        src/Main.c
        ../core/src/Derelict.c
        ../core/src/Core.c
        include/Renderer.h
        ../common/src/PackedFileReader.c
        ../common/src/NullMusic.c
        ../common/src/Common.c
        ../common/src/Engine.c
        src/HackingMinigame.c
        include/HackingMinigame.h
        src/UI.c
        src/Events.c
        src/RendererRasterizer.c
        src/RendererTesselation.c
        src/RendererScene.c
        src/KeyboardUI.c
        src/Crawler.c
        src/Help.c
        src/MainMenu.c
        src/Credits.c
        src/GameMenu.c
        ../common/src/HackingMinigameRules.c
        ../common/include/HackingMinigameRules.h
        )

include(FindPkgConfig)

target_include_directories(derelict8 PUBLIC ${SDL2_INCLUDE_DIR})
target_link_libraries(derelict8 LINK_PUBLIC ${SDL2_LIBRARY} m)

