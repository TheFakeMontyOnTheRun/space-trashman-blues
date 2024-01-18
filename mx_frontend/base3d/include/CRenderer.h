#ifndef RENDERER_H
#define RENDERER_H

#include "LoadBitmap.h"

#ifdef AGS
#define PAGE_FLIP_TARGET 128
#define XRES 130
#define YRES 160
#define HALF_XRES 65
#define HALF_YRES 80
#else
#define PAGE_FLIP_TARGET 200
#define XRES 216
#define YRES 200
#define HALF_XRES 100
#define HALF_YRES 100
#endif

#ifndef AGS
#define XRES_FRAMEBUFFER 320
#define YRES_FRAMEBUFFER 200
#else
#define XRES_FRAMEBUFFER 240
#define YRES_FRAMEBUFFER 160
#endif

#define TOTAL_TEXTURES 16
#define TRANSPARENCY_COLOR 199
#define VISIBILITY_CONE_NARROWING 3
#define MASK_LEFT 1
#define MASK_FRONT 2
#define MASK_RIGHT 4
#define MASK_BEHIND 8
#define MASK_FORCE_LEFT 16
#define MASK_FORCE_RIGHT 32
#define WALKING_BIAS 4096
#define LEVEL_MAP(x, y) (map[ ( (MAP_SIZE) * (y) ) + (x) ])
#define ITEMS_IN_MAP(x, y) (itemsInMap[ ( (MAP_SIZE) * (y) ) + (x) ])

typedef uint32_t OutputPixelFormat;
typedef uint8_t FramebufferPixelFormat;
typedef uint8_t UVCoord;

struct Projection {
    struct Vec3 first;
    struct Vec2 second;
};

void graphicsInit(void);

void graphicsShutdown(void);

void clearRenderer(void);

void flipRenderer(void);

void render(long ms);

void loadTexturesForLevel(const uint8_t levelNumber);

void loadTileProperties(const uint8_t levelNumber);

void updateCursorForRenderer(const int x, const int y);

enum ECommand getInput(void);

void handleSystemEvents(void);

void initHW(int argc, char **argv);

void clear(void);

void clearTileProperties(void);

void shutdownHW(void);

void initZMap(void);

void projectAllVertices(const uint8_t count);

FramebufferPixelFormat getPaletteEntry(const uint32_t origin);

void enter2D(void);

void enter3D(void);

void computeLightning(void);

void renderPageFlip(uint8_t *stretchedBuffer, uint8_t *currentFrame, uint8_t *prevFrame, int turnState, int turnTarget,
                    int scale200To240);

void fillRect(
        const int x, const int y,
        const size_t dx, const size_t dy,
        const FramebufferPixelFormat pixel, const uint8_t stipple);


void drawMesh(const struct Mesh *mesh, const struct Vec3 at, FramebufferPixelFormat tint);

void renderRoomTransition(void);

void drawMap(const struct CActor *current);

void drawTextAtWithMargin(const int x, const int y, int margin, const char *text, const FramebufferPixelFormat colour);

void drawTextAtWithMarginWithFiltering(const int x, const int y, int margin, const char *text, const uint8_t colour,
                                       char charToReplaceHifenWith);

void drawTextAt(const int x,
                const int y,
                const char *text,
                const FramebufferPixelFormat colour);

void drawFloorAt(const struct Vec3 center,
                 const struct Texture *texture, enum EDirection rotation, FramebufferPixelFormat tint);

void drawRampAt(const struct Vec3 p0, const struct Vec3 p1,
                const struct Texture *texture, uint8_t rotation, uint8_t flipTexture, FramebufferPixelFormat tint);

void drawCeilingAt(const struct Vec3 center,
                   const struct Texture *texture, enum EDirection rotation, FramebufferPixelFormat tint);

void drawLeftNear(const struct Vec3 center,
                  const FixP_t scale,
                  const struct Texture *texture,
                  const uint8_t mask,
                  const uint8_t repeatedTexture, FramebufferPixelFormat tint);

void drawRightNear(const struct Vec3 center,
                   const FixP_t scale,
                   const struct Texture *texture,
                   const uint8_t mask,
                   const uint8_t repeatedTexture,
                   FramebufferPixelFormat tint);

void drawColumnAt(const struct Vec3 center,
                  const FixP_t scale,
                  const struct Texture *texture,
                  const uint8_t mask,
                  const uint8_t enableAlpha,
                  const uint8_t repeatedTexture,
                  FramebufferPixelFormat tint);

void drawBillboardAt(const struct Vec3 center,
                     struct Bitmap *bitmap,
                     const FixP_t scale,
                     const int size,
                     FramebufferPixelFormat tint);

