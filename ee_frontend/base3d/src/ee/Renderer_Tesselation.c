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
int usedTexture = 0;

extern MATRIX local_world;
extern MATRIX world_view;
extern MATRIX view_screen;
extern MATRIX local_screen;
extern packet_t *current;

extern qword_t *_q;
extern xyz_t *verts;
extern color_t *colors;
extern VECTOR *temp_vertices;
extern int vertex_count;
extern prim_t prim;
extern color_t color;

#define GEOMETRY_SCALE_X 2.0f
#define GEOMETRY_SCALE_Y 2.0f
#define GEOMETRY_SCALE_Z 2.0f
#define GEOMETRY_TEXTURE_SCALE_X 1.0f
#define GEOMETRY_TEXTURE_SCALE_Y 1.0f

#define BIAS (intToFix(8))
#define REVERSE_BIAS (1.0f/8.0f)


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

	if (center.mZ <= 0 ) {
		return;
	}

	qword_t *q;

	int points_count = 6;

	int points[6] = {
			0, 1, 2,
			1, 2, 3
	};

	VECTOR colours[4] = {
			{0.00f, 0.00f, 0.00f, 1.00f},
			{0.30f, 0.00f, 0.00f, 1.00f},
			{0.60f, 0.00f, 0.00f, 1.00f},
			{0.90f, 0.00f, 0.00f, 1.00f}
	};

	// Convert floating point colours to fixed point.
	draw_convert_rgbq(colors, vertex_count, (vertex_f_t *) temp_vertices, (color_f_t *) colours, 0x80);


	float centerY;
	float centerX;
	float centerZ;
	FixP_t acc;
	FixP_t scaled = Mul(scale, BIAS);
	float geometryScale = GEOMETRY_SCALE_Y * (fixToInt(scaled) * REVERSE_BIAS);
