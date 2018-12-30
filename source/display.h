#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "constants.h"
#include "gpu.h"
#include <EGL/egl.h>    // EGL library
#include <EGL/eglext.h> // EGL extensions
#include <glad/glad.h>  // glad library (OpenGL loader)

typedef struct Display Display;
typedef struct GPU GPU;

static const char* const vertexShaderSource = R"text(
#version 300 es
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec2 a_texCoord;
out vec2 v_texCoord;
void main()
{
    gl_Position = a_position;
    v_texCoord = a_texCoord;
}
)text";

static const char* const fragmentShaderSource = R"text(
#version 300 es
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
precision mediump float;
in vec2 v_texCoord;
layout(location = 0) out vec4 outColor;
uniform sampler2D s_texture;
void main()
{
    outColor = texture(s_texture, v_texCoord);
}
)text";

struct Display
{
    int width;
    int height;
    int scale;
    int pitch;
    EGLDisplay s_display;
    EGLContext s_context;
    EGLSurface s_surface;
    GLuint vertexShader, fragmentShader;
    GLuint shaderProgram;
    GLint  samplerLoc;
    GLuint textureId;
};

GLuint loadShaderProgram(GLenum type, const char* source);

Display* createDisplay(int width, int height, int scale, int pitch);

int initEGL(Display* display);

void updateDisplay(Display* display, GPU* gpu);

void deinitEGL(Display* display);

void quitDisplay(Display* display);

#endif

