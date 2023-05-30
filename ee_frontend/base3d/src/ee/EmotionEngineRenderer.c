#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

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


#include "libpad.h"


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

static char padBuf[256] __attribute__((aligned(64)));

static char actAlign[6];
static int actuators;

float leanX = 0.0f;
float leanY = 0.0f;

framebuffer_t frame;
zbuffer_t zBuffer;
int context = 0;
qword_t *_q;
qword_t *dmatag;
prim_t prim;
color_t color;

xyz_t *verts;
color_t *colors;
texel_t *st;
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

int ret;
int port, slot;
struct padButtonStatus buttons;
u32 paddata;
u32 old_pad = 0;
u32 new_pad;


qword_t *draw_setup_environment_with_repeat(qword_t *q, int context, framebuffer_t *frame, zbuffer_t *z)
{

	// Change this if modifying the gif packet after the giftag.
	int qword_count = 15;

	atest_t atest;
	dtest_t  dtest;
	ztest_t ztest;
	blend_t blend;
	texwrap_t wrap;

	atest.enable = DRAW_ENABLE;
	atest.method = ATEST_METHOD_GREATER;
	atest.compval = 0x0;
	atest.keep = ATEST_KEEP_ALL;

	dtest.enable = DRAW_DISABLE;
	dtest.pass = DRAW_DISABLE;

	// Enable or Disable ZBuffer
	if (z->enable)
	{
		ztest.enable = DRAW_ENABLE;
		ztest.method = z->method;
	}
	else
	{
		z->mask = 1;
		ztest.enable = DRAW_ENABLE;
		ztest.method = ZTEST_METHOD_ALLPASS;
	}

	// Setup alpha blending
	blend.color1 = BLEND_COLOR_SOURCE;
	blend.color2 = BLEND_COLOR_DEST;
	blend.alpha  = BLEND_ALPHA_SOURCE;
	blend.color3 = BLEND_COLOR_DEST;
	blend.fixed_alpha = 0x80;

	// Setup whole texture clamping
	wrap.horizontal = WRAP_REPEAT;
	wrap.vertical = WRAP_REPEAT;
	wrap.minu = wrap.maxu = 0;
	wrap.minv = wrap.maxv = 0;

	// Begin packed gif data packet with another qword.
	PACK_GIFTAG(q,GIF_SET_TAG(qword_count,0,0,0,GIF_FLG_PACKED,1),GIF_REG_AD);
	q++;
	// Framebuffer setting
	PACK_GIFTAG(q, GS_SET_FRAME(frame->address>>11,frame->width>>6,frame->psm,frame->mask), GS_REG_FRAME + context);
	q++;
	// ZBuffer setting
	PACK_GIFTAG(q, GS_SET_ZBUF(z->address>>11,z->zsm,z->mask), GS_REG_ZBUF + context);
	q++;
	// Override Primitive Control
	PACK_GIFTAG(q, GS_SET_PRMODECONT(PRIM_OVERRIDE_DISABLE),GS_REG_PRMODECONT);
	q++;
	// Primitive coordinate offsets
	PACK_GIFTAG(q, GS_SET_XYOFFSET(ftoi4(2048.0f),ftoi4(2048.0f)), GS_REG_XYOFFSET + context);
	q++;
	// Scissoring area
	PACK_GIFTAG(q, GS_SET_SCISSOR(0,frame->width-1,0,frame->height-1), GS_REG_SCISSOR + context);
	q++;
	// Pixel testing
	PACK_GIFTAG(q, GS_SET_TEST(atest.enable,atest.method,atest.compval,atest.keep,
							   dtest.enable,dtest.pass,
							   ztest.enable,ztest.method), GS_REG_TEST + context);
	q++;
	// Fog Color
	PACK_GIFTAG(q, GS_SET_FOGCOL(0,0,0), GS_REG_FOGCOL);
	q++;
	// Per-pixel Alpha Blending (Blends if MSB of ALPHA is true)
	PACK_GIFTAG(q, GS_SET_PABE(DRAW_DISABLE), GS_REG_PABE);
	q++;
	// Alpha Blending
	PACK_GIFTAG(q, GS_SET_ALPHA(blend.color1,blend.color2,blend.alpha,
								blend.color3,blend.fixed_alpha), GS_REG_ALPHA + context);
	q++;
	// Dithering
	PACK_GIFTAG(q, GS_SET_DTHE(GS_DISABLE), GS_REG_DTHE);
	q++;
	PACK_GIFTAG(q, GS_SET_DIMX(4,2,5,3,0,6,1,7,5,3,4,2,1,7,0,6), GS_REG_DIMX);
	q++;
	// Color Clamp
	PACK_GIFTAG(q,GS_SET_COLCLAMP(GS_ENABLE),GS_REG_COLCLAMP);
	q++;
	// Alpha Correction
	if ((frame->psm == GS_PSM_16) || (frame->psm == GS_PSM_16S))
	{
		PACK_GIFTAG(q,GS_SET_FBA(ALPHA_CORRECT_RGBA16),GS_REG_FBA + context);
		q++;
	}
	else
	{
		PACK_GIFTAG(q,GS_SET_FBA(ALPHA_CORRECT_RGBA32),GS_REG_FBA + context);
		q++;
	}
	// Texture wrapping/clamping
	PACK_GIFTAG(q, GS_SET_CLAMP(wrap.horizontal,wrap.vertical,wrap.minu,
								wrap.maxu,wrap.minv,wrap.maxv), GS_REG_CLAMP + context);
	q++;
	PACK_GIFTAG(q, GS_SET_TEXA(0x80,ALPHA_EXPAND_NORMAL,0x80),GS_REG_TEXA);
	q++;

	return q;

}

