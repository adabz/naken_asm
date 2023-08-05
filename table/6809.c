/**
 *  naken_asm assembler.
 *  Author: Michael Kohn
 *   Email: mike@mikekohn.net
 *     Web: https://www.mikekohn.net/
 * License: GPLv3
 *
 * Copyright 2010-2023 by Michael Kohn
 *
 */

#include <stdlib.h>

#include "table/6809.h"

struct _table_6809 table_6809[] =
{
  { "neg", 0x00, M6809_OP_DIRECT, 2, 6, 6 },
  { "com", 0x03, M6809_OP_DIRECT, 2, 6, 6 },
  { "lsr", 0x04, M6809_OP_DIRECT, 2, 6, 6 },
  { "ror", 0x06, M6809_OP_DIRECT, 2, 6, 6 },
  { "asr", 0x07, M6809_OP_DIRECT, 2, 6, 6 },
  { "lsl", 0x08, M6809_OP_DIRECT, 2, 6, 6 },
  { "rol", 0x09, M6809_OP_DIRECT, 2, 6, 6 },
  { "dec", 0x0a, M6809_OP_DIRECT, 2, 6, 6 },
  { "inc", 0x0c, M6809_OP_DIRECT, 2, 6, 6 },
  { "tst", 0x0d, M6809_OP_DIRECT, 2, 6, 6 },
  { "jmp", 0x0e, M6809_OP_DIRECT, 2, 3, 3 },
  { "clr", 0x0f, M6809_OP_DIRECT, 2, 6, 6 },
  //{ "page1+", 0x10, M6809_OP_VARIANT, 1, 1, 1 },
  //{ "page2+", 0x11, M6809_OP_VARIANT, 1, 1, 1 },
  { "nop", 0x12, M6809_OP_INHERENT, 1, 2, 2 },
  { "sync", 0x13, M6809_OP_INHERENT, 1, 2, 2 },
  { "lbra", 0x16, M6809_OP_LONG_RELATIVE, 3, 5, 5 },
  { "lbsr", 0x17, M6809_OP_LONG_RELATIVE, 3, 9, 9 },
  { "daa", 0x19, M6809_OP_INHERENT, 1, 2, 2 },
  { "orcc", 0x1a, M6809_OP_IMMEDIATE, 2, 3, 3 },
  { "andcc", 0x1c, M6809_OP_IMMEDIATE, 2, 3, 3 },
  { "sex", 0x1d, M6809_OP_INHERENT, 1, 2, 2 },
  { "exg", 0x1e, M6809_OP_TWO_REG, 2, 8, 8 },
  { "tfr", 0x1f, M6809_OP_TWO_REG, 2, 7, 7 },
  { "bra", 0x20, M6809_OP_RELATIVE, 2, 3, 3 },
  { "brn", 0x21, M6809_OP_RELATIVE, 2, 3, 3 },
  { "bhi", 0x22, M6809_OP_RELATIVE, 2, 3, 3 },
  { "bls", 0x23, M6809_OP_RELATIVE, 2, 3, 3 },
  { "bhs", 0x24, M6809_OP_RELATIVE, 2, 3, 3 },
  { "blo", 0x25, M6809_OP_RELATIVE, 2, 3, 3 },
  { "bne", 0x26, M6809_OP_RELATIVE, 2, 3, 3 },
  { "beq", 0x27, M6809_OP_RELATIVE, 2, 3, 3 },
  { "bvc", 0x28, M6809_OP_RELATIVE, 2, 3, 3 },
  { "bvs", 0x29, M6809_OP_RELATIVE, 2, 3, 3 },
  { "bpl", 0x2a, M6809_OP_RELATIVE, 2, 3, 3 },
  { "bmi", 0x2b, M6809_OP_RELATIVE, 2, 3, 3 },
  { "bge", 0x2c, M6809_OP_RELATIVE, 2, 3, 3 },
  { "blt", 0x2d, M6809_OP_RELATIVE, 2, 3, 3 },
  { "bgt", 0x2e, M6809_OP_RELATIVE, 2, 3, 3 },
  { "ble", 0x2f, M6809_OP_RELATIVE, 2, 3, 3 },
  { "leax", 0x30, M6809_OP_INDEXED, 2, 4, 4 },
  { "leay", 0x31, M6809_OP_INDEXED, 2, 4, 4 },
  { "leas", 0x32, M6809_OP_INDEXED, 2, 4, 4 },
  { "leau", 0x33, M6809_OP_INDEXED, 2, 4, 4 },
  { "pshs", 0x34, M6809_OP_STACK, 2, 5, 5 },
  { "puls", 0x35, M6809_OP_STACK, 2, 5, 5 },
  { "pshu", 0x36, M6809_OP_STACK, 2, 5, 5 },
  { "pulu", 0x37, M6809_OP_STACK, 2, 5, 5 },
  { "rts", 0x39, M6809_OP_INHERENT, 1, 5, 5 },
  { "abx", 0x3a, M6809_OP_INHERENT, 1, 3, 3 },
  { "rti", 0x3b, M6809_OP_INHERENT, 1, 6, 15 },
  { "cwai", 0x3c, M6809_OP_IMMEDIATE, 2, 21, 21 },
  { "mul", 0x3d, M6809_OP_INHERENT, 1, 11, 11 },
  //{ "reset", 0x3e, M6809_OP_INHERENT, 1, 0, 0 },
  { "swi", 0x3f, M6809_OP_INHERENT, 1, 19, 19 },
  { "nega", 0x40, M6809_OP_INHERENT, 1, 2, 2 },
  { "coma", 0x43, M6809_OP_INHERENT, 1, 2, 2 },
  { "lsra", 0x44, M6809_OP_INHERENT, 1, 2, 2 },
  { "rora", 0x46, M6809_OP_INHERENT, 1, 2, 2 },
  { "asra", 0x47, M6809_OP_INHERENT, 1, 2, 2 },
  { "lsla", 0x48, M6809_OP_INHERENT, 1, 2, 2 },
  { "rola", 0x49, M6809_OP_INHERENT, 1, 2, 2 },
  { "deca", 0x4a, M6809_OP_INHERENT, 1, 2, 2 },
  { "inca", 0x4c, M6809_OP_INHERENT, 1, 2, 2 },
  { "tsta", 0x4d, M6809_OP_INHERENT, 1, 2, 2 },
  { "clra", 0x4f, M6809_OP_INHERENT, 1, 2, 2 },
  { "negb", 0x50, M6809_OP_INHERENT, 1, 2, 2 },
  { "comb", 0x53, M6809_OP_INHERENT, 1, 2, 2 },
  { "lsrb", 0x54, M6809_OP_INHERENT, 1, 2, 2 },
  { "rorb", 0x56, M6809_OP_INHERENT, 1, 2, 2 },
  { "asrb", 0x57, M6809_OP_INHERENT, 1, 2, 2 },
  { "lslb", 0x58, M6809_OP_INHERENT, 1, 2, 2 },
  { "rolb", 0x59, M6809_OP_INHERENT, 1, 2, 2 },
  { "decb", 0x5a, M6809_OP_INHERENT, 1, 2, 2 },
  { "incb", 0x5c, M6809_OP_INHERENT, 1, 2, 2 },
  { "tstb", 0x5d, M6809_OP_INHERENT, 1, 2, 2 },
  { "clrb", 0x5f, M6809_OP_INHERENT, 1, 2, 2 },
  { "neg", 0x60, M6809_OP_INDEXED, 2, 6, 6 },
  { "com", 0x63, M6809_OP_INDEXED, 2, 6, 6 },
  { "lsr", 0x64, M6809_OP_INDEXED, 2, 6, 6 },
  { "ror", 0x66, M6809_OP_INDEXED, 2, 6, 6 },
  { "asr", 0x67, M6809_OP_INDEXED, 2, 6, 6 },
  { "lsl", 0x68, M6809_OP_INDEXED, 2, 6, 6 },
  { "rol", 0x69, M6809_OP_INDEXED, 2, 6, 6 },
  { "dec", 0x6a, M6809_OP_INDEXED, 2, 6, 6 },
  { "inc", 0x6c, M6809_OP_INDEXED, 2, 6, 6 },
  { "tst", 0x6d, M6809_OP_INDEXED, 2, 6, 6 },
  { "jmp", 0x6e, M6809_OP_INDEXED, 2, 3, 3 },
  { "clr", 0x6f, M6809_OP_INDEXED, 2, 6, 6 },
  { "neg", 0x70, M6809_OP_EXTENDED, 3, 7, 7 },
  { "com", 0x73, M6809_OP_EXTENDED, 3, 7, 7 },
  { "lsr", 0x74, M6809_OP_EXTENDED, 3, 7, 7 },
  { "ror", 0x76, M6809_OP_EXTENDED, 3, 7, 7 },
  { "asr", 0x77, M6809_OP_EXTENDED, 3, 7, 7 },
  { "lsl", 0x78, M6809_OP_EXTENDED, 3, 7, 7 },
  { "rol", 0x79, M6809_OP_EXTENDED, 3, 7, 7 },
  { "dec", 0x7a, M6809_OP_EXTENDED, 3, 7, 7 },
  { "inc", 0x7c, M6809_OP_EXTENDED, 3, 7, 7 },
  { "tst", 0x7d, M6809_OP_EXTENDED, 3, 7, 7 },
  { "jmp", 0x7e, M6809_OP_EXTENDED, 3, 3, 3 },
  { "clr", 0x7f, M6809_OP_EXTENDED, 3, 7, 7 },
  { "suba", 0x80, M6809_OP_IMMEDIATE, 2, 2, 2 },
  { "cmpa", 0x81, M6809_OP_IMMEDIATE, 2, 2, 2 },
  { "sbca", 0x82, M6809_OP_IMMEDIATE, 2, 2, 2 },
  { "subd", 0x83, M6809_OP_IMMEDIATE, 3, 4, 4 },
  { "anda", 0x84, M6809_OP_IMMEDIATE, 2, 2, 2 },
  { "bita", 0x85, M6809_OP_IMMEDIATE, 2, 2, 2 },
  { "lda", 0x86, M6809_OP_IMMEDIATE, 2, 2, 2 },
  { "eora", 0x88, M6809_OP_IMMEDIATE, 2, 2, 2 },
  { "adca", 0x89, M6809_OP_IMMEDIATE, 2, 2, 2 },
  { "ora", 0x8a, M6809_OP_IMMEDIATE, 2, 2, 2 },
  { "adda", 0x8b, M6809_OP_IMMEDIATE, 2, 2, 2 },
  { "cmpx", 0x8c, M6809_OP_IMMEDIATE, 3, 4, 4 },
  { "bsr", 0x8d, M6809_OP_RELATIVE, 2, 7, 7 },
  { "ldx", 0x8e, M6809_OP_IMMEDIATE, 3, 3, 3 },
  { "suba", 0x90, M6809_OP_DIRECT, 2, 4, 4 },
  { "cmpa", 0x91, M6809_OP_DIRECT, 2, 4, 4 },
  { "sbca", 0x92, M6809_OP_DIRECT, 2, 4, 4 },
  { "subd", 0x93, M6809_OP_DIRECT, 2, 6, 6 },
  { "anda", 0x94, M6809_OP_DIRECT, 2, 4, 4 },
  { "bita", 0x95, M6809_OP_DIRECT, 2, 4, 4 },
  { "lda", 0x96, M6809_OP_DIRECT, 2, 4, 4 },
  { "sta", 0x97, M6809_OP_DIRECT, 2, 4, 4 },
  { "eora", 0x98, M6809_OP_DIRECT, 2, 4, 4 },
  { "adca", 0x99, M6809_OP_DIRECT, 2, 4, 4 },
  { "ora", 0x9a, M6809_OP_DIRECT, 2, 4, 4 },
  { "adda", 0x9b, M6809_OP_DIRECT, 2, 4, 4 },
  { "cmpx", 0x9c, M6809_OP_DIRECT, 2, 6, 6 },
  { "jsr", 0x9d, M6809_OP_DIRECT, 2, 7, 7 },
  { "ldx", 0x9e, M6809_OP_DIRECT, 2, 5, 5 },
  { "stx", 0x9f, M6809_OP_DIRECT, 2, 5, 5 },
  { "suba", 0xa0, M6809_OP_INDEXED, 2, 4, 4 },
  { "cmpa", 0xa1, M6809_OP_INDEXED, 2, 4, 4 },
  { "sbca", 0xa2, M6809_OP_INDEXED, 2, 4, 4 },
  { "subd", 0xa3, M6809_OP_INDEXED, 2, 6, 6 },
  { "anda", 0xa4, M6809_OP_INDEXED, 2, 4, 4 },
  { "bita", 0xa5, M6809_OP_INDEXED, 2, 4, 4 },
  { "lda", 0xa6, M6809_OP_INDEXED, 2, 4, 4 },
  { "sta", 0xa7, M6809_OP_INDEXED, 2, 4, 4 },
  { "eora", 0xa8, M6809_OP_INDEXED, 2, 4, 4 },
  { "adca", 0xa9, M6809_OP_INDEXED, 2, 4, 4 },
  { "ora", 0xaa, M6809_OP_INDEXED, 2, 4, 4 },
  { "adda", 0xab, M6809_OP_INDEXED, 2, 4, 4 },
  { "cmpx", 0xac, M6809_OP_INDEXED, 2, 6, 6 },
  { "jsr", 0xad, M6809_OP_INDEXED, 2, 7, 7 },
  { "ldx", 0xae, M6809_OP_INDEXED, 2, 5, 5 },
  { "stx", 0xaf, M6809_OP_INDEXED, 2, 5, 5 },
  { "suba", 0xb0, M6809_OP_EXTENDED, 3, 5, 5 },
  { "cmpa", 0xb1, M6809_OP_EXTENDED, 3, 5, 5 },
  { "sbca", 0xb2, M6809_OP_EXTENDED, 3, 5, 5 },
  { "subd", 0xb3, M6809_OP_EXTENDED, 3, 7, 7 },
  { "anda", 0xb4, M6809_OP_EXTENDED, 3, 5, 5 },
  { "bita", 0xb5, M6809_OP_EXTENDED, 3, 5, 5 },
  { "lda", 0xb6, M6809_OP_EXTENDED, 3, 5, 5 },
  { "sta", 0xb7, M6809_OP_EXTENDED, 3, 5, 5 },
  { "eora", 0xb8, M6809_OP_EXTENDED, 3, 5, 5 },
  { "adca", 0xb9, M6809_OP_EXTENDED, 3, 5, 5 },
  { "ora", 0xba, M6809_OP_EXTENDED, 3, 5, 5 },
  { "adda", 0xbb, M6809_OP_EXTENDED, 3, 5, 5 },
  { "cmpx", 0xbc, M6809_OP_EXTENDED, 3, 7, 7 },
  { "jsr", 0xbd, M6809_OP_EXTENDED, 3, 8, 8 },
  { "ldx", 0xbe, M6809_OP_EXTENDED, 3, 6, 6 },
  { "stx", 0xbf, M6809_OP_EXTENDED, 3, 6, 6 },
  { "subb", 0xc0, M6809_OP_IMMEDIATE, 2, 2, 2 },
  { "cmpb", 0xc1, M6809_OP_IMMEDIATE, 2, 2, 2 },
  { "sbcb", 0xc2, M6809_OP_IMMEDIATE, 2, 2, 2 },
  { "addd", 0xc3, M6809_OP_IMMEDIATE, 3, 4, 4 },
  { "andb", 0xc4, M6809_OP_IMMEDIATE, 2, 2, 2 },
  { "bitb", 0xc5, M6809_OP_IMMEDIATE, 2, 2, 2 },
  { "ldb", 0xc6, M6809_OP_IMMEDIATE, 2, 2, 2 },
  { "eorb", 0xc8, M6809_OP_IMMEDIATE, 2, 2, 2 },
  { "adcb", 0xc9, M6809_OP_IMMEDIATE, 2, 2, 2 },
  { "orb", 0xca, M6809_OP_IMMEDIATE, 2, 2, 2 },
  { "addb", 0xcb, M6809_OP_IMMEDIATE, 2, 2, 2 },
  { "ldd", 0xcc, M6809_OP_IMMEDIATE, 3, 3, 3 },
  { "ldu", 0xce, M6809_OP_IMMEDIATE, 3, 3, 3 },
  { "subb", 0xd0, M6809_OP_DIRECT, 2, 4, 4 },
  { "cmpb", 0xd1, M6809_OP_DIRECT, 2, 4, 4 },
  { "sbcb", 0xd2, M6809_OP_DIRECT, 2, 4, 4 },
  { "addd", 0xd3, M6809_OP_DIRECT, 2, 6, 6 },
  { "andb", 0xd4, M6809_OP_DIRECT, 2, 4, 4 },
  { "bitb", 0xd5, M6809_OP_DIRECT, 2, 4, 4 },
  { "ldb", 0xd6, M6809_OP_DIRECT, 2, 4, 4 },
  { "stb", 0xd7, M6809_OP_DIRECT, 2, 4, 4 },
  { "eorb", 0xd8, M6809_OP_DIRECT, 2, 4, 4 },
  { "adcb", 0xd9, M6809_OP_DIRECT, 2, 4, 4 },
  { "orb", 0xda, M6809_OP_DIRECT, 2, 4, 4 },
  { "addb", 0xdb, M6809_OP_DIRECT, 2, 4, 4 },
  { "ldd", 0xdc, M6809_OP_DIRECT, 2, 5, 5 },
  { "std", 0xdd, M6809_OP_DIRECT, 2, 5, 5 },
  { "ldu", 0xde, M6809_OP_DIRECT, 2, 5, 5 },
  { "stu", 0xdf, M6809_OP_DIRECT, 2, 5, 5 },
  { "subb", 0xe0, M6809_OP_INDEXED, 2, 4, 4 },
  { "cmpb", 0xe1, M6809_OP_INDEXED, 2, 4, 4 },
  { "sbcb", 0xe2, M6809_OP_INDEXED, 2, 4, 4 },
  { "addd", 0xe3, M6809_OP_INDEXED, 2, 6, 6 },
  { "andb", 0xe4, M6809_OP_INDEXED, 2, 4, 4 },
  { "bitb", 0xe5, M6809_OP_INDEXED, 2, 4, 4 },
  { "ldb", 0xe6, M6809_OP_INDEXED, 2, 4, 4 },
  { "stb", 0xe7, M6809_OP_INDEXED, 2, 4, 4 },
  { "eorb", 0xe8, M6809_OP_INDEXED, 2, 4, 4 },
  { "adcb", 0xe9, M6809_OP_INDEXED, 2, 4, 4 },
  { "orb", 0xea, M6809_OP_INDEXED, 2, 4, 4 },
  { "addb", 0xeb, M6809_OP_INDEXED, 2, 4, 4 },
  { "ldd", 0xec, M6809_OP_INDEXED, 2, 5, 5 },
  { "std", 0xed, M6809_OP_INDEXED, 2, 5, 5 },
  { "ldu", 0xee, M6809_OP_INDEXED, 2, 5, 5 },
  { "stu", 0xef, M6809_OP_INDEXED, 2, 5, 5 },
  { "subb", 0xf0, M6809_OP_EXTENDED, 3, 5, 5 },
  { "cmpb", 0xf1, M6809_OP_EXTENDED, 3, 5, 5 },
  { "sbcb", 0xf2, M6809_OP_EXTENDED, 3, 5, 5 },
  { "addd", 0xf3, M6809_OP_EXTENDED, 3, 7, 7 },
  { "andb", 0xf4, M6809_OP_EXTENDED, 3, 5, 5 },
  { "bitb", 0xf5, M6809_OP_EXTENDED, 3, 5, 5 },
  { "ldb", 0xf6, M6809_OP_EXTENDED, 3, 5, 5 },
  { "stb", 0xf7, M6809_OP_EXTENDED, 3, 5, 5 },
  { "eorb", 0xf8, M6809_OP_EXTENDED, 3, 5, 5 },
  { "adcb", 0xf9, M6809_OP_EXTENDED, 3, 5, 5 },
  { "orb", 0xfa, M6809_OP_EXTENDED, 3, 5, 5 },
  { "addb", 0xfb, M6809_OP_EXTENDED, 3, 5, 5 },
  { "ldd", 0xfc, M6809_OP_EXTENDED, 3, 6, 6 },
  { "std", 0xfd, M6809_OP_EXTENDED, 3, 6, 6 },
  { "ldu", 0xfe, M6809_OP_EXTENDED, 3, 6, 6 },
  { "stu", 0xff, M6809_OP_EXTENDED, 3, 6, 6 },
  { NULL, 0x00, M6809_OP_ILLEGAL, 0, 0 },
};

