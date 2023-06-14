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

#ifndef NAKEN_ASM_DISASM_AVR8_H
#define NAKEN_ASM_DISASM_AVR8_H

#include "common/assembler.h"

int get_register_avr8(char *token);

int disasm_avr8(
  Memory *memory,
  uint32_t address,
  char *instruction,
  int *cycles_min,
  int *cycles_max);

void list_output_avr8(
  struct _asm_context *asm_context,
  uint32_t start,
  uint32_t end);

void disasm_range_avr8(
  Memory *memory,
  uint32_t flags,
  uint32_t start,
  uint32_t end);

#endif

