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

#include "disasm/riscv.h"
#include "table/riscv.h"

static const char *rm_string[] =
{
  ", rne",
  ", rtz",
  ", rdn",
  ", rup",
  ", rmm",
  ", [error]",
  ", [error]",
  "",
};

static const char *fence_string[] =
{
  "sw",
  "sr",
  "so",
  "si",
  "pw",
  "pr",
  "po",
  "pi",
};

int disasm_riscv(
  Memory *memory,
  uint32_t address,
  char *instruction,
  int length,
  int *cycles_min,
  int *cycles_max)
{
  uint32_t opcode;
  uint32_t immediate;
  int32_t offset;
  int32_t simmediate;
  int n;
  char temp[16];
  int count, i;

  *cycles_min = -1;
  *cycles_max = -1;

  opcode = memory->read32(address);

  n = 0;
  while (table_riscv[n].instr != NULL)
  {
    if ((opcode & table_riscv[n].mask) == table_riscv[n].opcode)
    {
      uint32_t rd = (opcode >> 7) & 0x1f;
      uint32_t rs1 = (opcode >> 15) & 0x1f;
      uint32_t rs2 = (opcode >> 20) & 0x1f;
      uint32_t rs3 = (opcode >> 27) & 0x1f;
      uint32_t rm = (opcode >> 12) & 0x7;
      const char *instr = table_riscv[n].instr;

      switch (table_riscv[n].type)
      {
        case OP_R_TYPE:
          snprintf(instruction, length, "%s x%d, x%d, x%d", instr, rd, rs1, rs2);
          break;
        case OP_R_R:
          snprintf(instruction, length, "%s x%d, x%d", instr, rd, rs1);
          break;
        case OP_I_TYPE:
          immediate = opcode >> 20;
          simmediate = immediate;
          if ((simmediate & 0x800) != 0) { simmediate |= 0xfffff000; }
          snprintf(instruction, length, "%s x%d, x%d, %d (0x%06x)", instr, rd, rs1, simmediate, immediate);
          break;
        case OP_UI_TYPE:
          immediate = opcode >> 20;
          snprintf(instruction, length, "%s x%d, 0x%x", instr, rd, immediate);
          break;
        case OP_SB_TYPE:
          immediate = ((opcode >> 31) & 0x1) << 12;
          immediate |= ((opcode >> 8) & 0xf) << 1;
          immediate |= ((opcode >> 7) & 0x1) << 11;
          immediate |= ((opcode >> 25) & 0x3f) << 5;
          if ((immediate & 0x1000) != 0) { immediate |= 0xffffe000; }
          snprintf(instruction, length, "%s x%d, x%d, 0x%x (%d)", instr, rs1, rs2, address + immediate, immediate);
          break;
        case OP_U_TYPE:
          immediate = opcode >> 12;
          snprintf(instruction, length, "%s x%d, 0x%06x", instr, rd, immediate);
          break;
        case OP_UJ_TYPE:
          offset = ((opcode >> 31) & 0x1) << 20;
          offset |= ((opcode >> 12) & 0xff) << 12;
          offset |= ((opcode >> 20) & 0x1) << 11;
          offset |= ((opcode >> 21) & 0x3ff) << 1;
          if ((offset & 0x100000) != 0) { offset |= 0xfff00000; }
          snprintf(instruction, length, "%s x%d, 0x%x (offset=%d)", instr, rd, address + offset, offset);
          break;
        case OP_SHIFT:
          immediate = (opcode >> 20) & 0x1f;
          snprintf(instruction, length, "%s x%d, x%d, %d", instr, rd, rs1, immediate);
          break;
        case OP_FENCE:
          immediate = (opcode >> 20) & 0xff;
          count = 0;
          snprintf(instruction, length, "%s", instr);
          for (i = 7; i >= 0; i--)
          {
            if ((immediate & (1 << i)) != 0)
            {
              if (count == 0) { strcat(instruction, " "); }
              else { strcat(instruction, ", "); }
              strcat(instruction, fence_string[i]);
              count++;
            }
          }
          break;
        case OP_FFFF:
          snprintf(instruction, length, "%s", instr);
          break;
        case OP_READ:
          snprintf(instruction, length, "%s x%d", instr, rd);
          break;
        case OP_RD_INDEX_R:
          immediate = opcode >> 20;
          simmediate = immediate;
          if ((simmediate & 0x800) != 0) { simmediate |= 0xfffff000; }
          snprintf(instruction, length, "%s x%d, %d(x%d)", instr, rd, simmediate, rs1);
          break;
        case OP_FD_INDEX_R:
          immediate = opcode >> 20;
          simmediate = immediate;
          if ((simmediate & 0x800) != 0) { simmediate |= 0xfffff000; }
          snprintf(instruction, length, "%s f%d, %d(x%d)", instr, rd, simmediate, rs1);
          break;
        case OP_RS_INDEX_R:
          immediate = ((opcode >> 25) & 0x7f) << 5;
          immediate |= ((opcode >> 7) & 0x1f);
          if ((immediate & 0x800) != 0) { immediate |= 0xfffff000; }
          snprintf(instruction, length, "%s x%d, %d(x%d)", instr, rs2, immediate, rs1);
          break;
        case OP_FS_INDEX_R:
          immediate = ((opcode >> 25) & 0x7f) << 5;
          immediate |= ((opcode >> 7) & 0x1f);
          if ((immediate & 0x800) != 0) { immediate |= 0xfffff000; }
          snprintf(instruction, length, "%s f%d, %d(x%d)", instr, rs2, immediate, rs1);
          break;
        case OP_LR:
          temp[0] = 0;
          if ((opcode & (1 << 26)) != 0) { strcat(temp, ".aq"); }
          if ((opcode & (1 << 25)) != 0) { strcat(temp, ".rl"); }
          snprintf(instruction, length, "%s%s x%d, (x%d)", instr, temp, rd, rs1);
          break;
        case OP_STD_EXT:
          temp[0] = 0;
          if ((opcode & (1 << 26)) != 0) { strcat(temp, ".aq"); }
          if ((opcode & (1 << 25)) != 0) { strcat(temp, ".rl"); }
          // FIXME - The docs say rs2 and rs1 are reversed. gnu-as is like this.
          snprintf(instruction, length, "%s%s x%d, x%d, (x%d)", instr, temp, rd, rs2, rs1);
          break;
        case OP_R_FP_RM:
          snprintf(instruction, length, "%s x%d, f%d%s", instr, rd, rs1, rm_string[rm]);
          break;
        case OP_R_FP_FP:
          snprintf(instruction, length, "%s x%d, f%d, f%d", instr, rd, rs1, rs2);
          break;
        case OP_FP:
          snprintf(instruction, length, "%s f%d", instr, rd);
          break;
        case OP_FP_FP:
          snprintf(instruction, length, "%s f%d, f%d", instr, rd, rs1);
          break;
        case OP_FP_FP_FP:
          snprintf(instruction, length, "%s f%d, f%d, f%d", instr, rd, rs1, rs3);
          break;
        case OP_FP_FP_RM:
          snprintf(instruction, length, "%s f%d, f%d%s", instr, rd, rs1, rm_string[rm]);
          break;
        case OP_FP_R:
          snprintf(instruction, length, "%s f%d, x%d", instr, rd, rs1);
          break;
        case OP_FP_R_RM:
          snprintf(instruction, length, "%s f%d, x%d%s", instr, rd, rs1, rm_string[rm]);
          break;
        case OP_FP_FP_FP_RM:
          snprintf(instruction, length, "%s f%d, f%d, f%d%s", instr, rd, rs1, rs2, rm_string[rm]);
          break;
        case OP_FP_FP_FP_FP_RM:
          snprintf(instruction, length, "%s f%d, f%d, f%d, f%d%s", instr, rd, rs1, rs2, rs3, rm_string[rm]);
          break;
        default:
          strcpy(instruction, "???");
          break;
      }

      return 4;
    }

    n++;
  }

  strcpy(instruction, "???");

  return -1;
}

