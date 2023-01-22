#include <stddef.h>
#include <stdint.h>
#include <string.h>
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
#include "CTile3DProperties.h"
#include "CRenderer.h"

int snapshotSignal = '.';

void graphicsInit() {

	enableSmoothMovement = TRUE;

	initGL();
}

void handleSystemEvents() {


	uint8_t entry = 0;

	switch(entry) {
		case 2:
			mBufferedCommand = kCommandUp;
			break;
		case 0:
			mBufferedCommand = kCommandRight;
			break;
		case 6:
			mBufferedCommand = kCommandDown;
			break;
		case 4:
			mBufferedCommand = kCommandLeft;
			break;
		case -1:
			mBufferedCommand = kCommandNone;
			break;
	}
//
//	if (pressed.c[0].C_up) {
//		mBufferedCommand = kCommandFire1;
//	}
//
//	if (pressed.c[0].C_down) {
//		mBufferedCommand = kCommandFire2;
//	}
//
//	if (pressed.c[0].C_left) {
//		mBufferedCommand = kCommandFire3;
//	}
//
//	if (pressed.c[0].C_right) {
//		mBufferedCommand = kCommandFire4;
//	}
//
//	if (pressed.c[0].start) {
//		mBufferedCommand = kCommandBack;
//	}
//
//	if (pressed.c[0].A) {
//		mBufferedCommand = kCommandFire1;
//	}
//
//	if (pressed.c[0].B) {
//		mBufferedCommand = kCommandFire2;
//	}
//
//	if (pressed.c[0].Z) {
//		mBufferedCommand = kCommandFire3;
//	}
//
//	if (pressed.c[0].L) {
//		mBufferedCommand = kCommandStrafeLeft;
//	}
//
//	if (pressed.c[0].R) {
//		mBufferedCommand = kCommandStrafeRight;
//	}
}

void graphicsShutdown() {
	texturesUsed = 0;
}

void flipRenderer() {
}
