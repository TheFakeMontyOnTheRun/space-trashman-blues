# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

include_directories(../../../../../../core/include)
include_directories(../../../../../base3d/include)
include_directories(../../../../../common/include)
include_directories(../../../../../menu/include)
include_directories(../../../../../SoundSystem)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSDLSW -std=c90 -DANDROID  -D_BSD_SOURCE")


# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ../../../../../../core/src/Derelict.c
        ../../../../../../core/src/Core.c
        ../../../../../../core/src/Parser.c
        ../../../../../menu/src/MainMenu.c
        ../../../../../menu/src/Crawler.c
        ../../../../../menu/src/HelpScreen.c
        ../../../../../menu/src/CreditsScreen.c
        ../../../../../menu/src/HackingScreen.c
        ../../../../../menu/src/BattleScreen.c
        ../../../../../menu/src/GameMenu.c
        ../../../../../menu/src/Main.c
        ../../../../../base3d/src/Globals.c
        ../../../../../base3d/src/Vec.c
        ../../../../../base3d/src/CTile3DProperties.c
        ../../../../../base3d/src/LoadBitmap.c
        ../../../../../base3d/src/CRenderer.c
        ../../../../../base3d/src/VisibilityStrategy.c
        ../../../../../base3d/src/FixP.c
        ../../../../../menu/src/Engine.c
        ../../../../../base3d/src/Events.c
        ../../../../../base3d/src/CRenderer_Rasterization.c
        ../../../../../base3d/src/PackedFileReader.c
        ../../../../../base3d/src/EDirection_Utils.c
        ../../../../../base3d/src/CRenderer_Tesselation.c
        ../../../../../base3d/src/MapWithCharKey.c
        ../../../../../base3d/src/Dungeon.c
        ../../../../../menu/src/UI.c
        ../../../../../SoundSystem/PCMAudio.c
        native-lib.c)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

find_library( # Sets the name of the path variable.
        android-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        android )


# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        native-lib
        ${android-lib}

        android
        jnigraphics
        EGL
        OpenSLES
        GLESv2

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})
