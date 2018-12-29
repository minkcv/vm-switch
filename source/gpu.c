#include "gpu.h"
#include <stdlib.h>
#include <EGL/egl.h>    // EGL library
#include <EGL/eglext.h> // EGL extensions
#include <glad/glad.h>  // glad library (OpenGL loader)

GPU* createGPU(Display* display, uint8_t* pixels)
{
    GPU* gpu = (GPU*)malloc(sizeof(GPU));
    gpu->active = 1;
    gpu->bytesPerPixel = 3;
    gpu->pitch = SCREEN_WIDTH * gpu->bytesPerPixel;
    gpu->scale = display->scale;
    gpu->leftMargin = (1280 / 2) - ((SCREEN_WIDTH * gpu->scale) / 2);
    gpu->topMargin = (720 / 2) - ((SCREEN_HEIGHT * gpu->scale) / 2);
    gpu->pixels = pixels;
    return gpu;
}

void destroyGPU(GPU* gpu)
{
    free(gpu);
}

void updateGPU(GPU* gpu, uint8_t memory[MEMORY_SEGMENT_COUNT][MEMORY_SEGMENT_SIZE])
{
    gpu->active = memory[GPU_FLAG_SEG][GPU_FLAG_OFFSET] & 0x1;
    if (gpu->active)
    {
        // Preserve other bits, only change the second bit
        gpu->refreshed = !gpu->refreshed;
        memory[GPU_FLAG_SEG][GPU_FLAG_OFFSET] &= 0xFD; // Clear the second bit
        memory[GPU_FLAG_SEG][GPU_FLAG_OFFSET] |= (gpu->refreshed << 1); // Set the second bit according to gpu->refreshed
    }
}

void readSpritesFromMem(GPU* gpu, uint8_t memory[MEMORY_SEGMENT_COUNT][MEMORY_SEGMENT_SIZE])
{
    int i;
    for (i = 0; i < NUM_SPRITES; i++)
    {
        uint8_t flags = memory[SPRITE_ATTR_SEG][i * SPRITE_ATTR_LENGTH];
        gpu->sprAttrs[i].active = flags >> 7;
        gpu->sprAttrs[i].flipHor = flags >> 6 & 0x1;
        gpu->sprAttrs[i].flipVer = flags >> 5 & 0x1;
        gpu->sprAttrs[i].color4Alpha = flags >> 4 & 0x1;
        gpu->sprAttrs[i].x = memory[SPRITE_ATTR_SEG][i * SPRITE_ATTR_LENGTH + 1];
        gpu->sprAttrs[i].y = memory[SPRITE_ATTR_SEG][i * SPRITE_ATTR_LENGTH + 2];
        gpu->sprAttrs[i].width = memory[SPRITE_ATTR_SEG][i * SPRITE_ATTR_LENGTH + 3];
        gpu->sprAttrs[i].height = memory[SPRITE_ATTR_SEG][i * SPRITE_ATTR_LENGTH + 4];
        gpu->sprAttrs[i].segmentAddr = memory[SPRITE_ATTR_SEG][i * SPRITE_ATTR_LENGTH + 5];
        gpu->sprAttrs[i].byteAddr = memory[SPRITE_ATTR_SEG][i * SPRITE_ATTR_LENGTH + 6];
        gpu->sprAttrs[i].colors[0] = memory[SPRITE_ATTR_SEG][i * SPRITE_ATTR_LENGTH + 7];
        gpu->sprAttrs[i].colors[1] = memory[SPRITE_ATTR_SEG][i * SPRITE_ATTR_LENGTH + 8];
        gpu->sprAttrs[i].colors[2] = memory[SPRITE_ATTR_SEG][i * SPRITE_ATTR_LENGTH + 9];
        gpu->sprAttrs[i].colors[3] = memory[SPRITE_ATTR_SEG][i * SPRITE_ATTR_LENGTH + 10];
    }
}

