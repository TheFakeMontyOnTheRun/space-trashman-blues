cmake_minimum_required(VERSION 3.11)
project(derelict16)

set(CMAKE_C_STANDARD 99)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-sdl2")

include_directories(src ../core/src ../core/include include ../common/include
        ${SDL2_INCLUDE_DIR}
        )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSDLW -DENDIANESS_AWARE -DMONOCHROME_VECTORS -DEMBEDDED_DATA -DUSE_OWN_MIN_MAX -DRLE_COMPRESSED_MAPS -DSUPPORTS_ROOM_TRANSITION_ANIMATION -DSUPPORTS_HACKING_MINIGAME -DTRACE_OBJECTS_OVER_FLOOR -DCAN_PICK_OBJECT_AT_ANY_DISTANCE -DRES128X128 -Wall -Wextra -pedantic -Wunused-parameter -DXRES_FRAMEBUFFER=256 -DYRES_FRAMEBUFFER=192 -ggdb3 -DEMIT_QUIT_OPTION -DENDIANESS_AWARE -DSDL_DISABLE_IMMINTRIN_H")

find_package(SDL2 REQUIRED)

add_executable(derelict16
        src/RendererTesselation.c
        src/Main.c
        ../core/src/Derelict.c
        ../core/src/Core.c
        ../common/src/PackedFileReader.c
        ../common/src/NullMusic.c
        ../common/src/Common.c
        ../common/src/Engine.c
        ../common/src/CTile3DProperties.c
        ../common/src/MapWithCharKey.c
        src/HackingMinigame.c
        include/HackingMinigame.h
        src/SDLVersion/sdl2.c
        src/UI.c
        src/Events.c
        src/RendererRasterizer.c
        src/RendererTesselation.c
        src/RendererScene.c
        src/KeyboardUI.c
        src/Crawler.c
        src/GameMenu.c
        ../common/src/HelpScreen.c
        ../common/src/MainMenu.c
        ../common/src/CreditsScreen.c
        ../common/src/HackingMinigameRules.c
        ../common/include/HackingMinigameRules.h
        )

include(FindPkgConfig)

target_include_directories(derelict16 PUBLIC ${SDL2_INCLUDE_DIR})
target_link_libraries(derelict16 LINK_PUBLIC ${SDL2_LIBRARY} m)

