#ifndef IPU_H_
#define IPU_H_

#include "constants.h"
#include "joystick.h"
#include <SDL2/SDL.h>

// Input Processing Unit
typedef struct IPU IPU;

struct IPU
{
    JOYSTICK* js1;
    JOYSTICK* js2;
};

IPU* createIPU();

void updateIPU(IPU* ipu, SDL_Event event,
        uint8_t memory[MEMORY_SEGMENT_COUNT][MEMORY_SEGMENT_SIZE]);

void updateJoystick(JOYSTICK* js, SDL_Event event);

#endif