void drawBitmapRaw(const int dx,
                   const int dy,
                   int width,
                   int height,
                   BitmapPixelFormat *bitmapData,
                   const int transparent,
                   FramebufferPixelFormat tint);


void drawBitmap(const int x,
                const int y,
                struct Bitmap *tile,
                const uint8_t transparent,
                FramebufferPixelFormat tint);

void drawBitmapRegion(const int _x,
                      const int _y,
                      const int _dx,
                      const int _dy,
                      BitmapPixelFormat tint,
                      struct Bitmap *bitmap,
                      const uint8_t transparent,
                      float u0, float u1, float v0, float v1);

void drawRect(const int x,
              const int y,
              const size_t dx,
              const size_t dy,
              const FramebufferPixelFormat pixel);


void fillTriangle(int *coords, FramebufferPixelFormat colour, FramebufferPixelFormat tint);

void drawTexturedTriangle(int *coords, uint8_t *uvCoords, struct Texture *texture, int z, FramebufferPixelFormat tint);

void drawWall(FixP_t x0,
              FixP_t x1,
              FixP_t x0y0,
              FixP_t x0y1,
              FixP_t x1y0,
              FixP_t x1y1,
              const TexturePixelFormat *texture,
              const FixP_t textureScaleY,
              const int z,
              FramebufferPixelFormat tint);

void drawFloor(FixP_t y0,
               FixP_t y1,
               FixP_t x0y0,
               FixP_t x1y0,
               FixP_t x0y1,
               FixP_t x1y1,
               int z,
               const TexturePixelFormat *texture,
               FramebufferPixelFormat tint);

void drawFrontWall(FixP_t x0,
                   FixP_t y0,
                   FixP_t x1,
                   FixP_t y1,
                   const TexturePixelFormat *texture,
                   const FixP_t textureScaleY,
                   const int z,
                   const int enableAlpha,
                   const int size,
                   FramebufferPixelFormat tint);

void drawMask(const FixP_t x0,
              const FixP_t y0,
              const FixP_t x1,
              const FixP_t y1,
              FramebufferPixelFormat tint);

void maskWall(
        FixP_t x0,
        FixP_t x1,
        FixP_t x0y0,
        FixP_t x0y1,
        FixP_t x1y0,
        FixP_t x1y1,
        FramebufferPixelFormat tint);

void maskFloor(
        FixP_t y0,
        FixP_t y1,
        FixP_t x0y0,
        FixP_t x1y0,
        FixP_t x0y1,
        FixP_t x1y1,
        FramebufferPixelFormat pixel,
        FramebufferPixelFormat tint
);

int submitBitmapToGPU(struct Bitmap *bitmap);

void initRenderer(void);

void startFrame(int x, int y, int width, int height);

void endFrame(void);

extern struct MapWithCharKey occluders;
extern struct MapWithCharKey enemySightBlockers;
extern struct MapWithCharKey colliders;
extern int visibilityCached;
extern int needsToRedrawVisibleMeshes;
extern struct Bitmap *defaultFont;
#ifndef AGS
extern uint8_t framebuffer[XRES_FRAMEBUFFER * YRES_FRAMEBUFFER];
extern uint8_t previousFrame[XRES_FRAMEBUFFER * YRES_FRAMEBUFFER];
extern uint32_t palette[256];
#else
extern uint8_t *framebuffer;
#endif
extern enum EDirection cameraDirection;
extern long gameTicks;
extern int hasSnapshot;
extern int turnTarget;
extern int turnStep;
extern int needsToRedrawHUD;
extern struct MapWithCharKey tileProperties;
extern struct Vec2i cameraPosition;
extern uint8_t texturesUsed;
extern enum ECommand mBufferedCommand;
extern struct Texture *nativeTextures[TOTAL_TEXTURES];
extern uint16_t clippingY1;
extern struct Projection projectionVertices[8];
extern FixP_t playerHeight;
extern FixP_t walkingBias;
extern FixP_t playerHeightChangeRate;
extern FixP_t playerHeightTarget;
extern FixP_t xCameraOffset;
extern FixP_t yCameraOffset;
extern FixP_t zCameraOffset;
extern int enable3DRendering;
extern uint8_t enableSmoothMovement;
#ifdef TILED_BITMAPS
extern struct Bitmap *mapTopLevel[8];
#else
extern struct Bitmap *mapTopLevel;
#endif
extern int dirtyLineY0;
extern int dirtyLineY1;
extern char mTurnBuffer;
extern uint8_t *map;
extern uint8_t *itemsInMap;
extern FixP_t divLut[320];
#endif
