#ifndef RENDERER_H
#define RENDERER_H

#define NATIVE_TEXTURE_SIZE 32

#ifdef AMIGA
#define XRES 200
#define YRES 128
#define HALF_XRES 100
#define HALF_YRES 64
#else
#define XRES 216
#define YRES 200
#define HALF_XRES 100
#define HALF_YRES 100
#endif

#define XRES_FRAMEBUFFER 320
#define YRES_FRAMEBUFFER 200

#define TOTAL_TEXTURES 64
#define TRANSPARENCY_COLOR 199

struct Projection {
	struct Vec3 first;
	struct Vec2 second;
};

typedef uint32_t OutputPixelFormat;
typedef uint32_t FramebufferPixelFormat;
typedef uint8_t UVCoord;

extern struct MapWithCharKey occluders;
extern struct MapWithCharKey enemySightBlockers;
extern struct MapWithCharKey colliders;
extern int useDither;
extern int visibilityCached;
extern int needsToRedrawVisibleMeshes;
extern uint8_t *visibleElementsMap;
extern struct Bitmap *defaultFont;
extern uint8_t framebuffer[320 * 200];
extern uint8_t previousFrame[320 * 200];
extern enum EDirection cameraDirection;
extern long gameTicks;
extern int playerHealth;
extern int hasSnapshot;
extern int distanceForPenumbra;
extern int distanceForDarkness;
extern int turnTarget;
extern int turnStep;
extern int needToRedrawHUD;

#define MASK_LEFT 1
#define MASK_FRONT 2
#define MASK_RIGHT 4
#define MASK_BEHIND 8
#define MASK_FORCE_LEFT 16
#define MASK_FORCE_RIGHT 32

#define WALKING_BIAS 4096

extern struct MapWithCharKey tileProperties;
extern struct Vec2i cameraPosition;
extern uint32_t palette[256];
extern uint8_t texturesUsed;
extern enum ECommand mBufferedCommand;
extern struct Texture *nativeTextures[TOTAL_TEXTURES];
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
extern struct Bitmap *mapTopLevel;
extern char mTurnBuffer;
extern struct Vec3 cameraOffset;

struct Mesh {
    uint16_t triangleCount;
    uint8_t *uvCoords;
    FixP_t *geometry;
    struct Texture* texture;
    uint8_t colour;
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

void initHW(void);

void shutdownHW(void);

void loadMesh(struct Mesh* mesh, char* filename );

void projectAllVertices(const int count);

uint32_t getPaletteEntry(const uint32_t origin);

void renderPageFlip(OutputPixelFormat *stretchedBuffer, FramebufferPixelFormat *currentFrame,
					FramebufferPixelFormat *prevFrame, int turnState, int turnTarget, uint8_t scale200To240);

void fill(
		const int x, const int y,
		const size_t dx, const size_t dy,
		const FramebufferPixelFormat pixel, const uint8_t stipple);


void drawMesh(const struct Mesh* mesh, const struct Vec3 at );

void drawMap(const uint8_t *  elements,
			 const uint8_t *  items,
			 const uint8_t *  actors,
			 uint8_t *  effects,
			 const struct CActor *  current);

void drawTextAtWithMargin(const int x, const int y, int margin, const char * text, const FramebufferPixelFormat colour);

void drawTextAtWithMarginWithFiltering(const int x, const int y, int margin, const char * text, const uint8_t colour, char charToReplaceHifenWith);

void drawTextAt(const int x,
				const int y,
				const char *  text,
				const FramebufferPixelFormat colour);

void drawFloorAt(const struct Vec3 center,
				 const struct Texture *texture, enum EDirection rotation);

void drawRampAt(const struct Vec3 p0, const struct Vec3 p1,
                 const struct Texture *  texture, uint8_t rotation, uint8_t flipTexture);

void drawCeilingAt(const struct Vec3 center,
				   const struct Texture *  texture, enum EDirection rotation);

void drawLeftNear(const struct Vec3 center,
				  const FixP_t scale,
				  const struct Texture *texture,
				  const uint8_t mask,
				  const uint8_t repeatedTexture);

void drawRightNear(const struct Vec3 center,
				   const FixP_t scale,
				   const struct Texture *texture,
				   const uint8_t mask,
				   const uint8_t repeatedTexture);

void drawColumnAt(const struct Vec3 center,
				  const FixP_t scale,
				  const struct Texture *texture,
				  const uint8_t mask,
				  const uint8_t enableAlpha,
				  const uint8_t repeatedTexture);

void drawBillboardAt(const struct Vec3 center,
					 struct Texture *bitmap,
					 const FixP_t scale,
					 const int size);

void drawBitmapRaw(const int dx,
                   const int dy,
                   int width,
                   int height,
                   BitmapPixelFormat *bitmapData,
                   const int transparent);


void drawBitmap(const int x,
				const int y,
				struct Bitmap *  tile,
						const uint8_t transparent);

void drawSlantedFloor(
                      FixP_t p0x,
                      FixP_t p0y,
                      FixP_t p1x,
                      FixP_t p1y,
                      FixP_t p2x,
                      FixP_t p2y,
                      FixP_t p3x,
                      FixP_t p3y,
                      int z,
                      const TexturePixelFormat *texture);

void drawRepeatBitmap(
		const int x,
		const int y,
		const size_t dx,
		const size_t dy,
		const struct Bitmap *  tile);

void drawRect(const int x,
			const int y,
			const size_t dx,
			const size_t dy,
			const FramebufferPixelFormat pixel);


void fillTriangle( int* coords, FramebufferPixelFormat colour );

void drawTexturedTriangle( int* coords, UVCoord* uvCoords, struct Texture* texture);

void drawWall(FixP_t x0,
			  FixP_t x1,
			  FixP_t x0y0,
			  FixP_t x0y1,
			  FixP_t x1y0,
			  FixP_t x1y1,
			  const TexturePixelFormat *texture,
			  const FixP_t textureScaleY,
			  const int z);

void drawFloor(FixP_t y0,
			   FixP_t y1,
			   FixP_t x0y0,
			   FixP_t x1y0,
			   FixP_t x0y1,
			   FixP_t x1y1,
			   int z,
			   const TexturePixelFormat *  texture);

void drawFrontWall(FixP_t x0,
				   FixP_t y0,
				   FixP_t x1,
				   FixP_t y1,
				   const TexturePixelFormat *  texture,
				   const FixP_t textureScaleY,
				   const int z,
				   const int enableAlpha,
				   const int size);

void drawMask(const FixP_t x0,
			const FixP_t y0,
			const FixP_t x1,
			const FixP_t y1);

void maskWall(
		FixP_t x0,
		FixP_t x1,
		FixP_t x0y0,
		FixP_t x0y1,
		FixP_t x1y0,
		FixP_t x1y1);

void maskFloor(
		FixP_t y0,
		FixP_t y1,
		FixP_t x0y0,
		FixP_t x1y0,
		FixP_t x0y1,
		FixP_t x1y1,
        FramebufferPixelFormat pixel
		);

int submitBitmapToGPU(struct Bitmap* bitmap);

#endif
