#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

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
	glBindTexture(GL_TEXTURE_2D, toReturn->raw->uploadId);
#ifndef NDS
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
#endif
	glBindTexture(GL_TEXTURE_2D, 0);
	return toReturn;
}

void drawRampAt(const struct Vec3 p0, const struct Vec3 p1,
				const struct Texture *texture, uint8_t direction, uint8_t flipTexture) {

	float centerX, centerY0, centerY1, centerZ;
	centerX = fixToInt(p0.mX + xCameraOffset);
	centerZ = -fixToInt(p0.mZ + zCameraOffset);
	FixP_t acc;
	FixP_t bias = intToFix(128);
	FixP_t scaled;

	acc = (p0.mY + playerHeight + walkingBias + yCameraOffset);
	scaled = Mul( acc, bias );
	centerY1 = (fixToInt(scaled) / 128.0f);


	acc = (p1.mY + playerHeight + walkingBias + yCameraOffset);
	scaled = Mul( acc, bias );
	centerY0 = (fixToInt(scaled) / 128.0f);

	glPushMatrix();
	glTranslatef(centerX, 0.0f, centerZ);

	glBindTexture(GL_TEXTURE_2D, texture->raw->uploadId);
	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 1.0f);

	switch (direction) {
		case kNorth:
			glVertex3f(- 1.0f, centerY0, - 1.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(+ 1.0f, centerY0, - 1.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(+ 1.0f, centerY1, + 1.0f);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(- 1.0f, centerY1, + 1.0f);
			break;
		case kSouth:
			glVertex3f(- 1.0f, centerY1, - 1.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(+ 1.0f, centerY1, - 1.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(+ 1.0f, centerY0, + 1.0f);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(- 1.0f, centerY0, + 1.0f);
			break;
		case kEast:
			glVertex3f(- 1.0f, centerY0, - 1.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(+ 1.0f, centerY1, - 1.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(+ 1.0f, centerY1, + 1.0f);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(- 1.0f, centerY0, + 1.0f);
			break;
		case kWest:
			glVertex3f(- 1.0f, centerY1, - 1.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(+ 1.0f, centerY0, - 1.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(+ 1.0f, centerY0, + 1.0f);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(- 1.0f, centerY1, + 1.0f);
			break;
	}

#ifndef NDS
	glPopMatrix();
#else
	glPopMatrix(1);
#endif

	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void drawBillboardAt(const struct Vec3 center,
					 struct Texture *bitmap,
					 const FixP_t scale,
					 const int size) {
	FixP_t one = intToFix(1);
	struct Vec3 scaledCenter;

	if (center.mZ <= kMinZCull) {
		return;
	}

	initVec3(&scaledCenter, center.mX, (center.mY), center.mZ);

    float centerX, centerY, centerZ;
	FixP_t acc;
	FixP_t bias = intToFix(128);
	FixP_t scaled = Mul( scale, bias );
	float textureScale = (fixToInt(scaled) / 128.0f);
	float geometryScale = textureScale;


    centerX = fixToInt(center.mX + xCameraOffset);


	acc = (center.mY + playerHeight + walkingBias + yCameraOffset);
	scaled = Mul( acc, bias );
	centerY = (fixToInt(scaled) / 128.0f);

    centerZ = -fixToInt(center.mZ + zCameraOffset);

	glPushMatrix();
	glTranslatef(centerX, 0.0f, centerZ);

    glBindTexture(GL_TEXTURE_2D, bitmap->raw->uploadId);

    glEnable(GL_ALPHA_TEST);

    glBegin(GL_QUADS);

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(- 1.0f, centerY - geometryScale, + 1.0f);
    glTexCoord2f(1, 1.0f);
    glVertex3f(+ 1.0f, centerY - geometryScale, + 1.0f);
    glTexCoord2f(1, 0.0f);
    glVertex3f(+ 1.0f, centerY + geometryScale, + 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(- 1.0f, centerY + geometryScale, + 1.0f);

    glEnd();
    glDisable(GL_ALPHA_TEST);
	glBindTexture(GL_TEXTURE_2D, 0);
#ifndef NDS
	glPopMatrix();
#else
	glPopMatrix(1);
#endif
}

void drawColumnAt(const struct Vec3 center,
				  const FixP_t scale,
				  const struct Texture *texture,
				  const uint8_t mask,
				  const uint8_t enableAlpha,
				  const uint8_t repeatTexture) {

    float centerX, centerY, centerZ;
	FixP_t acc;
	FixP_t bias = intToFix(128);
	FixP_t scaled = Mul( scale, bias );
    float textureScale = (fixToInt(scaled) / 128.0f);
    float geometryScale = textureScale;
	textureScale = textureScale;
    
    if (!repeatTexture) {
        textureScale = 1.0f;
    }
    
    centerX = fixToInt(center.mX + xCameraOffset);

	acc = (center.mY + playerHeight + walkingBias + yCameraOffset);
	scaled = Mul( acc, bias );
	centerY = (fixToInt(scaled) / 128.0f);

    centerZ = -fixToInt(center.mZ + zCameraOffset);

	glPushMatrix();
	glTranslatef(centerX, 0.0f, centerZ);

    glBindTexture(GL_TEXTURE_2D, texture->raw->uploadId);

    if (enableAlpha) {
        glEnable(GL_ALPHA_TEST);
    }
    
    glBegin(GL_QUADS);
	if ((mask & MASK_BEHIND)) {
        glTexCoord2f(0, textureScale);
        glVertex3f(- 1.0f, centerY - geometryScale, - 1.0f);
        glTexCoord2f(1.0f, textureScale);
        glVertex3f(+ 1.0f, centerY - geometryScale, - 1.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(+ 1.0f, centerY + geometryScale, - 1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(- 1.0f, centerY + geometryScale, - 1.0f);
    }

    if (((mask & MASK_RIGHT) && fixToInt(center.mX) > 0 ) || (mask & MASK_FORCE_RIGHT)) {
        glTexCoord2f(0, textureScale);
        glVertex3f(centerX - 1.0f, centerY - geometryScale, - 1.0f);
        glTexCoord2f(1.0f, textureScale);
        glVertex3f(centerX - 1.0f, centerY - geometryScale, + 1.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(centerX - 1.0f, centerY + geometryScale, + 1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(centerX - 1.0f, centerY + geometryScale, - 1.0f);
    }

    if (((mask & MASK_LEFT) && fixToInt(center.mX) < 0 ) || (mask & MASK_FORCE_LEFT)) {
        glTexCoord2f(0, textureScale);
        glVertex3f(+ 1.0f, centerY - geometryScale, - 1.0f);
        glTexCoord2f(1.0f, textureScale);
        glVertex3f(+ 1.0f, centerY - geometryScale, + 1.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(+ 1.0f, centerY + geometryScale, + 1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(+ 1.0f, centerY + geometryScale, - 1.0f);
    }

	if ((mask & MASK_FRONT)) {
        glTexCoord2f(0, textureScale);
        glVertex3f(- 1.0f, centerY - geometryScale, + 1.0f);
        glTexCoord2f(1.0f, textureScale);
        glVertex3f(+ 1.0f, centerY - geometryScale, + 1.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(+ 1.0f, centerY + geometryScale, + 1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(- 1.0f, centerY + geometryScale, + 1.0f);
    }

    glEnd();
    glDisable(GL_ALPHA_TEST);
#ifndef NDS
	glPopMatrix();
#else
	glPopMatrix(1);
#endif
}

void drawFloorAt(const struct Vec3 center,
				 const struct Texture *texture, enum EDirection cameraDirection) {
    
    float centerX, centerY, centerZ;
	FixP_t acc;
	FixP_t bias = intToFix(128);
	FixP_t scaled;

	acc = center.mY + playerHeight + walkingBias + yCameraOffset;
	scaled = Mul( acc, bias );
	centerY = (fixToInt(scaled) / 128.0f);

	centerX = fixToInt(center.mX + xCameraOffset);
    centerZ = -fixToInt(center.mZ + zCameraOffset);


	int x[4], y[4];

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


    if (center.mY <= 0) {
		glPushMatrix();
		glTranslatef(centerX, 0.0f, centerZ);
        glBindTexture(GL_TEXTURE_2D, texture->raw->uploadId);
        glBegin(GL_QUADS);
        
        glTexCoord2f(x[0], y[0]);
        glVertex3f(- 1.0f, centerY, - 1.0f);
		glTexCoord2f(x[1], y[1]);
        glVertex3f(+ 1.0f, centerY, - 1.0f);
		glTexCoord2f(x[2], y[2]);
        glVertex3f(+ 1.0f, centerY, + 1.0f);
		glTexCoord2f(x[3], y[3]);
        
        glVertex3f(- 1.0f, centerY, + 1.0f);
        glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);
#ifndef NDS
		glPopMatrix();
#else
		glPopMatrix(1);
#endif
    }
}

void drawCeilingAt(const struct Vec3 center,
				   const struct Texture *texture, enum EDirection cameraDirection) {

    float centerX, centerY, centerZ;
	FixP_t acc;
	FixP_t bias = intToFix(128);
	FixP_t scaled;

	acc = (center.mY + playerHeight + walkingBias + yCameraOffset);
	scaled = Mul( acc, bias );
	centerY = (fixToInt(scaled) / 128.0f);

    centerX = fixToInt(center.mX+ xCameraOffset);
    centerZ = -fixToInt(center.mZ+ zCameraOffset);


	int x[4], y[4];

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

    if (center.mY >= 0) {
		glPushMatrix();
		glTranslatef(centerX, 0.0f, centerZ);

        glBindTexture(GL_TEXTURE_2D, texture->raw->uploadId);
        glBegin(GL_QUADS);

		glTexCoord2f(x[0], y[0]);
        glVertex3f(- 1.0f, centerY, - 1.0f);
		glTexCoord2f(x[1], y[1]);
        glVertex3f(+ 1.0f, centerY, - 1.0f);
		glTexCoord2f(x[2], y[2]);
        glVertex3f(+ 1.0f, centerY, + 1.0f);
		glTexCoord2f(x[3], y[3]);
        glVertex3f(- 1.0f, centerY, + 1.0f);
        glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);
#ifndef NDS
		glPopMatrix();
#else
		glPopMatrix(1);
#endif
    }
}

void drawLeftNear(const struct Vec3 center,
				  const FixP_t scale,
				  const struct Texture *texture,
				  const uint8_t mask,
				  const uint8_t repeatTexture) {


	FixP_t acc;
	FixP_t bias = intToFix(128);
	FixP_t scaled;

    float centerX, centerY, centerZ;
	scaled = Mul( scale, bias );
    float textureScale = (fixToInt(scaled) / 128.0f);
    float geometryScale = textureScale;

	acc = (center.mY + playerHeight + walkingBias + yCameraOffset);
	scaled = Mul( acc, bias );
	centerY = (fixToInt(scaled) / 128.0f);


    centerX = fixToInt(center.mX+ xCameraOffset);
    centerZ = -fixToInt(center.mZ+ zCameraOffset);

	glPushMatrix();
	glTranslatef(centerX, 0.0f, centerZ);

    glBindTexture(GL_TEXTURE_2D, texture->raw->uploadId);
    glBegin(GL_QUADS);

	if (cameraDirection == kWest || cameraDirection == kEast) {
		glTexCoord2f(0, textureScale);
		glVertex3f(- 1.0f, centerY - geometryScale, - 1.0f);
		glTexCoord2f(1, textureScale);
		glVertex3f(+ 1.0f, centerY - geometryScale, + 1.0f);
		glTexCoord2f(1, 0.0f);
		glVertex3f(+ 1.0f, centerY + geometryScale, + 1.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(- 1.0f, centerY + geometryScale, - 1.0f);
	} else {
		glTexCoord2f(0, textureScale);
		glVertex3f(- 1.0f, centerY - geometryScale, + 1.0f);
		glTexCoord2f(1, textureScale);
		glVertex3f(+ 1.0f, centerY - geometryScale, - 1.0f);
		glTexCoord2f(1, 0.0f);
		glVertex3f(+ 1.0f, centerY + geometryScale, - 1.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(- 1.0f, centerY + geometryScale, + 1.0f);
	}
#ifndef NDS
	glPopMatrix();
#else
	glPopMatrix(1);
#endif
    glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void drawRightNear(const struct Vec3 center,
				   const FixP_t scale,
				   const struct Texture *texture,
				   const uint8_t mask,
				   const uint8_t repeatTexture) {

	float centerX, centerY, centerZ;
	FixP_t acc;
	FixP_t bias = intToFix(128);
	FixP_t scaled;

	scaled = Mul( scale, bias );
	float textureScale = (fixToInt(scaled) / 128.0f);
	float geometryScale = textureScale;


	acc = (center.mY + playerHeight + walkingBias + yCameraOffset);
	scaled = Mul( acc, bias );
	centerY = (fixToInt(scaled) / 128.0f);

    centerX = fixToInt(center.mX+ xCameraOffset);
    centerZ = -fixToInt(center.mZ+ zCameraOffset);

	glPushMatrix();
	glTranslatef(centerX, 0.0f, centerZ);

    glBindTexture(GL_TEXTURE_2D, texture->raw->uploadId);
    glBegin(GL_QUADS);


	if (cameraDirection == kWest || cameraDirection == kEast) {
		glTexCoord2f(0, textureScale);
		glVertex3f(- 1.0f, centerY - geometryScale, + 1.0f);
		glTexCoord2f(1, textureScale);
		glVertex3f(+ 1.0f, centerY - geometryScale, - 1.0f);
		glTexCoord2f(1, 0);
		glVertex3f(+ 1.0f, centerY + geometryScale, - 1.0f);
		glTexCoord2f(0, 0);
		glVertex3f(- 1.0f, centerY + geometryScale, + 1.0f);
	} else {
		glTexCoord2f(0, textureScale);
		glVertex3f(- 1.0f, centerY - geometryScale, - 1.0f);
		glTexCoord2f(1, textureScale);
		glVertex3f(+ 1.0f, centerY - geometryScale, + 1.0f);
		glTexCoord2f(1, 0);
		glVertex3f(+ 1.0f, centerY + geometryScale, + 1.0f);
		glTexCoord2f(0, 0);
		glVertex3f(- 1.0f, centerY + geometryScale, - 1.0f);
	}
#ifndef NDS
	glPopMatrix();
#else
	glPopMatrix(1);
#endif
    glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}
