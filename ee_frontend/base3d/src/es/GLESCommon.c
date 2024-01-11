#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#include "Core.h"
#include "FixP.h"
#include "Enums.h"
#include "Common.h"
#include "Vec.h"
#include "CActor.h"
#include "Dungeon.h"
#include "MapWithCharKey.h"
#include "Globals.h"
#include "LoadBitmap.h"
#include "Mesh.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"
#include "VisibilityStrategy.h"
#include "PackedFileReader.h"
#include "UI.h"
#include "Engine.h"

#ifdef SDLGL
#define GL_GLEXT_PROTOTYPES
#include <SDL.h>
#include <SDL_opengl.h>
#else
#ifndef ANDROID
#if __APPLE__
#include <TargetConditionals.h>
#define GL_SILENCE_DEPRECATION
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
// iOS, tvOS, or watchOS Simulator
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#elif TARGET_OS_MACCATALYST
// Mac's Catalyst (ports iOS API into Mac, like UIKit).
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#elif TARGET_OS_IPHONE
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
// iOS, tvOS, or watchOS device
#elif TARGET_OS_MAC
// Other kinds of Apple platforms
#include <OpenGL/gl3.h>
#else
#   error "Unknown Apple platform"
#endif
#endif
#else

#include <GLES2/gl2.h>

#endif
#endif

extern const int width, height;

t_mat4x4 projection_matrix;
t_mat4x4 viewMatrix;

t_mat4x4 transformMatrix;
t_mat4x4 rotateXMatrix;
t_mat4x4 rotateYMatrix;
t_mat4x4 rotateZMatrix;

unsigned int vs, fs, program;

extern struct Texture *itemSprites[TOTAL_ITEMS];

#ifdef TILED_BITMAPS
extern struct Bitmap *mapTopLevel[8];
#else
extern struct Bitmap *mapTopLevel;
#endif

const float planeXYVertices[] = {
        -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f
};

const float planeYZVertices[] = {
        0.0f, 1.0f, -1.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, -1.0f, 1.0f,
        0.0f, -1.0f, -1.0f
};


const float cornerLeftFarVertices[] = {
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f
};

const float cornerLeftNearVertices[] = {
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f
};


const float floorVertices[] = {
        -1.0f, 0.0f, -1.0f,
        1.0f, 0.0f, -1.0f,
        1.0f, 0.0f, 1.0f,
        -1.0f, 0.0f, 1.0f
};

const float rampVertices[] = {
        -1.0f, -0.5f, -1.0f,
        1.0f, -0.5f, -1.0f,
        1.0f, 0.5f, 1.0f,
        -1.0f, 0.5f, 1.0f
};


const float planeXYUVs[] = {
        0.0f, .0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
};

const float planeYZUVs[] = {
        0.0f, .0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
};


const float cornerLeftFarUVs[] = {
        0.0f, .0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
};

const float cornerLeftNearUVs[] = {
        0.0f, .0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
};


const float floorUVs[] = {
        0.0f, .0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
};

const float rampUVs[] = {
        0.0f, .0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
};

const unsigned short planeXYIndices[] = {
        0, 1, 2,
        0, 2, 3
};

const unsigned short planeYZIndices[] = {
        0, 1, 2,
        0, 2, 3
};

const unsigned short cornerLeftFarIndices[] = {
        0, 1, 2,
        0, 2, 3
};

const unsigned short cornerLeftNearIndices[] = {
        0, 1, 2,
        0, 2, 3
};

const unsigned short floorIndices[] = {
        0, 1, 2,
        0, 2, 3
};

const unsigned short rampIndices[] = {
        0, 1, 2,
        0, 2, 3
};

unsigned int aPositionAttributeLocation;
unsigned int aTexCoordAttributeLocation;
unsigned int uProjectionMatrixUniformLocation;
unsigned int uViewMatrixUniformLocation;
unsigned int uTransformMatrixUniformLocation;
unsigned int uViewMatrixUniformLocation;
unsigned int uRotateXMatrixUniformLocation;
unsigned int uRotateYMatrixUniformLocation;
unsigned int uRotateZMatrixUniformLocation;
unsigned int sTextureUniformLocation;
unsigned int uModUniformLocation;
unsigned int uFadeUniformLocation;
unsigned int uScaleUniformLocation;


