#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <libdragon.h>
#include <GL/gl.h>
#include <GL/gl_integration.h>
#include <malloc.h>
#include <math.h>

#include "Common.h"
#include "Enums.h"
#include "FixP.h"
#include "Vec.h"
#include "Globals.h"
#include "Vec.h"
#include "LoadBitmap.h"
#include "CActor.h"
#include "Core.h"
#include "Engine.h"
#include "Dungeon.h"
#include "MapWithCharKey.h"
#include "Mesh.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"

int snapshotSignal = '.';
rdpq_font_t *fnt1;
#define ANGLE_TURN_THRESHOLD 40
#define ANGLE_TURN_STEP 5

extern int turning;
extern int leanX;
extern int leanY;

void graphicsInit(void) {

    debug_init_isviewer();
    debug_init_usblog();

    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, ANTIALIAS_RESAMPLE_FETCH_ALWAYS);

    enableSmoothMovement = TRUE;

    // that is from the N64 SDK...not our own initGL
    gl_init();
    rdpq_init();
    initGL();
    controller_init();
    fnt1 = rdpq_font_load("rom:/font.font64");
    rdpq_text_register_font(1, fnt1);
}

void handleSystemEvents(void) {

    controller_scan();
    struct controller_data pressed = get_keys_pressed();
//	struct controller_data down = get_keys_down();

    switch (get_dpad_direction(0)) {
        case 2:
            mBufferedCommand = kCommandUp;
            break;
        case 0:
            turning = 1;
            leanX = ANGLE_TURN_STEP;
            break;
        case 6:
            mBufferedCommand = kCommandDown;
            break;
        case 4:
            turning = 1;
            leanX = -ANGLE_TURN_STEP;
            break;
    }

    if (pressed.c[0].C_up) {
        mBufferedCommand = kCommandFire1;
    }

    if (pressed.c[0].C_down) {
        mBufferedCommand = kCommandFire2;
    }

    if (pressed.c[0].C_left) {
        mBufferedCommand = kCommandFire3;
    }

    if (pressed.c[0].C_right) {
        mBufferedCommand = kCommandFire4;
    }

    if (pressed.c[0].start) {
        mBufferedCommand = kCommandBack;
    }

    if (pressed.c[0].A) {
        mBufferedCommand = kCommandFire1;
    }

    if (pressed.c[0].B) {
        mBufferedCommand = kCommandFire2;
    }

    if (pressed.c[0].Z) {
        mBufferedCommand = kCommandFire3;
    }

    if (pressed.c[0].L) {
        mBufferedCommand = kCommandStrafeLeft;
    }

    if (pressed.c[0].R) {
        mBufferedCommand = kCommandStrafeRight;
    }

    if (pressed.c[0].x < 0) {
        leanX = -ANGLE_TURN_THRESHOLD;
    }

    if (pressed.c[0].x > 0) {
        leanX = ANGLE_TURN_THRESHOLD;
    }

    if (pressed.c[0].y < 0) {
        leanY = ANGLE_TURN_THRESHOLD;
    }

    if (pressed.c[0].y > 0) {
        leanY = -ANGLE_TURN_THRESHOLD;
    }
}

void graphicsShutdown(void) {
    texturesUsed = 0;
}

void flipRenderer(void) {
}