void drawSprites(GPU* gpu, uint8_t memory[MEMORY_SEGMENT_COUNT][MEMORY_SEGMENT_SIZE])
{
    int i;
    uint8_t bgColorIndex = memory[BACK_COLOR_SEG][BACK_COLOR_OFFSET];
    uint8_t bgRed = getRed(bgColorIndex);
    uint8_t bgGreen = getGreen(bgColorIndex);
    uint8_t bgBlue = getBlue(bgColorIndex);
    for (i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * gpu->bytesPerPixel; i+=gpu->bytesPerPixel)
    {
        *(gpu->pixels + i) = bgRed;
        *(gpu->pixels + i + 1) = bgGreen;
        *(gpu->pixels + i + 2) = bgBlue;
    }
    for (i = 0; i < NUM_SPRITES; i++)
    {
        if (gpu->sprAttrs[i].active && 
                gpu->sprAttrs[i].x + gpu->sprAttrs[i].width <= SCREEN_WIDTH &&
                gpu->sprAttrs[i].y + gpu->sprAttrs[i].height <= SCREEN_HEIGHT)
        {
            int w, h;
            int x = gpu->sprAttrs[i].x;
            int y = gpu->sprAttrs[i].y;
            int flipHor = gpu->sprAttrs[i].flipHor;
            int flipVer = gpu->sprAttrs[i].flipVer;
            int width = gpu->sprAttrs[i].width;
            int height = gpu->sprAttrs[i].height;
            uint8_t* sprite = &memory[gpu->sprAttrs[i].segmentAddr][gpu->sprAttrs[i].byteAddr];
            for (h = 0; h < gpu->sprAttrs[i].height; h++)
            {
                for (w = 0; w < gpu->sprAttrs[i].width / 4; w++)
                {
                    uint8_t fourPixels = 0;
                    if (flipHor == 0 && flipVer == 0)
                        fourPixels = *(sprite + w + (h * width / 4));
                    else if (flipHor == 1 && flipVer == 0)
                    {
                        uint8_t fourReversedPixels = *(sprite + (width / 4) - w - 1 + (h * width / 4));
                        fourPixels |= (fourReversedPixels & 0xC0) >> 6;
                        fourPixels |= (fourReversedPixels & 0x30) >> 2;
                        fourPixels |= (fourReversedPixels & 0x0C) << 2;
                        fourPixels |= (fourReversedPixels & 0x3) << 6;
                    }
                    else if (flipHor == 0 && flipVer == 1)
                    {
                        fourPixels = *(sprite + w + (height * width / 4) - ((h + 1) * width / 4));
                    }
                    else if (flipHor == 1 && flipVer == 1)
                    {
                        uint8_t fourReversedPixels = *(sprite + (width / 4) - w - 1 + (height * width / 4) - ((h + 1) * width / 4));
                        fourPixels |= (fourReversedPixels & 0xC0) >> 6;
                        fourPixels |= (fourReversedPixels & 0x30) >> 2;
                        fourPixels |= (fourReversedPixels & 0x0C) << 2;
                        fourPixels |= (fourReversedPixels & 0x3) << 6;
                    }
                    // Sprite color palette indicies
                    uint8_t bits1 = (fourPixels >> 6) & 0x3; // MSB
                    uint8_t bits2 = (fourPixels >> 4) & 0x3;
                    uint8_t bits3 = (fourPixels >> 2) & 0x3;
                    uint8_t bits4 = fourPixels & 0x3;
                    // These are full color palette indicies
                    uint8_t pixel1 = gpu->sprAttrs[i].colors[bits1];
                    uint8_t pixel2 = gpu->sprAttrs[i].colors[bits2];
                    uint8_t pixel3 = gpu->sprAttrs[i].colors[bits3];
                    uint8_t pixel4 = gpu->sprAttrs[i].colors[bits4];
                    uint8_t* curPixel = gpu->pixels + (x * gpu->bytesPerPixel) + (y * gpu->pitch) + (w * gpu->bytesPerPixel * 4) + (h * gpu->pitch);
                    if (!(gpu->sprAttrs[i].color4Alpha && bits1 == 0x3))
                    {
                        *curPixel = getRed(pixel1);
                        *(curPixel + 1) = getGreen(pixel1);
                        *(curPixel + 2) = getBlue(pixel1);
                    }
                    if (!(gpu->sprAttrs[i].color4Alpha && bits2 == 0x3))
                    {
                        *(curPixel + 3) = getRed(pixel2);
                        *(curPixel + 4) = getGreen(pixel2);
                        *(curPixel + 5) = getBlue(pixel2);
                    }
                    if (!(gpu->sprAttrs[i].color4Alpha && bits3 == 0x3))
                    {
                        *(curPixel + 6) = getRed(pixel3);
                        *(curPixel + 7) = getGreen(pixel3);
                        *(curPixel + 8) = getBlue(pixel3);
                    }
                    if (!(gpu->sprAttrs[i].color4Alpha && bits4 == 0x3))
                    {
                        *(curPixel + 9) = getRed(pixel4);
                        *(curPixel + 10) = getGreen(pixel4);
                        *(curPixel + 11) = getBlue(pixel4);
                    }
                }
            }
        }
    }
}

uint8_t getRed(uint8_t paletteIndex)
{
    return (paletteIndex / 32) * 36;
}

uint8_t getGreen(uint8_t paletteIndex)
{
    return (paletteIndex % 32) / 4 * 36;
}

uint8_t getBlue(uint8_t paletteIndex)
{
    return (paletteIndex % 4) * 85;
}

