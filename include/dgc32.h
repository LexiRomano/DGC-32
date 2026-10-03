#ifndef __DGC32_H__
#define __DGC32_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <GLFW/glfw3.h>
// Included at the bottom since it requires memTransFP_t
//#include "motherboard.h"

#ifdef SELF_TEST
#include "selftest.h"
#endif

#define ROM_SIZE 0x4000
#define MEMORY_SIZE 0x100000000

// Op code defintions
#define OP_DEBUG               0x00
#define OP_MOVE_REG            0x01
#define OP_MOVE_IM_32          0x02
#define OP_MOVE_IM_16          0x03
#define OP_MOVE_IM_16_SIG      0x04
#define OP_MOVE_IM_8           0x05
#define OP_MOVE_IM_8_SIG       0x06
#define OP_LOAD_32_REG_ABS     0x07
#define OP_LOAD_32_REG_PC      0x08
#define OP_LOAD_32_REG_OA      0x09
#define OP_LOAD_32_REG_OB      0x0A
#define OP_LOAD_32_REG_OC      0x0B
#define OP_LOAD_16_REG_ABS     0x0C
#define OP_LOAD_16_REG_PC      0x0D
#define OP_LOAD_16_REG_OA      0x0E
#define OP_LOAD_16_REG_OB      0x0F
#define OP_LOAD_16_REG_OC      0x10
#define OP_LOAD_16_SIG_REG_ABS 0x11
#define OP_LOAD_16_SIG_REG_PC  0x12
#define OP_LOAD_16_SIG_REG_OA  0x13
#define OP_LOAD_16_SIG_REG_OB  0x14
#define OP_LOAD_16_SIG_REG_OC  0x15
#define OP_LOAD_8_REG_ABS      0x16
#define OP_LOAD_8_REG_PC       0x17
#define OP_LOAD_8_REG_OA       0x18
#define OP_LOAD_8_REG_OB       0x19
#define OP_LOAD_8_REG_OC       0x1A
#define OP_LOAD_8_SIG_REG_ABS  0x1B
#define OP_LOAD_8_SIG_REG_PC   0x1C
#define OP_LOAD_8_SIG_REG_OA   0x1D
#define OP_LOAD_8_SIG_REG_OB   0x1E
#define OP_LOAD_8_SIG_REG_OC   0x1F
#define OP_LOAD_32_IM_ABS      0x20
#define OP_LOAD_32_IM_PC       0x21
#define OP_LOAD_32_IM_OA       0x22
#define OP_LOAD_32_IM_OB       0x23
#define OP_LOAD_32_IM_OC       0x24
#define OP_LOAD_16_IM_ABS      0x25
#define OP_LOAD_16_IM_PC       0x26
#define OP_LOAD_16_IM_OA       0x27
#define OP_LOAD_16_IM_OB       0x28
#define OP_LOAD_16_IM_OC       0x29
#define OP_LOAD_16_SIG_IM_ABS  0x2A
#define OP_LOAD_16_SIG_IM_PC   0x2B
#define OP_LOAD_16_SIG_IM_OA   0x2C
#define OP_LOAD_16_SIG_IM_OB   0x2D
#define OP_LOAD_16_SIG_IM_OC   0x2E
#define OP_LOAD_8_IM_ABS       0x2F
#define OP_LOAD_8_IM_PC        0x30
#define OP_LOAD_8_IM_OA        0x31
#define OP_LOAD_8_IM_OB        0x32
#define OP_LOAD_8_IM_OC        0x33
#define OP_LOAD_8_SIG_IM_ABS   0x34
#define OP_LOAD_8_SIG_IM_PC    0x35
#define OP_LOAD_8_SIG_IM_OA    0x36
#define OP_LOAD_8_SIG_IM_OB    0x37
#define OP_LOAD_8_SIG_IM_OC    0x38
#define OP_STOR_32_REG_ABS     0x39
#define OP_STOR_32_REG_PC      0x3A
#define OP_STOR_32_REG_OA      0x3B
#define OP_STOR_32_REG_OB      0x3C
#define OP_STOR_32_REG_OC      0x3D
#define OP_STOR_16_REG_ABS     0x3E
#define OP_STOR_16_REG_PC      0x3F
#define OP_STOR_16_REG_OA      0x40
#define OP_STOR_16_REG_OB      0x41
#define OP_STOR_16_REG_OC      0x42
#define OP_STOR_8_REG_ABS      0x43
#define OP_STOR_8_REG_PC       0x44
#define OP_STOR_8_REG_OA       0x45
#define OP_STOR_8_REG_OB       0x46
#define OP_STOR_8_REG_OC       0x47
#define OP_STOR_32_IM_ABS      0x48
#define OP_STOR_32_IM_PC       0x49
#define OP_STOR_32_IM_OA       0x4A
#define OP_STOR_32_IM_OB       0x4B
#define OP_STOR_32_IM_OC       0x4C
#define OP_STOR_16_IM_ABS      0x4D
#define OP_STOR_16_IM_PC       0x4E
#define OP_STOR_16_IM_OA       0x4F
#define OP_STOR_16_IM_OB       0x50
#define OP_STOR_16_IM_OC       0x51
#define OP_STOR_8_IM_ABS       0x52
#define OP_STOR_8_IM_PC        0x53
#define OP_STOR_8_IM_OA        0x54
#define OP_STOR_8_IM_OB        0x55
#define OP_STOR_8_IM_OC        0x56
#define OP_ADD_INT_REG         0x57
#define OP_ADD_INT_IM_32       0x58
#define OP_ADD_INT_IM_16       0x59
#define OP_ADD_INT_IM_16_SIG   0x5A
#define OP_ADD_INT_IM_8        0x5B
#define OP_ADD_INT_IM_8_SIG    0x5C
#define OP_ADD_FL_REG          0x5D
#define OP_ADD_FL_IM           0x5E
#define OP_SUB_INT_REG         0x5F
#define OP_SUB_INT_IM_32       0x60
#define OP_SUB_INT_IM_16       0x61
#define OP_SUB_INT_IM_16_SIG   0x62
#define OP_SUB_INT_IM_8        0x63
#define OP_SUB_INT_IM_8_SIG    0x64
#define OP_SUB_FL_REG          0x65
#define OP_SUB_FL_IM           0x66
#define OP_MUL_INT_REG         0x67
#define OP_MUL_INT_IM_32       0x68
#define OP_MUL_INT_IM_16       0x69
#define OP_MUL_INT_IM_16_SIG   0x6A
#define OP_MUL_INT_IM_8        0x6B
#define OP_MUL_INT_IM_8_SIG    0x6C
#define OP_MUL_FL_REG          0x6D
#define OP_MUL_FL_IM           0x6E
#define OP_DIV_INT_REG         0x6F
#define OP_DIV_INT_IM_32       0x70
#define OP_DIV_INT_IM_16       0x71
#define OP_DIV_INT_IM_16_SIG   0x72
#define OP_DIV_INT_IM_8        0x73
#define OP_DIV_INT_IM_8_SIG    0x74
#define OP_DIV_FL_REG          0x75
#define OP_DIV_FL_IM           0x76
#define OP_MOD_INT_REG         0x77
#define OP_MOD_INT_IM_32       0x78
#define OP_MOD_INT_IM_16       0x79
#define OP_MOD_INT_IM_16_SIG   0x7A
#define OP_MOD_INT_IM_8        0x7B
#define OP_MOD_INT_IM_8_SIG    0x7C
#define OP_AND_REG             0x7D
#define OP_AND_IM_32           0x7E
#define OP_AND_IM_16           0x7F
#define OP_AND_IM_8            0x80
#define OP_OR_REG              0x81
#define OP_OR_IM_32            0x82
#define OP_OR_IM_16            0x83
#define OP_OR_IM_8             0x84
#define OP_XOR_REG             0x85
#define OP_XOR_IM_32           0x86
#define OP_XOR_IM_16           0x87
#define OP_XOR_IM_8            0x88
#define OP_BSLT_REG            0x89
#define OP_BSLT_IM_8           0x8A
#define OP_BSLC_REG            0x8B
#define OP_BSLC_IM_8           0x8C
#define OP_BSRT_REG            0x8D
#define OP_BSRT_IM_8           0x8E
#define OP_BSRC_REG            0x8F
#define OP_BSRC_IM_8           0x90
#define OP_NOT_REG             0x91
#define OP_COMP_INT_REG        0x92
#define OP_COMP_INT_IM_32      0x93
#define OP_COMP_INT_IM_16      0x94
#define OP_COMP_INT_IM_16_SIG  0x95
#define OP_COMP_INT_IM_8       0x96
#define OP_COMP_INT_IM_8_SIG   0x97
#define OP_COMP_FL_REG         0x98
#define OP_COMP_FL_IM          0x99
#define OP_EVAL_INT_REG        0x9A
#define OP_EVAL_INT_IM_32      0x9B
#define OP_EVAL_INT_IM_16      0x9C
#define OP_EVAL_INT_IM_16_SIG  0x9D
#define OP_EVAL_INT_IM_8       0x9E
#define OP_EVAL_INT_IM_8_SIG   0x9F
#define OP_EVAL_FL_REG         0xA0
#define OP_EVAL_FL_IM          0xA1
#define OP_BRNC_REG_ABS        0xA2
#define OP_BRNC_REG_PC         0xA3
#define OP_BRNC_REG_OA         0xA4
#define OP_BRNC_REG_OB         0xA5
#define OP_BRNC_REG_OC         0xA6
#define OP_BRNC_P_REG_ABS      0xA7
#define OP_BRNC_P_REG_PC       0xA8
#define OP_BRNC_P_REG_OA       0xA9
#define OP_BRNC_P_REG_OB       0xAA
#define OP_BRNC_P_REG_OC       0xAB
#define OP_BRNC_IM_ABS         0xAC
#define OP_BRNC_IM_PC          0xAD
#define OP_BRNC_IM_OA          0xAE
#define OP_BRNC_IM_OB          0xAF
#define OP_BRNC_IM_OC          0xB0
#define OP_BRNC_P_IM_ABS       0xB1
#define OP_BRNC_P_IM_PC        0xB2
#define OP_BRNC_P_IM_OA        0xB3
#define OP_BRNC_P_IM_OB        0xB4
#define OP_BRNC_P_IM_OC        0xB5
#define OP_FL_TO_INT           0xB6
#define OP_INT_TO_FL           0xB7
#define OP_INT_SIG_TO_FL       0xB8
#define OP_PUSH                0xB9
#define OP_POP                 0xBA
#define OP_PEEK                0xBB
#define OP_RETURN              0xBC
#define OP_INTR_SUS            0xBD
#define OP_INTR_RES            0xBE
#define OP_INTR_FIN            0xBF
#define OP_INTR_TGR_REG        0xC0
#define OP_INTR_TGR_IM_16      0xC1
#define OP_INTR_GET_PARAM      0xC2
#define OP_INTR_GET_RET_AD     0xC3
#define OP_INTR_SET_RET_AD_REG 0xC4
#define OP_INTR_SET_RET_AD_IM  0xC5
#define OP_GETABS_REG_PC       0xC6
#define OP_GETABS_IM_PC        0xC7
#define OP_GETREL_REG_OA       0xC8
#define OP_GETREL_REG_OB       0xC9
#define OP_GETREL_REG_OC       0xCA
#define OP_GETREL_IM_OA        0xCB
#define OP_GETREL_IM_OB        0xCC
#define OP_GETREL_IM_OC        0xCD
#define OP_SWAP_32_REG_ABS     0xCE
#define OP_SWAP_32_REG_PC      0xCF
#define OP_SWAP_32_REG_OA      0xD0
#define OP_SWAP_32_REG_OB      0xD1
#define OP_SWAP_32_REG_OC      0xD2
#define OP_SWAP_16_REG_ABS     0xD3
#define OP_SWAP_16_REG_PC      0xD4
#define OP_SWAP_16_REG_OA      0xD5
#define OP_SWAP_16_REG_OB      0xD6
#define OP_SWAP_16_REG_OC      0xD7
#define OP_SWAP_16_SIG_REG_ABS 0xD8
#define OP_SWAP_16_SIG_REG_PC  0xD9
#define OP_SWAP_16_SIG_REG_OA  0xDA
#define OP_SWAP_16_SIG_REG_OB  0xDB
#define OP_SWAP_16_SIG_REG_OC  0xDC
#define OP_SWAP_8_REG_ABS      0xDD
#define OP_SWAP_8_REG_PC       0xDE
#define OP_SWAP_8_REG_OA       0xDF
#define OP_SWAP_8_REG_OB       0xE0
#define OP_SWAP_8_REG_OC       0xE1
#define OP_SWAP_8_SIG_REG_ABS  0xE2
#define OP_SWAP_8_SIG_REG_PC   0xE3
#define OP_SWAP_8_SIG_REG_OA   0xE4
#define OP_SWAP_8_SIG_REG_OB   0xE5
#define OP_SWAP_8_SIG_REG_OC   0xE6
#define OP_SWAP_32_IM_ABS      0xE7
#define OP_SWAP_32_IM_PC       0xE8
#define OP_SWAP_32_IM_OA       0xE9
#define OP_SWAP_32_IM_OB       0xEA
#define OP_SWAP_32_IM_OC       0xEB
#define OP_SWAP_16_IM_ABS      0xEC
#define OP_SWAP_16_IM_PC       0xED
#define OP_SWAP_16_IM_OA       0xEE
#define OP_SWAP_16_IM_OB       0xEF
#define OP_SWAP_16_IM_OC       0xF0
#define OP_SWAP_16_SIG_IM_ABS  0xF1
#define OP_SWAP_16_SIG_IM_PC   0xF2
#define OP_SWAP_16_SIG_IM_OA   0xF3
#define OP_SWAP_16_SIG_IM_OB   0xF4
#define OP_SWAP_16_SIG_IM_OC   0xF5
#define OP_SWAP_8_IM_ABS       0xF6
#define OP_SWAP_8_IM_PC        0xF7
#define OP_SWAP_8_IM_OA        0xF8
#define OP_SWAP_8_IM_OB        0xF9
#define OP_SWAP_8_IM_OC        0xFA
#define OP_SWAP_8_SIG_IM_ABS   0xFB
#define OP_SWAP_8_SIG_IM_PC    0xFC
#define OP_SWAP_8_SIG_IM_OA    0xFD
#define OP_SWAP_8_SIG_IM_OB    0xFE
#define OP_SWAP_8_SIG_IM_OC    0xFF

