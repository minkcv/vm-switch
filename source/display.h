#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "constants.h"
#include "gpu.h"
#include <SDL2/SDL.h>

typedef struct Display Display;
typedef struct GPU GPU;

struct Display
{
    SDL_Window* window;
    SDL_Surface* surface;
    int width;
    int height;
    int scale;
    int pitch;
};

Display* createDisplay(int width, int height, int scale, int pitch);

void updateDisplay(Display* display);

void quitDisplay(Display* display);

#endif

