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
static uint8_t  instructionAugment  = 0;
static uint32_t argumentAugment     = 0;

#ifdef SELF_TEST
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

static void transferMemToReg(uint8_t toRegsel, uint32_t fromAddress, uint8_t insAug)
{
    uint32_t  buf32 = 0;
    uint16_t  buf16 = 0;
    uint8_t   buf8  = 0;
    uint32_t *reg32 = NULL;
    uint16_t *reg16 = NULL;
    uint8_t  *reg8  = NULL;

    switch (regSize[toRegsel])
    {
        case 4:
            reg32 = regMap4[toRegsel];
            switch (insAug & INS_AUG_WORD_SIZE_MASK)
            {
                case INS_AUG_WORD_SIZE_4:
                case INS_AUG_WORD_SIZE_INVALID:
                    memcpy(reg32, &(memory[fromAddress]), sizeof(uint32_t));
                    break;
                case INS_AUG_WORD_SIZE_2:
                    memcpy(&buf16, &(memory[fromAddress]), sizeof(uint16_t));
                    *reg32 = (uint32_t) buf16;
                    break;
                case INS_AUG_WORD_SIZE_1:
                    memcpy(&buf8, &(memory[fromAddress]), sizeof(uint8_t));
                    *reg32 = (uint32_t) buf8;
            }

            switch (insAug & INS_AUG_WORD_OFFSET_MASK)
            {
                case INS_AUG_WORD_OFFSET_1:
                    *reg32 = *reg32 << 8;
                    break;
                case INS_AUG_WORD_OFFSET_2:
                    *reg32 = *reg32 << 16;
                    break;
                case INS_AUG_WORD_OFFSET_3:
                    *reg32 = *reg32 << 24;
            }
            
            return;
        case 2:
            reg16 = regMap2[toRegsel];
            switch (insAug & INS_AUG_WORD_SIZE_MASK)
            {
                case INS_AUG_WORD_SIZE_4:
                case INS_AUG_WORD_SIZE_INVALID:
                    memcpy(&buf32, &(memory[fromAddress]), sizeof(uint32_t));
                    *reg16 = (uint16_t) buf32 & 0xFFFF;
                    break;
                case INS_AUG_WORD_SIZE_2:
                    memcpy(reg16, &(memory[fromAddress]), sizeof(uint16_t));
                    break;
                case INS_AUG_WORD_SIZE_1:
                    memcpy(&buf8, &(memory[fromAddress]), sizeof(uint8_t));
                    *reg16 = (uint16_t) buf8;
            }

            switch (insAug & INS_AUG_WORD_OFFSET_MASK)
            {
                case INS_AUG_WORD_OFFSET_1:
                    *reg16 = *reg16 << 8;
                    break;
                case INS_AUG_WORD_OFFSET_2:
                    *reg16 = *reg16 << 16;
                    break;
                case INS_AUG_WORD_OFFSET_3:
                    *reg16 = *reg16 << 24;
            }
            
            return;
        case 1:
            reg8 = regMap1[toRegsel];
            switch (insAug & INS_AUG_WORD_SIZE_MASK)
            {
                case INS_AUG_WORD_SIZE_4:
                case INS_AUG_WORD_SIZE_INVALID:
                    memcpy(&buf32, &(memory[fromAddress]), sizeof(uint32_t));
                    *reg8 = (uint8_t) buf32 & 0xFF;
                    break;
                case INS_AUG_WORD_SIZE_2:
                    memcpy(&buf16, &(memory[fromAddress]), sizeof(uint16_t));
                    *reg8 = (uint8_t) buf16 & 0xFF;
                    break;
                case INS_AUG_WORD_SIZE_1:
                    memcpy(reg8, &(memory[fromAddress]), sizeof(uint8_t));
            }

            switch (insAug & INS_AUG_WORD_OFFSET_MASK)
            {
                case INS_AUG_WORD_OFFSET_1:
                    *reg8 = *reg8 << 8;
                    break;
                case INS_AUG_WORD_OFFSET_2:
                    *reg8 = *reg8 << 16;
                    break;
                case INS_AUG_WORD_OFFSET_3:
                    *reg8 = *reg8 << 24;
            }
            
            return;
    }
}

