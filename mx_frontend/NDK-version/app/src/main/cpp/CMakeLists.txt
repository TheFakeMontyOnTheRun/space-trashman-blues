# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_C_STANDARD 11)


include_directories(../../../../../../core/include)
include_directories(../../../../../../common/include)
include_directories(../../../../../base3d/include)
include_directories(../../../../../common/include)
include_directories(../../../../../menu/include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSDLSW -DCLI_BUILD -DINCLUDE_ITEM_DESCRIPTIONS -DANDROID  -D_BSD_SOURCE")


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
        ../../../../../../common/src/MainMenu.c
        ../../../../../../common/src/Crawler.c
        ../../../../../../common/src/HelpScreen.c
        ../../../../../../common/src/CreditsScreen.c
        ../../../../../../common/src/HackingScreen.c
        ../../../../../../common/src/GameMenu.c
        ../../../../../../common/src/Globals.c
        ../../../../../../common/src/Vec.c
        ../../../../../../common/src/Common.c
        ../../../../../../common/src/PackedFileReader.c
        ../../../../../../common/src/MapWithCharKey.c
        ../../../../../../common/src/FixP.c
        ../../../../../../common/src/CTile3DProperties.c
        ../../../../../../common/src/VisibilityStrategy.c
        ../../../../../../common/src/Events.c
        ../../../../../../common/src/EDirection_Utils.c
        ../../../../../../common/src/Dungeon.c
        ../../../../../../common/src/Engine.c
        ../../../../../../common/src/UI.c
        ../../../../../../common/src/PCMAudio.c
        ../../../../../base3d/src/LoadBitmap.c
        ../../../../../base3d/src/CRenderer.c
        ../../../../../base3d/src/CRenderer_Rasterization.c
        ../../../../../base3d/src/CRenderer_Tesselation.c
        ../../../../../menu/src/Main.c
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
