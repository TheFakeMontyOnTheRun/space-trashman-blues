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

typedef float t_mat4x4[16];

static inline void mat4x4_ortho( t_mat4x4 out, float left, float right, float bottom, float top, float znear, float zfar )
{
    #define T(a, b) (a * 4 + b)

    out[T(0,0)] = 2.0f / (right - left);
    out[T(0,1)] = 0.0f;
    out[T(0,2)] = 0.0f;
    out[T(0,3)] = 0.0f;

    out[T(1,1)] = 2.0f / (top - bottom);
    out[T(1,0)] = 0.0f;
    out[T(1,2)] = 0.0f;
    out[T(1,3)] = 0.0f;

    out[T(2,2)] = -2.0f / (zfar - znear);
    out[T(2,0)] = 0.0f;
    out[T(2,1)] = 0.0f;
    out[T(2,3)] = 0.0f;

    out[T(3,0)] = -(right + left) / (right - left);
    out[T(3,1)] = -(top + bottom) / (top - bottom);
    out[T(3,2)] = -(zfar + znear) / (zfar - znear);
    out[T(3,3)] = 1.0f;

    #undef T
}



static const char * vertex_shader =
    "#version 130\n"
    "in vec2 i_position;\n"
    "in vec4 i_color;\n"
    "out vec4 v_color;\n"
    "uniform mat4 u_projection_matrix;\n"
    "void main() {\n"
    "    v_color = i_color;\n"
    "    gl_Position = u_projection_matrix * vec4( i_position, 0.0, 1.0 );\n"
    "}\n";

static const char * fragment_shader =
    "#version 130\n"
    "in vec4 v_color;\n"
    "out vec4 o_color;\n"
    "void main() {\n"
    "    o_color = v_color;\n"
    "}\n";

typedef enum t_attrib_id
{
    attrib_position,
    attrib_color
} t_attrib_id;

int snapshotSignal = '.';
int needsToRedrawHUD = TRUE;
int enable3DRendering = TRUE;

static const int width = 320;
static const int height = 240;


SDL_Window * window;
SDL_GLContext context;
GLuint vs, fs, program;
GLuint vao, vbo;
t_mat4x4 projection_matrix;

const GLfloat g_vertex_buffer_data[] = {
/*  R, G, B, A, X, Y  */
    1, 0, 0, 1, 0, 0,
    0, 1, 0, 1, XRES_FRAMEBUFFER, 0,
    0, 0, 1, 1, XRES_FRAMEBUFFER, YRES_FRAMEBUFFER,

    1, 0, 0, 1, 0, 0,
    0, 0, 1, 1, XRES_FRAMEBUFFER, YRES_FRAMEBUFFER,
    1, 1, 1, 1, 0, YRES_FRAMEBUFFER
};

void graphicsInit() {

    enableSmoothMovement = TRUE;
    defaultFont = NULL;
    
    
    SDL_Init( SDL_INIT_VIDEO );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );


    window = SDL_CreateWindow( "", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
    context = SDL_GL_CreateContext( window );

    vs = glCreateShader( GL_VERTEX_SHADER );
    fs = glCreateShader( GL_FRAGMENT_SHADER );

    int length = strlen( vertex_shader );
    glShaderSource( vs, 1, ( const GLchar ** )&vertex_shader, &length );
    glCompileShader( vs );

    GLint status;
    glGetShaderiv( vs, GL_COMPILE_STATUS, &status );
    if( status == GL_FALSE )
    {
        fprintf( stderr, "vertex shader compilation failed\n" );
        exit(0);
    }

    length = strlen( fragment_shader );
    glShaderSource( fs, 1, ( const GLchar ** )&fragment_shader, &length );
    glCompileShader( fs );

    glGetShaderiv( fs, GL_COMPILE_STATUS, &status );
    if( status == GL_FALSE )
    {
        fprintf( stderr, "fragment shader compilation failed\n" );
        exit(0);
    }

    program = glCreateProgram();
    glAttachShader( program, vs );
    glAttachShader( program, fs );

    glBindAttribLocation( program, attrib_position, "i_position" );
    glBindAttribLocation( program, attrib_color, "i_color" );
    glLinkProgram( program );

    glUseProgram( program );

    glDisable( GL_DEPTH_TEST );
    glClearColor( 0.5, 0.0, 0.0, 0.0 );
    glViewport( 0, 0, width, height );

    glGenVertexArrays( 1, &vao );
    glGenBuffers( 1, &vbo );
    glBindVertexArray( vao );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );

    glEnableVertexAttribArray( attrib_position );
    glEnableVertexAttribArray( attrib_color );

    glVertexAttribPointer( attrib_color, 4, GL_FLOAT, GL_FALSE, sizeof( float ) * 6, 0 );
    glVertexAttribPointer( attrib_position, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 6, ( void * )(4 * sizeof(float)) );


    glBufferData( GL_ARRAY_BUFFER, sizeof( g_vertex_buffer_data ), g_vertex_buffer_data, GL_STATIC_DRAW );

    mat4x4_ortho( projection_matrix, 0.0f, (float)width, (float)height, 0.0f, 0.0f, 100.0f );
    glUniformMatrix4fv( glGetUniformLocation( program, "u_projection_matrix" ), 1, GL_FALSE, projection_matrix );
}

void handleSystemEvents() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {

        if (event.type == SDL_QUIT) {
            mBufferedCommand = kCommandQuit;
            return;
        }

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
                case SDLK_k:
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
    
    SDL_GL_DeleteContext( context );
    SDL_DestroyWindow( window );
    SDL_Quit();
}

void flipRenderer() {
    
    glBindVertexArray( vao );
    glDrawArrays( GL_TRIANGLES, 0, 6 );

    SDL_GL_SwapWindow( window );
    SDL_Delay( 1 );
}