#define HIGH_NIBBLE(val) ((val & 0xF0) >> 4)
#define LOW_NIBBLE(val)   (val & 0x0F)

// Flags
#define FLAG_V 0b0001
#define FLAG_C 0b0010
#define FLAG_N 0b0100
#define FLAG_Z 0b1000

// Interrupts
#define INTERRUPT_QUEUE_BASE 0x4000
#define INTERRUPT_QUEUE_SIZE 0x100
#define INTERRUPT_FULL_MASK  0xFFFF
#define INTERRUPT_ID_MASK    0x00FF
#define INTERRUPT_ARG_MASK   0xFF00
#define INTERRUPT_ARG_OFFSET 8
#define INTERRUPT_CONSTRUCT(id, arg) ((uint16_t) (((id >> INTERRUPT_ARG_OFFSET) & INTERRUPT_ID_MASK) | (arg & INTERRUPT_ARG_MASK)))

#define INTERRUPT_CODE_KEY_PRESSED         0x0001
#define INTERRUPT_CODE_KEY_RELEASED        0x0002
#define INTERRUPT_CODE_PERIPHERAL_EVENT    0x0003
#define INTERRUPT_CODE_MOTHERBOARD_EVENT   0x0004
#define INTERRUPT_CODE_EMPTY_POP           0x0005
#define INTERRUPT_CODE_CRITICAL_STACK      0x0105
#define INTERRUPT_CODE_TIMER_EVENT         0x0006
#define INTERRUPT_CODE_STORAGE_EVENT       0x0007
#define INTERRUPT_CODE_GRAPHICAL_EVENT     0x0008
#define INTERRUPT_CODE_MEMORY_VIOLATION    0x0009
#define INTERRUPT_CODE_INVALID_INSTRUCTION 0x000A
#define INTERRUPT_CODE_DIVISION_BY_ZERO    0x000B

