#include "dgc32.h"

static uint8_t *memory      = NULL;
static char    *romLocation = NULL;
#define DEFAULT_ROM_LOCATION "./rom.bin"

// Exposed registers
static uint32_t generalRegisters[8]    = {0};
static uint32_t offsetRegisters[3]     = {0};
static uint32_t stackBase              = 0;
static uint16_t stackSize              = 0;
static uint16_t stackPointer           = 0;
static uint32_t interruptTable         = 0;
static uint8_t  flagsRegister          = 0;


// Internal registers
static uint32_t programCounter         = 0;
static uint32_t instructionRegister    = 0;
static uint8_t  instructionAugment     = 0;
static uint32_t argumentAugment        = 0;
static uint32_t interruptReturnAddress = 0;
static uint16_t currentInterrupt       = 0;
static uint8_t  interruptHead          = 0;
static uint8_t  interruptTail          = 0;
static uint8_t  statusRegister         = 0;

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

uint8_t criticalInterruptIDs[] =
{
    0x05, // Critical stack event
    0x09  // Memory violation
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
* Initialize memory, read ROM, start up motherboard.
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

    mb_initDevices();

    return true;
}

static inline void enqueueCriticalInterrupt(uint16_t interrupt)
{
    interruptTail = (interruptTail - 2) % INTERRUPT_QUEUE_SIZE;
    memcpy(&(memory[INTERRUPT_QUEUE_BASE + interruptTail]), &interrupt, sizeof(interrupt));
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

    // Check for memory violations
    switch (insAug & INS_AUG_WORD_SIZE_MASK)
    {
        case INS_AUG_WORD_SIZE_4:
        case INS_AUG_WORD_SIZE_INVALID:
            if (false == MEMBOUND_CAN_READ(fromAddress, 4))
            {
                enqueueCriticalInterrupt(INTERRUPT_CODE_MEMORY_VIOLATION);
                return;
            }
            break;
        case INS_AUG_WORD_SIZE_2:
            if (false == MEMBOUND_CAN_READ(fromAddress, 2))
            {
                enqueueCriticalInterrupt(INTERRUPT_CODE_MEMORY_VIOLATION);
                return;
            }
            break;
        case INS_AUG_WORD_SIZE_1:
            if (false == MEMBOUND_CAN_READ(fromAddress, 1))
            {
                enqueueCriticalInterrupt(INTERRUPT_CODE_MEMORY_VIOLATION);
                return;
            }
    }

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

    // Check for memory violations
    switch (insAug & INS_AUG_WORD_SIZE_MASK)
    {
        case INS_AUG_WORD_SIZE_4:
        case INS_AUG_WORD_SIZE_INVALID:
            if (false == MEMBOUND_CAN_WRITE(toAddress, 4))
            {
                enqueueCriticalInterrupt(INTERRUPT_CODE_MEMORY_VIOLATION);
                return;
            }
            break;
        case INS_AUG_WORD_SIZE_2:
            if (false == MEMBOUND_CAN_WRITE(toAddress, 2))
            {
                enqueueCriticalInterrupt(INTERRUPT_CODE_MEMORY_VIOLATION);
                return;
            }
            break;
        case INS_AUG_WORD_SIZE_1:
            if (false == MEMBOUND_CAN_WRITE(toAddress, 1))
            {
                enqueueCriticalInterrupt(INTERRUPT_CODE_MEMORY_VIOLATION);
                return;
            }
    }

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

static inline uint32_t getValFromRegsel(uint8_t regsel)
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

static inline void enqueueInterrupt(uint16_t interrupt)
{
    memcpy(&(memory[INTERRUPT_QUEUE_BASE + interruptHead]), &interrupt, sizeof(interrupt));

    interruptHead = (interruptHead + 2) % INTERRUPT_QUEUE_SIZE;
}

static inline void peekInterrupt(uint16_t *interrupt)
{
    memcpy(interrupt, &(memory[INTERRUPT_QUEUE_BASE + interruptTail]), sizeof(*interrupt));
}

static inline void dequeueInterrupt(uint16_t *interrupt)
{
    peekInterrupt(interrupt);

    interruptTail = (interruptTail + 2) % INTERRUPT_QUEUE_SIZE;
}

static inline bool isInterruptQueueEmpty()
{
    return interruptHead == interruptTail;
}

static inline uint32_t getInterruptHandleLocation(uint8_t iid)
{
    uint32_t out = 0;

    memcpy(&out, &(memory[interruptTable + (iid * sizeof(out))]), sizeof(out));

    return out;
}

static bool doInterruptUtils(uint8_t intVari, uint8_t regsel, uint32_t arg)
{
    switch (intVari)
    {
        case OP_CODE_INTR_SUS_VARI:
            statusRegister = statusRegister | STAT_REG_INT_SUS_MASK;
            break;

        case OP_CODE_INTR_RES_VARI:
            statusRegister = statusRegister & (~(STAT_REG_INT_IN_PROG_MASK+
                                                 STAT_REG_INT_SUS_MASK));
            break;

        case OP_CODE_INTR_TRIG_F4_VARI:
            enqueueInterrupt(getValFromRegsel(regsel) & INTERRUPT_FULL_MASK);
            break;

        case OP_CODE_INTR_TRIG_F7_VARI:
            enqueueInterrupt(arg & INTERRUPT_FULL_MASK);
            break;

        case OP_CODE_INTR_FIN_VARI:
            programCounter = interruptReturnAddress;
            statusRegister = statusRegister & (~STAT_REG_INT_IN_PROG_MASK);
            return false;

        case OP_CODE_INTR_GPR:
            transferVarToReg(regsel, (currentInterrupt & INTERRUPT_ARG_MASK) >> INTERRUPT_ARG_OFFSET);
    }

    return true;
}

static void detectInterrupt()
{
    bool isCriticalInterrupt = false;

    if (isInterruptQueueEmpty())
    {
        return;
    }

    if (0 != (statusRegister & STAT_REG_INT_IN_PROG_MASK))
    {
        return;
    }

    if (0 == interruptTable)
    {
        return;
    }

    peekInterrupt(&currentInterrupt);

    for (uint8_t i = 0; i < (sizeof(criticalInterruptIDs) / sizeof(criticalInterruptIDs[0])); i++)
    {
        if (criticalInterruptIDs[i] == (currentInterrupt & INTERRUPT_ID_MASK))
        {
            isCriticalInterrupt = true;
            break;
        }
    }

    if (false == isCriticalInterrupt &&
        ((statusRegister & STAT_REG_INT_SUS_MASK) != 0))
    {
        return;
    }

    dequeueInterrupt(&currentInterrupt);

    interruptReturnAddress = programCounter;
    
    programCounter = getInterruptHandleLocation(currentInterrupt & INTERRUPT_ID_MASK);

    statusRegister = statusRegister | STAT_REG_INT_IN_PROG_MASK;
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

static void doCompare(uint32_t a, uint32_t b)
{
    uint32_t result   = 0;
    bool     carry    = false;
    bool     overflow = false;

    b = (~b) +1;
    result = a + b;
    carry  = result < a;
    overflow = carry ^
                (0 !=(((a & 0x7FFFFFFF) + (b & 0x7FFFFFFF)) & 0x80000000));

    flagsRegister = ((result == 0          ? FLAG_Z : 0) +
                     (result >  0x7FFFFFFF ? FLAG_N : 0) +
                     (carry                ? FLAG_C : 0) +
                     (overflow             ? FLAG_V : 0));
}

static inline void push32(uint32_t data)
{
    memcpy(&(memory[stackBase + stackPointer]), &data, sizeof(data));
    stackPointer += sizeof(data);
}

static inline void push16(uint16_t data)
{
    memcpy(&(memory[stackBase + stackPointer]), &data, sizeof(data));
    stackPointer += sizeof(data);
}

static inline void push8(uint8_t data)
{
    memcpy(&(memory[stackBase + stackPointer]), &data, sizeof(data));
    stackPointer += sizeof(data);
}

static inline uint32_t peek32()
{
    uint32_t out = 0;
    memcpy(&out, &(memory[stackBase + stackPointer - sizeof(out)]), sizeof(out));
    return out;
}

static inline uint16_t peek16()
{
    uint16_t out = 0;
    memcpy(&out, &(memory[stackBase + stackPointer - sizeof(out)]), sizeof(out));
    return out;
}

static inline uint8_t peek8()
{
    uint8_t out = 0;
    memcpy(&out, &(memory[stackBase + stackPointer - sizeof(out)]), sizeof(out));
    return out;
}

static inline uint32_t pop32()
{
    uint32_t out = 0;
    memcpy(&out, &(memory[stackBase + stackPointer - sizeof(out)]), sizeof(out));
    stackPointer -= sizeof(out);
    return out;
}

static inline uint16_t pop16()
{
    uint16_t out = 0;
    memcpy(&out, &(memory[stackBase + stackPointer - sizeof(out)]), sizeof(out));
    stackPointer -= sizeof(out);
    return out;
}

static inline uint8_t pop8()
{
    uint8_t out = 0;
    memcpy(&out, &(memory[stackBase + stackPointer - sizeof(out)]), sizeof(out));
    stackPointer -= sizeof(out);
    return out;
}

static bool doStackUtils(uint8_t stackVari, uint8_t regsel)
{
    bool alreadyOverflowed = false;
    bool stackUnderflow    = false;

    alreadyOverflowed = stackPointer + STACK_OVERFLOW_THRESHOLD >= stackSize;

    switch (stackVari)
    {
        case OP_CODE_STCK_PUSH_VARI:
            // Check for memory violation
            if (false == MEMBOUND_CAN_WRITE(stackBase + stackPointer, regSize[regsel]))
            {
                enqueueCriticalInterrupt(INTERRUPT_CODE_MEMORY_VIOLATION);
                return true;
            }
            // Push
            switch(regSize[regsel])
            {
                case 4:
                    push32(getValFromRegsel(regsel));
                    break;
                case 2:
                    push16(getValFromRegsel(regsel));
                    break;
                case 1:
                    push8(getValFromRegsel(regsel));
            }
            break;

        case OP_CODE_STCK_POP_VARI:
            // Check for underflow
            if (stackPointer < regSize[regsel])
            {
                stackUnderflow = true;
                break;
            }
            // Check for memory violation
            if (false == MEMBOUND_CAN_READ(stackBase + stackPointer - regSize[regsel], regSize[regsel]))
            {
                enqueueCriticalInterrupt(INTERRUPT_CODE_MEMORY_VIOLATION);
                return true;
            }
            // Pop
            switch(regSize[regsel])
            {
                case 4:
                    transferVarToReg(regsel, pop32());
                    break;
                case 2:
                    transferVarToReg(regsel, pop16());
                    break;
                case 1:
                    transferVarToReg(regsel, pop8());
            }
            return true;

        case OP_CODE_STCK_PUSHALL_VARI:
            // Check for memory violation
            if (false == MEMBOUND_CAN_WRITE(stackBase + stackPointer, STACK_PUSHALL_SIZE))
            {
                enqueueCriticalInterrupt(INTERRUPT_CODE_MEMORY_VIOLATION);
                return true;
            }
            // Pushall
            for (uint8_t i = 0; i < 8; i++)
            {
                push32(getValFromRegsel(i));
            }
            push8(flagsRegister);
            break;

        case OP_CODE_STCK_POPALL_VARI:
            // Check for underflow
            if (stackPointer < STACK_PUSHALL_SIZE)
            {
                stackUnderflow = true;
                break;
            }
            // Check for memory violation
            if (false == MEMBOUND_CAN_READ(stackBase + stackPointer - STACK_PUSHALL_SIZE, STACK_PUSHALL_SIZE))
            {
                enqueueCriticalInterrupt(INTERRUPT_CODE_MEMORY_VIOLATION);
                return true;
            }
            // Popall
            flagsRegister = pop8();
            for (int8_t i = 7; i >= 0; i--)
            {
                generalRegisters[i] = pop32();
            }
            return true;

        case OP_CODE_STCK_PEEK_VARI:
            // Check for underflow
            if (stackPointer < regSize[regsel])
            {
                stackUnderflow = true;
                break;
            }
            // Check for memory violation
            if (false == MEMBOUND_CAN_READ(stackBase + stackPointer - regSize[regsel], regSize[regsel]))
            {
                enqueueCriticalInterrupt(INTERRUPT_CODE_MEMORY_VIOLATION);
                return true;
            }
            // Peek
            switch(regSize[regsel])
            {
                case 4:
                    transferVarToReg(regsel, peek32());
                    break;
                case 2:
                    transferVarToReg(regsel, peek16());
                    break;
                case 1:
                    transferVarToReg(regsel, peek8());
            }
            return true;

        case OP_CODE_STCK_RETURN_VARI:
            // Check for underflow
            if (stackPointer < 4)
            {
                stackUnderflow = true;
                break;
            }
            // Check for memory violation
            if (false == MEMBOUND_CAN_READ(stackBase + stackPointer - 4, 4))
            {
                enqueueCriticalInterrupt(INTERRUPT_CODE_MEMORY_VIOLATION);
                return true;
            }
            // Return
            programCounter = pop32();
            return false;
    }


    // Check for stack overflow
    if (false == alreadyOverflowed &&
        stackPointer + STACK_OVERFLOW_THRESHOLD >= stackSize)
    {
        enqueueCriticalInterrupt(INTERRUPT_CODE_CRITICAL_STACK);
    }
    else if (stackUnderflow)
    {
        enqueueCriticalInterrupt(INTERRULT_CODE_EMPTY_POP);
    }

    return true;
}

static bool checkBranch(uint8_t branchVari, uint8_t insAug, uint32_t address, bool isF4)
{
    bool branch = false;
    switch (branchVari)
    {
        case OP_CODE_BRNC_AL_VARI:
            branch = true;
            break;

        case OP_CODE_BRNC_EQ_VARI:
            // Z==1
            branch = (flagsRegister & FLAG_Z) != 0;
            break;

        case OP_CODE_BRNC_NE_VARI:
            // Z==0
            branch = (flagsRegister & FLAG_Z) == 0;
            break;

        case OP_CODE_BRNC_HI_VARI:
            // C==1 && Z==0
            branch = ((flagsRegister & FLAG_C) != 0) &&
                     ((flagsRegister & FLAG_Z) == 0);
            break;

        case OP_CODE_BRNC_HS_VARI:
            // C==1
            branch = (flagsRegister & FLAG_C) != 0;
            break;

        case OP_CODE_BRNC_LS_VARI:
            // C==0 || Z==1
            branch = ((flagsRegister & FLAG_C) == 0) ||
                     ((flagsRegister & FLAG_Z) != 0);
            break;

        case OP_CODE_BRNC_LO_VARI:
            // C==0
            branch = (flagsRegister & FLAG_C) == 0;
            break;

        case OP_CODE_BRNC_GT_VARI:
            // Z==0 && N==V
            branch = ((flagsRegister & FLAG_Z) == 0) &&
                     (((flagsRegister & FLAG_N) == 0) == 
                      ((flagsRegister & FLAG_V) == 0));
            break;
        
        case OP_CODE_BRNC_GE_VARI:
            // N==V
            branch = ((flagsRegister & FLAG_N) == 0) == 
                     ((flagsRegister & FLAG_V) == 0);
            break;

        case OP_CODE_BRNC_LE_VARI:
            // Z==1 || N!=V
             branch = ((flagsRegister & FLAG_Z) != 0) ||
                     (((flagsRegister & FLAG_N) == 0) != 
                      ((flagsRegister & FLAG_V) == 0));
            break;
        
        case OP_CODE_BRNC_LT_VARI:
            // N!=V
            branch = ((flagsRegister & FLAG_N) == 0) != 
                     ((flagsRegister & FLAG_V) == 0);
            break;

        case OP_CODE_BRNC_MI_VARI:
            // N==1
            branch = (flagsRegister & FLAG_N) != 0;
            break;

        case OP_CODE_BRNC_PZ_VARI:
            // N==0
            branch = (flagsRegister & FLAG_N) == 0;
            break;

        case OP_CODE_BRNC_OV_VARI:
            // V==1
            branch = (flagsRegister & FLAG_V) != 0;
            break;

        case OP_CODE_BRNC_NV_VARI:
            // V==0
            branch = (flagsRegister & FLAG_V) == 0;
    }

    if (branch)
    {
        if (insAug & INS_AUG_REL_MASK)
        {
            address += programCounter;
        }
        else
        {
            switch (insAug & INS_AUG_OFFSET_REGSEL_MASK)
            {
                case INS_AUG_OFFSET_REGSEL_OA:
                    address += offsetRegisters[0];
                    break;
                
                case INS_AUG_OFFSET_REGSEL_OB:
                    address += offsetRegisters[1];
                    break;

                case INS_AUG_OFFSET_REGSEL_OC:
                    address += offsetRegisters[2];
            }
        }

        if (insAug & INS_AUG_PUSH_MASK)
        {
            if (isF4)
            {
                push32(programCounter + 5);
            }
            else
            {
                push32(programCounter + 9);
            }
        }

        programCounter = address;
        
    }

    return branch;
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

static void run()
{
    #ifdef SELF_TEST
    st_defineStartTime();
    #endif //SELFTEST

    while (true)
    {
        detectInterrupt();

        memcpy(&instructionRegister, &(memory[programCounter]), sizeof(instructionRegister));

        switch (OP_CODE_GET_BASE(instructionRegister))
        {
            case OP_CODE_NOOP:
                programCounter+=4;
                break;

            case OP_CODE_INTR_BASE:
                if (true == doInterruptUtils(OP_CODE_GET_VARI(instructionRegister),
                                             REGSEL_1_GET(instructionRegister),
                                             ARG_F7_GET(instructionRegister)))
                {
                    programCounter+=4;
                }
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
                doCompare(getValFromRegsel(REGSEL_1_GET(instructionRegister)),
                          getValFromRegsel(REGSEL_2_GET(instructionRegister)));
                programCounter+=4;
                break;

            case OP_CODE_COMP_F4:
                memcpy(&argumentAugment, &(memory[programCounter + 4]), sizeof(argumentAugment));
                doCompare(getValFromRegsel(REGSEL_1_GET(instructionRegister)),
                          argumentAugment);
                programCounter+=8;
                break;

            case OP_CODE_BRNC_BASE_F4:
                memcpy(&instructionAugment, &(memory[programCounter + 4]), sizeof(instructionAugment));

                if (false == checkBranch(OP_CODE_GET_VARI(instructionRegister),
                                         instructionAugment,
                                         getValFromRegsel(REGSEL_1_GET(instructionRegister)),
                                         true))
                {
                    programCounter+=5;
                }

                break;

            case OP_CODE_BRNC_BASE_F6:
                memcpy(&instructionAugment, &(memory[programCounter + 4]), sizeof(instructionAugment));
                memcpy(&argumentAugment,    &(memory[programCounter + 5]), sizeof(argumentAugment));
                
                if (false == checkBranch(OP_CODE_GET_VARI(instructionRegister),
                                         instructionAugment,
                                         argumentAugment,
                                         false))
                {
                    programCounter+=9;
                }
                break;

            case OP_CODE_STCK_BASE:
                if (true == doStackUtils(OP_CODE_GET_VARI(instructionRegister),
                                         REGSEL_1_GET(instructionRegister)))
                {
                    programCounter+=4;
                }
                break;

            case OP_CODE_TERM_BASE:
                return;
        }

        #ifdef SELF_TEST

        st_startInterruptTime();

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
                                   currentInterrupt,
                                   memory))
        {
            return;
        }

        st_endInterruptTime();
        #endif // SELF_TEST
    }
}

static void teardown()
{
    #ifdef SELF_TEST
    st_exit();
    #endif // SELF_TEST

    mb_teardownDevices();

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