#include "dgc32.h"

static uint8_t *memory      = NULL;
static char    *romLocation = NULL;
#define DEFAULT_ROM_LOCATION "./rom.bin"



// Exposed registers

static uint32_t generalRegisters[8] = {0};
static uint32_t offsetRegisters[3]  = {0};
static uint32_t stackBase           = 0;
static uint16_t stackSize           = 0;
static uint16_t stackPointer        = 0;
static uint32_t interruptTable      = 0;
static uint8_t  flagsRegister       = 0;


// Internal registers
static uint32_t programCounter      = 0;
static uint32_t instructionRegister = 0;

#ifdef SELF_TEST
static uint8_t  instructionAugment  = 0;
static uint8_t  interruptHead       = 0;
static uint8_t  interruptTail       = 0;
#endif //SELF_TEST

uint8_t regSize[] =
{
    4, // G0
    4, // G1
    4, // G2
    4, // G3
    4, // G4
    4, // G5
    4, // G6
    4, // G7
    4, // OA
    4, // OB
    4, // OC
    4, // SB
    2, // SS
    2, // SP
    4, // IL
    1  // FL
};

uint32_t *regMap4[] =
{
    &(generalRegisters[0]),
    &(generalRegisters[1]),
    &(generalRegisters[2]),
    &(generalRegisters[3]),
    &(generalRegisters[4]),
    &(generalRegisters[5]),
    &(generalRegisters[6]),
    &(generalRegisters[7]),
    &(offsetRegisters[0]),
    &(offsetRegisters[1]),
    &(offsetRegisters[2]),
    &stackBase,
    NULL, //SS
    NULL, //SP
    &interruptTable,
    NULL // FL
};

uint16_t *regMap2[] =
{
    NULL, // G0
    NULL, // G1
    NULL, // G2
    NULL, // G3
    NULL, // G4
    NULL, // G5
    NULL, // G6
    NULL, // G7
    NULL, // OA
    NULL, // OB
    NULL, // OC
    NULL, // SB
    &stackSize,
    &stackPointer,
    NULL, // IL
    NULL  // FL
};

uint8_t *regMap1[] =
{
    NULL, // G0
    NULL, // G1
    NULL, // G2
    NULL, // G3
    NULL, // G4
    NULL, // G5
    NULL, // G6
    NULL, // G7
    NULL, // OA
    NULL, // OB
    NULL, // OC
    NULL, // SB
    NULL, // SS
    NULL, // SP
    NULL, // IL
    &flagsRegister
};

/*******************************************************************************
* Parse command line arguments.
*******************************************************************************/
static bool parseArgs(int argc, char* argv[])
{
    uint8_t index = 1;

    #ifdef SELF_TEST

    if (index < argc)
    {
        if (false == st_setTestFile(argv[index]))
        {
            return false;
        }
    }
    else
    {
        printf("Self test error: test file required (1st argument)\n");
        return false;
    }

    index++;

    #endif // SELF_TEST

    if (index < argc)
    {
        // Rom
        romLocation = argv[index];
    }

    return true;
}

/*******************************************************************************
* Initialize memory, read ROM.
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
        memory = NULL;
        return false;
    }

    fread(memory, sizeof(uint8_t), ROM_SIZE, romFile);

    if (0 != ferror(romFile))
    {
        printf("Error reading from ROM file\n");
        free(memory);
        memory = NULL;
        return false;
    }

    fclose(romFile);

    return true;
}

static inline void transferRegToReg(uint8_t toRegsel, uint8_t fromRegsel)
{
    uint32_t bus = 0;

    switch (regSize[fromRegsel])
    {
        case 4:
            bus = *(regMap4[fromRegsel]);
            break;
        case 2:
            bus = *(regMap2[fromRegsel]);
            break;
        case 1:
            bus = *(regMap1[fromRegsel]);
    }

    switch (regSize[toRegsel])
    {
        case 4:
            *(regMap4[toRegsel]) = bus;
            break;
        case 2:
            *(regMap2[toRegsel]) = (uint16_t) bus & 0xFFFF;
            break;
        case 1:
            *(regMap1[toRegsel]) = (uint8_t) bus & 0xFF;

    }
}

static inline void transferMemToReg(uint8_t toRegsel, uint32_t fromAddress)
{
    switch (regSize[toRegsel])
    {
        case 4:
            memcpy(regMap4[toRegsel], &(memory[fromAddress]), sizeof(uint32_t));
            break;
        case 2:
            memcpy(regMap2[toRegsel], &(memory[fromAddress]), sizeof(uint16_t));
            break;
        case 1:
            memcpy(regMap1[toRegsel], &(memory[fromAddress]), sizeof(uint8_t));
    }
}

static inline void transferRegToMem(uint32_t toAddress, uint8_t fromRegsel)
{
    switch (regSize[fromRegsel])
    {
        case 4:
            memcpy(&(memory[toAddress]), regMap4[fromRegsel], sizeof(uint32_t));
            return;
        case 2:
            memcpy(&(memory[toAddress]), regMap2[fromRegsel], sizeof(uint16_t));
            return;
        case 1:
            memcpy(&(memory[toAddress]), regMap1[fromRegsel], sizeof(uint8_t));
            return;
    }
}

static inline void transferVarToReg(uint8_t toRegsel, uint32_t fromVar)
{
    switch (regSize[toRegsel])
    {
        case 4:
            *regMap4[toRegsel] = fromVar;
            break;
        case 2:
            *regMap2[toRegsel] = (uint16_t) fromVar & 0xFFFF;
            break;
        case 1:
            *regMap1[toRegsel] = (uint8_t) fromVar & 0xFF;
    }
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

            case OP_CODE_LOAD_F2:
                // TODO
                programCounter+=5;
                break;

            case OP_CODE_LOAD_F4:
                // TODO
                programCounter+=9;
                break;

            case OP_CODE_STOR_F2:
                // TODO
                programCounter+=5;
                break;

            case OP_CODE_STOR_F4:
                // TODO
                programCounter+=9;
                break;

            case OP_CODE_MOVE_F2:
                // TODO
                transferRegToReg(REGSEL_1_GET(instructionRegister), REGSEL_2_GET(instructionRegister));
                programCounter+=4;
                break;

            case OP_CODE_MOVE_F5:
                // TODO
                transferVarToReg(REGSEL_1_GET(instructionRegister), ARG_F5_GET(instructionRegister));
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

            case OP_CODE_COMP_F2:
                // TODO
                programCounter+=4;
                break;

            case OP_CODE_COMP_F4:
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

            case OP_CODE_TERM_BASE:
                return;
        }

        #ifdef SELF_TEST

        if (false == st_checkFrame(generalRegisters,
                                   offsetRegisters,
                                   stackBase,
                                   stackSize,
                                   interruptTable,
                                   programCounter,
                                   instructionRegister,
                                   instructionAugment,
                                   interruptHead,
                                   interruptTail,
                                   stackPointer,
                                   flagsRegister,
                                   memory))
        {
            return;
        }
        #endif // SELF_TEST
    }
}

static void teardown()
{
    #ifdef SELF_TEST
    st_exit();
    #endif // SELF_TEST

    if (NULL != memory)
    {
        free(memory);
        memory = NULL;
    }
    
}

int main(int argc, char* argv[])
{
    printf("DGC-32\n");

    #ifdef SELF_TEST
    printf("Self test mode\n");
    #endif // SELF_TEST

    if (false == parseArgs(argc, argv))
    {
        teardown();
        return -1;
    }

    if (false == init())
    {
        teardown();
        return -1;
    }

    run();

    teardown();

    return 0;
}