void init_gs() {

	// Define a 32-bit 640x512 framebuffer.
	frame.width = 640;
	frame.height = 512;
	frame.mask = 0;
	frame.psm = GS_PSM_32;
	frame.address = graph_vram_allocate(frame.width, frame.height, frame.psm, GRAPH_ALIGN_PAGE);

	// Enable the zbuffer.
	zBuffer.enable = DRAW_ENABLE;
	zBuffer.mask = 0;
	zBuffer.method = ZTEST_METHOD_GREATER_EQUAL;
	zBuffer.zsm = GS_ZBUF_32;
	zBuffer.address = graph_vram_allocate(frame.width, frame.height, zBuffer.zsm, GRAPH_ALIGN_PAGE);

	// Initialize the screen and tie the first framebuffer to the read circuits.
	graph_initialize(frame.address, frame.width, frame.height, frame.psm, 0, 0);
}


void init_drawing_environment() {

	packet_t *packet = packet_init(16, PACKET_NORMAL);

	// This is our generic qword pointer.
	qword_t *q = packet->data;

	// This will setup a default drawing environment.
	q = draw_setup_environment_with_repeat(q, 0, &frame, &zBuffer);

	// Now reset the primitive origin to 2048-width/2,2048-height/2.
	q = draw_primitive_xyoffset(q, 0, (2048 - 320), (2048 - 256));

	// Finish setting up the environment.
	q = draw_finish(q);

	// Now send the packet, no need to wait since it's the first.
	dma_channel_send_normal(DMA_CHANNEL_GIF, packet->data, q - packet->data, 0, 0);
	dma_wait_fast();

	packet_free(packet);

}


/*
 * waitPadReady()
 */
static int waitPadReady(int port, int slot)
{
	int state;
	int lastState;
	char stateString[16];

	state = padGetState(port, slot);
	lastState = -1;
	while((state != PAD_STATE_STABLE) && (state != PAD_STATE_FINDCTP1)) {
		if (state != lastState) {
			padStateInt2String(state, stateString);
			printf("Please wait, pad(%d,%d) is in state %s\n",
				   port, slot, stateString);
		}
		lastState = state;
		state=padGetState(port, slot);
	}
	// Were the pad ever 'out of sync'?
	if (lastState != -1) {
		printf("Pad OK!\n");
	}
	return 0;
}