/*
	if (!repeatTexture) {
		textureScale = 1;
	}
*/
	acc = center.mY + playerHeight + walkingBias + yCameraOffset;
	scaled = Mul(acc, BIAS);
	centerY = GEOMETRY_SCALE_Y * (fixToInt(scaled) * REVERSE_BIAS);

	centerX = GEOMETRY_SCALE_X *  (fixToInt(Mul(center.mX, BIAS)) * 0.5f * REVERSE_BIAS);
	centerZ = -GEOMETRY_SCALE_Z * (fixToInt(Mul(center.mZ, BIAS)) * 0.5f * REVERSE_BIAS);

	VECTOR object_position = {centerX, centerY, centerZ, 1.00f};
	VECTOR object_rotation = {0.00f, 0.00f, 0.00f, 1.00f};

	create_local_world(local_world, object_position, object_rotation);

	create_local_screen(local_screen, local_world, world_view, view_screen);


	if ((mask & MASK_BEHIND)) {

		VECTOR vertices[4] = {
				{ + GEOMETRY_SCALE_X * 0.5f, + geometryScale, - GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ - GEOMETRY_SCALE_X * 0.5f, + geometryScale, - GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ + GEOMETRY_SCALE_X * 0.5f, - geometryScale, - GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ - GEOMETRY_SCALE_X * 0.5f, - geometryScale, - GEOMETRY_SCALE_Z * 0.5f, 1.00f}
		};

		q = _q;

		// Calculate the vertex values.
		calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

		// Convert floating point vertices to fixed point and translate to center of screen.
		draw_convert_xyz(verts, 2048, 2048, 2048, vertex_count, (vertex_f_t *) temp_vertices);

		// Draw the triangles using triangle primitive type.
		q = draw_prim_start(q, 0, &prim, &color);

		for (int i = 0; i < points_count; i++) {
			q->dw[0] = colors[points[i]].rgbaq;
			q->dw[1] = verts[points[i]].xyz;
			q++;
		}

		q = draw_prim_end(q, 2, DRAW_RGBAQ_REGLIST);
		++q;

		_q = q;
	}

	if (((mask & MASK_RIGHT) && fixToInt(center.mX) > 0) || (mask & MASK_FORCE_RIGHT)) {
		VECTOR vertices[4] = {
				{ - GEOMETRY_SCALE_X * 0.5f,  + geometryScale,  + GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ - GEOMETRY_SCALE_X * 0.5f,  + geometryScale,  - GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ - GEOMETRY_SCALE_X * 0.5f,  - geometryScale,  + GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ - GEOMETRY_SCALE_X * 0.5f,  - geometryScale,  - GEOMETRY_SCALE_Z * 0.5f, 1.00f}
		};

		q = _q;

		// Calculate the vertex values.
		calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

		// Convert floating point vertices to fixed point and translate to center of screen.
		draw_convert_xyz(verts, 2048, 2048, 2048, vertex_count, (vertex_f_t *) temp_vertices);

		// Draw the triangles using triangle primitive type.
		q = draw_prim_start(q, 0, &prim, &color);

		for (int i = 0; i < points_count; i++) {
			q->dw[0] = colors[points[i]].rgbaq;
			q->dw[1] = verts[points[i]].xyz;
			q++;
		}

		q = draw_prim_end(q, 2, DRAW_RGBAQ_REGLIST);
		++q;

		_q = q;
	}

	if (((mask & MASK_LEFT) && fixToInt(center.mX) < 0) || (mask & MASK_FORCE_LEFT)) {
		VECTOR vertices[4] = {
				{ + GEOMETRY_SCALE_X * 0.5f,  + geometryScale,  + GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ + GEOMETRY_SCALE_X * 0.5f,  + geometryScale,  - GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ + GEOMETRY_SCALE_X * 0.5f,  - geometryScale,  + GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ + GEOMETRY_SCALE_X * 0.5f,  - geometryScale,  - GEOMETRY_SCALE_Z * 0.5f, 1.00f}
		};

		q = _q;

		// Calculate the vertex values.
		calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

		// Convert floating point vertices to fixed point and translate to center of screen.
		draw_convert_xyz(verts, 2048, 2048, 2048, vertex_count, (vertex_f_t *) temp_vertices);

		// Draw the triangles using triangle primitive type.
		q = draw_prim_start(q, 0, &prim, &color);

		for (int i = 0; i < points_count; i++) {
			q->dw[0] = colors[points[i]].rgbaq;
			q->dw[1] = verts[points[i]].xyz;
			q++;
		}

		q = draw_prim_end(q, 2, DRAW_RGBAQ_REGLIST);
		++q;

		_q = q;
	}

	if ((mask & MASK_FRONT)) {
		VECTOR vertices[4] = {
				{ + GEOMETRY_SCALE_X * 0.5f,  + geometryScale,  + GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ - GEOMETRY_SCALE_X * 0.5f,  + geometryScale,  + GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ + GEOMETRY_SCALE_X * 0.5f,  - geometryScale,  + GEOMETRY_SCALE_Z * 0.5f, 1.00f},
				{ - GEOMETRY_SCALE_X * 0.5f,  - geometryScale,  + GEOMETRY_SCALE_Z * 0.5f, 1.00f}
		};

		q = _q;

		// Calculate the vertex values.
		calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

		// Convert floating point vertices to fixed point and translate to center of screen.
		draw_convert_xyz(verts, 2048, 2048, 2048, vertex_count, (vertex_f_t *) temp_vertices);

		// Draw the triangles using triangle primitive type.
		q = draw_prim_start(q, 0, &prim, &color);

		for (int i = 0; i < points_count; i++) {
			q->dw[0] = colors[points[i]].rgbaq;
			q->dw[1] = verts[points[i]].xyz;
			q++;
		}

		q = draw_prim_end(q, 2, DRAW_RGBAQ_REGLIST);
		++q;

		_q = q;
	}

}

