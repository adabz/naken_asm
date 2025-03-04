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

#include "disasm/pdk13.h"
#include "table/pdk13.h"

int disasm_pdk13(
  Memory *memory,
  uint32_t address,
  char *instruction,
  int length,
  int *cycles_min,
  int *cycles_max)
{
  int opcode;
  int n;
  int m, k, bit;

  *cycles_min = -1;
  *cycles_max = -1;

  opcode = memory->read16(address);

  for (n = 0; table_pdk13[n].instr != NULL; n++)
  {
    if ((opcode & table_pdk13[n].mask) == table_pdk13[n].opcode)
    {
      *cycles_min = table_pdk13[n].cycles_min;
      *cycles_max = table_pdk13[n].cycles_min;

      switch (table_pdk13[n].type)
      {
        case OP_NONE:
          strcpy(instruction, table_pdk13[n].instr);
          return 2;
        case OP_A:
          snprintf(instruction, length, "%s a", table_pdk13[n].instr);
          return 2;
        case OP_IO_A:
          m = opcode & 0x1f;
          snprintf(instruction, length, "%s %d, a", table_pdk13[n].instr, m);
          return 2;
        case OP_A_IO:
          m = opcode & 0x1f;
          snprintf(instruction, length, "%s a, %d", table_pdk13[n].instr, m);
          return 2;
        case OP_M:
          m = opcode & 0x3f;
          snprintf(instruction, length, "%s [%d]", table_pdk13[n].instr, m);
          return 2;
        case OP_M4:
          m = opcode & 0x1e;
          snprintf(instruction, length, "%s [%d]", table_pdk13[n].instr, m);
          return 2;
        case OP_K8:
          k = opcode & 0xff;
          snprintf(instruction, length, "%s 0x%02x", table_pdk13[n].instr, k);
          return 2;
        case OP_A_M:
          m = opcode & 0x3f;
          snprintf(instruction, length, "%s a, [%d]", table_pdk13[n].instr, m);
          return 2;
        case OP_M_A:
          m = opcode & 0x3f;
          snprintf(instruction, length, "%s [%d], a", table_pdk13[n].instr, m);
          return 2;
        case OP_A_M4:
          bit = (opcode >> 6) & 0x7;
          m = opcode & 0x1e;
          snprintf(instruction, length, "%s a, [%d].%d", table_pdk13[n].instr, m, bit);
          return 2;
        case OP_M4_A:
          bit = (opcode >> 6) & 0x7;
          m = opcode & 0x1e;
          snprintf(instruction, length, "%s [%d].%d, a", table_pdk13[n].instr, m, bit);
          return 2;
        case OP_IO_N:
          bit = (opcode >> 5) & 0x7;
          m = opcode & 0x1f;
          snprintf(instruction, length, "%s %d.%d", table_pdk13[n].instr, m, bit);
          return 2;
        case OP_M_N:
          bit = (opcode >> 5) & 0x7;
          m = opcode & 0xf;
          snprintf(instruction, length, "%s [%d].%d", table_pdk13[n].instr, m, bit);
          return 2;
        case OP_A_K:
          k = opcode & 0xff;
          snprintf(instruction, length, "%s a, %d", table_pdk13[n].instr, k);
          return 2;
        case OP_K10:
          k = opcode & 0x3ff;
          snprintf(instruction, length, "%s 0x%03x", table_pdk13[n].instr, k);
          return 2;
        default:
        {
          //print_error_internal(asm_context, __FILE__, __LINE__);
          break;
        }
      }
    }
  }

  strcpy(instruction, "???");

  return 2;
}

void list_output_pdk13(
  AsmContext *asm_context,
  uint32_t start,
  uint32_t end)
{
  int cycles_min, cycles_max;
  uint32_t opcode;
  char instruction[128];
  int count;

  Memory *memory = &asm_context->memory;

  fprintf(asm_context->list, "\n");

  while (start < end)
  {
    count = disasm_pdk13(
      memory,
      start,
      instruction,
      sizeof(instruction),
      &cycles_min,
      &cycles_max);

    opcode = memory->read16(start);

    fprintf(asm_context->list, "0x%04x: 0x%04x %-40s cycles: ", start / 2, opcode, instruction);

    if (cycles_min == 0)
    {
      fprintf(asm_context->list, "?\n");
    }
      else
    if (cycles_min == cycles_max)
    {
      fprintf(asm_context->list, "%d\n", cycles_min);
    }
      else
    {
      fprintf(asm_context->list, "%d-%d\n", cycles_min, cycles_max);
    }

    start += count;
  }
}

void disasm_range_pdk13(
  Memory *memory,
  uint32_t flags,
  uint32_t start,
  uint32_t end)
{
  char instruction[128];
  int cycles_min = 0,cycles_max = 0;
  uint16_t opcode;

  printf("\n");

  printf("%-7s %-5s %-40s Cycles\n", "Addr", "Opcode", "Instruction");
  printf("------- ------ ----------------------------------       ------\n");

  while (start <= end)
  {
    disasm_pdk13(
      memory,
      start,
      instruction,
      sizeof(instruction),
      &cycles_min,
      &cycles_max);

    opcode = memory->read16(start);

    printf("0x%04x: 0x%04x %-40s ", start / 2, opcode, instruction);

    if (cycles_min == 0)
    {
      printf("?\n");
    }
      else
    if (cycles_min == cycles_max)
    {
      printf("%d\n", cycles_min);
    }
      else
    {
      printf("%d-%d\n", cycles_min, cycles_max);
    }

    start = start + 2;
  }
}

