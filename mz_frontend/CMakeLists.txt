cmake_minimum_required(VERSION 3.11)
project(mistral-ocs)

set(CMAKE_C_STANDARD 99)

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

add_executable(mistral-ocs
		menu/src/MainMenu.c
		menu/src/Interrogation.c
        menu/src/HelpScreen.c
		menu/src/CreditsScreen.c
        menu/src/Main.c
		menu/src/Engine.c
		base3d/src/SDLVersion/CSDL2Renderer.c
		base3d/src/Globals.c
		base3d/src/LoadBitmap.c
		base3d/src/CRenderer.c
		base3d/src/FixP.c
		base3d/src/CRenderer_Rasterization.c
		base3d/src/CPackedFileReader.c
        menu/src/UI.c
		)


include(FindPkgConfig)

target_include_directories(mistral-ocs PUBLIC ${SDL2_INCLUDE_DIR})
target_link_libraries(mistral-ocs LINK_PUBLIC ${SDL2_LIBRARY} m)

