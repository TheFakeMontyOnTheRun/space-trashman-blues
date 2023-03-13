#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

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

#include "Common.h"
#include "Enums.h"
#include "FixP.h"
#include "Vec.h"
#include "Globals.h"
#include "Vec.h"
#include "LoadBitmap.h"
#include "CActor.h"
#include "Core.h"
#include "Engine.h"
#include "Dungeon.h"
#include "MapWithCharKey.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"

framebuffer_t frame;
zbuffer_t zBuffer;
int context = 0;
qword_t *_q;
qword_t *dmatag;
prim_t prim;
color_t color;

xyz_t *verts;
color_t *colors;
VECTOR *temp_vertices;

// The data packets for double buffering dma sends.
packet_t *packets[2];
MATRIX local_world;
MATRIX world_view;
MATRIX view_screen;
MATRIX local_screen;
packet_t *current;

int vertex_count = 4;

int snapshotSignal = '.';


void init_gs() {

	// Define a 32-bit 640x512 framebuffer.
	frame.width = 640;
	frame.height = 512;
	frame.mask = 0;
	frame.psm = GS_PSM_32;
	frame.address = graph_vram_allocate(frame.width, frame.height, frame.psm, GRAPH_ALIGN_PAGE);
	puts("Frame allocated");
	// Enable the zbuffer.
	zBuffer.enable = DRAW_ENABLE;
	zBuffer.mask = 0;
	zBuffer.method = ZTEST_METHOD_GREATER_EQUAL;
	zBuffer.zsm = GS_ZBUF_32;
	printf("graph_vram_allocate %d, %d, %d\n", frame.width, frame.height, zBuffer.zsm);
	zBuffer.address = graph_vram_allocate(frame.width, frame.height, zBuffer.zsm, GRAPH_ALIGN_PAGE);
	puts("Z-Buffer allocated");
	// Initialize the screen and tie the first framebuffer to the read circuits.
	printf("graph_initialize address: %p, width: %u, height: %u, psm: %u\n", frame.address, frame.width, frame.height,
		   frame.psm);
	graph_initialize(frame.address, frame.width, frame.height, frame.psm, 0, 0);
	puts("Graph initialized");
}


void init_drawing_environment() {

	packet_t *packet = packet_init(16, PACKET_NORMAL);

	// This is our generic qword pointer.
	qword_t *q = packet->data;

	// This will setup a default drawing environment.
	q = draw_setup_environment(q, 0, &frame, &zBuffer);

	// Now reset the primitive origin to 2048-width/2,2048-height/2.
	q = draw_primitive_xyoffset(q, 0, (2048 - 320), (2048 - 256));

	// Finish setting up the environment.
	q = draw_finish(q);

	// Now send the packet, no need to wait since it's the first.
	dma_channel_send_normal(DMA_CHANNEL_GIF, packet->data, q - packet->data, 0, 0);
	dma_wait_fast();

	packet_free(packet);

}

void graphicsInit() {

	enableSmoothMovement = TRUE;

	// Init GIF dma channel.

	dma_channel_initialize(DMA_CHANNEL_GIF, NULL, 0);
	dma_channel_fast_waits(DMA_CHANNEL_GIF);
	puts("DMA init done");

	// Init the GS, framebuffer, and zbuffer.
	init_gs();
	puts("GS init done");

	// Init the drawing environment and framebuffer.
	init_drawing_environment();
	puts("DrawEnv init done");

	packets[0] = packet_init(40000, PACKET_NORMAL);
	packets[1] = packet_init(40000, PACKET_NORMAL);

	// Allocate calculation space.
	temp_vertices = memalign(128, sizeof(VECTOR) * vertex_count);

	// Allocate register space.
	verts = memalign(128, sizeof(vertex_t) * vertex_count);
	colors = memalign(128, sizeof(color_t) * vertex_count);

	// Define the triangle primitive we want to use.
	prim.type = PRIM_TRIANGLE;
	prim.shading = PRIM_SHADE_GOURAUD;
	prim.mapping = DRAW_DISABLE;
	prim.fogging = DRAW_DISABLE;
	prim.blending = DRAW_DISABLE;
	prim.antialiasing = DRAW_ENABLE;
	prim.mapping_type = PRIM_MAP_ST;
	prim.colorfix = PRIM_UNFIXED;

	color.r = 0x80;
	color.g = 0x80;
	color.b = 0x80;
	color.a = 0x80;
	color.q = 1.0f;

	// Create the view_screen matrix.
	create_view_screen(view_screen, graph_aspect_ratio(), -8.00f, 8.00f, -8.00f, 8.00f, 1.00f, 1024.00f);

	// Wait for any previous dma transfers to finish before starting.
	dma_wait_fast();
	puts("CreateView done");

	initGL();
}

void handleSystemEvents() {

	uint8_t entry = 0;

	switch (entry) {
		case 2:
			mBufferedCommand = kCommandUp;
			break;
		case 0:
			mBufferedCommand = kCommandRight;
			break;
		case 6:
			mBufferedCommand = kCommandDown;
			break;
		case 4:
			mBufferedCommand = kCommandLeft;
			break;
		case -1:
			mBufferedCommand = kCommandNone;
			break;
	}
}

void graphicsShutdown() {
	packet_free(packets[0]);
	packet_free(packets[1]);
	texturesUsed = 0;
}

void flipRenderer() {
	// Setup a finish event.
	_q = draw_finish(_q);


	// Define our dmatag for the dma chain.
	DMATAG_END(dmatag, (_q - current->data) - 1, 0, 0, 0);

	// Now send our current dma chain.
	dma_wait_fast();
	dma_channel_send_chain(DMA_CHANNEL_GIF, current->data, _q - current->data, 0, 0);


	// Now switch our packets so we can process data while the DMAC is working.
	context ^= 1;

	// Wait for scene to finish drawing
	draw_wait_finish();

	graph_wait_vsync();
}
