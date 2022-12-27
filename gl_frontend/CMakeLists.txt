cmake_minimum_required(VERSION 3.11)
project(derelict64)

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

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCLI_BUILD -DINCLUDE_ITEM_DESCRIPTIONS" )

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(derelict64
		menu/src/MainMenu.c
		menu/src/Crawler.c
		menu/src/HelpScreen.c
		menu/src/BattleScreen.c
		menu/src/CreditsScreen.c
		menu/src/GameMenu.c
		menu/src/Main.c
		menu/src/Engine.c
		base3d/src/SDLVersion/CSDL2Renderer.c
		base3d/src/Globals.c
		base3d/src/CTile3DProperties.c
		GL1Common/LoadBitmapGL1.c
		GL1Common/RendererGL1.c
		GL1Common/Renderer_RasterizationGL1.c
		GL1Common/Renderer_TesselationGL1.c
		base3d/src/VisibilityStrategy.c
		../common/src/Vec.c
		../common/src/Common.c
		../common/src/FixP.c
		../common/src/MapWithCharKey.c
		../common/src/PackedFileReader.c
		base3d/src/Events.c
		base3d/src/EDirection_Utils.c
		SoundSystem/NullMusic.c
		base3d/src/Dungeon.c
		menu/src/UI.c
		menu/src/HackingScreen.c
		SoundSystem/PCMAudio.c
		../core/src/Derelict.c
		../core/src/Core.c
		../core/src/Parser.c
		)


include(FindPkgConfig)

target_include_directories(derelict64 PUBLIC ${SDL2_INCLUDE_DIR} )
target_link_libraries(derelict64 LINK_PUBLIC ${SDL2_LIBRARY} ${OPENGL_LIBRARY} m)

