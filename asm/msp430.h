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

#ifndef NAKEN_ASM_ASM_MSP430_H
#define NAKEN_ASM_ASM_MSP430_H

#include "common/assembler.h"

int parse_instruction_msp430(struct _asm_context *asm_context, char *instr);

int link_function_msp430(
  struct _asm_context *asm_context,
  Imports *imports,
  const uint8_t *code,
  uint32_t function_offset,
  int size,
  uint8_t *obj_file,
  uint32_t obj_size);

#endif

