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

#include "disasm/lc3.h"
#include "table/lc3.h"

#define READ_RAM16(a) (memory->read8(a + 0) << 8) | memory->read8(a + 1)

static const char *br[] =
{
  "???",
  "p",
  "z",
  "zp",
  "n",
  "np",
  "nz",
  "",
};

int disasm_lc3(
  Memory *memory,
  uint32_t address,
  char *instruction,
  int length,
  int *cycles_min,
  int *cycles_max)
{
  uint16_t opcode;
  int n;

  opcode = READ_RAM16(address);

  *cycles_min = -1;
  *cycles_max = -1;

  n = 0;
  while (table_lc3[n].instr != NULL)
  {
    if ((opcode & table_lc3[n].mask) == table_lc3[n].opcode)
    {
      int r0 = (opcode >> 9) & 0x7;
      int r1 = (opcode >> 6) & 0x7;
      int r2 = opcode & 0x7;

      switch (table_lc3[n].type)
      {
        case OP_NONE:
        {
          strcpy(instruction, table_lc3[n].instr);
          return 2;
        }
        case OP_R_R_R:
        {
          snprintf(instruction, length, "%s r%d, r%d, r%d", table_lc3[n].instr, r0, r1, r2);
          return 2;
        }
        case OP_R_R_IMM5:
        {
          int16_t imm5 = opcode & 0x1f;
          if ((imm5 & 0x10) != 0) { imm5 |= 0xffe0; }

          snprintf(instruction, length, "%s r%d, r%d, #%d", table_lc3[n].instr, r0, r1, imm5);
          return 2;
        }
        case OP_BR:
        {
          const char *nzp = br[(opcode >> 9) & 0x7];
          int16_t offset9 = opcode & 0x1ff;
          if ((offset9 & 0x100) != 0) { offset9 |= 0xfe00; }

          snprintf(instruction, length, "%s%s 0x%04x (offset=%d)", table_lc3[n].instr, nzp, (address / 2) + 1 + offset9, offset9);

          return 2;
        }
        case OP_BASER:
        {
          snprintf(instruction, length, "%s r%d", table_lc3[n].instr, r1);
          return 2;
        }
        case OP_OFFSET11:
        {
          uint16_t offset11 = opcode & 0x7ff;
          if ((offset11 & 0x400) != 0) { offset11 |= 0xfc00; }

          snprintf(instruction, length, "%s 0x%04x (offset=%d)", table_lc3[n].instr, (address / 2) + 1 + offset11, offset11);
          return 2;
        }
        case OP_R_OFFSET9:
        {
          uint16_t offset9 = opcode & 0x1ff;
          if ((offset9 & 0x100) != 0) { offset9 |= 0xfe00; }

          snprintf(instruction, length, "%s r%d, 0x%04x (offset=%d)", table_lc3[n].instr, r0, (address / 2) + 1 + offset9, offset9);
          return 2;
        }
        case OP_R_R_OFFSET6:
        {
          int16_t offset6 = opcode & 0x03f;
          if ((offset6 & 0x20) != 0) { offset6 |= 0xffc0; }

          snprintf(instruction, length, "%s r%d, r%d, #%d", table_lc3[n].instr, r0, r1, offset6);
          return 2;
        }
        case OP_R_R:
        {
          snprintf(instruction, length, "%s r%d, r%d", table_lc3[n].instr, r0, r1);
          return 2;
        }
        case OP_VECTOR:
        {
          int vector = opcode & 0xff;
          snprintf(instruction, length, "%s %d", table_lc3[n].instr, vector);
          return 2;
        }
        default:
        {
          //print_error_internal(asm_context, __FILE__, __LINE__);
          break;
        }
      }
    }

    n++;
  }

  strcpy(instruction, "???");

  return 2;
}

void list_output_lc3(
  AsmContext *asm_context,
  uint32_t start,
  uint32_t end)
{
  char instruction[128];
  int cycles_min, cycles_max;
  uint16_t opcode;
  int count;

  Memory *memory = &asm_context->memory;

  fprintf(asm_context->list, "\n");

  while (start < end)
  {
    count = disasm_lc3(
      memory,
      start,
      instruction,
      sizeof(instruction),
      &cycles_min,
      &cycles_max);

    opcode = (memory->read8(start + 0) << 8) | memory->read8(start + 1);

    fprintf(asm_context->list, "0x%04x: %04x %-40s\n", start / 2, opcode, instruction);

    start += count;
  }
}

void disasm_range_lc3(
  Memory *memory,
  uint32_t flags,
  uint32_t start,
  uint32_t end)
{
  char instruction[128];
  int cycles_min, cycles_max;
  uint16_t opcode;
  int count;

  printf("\n");

  printf("%-7s %-5s %-40s\n", "Addr", "Opcode", "Instruction");
  printf("------- ------ ----------------------------------       ------\n");

  while (start <= end)
  {
    count = disasm_lc3(
      memory,
      start,
      instruction,
      sizeof(instruction),
      &cycles_min,
      &cycles_max);

    opcode = READ_RAM16(start);

    printf("0x%04x: %04x %-40s\n", start / 2, opcode, instruction);

    start = start + count;
  }
}

