#include <jni.h>
#include <string.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "Common.h"
#include "Enums.h"
#include "FixP.h"
#include "Vec.h"
#include "Vec.h"
#include "LoadBitmap.h"
#include "CActor.h"
#include "Core.h"
#include "Engine.h"
#include "Dungeon.h"
#include "MapWithCharKey.h"
#include "CTile3DProperties.h"
#include "Renderer.h"
#include "FixP.h"
#include "Vec.h"
#include "Enums.h"
#include "CActor.h"
#include "MapWithCharKey.h"
#include "Common.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "CTile3DProperties.h"
#include "Renderer.h"
#include "VisibilityStrategy.h"
#include "PackedFileReader.h"
#include "SoundSystem.h"

int width = 640;
int height = 480;
#define ANGLE_TURN_THRESHOLD 40
#define ANGLE_TURN_STEP 5

extern int turning;

enum ESoundDriver soundDriver = kNoSound;

int isInstantApp = FALSE;

void enterState(enum EGameMenuState newState);

void mainLoop(void);

extern int currentGameMenuState;

int enable3DRendering = FALSE;

AAssetManager *defaultAssetManager = NULL;

void graphicsInit(void) {

    enableSmoothMovement = TRUE;
    defaultFont = NULL;
}

void handleSystemEvents(void) {

}

void graphicsShutdown(void) {
    releaseBitmap(defaultFont);
    texturesUsed = 0;
}

void flipRenderer(void) {

}


JNIEXPORT void JNICALL
Java_pt_b13h_derelictgles2_DerelictJNI_initAssets(JNIEnv *env, jclass clazz,
                                                       jobject assetManager) {

    AAssetManager *asset_manager = AAssetManager_fromJava(env, assetManager);
    defaultAssetManager = asset_manager;
    initHW(0, NULL);
    enableSmoothMovement = TRUE;
    enterState(kPlayGame);
}

int soundToPlay = -1;

void stopSounds(void) {}

void playSound(const int action) {
    soundToPlay = action;
}

void soundTick(void) {}

void muteSound(void) {}

JNIEXPORT jint JNICALL
Java_pt_b13h_derelictgles2_DerelictJNI_getSoundToPlay(JNIEnv *env, jclass clazz) {
    int toReturn = soundToPlay;
    soundToPlay = -1;
    return toReturn;
}

JNIEXPORT jint JNICALL
Java_pt_b13h_derelictgles2_DerelictJNI_isOnMainMenu(JNIEnv *env, jclass clazz) {
    return currentGameMenuState == (kMainMenu);
}

JNIEXPORT void JNICALL
Java_pt_b13h_derelictgles2_DerelictJNI_sendCommand(JNIEnv *env, jclass clazz, jchar cmd) {
    switch (cmd) {
        case 'w':
            mBufferedCommand = kCommandUp;
            break;

        case 's':
            mBufferedCommand = kCommandDown;
            break;

        case 'z':
            mBufferedCommand = kCommandFire1;
            break;


        case 'x':
            mBufferedCommand = kCommandFire2;
            break;

        case 'c':
            mBufferedCommand = kCommandFire3;
            break;

        case 'v':
            mBufferedCommand = kCommandFire4;
            break;

        case 'a':
            turning = 1;
            leanX = -ANGLE_TURN_STEP;
            break;

        case 'd':
            turning = 1;
            leanX = ANGLE_TURN_STEP;
            break;

        case 'q':
            mBufferedCommand = kCommandBack;
            break;

        case 'n':
            mBufferedCommand = kCommandStrafeLeft;
            break;

        case 'm':
            mBufferedCommand = kCommandStrafeRight;
            break;
    }
}

JNIEXPORT void JNICALL
Java_pt_b13h_derelictgles2_DerelictJNI_init(JNIEnv *env, jclass clazz, jint jwidth, jint jheight) {
    width = jwidth;
    height = jheight;

    initGL();

    enableSmoothMovement = TRUE;
    enable3DRendering = TRUE;
}

JNIEXPORT void JNICALL
Java_pt_b13h_derelictgles2_DerelictJNI_drawFrame(JNIEnv *env, jclass clazz) {
    if (enable3DRendering) {
        startFrame(0, 0, 640, 480);

        isRunning = isRunning && menuTick(10);

        endFrame();
        flipRenderer();
    }
}

JNIEXPORT void JNICALL
Java_pt_b13h_derelictgles2_DerelictJNI_onDestroy(JNIEnv *env, jclass clazz) {
    enable3DRendering = FALSE;
    unloadTextures();
}