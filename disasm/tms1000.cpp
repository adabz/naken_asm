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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "disasm/tms1000.h"
#include "table/tms1000.h"

#define READ_RAM(a) \
  memory->read8((a & 0xfc0) | tms1000_address_to_lsfr[a & 0x3f])

#if 0
int get_cycle_count_tms1000(uint16_t opcode)
{
  return 6;
}

int get_cycle_count_tms1100(uint16_t opcode)
{
  return 6;
}
#endif

static void compute_address(int address, int *chapter, int *page, int *pc)
{
  *chapter = (address >> 10) & 3;
  *page = (address >> 6) & 0xf;
  *pc = address & 0x3f;
}

int disasm_tms1000(
  Memory *memory,
  uint32_t address,
  char *instruction,
  int length,
  int *cycles_min,
  int *cycles_max)
{
  int bit_instr;
  int opcode;
  int n, c;

  *cycles_min = 6;
  *cycles_max = 6;

  opcode = READ_RAM(address);

  n = 0;
  while (table_tms1000[n].instr != NULL)
  {
    if (opcode == table_tms1000[n].op1000)
    {
      strcpy(instruction, table_tms1000[n].instr);
      return 1;
    }
    n++;
  }

  bit_instr = opcode >> 2;
  c = tms1000_reverse_bit_address[opcode & 0x3];

  if (bit_instr == 0xc) { snprintf(instruction, length, "sbit %d", c); return 1; }
    else
  if (bit_instr == 0xd) { snprintf(instruction, length, "rbit %d", c); return 1; }
    else
  if (bit_instr == 0xe) { snprintf(instruction, length, "tbit1 %d", c); return 1;}
    else
  if (bit_instr == 0xf) { snprintf(instruction, length, "ldx %d", c); return 1; }

  bit_instr = opcode >> 4;
  c = tms1000_reverse_constant[opcode & 0xf];

  if (bit_instr == 0x4) { snprintf(instruction, length, "tcy %d", c); return 1; }
    else
  if (bit_instr == 0x6) { snprintf(instruction, length, "tcmiy %d", c); return 1;}
    else
  if (bit_instr == 0x1) { snprintf(instruction, length, "ldp %d", c); return 1; }
    else
  if (bit_instr == 0x7) { snprintf(instruction, length, "alec %d", c); return 1; }
    else
  if (bit_instr == 0x5) { snprintf(instruction, length, "ynec %d", c); return 1; }

  bit_instr = opcode >> 6;
  uint8_t branch_address = opcode & 0x3f;

  if (bit_instr == 0x2)
  {
    snprintf(instruction, length, "br 0x%02x  (linear_address=0x%02x)",
      branch_address,
      tms1000_lsfr_to_address[branch_address]);
    return 1;
  }
    else
  if (bit_instr == 0x3)
  {
    snprintf(instruction, length, "call 0x%02x  (linear_address=0x%02x)",
      branch_address,
      tms1000_lsfr_to_address[branch_address]);
    return 1;
  }

  strcpy(instruction, "???");

  return 1;
}

int disasm_tms1100(
  Memory *memory,
  uint32_t address,
  char *instruction,
  int length,
  int *cycles_min,
  int *cycles_max)
{
  int bit_instr;
  int opcode;
  int n, c;

  *cycles_min = 6;
  *cycles_max = 6;

  opcode = READ_RAM(address);

  n = 0;
  while (table_tms1000[n].instr != NULL)
  {
    if (opcode == table_tms1000[n].op1100)
    {
      strcpy(instruction, table_tms1000[n].instr);
      return 1;
    }
    n++;
  }

  bit_instr = opcode >> 2;
  c = tms1000_reverse_bit_address[opcode & 0x3];

  if (bit_instr == 0xc) { snprintf(instruction, length, "sbit %d", c); return 1; }
    else
  if (bit_instr == 0xd) { snprintf(instruction, length, "rbit %d", c); return 1; }
    else
  if (bit_instr == 0xe) { snprintf(instruction, length, "tbit1 %d", c); return 1; }

  bit_instr = opcode >> 3;
  c = tms1000_reverse_constant[opcode & 0x7] >> 1;

  if (bit_instr == 0x5) { snprintf(instruction, length, "ldx %d", c); return 1; }

  bit_instr = opcode >> 4;
  c = tms1000_reverse_constant[opcode & 0xf];

  if (bit_instr == 0x4) { snprintf(instruction, length, "tcy %d", c); return 1; }
    else
  if (bit_instr == 0x6) { snprintf(instruction, length, "tcmiy %d", c); return 1; }
    else
  if (bit_instr == 0x1) { snprintf(instruction, length, "ldp %d", c); return 1; }
    else
  if (bit_instr == 0x5) { snprintf(instruction, length, "ynec %d", c); return 1; }

  bit_instr = opcode >> 6;
  uint8_t branch_address = opcode & 0x3f;
  //if ((offset & 0x20) != 0) { offset |= 0xc0; }
  //int branch_address = (address + 1) + offset;

  if (bit_instr == 0x2)
  {
     snprintf(instruction, length, "br 0x%02x (linear_address=0x%02x)",
       branch_address,
       tms1000_lsfr_to_address[branch_address]);
     return 1;
  }
    else
  if (bit_instr == 0x3)
  {
    snprintf(instruction, length, "call 0x%02x (linear_address=0x%02x)",
      branch_address,
      tms1000_lsfr_to_address[branch_address]);
    return 1;
  }

  strcpy(instruction, "???");

  return 1;
}

