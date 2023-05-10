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
#define NORMALIZE_ORTHO_Y (1.0f / 200.0f)
#define ADJUST_RESOLUTION_Y (((200.0f/256.0f) * 200.0f) / 240.0f )
#define OFFSET_X (-0.5f)
#define OFFSET_Y (-0.375f)
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

    fill(x, y, 1, dy - 1, pixel, 0);

    fill(x, y, dx - 1, 1, pixel, 0);

    fill(x + dx - 1, y, 1, dy - 1, pixel, 0);

    fill(x, y + dy - 1, dx - 1, 1, pixel, 0);
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

    if (defaultFont == NULL) {
        defaultFont = loadBitmap("font.img");
    }

    if (defaultFont->uploadId == -1) {
        submitBitmapToGPU(defaultFont);
    }

    bindTexture(defaultFont);

    qword_t *q;

    int points_count = 6;

    int points[6] = {
            0, 1, 2,
            1, 2, 3
    };

    u64 *dw;

    float x = OFFSET_X + _x * NORMALIZE_ORTHO_X;
    float y = OFFSET_Y + _y * NORMALIZE_ORTHO_Y * ADJUST_RESOLUTION_Y;
    float dx = _dx * NORMALIZE_ORTHO_X;
    float dy = _dy * NORMALIZE_ORTHO_Y * ADJUST_RESOLUTION_Y;

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

void drawBitmapRegion(const int _x,
                      const int _y,
                      const int _dx,
                      const int _dy,
                      BitmapPixelFormat tint,
                      struct Bitmap *bitmap,
                      const uint8_t transparent,
                      float u0, float u1, float v0, float v1) {
    float r, g, b, a;
    r = (tint & 0xFF) * NORMALIZE_COLOUR * 0.5f;
    g = ((tint & 0x00FF00) >> 8) * NORMALIZE_COLOUR * 0.5f;
    b = ((tint & 0xFF0000) >> 16) * NORMALIZE_COLOUR * 0.5f;
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

    float x = OFFSET_X + _x * NORMALIZE_ORTHO_X;
    float y = OFFSET_Y + _y * NORMALIZE_ORTHO_Y * ADJUST_RESOLUTION_Y;
    float dx = _dx * NORMALIZE_ORTHO_X;
    float dy = _dy * NORMALIZE_ORTHO_Y * ADJUST_RESOLUTION_Y;

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
            { u1,  v1,  0, 0},
            { u0,  v1,  0, 0},
            { u1,  v0,  0, 0},
            { u0,  v0,  0, 0}
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

void drawBitmap(const int _x,
				const int _y,
                struct Bitmap *bitmap,
				const uint8_t transparent) {
    drawBitmapRegion(_x, _y, bitmap->width, bitmap->height, getPaletteEntry(0xFFFFFFFF), bitmap, transparent, 0.0f, 0.999f, 0.0f, 0.999f);
}

void drawRepeatBitmap(
		const int x,
		const int y,
		const size_t dx,
		const size_t dy,
		const struct Bitmap *tile) {

    size_t repeatX = (dx / tile->width) + 1;
    size_t repeatY = (dy / tile->height) + 1;
    size_t c, d;
    for (c = 0; c < repeatY; ++c) {
        for (d = 0; d < repeatX; ++d) {

            size_t px = x + d * tile->width;
            size_t py = y + c * tile->height;

            if (px < XRES_FRAMEBUFFER && py < YRES_FRAMEBUFFER) {
                drawBitmap(px, py, tile, FALSE);
            }
        }
    }
}

void drawTextAt(const int _x, const int _y, const char *text, const FramebufferPixelFormat colour) {

    if (defaultFont == NULL) {
        defaultFont = loadBitmap("font.img");
    }

    if (defaultFont->uploadId == -1) {
        submitBitmapToGPU(defaultFont);
    }

    size_t len = strlen(text);
    int32_t dstX = (_x - 1) * 8;
    int32_t dstY = (_y - 1) * 8;
    size_t c;
    uint32_t ascii;
    float line;
    float col;

    float fontWidth = defaultFont->width;
    float fontHeight = defaultFont->height;
    float blockWidth = (8.0f / fontWidth) * 0.999f;
    float blockHeight = (8.0f / fontHeight) * 0.999f;

    for (c = 0; c < len; ++c) {
        if (text[c] == '\n' || dstX >= XRES_FRAMEBUFFER) {
            dstX = (_x - 1) * 8;
            dstY += 8;
            continue;
        }

        if (text[c] == ' ' || text[c] == '\r') {
            dstX += 8;
            continue;
        }

        ascii = text[c] - ' ';

        line = (((float)((ascii >> 5))) * blockHeight);
        col = (((ascii & 31)) * blockWidth);

        drawBitmapRegion(dstX, dstY, 8, 8, colour, defaultFont, 1, col, col + blockWidth, line, line + blockHeight);

        dstX += 8;
    }
}

void drawTextAtWithMarginWithFiltering(const int x, const int y, int margin, const char *__restrict__ text, const uint8_t colour, char charToReplaceHifenWith) {
    drawTextAt( x, y, text, colour);
}

void drawTextAtWithMargin(const int x, const int y, int margin, const char * text, const FramebufferPixelFormat colour) {
    drawTextAt( x, y, text, colour);
}