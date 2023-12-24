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

#define GL_GLEXT_PROTOTYPES

#include <SDL.h>
#include <SDL_opengl.h>

#include <stdio.h>

#define ANGLE_TURN_THRESHOLD 40
#define ANGLE_TURN_STEP 5

extern int turning;
extern int leanX;
extern int leanY;

unsigned int aPositionAttributeLocation;
unsigned int aTexCoordAttributeLocation;
unsigned int uProjectionViewUniformLocation;
unsigned int sTextureUniformLocation;
unsigned int uModUniformLocation;
unsigned int uFadeUniformLocation;
unsigned int uModelPositionUniformLocation;
unsigned int uScaleUniformLocation;

static const char *vertex_shader =
        "#ifdef GL_ES\n"
        "precision mediump float;\n"
        "#endif\n"
        "#if __VERSION__ >= 140\n"
        "in vec4 aPosition;\n"
        "in vec2 aTexCoord;\n"
        "out vec2 vTextureCoords;\n"
        "#else\n"
        "attribute vec4 aPosition;\n"
        "attribute vec2 aTexCoord;\n"
        "varying vec2 vTextureCoords;\n"
        "#endif\n"
        "uniform vec4 uModelPosition;\n"
        "uniform mat4 uProjectionView;\n"
        "uniform vec2 uScale;\n"
        "void main() {\n"
        "gl_Position =  uProjectionView * ( vec4(aPosition.x * uScale.x, aPosition.y * uScale.y, aPosition.z, aPosition.w) + uModelPosition);\n"
        "vTextureCoords = aTexCoord  * uScale;\n"
        "}\n";

static const char *fragment_shader =
        "#ifdef GL_ES\n"
        "        precision mediump float;\n"
        "#endif\n"
        "#if __VERSION__ >= 140\n"
        "        in vec2 vTextureCoords;\n"
        "out vec4 fragColor;\n"
        "#else\n"
        "        varying vec2 vTextureCoords;\n"
        "#endif\n"
        "uniform sampler2D sTexture;\n"
        "uniform vec4 uMod;\n"
        "uniform vec4 uFade;\n"
        "void main() {\n"
        "#if __VERSION__ >= 140\n"
        "    fragColor = texture2D( sTexture, vTextureCoords );\n"
        "   if ( fragColor.a < 0.5 ) {\n"
        "        discard;\n"
        "    }\n"
        "    fragColor = fragColor * uFade.a * uMod;\n"
        "#else\n"
        "    gl_FragColor = texture2D( sTexture, vTextureCoords );\n"
        "    if ( gl_FragColor.a < 0.5 ) {\n"
        "        discard;\n"
        "    }\n"
        "    gl_FragColor = gl_FragColor * uFade.a * uMod;\n"
        "#endif\n"
        "}\n";

int snapshotSignal = '.';
int needsToRedrawHUD = TRUE;
int enable3DRendering = TRUE;

static const int width = 320;
static const int height = 240;

SDL_Window *window;
SDL_GLContext context;

unsigned int vs, fs, program;


void graphicsInit() {

    enableSmoothMovement = TRUE;
    defaultFont = NULL;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    window = SDL_CreateWindow(
            "", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    if (window == NULL) {
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
    }

    context = SDL_GL_CreateContext(window);

    if (context == NULL) {
        printf(
                "OpenGL context could not be created! SDL Error: %s\n",
                SDL_GetError());
    }

    if (SDL_GL_SetSwapInterval(1) < 0) {
        printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
    }

    glViewport(0, 0, width, height);

    /* creating shader */
    vs = glCreateShader(GL_VERTEX_SHADER);
    fs = glCreateShader(GL_FRAGMENT_SHADER);

    int length = strlen(vertex_shader);
    glShaderSource(vs, 1, (const GLchar **) &vertex_shader, &length);
    glCompileShader(vs);

    GLint status;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
        fprintf(stderr, "vertex shader compilation failed\n");
        exit(0);
    }

    length = strlen(fragment_shader);
    glShaderSource(fs, 1, (const GLchar **) &fragment_shader, &length);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
        fprintf(stderr, "fragment shader compilation failed\n");
        exit(0);
    }

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glUseProgram(program);

    /* attaching data to shaders */

    aPositionAttributeLocation = glGetAttribLocation(program, "aPosition");
    aTexCoordAttributeLocation = glGetAttribLocation(program, "aTexCoord");
    uProjectionViewUniformLocation = glGetUniformLocation(program, "uProjectionView");
    sTextureUniformLocation = glGetUniformLocation(program, "sTexture");
    uModUniformLocation = glGetUniformLocation(program, "uMod");
    uFadeUniformLocation = glGetUniformLocation(program, "uFade");
    uModelPositionUniformLocation = glGetUniformLocation(program, "uModelPosition");
    uScaleUniformLocation = glGetUniformLocation(program, "uScale");

    initGL();
}

