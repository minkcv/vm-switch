#include "display.h"

GLuint loadShaderProgram(GLenum type, const char* source)
{
	GLint success;
	GLchar msg[512];

	GLuint handle = glCreateShaderProgramv(type, 1, &source);
	glGetProgramiv(handle, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		glGetProgramInfoLog(handle, sizeof(msg), NULL, msg);
		//TRACE("Shader error: %s", msg);
		glDeleteProgram(handle);
		handle = 0;
	}

	return handle;
}

Display* createDisplay(int width, int height, int scale, int pitch)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

    Display* display = (Display*)malloc(sizeof(Display));
    display->width = width;
    display->height = height;
    display->scale = scale;
    display->pitch = pitch;
    initEGL(display);
    gladLoadGL();

    // Load our shaders
	display->s_tilemapVsh = loadShaderProgram(GL_VERTEX_SHADER, vertexShaderSource);
	display->s_tilemapFsh = loadShaderProgram(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // Configure tilemap dimensions
	glProgramUniform2i(display->s_tilemapVsh, glGetUniformLocation(display->s_tilemapVsh, "dimensions"),
		display->width, display->height
	);

    // Create a program pipeline and attach the programs to their respective stages
	glGenProgramPipelines(1, &(display->s_tilemapPipeline));
	glUseProgramStages(display->s_tilemapPipeline, GL_VERTEX_SHADER_BIT,   display->s_tilemapVsh);
	glUseProgramStages(display->s_tilemapPipeline, GL_FRAGMENT_SHADER_BIT, display->s_tilemapFsh);

	// Create a VAO and a VBO for the tilemap
	glGenVertexArrays(1, &(display->s_tilemapVao));
	glBindVertexArray(display->s_tilemapVao);

	// Allocate the tilemap data
	glGenBuffers(1, &(display->s_tilemapVbo));
	glBindBuffer(GL_ARRAY_BUFFER, display->s_tilemapVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uint8_t) * display->width * display->height, NULL, GL_DYNAMIC_DRAW);

	// Configure the only vertex attribute (which is per-instance)
	glVertexAttribIPointer(0, 1, GL_UNSIGNED_BYTE, sizeof(uint8_t), (void*)0);
	glVertexAttribDivisor(0, 1);
	glEnableVertexAttribArray(0);

	// We're done with the VBO/VAO, unbind them
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

    // Other miscellaneous init
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Configure viewport
	glViewport(0, 0, 1280, 720);

    return display;
}

int initEGL(Display* display)
{
    // Connect to the EGL default display
	display->s_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (!display->s_display)
	{
		//TRACE("Could not connect to display! error: %d", eglGetError());
		goto _fail0;
	}

	// Initialize the EGL display connection
	eglInitialize(display->s_display, NULL, NULL);

	// Select OpenGL (Core) as the desired graphics API
	if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE)
	{
		//TRACE("Could not set API! error: %d", eglGetError());
		goto _fail1;
	}

	// Get an appropriate EGL framebuffer configuration
	EGLConfig config;
	EGLint numConfigs;
	static const EGLint framebufferAttributeList[] =
	{
		EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
		EGL_RED_SIZE,     8,
		EGL_GREEN_SIZE,   8,
		EGL_BLUE_SIZE,    8,
		EGL_ALPHA_SIZE,   8,
		EGL_NONE
	};
	eglChooseConfig(display->s_display, framebufferAttributeList, &config, 1, &numConfigs);
	if (numConfigs == 0)
	{
		//TRACE("No config found! error: %d", eglGetError());
		goto _fail1;
	}

	// Create an EGL window surface
	display->s_surface = eglCreateWindowSurface(display->s_display, config, (char*)"", NULL);
	if (!display->s_surface)
	{
		//TRACE("Surface creation failed! error: %d", eglGetError());
		goto _fail1;
	}

	// Create an EGL rendering context
	static const EGLint contextAttributeList[] =
	{
		EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR,
		EGL_CONTEXT_MAJOR_VERSION_KHR, 4,
		EGL_CONTEXT_MINOR_VERSION_KHR, 3,
		EGL_NONE
	};
	display->s_context = eglCreateContext(display->s_display, config, EGL_NO_CONTEXT, contextAttributeList);
	if (!display->s_context)
	{
		//TRACE("Context creation failed! error: %d", eglGetError());
		goto _fail2;
	}

	// Connect the context to the surface
	eglMakeCurrent(display->s_display, display->s_surface, display->s_surface, display->s_context);
	return 0;

_fail2:
	eglDestroySurface(display->s_display, display->s_surface);
	display->s_surface = NULL;
_fail1:
	eglTerminate(display->s_display);
	display->s_display = NULL;
_fail0:
	return 1;
}

void updateDisplay(Display* display, GPU* gpu)
{
    // Clear the framebuffer
	glClearColor(0x10/255.0f, 0x10/255.0f, 0x10/255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Update tilemap
	glBindBuffer(GL_ARRAY_BUFFER, display->s_tilemapVbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(uint8_t) * display->width * display->height, gpu->pixels);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Draw the tilemap
	glBindProgramPipeline(display->s_tilemapPipeline);
	glBindVertexArray(display->s_tilemapVao);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, display->width * display->height);
	glBindVertexArray(0);

	// Swap buffers
	eglSwapBuffers(display->s_display, display->s_surface);
}

void deinitEGL(Display* display)
{
    if (display->s_display)
	{
		eglMakeCurrent(display->s_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (display->s_context)
		{
			eglDestroyContext(display->s_display, display->s_context);
			display->s_context = NULL;
		}
		if (display->s_surface)
		{
			eglDestroySurface(display->s_display, display->s_surface);
			display->s_surface = NULL;
		}
		eglTerminate(display->s_display);
		display->s_display = NULL;
	}
}

void quitDisplay(Display* display)
{
	glDeleteBuffers(1, &(display->s_tilemapVbo));
	glDeleteVertexArrays(1, &(display->s_tilemapVao));
	glDeleteProgramPipelines(1, &(display->s_tilemapPipeline));
	glDeleteProgram(display->s_tilemapFsh);
	glDeleteProgram(display->s_tilemapVsh);
    deinitEGL(display);
    free(display);
    display = NULL;
    SDL_Quit();
}
