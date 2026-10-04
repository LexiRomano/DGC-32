#include "dgc32.h"

static uint8_t *memory               = NULL;
static mtx_t    interruptAccessMutex = {0};

// Exposed registers
static uint32_t generalRegisters[8] = {0};
static uint32_t offsetRegisters[3]  = {0};
static uint32_t stackBase           = 0;
static uint16_t stackSize           = 0;
static uint16_t stackPointer        = 0;
static uint32_t interruptTable      = 0;
static uint8_t  flagsRegister       = 0;

// Internal registers
static uint32_t programCounter              = 0;
static uint8_t  opCodeRegister              = 0;
static uint8_t  regselArg1Register          = 0;
static uint8_t  regselArg2Register          = 0;
static uint32_t instructionArgumentRegister = 0;
static uint32_t interruptReturnAddress      = 0;
static uint16_t currentInterrupt            = 0;
static uint8_t  interruptHead               = 0;
static uint8_t  interruptTail               = 0;
static uint8_t  statusRegister              = 0;

// Fudge factor
static uint32_t pcIncrementedBy = 0;

#define FETCH_FORM_1W                 \
do                                     \
{                                       \
    memcpy(&instructionArgumentRegister, \
           &memory[programCounter+1],     \
           sizeof(uint32_t));              \
    programCounter+=4;                      \
    pcIncrementedBy+=4;                      \
} while (0)

#define FETCH_FORM_1H             \
do                                 \
{                                   \
    uint16_t tmp16;                  \
    memcpy(&tmp16,                    \
           &memory[programCounter+1],  \
           sizeof(tmp16));              \
    instructionArgumentRegister = tmp16; \
    programCounter+=2;                    \
    pcIncrementedBy+=2;                    \
} while (0)

#define FETCH_FORM_2               \
do                                  \
{                                    \
    memcpy(&regselArg1Register,       \
           &memory[programCounter],    \
           sizeof(regselArg1Register)); \
    programCounter+=1;                   \
    pcIncrementedBy+=1;                   \
} while (0)

#define FETCH_FORM_2W              \
do                                  \
{                                    \
    memcpy(&regselArg1Register,       \
           &memory[programCounter],    \
           sizeof(regselArg1Register)); \
    memcpy(&instructionArgumentRegister, \
           &memory[programCounter+1],     \
           sizeof(uint32_t));              \
    programCounter+=5;                      \
    pcIncrementedBy+=5;                      \
} while (0)

#define FETCH_FORM_2H              \
do                                  \
{                                    \
    memcpy(&regselArg1Register,       \
           &memory[programCounter],    \
           sizeof(regselArg1Register)); \
    uint16_t tmp16;                      \
    memcpy(&tmp16,                        \
           &memory[programCounter+1],      \
           sizeof(tmp16));                  \
    instructionArgumentRegister = tmp16;     \
    programCounter+=3;                        \
    pcIncrementedBy+=3;                        \
} while (0)

#define FETCH_FORM_2C              \
do                                  \
{                                    \
    memcpy(&regselArg1Register,       \
           &memory[programCounter],    \
           sizeof(regselArg1Register)); \
    uint8_t tmp8;                        \
    memcpy(&tmp8,                         \
           &memory[programCounter+1],      \
           sizeof(tmp8));                   \
    instructionArgumentRegister = tmp8;      \
    programCounter+=2;                        \
    pcIncrementedBy+=2;                        \
} while (0)


#define FETCH_FORM_3  FETCH_FORM_2
#define FETCH_FORM_3W FETCH_FORM_2W
#define FETCH_FORM_3H FETCH_FORM_2H
#define FETCH_FORM_3C FETCH_FORM_2C

#define FETCH_FORM_4               \
do                                  \
{                                    \
    memcpy(&regselArg1Register,       \
           &memory[programCounter],    \
           sizeof(regselArg1Register)); \
    memcpy(&regselArg2Register,          \
           &memory[programCounter+1],     \
           sizeof(regselArg2Register));    \
    programCounter+=2;                      \
    pcIncrementedBy+=2;                      \
} while (0)

#define FETCH_FORM_4W              \
do                                  \
{                                    \
    memcpy(&regselArg1Register,       \
           &memory[programCounter],    \
           sizeof(regselArg1Register)); \
    memcpy(&regselArg2Register,          \
           &memory[programCounter+1],     \
           sizeof(regselArg2Register));    \
    memcpy(&instructionArgumentRegister,    \
           &memory[programCounter+2],        \
           sizeof(uint32_t));                 \
    programCounter+=6;                         \
    pcIncrementedBy+=6;                         \
} while (0)

#define FETCH_FORM_4H              \
do                                  \
{                                    \
    memcpy(&regselArg1Register,       \
           &memory[programCounter],    \
           sizeof(regselArg1Register)); \
    memcpy(&regselArg2Register,          \
           &memory[programCounter+1],     \
           sizeof(regselArg2Register));    \
    uint16_t tmp16;                         \
    memcpy(&tmp16,                           \
           &memory[programCounter+2],         \
           sizeof(tmp16));                     \
    instructionArgumentRegister = tmp16;        \
    programCounter+=4;                           \
    pcIncrementedBy+=4;                           \
} while (0)

#define FETCH_FORM_4C              \
do                                  \
{                                    \
    memcpy(&regselArg1Register,       \
           &memory[programCounter],    \
           sizeof(regselArg1Register)); \
    memcpy(&regselArg2Register,          \
           &memory[programCounter+1],     \
           sizeof(regselArg2Register));    \
    uint8_t tmp8;                           \
    memcpy(&tmp8,                            \
           &memory[programCounter+2],         \
           sizeof(tmp8));                      \
    instructionArgumentRegister = tmp8;         \
    programCounter+=3;                           \
    pcIncrementedBy+=3;                           \
} while (0)

#define FETCH_FORM_5  FETCH_FORM_2
#define FETCH_FORM_5W FETCH_FORM_2W
#define FETCH_FORM_5H FETCH_FORM_2H
#define FETCH_FORM_5C FETCH_FORM_2C

#define FETCH_FORM_6  FETCH_FORM_2
#define FETCH_FORM_6W FETCH_FORM_2W
#define FETCH_FORM_6H FETCH_FORM_2H
#define FETCH_FORM_6C FETCH_FORM_2C

#define FETCH_FORM_7  FETCH_FORM_4
#define FETCH_FORM_7W FETCH_FORM_4W
#define FETCH_FORM_7H FETCH_FORM_4H
#define FETCH_FORM_7C FETCH_FORM_4C

#define FETCH_FORM_8  FETCH_FORM_4
#define FETCH_FORM_8W FETCH_FORM_4W
#define FETCH_FORM_8H FETCH_FORM_4H
#define FETCH_FORM_8C FETCH_FORM_4C

#define SIG_EXT_H(var)  \
do                       \
{                         \
    if ((var) & 0x8000)    \
    {                       \
        (var) |= 0xFFFF0000; \
    }                         \
} while (0)

#define SIG_EXT_C(var)  \
do                       \
{                         \
    if ((var) & 0x80)      \
    {                       \
        (var) |= 0xFFFFFF00; \
    }                         \
} while (0)

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
    INTERRUPT_CODE_EMPTY_POP, // Critical stack event
    INTERRUPT_CODE_MEMORY_VIOLATION,
    INTERRUPT_CODE_INVALID_INSTRUCTION
};

#ifdef USER_TEST
char *regselNames[] =
{
    "G0",
    "G1",
    "G2",
    "G3",
    "G4",
    "G5",
    "G6",
    "G7",
    "OA",
    "OB",
    "OC",
    "SB",
    "SS",
    "SP",
    "IL",
    "FL"
};
#endif // USER_TEST

// GLFW data
static glfwInfo_t glfwInfo = {0};

/*******************************************************************************
* Motherboard memory access functions
*******************************************************************************/
static void readMemForMB(uint32_t address, uint8_t numBytes, void *data)
{
    memcpy(data, &(memory[address]), numBytes);
}

static void writeMemForMB(uint32_t address, uint8_t numBytes, void *data)
{
    memcpy(&(memory[address]), data, numBytes);
}

/*******************************************************************************
* Enqueues an interrupt. Accessable by the motherboard
*******************************************************************************/
static inline void enqueueInterrupt(uint16_t interrupt)
{
    mtx_lock(&interruptAccessMutex);

    memcpy(&(memory[INTERRUPT_QUEUE_BASE + interruptHead]), &interrupt, sizeof(interrupt));

    interruptHead = (interruptHead + 2) % INTERRUPT_QUEUE_SIZE;

    mtx_unlock(&interruptAccessMutex);
}


