#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#ifdef N64
#include <libdragon.h>
#endif

#ifndef NDS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION

#include <OpenGL/gl.h>

#else
#include <GL/gl.h>
#endif
#else
#include <nds.h>
#include <malloc.h>
#include <stdio.h>
#include <nds/arm9/image.h>
#include <nds/arm9/trig_lut.h>
#endif

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

#define BIAS (intToFix(128))
#define REVERSE_BIAS (1.0f/128.0f)

void clearTextures() {
	usedTexture = 0;
}


struct Texture *makeTextureFrom(const char *filename) {
	struct Texture *toReturn =
#ifndef N64
			(struct Texture *) calloc(1, sizeof(struct Texture));
#else
	(struct Texture *) malloc_uncached(sizeof(struct Texture));
	memset(toReturn, 0, sizeof(struct Texture));
#endif
	toReturn->raw = loadBitmap(filename);
	toReturn->raw->uploadId = submitBitmapToGPU(toReturn->raw);
#ifndef NDS
	glBindTexture(GL_TEXTURE_2D, toReturn->raw->uploadId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif
	return toReturn;
}

void drawRampAt(const struct Vec3 p0, const struct Vec3 p1,
				const struct Texture *texture, uint8_t direction, uint8_t flipTexture) {

	float centerY0, centerY1;
	FixP_t acc;
	FixP_t scaled;
	float geometryScale;
	float centerY;

	acc = (p0.mY + playerHeight + walkingBias + yCameraOffset);
	scaled = Mul(acc, BIAS);
	centerY1 = (fixToInt(scaled) * REVERSE_BIAS);

	acc = (p1.mY + playerHeight + walkingBias + yCameraOffset);
	scaled = Mul(acc, BIAS);
	centerY0 = (fixToInt(scaled) * REVERSE_BIAS);

	geometryScale = (centerY1 - centerY0);
	centerY = centerY0 + (centerY1 - centerY0) * 0.5f;

	glTranslatef(0.0f, centerY, 0.0f);
	glScalef(1.0f, geometryScale, 1.0f);

	glBindTexture(GL_TEXTURE_2D, texture->raw->uploadId);
	glBegin(GL_QUADS);

	glTexCoord2f(0, 1);

	switch (direction) {
		case kNorth:
			glVertex3f(-1, -0.5f, -1);
			glTexCoord2f(1, 1);
			glVertex3f(1, -0.5f, -1);
			glTexCoord2f(1, 0);
			glVertex3f(1, 0.5f, 1);
			glTexCoord2f(0, 0);
			glVertex3f(-1, 0.5f, 1);
			break;
		case kSouth:
			glVertex3f(-1, 0.5f, -1);
			glTexCoord2f(1, 1);
			glVertex3f(1, 0.5f, -1);
			glTexCoord2f(1, 0);
			glVertex3f(1, -0.5f, 1);
			glTexCoord2f(0, 0);
			glVertex3f(-1, -0.5f, 1);
			break;
		case kEast:
			glVertex3f(-1, -0.5f, -1);
			glTexCoord2f(1, 1);
			glVertex3f(1, 0.5f, -1);
			glTexCoord2f(1, 0);
			glVertex3f(1, 0.5f, 1);
			glTexCoord2f(0, 0);
			glVertex3f(-1, -0.5f, 1);
			break;
		case kWest:
			glVertex3f(-1, 0.5f, -1);
			glTexCoord2f(1, 1);
			glVertex3f(1, -0.5f, -1);
			glTexCoord2f(1, 0);
			glVertex3f(1, -0.5f, 1);
			glTexCoord2f(0, 0);
			glVertex3f(-1, 0.5f, 1);
			break;
	}

	glEnd();

	glScalef(1.0f, 1.0f / geometryScale, 1.0f);
	glTranslatef(0.0f, -centerY, 0.0f);

}

void drawBillboardAt(const struct Vec3 center,
					 struct Texture *bitmap,
					 const FixP_t scale,
					 const int size) {
	struct Vec3 scaledCenter;
	float centerY;
	FixP_t acc;
	FixP_t scaled;
	float textureScale;
	float geometryScale;

	if (center.mZ <= kMinZCull) {
		return;
	}

	initVec3(&scaledCenter, center.mX, (center.mY), center.mZ);

        scaled = Mul(scale, BIAS);
	textureScale = (fixToInt(scaled) * REVERSE_BIAS);
	geometryScale = textureScale;


	acc = (center.mY + playerHeight + walkingBias + yCameraOffset);
	scaled = Mul(acc, BIAS);
	centerY = (fixToInt(scaled) * REVERSE_BIAS);

	glBindTexture(GL_TEXTURE_2D, bitmap->raw->uploadId);

	glTranslatef(0.0f, centerY, 0.0f);
	glScalef(1.0f, geometryScale, 1.0f);

	glEnable(GL_ALPHA_TEST);

	glBegin(GL_QUADS);

	glTexCoord2f(0, 1);
	glVertex3f(-1, -1, 1);
	glTexCoord2f(1, 1);
	glVertex3f(1, -1, 1);
	glTexCoord2f(1, 0);
	glVertex3f(1, 1, 1);
	glTexCoord2f(0, 0);
	glVertex3f(-1, 1, 1);

	glEnd();
	glDisable(GL_ALPHA_TEST);
	glTranslatef(0.0f, -centerY, 0.0f);
	glScalef(1.0f, 1.0f / geometryScale, 1.0f);

}

void drawColumnAt(const struct Vec3 center,
				  const FixP_t scale,
				  const struct Texture *texture,
				  const uint8_t mask,
				  const uint8_t enableAlpha,
				  const uint8_t repeatTexture) {

	float centerY;
	FixP_t acc;
	FixP_t scaled = Mul(scale, BIAS);
	float textureScale = (fixToInt(scaled) * REVERSE_BIAS);
	float geometryScale = textureScale;

	if (!repeatTexture) {
		textureScale = 1;
	}

	acc = (center.mY + playerHeight + walkingBias + yCameraOffset);
	scaled = Mul(acc, BIAS);
	centerY = (fixToInt(scaled) * REVERSE_BIAS);

	glBindTexture(GL_TEXTURE_2D, texture->raw->uploadId);

	if (enableAlpha) {
		glEnable(GL_ALPHA_TEST);
	}
	glTranslatef(0.0f, centerY, 0.0f);
	glScalef(1.0f, geometryScale, 1.0f);
	glBegin(GL_QUADS);
	if ((mask & MASK_BEHIND)) {
		glTexCoord2f(0, textureScale);
		glVertex3f(-1, -1, -1);
		glTexCoord2f(1, textureScale);
		glVertex3f(1, -1, -1);
		glTexCoord2f(1, 0);
		glVertex3f(1, 1, -1);
		glTexCoord2f(0, 0);
		glVertex3f(-1, 1, -1);
	}

	if (((mask & MASK_RIGHT) && fixToInt(center.mX) > 0) || (mask & MASK_FORCE_RIGHT)) {
		glTexCoord2f(0, textureScale);
		glVertex3f(-1, -1, -1);
		glTexCoord2f(1, textureScale);
		glVertex3f(-1, -1, 1);
		glTexCoord2f(1, 0);
		glVertex3f(-1, 1, 1);
		glTexCoord2f(0, 0);
		glVertex3f(-1, 1, -1);
	}

	if (((mask & MASK_LEFT) && fixToInt(center.mX) < 0) || (mask & MASK_FORCE_LEFT)) {
		glTexCoord2f(0, textureScale);
		glVertex3f(1, -1, -1);
		glTexCoord2f(1, textureScale);
		glVertex3f(1, -1, 1);
		glTexCoord2f(1, 0);
		glVertex3f(1, 1, 1);
		glTexCoord2f(0, 0);
		glVertex3f(1, 1, -1);
	}

	if ((mask & MASK_FRONT)) {
		glTexCoord2f(0, textureScale);
		glVertex3f(-1, -1, 1);
		glTexCoord2f(1, textureScale);
		glVertex3f(1, -1, 1);
		glTexCoord2f(1, 0);
		glVertex3f(1, 1, 1);
		glTexCoord2f(0, 0);
		glVertex3f(-1, 1, 1);
	}
	glEnd();
	glScalef(1.0f, 1.0f / (geometryScale), 1.0f);
	glTranslatef(0.0f, -centerY, 0.0f);
	glDisable(GL_ALPHA_TEST);
}

void drawFloorAt(const struct Vec3 center,
				 const struct Texture *texture, enum EDirection cameraDirection) {

	if (center.mY <= 0) {

		float centerY;
		FixP_t acc;
		FixP_t scaled;
		int x[4], y[4];

		acc = center.mY + playerHeight + walkingBias + yCameraOffset;
		scaled = Mul(acc, BIAS);
		centerY = (fixToInt(scaled) * REVERSE_BIAS);


		switch (cameraDirection) {
			case kNorth:
				x[0] = 0;
				y[0] = 1;
				x[1] = 1;
				y[1] = 1;
				x[2] = 1;
				y[2] = 0;
				x[3] = 0;
				y[3] = 0;
				break;
			case kSouth:
				x[0] = 1;
				y[0] = 0;
				x[1] = 0;
				y[1] = 0;
				x[2] = 0;
				y[2] = 1;
				x[3] = 1;
				y[3] = 1;
				break;
			case kWest:
				x[0] = 0;
				y[0] = 0;
				x[1] = 0;
				y[1] = 1;
				x[2] = 1;
				y[2] = 1;
				x[3] = 1;
				y[3] = 0;
				break;
			case kEast:
			default:
				x[0] = 1;
				y[0] = 1;
				x[1] = 1;
				y[1] = 0;
				x[2] = 0;
				y[2] = 0;
				x[3] = 0;
				y[3] = 1;
				break;
		}

		glTranslatef(0, centerY, 0);
		glBindTexture(GL_TEXTURE_2D, texture->raw->uploadId);
		glBegin(GL_QUADS);

		glTexCoord2f(x[0], y[0]);
		glVertex3f(-1, 0, -1);
		glTexCoord2f(x[1], y[1]);
		glVertex3f(1, 0, -1);
		glTexCoord2f(x[2], y[2]);
		glVertex3f(1, 0, 1);
		glTexCoord2f(x[3], y[3]);
		glVertex3f(-1, 0, 1);

		glEnd();
		glTranslatef(0, -centerY, 0);
	}
}

void drawCeilingAt(const struct Vec3 center,
				   const struct Texture *texture, enum EDirection cameraDirection) {

	if (center.mY >= 0) {
		float centerY;
		FixP_t acc;
		FixP_t scaled;
		int x[4], y[4];

		acc = (center.mY + playerHeight + walkingBias + yCameraOffset);
		scaled = Mul(acc, BIAS);
		centerY = (fixToInt(scaled) * REVERSE_BIAS);

		switch (cameraDirection) {
			case kNorth:
				x[0] = 0;
				y[0] = 1;
				x[1] = 1;
				y[1] = 1;
				x[2] = 1;
				y[2] = 0;
				x[3] = 0;
				y[3] = 0;
				break;
			case kSouth:
				x[0] = 1;
				y[0] = 0;
				x[1] = 0;
				y[1] = 0;
				x[2] = 0;
				y[2] = 1;
				x[3] = 1;
				y[3] = 1;
				break;
			case kWest:
				x[0] = 0;
				y[0] = 0;
				x[1] = 0;
				y[1] = 1;
				x[2] = 1;
				y[2] = 1;
				x[3] = 1;
				y[3] = 0;
				break;
			default:
			case kEast:
				x[0] = 1;
				y[0] = 1;
				x[1] = 1;
				y[1] = 0;
				x[2] = 0;
				y[2] = 0;
				x[3] = 0;
				y[3] = 1;
				break;
		}

		glTranslatef(0, centerY, 0);
		glBindTexture(GL_TEXTURE_2D, texture->raw->uploadId);
		glBegin(GL_QUADS);

		glTexCoord2f(x[0], y[0]);
		glVertex3f(-1, 0, -1);
		glTexCoord2f(x[1], y[1]);
		glVertex3f(1, 0, -1);
		glTexCoord2f(x[2], y[2]);
		glVertex3f(1, 0, 1);
		glTexCoord2f(x[3], y[3]);
		glVertex3f(-1, 0, 1);
		glEnd();
		glTranslatef(0, -centerY, 0);
	}
}

void drawLeftNear(const struct Vec3 center,
				  const FixP_t scale,
				  const struct Texture *texture,
				  const uint8_t mask,
				  const uint8_t repeatTexture) {


	FixP_t acc;
	FixP_t scaled;
	float textureScale;
	float geometryScale;
	float centerY;
	scaled = Mul(scale, BIAS);
	textureScale = (fixToInt(scaled) * REVERSE_BIAS);
	geometryScale = textureScale;

	acc = (center.mY + playerHeight + walkingBias + yCameraOffset);
	scaled = Mul(acc, BIAS);
	centerY = (fixToInt(scaled) * REVERSE_BIAS);

	glTranslatef(0.0f, centerY, 0.0f);
	glScalef(1.0f, geometryScale, 1.0f);

	glBindTexture(GL_TEXTURE_2D, texture->raw->uploadId);
	glBegin(GL_QUADS);

	if (cameraDirection == kWest || cameraDirection == kEast) {
		glTexCoord2f(0, textureScale);
		glVertex3f(-1, -1, -1);
		glTexCoord2f(1, textureScale);
		glVertex3f(1, -1, 1);
		glTexCoord2f(1, 0);
		glVertex3f(1, 1, 1);
		glTexCoord2f(0, 0);
		glVertex3f(-1, 1, -1);
	} else {
		glTexCoord2f(0, textureScale);
		glVertex3f(-1, -1, 1);
		glTexCoord2f(1, textureScale);
		glVertex3f(1, -1, -1);
		glTexCoord2f(1, 0);
		glVertex3f(1, 1, -1);
		glTexCoord2f(0, 0);
		glVertex3f(-1, 1, 1);
	}
	glEnd();

	glScalef(1.0f, 1.0f / geometryScale, 1.0f);
	glTranslatef(0.0f, -centerY, 0.0f);
}

void drawRightNear(const struct Vec3 center,
				   const FixP_t scale,
				   const struct Texture *texture,
				   const uint8_t mask,
				   const uint8_t repeatTexture) {

	float centerY;
	FixP_t acc;
	FixP_t scaled;
	float textureScale;
	float geometryScale;
	scaled = Mul(scale, BIAS);
        textureScale = (fixToInt(scaled) * REVERSE_BIAS);
	geometryScale = textureScale;


	acc = (center.mY + playerHeight + walkingBias + yCameraOffset);
	scaled = Mul(acc, BIAS);
	centerY = (fixToInt(scaled) * REVERSE_BIAS);

	glTranslatef(0.0f, centerY, 0.0f);
	glScalef(1.0f, geometryScale, 1.0f);

	glBindTexture(GL_TEXTURE_2D, texture->raw->uploadId);
	glBegin(GL_QUADS);


	if (cameraDirection == kWest || cameraDirection == kEast) {
		glTexCoord2f(0, textureScale);
		glVertex3f(-1, -1, 1);
		glTexCoord2f(1, textureScale);
		glVertex3f(1, -1, -1);
		glTexCoord2f(1, 0);
		glVertex3f(1, 1, -1);
		glTexCoord2f(0, 0);
		glVertex3f(-1, 1, 1);
	} else {
		glTexCoord2f(0, textureScale);
		glVertex3f(-1, -1, -1);
		glTexCoord2f(1, textureScale);
		glVertex3f(1, -1, 1);
		glTexCoord2f(1, 0);
		glVertex3f(1, 1, 1);
		glTexCoord2f(0, 0);
		glVertex3f(-1, 1, -1);
	}
	glEnd();

	glScalef(1.0f, 1.0f / geometryScale, 1.0f);
	glTranslatef(0.0f, -centerY, 0.0f);
}