#define STAT_REG_INT_IN_PROG_MASK 0b00000001
#define STAT_REG_INT_SUS_MASK     0b00000010

// Memory bounds
#define MEMBOUND_ROM_START  0x00000000
#define MEMBOUND_ROM_END    0x00003FFF
#define MEMBOUND_HRES_START 0x00004000
#define MEMBOUND_HRES_END   0x00005FFF
#define MEMBOUND_IO_START   0x00006000
#define MEMBOUND_DREG_START MEMBOUND_IO_START
#define MEMBOUND_DREG_END   0x000060FF
#define MEMBOUND_DDAT_START 0x00006100
#define MEMBOUND_DDAT_END   0x00007FFF
#define MEMBOUND_IO_END     MEMBOUNT_DDAT_END
#define MEMBOUND_GEN_START  0x00008000
#define MEMBOUND_GEN_END    0xFFFFFFFF

#define MEMBOUND_READ_1_START  MEMBOUND_DREG_START
#define MEMBOUND_READ_1_END    MEMBOUND_GEN_END
#define MEMBOUND_READ_2_START  MEMBOUND_ROM_START
#define MEMBOUND_READ_2_END    MEMBOUND_ROM_END
#define MEMBOUND_WRITE_START   MEMBOUND_DDAT_START
#define MEMBOUND_WRITE_END     MEMBOUND_GEN_END

