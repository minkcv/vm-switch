#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "constants.h"
#include "gpu.h"
#include <switch.h>

// raw RGB888 image
//#include "title_image_bin.h"

typedef struct Display Display;
typedef struct GPU GPU;

struct Display
{
    u32* framebuffer;
    int width;
    int height;
    int scale;
    int pitch;
};

Display* createDisplay(int width, int height, int scale, int pitch);

void updateDisplay(Display* display, GPU* gpu);

//void dumpColors(SDL_Color* colors, int ncolors);

void quitDisplay(Display* display);

#endif

