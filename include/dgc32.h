#ifndef __DGC32_H__
#define __DGC32_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef SELF_TEST
#include "selftest.h"
#endif

#define ROM_SIZE 0x4000
#define MEMORY_SIZE 0x100000000

// Op codes utils
#define OP_CODE_FULL_MASK  0xFF000000
#define OP_CODE_BASE_MASK  0x0F000000
#define OP_CODE_VARI_MASK  0xF0000000
#define OP_CODE_SHIFT 24

#define OP_CODE_GET_FULL(instruction) ((uint8_t)((instruction & OP_CODE_FULL_MASK) >> OP_CODE_SHIFT))
#define OP_CODE_GET_BASE(instruction) ((uint8_t)((instruction & OP_CODE_BASE_MASK) >> OP_CODE_SHIFT))
#define OP_CODE_GET_VARI(instruction) ((uint8_t)((instruction & OP_CODE_VARI_MASK) >> OP_CODE_SHIFT))

#define OP_CODE_CHECK_FULL(instruction, opcode) (OP_CODE_GET_FULL(instruction) == opcode)
#define OP_CODE_CHECK_BASE(instruction, opcode) (OP_CODE_GET_BASE(instruction) == (opcode & 0x0F))
#define OP_CODE_CHECK_VARI(instruction, opcode) (OP_CODE_GET_VARI(instruction) == (opcode & 0xF0))

// Op code defs
#define OP_CODE_NOOP 0x00

#define OP_CODE_INTR_BASE         0x01
#define OP_CODE_INTR_SUS_VARI     0x00
#define OP_CODE_INTR_RES_VARI     0x10
#define OP_CODE_INTR_TRIG_F4_VARI 0x20
#define OP_CODE_INTR_TRIG_F7_VARI 0x30
#define OP_CODE_INTR_FIN_VARI     0x40
#define OP_CODE_INTR_GPR          0x50

#define OP_CODE_LOAD_F2 0x02
#define OP_CODE_LOAD_F4 0x03

#define OP_CODE_STOR_F2 0x04
#define OP_CODE_STOR_F4 0x05

#define OP_CODE_MOVE_F2 0x06
#define OP_CODE_MOVE_F5 0x07

#define OP_CODE_MATH_BASE_F1  0x08
#define OP_CODE_MATH_BASE_F3  0x09
#define OP_CODE_ADD_VARI      0x10
#define OP_CODE_SUB_VARI      0x20
#define OP_CODE_AND_VARI      0x30
#define OP_CODE_OR_VARI       0x40
#define OP_CODE_XOR_VARI      0x50
#define OP_CODE_NOT_VARI      0x60
#define OP_CODE_BSLT_VARI     0x70
#define OP_CODE_BSRT_VARI     0x80
#define OP_CODE_BSLC_VARI     0x90
#define OP_CODE_BSRC_VARI     0xA0

#define OP_CODE_COMP_F2 0x0A
#define OP_CODE_COMP_F4 0x0B

#define OP_CODE_BRNC_BASE_F4 0x0C
#define OP_CODE_BRNC_BASE_F6 0x0D
#define OP_CODE_BRNC_AL_VARI 0x00
#define OP_CODE_BRNC_EQ_VARI 0x10
#define OP_CODE_BRNC_NE_VARI 0x20
#define OP_CODE_BRNC_HI_VARI 0x30
#define OP_CODE_BRNC_HS_VARI 0x40
#define OP_CODE_BRNC_LS_VARI 0x50
#define OP_CODE_BRNC_LO_VARI 0x60
#define OP_CODE_BRNC_GT_VARI 0x70
#define OP_CODE_BRNC_GE_VARI 0x80
#define OP_CODE_BRNC_LE_VARI 0x90
#define OP_CODE_BRNC_LT_VARI 0xA0
#define OP_CODE_BRNC_MI_VARI 0xB0
#define OP_CODE_BRNC_PZ_VARI 0xC0
#define OP_CODE_BRNC_OV_VARI 0xD0
#define OP_CODE_BRNC_NV_VARI 0xE0

#define OP_CODE_STCK_BASE         0x0E
#define OP_CODE_STCK_PUSH_VARI    0x00
#define OP_CODE_STCK_POP_VARI     0x10
#define OP_CODE_STCK_PUSHALL_VARI 0x20
#define OP_CODE_STCK_POPALL_VARI  0x30
#define OP_CODE_STCK_PEEK_VARI    0x40
#define OP_CODE_STCK_RETURN_VARI  0x50

#define OP_CODE_TERM_BASE 0x0F
#define OP_CODE_TERM_FULL 0xFF

// Regsel
#define REGSEL_1_OFFSET 20
#define REGSEL_2_OFFSET 16
#define REGSEL_3_OFFSET 12

#define REGSEL_1_MASK 0x00F00000
#define REGSEL_2_MASK 0x000F0000
#define REGSEL_3_MASK 0x0000F000

#define REGSEL_1_GET(instruction) ((uint8_t) ((instruction & REGSEL_1_MASK) >> REGSEL_1_OFFSET))
#define REGSEL_2_GET(instruction) ((uint8_t) ((instruction & REGSEL_2_MASK) >> REGSEL_2_OFFSET))
#define REGSEL_3_GET(instruction) ((uint8_t) ((instruction & REGSEL_3_MASK) >> REGSEL_3_OFFSET))

// Argument
#define ARG_F3_MASK 0x0000FFFF
#define ARG_F5_MASK 0x000FFFFF
#define ARG_F7_MASK 0x00FFFFFF

#define ARG_F3_GET(instruction) (instruction & ARG_F3_MASK)
#define ARG_F5_GET(instruction) (instruction & ARG_F5_MASK)
#define ARG_F7_GET(instruction) (instruction & ARG_F7_MASK)

// Instruction Agument
#define INS_AUG_OFFSET_REGSEL_MASK 0b00000011
#define INS_AUG_WORD_SIZE_MASK     0b00001100
#define INS_AUG_WORD_OFFSET_MASK   0b00110000
#define INS_AUG_PUSH_MASK          0b01000000
#define INS_AUG_ABS_MASK           0b01000000
#define INS_AUG_REL_MASK           0b10000000

#define INS_AUG_OFFSET_REGSEL_NONE 0b00000000
#define INS_AUG_OFFSET_REGSEL_OA   0b00000001
#define INS_AUG_OFFSET_REGSEL_OB   0b00000010
#define INS_AUG_OFFSET_REGSEL_OC   0b00000011

#define INS_AUG_WORD_SIZE_4        0b00000000
#define INS_AUG_WORD_SIZE_2        0b00000100
#define INS_AUG_WORD_SIZE_1        0b00001000
#define INS_AUG_WORD_SIZE_INVALID  0b00001100

#define INS_AUG_WORD_OFFSET_0      0b00000000
#define INS_AUG_WORD_OFFSET_1      0b00010000
#define INS_AUG_WORD_OFFSET_2      0b00100000
#define INS_AUG_WORD_OFFSET_3      0b00110000

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

#define INTERRULT_CODE_EMPTY_POP      0x0005
#define INTERRUPT_CODE_CRITICAL_STACK 0x0105

#define STAT_REG_INT_IN_PROG_MASK 0b00000001
#define STAT_REG_INT_SUS_MASK     0b00000010

// Misc
#define STACK_OVERFLOW_THRESHOLD 34

#endif // __DGC32_H__