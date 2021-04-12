#ifndef TILEPROPS_H
#define TILEPROPS_H

typedef uint8_t TextureIndex;

struct CTile3DProperties;

enum GeometryType {
	kNoGeometry, kCube, kLeftNearWall, kRightNearWall, kFloor, kRampNorth, kRampEast, kRampSouth, kRampWest, kWallNorth, kWallWest, kWallCorner
};

struct CTile3DProperties {
	int mNeedsAlphaTest;
	int mBlockVisibility;
	int mBlockMovement;
	int mBlockEnemySight;
	int mRepeatMainTexture;
	TextureIndex mCeilingTextureIndex;
	TextureIndex mFloorTextureIndex;
	TextureIndex mMainWallTextureIndex;
    enum GeometryType mGeometryType;
	TextureIndex mCeilingRepeatedTextureIndex;
	TextureIndex mFloorRepeatedTextureIndex;
	uint8_t mCeilingRepetitions;
	uint8_t mFloorRepetitions;
	FixP_t mCeilingHeight;
	FixP_t mFloorHeight;
};

void loadPropertyList(const char * __restrict__ propertyFile, struct MapWithCharKey *map);

#endif
