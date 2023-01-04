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
}

void graphicsShutdown() {
	texturesUsed = 0;
}

void flipRenderer() {
}
