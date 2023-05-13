#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


#include <kernel.h>
#include <stdlib.h>
#include <malloc.h>
#include <tamtypes.h>
#include <math3d.h>
#include <packet.h>
#include <dma_tags.h>
#include <gif_tags.h>
#include <gs_psm.h>
#include <dma.h>
#include <graph.h>
#include <draw.h>
#include <draw3d.h>


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

extern MATRIX local_world;
extern MATRIX world_view;
extern MATRIX view_screen;
extern MATRIX local_screen;
extern packet_t *current;

extern qword_t *_q;
extern xyz_t *verts;
extern texel_t *st;
extern color_t *colors;
extern VECTOR *temp_vertices;
extern int vertex_count;
extern prim_t prim;
extern color_t color;

#define GEOMETRY_SCALE_X 2.0f
#define GEOMETRY_SCALE_Y 1.0f
#define GEOMETRY_SCALE_Z 2.0f
#define GEOMETRY_TEXTURE_SCALE_X 1.0f
#define GEOMETRY_TEXTURE_SCALE_Y 1.0f

#define BIAS (intToFix(8))
#define REVERSE_BIAS (1.0f/8.0f)
#define FOG_MAX_DISTANCE 32.0f

void clearTextures(void) {
    char buffer[256];

    for (int c = 1; c < itemsCount; ++c) {
        sprintf(&buffer[0], "%s.img", getItem(c)->name);
        itemSprites[c] = (makeTextureFrom(&buffer[0]));

        if (itemSprites[c]) {
            releaseBitmap(itemSprites[c]->raw);
            free(itemSprites[c]);
        }
    }

	for ( int c = 0; c < texturesUsed; ++c ) {
		if (nativeTextures[c]) {
			releaseBitmap(nativeTextures[c]->raw);
			free(nativeTextures[c]);
		}
	}

	texturesUsed = 0;
}

float clampf(float v0, float v1, float v) {
    if (v < v0) {
        return v0;
    } else if (v > v1) {
        return v1;
    } else {
        return v;
    }
}


struct Texture *makeTextureFrom(const char *filename) {
	struct Texture *toReturn =
			(struct Texture *) calloc(1, sizeof(struct Texture));

	toReturn->raw = loadBitmap(filename);
	submitBitmapToGPU(toReturn->raw);


	return toReturn;
}

