#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <stdint.h>
#include <unistd.h>
#include "FixP.h"
#include "Enums.h"
#include "Common.h"
#include "LoadBitmap.h"
#include "CRenderer.h"
#include "CPackedFileReader.h"

struct Bitmap *defaultFont;
uint8_t framebuffer[320 * 200];


uint32_t palette[256];
enum ECommand mBufferedCommand = kCommandNone;


enum ECommand getInput() {
    const enum ECommand toReturn = mBufferedCommand;
    mBufferedCommand = kCommandNone;
    return toReturn;
}
