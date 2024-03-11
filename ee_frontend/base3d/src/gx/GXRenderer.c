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
#define ANGLE_TURN_THRESHOLD 40
#define ANGLE_TURN_STEP 5
extern int turning;
int snapshotSignal = '.';
extern int needsToRedrawHUD;
int enable3DRendering = TRUE;

#define DEFAULT_FIFO_SIZE    (256*1024)

static void *frameBuffer[2] = {NULL, NULL};
GXRModeObj *rmode;
f32 yscale;
u32 xfbHeight;
Mtx view;
Mtx44 perspective;
Mtx model, modelview;
u32 fb = 0;
GXColor background = {0, 0, 0, 0xff};

u32 first_frame = 1;
guVector cubeAxis = {1, 1, 1};
f32 rquad = 0.0f;
f32 zt = 0;
void *gpfifo = NULL;

uint8_t whiteTextureData[32 * 32 * 4];
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

    gpfifo = memalign(32, DEFAULT_FIFO_SIZE);
    memset(gpfifo, 0, DEFAULT_FIFO_SIZE);

    frameBuffer[0] = SYS_AllocateFramebuffer(rmode);
    frameBuffer[1] = SYS_AllocateFramebuffer(rmode);

    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(frameBuffer[fb]);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();

    fb ^= 1;

    GX_Init(gpfifo, DEFAULT_FIFO_SIZE);

    GX_SetCopyClear(background, 0x00ffffff);

    GX_SetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);
    yscale = GX_GetYScaleFactor(rmode->efbHeight, rmode->xfbHeight);
    xfbHeight = GX_SetDispCopyYScale(yscale);
    GX_SetScissor(0, 0, rmode->fbWidth, rmode->efbHeight);
    GX_SetDispCopySrc(0, 0, rmode->fbWidth, rmode->efbHeight);
    GX_SetDispCopyDst(rmode->fbWidth, xfbHeight);
    GX_SetCopyFilter(rmode->aa, rmode->sample_pattern, GX_TRUE, rmode->vfilter);
    GX_SetFieldMode(rmode->field_rendering, ((rmode->viHeight == 2 * rmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));

    if (rmode->aa) {
        GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
    } else {
        GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    }

    GX_SetCullMode(GX_CULL_NONE);
    GX_CopyDisp(frameBuffer[fb], GX_TRUE);
    GX_SetDispCopyGamma(GX_GM_1_0);

    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGB8, 0);

    GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

    GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

    GX_SetNumTexGens(1);
    GX_SetNumChans(1);
    GX_InvVtxCache();
    GX_InvalidateTexAll();

    GX_SetAlphaUpdate(GX_TRUE);
    GX_SetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GX_SetZCompLoc(GX_FALSE);

    memset(&whiteTextureData[0], 0xFF, 32 * 32 * 4);

    GX_InitTexObj(&whiteTextureObj, &whiteTextureData[0], 32, 32, GX_TF_RGBA8, GX_REPEAT, GX_REPEAT, GX_FALSE);
}

void handleSystemEvents() {
#ifdef GX
    PAD_ScanPads();

    s8 tpad = PAD_StickX(0);

    // Rotate left
    if (tpad < -8) {
        turning = 1;
        leanX = -ANGLE_TURN_STEP;
    }

    // Rotate right.
    if (tpad > 8) {
        turning = 1;
        leanX = ANGLE_TURN_STEP;
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

    if (!turning) {
        leanX = PAD_SubStickX(0) / 5;
        leanY = PAD_SubStickY(0) / 5;
    }

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
                            turning = 1;
                    leanX = -ANGLE_TURN_STEP;

    }

    if (WPAD_ButtonsDown(0) & WPAD_BUTTON_RIGHT){
                    turning = 1;
                    leanX = ANGLE_TURN_STEP;

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


extern GXRModeObj *rmode;
extern Mtx model, modelview;
extern Mtx view;
extern Mtx44 perspective;
guVector Yaxis = {0, 1, 0};
guVector Xaxis = {1, 0, 0};


void drawTriangle(const struct Vec3 pos1,
                  const struct Vec2i uv1,
                  const struct Vec3 pos2,
                  const struct Vec2i uv2,
                  const struct Vec3 pos3,
                  const struct Vec2i uv3,
                  const struct Texture *texture);

uint32_t getPaletteEntry(const uint32_t origin) {
    return (0x80 << 24) + (origin & 0x00FFFFFF);
}

void enter2D(void) {
    guVector cam = {0.0F, 0.0F, 0.0F},
            up = {0.0F, 1.0F, 0.0F},
            look = {-0.0F, 0.0F, -1.0F};

    guLookAt(view, &cam, &up, &look);
    guMtxIdentity(model);
    guMtxTransApply(model, model, 0.0f, 0.0f, -0.8f);
    guMtxConcat(view, model, modelview);
    GX_LoadPosMtxImm(modelview, GX_PNMTX3);
    GX_SetCurrentMtx(GX_PNMTX3);

    f32 w = rmode->viWidth;
    f32 h = rmode->viHeight;
    guPerspective(perspective, 45, (f32) w / h, 0.1F, 1024.0F);
    GX_LoadProjectionMtx(perspective, GX_PERSPECTIVE);
}

void initGL() {
    /* tmp */
    memFill(&nativeTextures[0], 0, sizeof(struct Texture) * TOTAL_TEXTURES);
}

void clearRenderer() {
}

void startFrame(int x, int y, int width, int height) {
    visibilityCached = FALSE;
    needsToRedrawVisibleMeshes = FALSE;
    enter2D();
}

void endFrame() {
}

void enter3D(void) {
    float _leanX = 0.0f;
    float _leanY = 0.0f;
    _leanX = (leanX * 3.14159f * 0.25f) / ((float)ANGLE_TURN_THRESHOLD);
    _leanY = (leanY * 3.14159f * 0.25f) / ((float)ANGLE_TURN_THRESHOLD);
    f32 w = rmode->viWidth;
    f32 h = rmode->viHeight;
    guPerspective(perspective, 90, (f32) w / h, 0.1F, 256.0F);
    GX_LoadProjectionMtx(perspective, GX_PERSPECTIVE);

    guVector cam = {0.0F, 0.0F, 0.0F},
            up = {0.0F, 1.0F, 0.0F},
            look = {_leanX, _leanY, -1.5F};

    guLookAt(view, &cam, &up, &look);
    guMtxIdentity(model);
    guMtxTransApply(model, model, 0.0f, 0.0f, -0.8f);
    guMtxConcat(view, model, modelview);
    GX_LoadPosMtxImm(modelview, GX_PNMTX3);
    GX_SetCurrentMtx(GX_PNMTX3);

}

void flipRenderer() {
    GX_SetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0);

    GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GX_SetColorUpdate(GX_TRUE);
    GX_CopyDisp(frameBuffer[fb], GX_TRUE);

    GX_DrawDone();

    VIDEO_SetNextFramebuffer(frameBuffer[fb]);
    if (first_frame) {
        first_frame = 0;
        VIDEO_SetBlack(FALSE);
    }
    VIDEO_Flush();
    VIDEO_WaitVSync();
    fb ^= 1;
}
