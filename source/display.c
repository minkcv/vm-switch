#include "display.h"

Display* createDisplay(int width, int height, int scale, int pitch)
{
    // Init switch gfx
    gfxInitResolution(width, height);
    gfxInitDefault();
    //gfxConfigureResolution(width, height);
    //gfxConfigureCrop(0, 0, width, height);

    Display* display = (Display*)malloc(sizeof(Display));
    display->width = width;
    display->height = height;
    display->scale = scale;
    display->pitch = pitch;
    u32 createdWidth, createdHeight;
    display->framebuffer = (u32*) gfxGetFramebuffer((u32*)&createdWidth, (u32*)&createdHeight);

    /* TODO
    u8* title_image = (u8*)title_image_bin;
    u32 pos;
    u32 x, y;
    u32 image_width = scale * width;
    u32 image_height = scale * height;
    for (x = 0; x < image_width; x++)
    {
        for (y = 0; y < image_height; y++)
        {
            pos = y * width + x;
            display->framebuffer[pos] = RGBA8_MAXALPHA(title_image[pos*3+0], title_image[pos*3+1], title_image[pos*3+2]);
        }
    }
    */
    gfxFlushBuffers();
    gfxSwapBuffers();
    
    // TODO: SDL_Delay(2000); // Show splash screen for 2 seconds
    return display;
}

void updateDisplay(Display* display, GPU* gpu)
{
    u32 pos;
    u32 x, y;
    for (x = 0; x < display->width; x++)
    {
        for (y = 0; y < display->height; y++)
        {
            pos = y * display->width + x;
            display->framebuffer[pos] = RGBA8_MAXALPHA(gpu->pixels[pos*3+0], gpu->pixels[pos*3+1], gpu->pixels[pos*3+2]);
        }
    }
    gfxFlushBuffers();
    gfxSwapBuffers();
}

void quitDisplay(Display* display)
{
    free(display);
    display = NULL;
    gfxExit(); // Does this need to happen before free?
}

