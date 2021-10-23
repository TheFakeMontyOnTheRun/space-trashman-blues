//
// Created by Daniel Monteiro on 2021-10-22.
//

#ifndef DERELICT8_ENGINE3D_H
#define DERELICT8_ENGINE3D_H

#ifdef RES96x64
    #define XRES 96
    #define YRES 64
#else
#ifdef RES64X128
    #define XRES 64
    #define YRES 128
#else
#ifdef RES128X128
#define XRES 128
#define YRES 128
#else
#define XRES 64
        #define YRES 64
#endif
#endif
#endif

#define XRESMINUSONE XRES - 1
#define YRESMINUSONE YRES - 1


#define WALKSTEP 1
#define CAMERA_HEIGHT 2
#define VISIBILITY_LIMIT 32


void HUD_initialPaint();
void HUD_refresh();

#endif //DERELICT8_ENGINE3D_H
