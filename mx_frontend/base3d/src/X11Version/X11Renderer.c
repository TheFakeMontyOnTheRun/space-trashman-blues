#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "Core.h"
#include "Common.h"
#include "Enums.h"
#include "FixP.h"
#include "Vec.h"
#include "Vec.h"
#include "LoadBitmap.h"
#include "CActor.h"
#include "Engine.h"
#include "Dungeon.h"
#include "MapWithCharKey.h"
#include "CTile3DProperties.h"
#include "Renderer.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

Display *display;
int screen_num;
Window root;
Visual *visual;
char *data;
XImage *img;
Window win;

void graphicsInit(void) {
	int r, g, b;

	display = XOpenDisplay(NULL);
	screen_num = DefaultScreen(display);
	root = RootWindow(display, screen_num);
	visual = DefaultVisual(display, screen_num);

	data = (char *) malloc(320 * 240 * 4);

	img = XCreateImage(display, visual, DefaultDepth(display, screen_num), ZPixmap,
					   0, data, 320, 240, 32, 0);

	/* window attributes */
	XSetWindowAttributes attr;
	unsigned long mask;

	attr.background_pixel = 0;
	attr.border_pixel = 0;
	attr.colormap = XCreateColormap(display, root, visual, AllocNone);
	attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;

	mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

	win = XCreateWindow(display, root, 50, 50, 320, 240, 0,
						DefaultDepth(display, screen_num),
						InputOutput,
						visual,
						mask,
						&attr
	);

	XSelectInput(display, win, ExposureMask | KeyPressMask);
	XMapWindow(display, win);

	for (r = 0; r < 256; r += 16) {
	  for (g = 0; g < 256; g += 8) {
            for (b = 0; b < 256; b += 8) {
	      uint32_t pixel = 0xFF000000 + (r << 16) + (g << 8) + (b);
	      uint8_t paletteEntry = getPaletteEntry(pixel);
	      palette[paletteEntry] = pixel;
            }
	  }
	}

	enableSmoothMovement = TRUE;
	defaultFont = loadBitmap("font.img");
}


uint8_t getPaletteEntry(uint32_t origin) {
    uint8_t shade;

    if (!(origin & 0xFF000000)) {
        return TRANSPARENCY_COLOR;
    }

    shade = 0;
    shade += (((((origin & 0x0000FF)) << 3) >> 8)) << 0;
    shade += (((((origin & 0x00FF00) >> 8) << 3) >> 8)) << 3;
    shade += (((((origin & 0xFF0000) >> 16) << 2) >> 8)) << 6;

    return shade;
}

void handleSystemEvents(void) {

	XEvent event;
	//while(1)
	{

	  if (XCheckWindowEvent(display, win, ExposureMask | KeyPressMask, &event)) {
	    if (event.type == Expose) {

	      int x = 0, y = 0;
	      int dstY = 0;
	      int scaller = 0;
	      int even = 0;
	      int heightY = 1;
	      
	      uint8_t *src;
	      uint32_t *dst;
	      
	      uint8_t newFrame[320 * 240];
	      
	      renderPageFlip(newFrame, framebuffer,
			     previousFrame, turnStep, turnTarget, 1);

	      for (int y = 0; y < 240; ++y) {
		for (int x = 0; x < 320; ++x) {
		  XPutPixel(img, x, y, palette[newFrame[(320 * y) + x]]);
		}
	      }
	      XPutImage(display, win, DefaultGC(display, screen_num), img, 0, 0, 0, 0, 320, 240);
	      XFlush(display);
	    }
	    
	    if (event.type == KeyPress) {
	      
	      int code = XLookupKeysym(&event.xkey, 0);
	      
	      switch (code) {
	      case XK_Return:
	      case XK_z:
		mBufferedCommand = kCommandFire1;
		visibilityCached = FALSE;
		needsToRedrawVisibleMeshes = TRUE;
		break;
		
	      case XK_Escape:
	      case XK_q:
		mBufferedCommand = kCommandBack;
		visibilityCached = FALSE;
		break;
		
	      case XK_s:
		mBufferedCommand = kCommandStrafeLeft;
		visibilityCached = FALSE;
		break;
	      case XK_d:
		mBufferedCommand = kCommandStrafeRight;
		visibilityCached = FALSE;
		break;
		
	      case XK_e:
		visibilityCached = FALSE;
		break;
	      case XK_b:
		visibilityCached = FALSE;
		break;
		
	      case XK_j:
		visibilityCached = FALSE;
		break;
	      case XK_k:
		visibilityCached = FALSE;
		break;
		
	      case XK_x:
		mBufferedCommand = kCommandFire2;
		visibilityCached = FALSE;
		needsToRedrawVisibleMeshes = TRUE;
		break;
	      case XK_c:
		mBufferedCommand = kCommandFire3;
		visibilityCached = FALSE;
		needsToRedrawVisibleMeshes = TRUE;
		break;
	      case XK_v:
		mBufferedCommand = kCommandFire4;
		visibilityCached = FALSE;
		needsToRedrawVisibleMeshes = TRUE;
		break;
		
	      case XK_Left:
		mBufferedCommand = kCommandLeft;
		visibilityCached = FALSE;
		break;
	      case XK_Right:
		mBufferedCommand = kCommandRight;
		visibilityCached = FALSE;
		break;
	      case XK_Up:
		mBufferedCommand = kCommandUp;
		visibilityCached = FALSE;
		break;
	      case XK_Down:
		mBufferedCommand = kCommandDown;
		visibilityCached = FALSE;
		break;
	      default:
		return;
	      }
	    }
	  }
	}
}

void graphicsShutdown(void) {
  XCloseDisplay(display);
  texturesUsed = 0;
}

 void flipRenderer(void) {
   int x = 0, y = 0;
   int dstY = 0;
   int scaller = 0;
   int even = 0;
   int heightY = 1;
   
   uint8_t *src;
   uint32_t *dst;
   
   uint8_t newFrame[320 * 240];
   
   renderPageFlip(newFrame, framebuffer,
		  previousFrame, turnStep, turnTarget, 1);
   
   for (int y = 0; y < 240; ++y) {
     for (int x = 0; x < 320; ++x) {
       XPutPixel(img, x, y, palette[newFrame[(320 * y) + x]]);
     }
   }
   XPutImage(display, win, DefaultGC(display, screen_num), img, 0, 0, 0, 0, 320, 240);
   XFlush(display);
 }


 void clearRenderer(void) {
 }
