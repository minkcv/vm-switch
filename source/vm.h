#ifndef VM_H_
#define VM_H_

#include "constants.h"
#include "instruction.h"
#include "display.h"
#include "gpu.h"
#include "ipu.h"
#include <stdint.h>
#include <switch.h>
#include <stdlib.h>
#include <string.h>

typedef struct VM VM;
typedef struct GPU GPU;
typedef struct Display Display;

struct VM
{
    u8 regs[REGISTER_COUNT]; // General purpose registers
    // Memory is indexed by segment and then byte
    u8 memory[MEMORY_SEGMENT_COUNT][MEMORY_SEGMENT_SIZE];
    u16* pc; // Program counter / instruction pointer
    u16* code; // List of instructions
    GPU* gpu;
    IPU* ipu;
    Display* display;
    // Debugging
    int debugMode;
    int breakState;
    int step;
};

VM* createVM(u16* code, u8* rom, Display* display, int debugMode);

void run(VM* vm);

//void handleDebugKey(VM* vm, SDL_Keycode key);

void disassemble(Instruction* instr, char* assembly);

void decode(u16 instr, Instruction* decoded);

void exec(VM* vm, Instruction* instr);

#endif

