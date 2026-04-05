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

#define OP_CODE_LOAD_F5 0x02
#define OP_CODE_LOAD_F4 0x03

#define OP_CODE_STOR_F5 0x04
#define OP_CODE_STOR_F4 0x05

#define OP_CODE_MOVE_F2 0x06
#define OP_CODE_MOVE_F4 0x07

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
#define OP_CODE_COMP_F5 0x0B

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
#define OP_CODE_STCK_INIT_VARI    0x50
#define OP_CODE_STCK_RETURN_VARI  0x60

#define OP_CODE_TERM_BASE 0x0F
#define OP_CODE_TERM_FULL 0xFF

#endif // __DGC32_H__