void handleSystemEvents() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {

#ifdef EMIT_QUIT_OPTION
        if (event.type == SDL_QUIT) {
            mBufferedCommand = kCommandQuit;
            return;
        }
#endif

        if (event.type == SDL_KEYUP) {
            visibilityCached = FALSE;
            needsToRedrawVisibleMeshes = TRUE;
        }

        if (event.type == SDL_KEYDOWN) {
            leanY = leanX = 0;
            switch (event.key.keysym.sym) {
                case SDLK_RETURN:
                case SDLK_z:
                    mBufferedCommand = kCommandFire1;
                    visibilityCached = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
                    break;
                case SDLK_x:
                    mBufferedCommand = kCommandFire2;
                    visibilityCached = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
                    break;
                case SDLK_c:
                    mBufferedCommand = kCommandFire3;
                    visibilityCached = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
                    break;
                case SDLK_v:
                    mBufferedCommand = kCommandFire4;
                    visibilityCached = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
                    break;
                case SDLK_k:
                    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
                    break;
                case SDLK_w:
                    SDL_SetWindowFullscreen(window, 0);
                    break;

                case SDLK_t:
                    leanY = -1;
                    visibilityCached = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
                    break;

                case SDLK_g:
                    leanY = 1;
                    visibilityCached = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
                    break;

                case SDLK_f:
                    leanX = -1;
                    visibilityCached = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
                    break;

                case SDLK_h:
                    leanX = 1;
                    visibilityCached = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
                    break;

                case SDLK_ESCAPE:
                case SDLK_q:
                    mBufferedCommand = kCommandBack;
                    visibilityCached = FALSE;
                    break;

                case SDLK_SPACE:

                case SDLK_s:
                    mBufferedCommand = kCommandStrafeLeft;
                    visibilityCached = FALSE;
                    break;
                case SDLK_d:
                    mBufferedCommand = kCommandStrafeRight;
                    visibilityCached = FALSE;
                    break;

                case SDLK_i:
                    visibilityCached = FALSE;
                    break;
                case SDLK_o:
                    visibilityCached = FALSE;
                    break;

                case SDLK_j:
                    visibilityCached = FALSE;
                    break;

                case SDLK_LEFT:
                    turning = 1;
                    leanX = -ANGLE_TURN_STEP;
                    break;
                case SDLK_RIGHT:
                    leanX = ANGLE_TURN_STEP;
                    turning = 1;
                    break;
                case SDLK_UP:
                    mBufferedCommand = kCommandUp;
                    visibilityCached = FALSE;
                    break;
                case SDLK_1:
                    enableSmoothMovement = TRUE;
                    break;

                case SDLK_2:
                    enableSmoothMovement = FALSE;
                    break;

                case SDLK_3:
                    leanX = -ANGLE_TURN_STEP;
                    break;

                case SDLK_4:
                    leanX = ANGLE_TURN_STEP;

                    break;

                case SDLK_DOWN:
                    mBufferedCommand = kCommandDown;
                    visibilityCached = FALSE;
                    break;

                case SDLK_n:
                    needsToRedrawVisibleMeshes = TRUE;
                    visibilityCached = FALSE;
                    break;
                case SDLK_m:
                    needsToRedrawVisibleMeshes = TRUE;
                    visibilityCached = FALSE;
                    break;
                case SDLK_l:
                    graphicsShutdown();
                    exit(0);
                    return;

                default:
                    return;
            }
        }
    }
}

void graphicsShutdown() {
    texturesUsed = 0;

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void flipRenderer() {
    SDL_GL_SwapWindow(window);
    SDL_Delay(1);
}
