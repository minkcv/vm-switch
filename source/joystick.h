#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#include <switch.h>

typedef struct JOYSTICK JOYSTICK;

struct JOYSTICK
{
    u64 buttons[NUM_JOYSTICK_BUTTONS]; // Up, Down, Left, Right, A, B, C, D
    u8 keysAsBits; // LSB = Up, MSB = D. 1 = pressed, 0 = not pressed
};

#endif
