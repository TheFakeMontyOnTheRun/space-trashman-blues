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
struct Projection projectionVertices[8];

void projectAllVertices(const int count) {
}

struct Texture *nativeTextures[TOTAL_TEXTURES];
int usedTexture = 0;

void clearTextures() {
    usedTexture = 0;
}


struct Texture *makeTextureFrom(const char *filename) {
    struct StaticBuffer src = loadBinaryFileFromPath(filename);
    struct Texture *toReturn;
    uint8_t *ptr = src.data + 4; //skip header;
    size_t sizeInDisk = src.size - 4;

#ifndef N64
	uint8_t * diskBuffer = (struct Texture*)calloc(1, sizeof(struct Texture));
#else
	uint8_t * diskBuffer = (struct Texture*)malloc_uncached(sizeInDisk);
	memset( diskBuffer, 0, sizeInDisk);
#endif


    memcpy(diskBuffer, ptr, sizeInDisk);

    BitmapPixelFormat pixel;
    int pixelIndex = 0;
    int y;
    BitmapPixelFormat buffer[NATIVE_TEXTURE_SIZE * NATIVE_TEXTURE_SIZE];

#ifdef PALETTE_COLOURS_FRAMEBUFFER
    for (size_t c = 0; c < sizeInDisk; c += 2) {
        pixel = diskBuffer[c];
        unsigned int repetitions = diskBuffer[c + 1];

        for (uint8_t d = 0; d < repetitions; ++d) {
            buffer[pixelIndex++] = pixel;
        }
    }
#else
    for (size_t c = 0; c < sizeInDisk; c += 5) {
        pixel = 0;

        if (diskBuffer[c + 3] < 255) {
            pixel = TRANSPARENCY_COLOR;
        } else {
            pixel += diskBuffer[c + 3] << 24;
            pixel += diskBuffer[c + 2] << 16;
            pixel += diskBuffer[c + 1] << 8;
            pixel += diskBuffer[c + 0] << 0;
        }

        unsigned int repetitions = diskBuffer[c + 4];

        for (uint8_t d = 0; d < repetitions; ++d) {
            buffer[pixelIndex++] = pixel;
        }
    }
#endif
    free(diskBuffer);

    assert((usedTexture + 1 ) < TOTAL_TEXTURES);
#ifndef N64
    toReturn = (struct Texture*)calloc(1, sizeof(struct Texture));
#else
	toReturn = (struct Texture*)malloc_uncached(sizeof(struct Texture));
	memset( toReturn, 0, sizeof(struct Texture));
#endif

    nativeTextures[usedTexture++] = toReturn;
    toReturn->uploadId = usedTexture;
    
    for (int y = 0; y < NATIVE_TEXTURE_SIZE; ++y) {
        BitmapPixelFormat *sourceLine = &buffer[y * NATIVE_TEXTURE_SIZE];
        BitmapPixelFormat *dstLine = &toReturn->rowMajor[y];
        for (int x = 0; x < NATIVE_TEXTURE_SIZE; ++x) {
            *dstLine = *sourceLine;
            sourceLine++;
            dstLine += NATIVE_TEXTURE_SIZE;
        }
    }
    
    uint8_t expanded[NATIVE_TEXTURE_SIZE * NATIVE_TEXTURE_SIZE * 4];
    
    for (int c = 0; c < NATIVE_TEXTURE_SIZE * NATIVE_TEXTURE_SIZE; ++c ) {
        uint32_t index = toReturn->rowMajor[c];
                  
        uint32_t texel = index;//palette[index];
        
        if (index == TRANSPARENCY_COLOR) {
            expanded[4 * c + 0] = 0xFF;
            expanded[4 * c + 1] = 0;
            expanded[4 * c + 2] = 0;
            expanded[4 * c + 3] = 0;
        } else {
            expanded[4 * c + 0] = (texel & 0xFF);
            expanded[4 * c + 1] = (texel & 0x00FF00) >> 8;
            expanded[4 * c + 2] = (texel & 0xFF0000) >> 16;

            expanded[4 * c + 3] = 1;
        }
    }
    

