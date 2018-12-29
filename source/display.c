#include "display.h"

GLuint loadShaderProgram(GLenum type, const char* source)
{
	GLuint handle = glCreateShader(type);
	glShaderSource(handle, 1, &source, NULL);
	glCompileShader(handle);
	GLint compiled;
	glGetShaderiv(handle, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		glDeleteShader(handle);
		return 0;
	}
	return handle;
}

Display* createDisplay(int width, int height, int scale, int pitch)
{
    SDL_Init(SDL_INIT_JOYSTICK);

    Display* display = (Display*)malloc(sizeof(Display));
    display->width = width;
    display->height = height;
    display->scale = scale;
    display->pitch = pitch;
    initEGL(display);
    gladLoadGL();

    // Load our shaders
	display->vertexShader = loadShaderProgram(GL_VERTEX_SHADER, vertexShaderSource);
	display->fragmentShader = loadShaderProgram(GL_FRAGMENT_SHADER, fragmentShaderSource);

	display->shaderProgram = glCreateProgram();
	glAttachShader(display->shaderProgram, display->vertexShader);
    glAttachShader(display->shaderProgram, display->fragmentShader);
 
    glLinkProgram(display->shaderProgram);
    glUseProgram(display->shaderProgram);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &(display->textureId));
	glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
	glBindTexture(GL_TEXTURE_2D, display->textureId);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	display->samplerLoc = glGetUniformLocation(display->shaderProgram, "s_texture");

    // Other miscellaneous init
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Configure viewport
	glViewport(0, 0, 1280, 800);

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
	if (eglBindAPI(EGL_OPENGL_ES_API) == EGL_FALSE)
	{
		//TRACE("Could not set API! error: %d", eglGetError());
		goto _fail1;
	}

	// Get an appropriate EGL framebuffer configuration
	EGLConfig config;
	EGLint numConfigs;
	static const EGLint framebufferAttributeList[] =
	{
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
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
		EGL_CONTEXT_CLIENT_VERSION, 2,
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
    GLfloat vVertices[] = { -0.5f,  0.5f, 0.0f, // Position 0
                            0.0f,  0.0f,        // TexCoord 0 
                           -0.5f, -0.5f, 0.0f,  // Position 1
                            0.0f,  1.0f,        // TexCoord 1
                            0.5f, -0.5f, 0.0f,  // Position 2
                            1.0f,  1.0f,        // TexCoord 2
                            0.5f,  0.5f, 0.0f,  // Position 3
                            1.0f,  0.0f         // TexCoord 3
                         };
   	GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

   	// Set the viewport
   	//glViewport ( 0, 0, display->width, display->height );

   	glClearColor(0, 0, 0, 1.0);

   	// Clear the color buffer
   	glClear ( GL_COLOR_BUFFER_BIT );

   	// Use the program object
   	//glUseProgram ( display->vertexShader );

   	// Load the vertex position
   	glVertexAttribPointer ( 0, 3, GL_FLOAT,
                           GL_FALSE, 5 * sizeof ( GLfloat ), vVertices );
   	// Load the texture coordinate
   	glVertexAttribPointer ( 1, 2, GL_FLOAT,
                           GL_FALSE, 5 * sizeof ( GLfloat ), &vVertices[3] );

   	glEnableVertexAttribArray ( 0 );
   	glEnableVertexAttribArray ( 1 );

   	// Bind the texture
   	glActiveTexture ( GL_TEXTURE0 );
   	glBindTexture ( GL_TEXTURE_2D, display->textureId );
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, display->width, display->height, 0, GL_RGB, GL_UNSIGNED_BYTE, gpu->pixels);

   	// Set the sampler texture unit to 0
   	glUniform1i ( display->samplerLoc, 0 );

   	glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );

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
	glDeleteProgram(display->shaderProgram);
	glDeleteProgram(display->fragmentShader);
	glDeleteProgram(display->vertexShader);
    deinitEGL(display);
    free(display);
    display = NULL;
    SDL_Quit();
}