void drawRampAt(const struct Vec3 p0, const struct Vec3 p1,
				const struct Texture *texture, uint8_t direction, uint8_t flipTexture) {



	if ((p0.mZ + zCameraOffset) > 0 && (p1.mZ + zCameraOffset) > 0  ) {

		float centerY0, centerY1;
		FixP_t acc;
		FixP_t scaled;
		float geometryScale;
		float centerY;

		bindTexture(texture->raw);


		acc = (p1.mY)  + playerHeight + walkingBias + yCameraOffset;
		scaled = Mul(acc, BIAS);
		centerY1 = GEOMETRY_SCALE_Y * (fixToInt(scaled) * REVERSE_BIAS);

		acc = (p0.mY)  + playerHeight + walkingBias + yCameraOffset;
		scaled = Mul(acc, BIAS);
		centerY0 = GEOMETRY_SCALE_Y * (fixToInt(scaled) * REVERSE_BIAS);

        geometryScale = (centerY1 - centerY0) * 0.5f;
        centerY = (centerY1 + centerY0) * 0.5f;

		int x[4], y[4];
		float centerX;
		float centerZ;
		u64 *dw;
		centerX = GEOMETRY_SCALE_X *  (fixToInt(Mul(p0.mX + xCameraOffset, BIAS)) * 0.5f * REVERSE_BIAS);
		centerZ = -GEOMETRY_SCALE_Z * (fixToInt(Mul(p0.mZ + zCameraOffset, BIAS)) * 0.5f * REVERSE_BIAS);

		VECTOR object_position = {centerX, centerY, centerZ, 1.00f};
		VECTOR object_rotation = {0.00f, 0.00f, 0.00f, 1.00f};

		// Create the local_world matrix.
		create_local_world(local_world, object_position, object_rotation);

		// Create the local_screen matrix.
		create_local_screen(local_screen, local_world, world_view, view_screen);

		qword_t *q;

		int points_count = 6;

		int points[6] = {
				0, 1, 2,
				1, 2, 3
		};

        float fogAttenuation = 1.0f - (1.0f - (centerZ / FOG_MAX_DISTANCE));

        VECTOR colours[4] = {
                {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
                {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
                {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
                {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
        };
		switch (direction) {
			case kSouth: {
                x[0] = 0;
                y[0] = 0;
                x[1] = 1;
                y[1] = 0;
                x[2] = 0;
                y[2] = 1;
                x[3] = 1;
                y[3] = 1;

				VECTOR vertices[4] = {
						{- GEOMETRY_SCALE_X * 0.5f, - geometryScale, - GEOMETRY_SCALE_Z * 0.5f, 1.00f},
						{+ GEOMETRY_SCALE_X * 0.5f, - geometryScale, - GEOMETRY_SCALE_Z * 0.5f, 1.00f},
						{- GEOMETRY_SCALE_X * 0.5f,   geometryScale, + GEOMETRY_SCALE_Z * 0.5f, 1.00f},
						{+ GEOMETRY_SCALE_X * 0.5f,   geometryScale, + GEOMETRY_SCALE_Z * 0.5f, 1.00f}
				};

				q = _q;

				// Calculate the vertex values.
				calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);
			}
				break;
			case kNorth: {
                x[0] = 1;
                y[0] = 1;
                x[1] = 0;
                y[1] = 1;
                x[2] = 1;
                y[2] = 0;
                x[3] = 0;
                y[3] = 0;


				VECTOR vertices[4] = {
						{- GEOMETRY_SCALE_X * 0.5f,   geometryScale, - GEOMETRY_SCALE_Z * 0.5f, 1.00f},
						{+ GEOMETRY_SCALE_X * 0.5f,   geometryScale, - GEOMETRY_SCALE_Z * 0.5f, 1.00f},
						{- GEOMETRY_SCALE_X * 0.5f, - geometryScale, + GEOMETRY_SCALE_Z * 0.5f, 1.00f},
						{+ GEOMETRY_SCALE_X * 0.5f, - geometryScale, + GEOMETRY_SCALE_Z * 0.5f, 1.00f}
				};

				q = _q;

				// Calculate the vertex values.
				calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);
			}
				break;
			case kWest: {
                x[0] = 1;
                y[0] = 0;
                x[1] = 1;
                y[1] = 1;
                x[2] = 0;
                y[2] = 0;
                x[3] = 0;
                y[3] = 1;
				VECTOR vertices[4] = {
						{- GEOMETRY_SCALE_X * 0.5f, - geometryScale, - GEOMETRY_SCALE_Z * 0.5f, 1.00f},
						{+ GEOMETRY_SCALE_X * 0.5f,   geometryScale, - GEOMETRY_SCALE_Z * 0.5f, 1.00f},
						{- GEOMETRY_SCALE_X * 0.5f, - geometryScale, + GEOMETRY_SCALE_Z * 0.5f, 1.00f},
						{+ GEOMETRY_SCALE_X * 0.5f,   geometryScale, + GEOMETRY_SCALE_Z * 0.5f, 1.00f}
				};

				q = _q;

				// Calculate the vertex values.
				calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);
			}
				break;
			case kEast:
			default: {
                x[0] = 0;
                y[0] = 1;
                x[1] = 0;
                y[1] = 0;
                x[2] = 1;
                y[2] = 1;
                x[3] = 1;
                y[3] = 0;

				VECTOR vertices[4] = {
						{- GEOMETRY_SCALE_X * 0.5f,   geometryScale, - GEOMETRY_SCALE_Z * 0.5f, 1.00f},
						{+ GEOMETRY_SCALE_X * 0.5f, - geometryScale, - GEOMETRY_SCALE_Z * 0.5f, 1.00f},
						{- GEOMETRY_SCALE_X * 0.5f,   geometryScale, + GEOMETRY_SCALE_Z * 0.5f, 1.00f},
						{+ GEOMETRY_SCALE_X * 0.5f, - geometryScale, + GEOMETRY_SCALE_Z * 0.5f, 1.00f}
				};

				q = _q;

				// Calculate the vertex values.
				calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);
			}
				break;
		}

        VECTOR coordinates[4] = {
                {x[0], y[0], 0, 0},
                {x[1], y[1], 0, 0},
                {x[2], y[2], 0, 0},
                {x[3], y[3], 0, 0}
        };

		draw_convert_st(st, vertex_count, (vertex_f_t*)temp_vertices, (texel_f_t*)coordinates);

		// Convert floating point vertices to fixed point and translate to center of screen.
		draw_convert_xyz(verts, 2048, 2048, 2048, vertex_count, (vertex_f_t *) temp_vertices);

		// Convert floating point colours to fixed point.
		draw_convert_rgbaq(colors, vertex_count, (vertex_f_t *) temp_vertices, (color_f_t *) colours);

		// Draw the triangles using triangle primitive type.
		dw = (u64*)draw_prim_start(q,0,&prim, &color);

		for(int i = 0; i < points_count; i++)
		{
			*dw++ = colors[points[i]].rgbaq;
			*dw++ = st[points[i]].uv;
			*dw++ = verts[points[i]].xyz;
		}

		// Check if we're in middle of a qword or not.
		if ((u32)dw % 16) {
			*dw++ = 0;
		}

		q = draw_prim_end((qword_t*)dw,3,DRAW_STQ_REGLIST);

		++q;

		_q = q;
	}
}

