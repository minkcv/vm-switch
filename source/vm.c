#include "vm.h"
#include "opcodes.h"
#include "instruction.h"
#include <stdlib.h>
#include <stdio.h>
#include <switch.h>

// Creates a vm with the supplied code
VM* createVM(u16* code, u8* rom, Display* display, int debugMode)
{
    VM* vm = (VM*)malloc(sizeof(VM));
    vm->code = code;
    vm->pc = code;
    vm->debugMode = debugMode;
    vm->breakState = 0;
    vm->step = 0;
    vm->display = display;
    vm->gpu = createGPU();
    vm->ipu = createIPU();
    memset(vm->memory, 0, sizeof(vm->memory[0][0]) * MEMORY_SEGMENT_COUNT * MEMORY_SEGMENT_SIZE);
    memset(vm->regs, 0, sizeof(vm->regs[0]) * REGISTER_COUNT);
    if (rom != NULL) // ROM is optional
        memcpy(vm->memory[128], rom, 128 * MEMORY_SEGMENT_SIZE);
    return vm;
}

// Starts the fetch, decode, execute cycle
// of a vm from the start of its code
void run(VM* vm)
{
    u64 displayStartTime = armGetSystemTick();
    u64 displayWaitTime = 16; // 16ms = about 60 refreshes per second
    u64 cpuStartTime = armGetSystemTick();
    u64 cpuInstructionCount = 0;

    // Enforce the instructions per second limit in sync with the display refreshes
    // 500,000 instructions per second is almost the same as 8064 instructions per 16 milliseconds
    u32 instructionsPerSecondFactor = 62;

    int wait = 0;
    while (appletMainLoop())
    {
        hidScanInput();
        u32 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        u32 kUp = hidKeysUp(CONTROLLER_P1_AUTO);
        u32 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);

        if (kDown & KEY_PLUS)
        {
            break; // break in order to return to hbmenu
        }
        // TODO: handleDebugKey(vm, event.key.keysym.sym);
        updateIPU(vm->ipu, kDown, kUp, kHeld, vm->memory);

        if (vm->breakState)
            continue;

        // Executing
        if (!wait)
        {
            u16 instr = *(vm->pc);
            Instruction* decoded = decode(instr);
            exec(vm, decoded);
            vm->pc++;
            cpuInstructionCount++;
            free(decoded);

            if (cpuInstructionCount > INSTRUCTIONS_PER_SECOND / instructionsPerSecondFactor)
                wait = 1;
        }
        if ((armGetSystemTick() - cpuStartTime) > 1000 / instructionsPerSecondFactor)
        {
            // 16 milliseconds have passed
            // Reset the instruction count and timer
            if (cpuInstructionCount < INSTRUCTIONS_PER_SECOND / instructionsPerSecondFactor)
            {
                //printf("Running below desired instructions per second\n");
                //printf("Desired: %d Actual: %d\n", INSTRUCTIONS_PER_SECOND / instructionsPerSecondFactor, cpuInstructionCount);
            }
            cpuInstructionCount = 0;
            cpuStartTime = armGetSystemTick();
            wait = 0;
        }
        if (displayStartTime + displayWaitTime < armGetSystemTick())
        {
            updateGPU(vm->gpu, vm->memory);
            if (vm->gpu->active)
            {
                readSpritesFromMem(vm->gpu, vm->memory);
                drawSprites(vm->gpu, vm->memory);
                updateDisplay(vm->display, vm->gpu);
            }
            displayStartTime = armGetSystemTick();
        }
        if (vm->step)
            vm->breakState = 1;
    }
}