struct Bitmap whiteTexture;
BitmapPixelFormat whiteRaw[4];

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
        "uniform mat4 uProjectionMatrix;\n"
        "uniform mat4 uViewMatrix;\n"
        "uniform mat4 uTransformMatrix;\n"
        "uniform mat4 uRotateXMatrix;\n"
        "uniform mat4 uRotateYMatrix;\n"
        "uniform mat4 uRotateZMatrix;\n"
        "uniform vec2 uScale;\n"
        "void main() {\n"
        "gl_Position =  uProjectionMatrix * uViewMatrix * uTransformMatrix * uRotateXMatrix * uRotateYMatrix * uRotateZMatrix * aPosition;\n"
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
        "    fragColor = texture( sTexture, vTextureCoords );\n"
        "   if ( fragColor.a < 0.1 ) {\n"
        "        discard;\n"
        "    }\n"
        "    fragColor = fragColor * uFade.a * uMod;\n"
        "#else\n"
        "    gl_FragColor = texture2D( sTexture, vTextureCoords );\n"
        "    if ( gl_FragColor.a < 0.1 ) {\n"
        "        discard;\n"
        "    }\n"
        "    gl_FragColor = gl_FragColor * uFade.a * uMod;\n"
        "#endif\n"
        "}\n";


struct VBORegister planeXYVBO, leftFarVBO, leftNearVBO, floorVBO, rampVBO, planeYZVBO;

void mat4x4_ortho(t_mat4x4 out, float left, float right, float bottom, float top, float znear,
                  float zfar) {
#define T(a, b) (a * 4 + b)

    out[T(0, 0)] = 2.0f / (right - left);
    out[T(0, 1)] = 0.0f;
    out[T(0, 2)] = 0.0f;
    out[T(0, 3)] = 0.0f;

    out[T(1, 1)] = 2.0f / (top - bottom);
    out[T(1, 0)] = 0.0f;
    out[T(1, 2)] = 0.0f;
    out[T(1, 3)] = 0.0f;

    out[T(2, 2)] = -2.0f / (zfar - znear);
    out[T(2, 0)] = 0.0f;
    out[T(2, 1)] = 0.0f;
    out[T(2, 3)] = 0.0f;

    out[T(3, 0)] = -(right + left) / (right - left);
    out[T(3, 1)] = -(top + bottom) / (top - bottom);
    out[T(3, 2)] = -(zfar + znear) / (zfar - znear);
    out[T(3, 3)] = 1.0f;

#undef T
}


void mat4x4_perspective(t_mat4x4 out, float fov, float ratio, float znear, float zfar) {
    float rad = M_PI / 180.0f;
    float oneOverTanFovDiv2 = 1.0f / tan(fov * rad / 2.0f);
    out[0] = oneOverTanFovDiv2 / (ratio);
    out[1] = 0.0f;
    out[2] = 0.0f;
    out[3] = 0.0f;

    out[4] = 0.0f;
    out[5] = oneOverTanFovDiv2;
    out[6] = 0.0f;
    out[7] = 0.0f;

    out[8] = 0.0f;
    out[9] = 0.0f;
    out[10] = -((zfar + znear) / (zfar - znear));
    out[11] = -1.0f;

    out[12] = 0.0f;
    out[13] = 0.0f;
    out[14] = -((2.0f * zfar * znear) / (zfar - znear));
    out[15] = 0.0f;
}

void mat4x4_view(t_mat4x4 out, float cx, float cy, float cz, float tx, float ty, float tz, float ux,
                 float uy, float uz) {
    out[0] = 1;
    out[1] = 0.0f;
    out[2] = 0.0f;
    out[3] = 0.0f;
    out[4] = 0.0f;
    out[5] = 1;
    out[6] = 0.0f;
    out[7] = 0.0f;
    out[8] = 0.0f;
    out[9] = 0.0f;
    out[10] = 1;
    out[11] = 0.0f;
    out[12] = 0.0f;
    out[13] = 0.0f;
    out[14] = 0;
    out[15] = 1;
}

