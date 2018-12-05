#include "gpu.h"
#include <stdlib.h>

GPU* createGPU()
{
    GPU* gpu = (GPU*)malloc(sizeof(GPU));
    gpu->active = 1;
    gpu->bytesPerPixel = 3; // From SDL_PIXELFORMAT_RGB24 in display.c
    gpu->pitch = SCREEN_WIDTH * gpu->bytesPerPixel;
    gpu->pixels = malloc(sizeof(u8) * gpu->bytesPerPixel * SCREEN_WIDTH * SCREEN_HEIGHT);
    return gpu;
}

void updateGPU(GPU* gpu, u8 memory[MEMORY_SEGMENT_COUNT][MEMORY_SEGMENT_SIZE])
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

void readSpritesFromMem(GPU* gpu, u8 memory[MEMORY_SEGMENT_COUNT][MEMORY_SEGMENT_SIZE])
{
    int i;
    for (i = 0; i < NUM_SPRITES; i++)
    {
        u8 flags = memory[SPRITE_ATTR_SEG][i * SPRITE_ATTR_LENGTH];
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

void drawSprites(GPU* gpu, u8 memory[MEMORY_SEGMENT_COUNT][MEMORY_SEGMENT_SIZE])
{
    int i;
    u8* pixels = gpu->pixels;
    u8 bgColorIndex = memory[BACK_COLOR_SEG][BACK_COLOR_OFFSET];
    u8 bgRed = getRed(bgColorIndex);
    u8 bgGreen = getGreen(bgColorIndex);
    u8 bgBlue = getBlue(bgColorIndex);
    for (i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * gpu->bytesPerPixel; i+=gpu->bytesPerPixel)
    {
        *(pixels + i) = bgRed;
        *(pixels + i + 1) = bgGreen;
        *(pixels + i + 2) = bgBlue;
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
            u8* sprite = &memory[gpu->sprAttrs[i].segmentAddr][gpu->sprAttrs[i].byteAddr];
            for (h = 0; h < gpu->sprAttrs[i].height; h++)
            {
                for (w = 0; w < gpu->sprAttrs[i].width / 4; w++)
                {
                    u8 fourPixels = 0;
                    if (flipHor == 0 && flipVer == 0)
                        fourPixels = *(sprite + w + (h * width / 4));
                    else if (flipHor == 1 && flipVer == 0)
                    {
                        u8 fourReversedPixels = *(sprite + (width / 4) - w - 1 + (h * width / 4));
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
                        u8 fourReversedPixels = *(sprite + (width / 4) - w - 1 + (height * width / 4) - ((h + 1) * width / 4));
                        fourPixels |= (fourReversedPixels & 0xC0) >> 6;
                        fourPixels |= (fourReversedPixels & 0x30) >> 2;
                        fourPixels |= (fourReversedPixels & 0x0C) << 2;
                        fourPixels |= (fourReversedPixels & 0x3) << 6;
                    }
                    // Sprite color palette indicies
                    u8 bits1 = (fourPixels >> 6) & 0x3; // MSB
                    u8 bits2 = (fourPixels >> 4) & 0x3;
                    u8 bits3 = (fourPixels >> 2) & 0x3;
                    u8 bits4 = fourPixels & 0x3;
                    // These are full color palette indicies
                    u8 pixel1 = gpu->sprAttrs[i].colors[bits1];
                    u8 pixel2 = gpu->sprAttrs[i].colors[bits2];
                    u8 pixel3 = gpu->sprAttrs[i].colors[bits3];
                    u8 pixel4 = gpu->sprAttrs[i].colors[bits4];
                    u8* curPixel = pixels + (x * gpu->bytesPerPixel) + (y * gpu->pitch) + (w * gpu->bytesPerPixel * 4) + (h * gpu->pitch);
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

u8 getRed(u8 paletteIndex)
{
    return (paletteIndex / 32) * 36;
}

u8 getGreen(u8 paletteIndex)
{
    return (paletteIndex % 32) / 4 * 36;
}

u8 getBlue(u8 paletteIndex)
{
    return (paletteIndex % 4) * 85;
}