struct _table_6809 table_6809_16[] = {
  { "lbrn", 0x1021, M6809_OP_RELATIVE, 4, 5, 6 },
  { "lbhi", 0x1022, M6809_OP_RELATIVE, 4, 5, 6 },
  { "lbls", 0x1023, M6809_OP_RELATIVE, 4, 5, 6 },
  { "lbhs", 0x1024, M6809_OP_RELATIVE, 4, 5, 6 },
  { "lblo", 0x1025, M6809_OP_RELATIVE, 4, 5, 6 },
  { "lbne", 0x1026, M6809_OP_RELATIVE, 4, 5, 6 },
  { "lbeq", 0x1027, M6809_OP_RELATIVE, 4, 5, 6 },
  { "lbvc", 0x1028, M6809_OP_RELATIVE, 4, 5, 6 },
  { "lbvs", 0x1029, M6809_OP_RELATIVE, 4, 5, 6 },
  { "lbpl", 0x102a, M6809_OP_RELATIVE, 4, 5, 6 },
  { "lbmi", 0x102b, M6809_OP_RELATIVE, 4, 5, 6 },
  { "lbge", 0x102c, M6809_OP_RELATIVE, 4, 5, 6 },
  { "lblt", 0x102d, M6809_OP_RELATIVE, 4, 5, 6 },
  { "lbgt", 0x102e, M6809_OP_RELATIVE, 4, 5, 6 },
  { "lble", 0x102f, M6809_OP_RELATIVE, 4, 5, 6 },
  { "swi2", 0x103f, M6809_OP_INHERENT, 2, 20, 20 },
  { "cmpd", 0x1083, M6809_OP_IMMEDIATE, 4, 5, 5 },
  { "cmpy", 0x108c, M6809_OP_IMMEDIATE, 4, 5, 5 },
  { "ldy", 0x108e, M6809_OP_IMMEDIATE, 4, 4, 4 },
  { "cmpd", 0x1093, M6809_OP_DIRECT, 3, 7, 7 },
  { "cmpy", 0x109c, M6809_OP_DIRECT, 3, 7, 7 },
  { "ldy", 0x109e, M6809_OP_DIRECT, 3, 6, 6 },
  { "sty", 0x109f, M6809_OP_DIRECT, 3, 6, 6 },
  { "cmpd", 0x10a3, M6809_OP_INDEXED, 3, 7, 7 },
  { "cmpy", 0x10ac, M6809_OP_INDEXED, 3, 7, 7 },
  { "ldy", 0x10ae, M6809_OP_INDEXED, 3, 6, 6 },
  { "sty", 0x10af, M6809_OP_INDEXED, 3, 6, 6 },
  { "cmpd", 0x10b3, M6809_OP_EXTENDED, 4, 8, 8 },
  { "cmpy", 0x10bc, M6809_OP_EXTENDED, 4, 8, 8 },
  { "ldy", 0x10be, M6809_OP_EXTENDED, 4, 7, 7 },
  { "sty", 0x10bf, M6809_OP_EXTENDED, 4, 7, 7 },
  { "lds", 0x10ce, M6809_OP_IMMEDIATE, 4, 4, 4 },
  { "lds", 0x10de, M6809_OP_DIRECT, 3, 6, 6 },
  { "sts", 0x10df, M6809_OP_DIRECT, 3, 6, 6 },
  { "lds", 0x10ee, M6809_OP_INDEXED, 3, 6, 6 },
  { "sts", 0x10ef, M6809_OP_INDEXED, 3, 6, 6 },
  { "lds", 0x10fe, M6809_OP_EXTENDED, 4, 7, 7 },
  { "sts", 0x10ff, M6809_OP_EXTENDED, 4, 7, 7 },
  { "swi3", 0x113f, M6809_OP_INHERENT, 2, 20, 20 },
  { "cmpu", 0x1183, M6809_OP_IMMEDIATE, 4, 5, 5 },
  { "cmps", 0x118c, M6809_OP_IMMEDIATE, 4, 5, 5 },
  { "cmpu", 0x1193, M6809_OP_DIRECT, 3, 7, 7 },
  { "cmps", 0x119c, M6809_OP_DIRECT, 3, 7, 7 },
  { "cmpu", 0x11a3, M6809_OP_INDEXED, 3, 7, 7 },
  { "cmps", 0x11ac, M6809_OP_INDEXED, 3, 7, 7 },
  { "cmpu", 0x11b3, M6809_OP_EXTENDED, 4, 8, 8 },
  { "cmps", 0x11bc, M6809_OP_EXTENDED, 4, 8, 8 },
  { NULL, 0x0000, M6809_OP_ILLEGAL, 0, 0 },
};