/*******************************************************************************
* Parse args, initialize memory, read ROM, start up motherboard.
*******************************************************************************/
static bool init(int argc, char* argv[])
{
    char              *romFileName                                         = NULL;
    FILE              *romFile                                             = NULL;
    FILE              *testFileExists                                      = NULL;
    char              *storageDeviceFileNames[MAX_INITIAL_STORAGE_DEVICES] = {0};
    externalFileInfo_t motherboardInitParams                               = {0};
    uint8_t            argumentIndex                                       = 1;
    const char*        glfwErrorCode                                       = NULL;

    // Init the interrupt access mutex
    mtx_init(&interruptAccessMutex, mtx_plain);

    // Parsing command line arguments
    #ifdef SELF_TEST

    if (argumentIndex < argc)
    {
        if (false == st_setTestFile(argv[argumentIndex]))
        {
            return false;
        }
    }
    else
    {
        printf("Self test error: test file required (1st argument)\n");
        return false;
    }

    argumentIndex++;

    #endif // SELF_TEST

    if (argumentIndex < argc)
    {
        // Rom
        romFileName = argv[argumentIndex++];
    }
    else
    {
        printf("ROM file must be specified\n");
        return false;
    }

    for (uint8_t i = 0; i < MAX_INITIAL_STORAGE_DEVICES && argumentIndex < argc; i++)
    {
        storageDeviceFileNames[i] = argv[argumentIndex];
        argumentIndex++;
    }

    if (argc > argumentIndex)
    {
        printf("Too many arguments, exceded maximum of %d storage devices at startup\n", MAX_INITIAL_STORAGE_DEVICES);
        return false;
    }

    // Init memory
    memory = malloc(MEMORY_SIZE * sizeof(uint8_t));

    if (NULL == memory)
    {
        printf("Internal error: failed to allocate space for memory\n");
        return false;
    }

    // Load ROM
    romFile = fopen(romFileName, "rb");

    if (NULL == romFile)
    {
        printf("Could not open ROM file at \"%s\"\n", romFileName);
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

    // Preparing motherboard arguments
    motherboardInitParams.romFileName = romFileName;

    for (uint8_t i = 0; i < MAX_INITIAL_STORAGE_DEVICES; i++)
    {
        if (NULL == storageDeviceFileNames[i])
        {
            break;
        }

        testFileExists = fopen(storageDeviceFileNames[i], "r");

        if (NULL == testFileExists)
        {
            printf("Could not open storage device file at \"%s\"\n", storageDeviceFileNames[i]);
            return false;
        }

        fclose(testFileExists);
        testFileExists = NULL;

        motherboardInitParams.storageFileNames[i] = storageDeviceFileNames[i];
    }

    // Setup GLFW
    if (!glfwInit())
    {
        printf("Failed to initialze GLFW\n");
        // Initialization failed
        return false;
    }

    glfwInfo.window = glfwCreateWindow(960, 540, "DGC-32", NULL, NULL);

    if (NULL == glfwInfo.window)
    {
        (void) glfwGetError(&glfwErrorCode);
        printf("Failed to create GLFW window: %s\n", glfwErrorCode);
        return false;
    }

    glfwMakeContextCurrent(glfwInfo.window);

    // Initialize the motherboard
    return mb_init(&motherboardInitParams, &glfwInfo, readMemForMB, writeMemForMB, enqueueInterrupt);
}

static inline void enqueueCriticalInterrupt(uint16_t interrupt)
{
    mtx_lock(&interruptAccessMutex);

    interruptTail = (interruptTail - 2) % INTERRUPT_QUEUE_SIZE;
    memcpy(&(memory[INTERRUPT_QUEUE_BASE + interruptTail]), &interrupt, sizeof(interrupt));

    mtx_unlock(&interruptAccessMutex);
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

static void transferMemToReg(uint8_t toRegsel, uint32_t fromAddress, uint8_t dataSize)
{
    uint32_t  buf32 = 0;
    uint16_t  buf16 = 0;
    uint8_t   buf8  = 0;
    uint32_t *reg32 = NULL;
    uint16_t *reg16 = NULL;
    uint8_t  *reg8  = NULL;

    // Check for memory violations
    switch (dataSize)
    {
        case 4:
        {
            if (false == MEMBOUND_CAN_READ(fromAddress, 4))
            {
                enqueueCriticalInterrupt(INTERRUPT_CODE_MEMORY_VIOLATION);
                return;
            }
            break;
        }
        case 2:
        {
            if (false == MEMBOUND_CAN_READ(fromAddress, 2))
            {
                enqueueCriticalInterrupt(INTERRUPT_CODE_MEMORY_VIOLATION);
                return;
            }
            break;
        }
        case 1:
        {
            if (false == MEMBOUND_CAN_READ(fromAddress, 1))
            {
                enqueueCriticalInterrupt(INTERRUPT_CODE_MEMORY_VIOLATION);
                return;
            }
            break;
        }
        default:
        {
            printf("EMULATOR ERROR: unexpected data size %hhu in %s:%d",
                   dataSize, __FUNCTION__, __LINE__);
            break;
        }
    }

    switch (regSize[toRegsel])
    {
        case 4:
        {
            reg32 = regMap4[toRegsel];
            switch (dataSize)
            {
                case 4:
                {
                    memcpy(reg32, &(memory[fromAddress]), sizeof(uint32_t));
                    break;
                }
                case 2:
                {
                    memcpy(&buf16, &(memory[fromAddress]), sizeof(uint16_t));
                    *reg32 = (uint32_t) buf16;
                    break;
                }
                case 1:
                {
                    memcpy(&buf8, &(memory[fromAddress]), sizeof(uint8_t));
                    *reg32 = (uint32_t) buf8;
                    break;
                }
            }

            break;
        }
        case 2:
        {
            reg16 = regMap2[toRegsel];
            switch (dataSize)
            {
                case 4:
                {
                    memcpy(&buf32, &(memory[fromAddress]), sizeof(uint32_t));
                    *reg16 = (uint16_t) buf32 & 0xFFFF;
                    break;
                }
                case 2:
                {
                    memcpy(reg16, &(memory[fromAddress]), sizeof(uint16_t));
                    break;
                }
                case 1:
                {
                    memcpy(&buf8, &(memory[fromAddress]), sizeof(uint8_t));
                    *reg16 = (uint16_t) buf8;
                    break;
                }
            }

            break;
        }
        case 1:
        {
            reg8 = regMap1[toRegsel];
            switch (dataSize)
            {
                case 4:
                {
                    memcpy(&buf32, &(memory[fromAddress]), sizeof(uint32_t));
                    *reg8 = (uint8_t) buf32 & 0xFF;
                    break;
                }
                case 2:
                {
                    memcpy(&buf16, &(memory[fromAddress]), sizeof(uint16_t));
                    *reg8 = (uint8_t) buf16 & 0xFF;
                    break;
                }
                case 1:
                {
                    memcpy(reg8, &(memory[fromAddress]), sizeof(uint8_t));
                    break;
                }
            }

            break;
        }
    }

    switch (dataSize)
    {
        case 4:
            mb_readFromDeviceData(fromAddress, 4);
            return;

        case 2:
            mb_readFromDeviceData(fromAddress, 2);
            return;

        case 1:
            mb_readFromDeviceData(fromAddress, 1);
            return;
    }
}

static void transferRegToMem(uint32_t toAddress, uint8_t fromRegsel, uint8_t dataSize)
{
    uint32_t  buf32 = 0;
    uint16_t  buf16 = 0;
    uint8_t   buf8  = 0;

    // Check for memory violations
    switch (dataSize)
    {
        case 4:
        {
            if (false == MEMBOUND_CAN_WRITE(toAddress, 4))
            {
                enqueueCriticalInterrupt(INTERRUPT_CODE_MEMORY_VIOLATION);
                return;
            }
            if (false == mb_canWriteToDeviceData(toAddress, 4))
            {
                return;
            }
            break;
        }
        case 2:
        {
            if (false == MEMBOUND_CAN_WRITE(toAddress, 2))
            {
                enqueueCriticalInterrupt(INTERRUPT_CODE_MEMORY_VIOLATION);
                return;
            }
            if (false == mb_canWriteToDeviceData(toAddress, 2))
            {
                return;
            }
            break;
        }
        case 1:
        {
            if (false == MEMBOUND_CAN_WRITE(toAddress, 1))
            {
                enqueueCriticalInterrupt(INTERRUPT_CODE_MEMORY_VIOLATION);
                return;
            }
            if (false == mb_canWriteToDeviceData(toAddress, 1))
            {
                return;
            }
            break;
        }
        default:
        {
            printf("EMULATOR ERROR: unexpected data size %hhu in %s:%d",
                   dataSize, __FUNCTION__, __LINE__);
            break;
        }

    }

    switch (regSize[fromRegsel])
    {
        case 4:
        {
            buf32 = *(regMap4[fromRegsel]);

            switch(dataSize)
            {
                case 4:
                {
                    memcpy(&(memory[toAddress]), &buf32, sizeof(uint32_t));
                    mb_writeToDeviceData(toAddress, 4, &(memory[toAddress]));
                    break;
                }
                case 2:
                {
                    buf16 = (uint16_t) buf32 & 0xFFFF;
                    memcpy(&(memory[toAddress]), &buf16, sizeof(uint16_t));
                    mb_writeToDeviceData(toAddress, 2, &(memory[toAddress]));
                    break;
                }
                case 1:
                {
                    buf8 = (uint8_t) buf32 & 0xFF;
                    memcpy(&(memory[toAddress]), &buf8, sizeof(uint8_t));
                    mb_writeToDeviceData(toAddress, 1, &(memory[toAddress]));
                    break;
                }
            }

            return;
        }
        case 2:
        {
            buf16 = *(regMap2[fromRegsel]);

            switch(dataSize)
            {
                case 4:
                {
                    buf32 = (uint32_t) buf16;
                    memcpy(&(memory[toAddress]), &buf32, sizeof(uint32_t));
                    mb_writeToDeviceData(toAddress, 4, &(memory[toAddress]));
                    break;
                }
                case 2:
                {
                    memcpy(&(memory[toAddress]), &buf16, sizeof(uint16_t));
                    mb_writeToDeviceData(toAddress, 2, &(memory[toAddress]));
                    break;
                }
                case 1:
                {
                    buf8 = (uint8_t) buf16 & 0xFF;
                    memcpy(&(memory[toAddress]), &buf8, sizeof(uint8_t));
                    mb_writeToDeviceData(toAddress, 1, &(memory[toAddress]));
                    break;
                }
            }

            return;
        }
        case 1:
        {
            buf8 = *(regMap1[fromRegsel]);

            switch(dataSize)
            {
                case 4:
                    buf32 = (uint32_t) buf8;
                    memcpy(&(memory[toAddress]), &buf32, sizeof(uint32_t));
                    mb_writeToDeviceData(toAddress, 4, &(memory[toAddress]));
                    break;
                case 2:
                    buf16 = (uint16_t) buf8;
                    memcpy(&(memory[toAddress]), &buf16, sizeof(uint16_t));
                    mb_writeToDeviceData(toAddress, 2, &(memory[toAddress]));
                    break;
                case 1:
                    memcpy(&(memory[toAddress]), &buf8, sizeof(uint8_t));
                    mb_writeToDeviceData(toAddress, 1, &(memory[toAddress]));
            }

            return;
        }
    }
}

static void doSigExt(uint8_t regsel, bool toChar)
{
    switch (regSize[regsel])
    {
        case 4:
        {
            if (toChar)
            {
                SIG_EXT_C(*regMap4[regsel]);
            }
            else
            {
                SIG_EXT_H(*regMap4[regsel]);
            }
            break;
        }
        case 2:
        {
            if (toChar)
            {
                SIG_EXT_C(*regMap4[regsel]);
            }
            break;
        }
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

static void detectInterrupt()
{
    bool isCriticalInterrupt = false;

    mtx_lock(&interruptAccessMutex);

    if (isInterruptQueueEmpty())
    {
        mtx_unlock(&interruptAccessMutex);
        return;
    }

    if (0 != (statusRegister & STAT_REG_INT_IN_PROG_MASK))
    {
        mtx_unlock(&interruptAccessMutex);
        return;
    }

    if (0 == interruptTable)
    {
        mtx_unlock(&interruptAccessMutex);
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
        mtx_unlock(&interruptAccessMutex);
        return;
    }

    dequeueInterrupt(&currentInterrupt);

    mtx_unlock(&interruptAccessMutex);

    interruptReturnAddress = programCounter;
    
    programCounter = getInterruptHandleLocation(currentInterrupt & INTERRUPT_ID_MASK);

    statusRegister = statusRegister | STAT_REG_INT_IN_PROG_MASK;
}

static void doMath(mathOperation_e operation, uint8_t destRegsel, uint32_t a, uint32_t b)
{
    uint32_t result   = 0;
    uint64_t buf64    = 0;
    float    aFloat   = 0;
    float    bFloat   = 0;
    float    rFloat   = 0;
    bool     wasFloat = false;
    bool     overflow = false;
    bool     carry    = false;

    switch (operation)
    {
        case MATH_OPERATION_SUB_INT:
            b = (~b) +1;
            // Fall through \/
        case MATH_OPERATION_ADD_INT:
            result = a + b;
            carry  = result < a;
            overflow = carry ^
                       (0 !=(((a & 0x7FFFFFFF) + (b & 0x7FFFFFFF)) & 0x80000000));
            break;

        case MATH_OPERATION_ADD_FL:
        {
            wasFloat = true;
            aFloat = *((float*) &a);
            bFloat = *((float*) &b);

            rFloat = aFloat + bFloat;

            result = *((uint32_t*) &rFloat);

            break;
        }
        case MATH_OPERATION_SUB_FL:
        {
            wasFloat = true;
            aFloat = *((float*) &a);
            bFloat = *((float*) &b);

            rFloat = aFloat - bFloat;

            result = *((uint32_t*) &rFloat);

            // Make the comparisons work
            if (aFloat >= bFloat)
            {
                overflow = aFloat >= 0;
            }
            else
            {
                overflow = aFloat < 0;
            }
            break;
        }
        case MATH_OPERATION_MUL_INT:
        {
            result   = a * b;
            overflow = a != 0 &&
                       result / a != b;
            break;
        }
        case MATH_OPERATION_MUL_FL:
        {
            wasFloat = true;
            aFloat = *((float*) &a);
            bFloat = *((float*) &b);

            rFloat = aFloat * bFloat;

            result = *((uint32_t*) &rFloat);

            break;
        }
        case MATH_OPERATION_DIV_INT:
        {
            if (b == 0)
            {
                enqueueCriticalInterrupt(INTERRUPT_CODE_DIVISION_BY_ZERO);
                return;
            }
            result = a / b;

            break;
        }
        case MATH_OPERATION_DIV_FL:
        {
            if (b == 0)
            {
                enqueueCriticalInterrupt(INTERRUPT_CODE_DIVISION_BY_ZERO);
                return;
            }

            wasFloat = true;
            aFloat = *((float*) &a);
            bFloat = *((float*) &b);

            rFloat = aFloat / bFloat;

            result = *((uint32_t*) &rFloat);

            break;
        }
        case MATH_OPERATION_MOD:
        {
            if (b == 0)
            {
                enqueueCriticalInterrupt(INTERRUPT_CODE_DIVISION_BY_ZERO);
                return;
            }

            result = a - (a / b * b);

            break;
        }
        case MATH_OPERATION_AND:
        {
            result = a & b;
            break;
        }
        case MATH_OPERATION_OR:
        {
            result = a | b;
            break;
        }
        case MATH_OPERATION_XOR:
        {
            result = a ^ b;
            break;
        }
        case MATH_OPERATION_NOT:
        {
            result = ~a;
            break;
        }
        case MATH_OPERATION_BSLT:
        {
            buf64 = (uint64_t) a;
            buf64 = buf64 << (b % 32);
            result = (uint32_t) buf64 & 0xFFFFFFFF;
            carry = (buf64 & 0x0000000100000000) != 0;
            break;
        }
        case MATH_OPERATION_BSRT:
        {
            buf64 = ((uint64_t) a) << 32;
            buf64 = buf64 >> (b % 32);
            result = (uint32_t) ((buf64 & 0xFFFFFFFF00000000) >> 32);
            carry = (buf64 & 0x80000000) != 0;
            break;
        }
        case MATH_OPERATION_BSLC:
        {
            buf64 = (uint64_t) a;
            carry = (flagsRegister & FLAG_C) != 0;
            for (uint8_t i = 0; i < (b % 32); i++)
            {
                buf64 = buf64 << 1;
                buf64 += carry ? 1 : 0;
                carry = (buf64 & 0x100000000) != 0;
                buf64 = (buf64 & 0xFFFFFFFF);
            }
            result = buf64;
            break;
        }
        case MATH_OPERATION_BSRC:
        {
            buf64 = (uint64_t) a;
            carry = (flagsRegister & FLAG_C) != 0;
            for (uint8_t i = 0; i < (b % 32); i++)
            {
                buf64 += carry ? 0x100000000 : 0;
                carry = (buf64 & 0x1) != 0;
                buf64 = buf64 >> 1;
            }
            result = buf64;
            break;
        }
        default:
        {
            printf("EMULATOR ERROR: unexpected math operation %d in %s:%d",
                   operation, __FUNCTION__, __LINE__);
            return;
        }
    }

    transferVarToReg(destRegsel, result);
    if (wasFloat)
    {
        flagsRegister = ((rFloat == 0 ? FLAG_Z : 0) +
                         (rFloat <  0 ? FLAG_N : 0) +
                         (carry       ? FLAG_C : 0) +
                         (overflow    ? FLAG_V : 0));
    }
    else
    {
        flagsRegister = ((result == 0          ? FLAG_Z : 0) +
                         (result >  0x7FFFFFFF ? FLAG_N : 0) +
                         (carry                ? FLAG_C : 0) +
                         (overflow             ? FLAG_V : 0));
    }
    return;
}

static void doCompare(bool isInt, uint32_t a, uint32_t b)
{
    uint32_t result   = 0;
    bool     carry    = false;
    bool     overflow = false;
    float    aFloat   = 0;
    float    bFloat   = 0;
    float    rFloat   = 0;

    if (isInt)
    {
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
    else
    {
        aFloat = *((float*) &a);
        bFloat = *((float*) &b);

        rFloat = aFloat - bFloat;

        if (aFloat >= bFloat)
        {
            overflow = aFloat >= 0;
        }
        else
        {
            overflow = aFloat < 0;
        }

        flagsRegister = ((rFloat == 0 ? FLAG_Z : 0) +
                         (rFloat <  0 ? FLAG_N : 0) +
                         (carry       ? FLAG_C : 0) +
                         (overflow    ? FLAG_V : 0));
    }
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

static void doStackUtils(stackUtil_e type, uint8_t regsel)
{
    bool alreadyOverflowed = false;
    bool stackUnderflow    = false;

    alreadyOverflowed = stackPointer + STACK_OVERFLOW_THRESHOLD >= stackSize;

    switch (type)
    {
        case STACK_UTIL_PUSH:
        {
            // Check for memory violation
            if (false == MEMBOUND_CAN_WRITE(stackBase + stackPointer, regSize[regsel]))
            {
                enqueueCriticalInterrupt(INTERRUPT_CODE_MEMORY_VIOLATION);
                return;
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
        }
        case STACK_UTIL_POP:
        {
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
                return;
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
            return;
        }
        case STACK_UTIL_PEEK:
        {
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
                return;
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
            return;
        }
        case STACK_UTIL_RETURN:
        {
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
                return;
            }
            // Return
            programCounter = pop32();
            return;
        }
        default:
        {
            printf("EMULATOR ERROR: unexpected math operation %d in %s:%d",
                   type, __FUNCTION__, __LINE__);
            return;
        }
    }

    // Check for stack overflow
    if (false == alreadyOverflowed &&
        stackPointer + STACK_OVERFLOW_THRESHOLD >= stackSize)
    {
        enqueueCriticalInterrupt(INTERRUPT_CODE_CRITICAL_STACK);
    }
    else if (stackUnderflow)
    {
        enqueueCriticalInterrupt(INTERRUPT_CODE_EMPTY_POP);
    }

    return;
}

static void protectedPush32(uint32_t val)
{
    bool alreadyOverflowed = stackPointer + STACK_OVERFLOW_THRESHOLD >= stackSize;

    // Check for memory violation
    if (false == MEMBOUND_CAN_WRITE(stackBase + stackPointer, 4))
    {
        enqueueCriticalInterrupt(INTERRUPT_CODE_MEMORY_VIOLATION);
        return;
    }

    push32(val);

    if (false == alreadyOverflowed &&
        stackPointer + STACK_OVERFLOW_THRESHOLD >= stackSize)
    {
        enqueueCriticalInterrupt(INTERRUPT_CODE_CRITICAL_STACK);
    }
}

static bool checkCondition(uint8_t compareCode)
{
    switch (compareCode)
    {
        case COMP_CODE_AL:
        {
            return true;
        }
        case COMP_CODE_EQ:
        {
            // Z==1
            return (flagsRegister & FLAG_Z) != 0;
        }
        case COMP_CODE_NE:
        {   // Z==0
            return (flagsRegister & FLAG_Z) == 0;
        }
        case COMP_CODE_HI:
        {   // C==1 && Z==0
            return ((flagsRegister & FLAG_C) != 0) &&
                     ((flagsRegister & FLAG_Z) == 0);
        }
        case COMP_CODE_HS:
        {   // C==1
            return (flagsRegister & FLAG_C) != 0;
        }
        case COMP_CODE_LS:
        {   // C==0 || Z==1
            return ((flagsRegister & FLAG_C) == 0) ||
                     ((flagsRegister & FLAG_Z) != 0);
        }
        case COMP_CODE_LO:
        {   // C==0
            return (flagsRegister & FLAG_C) == 0;
        }
        case COMP_CODE_GT:
        {   // Z==0 && N==V
            return ((flagsRegister & FLAG_Z) == 0) &&
                     (((flagsRegister & FLAG_N) == 0) == 
                      ((flagsRegister & FLAG_V) == 0));
        }
        case COMP_CODE_GE:
        {   // N==V
            return ((flagsRegister & FLAG_N) == 0) == 
                     ((flagsRegister & FLAG_V) == 0);
        }
        case COMP_CODE_LE:
        {   // Z==1 || N!=V
            return ((flagsRegister & FLAG_Z) != 0) ||
                     (((flagsRegister & FLAG_N) == 0) != 
                      ((flagsRegister & FLAG_V) == 0));
        }
        case COMP_CODE_LT:
        {   // N!=V
            return ((flagsRegister & FLAG_N) == 0) != 
                     ((flagsRegister & FLAG_V) == 0);
        }
        case COMP_CODE_MI:
        {   // N==1
            return (flagsRegister & FLAG_N) != 0;
        }
        case COMP_CODE_PZ:
        {   // N==0
            return (flagsRegister & FLAG_N) == 0;
        }
        case COMP_CODE_OV:
        {   // V==1
            return (flagsRegister & FLAG_V) != 0;
        }
        case COMP_CODE_NV:
        {   // V==0
            return (flagsRegister & FLAG_V) == 0;
        }
        default:
        {
            enqueueCriticalInterrupt(INTERRUPT_CODE_INVALID_INSTRUCTION);
            return false; // Return false so branches don't push PC
        }
    }
}

static uint32_t applyOffset(addressingMode_e addressingMode, uint32_t baseAddress)
{
    switch (addressingMode)
    {
        case ADDRESSING_MODE_ABS:
        {
            return baseAddress;
        }
        case ADDRESSING_MODE_PC:
        {
            return baseAddress + programCounter - pcIncrementedBy;
        }
        case ADDRESSING_MODE_OA:
        {
            return baseAddress + offsetRegisters[0];
        }
        case ADDRESSING_MODE_OB:
        {
            return baseAddress + offsetRegisters[1];
        }
        case ADDRESSING_MODE_OC:
        {
            return baseAddress + offsetRegisters[2];
        }
        case ADDRESSING_MODE_PC_M_OA:
        {
            return baseAddress - offsetRegisters[0] + programCounter - pcIncrementedBy;
        }
        case ADDRESSING_MODE_PC_M_OB:
        {
            return baseAddress - offsetRegisters[1] + programCounter - pcIncrementedBy;
        }
        case ADDRESSING_MODE_PC_M_OC:
        {
            return baseAddress - offsetRegisters[2] + programCounter - pcIncrementedBy;
        }
        default:
        {
            printf("EMULATOR ERROR: unexpected addressing mode %d in %s:%d",
                   addressingMode, __FUNCTION__, __LINE__);
            return 0;
        }
    }
}

#define SWAP_PRE_LOAD                                       \
do                                                           \
{                                                             \
    if (opCodeRegister > 0x80)                                 \
    {                                                           \
        tmpA = getValFromRegsel(LOW_NIBBLE(regselArg1Register)); \
    }                                                             \
} while (0)

#define SWAP_POST_LOAD(destAddress, wordSize)               \
do                                                           \
{                                                             \
    if (opCodeRegister > 0x80)                                 \
    {                                                           \
        tmpB = getValFromRegsel(LOW_NIBBLE(regselArg1Register)); \
        transferVarToReg(LOW_NIBBLE(regselArg1Register), tmpA);   \
        transferRegToMem(destAddress,                              \
                         LOW_NIBBLE(regselArg1Register),            \
                         wordSize);                                  \
        transferVarToReg(LOW_NIBBLE(regselArg1Register), tmpB);       \
    }                                                                  \
} while (0)

static int cpuThreadFunction(void *arg)
{
    (void)arg;
    uint32_t tmpA, tmpB = 0;
    #ifdef SELF_TEST
    st_defineStartTime();
    #endif //SELFTEST

    while (mb_powerState())
    {
        detectInterrupt();

        memcpy(&opCodeRegister, &(memory[programCounter++]), sizeof(opCodeRegister));
        pcIncrementedBy=1;

        // Avert your eyes
        switch (opCodeRegister)
        {
            case OP_DEBUG:
            {
                FETCH_FORM_2;
                #ifdef USER_TEST
                printf("DGC-32 DEBUG: %s = 0x%08x\n", regselNames[LOW_NIBBLE(regselArg1Register)],
                                                      getValFromRegsel(LOW_NIBBLE(regselArg1Register)));
                #else
                enqueueCriticalInterrupt(INTERRUPT_CODE_INVALID_INSTRUCTION);
                #endif //USER_TEST
                break;
            }
            case OP_MOVE_REG:
            {
                FETCH_FORM_3;
                transferRegToReg(LOW_NIBBLE(regselArg1Register),
                                 HIGH_NIBBLE(regselArg1Register));
                break;
            }
            case OP_MOVE_IM_32:
            {
                FETCH_FORM_2W;
                transferVarToReg(LOW_NIBBLE(regselArg1Register), instructionArgumentRegister);
                break;
            }
            case OP_MOVE_IM_16:
            {
                FETCH_FORM_2H;
                transferVarToReg(LOW_NIBBLE(regselArg1Register), instructionArgumentRegister);
                break;
            }
            case OP_MOVE_IM_16_SIG:
            {
                FETCH_FORM_2H;
                SIG_EXT_H(instructionArgumentRegister);
                transferVarToReg(LOW_NIBBLE(regselArg1Register), instructionArgumentRegister);
                break;
            }
            case OP_MOVE_IM_8:
            {
                FETCH_FORM_2C;
                transferVarToReg(LOW_NIBBLE(regselArg1Register), instructionArgumentRegister);
                break;
            }
            case OP_MOVE_IM_8_SIG:
            {
                FETCH_FORM_2C;
                SIG_EXT_C(instructionArgumentRegister);
                transferVarToReg(LOW_NIBBLE(regselArg1Register), instructionArgumentRegister);
                break;
            }
            case OP_LOAD_32_REG_ABS:
            case OP_SWAP_32_REG_ABS:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                                 4);
                SWAP_POST_LOAD(getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                               4);
                break;
            }
            case OP_LOAD_32_REG_PC:
            case OP_SWAP_32_REG_PC:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_PC,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 4);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_PC,
                                           getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                               4);
                break;
            }
            case OP_LOAD_32_REG_OA:
            case OP_SWAP_32_REG_OA:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OA,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 4);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OA,
                                           getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                               4);
                break;
            }
            case OP_LOAD_32_REG_OB:
            case OP_SWAP_32_REG_OB:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OB,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 4);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OB,
                                           getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                               4);
                break;
            }
            case OP_LOAD_32_REG_OC:
            case OP_SWAP_32_REG_OC:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OC,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 4);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OC,
                                           getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                               4);
                break;
            }
            case OP_LOAD_16_REG_ABS:
            case OP_SWAP_16_REG_ABS:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                                 2);
                SWAP_POST_LOAD(getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                               2);
                break;
            }
            case OP_LOAD_16_REG_PC:
            case OP_SWAP_16_REG_PC:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_PC,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 2);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_PC,
                                           getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                               2);
                break;
            }
            case OP_LOAD_16_REG_OA:
            case OP_SWAP_16_REG_OA:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OA,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 2);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OA,
                                           getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                               2);
                break;
            }
            case OP_LOAD_16_REG_OB:
            case OP_SWAP_16_REG_OB:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OB,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 2);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OB,
                                           getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                               2);
                break;
            }
            case OP_LOAD_16_REG_OC:
            case OP_SWAP_16_REG_OC:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OC,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 2);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OC,
                                           getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                               2);
                break;
            }
            case OP_LOAD_16_SIG_REG_ABS:
            case OP_SWAP_16_SIG_REG_ABS:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                                 2);
                doSigExt(LOW_NIBBLE(regselArg1Register), false);
                SWAP_POST_LOAD(getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                               2);
                break;
            }
            case OP_LOAD_16_SIG_REG_PC:
            case OP_SWAP_16_SIG_REG_PC:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_PC,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 2);
                doSigExt(LOW_NIBBLE(regselArg1Register), false);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_PC,
                                           getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                               2);
                break;
            }
            case OP_LOAD_16_SIG_REG_OA:
            case OP_SWAP_16_SIG_REG_OA:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OA,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 2);
                doSigExt(LOW_NIBBLE(regselArg1Register), false);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OA,
                                           getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                               2);
                break;
            }
            case OP_LOAD_16_SIG_REG_OB:
            case OP_SWAP_16_SIG_REG_OB:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OB,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 2);
                doSigExt(LOW_NIBBLE(regselArg1Register), false);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OB,
                                           getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                               2);
                break;
            }
            case OP_LOAD_16_SIG_REG_OC:
            case OP_SWAP_16_SIG_REG_OC:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OC,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 2);
                doSigExt(LOW_NIBBLE(regselArg1Register), false);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OC,
                                           getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                               2);
                break;
            }
            case OP_LOAD_8_REG_ABS:
            case OP_SWAP_8_REG_ABS:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                                 1);
                SWAP_POST_LOAD(getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                               1);
                break;
            }
            case OP_LOAD_8_REG_PC:
            case OP_SWAP_8_REG_PC:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_PC,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 1);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_PC,
                                           getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                               1);
                break;
            }
            case OP_LOAD_8_REG_OA:
            case OP_SWAP_8_REG_OA:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OA,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 1);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OA,
                                           getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                               1);
                break;
            }
            case OP_LOAD_8_REG_OB:
            case OP_SWAP_8_REG_OB:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OB,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 1);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OB,
                                           getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                               1);
                break;
            }
            case OP_LOAD_8_REG_OC:
            case OP_SWAP_8_REG_OC:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OC,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 1);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OC,
                                           getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                               1);
                break;
            }
            case OP_LOAD_8_SIG_REG_ABS:
            case OP_SWAP_8_SIG_REG_ABS:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                                 1);
                doSigExt(LOW_NIBBLE(regselArg1Register), true);
                SWAP_POST_LOAD(getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                               1);
                break;
            }
            case OP_LOAD_8_SIG_REG_PC:
            case OP_SWAP_8_SIG_REG_PC:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_PC,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 1);
                doSigExt(LOW_NIBBLE(regselArg1Register), true);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_PC,
                                           getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                               1);
                break;
            }
            case OP_LOAD_8_SIG_REG_OA:
            case OP_SWAP_8_SIG_REG_OA:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OA,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 1);
                doSigExt(LOW_NIBBLE(regselArg1Register), true);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OA,
                                           getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                               1);
                break;
            }
            case OP_LOAD_8_SIG_REG_OB:
            case OP_SWAP_8_SIG_REG_OB:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OB,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 1);
                doSigExt(LOW_NIBBLE(regselArg1Register), true);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OB,
                                           getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                               1);
                break;
            }
            case OP_LOAD_8_SIG_REG_OC:
            case OP_SWAP_8_SIG_REG_OC:
            {
                FETCH_FORM_3;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OC,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 1);
                doSigExt(LOW_NIBBLE(regselArg1Register), true);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OC,
                                           getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                               1);
                break;
            }
            case OP_LOAD_32_IM_ABS:
            case OP_SWAP_32_IM_ABS:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 instructionArgumentRegister,
                                 4);
                SWAP_POST_LOAD(instructionArgumentRegister,
                               4);
                break;
            }
            case OP_LOAD_32_IM_PC:
            case OP_SWAP_32_IM_PC:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_PC,
                                             instructionArgumentRegister),
                                 4);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_PC,
                                           instructionArgumentRegister),
                               4);
                break;
            }
            case OP_LOAD_32_IM_OA:
            case OP_SWAP_32_IM_OA:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OA,
                                             instructionArgumentRegister),
                                 4);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OA,
                                           instructionArgumentRegister),
                               4);
                break;
            }
            case OP_LOAD_32_IM_OB:
            case OP_SWAP_32_IM_OB:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OB,
                                             instructionArgumentRegister),
                                 4);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OB,
                                           instructionArgumentRegister),
                               4);
                break;
            }
            case OP_LOAD_32_IM_OC:
            case OP_SWAP_32_IM_OC:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OC,
                                             instructionArgumentRegister),
                                 4);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OC,
                                           instructionArgumentRegister),
                               4);
                break;
            }
            case OP_LOAD_16_IM_ABS:
            case OP_SWAP_16_IM_ABS:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 instructionArgumentRegister,
                                 2);
                SWAP_POST_LOAD(instructionArgumentRegister,
                               2);
                break;
            }
            case OP_LOAD_16_IM_PC:
            case OP_SWAP_16_IM_PC:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_PC,
                                             instructionArgumentRegister),
                                 2);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_PC,
                                           instructionArgumentRegister),
                               2);
                break;
            }
            case OP_LOAD_16_IM_OA:
            case OP_SWAP_16_IM_OA:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OA,
                                             instructionArgumentRegister),
                                 2);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OA,
                                           instructionArgumentRegister),
                               2);
                break;
            }
            case OP_LOAD_16_IM_OB:
            case OP_SWAP_16_IM_OB:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OB,
                                             instructionArgumentRegister),
                                 2);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OB,
                                           instructionArgumentRegister),
                               2);
                break;
            }
            case OP_LOAD_16_IM_OC:
            case OP_SWAP_16_IM_OC:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OC,
                                             instructionArgumentRegister),
                                 2);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OC,
                                           instructionArgumentRegister),
                               2);
                break;
            }
            case OP_LOAD_16_SIG_IM_ABS:
            case OP_SWAP_16_SIG_IM_ABS:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 instructionArgumentRegister,
                                 2);
                doSigExt(LOW_NIBBLE(regselArg1Register), false);
                SWAP_POST_LOAD(instructionArgumentRegister,
                               2);
                break;
            }
            case OP_LOAD_16_SIG_IM_PC:
            case OP_SWAP_16_SIG_IM_PC:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_PC,
                                             instructionArgumentRegister),
                                 2);
                doSigExt(LOW_NIBBLE(regselArg1Register), false);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_PC,
                                           instructionArgumentRegister),
                               2);
                break;
            }
            case OP_LOAD_16_SIG_IM_OA:
            case OP_SWAP_16_SIG_IM_OA:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OA,
                                             instructionArgumentRegister),
                                 2);
                doSigExt(LOW_NIBBLE(regselArg1Register), false);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OA,
                                           instructionArgumentRegister),
                               2);
                break;
            }
            case OP_LOAD_16_SIG_IM_OB:
            case OP_SWAP_16_SIG_IM_OB:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OB,
                                             instructionArgumentRegister),
                                 2);
                doSigExt(LOW_NIBBLE(regselArg1Register), false);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OB,
                                           instructionArgumentRegister),
                               2);
                break;
            }
            case OP_LOAD_16_SIG_IM_OC:
            case OP_SWAP_16_SIG_IM_OC:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OC,
                                             instructionArgumentRegister),
                                 2);
                doSigExt(LOW_NIBBLE(regselArg1Register), false);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OC,
                                           instructionArgumentRegister),
                               2);
                break;
            }
            case OP_LOAD_8_IM_ABS:
            case OP_SWAP_8_IM_ABS:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 instructionArgumentRegister,
                                 1);
                SWAP_POST_LOAD(instructionArgumentRegister,
                               1);
                break;
            }
            case OP_LOAD_8_IM_PC:
            case OP_SWAP_8_IM_PC:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_PC,
                                             instructionArgumentRegister),
                                 1);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_PC,
                                           instructionArgumentRegister),
                               1);
                break;
            }
            case OP_LOAD_8_IM_OA:
            case OP_SWAP_8_IM_OA:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OA,
                                             instructionArgumentRegister),
                                 1);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OA,
                                           instructionArgumentRegister),
                               1);
                break;
            }
            case OP_LOAD_8_IM_OB:
            case OP_SWAP_8_IM_OB:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OB,
                                             instructionArgumentRegister),
                                 1);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OB,
                                           instructionArgumentRegister),
                               1);
                break;
            }
            case OP_LOAD_8_IM_OC:
            case OP_SWAP_8_IM_OC:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OC,
                                             instructionArgumentRegister),
                                 1);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OC,
                                           instructionArgumentRegister),
                               1);
                break;
            }
            case OP_LOAD_8_SIG_IM_ABS:
            case OP_SWAP_8_SIG_IM_ABS:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 instructionArgumentRegister,
                                 1);
                doSigExt(LOW_NIBBLE(regselArg1Register), true);
                SWAP_POST_LOAD(instructionArgumentRegister,
                               1);
                break;
            }
            case OP_LOAD_8_SIG_IM_PC:
            case OP_SWAP_8_SIG_IM_PC:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_PC,
                                             instructionArgumentRegister),
                                 1);
                doSigExt(LOW_NIBBLE(regselArg1Register), true);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_PC,
                                           instructionArgumentRegister),
                               1);
                break;
            }
            case OP_LOAD_8_SIG_IM_OA:
            case OP_SWAP_8_SIG_IM_OA:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OA,
                                             instructionArgumentRegister),
                                 1);
                doSigExt(LOW_NIBBLE(regselArg1Register), true);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OA,
                                           instructionArgumentRegister),
                               1);
                break;
            }
            case OP_LOAD_8_SIG_IM_OB:
            case OP_SWAP_8_SIG_IM_OB:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OB,
                                             instructionArgumentRegister),
                                 1);
                doSigExt(LOW_NIBBLE(regselArg1Register), true);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OB,
                                           instructionArgumentRegister),
                               1);
                break;
            }
            case OP_LOAD_8_SIG_IM_OC:
            case OP_SWAP_8_SIG_IM_OC:
            {
                FETCH_FORM_2W;
                SWAP_PRE_LOAD;
                transferMemToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_OC,
                                             instructionArgumentRegister),
                                 1);
                doSigExt(LOW_NIBBLE(regselArg1Register), true);
                SWAP_POST_LOAD(applyOffset(ADDRESSING_MODE_OC,
                                           instructionArgumentRegister),
                               1);
                break;
            }
            case OP_STOR_32_REG_ABS:
            {
                FETCH_FORM_3;
                transferRegToMem(getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                                 LOW_NIBBLE(regselArg1Register),
                                 4);
                break;
            }
            case OP_STOR_32_REG_PC:
            {
                FETCH_FORM_3;
                transferRegToMem(applyOffset(ADDRESSING_MODE_PC,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 LOW_NIBBLE(regselArg1Register),
                                 4);
                break;
            }
            case OP_STOR_32_REG_OA:
            {
                FETCH_FORM_3;
                transferRegToMem(applyOffset(ADDRESSING_MODE_OA,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 LOW_NIBBLE(regselArg1Register),
                                 4);
                break;
            }
            case OP_STOR_32_REG_OB:
            {
                FETCH_FORM_3;
                transferRegToMem(applyOffset(ADDRESSING_MODE_OB,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 LOW_NIBBLE(regselArg1Register),
                                 4);
                break;
            }
            case OP_STOR_32_REG_OC:
            {
                FETCH_FORM_3;
                transferRegToMem(applyOffset(ADDRESSING_MODE_OC,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 LOW_NIBBLE(regselArg1Register),
                                 4);
                break;
            }
            case OP_STOR_16_REG_ABS:
            {
                FETCH_FORM_3;
                transferRegToMem(getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                                 LOW_NIBBLE(regselArg1Register),
                                 2);
                break;
            }
            case OP_STOR_16_REG_PC:
            {
                FETCH_FORM_3;
                transferRegToMem(applyOffset(ADDRESSING_MODE_PC,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 LOW_NIBBLE(regselArg1Register),
                                 2);
                break;
            }
            case OP_STOR_16_REG_OA:
            {
                FETCH_FORM_3;
                transferRegToMem(applyOffset(ADDRESSING_MODE_OA,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 LOW_NIBBLE(regselArg1Register),
                                 2);
                break;
            }
            case OP_STOR_16_REG_OB:
            {
                FETCH_FORM_3;
                transferRegToMem(applyOffset(ADDRESSING_MODE_OB,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 LOW_NIBBLE(regselArg1Register),
                                 2);
                break;
            }
            case OP_STOR_16_REG_OC:
            {
                FETCH_FORM_3;
                transferRegToMem(applyOffset(ADDRESSING_MODE_OC,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 LOW_NIBBLE(regselArg1Register),
                                 2);
                break;
            }
            case OP_STOR_8_REG_ABS:
            {
                FETCH_FORM_3;
                transferRegToMem(getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                                 LOW_NIBBLE(regselArg1Register),
                                 1);
                break;
            }
            case OP_STOR_8_REG_PC:
            {
                FETCH_FORM_3;
                transferRegToMem(applyOffset(ADDRESSING_MODE_PC,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 LOW_NIBBLE(regselArg1Register),
                                 1);
                break;
            }
            case OP_STOR_8_REG_OA:
            {
                FETCH_FORM_3;
                transferRegToMem(applyOffset(ADDRESSING_MODE_OA,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 LOW_NIBBLE(regselArg1Register),
                                 1);
                break;
            }
            case OP_STOR_8_REG_OB:
            {
                FETCH_FORM_3;
                transferRegToMem(applyOffset(ADDRESSING_MODE_OB,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 LOW_NIBBLE(regselArg1Register),
                                 1);
                break;
            }
            case OP_STOR_8_REG_OC:
            {
                FETCH_FORM_3;
                transferRegToMem(applyOffset(ADDRESSING_MODE_OC,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))),
                                 LOW_NIBBLE(regselArg1Register),
                                 1);
                break;
            }
            case OP_STOR_32_IM_ABS:
            {
                FETCH_FORM_2W;
                transferRegToMem(instructionArgumentRegister,
                                 LOW_NIBBLE(regselArg1Register),
                                 4);
                break;
            }
            case OP_STOR_32_IM_PC:
            {
                FETCH_FORM_2W;
                transferRegToMem(applyOffset(ADDRESSING_MODE_PC,
                                             instructionArgumentRegister),
                                 LOW_NIBBLE(regselArg1Register),
                                 4);
                break;
            }
            case OP_STOR_32_IM_OA:
            {
                FETCH_FORM_2W;
                transferRegToMem(applyOffset(ADDRESSING_MODE_OA,
                                             instructionArgumentRegister),
                                 LOW_NIBBLE(regselArg1Register),
                                 4);
                break;
            }
            case OP_STOR_32_IM_OB:
            {
                FETCH_FORM_2W;
                transferRegToMem(applyOffset(ADDRESSING_MODE_OB,
                                             instructionArgumentRegister),
                                 LOW_NIBBLE(regselArg1Register),
                                 4);
                break;
            }
            case OP_STOR_32_IM_OC:
            {
                FETCH_FORM_2W;
                transferRegToMem(applyOffset(ADDRESSING_MODE_OC,
                                             instructionArgumentRegister),
                                 LOW_NIBBLE(regselArg1Register),
                                 4);
                break;
            }
            case OP_STOR_16_IM_ABS:
            {
                FETCH_FORM_2W;
                transferRegToMem(instructionArgumentRegister,
                                 LOW_NIBBLE(regselArg1Register),
                                 2);
                break;
            }
            case OP_STOR_16_IM_PC:
            {
                FETCH_FORM_2W;
                transferRegToMem(applyOffset(ADDRESSING_MODE_PC,
                                             instructionArgumentRegister),
                                 LOW_NIBBLE(regselArg1Register),
                                 2);
                break;
            }
            case OP_STOR_16_IM_OA:
            {
                FETCH_FORM_2W;
                transferRegToMem(applyOffset(ADDRESSING_MODE_OA,
                                             instructionArgumentRegister),
                                 LOW_NIBBLE(regselArg1Register),
                                 2);
                break;
            }
            case OP_STOR_16_IM_OB:
            {
                FETCH_FORM_2W;
                transferRegToMem(applyOffset(ADDRESSING_MODE_OB,
                                             instructionArgumentRegister),
                                 LOW_NIBBLE(regselArg1Register),
                                 2);
                break;
            }
            case OP_STOR_16_IM_OC:
            {
                FETCH_FORM_2W;
                transferRegToMem(applyOffset(ADDRESSING_MODE_OC,
                                             instructionArgumentRegister),
                                 LOW_NIBBLE(regselArg1Register),
                                 2);
                break;
            }
            case OP_STOR_8_IM_ABS:
            {
                FETCH_FORM_2W;
                transferRegToMem(instructionArgumentRegister,
                                 LOW_NIBBLE(regselArg1Register),
                                 1);
                break;
            }
            case OP_STOR_8_IM_PC:
            {
                FETCH_FORM_2W;
                transferRegToMem(applyOffset(ADDRESSING_MODE_PC,
                                             instructionArgumentRegister),
                                 LOW_NIBBLE(regselArg1Register),
                                 1);
                break;
            }
            case OP_STOR_8_IM_OA:
            {
                FETCH_FORM_2W;
                transferRegToMem(applyOffset(ADDRESSING_MODE_OA,
                                             instructionArgumentRegister),
                                 LOW_NIBBLE(regselArg1Register),
                                 1);
                break;
            }
            case OP_STOR_8_IM_OB:
            {
                FETCH_FORM_2W;
                transferRegToMem(applyOffset(ADDRESSING_MODE_OB,
                                             instructionArgumentRegister),
                                 LOW_NIBBLE(regselArg1Register),
                                 1);
                break;
            }
            case OP_STOR_8_IM_OC:
            {
                FETCH_FORM_2W;
                transferRegToMem(applyOffset(ADDRESSING_MODE_OC,
                                             instructionArgumentRegister),
                                 LOW_NIBBLE(regselArg1Register),
                                 1);
                break;
            }
            case OP_ADD_INT_REG:
            {
                FETCH_FORM_4;
                doMath(MATH_OPERATION_ADD_INT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       getValFromRegsel(LOW_NIBBLE (regselArg2Register)));
                break;
            }
            case OP_ADD_INT_IM_32:
            {
                FETCH_FORM_3W;
                doMath(MATH_OPERATION_ADD_INT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_ADD_INT_IM_16:
            {
                FETCH_FORM_3H;
                doMath(MATH_OPERATION_ADD_INT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_ADD_INT_IM_16_SIG:
            {
                FETCH_FORM_3H;
                SIG_EXT_H(instructionArgumentRegister);
                doMath(MATH_OPERATION_ADD_INT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_ADD_INT_IM_8:
            {
                FETCH_FORM_3C;
                doMath(MATH_OPERATION_ADD_INT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_ADD_INT_IM_8_SIG:
            {
                FETCH_FORM_3C;
                SIG_EXT_C(instructionArgumentRegister);
                doMath(MATH_OPERATION_ADD_INT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_ADD_FL_REG:
            {
                FETCH_FORM_4;
                doMath(MATH_OPERATION_ADD_FL,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       getValFromRegsel(LOW_NIBBLE (regselArg2Register)));
                break;
            }
            case OP_ADD_FL_IM:
            {
                FETCH_FORM_4;
                doMath(MATH_OPERATION_ADD_FL,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_SUB_INT_REG:
            {
                FETCH_FORM_4;
                doMath(MATH_OPERATION_SUB_INT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       getValFromRegsel(LOW_NIBBLE (regselArg2Register)));
                break;
            }
            case OP_SUB_INT_IM_32:
            {
                FETCH_FORM_3W;
                doMath(MATH_OPERATION_SUB_INT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_SUB_INT_IM_16:
            {
                FETCH_FORM_3H;
                doMath(MATH_OPERATION_SUB_INT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_SUB_INT_IM_16_SIG:
            {
                FETCH_FORM_3H;
                SIG_EXT_H(instructionArgumentRegister);
                doMath(MATH_OPERATION_SUB_INT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_SUB_INT_IM_8:
            {
                FETCH_FORM_3C;
                doMath(MATH_OPERATION_SUB_INT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_SUB_INT_IM_8_SIG:
            {
                FETCH_FORM_3C;
                SIG_EXT_C(instructionArgumentRegister);
                doMath(MATH_OPERATION_SUB_INT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_SUB_FL_REG:
            {
                FETCH_FORM_4;
                doMath(MATH_OPERATION_SUB_FL,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       getValFromRegsel(LOW_NIBBLE (regselArg2Register)));
                break;
            }
            case OP_SUB_FL_IM:
            {
                FETCH_FORM_4;
                doMath(MATH_OPERATION_SUB_FL,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_MUL_INT_REG:
            {
                FETCH_FORM_4;
                doMath(MATH_OPERATION_MUL_INT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       getValFromRegsel(LOW_NIBBLE (regselArg2Register)));
                break;
            }
            case OP_MUL_INT_IM_32:
            {
                FETCH_FORM_3W;
                doMath(MATH_OPERATION_MUL_INT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_MUL_INT_IM_16:
            {
                FETCH_FORM_3H;
                doMath(MATH_OPERATION_MUL_INT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_MUL_INT_IM_16_SIG:
            {
                FETCH_FORM_3H;
                SIG_EXT_H(instructionArgumentRegister);
                doMath(MATH_OPERATION_MUL_INT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_MUL_INT_IM_8:
            {
                FETCH_FORM_3C;
                doMath(MATH_OPERATION_MUL_INT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_MUL_INT_IM_8_SIG:
            {
                FETCH_FORM_3C;
                SIG_EXT_C(instructionArgumentRegister);
                doMath(MATH_OPERATION_MUL_INT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_MUL_FL_REG:
            {
                FETCH_FORM_4;
                doMath(MATH_OPERATION_MUL_FL,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       getValFromRegsel(LOW_NIBBLE (regselArg2Register)));
                break;
            }
            case OP_MUL_FL_IM:
            {
                FETCH_FORM_4;
                doMath(MATH_OPERATION_MUL_FL,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_DIV_INT_REG:
            {
                FETCH_FORM_4;
                doMath(MATH_OPERATION_DIV_INT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       getValFromRegsel(LOW_NIBBLE (regselArg2Register)));
                break;
            }
            case OP_DIV_INT_IM_32:
            {
                FETCH_FORM_3W;
                doMath(MATH_OPERATION_DIV_INT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_DIV_INT_IM_16:
            {
                FETCH_FORM_3H;
                doMath(MATH_OPERATION_DIV_INT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_DIV_INT_IM_16_SIG:
            {
                FETCH_FORM_3H;
                SIG_EXT_H(instructionArgumentRegister);
                doMath(MATH_OPERATION_DIV_INT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_DIV_INT_IM_8:
            {
                FETCH_FORM_3C;
                doMath(MATH_OPERATION_DIV_INT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_DIV_INT_IM_8_SIG:
            {
                FETCH_FORM_3C;
                SIG_EXT_C(instructionArgumentRegister);
                doMath(MATH_OPERATION_DIV_INT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_DIV_FL_REG:
            {
                FETCH_FORM_4;
                doMath(MATH_OPERATION_DIV_FL,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       getValFromRegsel(LOW_NIBBLE (regselArg2Register)));
                break;
            }
            case OP_DIV_FL_IM:
            {
                FETCH_FORM_4;
                doMath(MATH_OPERATION_DIV_FL,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_MOD_INT_REG:
            {
                FETCH_FORM_4;
                doMath(MATH_OPERATION_MOD,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       getValFromRegsel(LOW_NIBBLE (regselArg2Register)));
                break;
            }
            case OP_MOD_INT_IM_32:
            {
                FETCH_FORM_3W;
                doMath(MATH_OPERATION_MOD,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_MOD_INT_IM_16:
            {
                FETCH_FORM_3H;
                doMath(MATH_OPERATION_MOD,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_MOD_INT_IM_16_SIG:
            {
                FETCH_FORM_3H;
                SIG_EXT_H(instructionArgumentRegister);
                doMath(MATH_OPERATION_MOD,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_MOD_INT_IM_8:
            {
                FETCH_FORM_3C;
                doMath(MATH_OPERATION_MOD,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_MOD_INT_IM_8_SIG:
            {
                FETCH_FORM_3C;
                SIG_EXT_C(instructionArgumentRegister);
                doMath(MATH_OPERATION_MOD,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_AND_REG:
            {
                FETCH_FORM_4;
                doMath(MATH_OPERATION_AND,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       getValFromRegsel(LOW_NIBBLE (regselArg2Register)));
                break;
            }
            case OP_AND_IM_32:
            {
                FETCH_FORM_3W;
                doMath(MATH_OPERATION_AND,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_AND_IM_16:
            {
                FETCH_FORM_3H;
                doMath(MATH_OPERATION_AND,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_AND_IM_8:
            {
                FETCH_FORM_3C;
                doMath(MATH_OPERATION_AND,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_OR_REG:
            {
                FETCH_FORM_4;
                doMath(MATH_OPERATION_OR,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       getValFromRegsel(LOW_NIBBLE (regselArg2Register)));
                break;
            }
            case OP_OR_IM_32:
            {
                FETCH_FORM_3W;
                doMath(MATH_OPERATION_OR,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_OR_IM_16:
            {
                FETCH_FORM_3H;
                doMath(MATH_OPERATION_OR,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_OR_IM_8:
            {
                FETCH_FORM_3C;
                doMath(MATH_OPERATION_OR,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_XOR_REG:
            {
                FETCH_FORM_4;
                doMath(MATH_OPERATION_XOR,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       getValFromRegsel(LOW_NIBBLE (regselArg2Register)));
                break;
            }
            case OP_XOR_IM_32:
            {
                FETCH_FORM_3W;
                doMath(MATH_OPERATION_XOR,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_XOR_IM_16:
            {
                FETCH_FORM_3H;
                doMath(MATH_OPERATION_XOR,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_XOR_IM_8:
            {
                FETCH_FORM_3C;
                doMath(MATH_OPERATION_XOR,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_BSLT_REG:
            {
                FETCH_FORM_4;
                doMath(MATH_OPERATION_BSLT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       getValFromRegsel(LOW_NIBBLE (regselArg2Register)));
                break;
            }
            case OP_BSLT_IM_8:
            {
                FETCH_FORM_3C;
                doMath(MATH_OPERATION_BSLT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_BSLC_REG:
            {
                FETCH_FORM_4;
                doMath(MATH_OPERATION_BSLC,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       getValFromRegsel(LOW_NIBBLE (regselArg2Register)));
                break;
            }
            case OP_BSLC_IM_8:
            {
                FETCH_FORM_3C;
                doMath(MATH_OPERATION_BSLC,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_BSRT_REG:
            {
                FETCH_FORM_4;
                doMath(MATH_OPERATION_BSRT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       getValFromRegsel(LOW_NIBBLE (regselArg2Register)));
                break;
            }
            case OP_BSRT_IM_8:
            {
                FETCH_FORM_3C;
                doMath(MATH_OPERATION_BSRT,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_BSRC_REG:
            {
                FETCH_FORM_4;
                doMath(MATH_OPERATION_BSRC,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       getValFromRegsel(LOW_NIBBLE (regselArg2Register)));
                break;
            }
            case OP_BSRC_IM_8:
            {
                FETCH_FORM_3C;
                doMath(MATH_OPERATION_BSRC,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       instructionArgumentRegister);
                break;
            }
            case OP_NOT_REG:
            {
                FETCH_FORM_3;
                doMath(MATH_OPERATION_BSRC,
                       LOW_NIBBLE (regselArg1Register),
                       getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                       0);
                break;
            }
            case OP_COMP_INT_REG:
            {
                FETCH_FORM_3;
                doCompare(true,
                          getValFromRegsel(LOW_NIBBLE (regselArg1Register)),
                          getValFromRegsel(HIGH_NIBBLE(regselArg1Register)));
                break;
            }
            case OP_COMP_INT_IM_32:
            {
                FETCH_FORM_2W;
                doCompare(true,
                          getValFromRegsel(LOW_NIBBLE (regselArg1Register)),
                          instructionArgumentRegister);
                break;
            }
            case OP_COMP_INT_IM_16:
            {
                FETCH_FORM_2H;
                doCompare(true,
                          getValFromRegsel(LOW_NIBBLE (regselArg1Register)),
                          instructionArgumentRegister);
                break;
            }
            case OP_COMP_INT_IM_16_SIG:
            {
                FETCH_FORM_2H;
                SIG_EXT_H(instructionArgumentRegister);
                doCompare(true,
                          getValFromRegsel(LOW_NIBBLE (regselArg1Register)),
                          instructionArgumentRegister);
                break;
            }
            case OP_COMP_INT_IM_8:
            {
                FETCH_FORM_2C;
                doCompare(true,
                          getValFromRegsel(LOW_NIBBLE (regselArg1Register)),
                          instructionArgumentRegister);
                break;
            }
            case OP_COMP_INT_IM_8_SIG:
            {
                FETCH_FORM_2C;
                SIG_EXT_C(instructionArgumentRegister);
                doCompare(true,
                          getValFromRegsel(LOW_NIBBLE (regselArg1Register)),
                          instructionArgumentRegister);
                break;
            }
            case OP_COMP_FL_REG:
            {
                FETCH_FORM_3;
                doCompare(false,
                          getValFromRegsel(LOW_NIBBLE (regselArg1Register)),
                          getValFromRegsel(HIGH_NIBBLE(regselArg1Register)));
                break;
            }
            case OP_COMP_FL_IM:
            {
                FETCH_FORM_2W;
                doCompare(false,
                          getValFromRegsel(LOW_NIBBLE (regselArg1Register)),
                          instructionArgumentRegister);
                break;
            }
            case OP_EVAL_INT_REG:
            {
                FETCH_FORM_8;
                doCompare(true,
                          getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                          getValFromRegsel(LOW_NIBBLE (regselArg2Register)));
                transferVarToReg(LOW_NIBBLE(regselArg1Register),
                                 checkCondition(HIGH_NIBBLE(regselArg2Register)));
                break;
            }
            case OP_EVAL_INT_IM_32:
            {
                FETCH_FORM_7W;
                doCompare(true,
                          getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                          instructionArgumentRegister);
                transferVarToReg(LOW_NIBBLE(regselArg1Register),
                                 checkCondition(LOW_NIBBLE(regselArg2Register)));
                break;
            }
            case OP_EVAL_INT_IM_16:
            {
                FETCH_FORM_7H;
                doCompare(true,
                          getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                          instructionArgumentRegister);
                transferVarToReg(LOW_NIBBLE(regselArg1Register),
                                 checkCondition(LOW_NIBBLE(regselArg2Register)));
                break;
            }
            case OP_EVAL_INT_IM_16_SIG:
            {
                FETCH_FORM_7H;
                SIG_EXT_H(instructionArgumentRegister);
                doCompare(true,
                          getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                          instructionArgumentRegister);
                transferVarToReg(LOW_NIBBLE(regselArg1Register),
                                 checkCondition(LOW_NIBBLE(regselArg2Register)));
                break;
            }
            case OP_EVAL_INT_IM_8:
            {
                FETCH_FORM_7C;
                doCompare(true,
                          getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                          instructionArgumentRegister);
                transferVarToReg(LOW_NIBBLE(regselArg1Register),
                                 checkCondition(LOW_NIBBLE(regselArg2Register)));
                break;
            }
            case OP_EVAL_INT_IM_8_SIG:
            {
                FETCH_FORM_7C;
                SIG_EXT_C(instructionArgumentRegister);
                doCompare(true,
                          getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                          instructionArgumentRegister);
                transferVarToReg(LOW_NIBBLE(regselArg1Register),
                                 checkCondition(LOW_NIBBLE(regselArg2Register)));
                break;
            }
            case OP_EVAL_FL_REG:
            {
                FETCH_FORM_8;
                doCompare(false,
                          getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                          getValFromRegsel(LOW_NIBBLE (regselArg2Register)));
                transferVarToReg(LOW_NIBBLE(regselArg1Register),
                                 checkCondition(HIGH_NIBBLE(regselArg2Register)));
                break;
            }
            case OP_EVAL_FL_IM:
            {
                FETCH_FORM_7W;
                doCompare(false,
                          getValFromRegsel(HIGH_NIBBLE(regselArg1Register)),
                          instructionArgumentRegister);
                transferVarToReg(LOW_NIBBLE(regselArg1Register),
                                 checkCondition(LOW_NIBBLE(regselArg2Register)));
                break;
            }
            case OP_BRNC_REG_ABS:
            {
                FETCH_FORM_6;
                if (checkCondition(HIGH_NIBBLE(regselArg1Register)))
                {
                    programCounter = getValFromRegsel(LOW_NIBBLE(regselArg1Register));
                }
                break;
            }
            case OP_BRNC_REG_PC:
            {
                FETCH_FORM_6;
                if (checkCondition(HIGH_NIBBLE(regselArg1Register)))
                {
                    programCounter = getValFromRegsel(
                                         applyOffset(ADDRESSING_MODE_PC,
                                                    LOW_NIBBLE(regselArg1Register)));
                }
                break;
            }
            case OP_BRNC_REG_OA:
            {
                FETCH_FORM_6;
                if (checkCondition(HIGH_NIBBLE(regselArg1Register)))
                {
                    programCounter = getValFromRegsel(
                                         applyOffset(ADDRESSING_MODE_OA,
                                                    LOW_NIBBLE(regselArg1Register)));
                }
                break;
            }
            case OP_BRNC_REG_OB:
            {
                FETCH_FORM_6;
                if (checkCondition(HIGH_NIBBLE(regselArg1Register)))
                {
                    programCounter = getValFromRegsel(
                                         applyOffset(ADDRESSING_MODE_OB,
                                                    LOW_NIBBLE(regselArg1Register)));
                }
                break;
            }
            case OP_BRNC_REG_OC:
            {
                FETCH_FORM_6;
                if (checkCondition(HIGH_NIBBLE(regselArg1Register)))
                {
                    programCounter = getValFromRegsel(
                                         applyOffset(ADDRESSING_MODE_OC,
                                                    LOW_NIBBLE(regselArg1Register)));
                }
                break;
            }
            case OP_BRNC_P_REG_ABS:
            {
                FETCH_FORM_6;
                if (checkCondition(HIGH_NIBBLE(regselArg1Register)))
                {
                    protectedPush32(programCounter);
                    programCounter = getValFromRegsel(LOW_NIBBLE(regselArg1Register));
                }
                break;
            }
            case OP_BRNC_P_REG_PC:
            {
                FETCH_FORM_6;
                if (checkCondition(HIGH_NIBBLE(regselArg1Register)))
                {
                    protectedPush32(programCounter);
                    programCounter = getValFromRegsel(
                                         applyOffset(ADDRESSING_MODE_PC,
                                                    LOW_NIBBLE(regselArg1Register)));
                }
                break;
            }
            case OP_BRNC_P_REG_OA:
            {
                FETCH_FORM_6;
                if (checkCondition(HIGH_NIBBLE(regselArg1Register)))
                {
                    protectedPush32(programCounter);
                    programCounter = getValFromRegsel(
                                         applyOffset(ADDRESSING_MODE_OA,
                                                    LOW_NIBBLE(regselArg1Register)));
                }
                break;
            }
            case OP_BRNC_P_REG_OB:
            {
                FETCH_FORM_6;
                if (checkCondition(HIGH_NIBBLE(regselArg1Register)))
                {
                    protectedPush32(programCounter);
                    programCounter = getValFromRegsel(
                                         applyOffset(ADDRESSING_MODE_OB,
                                                    LOW_NIBBLE(regselArg1Register)));
                }
                break;
            }
            case OP_BRNC_P_REG_OC:
            {
                FETCH_FORM_6;
                if (checkCondition(HIGH_NIBBLE(regselArg1Register)))
                {
                    protectedPush32(programCounter);
                    programCounter = getValFromRegsel(
                                         applyOffset(ADDRESSING_MODE_OC,
                                                    LOW_NIBBLE(regselArg1Register)));
                }
                break;
            }
            case OP_BRNC_IM_ABS:
            {
                FETCH_FORM_5W;
                if (checkCondition(LOW_NIBBLE(regselArg1Register)))
                {
                    programCounter = instructionArgumentRegister;
                }
                break;
            }
            case OP_BRNC_IM_PC:
            {
                FETCH_FORM_5W;
                if (checkCondition(LOW_NIBBLE(regselArg1Register)))
                {
                    programCounter = applyOffset(ADDRESSING_MODE_PC,
                                                 instructionArgumentRegister);
                }
                break;
            }
            case OP_BRNC_IM_OA:
            {
                FETCH_FORM_5W;
                if (checkCondition(LOW_NIBBLE(regselArg1Register)))
                {
                    programCounter = applyOffset(ADDRESSING_MODE_OA,
                                                 instructionArgumentRegister);
                }
                break;
            }
            case OP_BRNC_IM_OB:
            {
                FETCH_FORM_5W;
                if (checkCondition(LOW_NIBBLE(regselArg1Register)))
                {
                    programCounter = applyOffset(ADDRESSING_MODE_OB,
                                                 instructionArgumentRegister);
                }
                break;
            }
            case OP_BRNC_IM_OC:
            {
                FETCH_FORM_5W;
                if (checkCondition(LOW_NIBBLE(regselArg1Register)))
                {
                    programCounter = applyOffset(ADDRESSING_MODE_OC,
                                                 instructionArgumentRegister);
                }
                break;
            }
            case OP_BRNC_P_IM_ABS:
            {
                FETCH_FORM_5W;
                if (checkCondition(LOW_NIBBLE(regselArg1Register)))
                {
                    protectedPush32(programCounter);
                    programCounter = instructionArgumentRegister;
                }
                break;
            }
            case OP_BRNC_P_IM_PC:
            {
                FETCH_FORM_5W;
                if (checkCondition(LOW_NIBBLE(regselArg1Register)))
                {
                    protectedPush32(programCounter);
                    programCounter = applyOffset(ADDRESSING_MODE_PC,
                                                 instructionArgumentRegister);
                }
                break;
            }
            case OP_BRNC_P_IM_OA:
            {
                FETCH_FORM_5W;
                if (checkCondition(LOW_NIBBLE(regselArg1Register)))
                {
                    protectedPush32(programCounter);
                    programCounter = applyOffset(ADDRESSING_MODE_OA,
                                                 instructionArgumentRegister);
                }
                break;
            }
            case OP_BRNC_P_IM_OB:
            {
                FETCH_FORM_5W;
                if (checkCondition(LOW_NIBBLE(regselArg1Register)))
                {
                    protectedPush32(programCounter);
                    programCounter = applyOffset(ADDRESSING_MODE_OB,
                                                 instructionArgumentRegister);
                }
                break;
            }
            case OP_BRNC_P_IM_OC:
            {
                FETCH_FORM_5W;
                if (checkCondition(LOW_NIBBLE(regselArg1Register)))
                {
                    protectedPush32(programCounter);
                    programCounter = applyOffset(ADDRESSING_MODE_OC,
                                                 instructionArgumentRegister);
                }
                break;
            }
            case OP_FL_TO_INT:
            {
                FETCH_FORM_3;
                int32_t tmpInt = getValFromRegsel(HIGH_NIBBLE(regselArg1Register));

                float tmpFloat = *((float*)&tmpInt);
                tmpInt = (int32_t) tmpFloat;

                transferVarToReg(LOW_NIBBLE(regselArg1Register), tmpInt);

                break;
            }
            case OP_INT_TO_FL:
            {
                FETCH_FORM_3;
                float tmpFloat = getValFromRegsel(HIGH_NIBBLE(regselArg1Register));

                uint32_t tmpInt = *((uint32_t*) &tmpFloat);

                transferVarToReg(LOW_NIBBLE(regselArg1Register), tmpInt);

                break;
            }
            case OP_INT_SIG_TO_FL:
            {
                FETCH_FORM_3;
                uint32_t tmpInt = getValFromRegsel(HIGH_NIBBLE(regselArg1Register));
                int32_t  tmpSigInt = *((int32_t*)&tmpInt);

                float    tmpFloat = tmpSigInt;

                tmpInt = *((uint32_t*) &tmpFloat);

                transferVarToReg(LOW_NIBBLE(regselArg1Register), tmpInt);

                break;
            }
            case OP_PUSH:
            {
                FETCH_FORM_2;
                doStackUtils(STACK_UTIL_PUSH, LOW_NIBBLE(regselArg1Register));
                break;
            }
            case OP_POP:
            {
                FETCH_FORM_2;
                doStackUtils(STACK_UTIL_POP, LOW_NIBBLE(regselArg1Register));
                break;
            }
            case OP_PEEK:
            {
                doStackUtils(STACK_UTIL_POP, LOW_NIBBLE(regselArg1Register));
                break;
            }
            case OP_RETURN:
            {
                // Fetch for form 1 already done
                doStackUtils(STACK_UTIL_RETURN, 0);
                break;
            }
            case OP_INTR_SUS:
            {
                // Fetch for form 1 already done
                statusRegister |= STAT_REG_INT_SUS_MASK;
                break;
            }
            case OP_INTR_RES:
            {
                // Fetch for form 1 already done
                statusRegister &= ~(STAT_REG_INT_IN_PROG_MASK |
                                    STAT_REG_INT_SUS_MASK);
                break;
            }
            case OP_INTR_FIN:
            {
                // Fetch for form 1 already done
                programCounter = interruptReturnAddress;
                statusRegister &= (~STAT_REG_INT_IN_PROG_MASK);
                break;
            }
            case OP_INTR_TGR_REG:
            {
                FETCH_FORM_2;
                enqueueInterrupt(getValFromRegsel(LOW_NIBBLE(regselArg1Register))
                                 & INTERRUPT_FULL_MASK);
                break;
            }
            case OP_INTR_TGR_IM_16:
            {
                FETCH_FORM_1H;
                enqueueInterrupt(instructionArgumentRegister & INTERRUPT_FULL_MASK);
                break;
            }
            case OP_INTR_GET_PARAM:
            {
                FETCH_FORM_2;
                transferVarToReg(LOW_NIBBLE(regselArg1Register),
                                (currentInterrupt & INTERRUPT_ARG_MASK) >> INTERRUPT_ARG_OFFSET);
                break;
            }
            case OP_INTR_GET_RET_AD:
            {
                FETCH_FORM_2;
                transferVarToReg(LOW_NIBBLE(regselArg1Register), interruptReturnAddress);
                break;
            }
            case OP_INTR_SET_RET_AD_REG:
            {
                FETCH_FORM_2;
                interruptReturnAddress = getValFromRegsel(LOW_NIBBLE(regselArg1Register));
                break;
            }
            case OP_INTR_SET_RET_AD_IM:
            {
                FETCH_FORM_1W;
                interruptReturnAddress = instructionArgumentRegister;
                break;
            }
            case OP_GETABS_REG_PC:
            {
                FETCH_FORM_3;
                transferVarToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_PC,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))));
                break;
            }
            case OP_GETABS_IM_PC:
            {
                FETCH_FORM_2W;
                transferVarToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_PC,
                                             instructionArgumentRegister));
                break;
            }
            case OP_GETREL_REG_OA:
            {
                FETCH_FORM_3;
                transferVarToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_PC_M_OA,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))));
                break;
            }
            case OP_GETREL_REG_OB:
            {
                FETCH_FORM_3;
                transferVarToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_PC_M_OB,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))));
                break;
            }
            case OP_GETREL_REG_OC:
            {
                FETCH_FORM_3;
                transferVarToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_PC_M_OC,
                                             getValFromRegsel(HIGH_NIBBLE(regselArg1Register))));
                break;
            }
            case OP_GETREL_IM_OA:
            {
                FETCH_FORM_2W;
                transferVarToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_PC_M_OA,
                                             instructionArgumentRegister));
                break;
            }
            case OP_GETREL_IM_OB:
            {
                FETCH_FORM_2W;
                transferVarToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_PC_M_OB,
                                             instructionArgumentRegister));
                break;
            }
            case OP_GETREL_IM_OC:
            {
                FETCH_FORM_2W;
                transferVarToReg(LOW_NIBBLE(regselArg1Register),
                                 applyOffset(ADDRESSING_MODE_PC_M_OC,
                                             instructionArgumentRegister));
                break;
            }
        }

        #ifdef SELF_TEST

        st_startInterruptTime();

        if (false == st_checkFrame(generalRegisters,
                                   offsetRegisters,
                                   stackBase,
                                   stackSize,
                                   stackPointer,
                                   interruptTable,
                                   flagsRegister,
                                   programCounter,
                                   opCodeRegister,
                                   regselArg1Register,
                                   regselArg2Register,
                                   instructionArgumentRegister,
                                   interruptReturnAddress,
                                   currentInterrupt,
                                   interruptHead,
                                   interruptTail,
                                   statusRegister,
                                   memory))
        {
            mb_abort();
            return 0;
        }

        st_endInterruptTime();
        #endif // SELF_TEST
    }

    return 0;
}

static void run()
{
    thrd_t cpuThread = {0};
    thrd_create(&cpuThread, cpuThreadFunction, NULL);

    // This can only exit from the power state turning
    // off, the cpu thread will then exit
    mb_mainThread();
    thrd_join(cpuThread, NULL);
}

static void teardown()
{
    #ifdef SELF_TEST
    st_exit();
    #endif // SELF_TEST

    mb_teardown();

    if (NULL != memory)
    {
        free(memory);
        memory = NULL;
    }

    if (NULL != glfwInfo.window)
    {
        glfwDestroyWindow(glfwInfo.window);
    }

    glfwTerminate();
}

int main(int argc, char* argv[])
{
    printf("DGC-32\n");

    #ifdef SELF_TEST
    printf("Self test mode\n");
    #endif // SELF_TEST

    if (false == init(argc, argv))
    {
        teardown();
        return -1;
    }

    run();

    teardown();

    return 0;
}