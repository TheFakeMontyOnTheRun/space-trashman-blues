cmake_minimum_required(VERSION 3.11)
project(derelict32)

set(CMAKE_C_STANDARD 90)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-sdl2")

include_directories(
		${SDL2_INCLUDE_DIR}
)

include_directories(../core/include)
include_directories(../common/include)
include_directories(base3d/include)
include_directories(common/include)
include_directories(menu/include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSDLSW -DCLI_BUILD -DPAGE_FLIP_ANIMATION -DINCLUDE_ITEM_DESCRIPTIONS -Wall -Wextra -pedantic -g" )

find_package(SDL2 REQUIRED)

add_executable(derelict32
		../common/src/Crawler.c
		../common/src/MainMenu.c
		../common/src/HelpScreen.c
		../common/src/CreditsScreen.c
		../common/src/GameMenu.c
		menu/src/Main.c
		../common/src/Engine.c
		base3d/src/SDLVersion/CSDL2Renderer.c
		base3d/src/LoadBitmap.c
		base3d/src/CRenderer.c
		../common/src/VisibilityStrategy.c
		../common/src/Globals.c
		../common/src/CTile3DProperties.c
		../common/src/Vec.c
		../common/src/Common.c
		../common/src/FixP.c
		../common/src/MapWithCharKey.c
		../common/src/PackedFileReader.c
		../common/src/EDirection_Utils.c
		../common/src/Dungeon.c
		../common/src/Events.c
		base3d/src/CRenderer_Rasterization.c
		base3d/src/CRenderer_Tesselation.c
		../common/src/NullMusic.c
		../common/src/UI.c
		../common/src/HackingScreen.c
		../common/src/PCMAudio.c
		../core/src/Derelict.c
		../core/src/Core.c
		../core/src/Parser.c
		)


include(FindPkgConfig)

target_include_directories(derelict32 PUBLIC ${SDL2_INCLUDE_DIR})
target_link_libraries(derelict32 LINK_PUBLIC ${SDL2_LIBRARY} m)

