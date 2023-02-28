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

extern MATRIX local_screen;
extern packet_t *current;

extern qword_t *_q;
extern xyz_t *verts;
extern color_t *colors;
extern VECTOR *temp_vertices;
extern int vertex_count;
extern prim_t prim;
extern color_t color;



void clearTextures() {
    usedTexture = 0;
}


struct Texture *makeTextureFrom(const char *filename) {
	struct Texture *toReturn =
	(struct Texture *) calloc(1, sizeof(struct Texture));

	return toReturn;
}


void drawQuad(float x, float y, float z) {
	qword_t *q = _q;

	int points_count = 6;

	int points[6] = {
			0, 1, 2,
			1, 2, 3
	};


	VECTOR vertices[4] = {
			{10.00f + x, 10.00f,  10.00f,  1.00f},
			{10.00f + x, 10.00f,  -10.00f, 1.00f},
			{10.00f + x, -10.00f, 10.00f,  1.00f},
			{10.00f + x, -10.00f, -10.00f, 1.00f}
	};

	VECTOR colours[4] = {
			{1.00f, 0.00f, 0.00f, 1.00f},
			{1.00f, 0.00f, 0.00f, 1.00f},
			{1.00f, 0.00f, 0.00f, 1.00f},
			{1.00f, 0.00f, 0.00f, 1.00f}
	};

	// Calculate the vertex values.
	calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

	// Convert floating point vertices to fixed point and translate to center of screen.
	draw_convert_xyz(verts, 2048, 2048, 32, vertex_count, (vertex_f_t *) temp_vertices);

	// Convert floating point colours to fixed point.
	draw_convert_rgbq(colors, vertex_count, (vertex_f_t *) temp_vertices, (color_f_t *) colours, 0x80);


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
