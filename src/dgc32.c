#include "dgc32.h"

static uint8_t *memory      = NULL;
static char    *romLocation = NULL;
#define DEFAULT_ROM_LOCATION "./rom.bin"



// Exposed registers
//static uint32_t generalRegisters[8] = {0};
//static uint32_t offsetRegisters[3]  = {0};
//static uint32_t stackBase           = 0;
//static uint16_t stackSize           = 0;
//static uint32_t interruptTable      = 0;

// Internal registers
static uint32_t programCounter      = 0;
static uint32_t instructionRegister = 0;
//static uint8_t  instructionAugment  = 0;
//static uint8_t  interruptHead       = 0;
//static uint8_t  interruptTail       = 0;
//static uint16_t stackPointer        = 0;
//static uint8_t  flagsRegister       = 0;

/*******************************************************************************
* Parse command line arguments
*******************************************************************************/
static bool parseArgs(int argc, char* argv[])
{
    if (argc >= 2)
    {
        // Rom
        romLocation = calloc(strlen(argv[1]) + 1, sizeof(char));
        snprintf(romLocation, strlen(argv[1]) + 1, "%s", argv[1]);
    }
    else
    {
        romLocation = calloc(strlen(DEFAULT_ROM_LOCATION) + 1, sizeof(char));
        snprintf(romLocation, strlen(DEFAULT_ROM_LOCATION) + 1, "%s", DEFAULT_ROM_LOCATION);
    }

    return true;
}

/*******************************************************************************
* Initialize memory, read ROM, .
*******************************************************************************/
static bool init()
{
    FILE *romFile = NULL;

    // Reading ROM.
    memory = malloc(MEMORY_SIZE * sizeof(uint8_t));

    if (NULL == memory)
    {
        printf("Internal error: failed to allocate space for memory\n");
        return false;
    }

    romFile = fopen(romLocation == NULL ? DEFAULT_ROM_LOCATION : romLocation, "rb");

    if (NULL == romFile)
    {
        printf("Could not open ROM file at \"%s\"\n", romLocation == NULL ? DEFAULT_ROM_LOCATION : romLocation);
        free(memory);
        return false;
    }

    fread(memory, sizeof(uint8_t), ROM_SIZE, romFile);

    if (0 != ferror(romFile))
    {
        printf("Error reading from ROM file\n");
        free(memory);
        return false;
    }

    fclose(romFile);

    return true;
}

static void run()
{
    while (true)
    {
        memcpy(&instructionRegister, &(memory[programCounter]), sizeof(instructionRegister));

        switch (OP_CODE_GET_BASE(instructionRegister))
        {
            case OP_CODE_NOOP:
                programCounter+=4;
                break;

            case OP_CODE_INTR_BASE:
                // TODO
                programCounter+=4;
                break;

            case OP_CODE_LOAD_F5:
                // TODO
                programCounter+=5;
                break;

            case OP_CODE_LOAD_F4:
                // TODO
                programCounter+=9;
                break;

            case OP_CODE_STOR_F5:
                // TODO
                programCounter+=5;
                break;

            case OP_CODE_STOR_F4:
                // TODO
                programCounter+=9;
                break;

            case OP_CODE_MOVE_F2:
                // TODO
                programCounter+=4;
                break;

            case OP_CODE_MOVE_F3:
                // TODO
                programCounter+=4;
                break;

            case OP_CODE_MATH_BASE_F1:
                // TODO
                programCounter+=4;
                break;

            case OP_CODE_MATH_BASE_F3:
                // TODO
                programCounter+=4;
                break;

            case OP_CODE_COMP_F4:
                // TODO
                programCounter+=4;
                break;

            case OP_CODE_COMP_F6:
                // TODO
                programCounter+=8;
                break;

            case OP_CODE_BRNC_BASE_F4:
                // TODO
                programCounter+=5;
                break;

            case OP_CODE_BRNC_BASE_F6:
                // TODO
                programCounter+=9;
                break;

            case OP_CODE_STCK_BASE:
                // TODO
                programCounter+=4;
                break;

            case OP_CODE_TERM:
                return;
        }
    }
}

int main(int argc, char* argv[])
{
    printf("DGC-32\n");

    if (false == parseArgs(argc, argv))
    {
        return -1;
    }

    if (false == init())
    {
        return -1;
    }

    run();

    return 0;
}