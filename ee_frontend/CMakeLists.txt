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

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCLI_BUILD -DINCLUDE_ITEM_DESCRIPTIONS" )

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(derelict128
		../common/src/MainMenu.c
		menu/src/Crawler.c
		../common/src/HelpScreen.c
		../common/src/CreditsScreen.c
		../common/src/GameMenu.c
		menu/src/Main.c
		../common/src/Engine.c
		base3d/src/ee/EmotionEngineRenderer.c
		base3d/src/ee/LoadBitmap.c
		base3d/src/ee/Renderer.c
		base3d/src/ee/Renderer_Rasterization.c
		base3d/src/ee/Renderer_Tesselation.c
		../common/src/VisibilityStrategy.c
		../common/src/Globals.c
		../common/src/CTile3DProperties.c
		../common/src/ArrayPackedFileReader.c
		../common/src/Vec.c
		../common/src/Common.c
		../common/src/FixP.c
		../common/src/MapWithCharKey.c
		../common/src/EDirection_Utils.c
		SoundSystem/NullMusic.c
		../common/src/UI.c
		../common/src/HackingScreen.c
		SoundSystem/PCMAudio.c
		../core/src/Derelict.c
		../core/src/Core.c
		../core/src/Parser.c)


include(FindPkgConfig)

target_include_directories(derelict128 PUBLIC ${SDL2_INCLUDE_DIR} )
target_link_libraries(derelict128 LINK_PUBLIC ${SDL2_LIBRARY} ${OPENGL_LIBRARY} m)

