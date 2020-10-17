cmake_minimum_required(VERSION 3.11)
project(opal_standoff)

set(CMAKE_C_STANDARD 11)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_find")

include_directories(
		${SDL2_INCLUDE_DIR}
)

include_directories(base3d/include)
include_directories(common/include)
include_directories(menu/include)
include_directories(SoundSystem)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSDLSW -std=c90 -W -pedantic -Wall")

find_package(SDL2 REQUIRED)

add_executable(opal_standoff
		menu/src/MainMenu.c
		menu/src/Crawler.c
		menu/src/HelpScreen.c
		menu/src/CreditsScreen.c
		menu/src/GameMenu.c
		menu/src/Main.c
		menu/src/Engine.c
		base3d/src/SDLVersion/CSDL2Renderer.c
		base3d/src/Globals.c
		base3d/src/Vec.c
		base3d/src/CTile3DProperties.c
		base3d/src/LoadBitmap.c
		base3d/src/CRenderer.c
		base3d/src/VisibilityStrategy.c
		base3d/src/FixP.c
		base3d/src/Events.c
		base3d/src/CRenderer_Rasterization.c
		base3d/src/CPackedFileReader.c
		base3d/src/EDirection_Utils.c
		base3d/src/CRenderer_Tesselation.c
		SoundSystem/NullMusic.c
		base3d/src/MapWithCharKey.c
		base3d/src/Dungeon.c
		menu/src/UI.c
		SoundSystem/PCMAudio.c
		)


include(FindPkgConfig)

target_include_directories(opal_standoff PUBLIC ${SDL2_INCLUDE_DIR})
target_link_libraries(opal_standoff LINK_PUBLIC ${SDL2_LIBRARY} m)

