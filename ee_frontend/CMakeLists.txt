cmake_minimum_required(VERSION 3.11)
project(derelict128)

set(CMAKE_C_STANDARD 11)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-sdl2")

include_directories(
		${SDL2_INCLUDE_DIR}
)

include_directories(../core/include)
include_directories(../common/include)
include_directories(base3d/include)
include_directories(common/include)
include_directories(menu/include)
include_directories(SoundSystem)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FORTIFY_SOURCE=3 -DCLI_BUILD -DSUPPORTS_HACKING_MINIGAME -DINCLUDE_ITEM_DESCRIPTIONS -DEMIT_QUIT_OPTION -DENDIANESS_AWARE -DTILED_BITMAPS -DSDLGL -DSDL_DISABLE_IMMINTRIN_H  -DXRES_FRAMEBUFFER=320 -DYRES_FRAMEBUFFER=200" )

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIR})

add_executable(derelict128
		menu/src/Main.c
		base3d/src/es/SDL2GL2Version.c
		base3d/src/es/LoadBitmap.c
		base3d/src/Renderer.c
		base3d/src/es/GLESCommon.c
        base3d/src/es/RendererRasterization.c
		base3d/src/es/RendererTesselation.c
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
		../common/src/PCMAudio.c
		../common/src/Matrices.c
		../common/src/HackingMinigameRules.c
		../core/src/Derelict.c
		../core/src/Core.c
		../core/src/Parser.c
)


include(FindPkgConfig)

target_include_directories(derelict128 PUBLIC ${SDL2_INCLUDE_DIR} )
target_link_libraries(derelict128 LINK_PUBLIC ${SDL2_LIBRARY} ${OPENGL_LIBRARY} m)
