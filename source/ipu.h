#ifndef IPU_H_
#define IPU_H_

#include "constants.h"
#include "joystick.h"
#include <switch.h>
#include <stdlib.h>

// Input Processing Unit
typedef struct IPU IPU;

struct IPU
{
    JOYSTICK* js1;
    JOYSTICK* js2;
};

IPU* createIPU();

void updateIPU(IPU* ipu, u32 kDown, u32 kUp, u32 kHeld,
        u8 memory[MEMORY_SEGMENT_COUNT][MEMORY_SEGMENT_SIZE]);

void updateJoystick(JOYSTICK* js, u32 kDown, u32 kUp, u32 kHeld);

#endif

