#ifndef RENDERER_H
#define RENDERER_H

#define NATIVE_TEXTURE_SIZE 32
#define XRES 256
#define YRES 200
#define HALF_XRES 128
#define HALF_YRES 100
#define TOTAL_TEXTURES 32
#define TRANSPARENCY_COLOR 199

struct Projection {
	struct Vec3 first;
	struct Vec2 second;
};

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
extern int linesOfSight[MAP_SIZE][MAP_SIZE];
extern int revealed[MAP_SIZE][MAP_SIZE];
extern enum EDirection cameraDirection;
extern long gameTicks;
extern int playerAmmo;
extern int playerHealth;
extern int hasSnapshot;
extern int distanceForPenumbra;
extern int distanceForDarkness;
extern int turnTarget;
extern int turnStep;


#define MASK_LEFT 1
#define MASK_FRONT 2
#define MASK_RIGHT 4
#define MASK_BEHIND 8

extern struct MapWithCharKey tileProperties;
extern struct Vec2i cameraPosition;
extern uint32_t palette[256];
extern uint8_t texturesUsed;
extern enum ECommand mBufferedCommand;
extern struct Texture *nativeTextures[TOTAL_TEXTURES];
extern uint16_t clippingY1;
extern int covered;
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




struct Mesh {
    uint16_t triangleCount;
    uint8_t *uvCoords;
    FixP_t *geometry;
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


void projectAllVertices(const uint8_t count);

uint8_t getPaletteEntry(const uint32_t origin);

void fill(
		const int16_t x, const int16_t y,
		const int16_t dx, const int16_t dy,
		const uint8_t pixel, const int stipple);


void drawMesh(const struct Mesh* mesh, const struct Vec3 at );

void drawMap(const uint8_t * __restrict__ elements,
			 const uint8_t * __restrict__ items,
			 const uint8_t * __restrict__ actors,
			 uint8_t * __restrict__ effects,
			 const struct CActor * __restrict__ current);

void drawTextAt(const uint16_t x,
				const uint16_t y,
				const char * __restrict__ text,
				const uint8_t colour);

void drawFloorAt(const struct Vec3 center,
				 const struct Texture * __restrict__ texture, uint8_t rotation);

void drawRampAt(const struct Vec3 p0, const struct Vec3 p1,
                 const struct Texture * __restrict__ texture, uint8_t rotation);

void drawCeilingAt(const struct Vec3 center,
				   const struct Texture * __restrict__ texture, uint8_t rotation);

void drawLeftNear(const struct Vec3 center,
				  const FixP_t scale,
				  const uint8_t * __restrict__ texture,
				  const uint8_t mask,
				  const int repeatedTexture);

void drawRightNear(const struct Vec3 center,
				   const FixP_t scale,
				   const uint8_t * __restrict__ texture,
				   const uint8_t mask,
				   const int repeatedTexture);

void drawColumnAt(const struct Vec3 center,
				  const FixP_t scale,
				  const struct Texture * __restrict__ texture,
				  const uint8_t mask,
				  const int enableAlpha,
				  const int repeatedTexture);

void drawBillboardAt(const struct Vec3 center,
					 const uint8_t * __restrict__ texture,
					 const FixP_t scale,
					 const int size);

void drawBitmap(const int16_t x,
				const int16_t y,
				const struct Bitmap * __restrict__ tile,
						const int transparent);

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
                      const uint8_t *__restrict__ texture);

void drawRepeatBitmap(
		const int16_t x,
		const int16_t y,
		const int16_t dx,
		const int16_t dy,
		const struct Bitmap * __restrict__ tile);

void drawRect(const int16_t x,
			const int16_t y,
			const uint16_t dx,
			const uint16_t dy,
			const uint8_t pixel);


void fillTriangle( int* coords, uint8_t colour );

void drawTexturedTriangle( int* coords, uint8_t* uvCoords, struct Texture* texture);

void drawWall(FixP_t x0,
			  FixP_t x1,
			  FixP_t x0y0,
			  FixP_t x0y1,
			  FixP_t x1y0,
			  FixP_t x1y1,
			  const uint8_t * __restrict__ texture,
			  const FixP_t textureScaleY,
			  const int z);

void drawFloor(FixP_t y0,
			   FixP_t y1,
			   FixP_t x0y0,
			   FixP_t x1y0,
			   FixP_t x0y1,
			   FixP_t x1y1,
			   int z,
			   const uint8_t * __restrict__ texture);

void drawFrontWall(FixP_t x0,
				   FixP_t y0,
				   FixP_t x1,
				   FixP_t y1,
				   const uint8_t * __restrict__ texture,
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
        uint8_t pixel
		);

#endif