    glGenTextures(1, (GLuint*)&toReturn->uploadId);
    glBindTexture(GL_TEXTURE_2D, toReturn->uploadId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, NATIVE_TEXTURE_SIZE, NATIVE_TEXTURE_SIZE,
                 0, GL_RGBA,   GL_UNSIGNED_BYTE, &expanded[0]);
    
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);

    return toReturn;
}

void drawRampAt(const struct Vec3 p0, const struct Vec3 p1,
				const struct Texture *texture, uint8_t direction, uint8_t flipTexture) {

	float centerX, centerY0, centerY1, centerZ;
	centerX = fixToInt(p0.mX + xCameraOffset);
	centerZ = -fixToInt(p0.mZ + zCameraOffset);

	centerY0 = fixToInt(p0.mY + playerHeight + walkingBias + yCameraOffset) * ratio;
	centerY1 = fixToInt(p1.mY + playerHeight + walkingBias + yCameraOffset) * ratio;


	glBindTexture(GL_TEXTURE_2D, texture->uploadId);
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
    
    if (bitmap->uploadId == -1 ) {
        uint32_t newId;
#ifndef N64
        uint8_t* expanded = (uint8_t*)malloc(NATIVE_TEXTURE_SIZE * NATIVE_TEXTURE_SIZE * sizeof(TexturePixelFormat));
#else
		uint8_t* expanded = (uint8_t*)malloc_uncached(NATIVE_TEXTURE_SIZE * NATIVE_TEXTURE_SIZE * sizeof(TexturePixelFormat));
#endif

        for (int c = 0; c < NATIVE_TEXTURE_SIZE * NATIVE_TEXTURE_SIZE; ++c ) {
            uint32_t index = bitmap->rotations[0][c];
                      
            uint32_t texel = index;//palette[index];
            
            if (index == TRANSPARENCY_COLOR) {
                expanded[4 * c + 0] = 0xFF;
                expanded[4 * c + 1] = 0;
                expanded[4 * c + 2] = 0;
                expanded[4 * c + 3] = 0;
            } else {
                expanded[4 * c + 0] = (texel & 0xFF);
                expanded[4 * c + 1] = (texel & 0x00FF00) >> 8;
                expanded[4 * c + 2] = (texel & 0xFF0000) >> 16;

                expanded[4 * c + 3] = 1;
            }
        }
        
        glGenTextures(1, (GLuint*)&newId);
        glBindTexture(GL_TEXTURE_2D, newId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, NATIVE_TEXTURE_SIZE, NATIVE_TEXTURE_SIZE, 0, GL_RGBA,   GL_UNSIGNED_BYTE, &expanded[0]);
        
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        free( expanded );
        
        bitmap->uploadId = newId;
    }
    
	initVec3(&scaledCenter, center.mX, (center.mY), center.mZ);
	initVec3(&projectionVertices[0].first, scaledCenter.mX, scaledCenter.mY,
			 scaledCenter.mZ);
	initVec3(&projectionVertices[1].first, scaledCenter.mX, scaledCenter.mY,
			 scaledCenter.mZ);
	addToVec3(&projectionVertices[0].first, minusOne, scale, zero);
	addToVec3(&projectionVertices[1].first, one, minusScale, zero);

    float centerX, centerY, centerZ;
    float textureScale = fixToInt(scale);
    float geometryScale = fixToInt(scale) * ratio;
    
    centerX = fixToInt(center.mX + xCameraOffset);
    centerY = fixToInt(center.mY + playerHeight + walkingBias + yCameraOffset) * (240.0f/200.0f);
    centerZ = -fixToInt(center.mZ + zCameraOffset);
    
    glBindTexture(GL_TEXTURE_2D, bitmap->uploadId);
    
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
    
    glBindTexture(GL_TEXTURE_2D, texture->uploadId);
    
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

        glBindTexture(GL_TEXTURE_2D, texture->uploadId);
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
        
        glBindTexture(GL_TEXTURE_2D, texture->uploadId);
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
    
    glBindTexture(GL_TEXTURE_2D, texture->uploadId);
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
    
    glBindTexture(GL_TEXTURE_2D, texture->uploadId);
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
