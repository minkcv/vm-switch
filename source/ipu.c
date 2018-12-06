#include "ipu.h"

IPU* createIPU()
{
    IPU* ipu = (IPU*)malloc(sizeof(IPU));
    JOYSTICK* js1 = (JOYSTICK*)malloc(sizeof(JOYSTICK));
    js1->buttons[0] = KEY_RSTICK_UP;
    js1->buttons[1] = KEY_RSTICK_DOWN;
    js1->buttons[2] = KEY_RSTICK_LEFT;
    js1->buttons[3] = KEY_RSTICK_RIGHT;
    js1->buttons[4] = KEY_A;
    js1->buttons[5] = KEY_B;
    js1->buttons[6] = KEY_X;
    js1->buttons[7] = KEY_Y;
    js1->keysAsBits = 0;
    ipu->js1 = js1;

    JOYSTICK* js2 = (JOYSTICK*)malloc(sizeof(JOYSTICK));
    
    js2->buttons[0] = KEY_LSTICK_UP;
    js2->buttons[1] = KEY_LSTICK_DOWN;
    js2->buttons[2] = KEY_LSTICK_LEFT;
    js2->buttons[3] = KEY_LSTICK_RIGHT;
    js2->buttons[4] = KEY_DUP;
    js2->buttons[5] = KEY_DDOWN;
    js2->buttons[6] = KEY_DRIGHT;
    js2->buttons[7] = KEY_DLEFT;
    js2->keysAsBits = 0;
    ipu->js2 = js2;

    return ipu;
}

void updateIPU(IPU* ipu, u32 kDown, u32 kUp, u32 kHeld,
        u8 memory[MEMORY_SEGMENT_COUNT][MEMORY_SEGMENT_SIZE])
{
    updateJoystick(ipu->js1, kDown, kUp, kHeld);
    updateJoystick(ipu->js2, kDown, kUp, kHeld);
    memory[JOYSTICK_SEG][JOYSTICK_1_OFFSET] = ipu->js1->keysAsBits;
    memory[JOYSTICK_SEG][JOYSTICK_2_OFFSET] = ipu->js2->keysAsBits;
}

void updateJoystick(JOYSTICK* js, u32 kDown, u32 kUp, u32 kHeld)
{
    int i;
    u8 bit = 1;
    for (i = 0; i < NUM_JOYSTICK_BUTTONS; i++)
    {
        if ((kDown & js->buttons[i]) == js->buttons[i])
        {
            // Leave the other bits but ensure the ith bit is 1
            js->keysAsBits |= (bit << i);
        }
        if ((kHeld & js->buttons[i]) == js->buttons[i])
        {
            // Leave the other bits but ensure the ith bit is 1
            js->keysAsBits |= (bit << i);
        }
        if ((kUp & js->buttons[i]) == js->buttons[i])
        {
            // Leave the other bits but ensure the ith bit is 0
            js->keysAsBits &= (~(bit << i)); // Bitwise inversion
        }
    }
}

