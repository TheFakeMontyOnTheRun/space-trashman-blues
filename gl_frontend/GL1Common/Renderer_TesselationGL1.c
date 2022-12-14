#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
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
const static float ratio = 240.0f / 200.0f;

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
	return toReturn;
}

void drawRampAt(const struct Vec3 p0, const struct Vec3 p1,
				const struct Texture *texture, uint8_t direction, uint8_t flipTexture) {

	float centerX, centerY0, centerY1, centerZ;
	centerX = fixToInt(p0.mX + xCameraOffset);
	centerZ = -fixToInt(p0.mZ + zCameraOffset);

	centerY0 = fixToInt(p0.mY + playerHeight + walkingBias + yCameraOffset) * ratio;
	centerY1 = fixToInt(p1.mY + playerHeight + walkingBias + yCameraOffset) * ratio;


	glBindTexture(GL_TEXTURE_2D, texture->raw->uploadId);
	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 0.0f);

	switch (direction) {
		case kNorth:
			glVertex3f(centerX - 1.0f, centerY0, centerZ - 1.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(centerX + 1.0f, centerY1, centerZ - 1.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(centerX + 1.0f, centerY1, centerZ + 1.0f);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(centerX - 1.0f, centerY0, centerZ + 1.0f);
			break;
		case kSouth:
			glVertex3f(centerX - 1.0f, centerY0, centerZ - 1.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(centerX + 1.0f, centerY1, centerZ - 1.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(centerX + 1.0f, centerY1, centerZ + 1.0f);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(centerX - 1.0f, centerY0, centerZ + 1.0f);
			break;
		case kEast:
			glVertex3f(centerX - 1.0f, centerY0, centerZ - 1.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(centerX + 1.0f, centerY1, centerZ - 1.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(centerX + 1.0f, centerY1, centerZ + 1.0f);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(centerX - 1.0f, centerY0, centerZ + 1.0f);
			break;
		case kWest:
			glVertex3f(centerX - 1.0f, centerY0, centerZ - 1.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(centerX + 1.0f, centerY1, centerZ - 1.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(centerX + 1.0f, centerY1, centerZ + 1.0f);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(centerX - 1.0f, centerY0, centerZ + 1.0f);
			break;
	}

	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void drawBillboardAt(const struct Vec3 center,
					 struct Texture *bitmap,
					 const FixP_t scale,
					 const int size) {
	FixP_t one = intToFix(1);
    FixP_t zero = 0;
    FixP_t minusOne = -one;
    FixP_t minusScale = (-scale);
	struct Vec3 scaledCenter;

	if (center.mZ <= kMinZCull) {
		return;
	}

	initVec3(&scaledCenter, center.mX, (center.mY), center.mZ);

    float centerX, centerY, centerZ;
    float textureScale = fixToInt(scale);
    float geometryScale = fixToInt(scale) * ratio;
    
    centerX = fixToInt(center.mX + xCameraOffset);
    centerY = fixToInt(center.mY + playerHeight + walkingBias + yCameraOffset) * (240.0f/200.0f);
    centerZ = -fixToInt(center.mZ + zCameraOffset);
    
    glBindTexture(GL_TEXTURE_2D, bitmap->raw->uploadId);
    
    glEnable(GL_ALPHA_TEST);

    glBegin(GL_QUADS);

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(centerX - 1.0f, centerY - geometryScale, centerZ + 1.0f);
    glTexCoord2f(textureScale, 1.0f);
    glVertex3f(centerX + 1.0f, centerY - geometryScale, centerZ + 1.0f);
    glTexCoord2f(textureScale, 0.0f);
    glVertex3f(centerX + 1.0f, centerY + geometryScale, centerZ + 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(centerX - 1.0f, centerY + geometryScale, centerZ + 1.0f);
    
    glEnd();
    glDisable(GL_ALPHA_TEST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void drawColumnAt(const struct Vec3 center,
				  const FixP_t scale,
				  const struct Texture *texture,
				  const uint8_t mask,
				  const uint8_t enableAlpha,
				  const uint8_t repeatTexture) {

    float centerX, centerY, centerZ;
    float textureScale = fixToInt(scale);
    float geometryScale = fixToInt(scale) * ratio;
    
    if (!repeatTexture) {
        textureScale = 1.0f;
    }
    
    centerX = fixToInt(center.mX + xCameraOffset);
    centerY = fixToInt(center.mY + playerHeight + walkingBias + yCameraOffset) * ratio;
    centerZ = -fixToInt(center.mZ + zCameraOffset);

    glBindTexture(GL_TEXTURE_2D, texture->raw->uploadId);

    if (enableAlpha) {
        glEnable(GL_ALPHA_TEST);
    }
    
    glBegin(GL_QUADS);

    if (enableAlpha && (mask & MASK_FRONT)) {

        glTexCoord2f(textureScale, 1.0f);
        glVertex3f(centerX - 1.0f, centerY - geometryScale, centerZ - 1.0f);
        glTexCoord2f(textureScale, 0.0f);
        glVertex3f(centerX + 1.0f, centerY - geometryScale, centerZ - 1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(centerX + 1.0f, centerY + geometryScale, centerZ - 1.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(centerX - 1.0f, centerY + geometryScale, centerZ - 1.0f);
    }

    if ((mask & MASK_RIGHT) && fixToInt(center.mX) > 0) {
        glTexCoord2f(textureScale, 0.0f);
        glVertex3f(centerX - 1.0f, centerY - geometryScale, centerZ - 1.0f);
        glTexCoord2f(textureScale, 1.0f);
        glVertex3f(centerX - 1.0f, centerY - geometryScale, centerZ + 1.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(centerX - 1.0f, centerY + geometryScale, centerZ + 1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(centerX - 1.0f, centerY + geometryScale, centerZ - 1.0f);
    }

    if ((mask & MASK_LEFT) && fixToInt(center.mX) < 0) {
        glTexCoord2f(textureScale, 0.0f);
        glVertex3f(centerX + 1.0f, centerY - geometryScale, centerZ - 1.0f);
        glTexCoord2f(textureScale, 1.0f);
        glVertex3f(centerX + 1.0f, centerY - geometryScale, centerZ + 1.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(centerX + 1.0f, centerY + geometryScale, centerZ + 1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(centerX + 1.0f, centerY + geometryScale, centerZ - 1.0f);
    }

    if ((mask & MASK_BEHIND) || (mask & MASK_FRONT)) {
        glTexCoord2f(textureScale, 1.0f);
        glVertex3f(centerX - 1.0f, centerY - geometryScale, centerZ + 1.0f);
        glTexCoord2f(textureScale, 0.0f);
        glVertex3f(centerX + 1.0f, centerY - geometryScale, centerZ + 1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(centerX + 1.0f, centerY + geometryScale, centerZ + 1.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(centerX - 1.0f, centerY + geometryScale, centerZ + 1.0f);
    }

    glEnd();
    glDisable(GL_ALPHA_TEST);
}

void drawFloorAt(const struct Vec3 center,
				 const struct Texture *texture, enum EDirection cameraDirection) {
    
    float centerX, centerY, centerZ;
    centerX = fixToInt(center.mX + xCameraOffset);
    centerY = fixToInt(center.mY + playerHeight + walkingBias + yCameraOffset) * ratio;
    centerZ = -fixToInt(center.mZ + zCameraOffset);
    
    if (center.mY <= 0) {
        glBindTexture(GL_TEXTURE_2D, texture->raw->uploadId);
        glBegin(GL_QUADS);
        
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(centerX - 1.0f, centerY, centerZ - 1.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(centerX + 1.0f, centerY, centerZ - 1.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(centerX + 1.0f, centerY, centerZ + 1.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(centerX - 1.0f, centerY, centerZ + 1.0f);
        
        glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void drawCeilingAt(const struct Vec3 center,
				   const struct Texture *texture, enum EDirection cameraDirection) {

    float centerX, centerY, centerZ;
    centerX = fixToInt(center.mX+ xCameraOffset);
    centerY = fixToInt(center.mY + playerHeight + walkingBias+ yCameraOffset) * ratio;
    centerZ = -fixToInt(center.mZ+ zCameraOffset);
    
    if (center.mY >= 0) {
        glBindTexture(GL_TEXTURE_2D, texture->raw->uploadId);
        glBegin(GL_QUADS);
        
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(centerX - 1.0f, centerY, centerZ - 1.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(centerX + 1.0f, centerY, centerZ - 1.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(centerX + 1.0f, centerY, centerZ + 1.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(centerX - 1.0f, centerY, centerZ + 1.0f);
        glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);
    }	
}

void drawLeftNear(const struct Vec3 center,
				  const FixP_t scale,
				  const struct Texture *texture,
				  const uint8_t mask,
				  const uint8_t repeatTexture) {

   
    
    float centerX, centerY, centerZ;
    float textureScale = fixToInt(scale);
    float geometryScale = fixToInt(scale) * ratio;
    centerX = fixToInt(center.mX+ xCameraOffset);
    centerY = fixToInt(center.mY + playerHeight + walkingBias+ yCameraOffset) * ratio;
    centerZ = -fixToInt(center.mZ+ zCameraOffset);
    
    glBindTexture(GL_TEXTURE_2D, texture->raw->uploadId);
    glBegin(GL_QUADS);
    
    glTexCoord2f(textureScale, 0.0f);
    glVertex3f(centerX - 1.0f, centerY - geometryScale, centerZ + 1.0f);
    glTexCoord2f(textureScale, 1.0f);
    glVertex3f(centerX + 1.0f, centerY - geometryScale, centerZ - 1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(centerX + 1.0f, centerY + geometryScale, centerZ - 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(centerX - 1.0f, centerY + geometryScale, centerZ + 1.0f);
    glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void drawRightNear(const struct Vec3 center,
				   const FixP_t scale,
				   const struct Texture *texture,
				   const uint8_t mask,
				   const uint8_t repeatTexture) {

	float centerX, centerY, centerZ;
    float textureScale = fixToInt(scale);
    float geometryScale = fixToInt(scale) * ratio;
    centerX = fixToInt(center.mX+ xCameraOffset);
    centerY = fixToInt(center.mY + playerHeight + walkingBias+ yCameraOffset) * ratio;
    centerZ = -fixToInt(center.mZ+ zCameraOffset);
    
    glBindTexture(GL_TEXTURE_2D, texture->raw->uploadId);
    glBegin(GL_QUADS);
    
    glTexCoord2f(textureScale, 0.0f);
    glVertex3f(centerX - 1.0f, centerY - geometryScale, centerZ - 1.0f);
    glTexCoord2f(textureScale, 1.0f);
    glVertex3f(centerX + 1.0f, centerY - geometryScale, centerZ + 1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(centerX + 1.0f, centerY + geometryScale, centerZ + 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(centerX - 1.0f, centerY + geometryScale, centerZ - 1.0f);
    glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}
