// Include the most common headers from the C standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"
#include "constants.h"
#include "instruction.h"
#include "opcodes.h"
#include "display.h"

// Include the main libnx system header, for Switch development
#include <switch.h>

// Main program entrypoint
int main(int argc, char* argv[])
{
    // This example uses a text console, as a simple way to output text to the screen.
    // If you want to write a software-rendered graphics application,
    //   take a look at the graphics/simplegfx example, which uses the libnx gfx API instead.
    // If on the other hand you want to write an OpenGL based application,
    //   take a look at the graphics/opengl set of examples, which uses EGL instead.
    //consoleInit(NULL);

    // Other initialization goes here. As a demonstration, we print hello world.
    //printf("Hello World!\n");

    // Main loop
    /*
    while (appletMainLoop())
    {
        // Scan all the inputs. This should be done once for each frame
        hidScanInput();

        // hidKeysDown returns information about which buttons have been
        // just pressed in this frame compared to the previous one
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_PLUS)
            break; // break in order to return to hbmenu

        // Your code goes here

        // Update the console, sending a new frame to the display
        consoleUpdate(NULL);
    }
    */

    // Deinitialize and clean up resources used by the console (important!)
    //consoleExit(NULL);
    int scale = 1;
    int debugMode = 0;

    Display* display = createDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, scale, SCREEN_WIDTH / 4);

    // PONG
    u16 code[] = {45376,45696,41488,45569,45832,41746,45570,45888,41746,45571,45832,41746,45572,45888,41746,45573,45952,41746,45575,46079,41746,45584,45952,41746,45585,46064,41746,45586,45888,41746,45587,45832,41746,45588,45888,41746,45589,45952,41746,45591,46079,41746,45600,45952,41746,45601,45948,41746,45602,45916,41746,45603,45832,41746,45604,45832,41746,45605,45952,41746,45607,46079,41746,45439,45585,45825,41746,45313,45632,41473,45314,41473,45315,45692,41473,45316,45660,41473,45439,37392,45825,50211,21571,48640,49056,34031,45439,37392,45826,50211,21571,48640,49065,34031,45439,45569,37650,50226,21570,48640,49075,34031,45439,45569,37650,45570,50226,21570,48640,49084,34031,45317,37377,48640,49094,25327,45317,37377,48640,49133,29423,45318,37377,48641,48915,25327,45318,37377,48641,48934,29423,45313,37377,45314,37377,45888,46098,41524,45313,37377,46082,41524,45315,37377,46113,41524,45316,37377,46114,41524,48641,48953,1519,45439,45585,37650,41728,48640,48974,1519,48640,48982,45314,37377,21536,34031,16900,41473,1519,48640,48990,45314,37377,45952,21539,34031,12804,41473,1519,48640,48998,45313,37377,21536,34031,16900,41473,1519,48640,49007,45313,37377,45952,21539,34031,12804,41473,1519,45315,37377,45828,21283,48640,48963,25583,45840,21283,48640,49126,29679,45313,37633,45316,37889,13320,21827,45376,4371,17416,22081,48640,49126,26095,30447,45317,45570,41473,48640,49012,1519,45315,37377,16900,41473,48640,49012,1519,45315,37377,46068,21283,48640,48963,29679,46056,21283,48641,48908,25583,45314,37633,45316,37889,13320,21827,45376,4371,17416,22081,48641,48908,26095,30447,45317,40961,48640,49017,1519,45315,37377,12804,41473,48640,49017,1519,45316,37377,20738,48641,48927,25071,45318,45570,41473,48640,49022,1519,45316,37377,16900,41473,48640,49022,1519,45316,37377,46008,20771,48641,48946,25071,45318,40961,48640,49022,1519,45316,37377,12804,41473,48640,49027,1519,1536,1536,1536,1536,1536,1536,1536,1536,45439,45585,37650,37888,45314,49939,50196,58164,20785,48640,49049,33263,48641,48953,1519};
    u8* rom = NULL;
    /*
    if (romName != NULL) // ROM is optional
        rom = readRom(romName, 0);
    */
    VM* vm = createVM(code, rom, display, debugMode);
    run(vm);
    quitDisplay(display);
    if (rom != NULL)
        free(rom);
    rom = NULL;
    return 0;
}