void list_output_riscv(
  AsmContext *asm_context,
  uint32_t start,
  uint32_t end)
{
  int cycles_min,cycles_max;
  char instruction[128];
  uint32_t opcode;

  Memory *memory = &asm_context->memory;

  fprintf(asm_context->list, "\n");

  while (start < end)
  {
    opcode = memory->read32(start);

    disasm_riscv(
      memory,
      start,
      instruction,
      sizeof(instruction),
      &cycles_min,
      &cycles_max);

    fprintf(asm_context->list, "0x%08x: 0x%08x %-40s cycles: ",
      start, opcode, instruction);

    if (cycles_min == -1)
    {
      fprintf(asm_context->list, "\n");
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

    start += 4;
  }
}

void disasm_range_riscv(
  Memory *memory,
  uint32_t flags,
  uint32_t start,
  uint32_t end)
{
  char instruction[128];
  uint32_t opcode;
  int cycles_min = 0, cycles_max = 0;
  int count;

  printf("\n");

  printf("%-7s %-5s %-40s Cycles\n", "Addr", "Opcode", "Instruction");
  printf("------- ------ ----------------------------------       ------\n");

  while (start <= end)
  {
    opcode = memory->read32(start);

    count = disasm_riscv(
      memory,
      start,
      instruction,
      sizeof(instruction),
      &cycles_min,
      &cycles_max);

    printf("0x%08x: 0x%08x %-40s cycles: ", start, opcode, instruction);

    if (cycles_min == -1)
    {
      printf("\n");
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

    start = start + count;
  }
}