/*
void handleDebugKey(VM* vm, SDL_Keycode key)
{
    if (key == SDLK_F2)
    {
        // Continue
        vm->breakState = 0;
        vm->step = 0;
        printf("Resuming execution\n");
    }
    if (key == SDLK_F3)
    {
        // Break all
        vm->breakState = 1;
        printf("Entering break state\n");
    }
    if (key == SDLK_F4)
    {
        // Step
        vm->breakState = 0;
        vm->step = 1;
        printf("Stepping\n");
    }
    if (key == SDLK_F9)
    {
        // Print program counter as segment.offset, disassembly of current instruction, and registers
        int instructionCount = vm->pc - vm->code;
        int instructionSegment = instructionCount / JUMP_SEGMENT_SIZE;
        int instructionOffset = instructionCount % JUMP_SEGMENT_SIZE;
        printf("PC:  %03d.%03d 0x%02X.0x%02X\n", 
                instructionSegment, instructionOffset,
                instructionSegment, instructionOffset);
        char* assembly = malloc(sizeof(char) * 256);
        memset(assembly, 0, sizeof(char) * 256);
        Instruction* instr = decode(*(vm->pc));
        disassemble(instr, assembly);
        free(instr);
        printf("ASM: %s\n", assembly);
        free(assembly);
        int i;
        for (i = 0; i < REGISTER_COUNT; i++)
            printf("r%02d: %03d (0x%02X)\n", i, vm->regs[i], vm->regs[i]);
    }
    if (key == SDLK_F10)
    {
        // Dump memory segments 0 through 63 (inclusive)
        int segment;
        int offset;
        int rowLength = 16;
        for (segment = 0; segment < 64; segment++)
        {
            for (offset = 0; offset < 256; offset++)
            {
                if (offset % rowLength == 0)
                    printf("\n0x%02X.0x%02X: 0x", segment, offset);
                printf("%02X", vm->memory[segment][offset]);
            }
        }
        printf("\n");
    }
    if (key == SDLK_F11)
    {
        // Dump memory segments 64 through 127 (inclusive)
        int segment;
        int offset;
        int rowLength = 16;
        for (segment = 64; segment < 128; segment++)
        {
            for (offset = 0; offset < 256; offset++)
            {
                if (offset % rowLength == 0)
                    printf("\n0x%02X.0x%02X: 0x", segment, offset);
                printf("%02X", vm->memory[segment][offset]);
            }
        }
        printf("\n");
    }
    if (key == SDLK_F12)
    {
        // Disassemble next 16 instructions
        int i;
        for (i = 0; i < 16; i++)
        {
            int instructionCount = vm->pc - vm->code + i;
            if (instructionCount > JUMP_SEGMENT_SIZE * JUMP_SEGMENT_SIZE)
                continue; // End of program, can't keep reading instructions.
            int instructionSegment = instructionCount / JUMP_SEGMENT_SIZE;
            int instructionOffset = instructionCount % JUMP_SEGMENT_SIZE;
            char* assembly = malloc(sizeof(char) * 256);
            memset(assembly, 0, sizeof(char) * 256);
            Instruction* instr = decode(*(vm->pc + i));
            disassemble(instr, assembly);
            free(instr);
            // Print star for current instruction
            if (i == 0)
                printf("*");
            else
                printf(" ");

            // Print disassembly with segment.offset address
            printf(" %-30s ; %03d.%03d 0x%02X.0x%02X\n", assembly,
                    instructionSegment, instructionOffset,
                    instructionSegment, instructionOffset);
            free(assembly);
        }
    }
}
*/

void disassemble(Instruction* instr, char* assembly)
{
    // If the instructions reads the last 2 arguments as a constant
    u8 constant = ((instr->arg1 << 4) & 0x00F0) + instr->arg2;
    switch(instr->opcode)
    {
        case EXT:
            switch(instr->arg0)
            {
                case EXT_HALT:
                    sprintf(assembly, "HALT");
                    break;
                case EXT_CPY:
                    sprintf(assembly, "CPY r%02d r%02d", instr->arg1, instr->arg2);
                    break;
                case EXT_NOT:
                    sprintf(assembly, "NOT %02d %02d", instr->arg1, instr->arg2);
                    break;
                case EXT_LSL:
                    sprintf(assembly, "LSL r%02d r%02d", instr->arg1, instr->arg2);
                    break;
                case EXT_LSR:
                    sprintf(assembly, "LSR r%02d r%02d", instr->arg1, instr->arg2);
                    break;
                case EXT_JMP:
                    sprintf(assembly, "JMP r%02d r%02d", instr->arg1, instr->arg2);
                    break;
                case EXT_NOP:
                    sprintf(assembly, "NOP");
                    break;
                case EXT_BRKP:
                    sprintf(assembly, "BRKP");
                    break;
            }
            break;
        case ADD:
            sprintf(assembly, "ADD r%02d r%02d r%02d", instr->arg0, instr->arg1, instr->arg2);
            break;
        case SUB:
            sprintf(assembly, "SUB r%02d r%02d r%02d", instr->arg0, instr->arg1, instr->arg2);
            break;
        case ADDC:
            sprintf(assembly, "ADDC r%02d #%03d ; (0x%04X)", instr->arg0, constant, constant);
            break;
        case SUBC:
            sprintf(assembly, "SUBC r%02d #%03d ; (0x%04X)", instr->arg0, constant, constant);
            break;
        case CMP:
            sprintf(assembly, "CMP r%02d r%02d r%02d", instr->arg0, instr->arg1, instr->arg2);
            break;
        case JLT:
            sprintf(assembly, "JLT r%02d r%02d r%02d", instr->arg0, instr->arg1, instr->arg2);
            break;
        case JGT:
            sprintf(assembly, "JGT r%02d r%02d r%02d", instr->arg0, instr->arg1, instr->arg2);
            break;
        case JEQ:
            sprintf(assembly, "JEQ r%02d r%02d r%02d", instr->arg0, instr->arg1, instr->arg2);
            break;
        case LDR:
            sprintf(assembly, "LDR r%02d r%02d r%02d", instr->arg0, instr->arg1, instr->arg2);
            break;
        case STR:
            sprintf(assembly, "STR r%02d r%02d r%02d", instr->arg0, instr->arg1, instr->arg2);
            break;
        case LRC:
            sprintf(assembly, "LRC r%02d #%03d ; (0x%04X)", instr->arg0, constant, constant);
            break;
        case AND:
            sprintf(assembly, "AND r%02d r%02d r%02d", instr->arg0, instr->arg1, instr->arg2);
            break;
        case OR:
            sprintf(assembly, "OR r%02d r%02d r%02d", instr->arg0, instr->arg1, instr->arg2);
            break;
        case XOR:
            sprintf(assembly, "XOR r%02d r%02d r%02d", instr->arg0, instr->arg1, instr->arg2);
            break;
    }
}