void mat4x4_transform(t_mat4x4 out, float ox, float oy, float oz, float sx, float sy, float sz) {
    out[0] = sx;
    out[1] = 0.0f;
    out[2] = 0.0f;
    out[3] = 0;
    out[4] = 0.0f;
    out[5] = sy;
    out[6] = 0.0f;
    out[7] = 0;
    out[8] = 0.0f;
    out[9] = 0.0f;
    out[10] = sz;
    out[11] = 0;
    out[12] = ox;
    out[13] = oy;
    out[14] = oz;
    out[15] = 1;
}

void mat4x4_rotateX(t_mat4x4 out, float deg) {

    float ca = cosf(deg * M_PI / 180.0f);
    float sa = sinf(deg * M_PI / 180.0f);

    out[0] = 1;
    out[1] = 0.0f;
    out[2] = 0.0f;
    out[3] = 0.0f;
    out[4] = 0.0f;
    out[5] = ca;
    out[6] = -sa;
    out[7] = 0.0f;
    out[8] = 0;
    out[9] = sa;
    out[10] = ca;
    out[11] = 0.0f;
    out[12] = 0.0f;
    out[13] = 0.0f;
    out[14] = 0;
    out[15] = 1;
}

void mat4x4_rotateY(t_mat4x4 out, float deg) {

    float ca = cosf(deg * M_PI / 180.0f);
    float sa = sinf(deg * M_PI / 180.0f);

    out[0] = ca;
    out[1] = 0.0f;
    out[2] = sa;
    out[3] = 0.0f;
    out[4] = 0.0f;
    out[5] = 1;
    out[6] = 0.0f;
    out[7] = 0.0f;
    out[8] = -sa;
    out[9] = 0.0f;
    out[10] = ca;
    out[11] = 0.0f;
    out[12] = 0.0f;
    out[13] = 0.0f;
    out[14] = 0;
    out[15] = 1;
}

void mat4x4_rotateZ(t_mat4x4 out, float deg) {

    float ca = cosf(deg * M_PI / 180.0f);
    float sa = sinf(deg * M_PI / 180.0f);
    out[0] = ca;
    out[1] = -sa;
    out[2] = 0.0f;
    out[3] = 0.0f;
    out[4] = sa;
    out[5] = ca;
    out[6] = 0.0f;
    out[7] = 0.0f;
    out[8] = 0.0f;
    out[9] = 0.0f;
    out[10] = 1;
    out[11] = 0.0f;
    out[12] = 0.0f;
    out[13] = 0.0f;
    out[14] = 0;
    out[15] = 1;
}


void checkGLError(const char *operation) {
#ifndef __EMSCRIPTEN__
    int errorCode = glGetError();

    if (errorCode != 0) {
        printf("Error: %d while %s\n", errorCode, operation);
    }
#endif
}

