#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "constants.h"
#include "gpu.h"
#include <SDL2/SDL.h>
#include <EGL/egl.h>    // EGL library
#include <EGL/eglext.h> // EGL extensions
#include <glad/glad.h>  // glad library (OpenGL loader)

typedef struct Display Display;
typedef struct GPU GPU;

static const char* const vertexShaderSource = R"text(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
out gl_PerVertex
{
	vec4 gl_Position;
};
layout (location = 0) in int inAttr;
layout (location = 0) out vec4 outColor;
layout (location = 1) out vec3 outUV;
uniform ivec2 dimensions;
uniform vec4 palettes[16] = vec4[](
	vec4(0.0, 0.0, 0.0, 1.0),
	vec4(0.5, 0.0, 0.0, 1.0),
	vec4(0.0, 0.5, 0.0, 1.0),
	vec4(0.5, 0.5, 0.0, 1.0),
	vec4(0.0, 0.0, 0.5, 1.0),
	vec4(0.5, 0.0, 0.5, 1.0),
	vec4(0.0, 0.5, 0.5, 1.0),
	vec4(0.75, 0.75, 0.75, 1.0),
	vec4(0.5, 0.5, 0.5, 1.0),
	vec4(1.0, 0.0, 0.0, 1.0),
	vec4(0.0, 1.0, 0.0, 1.0),
	vec4(1.0, 1.0, 0.0, 1.0),
	vec4(0.0, 0.0, 1.0, 1.0),
	vec4(1.0, 0.0, 1.0, 1.0),
	vec4(0.0, 1.0, 1.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0)
);
const vec2 builtin_vertices[] = vec2[](
	vec2(0.0, 0.0),
	vec2(0.0, -1.0),
	vec2(1.0, -1.0),
	vec2(0.0, 0.0),
	vec2(1.0, -1.0),
	vec2(1.0, 0.0)
);
void main()
{
	// Extract data from the attribute
	//float tileId = float(inAttr & 0x3FF);
	//bool hFlip = ((inAttr >> 10) & 1) != 0;
	//bool vFlip = ((inAttr >> 11) & 1) != 0;
	//int palId = (inAttr >> 12) & 0xF;
	//vec2 vtxData = builtin_vertices[gl_VertexID];
	//// Position
	//float tileRow = floor(float(gl_InstanceID) / dimensions.x);
	//float tileCol = float(gl_InstanceID) - tileRow*dimensions.x;
	//vec2 basePos;
	//basePos.x = 2.0 * tileCol / dimensions.x - 1.0;
	//basePos.y = 2.0 * (1.0 - tileRow / dimensions.y) - 1.0;
	//vec2 offsetPos = vec2(2.0) / vec2(dimensions.xy);
	//gl_Position.xy = basePos + offsetPos * vtxData;
	//gl_Position.zw = vec2(0.0, 1.0);
	// Color
	outColor = inAttr;
	// UVs
	//if (hFlip)
	//	vtxData.x = 1.0 - vtxData.x;
	//if (vFlip)
	//	vtxData.y = -1.0 - vtxData.y;
	//outUV.xy = vec2(0.0,1.0) + vtxData;
	//outUV.z  = tileId;
}
)text";

static const char* const fragmentShaderSource = R"text(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout (location = 0) in vec4 inColor;
layout (location = 1) in vec3 inUV;
layout (location = 0) out vec4 outColor;
uniform sampler2DArray tileset;
void main()
{
	//float alpha = inColor.a * texture(tileset, inUV).r;
	//if (alpha < 0.1) discard;
	outColor.rgb = inColor.rgb;
	outColor.a = alpha;
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
    GLuint s_tilemapVsh, s_tilemapFsh;
	GLuint s_tilemapPipeline;
	GLuint s_tilemapVao, s_tilemapVbo;
};

GLuint loadShaderProgram(GLenum type, const char* source);

Display* createDisplay(int width, int height, int scale, int pitch);

int initEGL(Display* display);

Display* createDisplay(int width, int height, int scale, int pitch);

void updateDisplay(Display* display, GPU* gpu);

void deinitEGL(Display* display);

void quitDisplay(Display* display);

#endif

