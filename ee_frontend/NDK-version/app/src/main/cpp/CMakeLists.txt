# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_C_STANDARD 11)



include_directories(../../../../../../core/include)
include_directories(../../../../../../common/include)
include_directories(../../../../../base3d/include)
include_directories(../../../../../common/include)
include_directories(../../../../../menu/include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSDLSW -DCLI_BUILD -DINCLUDE_ITEM_DESCRIPTIONS -DANDROID -DENDIANESS_AWARE -DTILED_BITMAPS -D_BSD_SOURCE -DXRES_FRAMEBUFFER=320 -DYRES_FRAMEBUFFER=200")


# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${CMAKE_PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.
add_library(native-lib SHARED
        ../../../../../../core/src/Derelict.c
        ../../../../../../core/src/Core.c
        ../../../../../../core/src/Parser.c
        ../../../../../../common/src/MainMenu.c
        ../../../../../../common/src/HackingMinigameRules.c
        ../../../../../../common/src/Mesh.c
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
        ../../../../../../common/src/Matrices.c
        ../../../../../base3d/src/es/LoadBitmap.c
        ../../../../../base3d/src/Renderer.c
        ../../../../../base3d/src/es/GLESCommon.c
        ../../../../../base3d/src/es/Renderer_Rasterization.c
        ../../../../../base3d/src/es/Renderer_Tesselation.c
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

target_link_libraries(
        native-lib
        android
        jnigraphics
        EGL
        OpenSLES
        GLESv2
        log)