struct VBORegister submitVBO(float *vertexData, float *uvData, int vertices,
                             unsigned short *indexData,
                             unsigned int indices) {

    unsigned int vertexDataIndex;
    unsigned int uvDataIndex;
    unsigned int indicesIndex;

    glGenBuffers(1, &vertexDataIndex);
    glBindBuffer(GL_ARRAY_BUFFER, vertexDataIndex);
    glBufferData(GL_ARRAY_BUFFER, vertices * sizeof(float) * 3, vertexData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    glGenBuffers(1, &uvDataIndex);
    glBindBuffer(GL_ARRAY_BUFFER, uvDataIndex);
    glBufferData(GL_ARRAY_BUFFER, vertices * sizeof(float) * 2, uvData, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    glGenBuffers(1, &indicesIndex);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesIndex);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices * sizeof(GLushort), indexData,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    struct VBORegister toReturn;
    toReturn.vertexDataIndex = vertexDataIndex;
    toReturn.uvDataIndex = uvDataIndex;
    toReturn.indicesIndex = indicesIndex;
    toReturn.indices = indices;

    return toReturn;
}

void printMessageTo3DView(const char *message);

void drawTriangle(const struct Vec3 pos1,
                  const struct Vec2i uv1,
                  const struct Vec3 pos2,
                  const struct Vec2i uv2,
                  const struct Vec3 pos3,
                  const struct Vec2i uv3,
                  const struct Texture *texture);

void enter2D(void) {
    mat4x4_ortho(projection_matrix, 0.0f, (float) XRES_FRAMEBUFFER, (float) YRES_FRAMEBUFFER, 0.0f,
                 0.1f, 100.0f);
    glUniformMatrix4fv(uProjectionMatrixUniformLocation, 1, GL_FALSE, projection_matrix);

    glDisable(GL_DEPTH_TEST);

    mat4x4_view(viewMatrix, 0, 0, 0, 0, 0, -1, 0, 1, 0);
    glUniformMatrix4fv(uViewMatrixUniformLocation, 1, GL_FALSE, viewMatrix);

    checkGLError("enter2D");
}

void unloadTextures(void) {
    for (int c = 0; c < texturesUsed; ++c) {
        nativeTextures[c]->raw->uploadId = -1;
    }

    whiteTexture.uploadId = -1;

    for ( int c = 0; c < TOTAL_ITEMS; ++c ) {
        if (itemSprites[c] != NULL) {
            itemSprites[c]->raw->uploadId = -1;
        }
    }

    if (defaultFont != NULL) {
        defaultFont->uploadId = -1;
    }

#ifdef TILED_BITMAPS
    for ( int c = 0; c < 8; ++c ) {
        if (mapTopLevel[c] != NULL) {
            mapTopLevel[c]->uploadId = -1;
        }
    }
#else
    if (mapTopLevel != NULL) {
        mapTopLevel->uploadId = -1;
    }
#endif
}

void initGL() {
    GLint status;

    glViewport(0, 0, width, height);

    /* creating shader */
    vs = glCreateShader(GL_VERTEX_SHADER);
    fs = glCreateShader(GL_FRAGMENT_SHADER);

    int length = strlen(vertex_shader);
    glShaderSource(vs, 1, (const GLchar **) &vertex_shader, &length);
    glCompileShader(vs);
    checkGLError("compiling shaders");

    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {

        GLint maxLength = 0;
        glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &maxLength);

        char *errorLog = (char *) malloc(maxLength);
        memset(errorLog, 0, maxLength);

        glGetShaderInfoLog(vs, maxLength, &maxLength, errorLog);

        fprintf(stderr, "vertex shader compilation failed:\n%s", errorLog);
        exit(0);
    }

    length = strlen(fragment_shader);
    glShaderSource(fs, 1, (const GLchar **) &fragment_shader, &length);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &maxLength);

        char *errorLog = (char *) malloc(maxLength);
        memset(errorLog, 0, maxLength);

        glGetShaderInfoLog(fs, maxLength, &maxLength, errorLog);


        fprintf(stderr, "fragment shader compilation failed:\n%s", errorLog);
        exit(0);
    }

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glUseProgram(program);
    checkGLError("creating and using program");


    /* attaching data to shaders */

    aPositionAttributeLocation = glGetAttribLocation(program, "aPosition");
    aTexCoordAttributeLocation = glGetAttribLocation(program, "aTexCoord");
    uProjectionMatrixUniformLocation = glGetUniformLocation(program, "uProjectionMatrix");
    uViewMatrixUniformLocation = glGetUniformLocation(program, "uViewMatrix");
    uTransformMatrixUniformLocation = glGetUniformLocation(program, "uTransformMatrix");

    uRotateXMatrixUniformLocation = glGetUniformLocation(program, "uRotateXMatrix");
    uRotateYMatrixUniformLocation = glGetUniformLocation(program, "uRotateYMatrix");
    uRotateZMatrixUniformLocation = glGetUniformLocation(program, "uRotateZMatrix");

    sTextureUniformLocation = glGetUniformLocation(program, "sTexture");
    uModUniformLocation = glGetUniformLocation(program, "uMod");
    uFadeUniformLocation = glGetUniformLocation(program, "uFade");
    uScaleUniformLocation = glGetUniformLocation(program, "uScale");
    checkGLError("Fetching locations in shaders");

    whiteTexture.height = 1;
    whiteTexture.width = 1;
    whiteRaw[0] = whiteRaw[1] = whiteRaw[2] = whiteRaw[3] = 0xFFFFFFFF;
    whiteTexture.data = &whiteRaw[0];
    submitBitmapToGPU(&whiteTexture);

    planeXYVBO = submitVBO((float *) planeXYVertices, planeXYUVs, 4,
                           (unsigned short *) planeXYIndices, 6);

    planeYZVBO = submitVBO((float *) planeYZVertices, planeYZUVs, 4,
                           (unsigned short *) planeYZIndices, 6);

    leftFarVBO = submitVBO((float *) cornerLeftFarVertices, cornerLeftFarUVs, 4,
                           (unsigned short *) cornerLeftFarIndices, 6);
    leftNearVBO = submitVBO((float *) cornerLeftNearVertices, cornerLeftNearUVs, 4,
                            (unsigned short *) cornerLeftNearIndices, 6);
    floorVBO = submitVBO((float *) floorVertices, floorUVs, 4,
                         (unsigned short *) floorIndices, 6);

    rampVBO = submitVBO((float *) rampVertices, rampUVs, 4,
                        (unsigned short *) rampIndices, 6);
    checkGLError("Creating VBOs");

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(1);
    checkGLError("initGL");

    float fade[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    glUniform4f(uFadeUniformLocation, fade[0], fade[1], fade[2], fade[3]);
    glUniform4f(uModUniformLocation, 1.0f, 1.0f, 1.0f, 1.0f);

    checkGLError("set uniforms");


}