static void transferRegToMem(uint32_t toAddress, uint8_t fromRegsel, uint8_t insAug)
{
    uint32_t  buf32 = 0;
    uint16_t  buf16 = 0;
    uint8_t   buf8  = 0;

    switch (regSize[fromRegsel])
    {
        case 4:
            buf32 = *(regMap4[fromRegsel]);

            switch(insAug & INS_AUG_WORD_OFFSET_MASK)
            {
                case INS_AUG_WORD_OFFSET_1:
                    buf32 = buf32 >> 8;
                    break;
                case INS_AUG_WORD_OFFSET_2:
                    buf32 = buf32 >> 16;
                    break;
                case INS_AUG_WORD_OFFSET_3:
                    buf32 = buf32 >> 24;
            }

            switch(insAug & INS_AUG_WORD_SIZE_MASK)
            {
                case INS_AUG_WORD_SIZE_4:
                case INS_AUG_WORD_SIZE_INVALID:
                    memcpy(&(memory[toAddress]), &buf32, sizeof(uint32_t));
                    break;
                case INS_AUG_WORD_SIZE_2:
                    buf16 = (uint16_t) buf32 & 0xFFFF;
                    memcpy(&(memory[toAddress]), &buf16, sizeof(uint16_t));
                    break;
                case INS_AUG_WORD_SIZE_1:
                    buf8 = (uint8_t) buf32 & 0xFF;
                    memcpy(&(memory[toAddress]), &buf8, sizeof(uint8_t));
            }
            
            return;
        case 2:
            buf16 = *(regMap2[fromRegsel]);

            switch(insAug & INS_AUG_WORD_OFFSET_MASK)
            {
                case INS_AUG_WORD_OFFSET_1:
                    buf16 = buf16 >> 8;
                    break;
                case INS_AUG_WORD_OFFSET_2:
                    buf16 = buf16 >> 16;
                    break;
                case INS_AUG_WORD_OFFSET_3:
                    buf16 = buf16 >> 24;
            }

            switch(insAug & INS_AUG_WORD_SIZE_MASK)
            {
                case INS_AUG_WORD_SIZE_4:
                case INS_AUG_WORD_SIZE_INVALID:
                    buf32 = (uint32_t) buf16;
                    memcpy(&(memory[toAddress]), &buf32, sizeof(uint32_t));
                    break;
                case INS_AUG_WORD_SIZE_2:
                    memcpy(&(memory[toAddress]), &buf16, sizeof(uint16_t));
                    break;
                case INS_AUG_WORD_SIZE_1:
                    buf8 = (uint8_t) buf16 & 0xFF;
                    memcpy(&(memory[toAddress]), &buf8, sizeof(uint8_t));
            }
            
            return;
        case 1:
            buf8 = *(regMap1[fromRegsel]);

            switch(insAug & INS_AUG_WORD_OFFSET_MASK)
            {
                case INS_AUG_WORD_OFFSET_1:
                    buf8 = buf8 >> 8;
                    break;
                case INS_AUG_WORD_OFFSET_2:
                    buf8 = buf8 >> 16;
                    break;
                case INS_AUG_WORD_OFFSET_3:
                    buf8 = buf8 >> 24;
            }

            switch(insAug & INS_AUG_WORD_SIZE_MASK)
            {
                case INS_AUG_WORD_SIZE_4:
                case INS_AUG_WORD_SIZE_INVALID:
                    buf32 = (uint32_t) buf8;
                    memcpy(&(memory[toAddress]), &buf32, sizeof(uint32_t));
                    break;
                case INS_AUG_WORD_SIZE_2:
                    buf16 = (uint16_t) buf8;
                    memcpy(&(memory[toAddress]), &buf16, sizeof(uint16_t));
                    break;
                case INS_AUG_WORD_SIZE_1:
                    memcpy(&(memory[toAddress]), &buf8, sizeof(uint8_t));
            }
            
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

static void doMath(uint8_t opcodeVari, uint8_t destRegsel, uint32_t a, uint32_t b)
{
    uint32_t result = 0;
    uint64_t buf64  = 0;
    bool     overflow = false;
    bool     carry    = false;

    switch (opcodeVari)
    {
        case OP_CODE_SUB_VARI:
            b = (~b) +1;
        case OP_CODE_ADD_VARI:
            result = a + b;
            carry  = result < a;
            overflow = carry ^
                       (0 !=(((a & 0x7FFFFFFF) + (b & 0x7FFFFFFF)) & 0x80000000));
            break;

        case OP_CODE_AND_VARI:
            result = a & b;
            break;

        case OP_CODE_OR_VARI:
            result = a | b;
            break;

        case OP_CODE_XOR_VARI:
            result = a ^ b;
            break;

        case OP_CODE_NOT_VARI:
            result = ~a;
            break;

        case OP_CODE_BSLT_VARI:
            buf64 = (uint64_t) a;
            buf64 = buf64 << (b % 32);
            result = (uint32_t) buf64 & 0xFFFFFFFF;
            carry = (buf64 & 0xFFFFFFFF00000000) != 0;
            break;

        case OP_CODE_BSRT_VARI:
            buf64 = ((uint64_t) a) << 32;
            buf64 = buf64 >> (b % 32);
            result = (uint32_t) ((buf64 & 0xFFFFFFFF00000000) >> 32);
            carry = (buf64 & 0xFFFFFFFF) != 0;
            break;

        case OP_CODE_BSLC_VARI:
            buf64 = (uint64_t) a;
            buf64 = buf64 << (b % 32);
            result = (uint32_t) (buf64 & 0xFFFFFFFF) +
                                ((buf64 & 0xFFFFFFFF00000000) >> 32);
            carry = (buf64 & 0xFFFFFFFF00000000) != 0;
            break;
        case OP_CODE_BSRC_VARI:
            buf64 = ((uint64_t) a) << 32;
            buf64 = buf64 >> (b % 32);
            result = (uint32_t) (buf64 & 0xFFFFFFFF) +
                                ((buf64 & 0xFFFFFFFF00000000) >> 32);
            carry = (buf64 & 0xFFFFFFFF) != 0;
    }

    transferVarToReg(destRegsel, result);
    flagsRegister = ((result == 0          ? FLAG_Z : 0) +
                     (result >  0x7FFFFFFF ? FLAG_N : 0) +
                     (carry                ? FLAG_C : 0) +
                     (overflow             ? FLAG_V : 0));
}

static uint32_t applyOffset(uint8_t insAug, uint32_t baseAddress)
{
    if (0 != (insAug & INS_AUG_REL_MASK))
    {
        // Relative to PC
        return baseAddress + programCounter;
    }

    switch (insAug & INS_AUG_OFFSET_REGSEL_MASK)
    {
        case INS_AUG_OFFSET_REGSEL_OA:
            return baseAddress + offsetRegisters[0];
        case INS_AUG_OFFSET_REGSEL_OB:
            return baseAddress + offsetRegisters[1];
        case INS_AUG_OFFSET_REGSEL_OC:
            return baseAddress + offsetRegisters[2];
    }

    return baseAddress;
}

static uint32_t getValFromRegsel(uint8_t regsel)
{
    switch (regSize[regsel])
    {
        case 4:
            return *regMap4[regsel];
        case 2:
            return (uint32_t) *regMap2[regsel];
        case 1:
            return (uint32_t) *regMap1[regsel];
    }

    return 0;
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
                memcpy(&instructionAugment, &(memory[programCounter + 4]), sizeof(instructionAugment));

                if (0 != (instructionAugment & INS_AUG_ABS_MASK))
                {
                    // Get absolute address
                    transferVarToReg(REGSEL_1_GET(instructionRegister),
                                     applyOffset(instructionAugment,
                                                 getValFromRegsel(REGSEL_2_GET(instructionRegister))));
                }
                else
                {
                    transferMemToReg(REGSEL_1_GET(instructionRegister),
                                     applyOffset(instructionAugment,
                                                 getValFromRegsel(REGSEL_2_GET(instructionRegister))),
                                     instructionAugment);
                }
                programCounter+=5;
                break;

            case OP_CODE_LOAD_F4:
                memcpy(&instructionAugment, &(memory[programCounter + 4]), sizeof(instructionAugment));
                memcpy(&argumentAugment,    &(memory[programCounter + 5]), sizeof(argumentAugment));

                if (0 != (instructionAugment & INS_AUG_ABS_MASK))
                {
                    // Get absolute address
                    transferVarToReg(REGSEL_1_GET(instructionRegister),
                                     applyOffset(instructionAugment, argumentAugment));
                }
                else
                {
                    transferMemToReg(REGSEL_1_GET(instructionRegister),
                                     applyOffset(instructionAugment, argumentAugment),
                                     instructionAugment);
                }
                programCounter+=9;
                break;

            case OP_CODE_STOR_F2:
                memcpy(&instructionAugment, &(memory[programCounter + 4]), sizeof(instructionAugment));

                transferRegToMem(applyOffset(instructionAugment,
                                             getValFromRegsel(REGSEL_2_GET(instructionRegister))),
                                 REGSEL_1_GET(instructionRegister),
                                 instructionAugment);
                programCounter+=5;
                break;

            case OP_CODE_STOR_F4:
                memcpy(&instructionAugment, &(memory[programCounter + 4]), sizeof(instructionAugment));
                memcpy(&argumentAugment,    &(memory[programCounter + 5]), sizeof(argumentAugment));

                transferRegToMem(applyOffset(instructionAugment, argumentAugment),
                                 REGSEL_1_GET(instructionRegister),
                                 instructionAugment);
                programCounter+=9;
                break;

            case OP_CODE_MOVE_F2:
                transferRegToReg(REGSEL_1_GET(instructionRegister), REGSEL_2_GET(instructionRegister));
                programCounter+=4;
                break;

            case OP_CODE_MOVE_F5:
                transferVarToReg(REGSEL_1_GET(instructionRegister), ARG_F5_GET(instructionRegister));
                programCounter+=4;
                break;

            case OP_CODE_MATH_BASE_F1:
                doMath(OP_CODE_GET_VARI(instructionRegister),
                       REGSEL_1_GET(instructionRegister),
                       getValFromRegsel(REGSEL_2_GET(instructionRegister)),
                       getValFromRegsel(REGSEL_3_GET(instructionRegister)));
                programCounter+=4;
                break;

            case OP_CODE_MATH_BASE_F3:
                doMath(OP_CODE_GET_VARI(instructionRegister),
                       REGSEL_1_GET(instructionRegister),
                       getValFromRegsel(REGSEL_2_GET(instructionRegister)),
                       ARG_F3_GET(instructionRegister));
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