#include "display.h"

Display* createDisplay(int width, int height, int scale, int pitch)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

    Display* display = (Display*)malloc(sizeof(Display));
    display->width = width;
    display->height = height;
    display->scale = scale;
    display->pitch = pitch;
    display->window = SDL_CreateWindow("Vapor Spec", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
            width * scale, height * scale, SDL_WINDOW_SHOWN);
    display->surface = SDL_GetWindowSurface(display->window);
    /* TODO?
    SDL_Surface* splash = SDL_LoadBMP("title.bmp");
    SDL_Texture* splashTex = SDL_CreateTextureFromSurface(display->renderer, splash);
    SDL_RenderCopy(display->renderer, splashTex, NULL, NULL);
    SDL_RenderPresent(display->renderer);
    SDL_Delay(2000); // Show splash screen for 2 seconds
    */
    return display;
}

void updateDisplay(Display* display)
{
    SDL_UpdateWindowSurface(display->window);
}

void quitDisplay(Display* display)
{
    display->window = NULL;
    free(display);
    display = NULL;
    SDL_Quit();
}