void bindTexture(struct Bitmap *bitmap) {

    if (bitmap->uploadId == -1) {
        submitBitmapToGPU(bitmap);
    }

    glBindTexture(GL_TEXTURE_2D, bitmap->uploadId);
    checkGLError("bind texture");
}

int submitBitmapToGPU(struct Bitmap *bitmap) {
    // Texture object handle
    int textureId = 0;

    //Generate texture storage
    glGenTextures(1, &textureId);

    //specify what we want for that texture
    glBindTexture(GL_TEXTURE_2D, textureId);

    //upload the data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap->width, bitmap->height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, bitmap->data);

    // Set the filtering mode - surprisingly, this is needed.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    checkGLError("submit bitmap to GPU");

    bitmap->uploadId = textureId;

    return textureId;
}


void startFrameGL(int x, int y, int width, int height) {
    visibilityCached = FALSE;
    needsToRedrawVisibleMeshes = FALSE;

    checkGLError("start frame");
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    checkGLError("clear buffers");

    glEnableVertexAttribArray(aPositionAttributeLocation);
    checkGLError("Enabled vertex position attribute");

    glEnableVertexAttribArray(aTexCoordAttributeLocation);
    checkGLError("Enabled vertex uv attribute");
    
    enter2D();
}

void endFrameGL() {
    checkGLError("end frame");
}

void enter3D(void) {
    mat4x4_perspective(projection_matrix, 90.0f,
                       (float) XRES_FRAMEBUFFER / (float) YRES_FRAMEBUFFER, 0.1f, 1024.0f);
    glUniformMatrix4fv(uProjectionMatrixUniformLocation, 1, GL_FALSE, projection_matrix);

    mat4x4_rotateY(viewMatrix, -leanX);
    glUniformMatrix4fv(uViewMatrixUniformLocation, 1, GL_FALSE, viewMatrix);

    glEnable(GL_DEPTH_TEST);
    checkGLError("enter3D");
}