void drawBillboardAt(const struct Vec3 center,
					 struct Texture *bitmap,
					 const FixP_t scale,
					 const int size) {
	if ((center.mZ + zCameraOffset) <= 0 ) {
		return;
	}

	bindTexture(bitmap->raw);

	qword_t *q;

	int points_count = 6;

	int points[6] = {
			0, 1, 2,
			1, 2, 3
	};

	u64 *dw;
	float centerY;
	float centerX;
	float centerZ;
	FixP_t acc;
	FixP_t scaled = Mul(scale, BIAS);
	float geometryScale =  (fixToInt(scaled) * REVERSE_BIAS);
	acc = center.mY + playerHeight + walkingBias + yCameraOffset;
	scaled = Mul(acc, BIAS);
	centerY = GEOMETRY_SCALE_Y * (fixToInt(scaled) * REVERSE_BIAS);

	centerX = GEOMETRY_SCALE_X *  (fixToInt(Mul(center.mX + xCameraOffset, BIAS)) * 0.5f * REVERSE_BIAS);
	centerZ = -GEOMETRY_SCALE_Z * (fixToInt(Mul(center.mZ + zCameraOffset, BIAS)) * 0.5f * REVERSE_BIAS);

	VECTOR object_position = {centerX, centerY, centerZ, 1.00f};
	VECTOR object_rotation = {0.00f, 0.00f, 0.00f, 1.00f};

	create_local_world(local_world, object_position, object_rotation);

	create_local_screen(local_screen, local_world, world_view, view_screen);

	VECTOR vertices[4] = {
			{ + GEOMETRY_SCALE_X * 0.5f,  + geometryScale,  0, 1.00f},
			{ - GEOMETRY_SCALE_X * 0.5f,  + geometryScale,  0, 1.00f},
			{ + GEOMETRY_SCALE_X * 0.5f,  - geometryScale,  0, 1.00f},
			{ - GEOMETRY_SCALE_X * 0.5f,  - geometryScale,  0, 1.00f}
	};

	q = _q;

	VECTOR coordinates[4] = {
			{ 1,  0,  0, 0},
			{ 0,  0,  0, 0},
			{ 1,  1,  0, 0},
			{ 0,  1,  0, 0}
	};

    float fogAttenuation = 1.0f - (1.0f - (centerZ / FOG_MAX_DISTANCE));

    VECTOR colours[4] = {
            {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
            {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
            {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
            {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
    };

	// Calculate the vertex values.
	calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

	draw_convert_st(st, vertex_count, (vertex_f_t*)temp_vertices, (texel_f_t*)coordinates);

	// Convert floating point vertices to fixed point and translate to center of screen.
	draw_convert_xyz(verts, 2048, 2048, 2048, vertex_count, (vertex_f_t *) temp_vertices);

	// Convert floating point colours to fixed point.
	draw_convert_rgbaq(colors, vertex_count, (vertex_f_t *) temp_vertices, (color_f_t *) colours);

	// Draw the triangles using triangle primitive type.
	dw = (u64*)draw_prim_start(q,0,&prim, &color);

	for(int i = 0; i < points_count; i++)
	{
		*dw++ = colors[points[i]].rgbaq;
		*dw++ = st[points[i]].uv;
		*dw++ = verts[points[i]].xyz;
	}

	// Check if we're in middle of a qword or not.
	if ((u32)dw % 16) {
		*dw++ = 0;
	}

	q = draw_prim_end((qword_t*)dw,3,DRAW_STQ_REGLIST);

	++q;

	_q = q;
}

void drawColumnAt(const struct Vec3 center,
				  const FixP_t scale,
				  const struct Texture *texture,
				  const uint8_t mask,
				  const uint8_t enableAlpha,
				  const uint8_t repeatTexture) {

	if ((center.mZ + zCameraOffset) <= 0 ) {
		return;
	}

	qword_t *q;
	u64 *dw;
	int points_count = 6;

	int points[6] = {
			0, 1, 2,
			1, 2, 3
	};

	float centerY;
	float centerX;
	float centerZ;
	FixP_t acc;
	FixP_t scaled = Mul(scale, BIAS);
    float textureScale = 1;
	float geometryScale = GEOMETRY_SCALE_Y * (fixToInt(scaled) * REVERSE_BIAS);

	acc = center.mY + playerHeight + walkingBias + yCameraOffset;
	scaled = Mul(acc, BIAS);
	centerY = GEOMETRY_SCALE_Y * (fixToInt(scaled) * REVERSE_BIAS);

	centerX = GEOMETRY_SCALE_X *  (fixToInt(Mul(center.mX + xCameraOffset, BIAS)) * 0.5f * REVERSE_BIAS);
	centerZ = -GEOMETRY_SCALE_Z * (fixToInt(Mul(center.mZ + zCameraOffset, BIAS)) * 0.5f * REVERSE_BIAS);

    float fogAttenuation = 1.0f - (1.0f - (centerZ / FOG_MAX_DISTANCE));

    VECTOR colours[4] = {
            {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
            {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
            {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
            {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
    };

	VECTOR object_position = {centerX, centerY, centerZ, 1.00f};
	VECTOR object_rotation = {0.00f, 0.00f, 0.00f, 1.00f};

	create_local_world(local_world, object_position, object_rotation);

	create_local_screen(local_screen, local_world, world_view, view_screen);

    if (repeatTexture) {
        textureScale = geometryScale;
    }

    if ((mask & MASK_BEHIND)) {

		bindTexture(texture->raw);

		VECTOR vertices[4] = {
				{ + GEOMETRY_SCALE_X * 0.5f, + geometryScale, - GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ - GEOMETRY_SCALE_X * 0.5f, + geometryScale, - GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ + GEOMETRY_SCALE_X * 0.5f, - geometryScale, - GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ - GEOMETRY_SCALE_X * 0.5f, - geometryScale, - GEOMETRY_SCALE_Z * 0.5f, 1.00f}
		};

		q = _q;

        VECTOR coordinates[4] = {
                { 1,  0,  0, 0},
                { 0,  0,  0, 0},
                { 1,  textureScale,  0, 0},
                { 0,  textureScale,  0, 0}
        };

		// Calculate the vertex values.
		calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

		draw_convert_st(st, vertex_count, (vertex_f_t*)temp_vertices, (texel_f_t*)coordinates);

		// Convert floating point vertices to fixed point and translate to center of screen.
		draw_convert_xyz(verts, 2048, 2048, 2048, vertex_count, (vertex_f_t *) temp_vertices);

		// Convert floating point colours to fixed point.
		draw_convert_rgbaq(colors, vertex_count, (vertex_f_t *) temp_vertices, (color_f_t *) colours);

		// Draw the triangles using triangle primitive type.
		dw = (u64*)draw_prim_start(q,0,&prim, &color);

		for(int i = 0; i < points_count; i++)
		{
			*dw++ = colors[points[i]].rgbaq;
			*dw++ = st[points[i]].uv;
			*dw++ = verts[points[i]].xyz;
		}

		// Check if we're in middle of a qword or not.
		if ((u32)dw % 16) {
			*dw++ = 0;
		}

		q = draw_prim_end((qword_t*)dw,3,DRAW_STQ_REGLIST);

		++q;

		_q = q;
	}

	if (((mask & MASK_RIGHT) && fixToInt(center.mX) > 0) || (mask & MASK_FORCE_RIGHT)) {

		bindTexture(texture->raw);

		VECTOR vertices[4] = {
				{ - GEOMETRY_SCALE_X * 0.5f,  + geometryScale,  + GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ - GEOMETRY_SCALE_X * 0.5f,  + geometryScale,  - GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ - GEOMETRY_SCALE_X * 0.5f,  - geometryScale,  + GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ - GEOMETRY_SCALE_X * 0.5f,  - geometryScale,  - GEOMETRY_SCALE_Z * 0.5f, 1.00f}
		};

		q = _q;

		VECTOR coordinates[4] = {
				{ 1,  0,  0, 0},
				{ 0,  0,  0, 0},
				{ 1,  textureScale,  0, 0},
				{ 0,  textureScale,  0, 0}
		};

		// Calculate the vertex values.
		calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

		draw_convert_st(st, vertex_count, (vertex_f_t*)temp_vertices, (texel_f_t*)coordinates);

		// Convert floating point vertices to fixed point and translate to center of screen.
		draw_convert_xyz(verts, 2048, 2048, 2048, vertex_count, (vertex_f_t *) temp_vertices);

		// Convert floating point colours to fixed point.
		draw_convert_rgbaq(colors, vertex_count, (vertex_f_t *) temp_vertices, (color_f_t *) colours);

		// Draw the triangles using triangle primitive type.
		dw = (u64*)draw_prim_start(q,0,&prim, &color);

		for(int i = 0; i < points_count; i++)
		{
			*dw++ = colors[points[i]].rgbaq;
			*dw++ = st[points[i]].uv;
			*dw++ = verts[points[i]].xyz;
		}

		// Check if we're in middle of a qword or not.
		if ((u32)dw % 16) {
			*dw++ = 0;
		}

		q = draw_prim_end((qword_t*)dw,3,DRAW_STQ_REGLIST);

		++q;

		_q = q;
	}

	if (((mask & MASK_LEFT) && fixToInt(center.mX) < 0) || (mask & MASK_FORCE_LEFT)) {

		bindTexture(texture->raw);

		VECTOR vertices[4] = {
				{ + GEOMETRY_SCALE_X * 0.5f,  + geometryScale,  - GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ + GEOMETRY_SCALE_X * 0.5f,  + geometryScale,  + GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ + GEOMETRY_SCALE_X * 0.5f,  - geometryScale,  - GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ + GEOMETRY_SCALE_X * 0.5f,  - geometryScale,  + GEOMETRY_SCALE_Z * 0.5f, 1.00f}
		};

		q = _q;

        VECTOR coordinates[4] = {
                { 1,  0,  0, 0},
                { 0,  0,  0, 0},
                { 1,  textureScale,  0, 0},
                { 0,  textureScale,  0, 0}
        };

		// Calculate the vertex values.
		calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

		draw_convert_st(st, vertex_count, (vertex_f_t*)temp_vertices, (texel_f_t*)coordinates);

		// Convert floating point vertices to fixed point and translate to center of screen.
		draw_convert_xyz(verts, 2048, 2048, 2048, vertex_count, (vertex_f_t *) temp_vertices);

		// Convert floating point colours to fixed point.
		draw_convert_rgbaq(colors, vertex_count, (vertex_f_t *) temp_vertices, (color_f_t *) colours);

		// Draw the triangles using triangle primitive type.
		dw = (u64*)draw_prim_start(q,0,&prim, &color);

		for(int i = 0; i < points_count; i++)
		{
			*dw++ = colors[points[i]].rgbaq;
			*dw++ = st[points[i]].uv;
			*dw++ = verts[points[i]].xyz;
		}

		// Check if we're in middle of a qword or not.
		if ((u32)dw % 16) {
			*dw++ = 0;
		}

		q = draw_prim_end((qword_t*)dw,3,DRAW_STQ_REGLIST);

		++q;

		_q = q;
	}

	if ((mask & MASK_FRONT)) {

		bindTexture(texture->raw);

		VECTOR vertices[4] = {
				{ + GEOMETRY_SCALE_X * 0.5f,  + geometryScale,  + GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ - GEOMETRY_SCALE_X * 0.5f,  + geometryScale,  + GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ + GEOMETRY_SCALE_X * 0.5f,  - geometryScale,  + GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ - GEOMETRY_SCALE_X * 0.5f,  - geometryScale,  + GEOMETRY_SCALE_Z * 0.5f, 1.00f}
		};

		q = _q;

        VECTOR coordinates[4] = {
                { 1,  0,  0, 0},
                { 0,  0,  0, 0},
                { 1,  textureScale,  0, 0},
                { 0,  textureScale,  0, 0}
        };

		// Calculate the vertex values.
		calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

		draw_convert_st(st, vertex_count, (vertex_f_t*)temp_vertices, (texel_f_t*)coordinates);

		// Convert floating point vertices to fixed point and translate to center of screen.
		draw_convert_xyz(verts, 2048, 2048, 2048, vertex_count, (vertex_f_t *) temp_vertices);

		draw_convert_rgbaq(colors, vertex_count, (vertex_f_t *) temp_vertices, (color_f_t *) colours);

		// Draw the triangles using triangle primitive type.
		dw = (u64*)draw_prim_start(q,0,&prim, &color);

		for(int i = 0; i < points_count; i++)
		{
			*dw++ = colors[points[i]].rgbaq;
			*dw++ = st[points[i]].uv;
			*dw++ = verts[points[i]].xyz;
		}

		// Check if we're in middle of a qword or not.
		if ((u32)dw % 16) {
			*dw++ = 0;
		}

		q = draw_prim_end((qword_t*)dw,3,DRAW_STQ_REGLIST);

		++q;

		_q = q;
	}
}

void drawFloorAt(const struct Vec3 center,
				 const struct Texture *texture, enum EDirection cameraDirection) {

	if (center.mY <= 0 && (center.mZ + zCameraOffset) > 0  ) {

		float centerY;
		FixP_t acc;
		FixP_t scaled;
		int x[4], y[4];

		bindTexture(texture->raw);

		acc = center.mY + playerHeight + walkingBias + yCameraOffset;
		scaled = Mul(acc, BIAS);
		centerY = GEOMETRY_SCALE_Y * (fixToInt(scaled) * REVERSE_BIAS);
		u64 *dw;
		float centerX;
		float centerZ;

		centerX = GEOMETRY_SCALE_X *  (fixToInt(Mul(center.mX + xCameraOffset, BIAS)) * 0.5f * REVERSE_BIAS);
		centerZ = -GEOMETRY_SCALE_Z * (fixToInt(Mul(center.mZ + zCameraOffset, BIAS)) * 0.5f * REVERSE_BIAS);

		VECTOR object_position = {centerX, centerY, centerZ, 1.00f};
		VECTOR object_rotation = {0.00f, 0.00f, 0.00f, 1.00f};

        switch (cameraDirection) {
            case kNorth: {
                x[0] = 0;
                y[0] = 0;
                x[1] = 1;
                y[1] = 0;
                x[2] = 0;
                y[2] = 1;
                x[3] = 1;
                y[3] = 1;
            } break;
            case kSouth: {
                x[0] = 1;
                y[0] = 1;
                x[1] = 0;
                y[1] = 1;
                x[2] = 1;
                y[2] = 0;
                x[3] = 0;
                y[3] = 0;
            } break;

            case kWest: {
                x[0] = 0;
                y[0] = 1;
                x[1] = 0;
                y[1] = 0;
                x[2] = 1;
                y[2] = 1;
                x[3] = 1;
                y[3] = 0;
            } break;
            case kEast:
            default: {
                x[0] = 1;
                y[0] = 0;
                x[1] = 1;
                y[1] = 1;
                x[2] = 0;
                y[2] = 0;
                x[3] = 0;
                y[3] = 1;
            } break;
        }

        // Create the local_world matrix.
        create_local_world(local_world, object_position, object_rotation);

        // Create the local_screen matrix.
        create_local_screen(local_screen, local_world, world_view, view_screen);

        VECTOR coordinates[4] = {
                {x[0], y[0], 0, 0},
                {x[1], y[1], 0, 0},
                {x[2], y[2], 0, 0},
                {x[3], y[3], 0, 0}
        };

        draw_convert_st(st, vertex_count, (vertex_f_t *) temp_vertices, (texel_f_t *) coordinates);

        qword_t *q;

		int points_count = 6;

		int points[6] = {
				0, 1, 2,
				1, 2, 3
		};

        float fogAttenuation = 1.0f - (1.0f - (centerZ / FOG_MAX_DISTANCE));

        VECTOR colours[4] = {
                {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
                {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
                {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
                {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
        };


		// Convert floating point colours to fixed point.
		draw_convert_rgbaq(colors, vertex_count, (vertex_f_t *) temp_vertices, (color_f_t *) colours);

		VECTOR vertices[4] = {
				{- GEOMETRY_SCALE_X * 0.5f, 0, - GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{+ GEOMETRY_SCALE_X * 0.5f, 0, - GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{- GEOMETRY_SCALE_X * 0.5f, 0, + GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{+ GEOMETRY_SCALE_X * 0.5f, 0, + GEOMETRY_SCALE_Z * 0.5f, 1.00f}
		};

		q = _q;

		// Calculate the vertex values.
		calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

		// Convert floating point vertices to fixed point and translate to center of screen.
		draw_convert_xyz(verts, 2048, 2048, 2048, vertex_count, (vertex_f_t *) temp_vertices);

		// Draw the triangles using triangle primitive type.
		dw = (u64*)draw_prim_start(q,0,&prim, &color);

		for(int i = 0; i < points_count; i++)
		{
			*dw++ = colors[points[i]].rgbaq;
			*dw++ = st[points[i]].uv;
			*dw++ = verts[points[i]].xyz;
		}

		// Check if we're in middle of a qword or not.
		if ((u32)dw % 16) {
			*dw++ = 0;
		}

		q = draw_prim_end((qword_t*)dw,3,DRAW_STQ_REGLIST);

		++q;

		_q = q;
	}
}

void drawCeilingAt(const struct Vec3 center,
				   const struct Texture *texture, enum EDirection cameraDirection) {

	if (center.mY >= 0 && center.mZ > 0 ) {

		float centerY;
		FixP_t acc;
		FixP_t scaled;
		int x[4], y[4];
		u64 *dw;

		bindTexture(texture->raw);

		acc = center.mY + playerHeight + walkingBias + yCameraOffset;
		scaled = Mul(acc, BIAS);
		centerY = GEOMETRY_SCALE_Y * (fixToInt(scaled) * REVERSE_BIAS);

		float centerX;
		float centerZ;

		centerX = GEOMETRY_SCALE_X *  (fixToInt(Mul(center.mX + xCameraOffset, BIAS)) * 0.5f * REVERSE_BIAS);
		centerZ = -GEOMETRY_SCALE_Z * (fixToInt(Mul(center.mZ + zCameraOffset, BIAS)) * 0.5f * REVERSE_BIAS);

		VECTOR object_position = {centerX, centerY, centerZ, 1.00f};
		VECTOR object_rotation = {0.00f, 0.00f, 0.00f, 1.00f};

        switch (cameraDirection) {
            case kNorth: {
                x[0] = 0;
                y[0] = 0;
                x[1] = 1;
                y[1] = 0;
                x[2] = 0;
                y[2] = 1;
                x[3] = 1;
                y[3] = 1;
            } break;
            case kSouth: {
                x[0] = 1;
                y[0] = 1;
                x[1] = 0;
                y[1] = 1;
                x[2] = 1;
                y[2] = 0;
                x[3] = 0;
                y[3] = 0;
            } break;

            case kWest: {
                x[0] = 0;
                y[0] = 1;
                x[1] = 0;
                y[1] = 0;
                x[2] = 1;
                y[2] = 1;
                x[3] = 1;
                y[3] = 0;
            } break;
            case kEast:
            default: {
                x[0] = 1;
                y[0] = 0;
                x[1] = 1;
                y[1] = 1;
                x[2] = 0;
                y[2] = 0;
                x[3] = 0;
                y[3] = 1;
            } break;
        }

        // Create the local_world matrix.
        create_local_world(local_world, object_position, object_rotation);

        // Create the local_screen matrix.
        create_local_screen(local_screen, local_world, world_view, view_screen);

        VECTOR coordinates[4] = {
                {x[0], y[0], 0, 0},
                {x[1], y[1], 0, 0},
                {x[2], y[2], 0, 0},
                {x[3], y[3], 0, 0}
        };

        draw_convert_st(st, vertex_count, (vertex_f_t *) temp_vertices, (texel_f_t *) coordinates);

		qword_t *q;

		int points_count = 6;

		int points[6] = {
				0, 1, 2,
				1, 2, 3
		};

        float fogAttenuation = 1.0f - (1.0f - (centerZ / FOG_MAX_DISTANCE));

        VECTOR colours[4] = {
                {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
                {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
                {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
                {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
        };

		// Convert floating point colours to fixed point.
		draw_convert_rgbaq(colors, vertex_count, (vertex_f_t *) temp_vertices, (color_f_t *) colours);

		VECTOR vertices[4] = {
				{ - GEOMETRY_SCALE_X * 0.5f, 0,  - GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ + GEOMETRY_SCALE_X * 0.5f, 0,  - GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ - GEOMETRY_SCALE_X * 0.5f, 0,  + GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ + GEOMETRY_SCALE_X * 0.5f, 0,  + GEOMETRY_SCALE_Z * 0.5f, 1.00f}
		};

		q = _q;

		// Calculate the vertex values.
		calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

		// Convert floating point vertices to fixed point and translate to center of screen.
		draw_convert_xyz(verts, 2048, 2048, 2048, vertex_count, (vertex_f_t *) temp_vertices);

		// Draw the triangles using triangle primitive type.
		dw = (u64*)draw_prim_start(q,0,&prim, &color);

		for(int i = 0; i < points_count; i++)
		{
			*dw++ = colors[points[i]].rgbaq;
			*dw++ = st[points[i]].uv;
			*dw++ = verts[points[i]].xyz;
		}

		// Check if we're in middle of a qword or not.
		if ((u32)dw % 16) {
			*dw++ = 0;
		}

		q = draw_prim_end((qword_t*)dw,3,DRAW_STQ_REGLIST);

		++q;

		_q = q;
	}
}

void drawLeftNear(const struct Vec3 center,
				  const FixP_t scale,
				  const struct Texture *texture,
				  const uint8_t mask,
				  const uint8_t repeatTexture) {


	if ((center.mZ + zCameraOffset) <= 0 ) {
		return;
	}

	bindTexture(texture->raw);

	qword_t *q;
	u64 *dw;
	int points_count = 6;

	int points[6] = {
			0, 1, 2,
			1, 2, 3
	};

	float centerY;
	float centerX;
	float centerZ;
	FixP_t acc;
	FixP_t scaled = Mul(scale, BIAS);
    float textureScale = 1;
	float geometryScale = GEOMETRY_SCALE_Y * (fixToInt(scaled) * REVERSE_BIAS);

    if (repeatTexture) {
        textureScale = geometryScale;
    }

    VECTOR coordinates[4] = {
			{ 1,  0,  0, 0},
			{ 0,  0,  0, 0},
			{ 1,  textureScale,  0, 0},
			{ 0,  textureScale,  0, 0}
	};

	acc = center.mY + playerHeight + walkingBias + yCameraOffset;
	scaled = Mul(acc, BIAS);
	centerY = GEOMETRY_SCALE_Y * (fixToInt(scaled) * REVERSE_BIAS);

	centerX = GEOMETRY_SCALE_X *  (fixToInt(Mul(center.mX + xCameraOffset, BIAS)) * 0.5f * REVERSE_BIAS);
	centerZ = -GEOMETRY_SCALE_Z * (fixToInt(Mul(center.mZ + zCameraOffset, BIAS)) * 0.5f * REVERSE_BIAS);

    float fogAttenuation = 1.0f - (1.0f - (centerZ / FOG_MAX_DISTANCE));

    VECTOR colours[4] = {
            {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
            {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
            {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
            {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
    };


	VECTOR object_position = {centerX, centerY, centerZ, 1.00f};
	VECTOR object_rotation = {0.00f, 0.00f, 0.00f, 1.00f};

	create_local_world(local_world, object_position, object_rotation);

	create_local_screen(local_screen, local_world, world_view, view_screen);

	if (cameraDirection == kWest || cameraDirection == kEast) {
		VECTOR vertices[4] = {
				{+GEOMETRY_SCALE_X * 0.5f, +geometryScale, +GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{-GEOMETRY_SCALE_X * 0.5f, +geometryScale, -GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{+GEOMETRY_SCALE_X * 0.5f, -geometryScale, +GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{-GEOMETRY_SCALE_X * 0.5f, -geometryScale, -GEOMETRY_SCALE_Z * 0.5f, 1.00f}
		};

		q = _q;

		// Calculate the vertex values.
		calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);
	} else {
		VECTOR vertices[4] = {
				{+GEOMETRY_SCALE_X * 0.5f, +geometryScale, -GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{-GEOMETRY_SCALE_X * 0.5f, +geometryScale, +GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{+GEOMETRY_SCALE_X * 0.5f, -geometryScale, -GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{-GEOMETRY_SCALE_X * 0.5f, -geometryScale, +GEOMETRY_SCALE_Z * 0.5f, 1.00f}
		};

		q = _q;

		// Calculate the vertex values.
		calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);
	}

	// Convert floating point vertices to fixed point and translate to center of screen.
	draw_convert_xyz(verts, 2048, 2048, 2048, vertex_count, (vertex_f_t *) temp_vertices);

	draw_convert_st(st, vertex_count, (vertex_f_t*)temp_vertices, (texel_f_t*)coordinates);

	// Convert floating point colours to fixed point.
	draw_convert_rgbaq(colors, vertex_count, (vertex_f_t *) temp_vertices, (color_f_t *) colours);

	// Draw the triangles using triangle primitive type.
	dw = (u64*)draw_prim_start(q,0,&prim, &color);

	for(int i = 0; i < points_count; i++)
	{
		*dw++ = colors[points[i]].rgbaq;
		*dw++ = st[points[i]].uv;
		*dw++ = verts[points[i]].xyz;
	}

	// Check if we're in middle of a qword or not.
	if ((u32)dw % 16) {
		*dw++ = 0;
	}

	q = draw_prim_end((qword_t*)dw,3,DRAW_STQ_REGLIST);

	++q;

	_q = q;
}

void drawRightNear(const struct Vec3 center,
				   const FixP_t scale,
				   const struct Texture *texture,
				   const uint8_t mask,
				   const uint8_t repeatTexture) {

	if ((center.mZ + zCameraOffset) <= 0 ) {
		return;
	}

	bindTexture(texture->raw);

	qword_t *q;
	u64 *dw;
	int points_count = 6;

	int points[6] = {
			0, 1, 2,
			1, 2, 3
	};

	float centerY;
	float centerX;
	float centerZ;
	FixP_t acc;
	FixP_t scaled = Mul(scale, BIAS);
    float textureScale = 1;
	float geometryScale = GEOMETRY_SCALE_Y * (fixToInt(scaled) * REVERSE_BIAS);

	acc = center.mY + playerHeight + walkingBias + yCameraOffset;
	scaled = Mul(acc, BIAS);
	centerY = GEOMETRY_SCALE_Y * (fixToInt(scaled) * REVERSE_BIAS);

	centerX = GEOMETRY_SCALE_X *  (fixToInt(Mul(center.mX + xCameraOffset, BIAS)) * 0.5f * REVERSE_BIAS);
	centerZ = -GEOMETRY_SCALE_Z * (fixToInt(Mul(center.mZ + zCameraOffset, BIAS)) * 0.5f * REVERSE_BIAS);

    float fogAttenuation = 1.0f - (1.0f - (centerZ / FOG_MAX_DISTANCE));

    VECTOR colours[4] = {
            {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
            {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
            {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
            {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
    };

    VECTOR object_position = {centerX, centerY, centerZ, 1.00f};
	VECTOR object_rotation = {0.00f, 0.00f, 0.00f, 1.00f};

	create_local_world(local_world, object_position, object_rotation);

	create_local_screen(local_screen, local_world, world_view, view_screen);

	q = _q;

	if (cameraDirection == kWest || cameraDirection == kEast) {
		VECTOR vertices[4] = {
				{+GEOMETRY_SCALE_X * 0.5f, +geometryScale, -GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{-GEOMETRY_SCALE_X * 0.5f, +geometryScale, +GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{+GEOMETRY_SCALE_X * 0.5f, -geometryScale, -GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{-GEOMETRY_SCALE_X * 0.5f, -geometryScale, +GEOMETRY_SCALE_Z * 0.5f, 1.00f}
		};

   		// Calculate the vertex values.
		calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

	} else {
		VECTOR vertices[4] = {
				{+GEOMETRY_SCALE_X * 0.5f, +geometryScale, +GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{-GEOMETRY_SCALE_X * 0.5f, +geometryScale, -GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{+GEOMETRY_SCALE_X * 0.5f, -geometryScale, +GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{-GEOMETRY_SCALE_X * 0.5f, -geometryScale, -GEOMETRY_SCALE_Z * 0.5f, 1.00f}
		};

		// Calculate the vertex values.
		calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);
	}

    if (repeatTexture) {
        textureScale = geometryScale;
    }

	VECTOR coordinates[4] = {
			{ 1,  0,  0, 0},
			{ 0,  0,  0, 0},
			{ 1,  textureScale,  0, 0},
			{ 0,  textureScale,  0, 0}
	};

	draw_convert_st(st, vertex_count, (vertex_f_t*)temp_vertices, (texel_f_t*)coordinates);

	// Convert floating point vertices to fixed point and translate to center of screen.
	draw_convert_xyz(verts, 2048, 2048, 2048, vertex_count, (vertex_f_t *) temp_vertices);

	// Convert floating point colours to fixed point.
	draw_convert_rgbaq(colors, vertex_count, (vertex_f_t *) temp_vertices, (color_f_t *) colours);

	// Draw the triangles using triangle primitive type.
	dw = (u64*)draw_prim_start(q,0,&prim, &color);

	for(int i = 0; i < points_count; i++)
	{
		*dw++ = colors[points[i]].rgbaq;
		*dw++ = st[points[i]].uv;
		*dw++ = verts[points[i]].xyz;
	}

	// Check if we're in middle of a qword or not.
	if ((u32)dw % 16) {
		*dw++ = 0;
	}

	q = draw_prim_end((qword_t*)dw,3,DRAW_STQ_REGLIST);

	++q;

	_q = q;
}
