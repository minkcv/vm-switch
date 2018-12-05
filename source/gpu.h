#ifndef GPU_H_
#define GPU_H_

#include "constants.h"
#include "vm.h"
#include <stdint.h>
#include <switch.h>
#include <stdlib.h>

typedef struct VM VM;
typedef struct GPU GPU;

// Sprite Attribute
// Describes a sprite and how it should be rendered
typedef struct 
{
    unsigned int active : 1;
    unsigned int flipHor : 1;
    unsigned int flipVer : 1;
    unsigned int color4Alpha : 1;
    // 4 bits here for use later
    u8 x;
    u8 y;
    u8 width;
    u8 height;
    u8 segmentAddr;
    u8 byteAddr;
    u8 colors[4];
} SpriteAttr;

// Graphics Processing Unit
// Reads System memory and draws sprites / background color to the display back buffer
struct GPU
{
    SpriteAttr sprAttrs[NUM_SPRITES];
    u8* pixels;
    int pitch;
    int bytesPerPixel;
    unsigned int active : 1;
    unsigned int refreshed : 1; // Changes 0 -> 1 or 1 -> 0 when the display refreshes
};

GPU* createGPU();

void updateGPU(GPU* gpu, u8 memory[MEMORY_SEGMENT_COUNT][MEMORY_SEGMENT_SIZE]);

void readSpritesFromMem(GPU* gpu, u8 memory[MEMORY_SEGMENT_COUNT][MEMORY_SEGMENT_SIZE]);

void drawSprites(GPU* gpu, u8 memory[MEMORY_SEGMENT_COUNT][MEMORY_SEGMENT_SIZE]);

u8 getRed(u8 paletteIndex);
u8 getGreen(u8 paletteIndex);
u8 getBlue(u8 paletteIndex);


#endif