void list_output_tms1000(
  AsmContext *asm_context,
  uint32_t start,
  uint32_t end)
{
  int cycles_min,cycles_max;
  char instruction[128];
  Memory *memory = &asm_context->memory;
  uint32_t opcode = READ_RAM(start);
  int chapter;
  int page;
  int pc;
  uint8_t lsfr;

  compute_address(start, &chapter, &page, &pc);

  lsfr = tms1000_address_to_lsfr[pc];

  disasm_tms1000(
    &asm_context->memory,
    start,
    instruction,
    sizeof(instruction),
    &cycles_min,
    &cycles_max);

  fprintf(asm_context->list, "%03x %x/%02x: %02x %-40s cycles: ", start, page, lsfr, opcode, instruction);

  if (cycles_min == cycles_max)
  {
    fprintf(asm_context->list, "%d\n", cycles_min);
  }
    else
  {
    fprintf(asm_context->list, "%d-%d\n", cycles_min, cycles_max);
  }
}

void list_output_tms1100(
  AsmContext *asm_context,
  uint32_t start,
  uint32_t end)
{
  int cycles_min,cycles_max;
  char instruction[128];
  Memory *memory = &asm_context->memory;
  uint32_t opcode = READ_RAM(start);
  int chapter;
  int page;
  int pc;
  uint8_t lsfr;

  compute_address(start, &chapter, &page, &pc);

  lsfr = tms1000_address_to_lsfr[pc];

  disasm_tms1100(
    &asm_context->memory,
    start,
    instruction,
    sizeof(instruction),
    &cycles_min,
    &cycles_max);

  fprintf(asm_context->list, "%03x %d/%x/%02x: %02x %-40s cycles: ", start, chapter, page, lsfr, opcode, instruction);

  if (cycles_min == cycles_max)
  {
    fprintf(asm_context->list, "%d\n", cycles_min);
  }
    else
  {
    fprintf(asm_context->list, "%d-%d\n", cycles_min, cycles_max);
  }
}

void disasm_range_tms1000(
  Memory *memory,
  uint32_t flags,
  uint32_t start,
  uint32_t end)
{
  char instruction[128];
  int cycles_min = 0, cycles_max = 0;
  int chapter;
  int page;
  int pc;
  int num;

  printf("\n");

  printf("%-4s %-4s %-5s %-40s Cycles\n", "Linr", "Addr", "Opcode", "Instruction");
  printf("---- ---- ------ ----------------------------------       ------\n");

  while (start <= end)
  {
    num = READ_RAM(start);

    disasm_tms1000(
      memory,
      start,
      instruction,
      sizeof(instruction),
      &cycles_min,
      &cycles_max);

    compute_address(start, &chapter, &page, &pc);

    uint8_t lsfr = tms1000_address_to_lsfr[pc];

    if (cycles_min < 1)
    {
      printf("%03x  %x/%02x: %02x    %-40s ?\n", start, page, lsfr, num, instruction);
    }
      else
    if (cycles_min == cycles_max)
    {
      printf("%03x  %x/%02x: %02x    %-40s %d\n", start, page, lsfr, num, instruction, cycles_min);
    }
      else
    {
      printf("%03x  %x/%02x: %02x    %-40s %d-%d\n", start, page, lsfr, num, instruction, cycles_min, cycles_max);
    }

    start++;
  }
}

void disasm_range_tms1100(
  Memory *memory,
  uint32_t flags,
  uint32_t start,
  uint32_t end)
{
  char instruction[128];
  int cycles_min = 0, cycles_max = 0;
  int chapter;
  int page;
  int pc;
  int num;

  printf("\n");

  printf("%-4s %-4s   %-5s %-40s Cycles\n", "Linr", "Addr", "Opcode", "Instruction");
  printf("---- ------ ------ ----------------------------------       ------\n");

  while (start <= end)
  {
    num = READ_RAM(start);

    disasm_tms1100(
      memory,
      start,
      instruction,
      sizeof(instruction),
      &cycles_min,
      &cycles_max);

    compute_address(start, &chapter, &page, &pc);

    uint8_t lsfr = tms1000_address_to_lsfr[pc];

    if (cycles_min < 1)
    {
      printf("%03x %d/%x/%02x: %02x     %-40s ?\n", start, chapter, page, lsfr, num, instruction);
    }
      else
    if (cycles_min == cycles_max)
    {
      printf("%03x %d/%x/%02x: %02x     %-40s %d\n", start, chapter, page, lsfr, num, instruction, cycles_min);
    }
      else
    {
      printf("%03x %d/%x/%02x: %02x     %-40s %d-%d\n", start, chapter, page, lsfr, num, instruction, cycles_min, cycles_max);
    }

    start++;
  }
}