/*
 * initializePad()
 */
static int
initializePad(int port, int slot)
{

	int ret;
	int modes;
	int i;

	waitPadReady(port, slot);

	// How many different modes can this device operate in?
	// i.e. get # entrys in the modetable
	modes = padInfoMode(port, slot, PAD_MODETABLE, -1);
	printf("The device has %d modes\n", modes);

	if (modes > 0) {
		printf("( ");
		for (i = 0; i < modes; i++) {
			printf("%d ", padInfoMode(port, slot, PAD_MODETABLE, i));
		}
		printf(")");
	}

	printf("It is currently using mode %d\n",
		   padInfoMode(port, slot, PAD_MODECURID, 0));

	// If modes == 0, this is not a Dual shock controller
	// (it has no actuator engines)
	if (modes == 0) {
		printf("This is a digital controller?\n");
		return 1;
	}

	// Verify that the controller has a DUAL SHOCK mode
	i = 0;
	do {
		if (padInfoMode(port, slot, PAD_MODETABLE, i) == PAD_TYPE_DUALSHOCK)
			break;
		i++;
	} while (i < modes);
	if (i >= modes) {
		printf("This is no Dual Shock controller\n");
		return 1;
	}

	// If ExId != 0x0 => This controller has actuator engines
	// This check should always pass if the Dual Shock test above passed
	ret = padInfoMode(port, slot, PAD_MODECUREXID, 0);
	if (ret == 0) {
		printf("This is no Dual Shock controller??\n");
		return 1;
	}

	printf("Enabling dual shock functions\n");

	// When using MMODE_LOCK, user cant change mode with Select button
	padSetMainMode(port, slot, PAD_MMODE_DUALSHOCK, PAD_MMODE_LOCK);

	waitPadReady(port, slot);
	printf("infoPressMode: %d\n", padInfoPressMode(port, slot));

	waitPadReady(port, slot);
	printf("enterPressMode: %d\n", padEnterPressMode(port, slot));

	waitPadReady(port, slot);
	actuators = padInfoAct(port, slot, -1, 0);
	printf("# of actuators: %d\n",actuators);

	if (actuators != 0) {
		actAlign[0] = 0;   // Enable small engine
		actAlign[1] = 1;   // Enable big engine
		actAlign[2] = 0xff;
		actAlign[3] = 0xff;
		actAlign[4] = 0xff;
		actAlign[5] = 0xff;

		waitPadReady(port, slot);
		printf("padSetActAlign: %d\n",
			   padSetActAlign(port, slot, actAlign));
	}
	else {
		printf("Did not find any actuators.\n");
	}

	waitPadReady(port, slot);

	return 1;
}

static void
loadModules(void)
{
	int ret;


	ret = SifLoadModule("rom0:SIO2MAN", 0, NULL);
	if (ret < 0) {
		printf("sifLoadModule sio failed: %d\n", ret);
		SleepThread();
	}

	ret = SifLoadModule("rom0:PADMAN", 0, NULL);
	if (ret < 0) {
		printf("sifLoadModule pad failed: %d\n", ret);
		SleepThread();
	}
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
	st    = memalign(128, sizeof(texel_t) * vertex_count);

	// Define the triangle primitive we want to use.
	prim.type = PRIM_TRIANGLE;
	prim.shading = PRIM_SHADE_GOURAUD;
	prim.mapping = DRAW_ENABLE;
	prim.fogging = DRAW_DISABLE;
	prim.blending = DRAW_ENABLE;
	prim.antialiasing = DRAW_DISABLE;
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

	old_pad = 0;

	SifInitRpc(0);

	loadModules();

	padInit(0);

	port = 0; // 0 -> Connector 1, 1 -> Connector 2
	slot = 0; // Always zero if not using multitap

	printf("PortMax: %d\n", padGetPortMax());
	printf("SlotMax: %d\n", padGetSlotMax(port));


	if((ret = padPortOpen(port, slot, padBuf)) == 0) {
		printf("padOpenPort failed: %d\n", ret);
		SleepThread();
	}

	if(!initializePad(port, slot)) {
		printf("pad initalization failed!\n");
		SleepThread();
	}

	puts("ALL GOOD IN EE LAND!");
    defaultFont = NULL;
}

