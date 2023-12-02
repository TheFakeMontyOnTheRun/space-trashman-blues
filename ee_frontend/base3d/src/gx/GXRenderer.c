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
guVector cubeAxis = {1,1,1};
f32 rquad = 0.0f;
f32 zt = 0;
void *gpfifo = NULL;

uint8_t whiteTextureData[ 32 * 32 * 4];
GXTexObj whiteTextureObj;

void graphicsInit() {

    enableSmoothMovement = TRUE;
    defaultFont = NULL;

    VIDEO_Init();


#ifdef GX
    PAD_Init();
#endif

#ifdef WII
    WPAD_Init();
#endif

    rmode = VIDEO_GetPreferredMode(NULL);

    // allocate the fifo buffer
    gpfifo = memalign(32,DEFAULT_FIFO_SIZE);
    memset(gpfifo,0,DEFAULT_FIFO_SIZE);

    // allocate 2 framebuffers for double buffering
    frameBuffer[0] = SYS_AllocateFramebuffer(rmode);
    frameBuffer[1] = SYS_AllocateFramebuffer(rmode);

    // configure video
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(frameBuffer[fb]);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

    fb ^= 1;

    // init the flipper
    GX_Init(gpfifo,DEFAULT_FIFO_SIZE);

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

    if (rmode->aa) {
        GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
    } else {
        GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    }

    GX_SetCullMode(GX_CULL_NONE);
    GX_CopyDisp(frameBuffer[fb],GX_TRUE);
    GX_SetDispCopyGamma(GX_GM_1_0);

    // setup the vertex attribute table
    // describes the data
    // args: vat location 0-7, type of data, data format, size, scale
    // so for ex. in the first call we are sending position data with
    // 3 values X,Y,Z of size F32. scale sets the number of fractional
    // bits for non float data.
    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGB8, 0);

    GX_InvVtxCache();
    GX_InvalidateTexAll();

    memset(&whiteTextureData[0], 0xFF, 32 * 32 * 4);

    GX_InitTexObj(&whiteTextureObj, &whiteTextureData[0], 32, 32, GX_TF_RGBA8, GX_REPEAT, GX_REPEAT, GX_FALSE);
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

    guMtxIdentity(model);
    guMtxRotAxisDeg(model, &cubeAxis, rquad);
    guMtxTransApply(model, model, 1.0f,0.0f,zt-7.0f);
    guMtxConcat(view,model,modelview);
    // load the modelview matrix into matrix memory
    GX_LoadPosMtxImm(modelview, GX_PNMTX3);
    GX_SetCurrentMtx(GX_PNMTX3);

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
