#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "Core.h"
#include "Common.h"
#include "FixP.h"
#include "Enums.h"
#include "Vec.h"
#include "CActor.h"
#include "Dungeon.h"
#include "MapWithCharKey.h"
#include "Vec.h"
#include "LoadBitmap.h"
#include "MapWithCharKey.h"
#include "CTile3DProperties.h"
#include "LoadBitmap.h"
#include "CRenderer.h"
#include "Engine.h"
#include "VisibilityStrategy.h"
#include "PackedFileReader.h"

#define kMinZCull 0
struct Vec3 cameraOffset;
FixP_t walkingBias = 0;
FixP_t playerHeight = 0;

struct Texture *nativeTextures[TOTAL_TEXTURES];
int usedTexture = 0;

void clearTextures() {
    usedTexture = 0;
}


struct Texture *makeTextureFrom(const char *filename) {
	struct Texture *toReturn =
	(struct Texture *) calloc(1, sizeof(struct Texture));

	return toReturn;
}

void drawRampAt(const struct Vec3 p0, const struct Vec3 p1,
				const struct Texture *texture, uint8_t direction, uint8_t flipTexture) {
}

void drawBillboardAt(const struct Vec3 center,
					 struct Texture *bitmap,
					 const FixP_t scale,
					 const int size) {
}

void drawColumnAt(const struct Vec3 center,
				  const FixP_t scale,
				  const struct Texture *texture,
				  const uint8_t mask,
				  const uint8_t enableAlpha,
				  const uint8_t repeatTexture) {
}

void drawFloorAt(const struct Vec3 center,
				 const struct Texture *texture, enum EDirection cameraDirection) {
}

void drawCeilingAt(const struct Vec3 center,
				   const struct Texture *texture, enum EDirection cameraDirection) {
}

void drawLeftNear(const struct Vec3 center,
				  const FixP_t scale,
				  const struct Texture *texture,
				  const uint8_t mask,
				  const uint8_t repeatTexture) {
}

void drawRightNear(const struct Vec3 center,
				   const FixP_t scale,
				   const struct Texture *texture,
				   const uint8_t mask,
				   const uint8_t repeatTexture) {
}
