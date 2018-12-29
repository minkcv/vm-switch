#ifndef IPU_H_
#define IPU_H_

#include "constants.h"
#include "joystick.h"
#include <stdlib.h>
#include <switch.h>

// Input Processing Unit
typedef struct IPU IPU;

struct IPU
{
    JOYSTICK* js1;
    JOYSTICK* js2;
};

IPU* createIPU();

void destroyIPU(IPU* ipu);

void updateIPU(IPU* ipu, uint64_t kDown, uint64_t kUp,
        uint8_t memory[MEMORY_SEGMENT_COUNT][MEMORY_SEGMENT_SIZE]);

void updateJoystick(JOYSTICK* js, uint64_t kDown, uint64_t kUp);

#endif

