#ifndef __SELFTEST_H__
#define __SELFTEST_H__

#include <time.h>
#include "dgc32.h"

#define SEC_TO_NSEC 1000000000

#define ST_FRAME_STRING "frame: %u"
#define ST_G0_STRING    "G0: 0x%x"
#define ST_G1_STRING    "G1: 0x%x"
#define ST_G2_STRING    "G2: 0x%x"
#define ST_G3_STRING    "G3: 0x%x"
#define ST_G4_STRING    "G4: 0x%x"
#define ST_G5_STRING    "G5: 0x%x"
#define ST_G6_STRING    "G6: 0x%x"
#define ST_G7_STRING    "G7: 0x%x"
#define ST_OA_STRING    "OA: 0x%x"
#define ST_OB_STRING    "OB: 0x%x"
#define ST_OC_STRING    "OC: 0x%x"
#define ST_SB_STRING    "SB: 0x%x"
#define ST_SS_STRING    "SS: 0x%hx"
#define ST_IL_STRING    "IL: 0x%x"
#define ST_PC_STRING    "PC: 0x%x"
#define ST_IR_STRING    "IR: 0x%x"
#define ST_IA_STRING    "IA: 0x%hhx"
#define ST_IH_STRING    "IH: 0x%hhx"
#define ST_IT_STRING    "IT: 0x%hhx"
#define ST_SP_STRING    "SP: 0x%hx"
#define ST_FL_STRING    "FL: 0b%hhb"
#define ST_CI_STRING    "CI: 0x%hx"
#define ST_MEM_STRING   "MEM: 0x%x 0x%hhx"

void st_defineStartTime();
void st_startInterruptTime();
void st_endInterruptTime();
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
                   uint16_t currentInterrupt,
                   uint8_t *memory);
bool st_setTestFile(char *arg);
void st_exit();

#endif // __SELFTEST_H__