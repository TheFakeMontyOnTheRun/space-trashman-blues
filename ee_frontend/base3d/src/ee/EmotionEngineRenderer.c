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


prim_t prim;
color_t color;

xyz_t *verts;
color_t *colors;
VECTOR *temp_vertices;

// The data packets for double buffering dma sends.
packet_t *packets[2];
MATRIX view_screen;


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

	packets[0] = packet_init(100, PACKET_NORMAL);
	packets[1] = packet_init(100, PACKET_NORMAL);

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
	create_view_screen(view_screen, graph_aspect_ratio(), -3.00f, 3.00f, -3.00f, 3.00f, 1.00f, 2000.00f);

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

qword_t *drawQuad(qword_t *q, packet_t *current, MATRIX local_screen, float x, float y, float z);

void flipRenderer() {

	packet_t *current = packets[context];
	MATRIX local_screen;
	MATRIX local_world;
	MATRIX world_view;


	VECTOR object_position = {0.00f, 0.00f, 0.00f, 1.00f};
	VECTOR object_rotation = {0.00f, 0.00f, 0.00f, 1.00f};

	VECTOR camera_position = {0.00f, 0.00f, 100.00f, 1.00f};
	VECTOR camera_rotation = {0.00f, 0.00f, 0.00f, 1.00f};

	// Spin the cube a bit.
	object_rotation[0] += 0.008f; //while (object_rotation[0] > 3.14f) { object_rotation[0] -= 6.28f; }
	object_rotation[1] += 0.012f; //while (object_rotation[1] > 3.14f) { object_rotation[1] -= 6.28f; }

	// Create the local_world matrix.
	create_local_world(local_world, object_position, object_rotation);

	// Create the world_view matrix.
	create_world_view(world_view, camera_position, camera_rotation);

	// Create the local_screen matrix.
	create_local_screen(local_screen, local_world, world_view, view_screen);

	// Grab our dmatag pointer for the dma chain.
	qword_t *dmatag;

	dmatag = current->data;

	// Now grab our qword pointer and increment past the dmatag.
	qword_t *q;
	q = dmatag;
	q++;

	// Clear framebuffer but don't update zbuffer.
	q = draw_disable_tests(q, 0, &zBuffer);
	q = draw_clear(q, 0, 2048.0f - 320.0f, 2048.0f - 256.0f, frame.width, frame.height, 0x00, 0x00, 0x00);
	q = draw_enable_tests(q, 0, &zBuffer);

	q = drawQuad(q, current, local_screen, 0.0f, 0.0f, 0.0f);
	q = drawQuad(q, current, local_screen, 10.0f, 0.0f, 0.0f);

	// Setup a finish event.
	q = draw_finish(q);


	// Define our dmatag for the dma chain.
	DMATAG_END(dmatag, (q - current->data) - 1, 0, 0, 0);

	// Now send our current dma chain.
	dma_wait_fast();
	dma_channel_send_chain(DMA_CHANNEL_GIF, current->data, q - current->data, 0, 0);


	// Now switch our packets so we can process data while the DMAC is working.
	context ^= 1;

	// Wait for scene to finish drawing
	draw_wait_finish();

	graph_wait_vsync();


}