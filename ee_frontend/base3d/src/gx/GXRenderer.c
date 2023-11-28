#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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

#include <gccore.h>

#ifdef WII
#include <wiiuse/wpad.h>
#endif

int snapshotSignal = '.';
int needsToRedrawHUD = TRUE;
int enable3DRendering = TRUE;

#define DEFAULT_FIFO_SIZE	(256*1024)

static void *frameBuffer[2] = { NULL, NULL};
GXRModeObj *rmode;
f32 yscale;
u32 xfbHeight;
Mtx view;
Mtx44 perspective;
Mtx model, modelview;
u32	fb = 0; 	// initial framebuffer index
GXColor background = {0, 0, 0, 0xff};

u32 first_frame = 1;

void graphicsInit() {

    enableSmoothMovement = TRUE;
    defaultFont = NULL;

    // init the vi.
    VIDEO_Init();

    rmode = VIDEO_GetPreferredMode(NULL);

#ifdef GX
    PAD_Init();
#endif

#ifdef WII
    WPAD_Init();
#endif

    // allocate 2 framebuffers for double buffering
    frameBuffer[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    frameBuffer[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(frameBuffer[fb]);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

    // setup the fifo and then init the flipper
    void *gp_fifo = NULL;
    gp_fifo = memalign(32,DEFAULT_FIFO_SIZE);
    memset(gp_fifo,0,DEFAULT_FIFO_SIZE);

    GX_Init(gp_fifo,DEFAULT_FIFO_SIZE);

    // clears the bg to color and clears the z buffer
    GX_SetCopyClear(background, 0x00ffffff);

    // other gx setup
    GX_SetViewport(0,0,rmode->fbWidth,rmode->efbHeight,0,1);
    yscale = GX_GetYScaleFactor(rmode->efbHeight,rmode->xfbHeight);
    xfbHeight = GX_SetDispCopyYScale(yscale);
    GX_SetScissor(0,0,rmode->fbWidth,rmode->efbHeight);
    GX_SetDispCopySrc(0,0,rmode->fbWidth,rmode->efbHeight);
    GX_SetDispCopyDst(rmode->fbWidth,xfbHeight);
    GX_SetCopyFilter(rmode->aa,rmode->sample_pattern,GX_TRUE,rmode->vfilter);
    GX_SetFieldMode(rmode->field_rendering,((rmode->viHeight==2*rmode->xfbHeight)?GX_ENABLE:GX_DISABLE));
    GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
    GX_SetCullMode(GX_CULL_NONE);
    GX_CopyDisp(frameBuffer[fb],GX_TRUE);
    GX_SetDispCopyGamma(GX_GM_1_0);


    // setup the vertex descriptor
    // tells the flipper to expect direct data
    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);

    // setup the vertex attribute table
    // describes the data
    // args: vat location 0-7, type of data, data format, size, scale
    // so for ex. in the first call we are sending position data with
    // 3 values X,Y,Z of size F32. scale sets the number of fractional
    // bits for non float data.
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGB8, 0);

    GX_SetNumChans(1);
    GX_SetNumTexGens(TOTAL_TEXTURES);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

    GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

    GX_InvVtxCache();
    GX_InvalidateTexAll();

    // setup our camera at the origin
    // looking down the -z axis with y up
    guVector cam = {0.0F, 0.0F, 0.0F},
            up = {0.0F, 1.0F, 0.0F},
            look = {0.0F, 0.0F, -1.0F};
    guLookAt(view, &cam, &up, &look);
}

void handleSystemEvents() {
#ifdef GX
    PAD_ScanPads();

    s8 tpad = PAD_StickX(0);

    // Rotate left
    if (tpad < -8) {
        mBufferedCommand = kCommandLeft;
    }

    // Rotate right.
    if (tpad > 8) {
        mBufferedCommand = kCommandRight;
    }

    // NOTE: walkbiasangle = head bob
    tpad = PAD_StickY(0);

    // Go forward.
    if(tpad > 50) {
        mBufferedCommand = kCommandUp;
    }

    // Go backward
    if(tpad < -50) {
        mBufferedCommand = kCommandDown;
    }

    tpad = PAD_SubStickY(0);

    if ( PAD_ButtonsDown(0) & PAD_BUTTON_A) {
        mBufferedCommand = kCommandFire1;
    }


    if ( PAD_ButtonsDown(0) & PAD_BUTTON_START) {
        mBufferedCommand = kCommandBack;
    }
#endif

#ifdef WII
    WPAD_ScanPads();

    if (WPAD_ButtonsDown(0) & WPAD_BUTTON_DOWN) {
        mBufferedCommand = kCommandDown;
    }

    if (WPAD_ButtonsDown(0) & WPAD_BUTTON_UP) {
        mBufferedCommand = kCommandUp;
    }

    if (WPAD_ButtonsDown(0) & WPAD_BUTTON_LEFT) {
        mBufferedCommand = kCommandLeft;
    }

    if (WPAD_ButtonsDown(0) & WPAD_BUTTON_RIGHT){
        mBufferedCommand = kCommandRight;
    }

    if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A) {
        mBufferedCommand = kCommandFire1;
    }


    if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B) {
        mBufferedCommand = kCommandBack;
    }

#endif
}

void graphicsShutdown() {
    texturesUsed = 0;
}

void flipRenderer() {
    GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GX_SetColorUpdate(GX_TRUE);
    GX_CopyDisp(frameBuffer[fb],GX_TRUE);

    GX_DrawDone();

    VIDEO_SetNextFramebuffer(frameBuffer[fb]);
    if(first_frame) {
        first_frame = 0;
        VIDEO_SetBlack(FALSE);
    }
    VIDEO_Flush();
    VIDEO_WaitVSync();
    fb ^= 1;
}