int framePad = 0;
void handleSystemEvents() {

	framePad++;
//	printf("Frame pad %d\n", framePad);
	uint8_t entry = 0;
	mBufferedCommand = kCommandNone;

	ret=padGetState(port, slot);
	while((ret != PAD_STATE_STABLE) && (ret != PAD_STATE_FINDCTP1)) {
		if(ret==PAD_STATE_DISCONN) {
			printf("Pad(%d, %d) is disconnected\n", port, slot);
		}
		ret=padGetState(port, slot);
	}

	ret = padRead(port, slot, &buttons); // port, slot, buttons

	if (ret != 0) {

        paddata = 0xffff ^ buttons.btns;

        new_pad = paddata & ~old_pad;
        old_pad = paddata;

        // Directions
        if (new_pad & PAD_LEFT) {
            printf("LEFT\n");
            mBufferedCommand = kCommandLeft;
        }
        if (new_pad & PAD_DOWN) {
            printf("DOWN\n");
            mBufferedCommand = kCommandDown;
        }
        if (new_pad & PAD_RIGHT) {
            printf("RIGHT\n");
            mBufferedCommand = kCommandRight;
        }
        if (new_pad & PAD_UP) {
            printf("UP\n");
            mBufferedCommand = kCommandUp;
        }
        if (new_pad & PAD_START) {
            printf("START\n");
            mBufferedCommand = kCommandFire1;
        }
        if (new_pad & PAD_R3) {
            printf("R3\n");
        }
        if (new_pad & PAD_L3) {
            printf("L3\n");
        }
        if (new_pad & PAD_SELECT) {
            printf("SELECT\n");
            mBufferedCommand = kCommandBack;
        }
        if (new_pad & PAD_SQUARE) {
            printf("SQUARE\n");
            mBufferedCommand = kCommandFire3;
        }
        if (new_pad & PAD_CROSS) {
            printf("CROSS - Enter press mode\n");
            mBufferedCommand = kCommandFire4;
        }
        if (new_pad & PAD_CIRCLE) {
            printf("CIRCLE - Exit press mode\n");
            mBufferedCommand = kCommandFire1;
        }
        if (new_pad & PAD_TRIANGLE) {
            // Check for the reason below..
            printf("TRIANGLE (press mode disabled, see code)\n");
            mBufferedCommand = kCommandFire2;
        }
        if (new_pad & PAD_R1) {
            printf("R1 - Start little engine\n");
            mBufferedCommand = kCommandStrafeRight;
        }
        if (new_pad & PAD_L1) {
            printf("L1 - Stop little engine\n");
            mBufferedCommand = kCommandStrafeLeft;
        }
        if (new_pad & PAD_R2) {
            printf("R2\n");
        }

        if (new_pad & PAD_L2) {
            printf("L2\n");
        }

        leanX = 0.0f;
        leanY = 0.0f;

        if( buttons.rjoy_h > 127) {
            leanX = -0.25f * ((buttons.rjoy_h - 127) / 128.0f);
        }

        if( buttons.rjoy_h < 127) {
            leanX = 0.25f * ((128 - buttons.rjoy_h) / 127.0f);
        }

        if( buttons.rjoy_v > 127) {
            leanY = -0.25f * ((buttons.rjoy_v - 127) / 128.0f);
        }

        if( buttons.rjoy_v < 127) {
            leanY = 0.25f * ((128 - buttons.rjoy_v) / 127.0f);
        }
    }
}

void graphicsShutdown() {
	packet_free(packets[0]);
	packet_free(packets[1]);
	texturesUsed = 0;
}

void flipRenderer() {
	_q = draw_finish(_q);
    graph_wait_vsync();
	dma_channel_send_normal(DMA_CHANNEL_GIF, current->data, _q - current->data, 0, 0);
	context ^= 1;
	draw_wait_finish();

}
