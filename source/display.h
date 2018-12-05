#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "constants.h"
#include "gpu.h"
#include <switch.h>

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

void quitDisplay(Display* display);

// TODO
// raw RGB888 image
// you can make this file with
// $ xxd -i title_image_bin
// where title_image_bin is a raw image data export from gimp.

#endif
