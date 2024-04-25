cmake_minimum_required(VERSION 3.11)
project(derelict64)

set(CMAKE_C_STANDARD 90)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-sdl2")

include_directories(
        ${SDL2_INCLUDE_DIR}
        ${OPENGL_INCLUDE_DIR}
)

include_directories(/usr/X11R7/include)
include_directories(../core/include)
include_directories(../common/include)
include_directories(base3d/include)
include_directories(menu/include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCLI_BUILD -DTILED_BITMAPS -DINCLUDE_ITEM_DESCRIPTIONS -Wall -Wextra -Wno-unused-parameter -Wno-switch -Wno-unused-variable -pedantic -g -DENDIANESS_AWARE -DEMIT_QUIT_OPTION -DSDL_DISABLE_IMMINTRIN_H -DXRES_FRAMEBUFFER=320 -DYRES_FRAMEBUFFER=200 -DXRES_SCREEN=320 -DYRES_SCREEN=240")

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(derelict64
        menu/src/Main.c
        base3d/src/SDLVersion/CSDL2Renderer.c
        GL1Common/LoadBitmap.c
        GL1Common/Renderer.c
        GL1Common/RendererRasterization.c
        GL1Common/RendererTesselation.c
        ../common/src/MainMenu.c
        ../common/src/Crawler.c
        ../common/src/HelpScreen.c
        ../common/src/CreditsScreen.c
        ../common/src/GameMenu.c
        ../common/src/Engine.c
        ../common/src/UI.c
        ../common/src/HackingScreen.c
        ../common/src/VisibilityStrategy.c
        ../common/src/Globals.c
        ../common/src/CTile3DProperties.c
        ../common/src/Vec.c
        ../common/src/Mesh.c
        ../common/src/Common.c
        ../common/src/FixP.c
        ../common/src/MapWithCharKey.c
        ../common/src/PackedFileReader.c
	    ../common/src/EDirection_Utils.c
        ../common/src/Events.c
        ../common/src/Dungeon.c
        ../common/src/NullMusic.c
        ../common/src//PCMAudio.c
        ../common/src/HackingMinigameRules.c
        ../core/src/Derelict.c
        ../core/src/Core.c
        ../core/src/Parser.c
        )


include(FindPkgConfig)

target_include_directories(derelict64 PUBLIC ${SDL2_INCLUDE_DIR})
target_link_libraries(derelict64 LINK_PUBLIC ${SDL2_LIBRARY} ${OPENGL_LIBRARY} m)

