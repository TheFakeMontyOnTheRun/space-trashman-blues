#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#include <kernel.h>
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
#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <dma_tags.h>
#include <gif_tags.h>

#include <gs_privileged.h>
#include <gs_gp.h>
#include <gs_psm.h>

#include <draw.h>
#include <draw2d.h>

#include "Core.h"
#include "FixP.h"
#include "Enums.h"
#include "Common.h"
#include "Vec.h"
#include "CActor.h"
#include "MapWithCharKey.h"
#include "Vec.h"
#include "MapWithCharKey.h"
#include "Dungeon.h"

#include "CTile3DProperties.h"
#include "LoadBitmap.h"
#include "CRenderer.h"

#include "Engine.h"
#include "VisibilityStrategy.h"

uint8_t shouldDrawLights = TRUE;
int useDither = TRUE;

struct Bitmap *defaultFont;


#define NORMALIZE_ORTHO_X (1.0f / 320.0f)
#define NORMALIZE_ORTHO_Y (1.0f / 305.0f)
#define NORMALIZE_COLOUR (2.0f / 256.0f)

#define GEOMETRY_SCALE_X 2.0f
#define GEOMETRY_SCALE_Y 1.0f
#define GEOMETRY_SCALE_Z 2.0f
#define GEOMETRY_TEXTURE_SCALE_X 1.0f
#define GEOMETRY_TEXTURE_SCALE_Y 1.0f

#define BIAS (intToFix(8))
#define REVERSE_BIAS (1.0f/8.0f)
#define FOG_MAX_DISTANCE 32.0f



extern qword_t *_q;
extern xyz_t *verts;
extern texel_t *st;
extern color_t *colors;
extern VECTOR *temp_vertices;
extern int vertex_count;
extern prim_t prim;
extern color_t color;

extern MATRIX local_world;
extern MATRIX world_view;
extern MATRIX view_screen;
extern MATRIX local_screen;
extern packet_t *current;

int submitBitmapToGPU(struct Bitmap* bitmap);

void drawRect(
		const int x,
		const int y,
		const size_t dx,
		const size_t dy,
		const FramebufferPixelFormat pixel) {

}