// Decodes a u16 instruction into a 16 bit instruction struct
Instruction* decode(u16 instr)
{
    Instruction* decoded = malloc(sizeof(Instruction));
    u16 clean = 0x000F;
    decoded->opcode = instr >> 12 & clean;
    decoded->arg0 = instr >> 8 & clean;
    decoded->arg1 = instr >> 4 & clean;
    decoded->arg2 = instr & clean;
    return decoded;
}

// Executes an instruction
void exec(VM* vm, Instruction* instr)
{
    switch(instr->opcode)
    {
        case EXT:
            switch(instr->arg0)
            {
                case EXT_HALT:
                    printf("Exiting at halt instruction (instruction %ld)\n", (vm->pc - vm->code));
                    exit(0);
                    break;
                case EXT_CPY:
                    vm->regs[instr->arg1] = vm->regs[instr->arg2];
                    break;
                case EXT_NOT:
                    vm->regs[instr->arg1] = ~(vm->regs[instr->arg2]);
                    break;
                case EXT_LSL:
                    vm->regs[instr->arg1] = vm->regs[instr->arg1] << vm->regs[instr->arg2];
                    break;
                case EXT_LSR:
                    vm->regs[instr->arg1] = vm->regs[instr->arg1] >> vm->regs[instr->arg2];
                    break;
                case EXT_JMP:
                    vm->pc = vm->code + (vm->regs[instr->arg1] * JUMP_SEGMENT_SIZE) + vm->regs[instr->arg2] - 1;
                    break;
                case EXT_NOP:
                    break;
                case EXT_BRKP:
                    if (vm->debugMode)
                    {
                        vm->breakState = 1;
                        printf("Entering break state\n");
                    }
                    break;
            }
            break;
        case ADD:
            vm->regs[instr->arg0] = vm->regs[instr->arg1] + vm->regs[instr->arg2];
            break;
        case SUB:
            vm->regs[instr->arg0] = vm->regs[instr->arg1] - vm->regs[instr->arg2];
            break;
        case ADDC:
            vm->regs[instr->arg0] += ((instr->arg1 << 4) & 0x00F0) + instr->arg2;
            break;
        case SUBC:
            vm->regs[instr->arg0] -= ((instr->arg1 << 4) & 0x00F0) + instr->arg2;
            break;
        case CMP:
            if (vm->regs[instr->arg1] < vm->regs[instr->arg2])
                vm->regs[instr->arg0] = 0;
            else if (vm->regs[instr->arg1] > vm->regs[instr->arg2])
                vm->regs[instr->arg0] = 2;
            else
                vm->regs[instr->arg0] = 1;
            break;
        case JLT:
            if (vm->regs[instr->arg0] == 0)
                vm->pc = vm->code + (vm->regs[instr->arg1] * JUMP_SEGMENT_SIZE) + vm->regs[instr->arg2] - 1;
            break;
        case JGT:
            if (vm->regs[instr->arg0] == 2)
                vm->pc = vm->code + (vm->regs[instr->arg1] * JUMP_SEGMENT_SIZE) + vm->regs[instr->arg2] - 1;
            break;
        case JEQ:
            if (vm->regs[instr->arg0] == 1)
                vm->pc = vm->code + (vm->regs[instr->arg1] * JUMP_SEGMENT_SIZE) + vm->regs[instr->arg2] - 1;
            break;
        case LDR:
            vm->regs[instr->arg0] = vm->memory[vm->regs[instr->arg1]][vm->regs[instr->arg2]];
            break;
        case STR:
            if (vm->regs[instr->arg1] < 128) // Segment is not part of ROM
                vm->memory[vm->regs[instr->arg1]][vm->regs[instr->arg2]] = vm->regs[instr->arg0];
            else
            {
                printf("Attempted illegal write to ROM\n");
                exit(1);
            }
            break;
        case LRC:
            vm->regs[instr->arg0] = ((instr->arg1 << 4) & 0x00F0) + instr->arg2;
            break;
        case AND:
            vm->regs[instr->arg0] = vm->regs[instr->arg1] & vm->regs[instr->arg2];
            break;
        case OR:
            vm->regs[instr->arg0] = vm->regs[instr->arg1] | vm->regs[instr->arg2];
            break;
        case XOR:
            vm->regs[instr->arg0] = vm->regs[instr->arg1] ^ vm->regs[instr->arg2];
            break;
    }
}

