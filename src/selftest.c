#include "selftest.h"

static FILE    *testFile       = NULL;
static uint32_t nextFrameCheck = 0;
static uint32_t frame          = 0;

bool st_checkFrame(uint32_t generalRegisters[8],
                   uint32_t offsetRegisters[3],
                   uint32_t stackBase,
                   uint16_t stackSize,
                   uint32_t interruptTable,
                   uint32_t programCounter,
                   uint32_t instructionRegister,
                   uint8_t  instructionAugment,
                   uint8_t  interruptHead,
                   uint8_t  interruptTail,
                   uint16_t stackPointer,
                   uint8_t  flagsRegister,
                   uint8_t *memory)
{
    char     buffer[2048] = {0};
    uint32_t buf32        = 0;
    uint16_t buf16        = 0;
    uint8_t  buf8         = 0;
    bool     rc           = true;

    printf("Frame %u\n", ++frame);

    if (frame != nextFrameCheck)
    {
        return true;
    }

    printf("Evaluating...\n");

    while (fgets(buffer, sizeof(buffer), testFile))
    {
        if (0 != sscanf(buffer, ST_FRAME_STRING, &nextFrameCheck))
        {
            return rc;
        }
        else if (0 != sscanf(buffer, ST_G0_STRING, &buf32))
        {
            if (generalRegisters[0] != buf32)
            {
                printf("G0 check failed in frame %u. Expected: 0x%08x, got 0x%08x\n", frame, buf32, generalRegisters[0]);
                rc = false;
            }
        }
        else if (0 != sscanf(buffer, ST_G1_STRING, &buf32))
        {
            if (generalRegisters[1] != buf32)
            {
                printf("G1 check failed in frame %u. Expected: 0x%08x, got 0x%08x\n", frame, buf32, generalRegisters[1]);
                rc = false;
            }
        }
        else if (0 != sscanf(buffer, ST_G2_STRING, &buf32))
        {
            if (generalRegisters[2] != buf32)
            {
                printf("G2 check failed in frame %u. Expected: 0x%08x, got 0x%08x\n", frame, buf32, generalRegisters[2]);
                rc = false;
            }
        }
        else if (0 != sscanf(buffer, ST_G3_STRING, &buf32))
        {
            if (generalRegisters[3] != buf32)
            {
                printf("G3 check failed in frame %u. Expected: 0x%08x, got 0x%08x\n", frame, buf32, generalRegisters[3]);
                rc = false;
            }
        }
        else if (0 != sscanf(buffer, ST_G4_STRING, &buf32))
        {
            if (generalRegisters[4] != buf32)
            {
                printf("G4 check failed in frame %u. Expected: 0x%08x, got 0x%08x\n", frame, buf32, generalRegisters[4]);
                rc = false;
            }
        }
        else if (0 != sscanf(buffer, ST_G5_STRING, &buf32))
        {
            if (generalRegisters[5] != buf32)
            {
                printf("G5 check failed in frame %u. Expected: 0x%08x, got 0x%08x\n", frame, buf32, generalRegisters[5]);
                rc = false;
            }
        }
        else if (0 != sscanf(buffer, ST_G6_STRING, &buf32))
        {
            if (generalRegisters[6] != buf32)
            {
                printf("G6 check failed in frame %u. Expected: 0x%08x, got 0x%08x\n", frame, buf32, generalRegisters[6]);
                rc = false;
            }
        }
        else if (0 != sscanf(buffer, ST_G7_STRING, &buf32))
        {
            if (generalRegisters[7] != buf32)
            {
                printf("G7 check failed in frame %u. Expected: 0x%08x, got 0x%08x\n", frame, buf32, generalRegisters[7]);
                rc = false;
            }
        }
        else if (0 != sscanf(buffer, ST_OA_STRING, &buf32))
        {
            if (offsetRegisters[0] != buf32)
            {
                printf("OA check failed in frame %u. Expected: 0x%08x, got 0x%08x\n", frame, buf32, offsetRegisters[0]);
                rc = false;
            }
        }
        else if (0 != sscanf(buffer, ST_OB_STRING, &buf32))
        {
            if (offsetRegisters[1] != buf32)
            {
                printf("OB check failed in frame %u. Expected: 0x%08x, got 0x%08x\n", frame, buf32, offsetRegisters[1]);
                rc = false;
            }
        }
        else if (0 != sscanf(buffer, ST_OC_STRING, &buf32))
        {
            if (offsetRegisters[2] != buf32)
            {
                printf("OC check failed in frame %u. Expected: 0x%08x, got 0x%08x\n", frame, buf32, offsetRegisters[2]);
                rc = false;
            }
        }
        else if (0 != sscanf(buffer, ST_SB_STRING, &buf32))
        {
            if (stackBase != buf32)
            {
                printf("SB check failed in frame %u. Expected: 0x%08x, got 0x%08x\n", frame, buf32, stackBase);
                rc = false;
            }
        }
        else if (0 != sscanf(buffer, ST_SS_STRING, &buf16))
        {
            if (stackSize != buf16)
            {
                printf("SS check failed in frame %u. Expected: 0x%08hx, got 0x%08hx\n", frame, buf16, stackSize);
                rc = false;
            }
        }
        else if (0 != sscanf(buffer, ST_IL_STRING, &buf32))
        {
            if (interruptTable != buf32)
            {
                printf("IL check failed in frame %u. Expected: 0x%08x, got 0x%08x\n", frame, buf32, interruptTable);
                rc = false;
            }
        }
        else if (0 != sscanf(buffer, ST_PC_STRING, &buf32))
        {
            if (programCounter != buf32)
            {
                printf("PC check failed in frame %u. Expected: 0x%08x, got 0x%08x\n", frame, buf32, programCounter);
                rc = false;
            }
        }
        else if (0 != sscanf(buffer, ST_IR_STRING, &buf32))
        {
            if (instructionRegister != buf32)
            {
                printf("IR check failed in frame %u. Expected: 0x%08x, got 0x%08x\n", frame, buf32, instructionRegister);
                rc = false;
            }
        }
        else if (0 != sscanf(buffer, ST_IA_STRING, &buf8))
        {
            if (instructionAugment != buf8)
            {
                printf("IA check failed in frame %u. Expected: 0x%08hhx, got 0x%08hhx\n", frame, buf8, instructionAugment);
                rc = false;
            }
        }
        else if (0 != sscanf(buffer, ST_IH_STRING, &buf8))
        {
            if (interruptHead != buf8)
            {
                printf("IH check failed in frame %u. Expected: 0x%08hhx, got 0x%08hhx\n", frame, buf8, interruptHead);
                rc = false;
            }
        }
        else if (0 != sscanf(buffer, ST_IT_STRING, &buf8))
        {
            if (interruptTail != buf8)
            {
                printf("IT check failed in frame %u. Expected: 0x%08hhx, got 0x%08hhx\n", frame, buf8, interruptTail);
                rc = false;
            }
        }
        else if (0 != sscanf(buffer, ST_SP_STRING, &buf16))
        {
            if (stackPointer != buf16)
            {
                printf("SP check failed in frame %u. Expected: 0x%08hx, got 0x%08hx\n", frame, buf16, stackSize);
                rc = false;
            }
        }
        else if (0 != sscanf(buffer, ST_FL_STRING, &buf8))
        {
            if (flagsRegister != buf8)
            {
                printf("FL check failed in frame %u. Expected: 0x%08hhx, got 0x%08hhx\n", frame, buf8, flagsRegister);
                rc = false;
            }
        }
        else if (0 != sscanf(buffer, ST_MEM_STRING, &buf32, &buf8))
        {
            if (memory[buf32] != buf8)
            {
                printf("MEM check at 0x%08x failed in frame %u. Expected: 0x%08hhx, got 0x%08hhx\n", buf32, frame, buf8, memory[buf32]);
                rc = false;
            }
        }
    }

    return false;
}

bool st_setTestFile(char *arg)
{
    char inputBuffer[2048] = {0};

    testFile = fopen(arg, "r");

    if (NULL == testFile)
    {
        printf("Self test error: failed to open test file \"%s\"\n", arg);
        return false;
    }

    while (fgets(inputBuffer, sizeof(inputBuffer), testFile))
    {
        if (0 != sscanf(inputBuffer, ST_FRAME_STRING, &nextFrameCheck))
        {
            return true;
        }
    }

    printf("Self test error: could not find frame marker in test file\n");

    return false;
}

void st_exit()
{
    if (NULL != testFile)
    {
        fclose(testFile);
        testFile = NULL;
    }
}