void fill(
		const int _x,
		const int _y,
		const size_t _dx,
		const size_t _dy,
		const FramebufferPixelFormat pixel,
		const uint8_t stipple) {


    float r, g, b, a;

    r = (pixel & 0xFF) * NORMALIZE_COLOUR;
    g = ((pixel & 0x00FF00) >> 8) * NORMALIZE_COLOUR;
    b = ((pixel & 0xFF0000) >> 16) * NORMALIZE_COLOUR;
    a = stipple ? 0.5f : 1.0f;

    struct Texture *bitmap = itemSprites[1];

    bindTexture(bitmap->raw);

    qword_t *q;

    int points_count = 6;

    int points[6] = {
            0, 1, 2,
            1, 2, 3
    };

    u64 *dw;

    float x = -0.5f + _x * NORMALIZE_ORTHO_X;
    float y = -0.375f + _y * NORMALIZE_ORTHO_Y;
    float dx = _dx * NORMALIZE_ORTHO_X;
    float dy = _dy * NORMALIZE_ORTHO_Y;

    VECTOR object_position = {x, y, -1.0f, 1.0f};
    VECTOR object_rotation = {0.00f, 0.00f, 0.00f, 1.00f};

    create_local_world(local_world, object_position, object_rotation);

    create_local_screen(local_screen, local_world, world_view, view_screen);

    VECTOR vertices[4] = {
            { dx, dy,  0, 1.00f},
            { 0, dy,  0, 1.00f},
            { dx, 0,  0, 1.00f},
            { 0, 0,  0, 1.00f}
    };

    q = _q;

    VECTOR coordinates[4] = {
            { 1,  0,  0, 0},
            { 0,  0,  0, 0},
            { 1,  1,  0, 0},
            { 0,  1,  0, 0}
    };

    VECTOR colours[4] = {
            {r, g, b, a},
            {r, g, b, a},
            {r, g, b, a},
            {r, g, b, a},
    };

    // Calculate the vertex values.
    calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

    draw_convert_st(st, vertex_count, (vertex_f_t*)temp_vertices, (texel_f_t*)coordinates);

    // Convert floating point vertices to fixed point and translate to center of screen.
    draw_convert_xyz(verts, 2048, 2048, 2048, vertex_count, (vertex_f_t *) temp_vertices);

    // Convert floating point colours to fixed point.
    draw_convert_rgbaq(colors, vertex_count, (vertex_f_t *) temp_vertices, (color_f_t *) colours);

    prim_t quad;

    quad.type = PRIM_TRIANGLE;
    quad.shading = PRIM_SHADE_GOURAUD;
    quad.mapping = DRAW_DISABLE;
    quad.fogging = DRAW_DISABLE;
    quad.blending = DRAW_ENABLE;
    quad.antialiasing = DRAW_DISABLE;
    quad.mapping_type = PRIM_MAP_ST;
    quad.colorfix = PRIM_UNFIXED;

    dw = (u64*)draw_prim_start(q,0,&quad, &color);

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

void drawBitmap(const int _x,
				const int _y,
                struct Bitmap *bitmap,
				const uint8_t transparent) {

    float r, g, b, a;
    r = 1.0f;
    g = 1.0f;
    b = 1.0f;
    a = 1.0f;

    if (bitmap->uploadId == -1) {
        submitBitmapToGPU(bitmap);
    }

    bindTexture(bitmap);

    qword_t *q;

    int points_count = 6;

    int points[6] = {
            0, 1, 2,
            1, 2, 3
    };

    u64 *dw;

    float x = -0.5f + _x * NORMALIZE_ORTHO_X;
    float y = -0.375f + _y * NORMALIZE_ORTHO_Y;
    float dx = bitmap->width * NORMALIZE_ORTHO_X;
    float dy = bitmap->height * NORMALIZE_ORTHO_Y;

    VECTOR object_position = {x, y, -1.0f, 1.0f};
    VECTOR object_rotation = {0.00f, 0.00f, 0.00f, 1.00f};

    create_local_world(local_world, object_position, object_rotation);

    create_local_screen(local_screen, local_world, world_view, view_screen);

    VECTOR vertices[4] = {
            { dx, dy,  0, 1.00f},
            { 0, dy,  0, 1.00f},
            { dx, 0,  0, 1.00f},
            { 0, 0,  0, 1.00f}
    };

    q = _q;

    VECTOR coordinates[4] = {
            { 1,  1,  0, 0},
            { 0,  1,  0, 0},
            { 1,  0,  0, 0},
            { 0,  0,  0, 0}
    };

    VECTOR colours[4] = {
            {r, g, b, a},
            {r, g, b, a},
            {r, g, b, a},
            {r, g, b, a},
    };

    // Calculate the vertex values.
    calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

    draw_convert_st(st, vertex_count, (vertex_f_t*)temp_vertices, (texel_f_t*)coordinates);

    // Convert floating point vertices to fixed point and translate to center of screen.
    draw_convert_xyz(verts, 2048, 2048, 2048, vertex_count, (vertex_f_t *) temp_vertices);

    // Convert floating point colours to fixed point.
    draw_convert_rgbaq(colors, vertex_count, (vertex_f_t *) temp_vertices, (color_f_t *) colours);

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

void drawRepeatBitmap(
		const int x,
		const int y,
		const size_t dx,
		const size_t dy,
		const struct Bitmap *tile) {
}

void drawTextAt(const int x, const int y, const char *text, const FramebufferPixelFormat colour) {

}

void drawTextAtWithMarginWithFiltering(const int x, const int y, int margin, const char *__restrict__ text, const uint8_t colour, char charToReplaceHifenWith) {
    drawTextAt( x, y, text, colour);
}

void drawTextAtWithMargin(const int x, const int y, int margin, const char * text, const FramebufferPixelFormat colour) {
    drawTextAt( x, y, text, colour);
}