#include "constants.h"
#include "instruction.h"
#include "opcodes.h"
#include "vm.h"
#include "display.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
// Include the main libnx system header, for Switch development
#include <switch.h>

uint16_t* readBinary(const char* filename, int print);
uint8_t* readRom(const char* filename, int print);

typedef struct GameData GameData;

struct GameData
{
    char* name;
    char* codeName;
    char* romName;
};

void printGameList(int cursorPos, int numGames, GameData gameData[256])
{
    int i;
    int startRow = 3;
    for (i = 0; i < numGames; i++)
    {
        if (i == cursorPos)
            printf("\x1b[%d;0H> %s - %s  %s", i + startRow, gameData[i].name, gameData[i].codeName, gameData[i].romName);
        else
            printf("\x1b[%d;0H  %s - %s  %s", i + startRow, gameData[i].name, gameData[i].codeName, gameData[i].romName);
    }
}

int gameMenu(uint16_t** code, uint8_t** rom)
{
    consoleInit(NULL);
    GameData gameData[256];
    FILE* gameList = fopen("vaporspec/gamelist.txt", "r");
    if (gameList == NULL)
    {
        printf("\x1b[0;0HMissing vaporspec/gamelist.txt");
        consoleUpdate(NULL);
        sleep(2);
        consoleExit(NULL);
        return 1; // QUIT
    }
    fseek(gameList, 0, SEEK_END);
    size_t size = ftell(gameList);
    fseek(gameList, 0, SEEK_SET);
    char* contents = malloc(size + 1);
    fread(contents, 1, size + 1, gameList);
    int numGames = 0;
    char* line = strtok(contents, "\n");
    while (line != NULL)
    {
        char* name = strsep(&line, ";");
        char* codeName = strsep(&line, ";");
        char* romName = strsep(&line, ";");
        gameData[numGames].name = name;
        gameData[numGames].codeName = codeName;
        gameData[numGames].romName = romName;
        numGames++;
        line = strtok(NULL, "\n");
    }

    int cursorPos = 0;
    while(1)
    {
        printf("\x1b[0;0HSelect game with up/down. Start with A. Quit with +.");
        printGameList(cursorPos, numGames, gameData);
        hidScanInput();
        uint64_t kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        if (kDown & KEY_PLUS)
        {
            fclose(gameList);
            free(contents);
            consoleExit(NULL);
            return 1; // QUIT
        }
        
        if (kDown & KEY_DUP || kDown & KEY_LSTICK_UP)
        {
            if (cursorPos > 0 )
                cursorPos--;
            else
                cursorPos = numGames - 1;
        }
        if (kDown & KEY_DDOWN || kDown & KEY_LSTICK_DOWN)
        {
            if (cursorPos == numGames - 1)
                cursorPos = 0;
            else
                cursorPos++;
        }
        if (kDown & KEY_A)
            break;
        consoleUpdate(NULL);
    }
    GameData selected = gameData[cursorPos];
    (*code) = readBinary(selected.codeName, 0);
    if (!strcmp(selected.romName, "\0") && strlen(selected.romName) > 0)
        (*rom) = readRom(selected.romName, 0);

    fclose(gameList);
    free(contents);
    consoleExit(NULL);
    return 0;
}

int main (int argc, char** argv)
{
    while (appletMainLoop())
    {
        uint16_t* code = NULL;
        uint8_t* rom = NULL;

        int quit = gameMenu(&code, &rom);
        if (quit)
            break;
        if (code == NULL)
            continue;

        int scale = 1;
        int debugMode = 0;
        Display* display = createDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, scale, SCREEN_WIDTH / 4);
        VM* vm = createVM(code, rom, display, debugMode);
        run(vm);
        quitDisplay(display);
        if (rom != NULL)
            free(rom);
        rom = NULL;
        free(code);
    }
    return 0;
}

uint16_t* readBinary(const char* filename, int print)
{
    char* folder = "vaporspec/";
    char* path = malloc(sizeof(char) * (strlen(folder) + strlen(filename)) + 1);
    strcat(path, folder);
    strcat(path, filename);
    FILE* bin = fopen(path, "rb");
    if (bin == NULL)
    {
        printf("\x1b[0;0HError reading file %s\n", filename);
        consoleUpdate(NULL);
        free(path);
        return NULL;
    }
    size_t numInstructions = 0;
    // binaries are length prefixed
    fread(&numInstructions, sizeof(uint16_t), 1, bin);
    printf("\x1b[0;0HBinary is %zd instructions\n", numInstructions);
    consoleUpdate(NULL);
    uint16_t* code = malloc(sizeof(uint16_t) * numInstructions);
    fread(code, sizeof(uint16_t), numInstructions, bin);
    if (print)
    {
        int i;
        for (i = 0; i < numInstructions; i++)
        {
            printf("\x1b[%d;0H%4X\n", i, code[i]);
            consoleUpdate(NULL);
        }
    }
    fclose(bin);
    free(path);
    return code;
}

uint8_t* readRom(const char* filename, int print)
{
    char* folder = "vaporspec/";
    char* path = malloc(sizeof(char) * (strlen(folder) + strlen(filename)) + 1);
    strcat(path, folder);
    strcat(path, filename);
    FILE* romfile = fopen(path, "rb");
    if (romfile == NULL)
    {
        printf("\x1b[0;0HError reading file %s\n", filename);
        consoleUpdate(NULL);
        free(path);
        return NULL;
    }
    uint8_t* rom = malloc(sizeof(uint8_t) * 128 * MEMORY_SEGMENT_SIZE);
    memset(rom, 0, sizeof(uint8_t) * 128 * MEMORY_SEGMENT_SIZE);
    size_t bytesRead = fread(rom, sizeof(uint8_t), 128 * MEMORY_SEGMENT_SIZE, romfile);
    printf("\x1b[0;0HRead %zd bytes from the rom\n", bytesRead);
    consoleUpdate(NULL);
    if (print)
    {
        int i;
        for (i = 0; i < bytesRead; i++)
        {
            printf("\x1b[%d;0H%2X\n", i, rom[i]);
            consoleUpdate(NULL);
        }
    }
    fclose(romfile);
    free(path);
    return rom;
}