#define MEMBOUND_CAN_READ(address, size)  (((address >= MEMBOUND_READ_1_START) && (((uint64_t) address) + size - 1 <= MEMBOUND_READ_1_END)) ||\
                                           ((address >= MEMBOUND_READ_2_START) && (address + size - 1 <= MEMBOUND_READ_2_END)))
#define MEMBOUND_CAN_WRITE(address, size) ((address >= MEMBOUND_WRITE_START) && (((uint64_t) address) + size - 1 <= MEMBOUND_WRITE_END))

// Misc
#define STACK_OVERFLOW_THRESHOLD 34
#define STACK_PUSHALL_SIZE 33

typedef enum
{
    ADDRESSING_MODE_ABS,
    ADDRESSING_MODE_PC,
    ADDRESSING_MODE_OA,
    ADDRESSING_MODE_OB,
    ADDRESSING_MODE_OC,
    ADDRESSING_MODE_PC_M_OA,
    ADDRESSING_MODE_PC_M_OB,
    ADDRESSING_MODE_PC_M_OC,
} addressingMode_e;

typedef enum
{
    MATH_OPERATION_ADD_INT,
    MATH_OPERATION_ADD_FL,
    MATH_OPERATION_SUB_INT,
    MATH_OPERATION_SUB_FL,
    MATH_OPERATION_MUL_INT,
    MATH_OPERATION_MUL_FL,
    MATH_OPERATION_DIV_INT,
    MATH_OPERATION_DIV_FL,
    MATH_OPERATION_MOD,
    MATH_OPERATION_AND,
    MATH_OPERATION_OR,
    MATH_OPERATION_XOR,
    MATH_OPERATION_BSLT,
    MATH_OPERATION_BSLC,
    MATH_OPERATION_BSRT,
    MATH_OPERATION_BSRC,
    MATH_OPERATION_NOT
} mathOperation_e;

typedef enum
{
    COMP_CODE_AL = 0x00,
    COMP_CODE_EQ = 0x01,
    COMP_CODE_NE = 0x02,
    COMP_CODE_HI = 0x03,
    COMP_CODE_HS = 0x04,
    COMP_CODE_LS = 0x05,
    COMP_CODE_LO = 0x06,
    COMP_CODE_GT = 0x07,
    COMP_CODE_GE = 0x08,
    COMP_CODE_LE = 0x09,
    COMP_CODE_LT = 0x0A,
    COMP_CODE_MI = 0x0B,
    COMP_CODE_PZ = 0x0C,
    COMP_CODE_OV = 0x0D,
    COMP_CODE_NV = 0x0E,
} comparisonCode_e;

typedef enum
{
    STACK_UTIL_PUSH,
    STACK_UTIL_POP,
    STACK_UTIL_PEEK,
    STACK_UTIL_RETURN,
} stackUtil_e;


typedef void (*memTransFP_t)(uint32_t, uint8_t, void*);
typedef void (*interruptFP_t)(uint16_t);
#include "motherboard.h"

#endif // __DGC32_H__