void drawFloorAt(const struct Vec3 center,
				 const struct Texture *texture, enum EDirection cameraDirection) {

	if (center.mY <= 0 && center.mZ > 0  ) {

		float centerY;
		FixP_t acc;
		FixP_t scaled;
		int x[4], y[4];

		acc = center.mY + playerHeight + walkingBias + yCameraOffset;
		scaled = Mul(acc, BIAS);
		centerY = GEOMETRY_SCALE_Y * (fixToInt(scaled) * REVERSE_BIAS);

		float centerX;
		float centerZ;

		centerX = GEOMETRY_SCALE_X *  (fixToInt(Mul(center.mX, BIAS)) * 0.5f * REVERSE_BIAS);
		centerZ = -GEOMETRY_SCALE_Z * (fixToInt(Mul(center.mZ, BIAS)) * 0.5f * REVERSE_BIAS);

		VECTOR object_position = {centerX, centerY, centerZ, 1.00f};
		VECTOR object_rotation = {0.00f, 0.00f, 0.00f, 1.00f};

		// Create the local_world matrix.
		create_local_world(local_world, object_position, object_rotation);

		// Create the local_screen matrix.
		create_local_screen(local_screen, local_world, world_view, view_screen);

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

		qword_t *q;

		int points_count = 6;

		int points[6] = {
				0, 1, 2,
				1, 2, 3
		};

		VECTOR colours[4] = {
				{0.00f, 0.00f, 0.00f, 1.00f},
				{0.00f, 0.00f, 0.3f, 1.00f},
				{0.00f, 0.00f, 0.6f, 1.00f},
				{0.00f, 0.00f, 0.9f, 1.00f}
		};

		// Convert floating point colours to fixed point.
		draw_convert_rgbq(colors, vertex_count, (vertex_f_t *) temp_vertices, (color_f_t *) colours, 0x80);

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
		q = draw_prim_start(q, 0, &prim, &color);

		for (int i = 0; i < points_count; i++) {
			q->dw[0] = colors[points[i]].rgbaq;
			q->dw[1] = verts[points[i]].xyz;
			q++;
		}

		q = draw_prim_end(q, 2, DRAW_RGBAQ_REGLIST);
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

		acc = center.mY + playerHeight + walkingBias + yCameraOffset;
		scaled = Mul(acc, BIAS);
		centerY = GEOMETRY_SCALE_Y * (fixToInt(scaled) * REVERSE_BIAS);

		float centerX;
		float centerZ;

		centerX = GEOMETRY_SCALE_X *  (fixToInt(Mul(center.mX, BIAS)) * 0.5f * REVERSE_BIAS);
		centerZ = -GEOMETRY_SCALE_Z * (fixToInt(Mul(center.mZ, BIAS)) * 0.5f * REVERSE_BIAS);

		VECTOR object_position = {centerX, centerY, centerZ, 1.00f};
		VECTOR object_rotation = {0.00f, 0.00f, 0.00f, 1.00f};

		// Create the local_world matrix.
		create_local_world(local_world, object_position, object_rotation);

		// Create the local_screen matrix.
		create_local_screen(local_screen, local_world, world_view, view_screen);

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

		qword_t *q;

		int points_count = 6;

		int points[6] = {
				0, 1, 2,
				1, 2, 3
		};

		VECTOR colours[4] = {
				{0.00f, 0.00f, 0.00f, 1.00f},
				{0.00f, 0.3f, 0.00f, 1.00f},
				{0.00f, 0.6f, 0.00f, 1.00f},
				{0.00f, 0.9f, 0.00f, 1.00f}
		};

		// Convert floating point colours to fixed point.
		draw_convert_rgbq(colors, vertex_count, (vertex_f_t *) temp_vertices, (color_f_t *) colours, 0x80);

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
		q = draw_prim_start(q, 0, &prim, &color);

		for (int i = 0; i < points_count; i++) {
			q->dw[0] = colors[points[i]].rgbaq;
			q->dw[1] = verts[points[i]].xyz;
			q++;
		}

		q = draw_prim_end(q, 2, DRAW_RGBAQ_REGLIST);
		++q;

		_q = q;
	}
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
