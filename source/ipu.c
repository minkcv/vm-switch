#include "ipu.h"

/* From devkitPro sdl fork, is this what the controlls are doing?
    Not even reading analog values on the joysticks?
    I suppose that the switch is a proprietary platform so I'm happy with what I have.
static const HidControllerKeys pad_mapping[] = {
    KEY_A, KEY_B, KEY_X, KEY_Y, // 0, 1, 2, 3
    KEY_LSTICK, KEY_RSTICK, // 4, 5
    KEY_L, KEY_R, // 6, 7
    KEY_ZL, KEY_ZR, // 8, 9
    KEY_PLUS, KEY_MINUS, // 10, 11
    KEY_DLEFT, KEY_DUP, KEY_DRIGHT, KEY_DDOWN, // 12, 13, 14, 15
    KEY_LSTICK_LEFT, KEY_LSTICK_UP, KEY_LSTICK_RIGHT, KEY_LSTICK_DOWN, // 16, 17, 18, 19
    KEY_RSTICK_LEFT, KEY_RSTICK_UP, KEY_RSTICK_RIGHT, KEY_RSTICK_DOWN, // 20, 21, 22, 23
    KEY_SL_LEFT, KEY_SR_LEFT, KEY_SL_RIGHT, KEY_SR_RIGHT // 24, 25, 26, 28
};
*/

IPU* createIPU()
{
    IPU* ipu = (IPU*)malloc(sizeof(IPU));
    JOYSTICK* js1 = (JOYSTICK*)malloc(sizeof(JOYSTICK));
    js1->buttons[0] = 21;
    js1->buttons[1] = 23;
    js1->buttons[2] = 20;
    js1->buttons[3] = 22;
    js1->buttons[4] = 0; // A
    js1->buttons[5] = 1; // B
    js1->buttons[6] = 2; // X
    js1->buttons[7] = 3; // Y
    js1->keysAsBits = 0;
    if (SDL_NumJoysticks() >= 1)
        js1->sdlJoystick = SDL_JoystickOpen(0);

    // MAJOR TODO, the joystick needs closing and do the joysticks need freeing as well?
    js1->which = 0;;
    ipu->js1 = js1;

    JOYSTICK* js2 = (JOYSTICK*)malloc(sizeof(JOYSTICK));
    js2->buttons[0] = 17;
    js2->buttons[1] = 19;
    js2->buttons[2] = 16;
    js2->buttons[3] = 18;
    js2->buttons[4] = 12; // Left
    js2->buttons[5] = 13; // Up
    js2->buttons[6] = 14; // Right
    js2->buttons[7] = 15; // Down
    js2->keysAsBits = 0;
    // Only call SDL_JoystickOpen ONCE when joycons are railed.
    js2->sdlJoystick = (js1->sdlJoystick);
    js2->which = 1;
    ipu->js2 = js2;

    return ipu;
}

void updateIPU(IPU* ipu, SDL_Event event, 
    uint8_t memory[MEMORY_SEGMENT_COUNT][MEMORY_SEGMENT_SIZE])
{
    updateJoystick(ipu->js1, event);
    updateJoystick(ipu->js2, event);
    memory[JOYSTICK_SEG][JOYSTICK_1_OFFSET] = ipu->js1->keysAsBits;
    memory[JOYSTICK_SEG][JOYSTICK_2_OFFSET] = ipu->js2->keysAsBits;
}

void updateJoystick(JOYSTICK* js, SDL_Event event)
{
    int i;
    uint8_t bit = 1;
    for (i = 0; i < NUM_JOYSTICK_BUTTONS; i++)
    {
        if (event.type == SDL_JOYBUTTONDOWN)
        {
            if (event.jbutton.button == js->buttons[i])
            {
                // Leave the other bits but ensure the ith bit is 1
                js->keysAsBits |= (bit << i);
            }
        }
        else if (event.type == SDL_JOYBUTTONUP)
        {
            if (event.jbutton.button == js->buttons[i])
            {
                // Leave the other bits but ensure the ith bit is 0
                js->keysAsBits &= (~(bit << i)); // Bitwise inversion
            }
        }
    }
}


