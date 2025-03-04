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

#include "disasm/propeller.h"
#include "table/propeller.h"

static const char *conditions[] =
{
  "if_never ",
  "if_a ",
  "if_z_and_nc ",
  "if_nc ",
  "if_c_and_nz ",
  "if_ne ",
  "if_c_ne_z ",
  "if_nc_or_nz ",
  "if_z_and_c ",
  "if_z_eq_c ",
  "if_e ",
  "if_z_or_nc ",
  "if_c ",
  "if_c_or_nz ",
  "if_be ",
  "",
};

int disasm_propeller(
  Memory *memory,
  uint32_t address,
  char *instruction,
  int length,
  int *cycles_min,
  int *cycles_max)
{
  int opcode;
  int n;
  int i, d, s;
  int cond;
  int wz, wc, wr;

  *cycles_min = -1;
  *cycles_max = -1;

  opcode = memory->read32(address);

  i = (opcode & 0x00400000) >> 22;
  s = opcode & 0x1ff;
  d = (opcode >> 9) & 0x1ff;
  wz = (opcode >> 25) & 1;
  wc = (opcode >> 24) & 1;
  wr = (opcode >> 23) & 1;
  char effects[32];
  cond = (opcode >> 18) & 0xf;
  const char *condition = conditions[cond];

  effects[0] = 0;

  n = 0;
  while (table_propeller[n].instr != NULL)
  {
    uint32_t op_no_effects = table_propeller[n].opcode & //0xfc7fffff;
       ((table_propeller[n].mask & 0x03800000) ^ 0xfc7fffff);

    if ((opcode & table_propeller[n].mask) == op_no_effects)
    {
      *cycles_min = table_propeller[n].cycles_min;
      *cycles_max = table_propeller[n].cycles_min;

      if (wz == 1 && (table_propeller[n].opcode & 0x02000000) == 0)
      {
        if (effects[0] == 0) { strcat(effects, ","); }
        strcat(effects, " wz");
      }

      if (wc == 1 && (table_propeller[n].opcode & 0x01000000) == 0)
      {
        if (effects[0] == 0) { strcat(effects, ","); }
        strcat(effects, " wc");
      }

      if (wr == 1 && (table_propeller[n].opcode & 0x00800000) == 0)
      {
        if (effects[0] == 0) { strcat(effects, ","); }
        strcat(effects, " wr");
      }

      if (wr == 0 && (table_propeller[n].opcode & 0x00800000) != 0)
      {
        if (effects[0] == 0) { strcat(effects, ","); }
        strcat(effects, " nr");
      }

      switch (table_propeller[n].type)
      {
        case PROPELLER_OP_NONE:
        {
          snprintf(instruction, length, "%s%s", condition, table_propeller[n].instr);
          strcat(instruction, effects);
          return 4;
        }
        case PROPELLER_OP_NOP:
        {
          strcpy(instruction, table_propeller[n].instr);
          strcat(instruction, effects);
          return 4;
        }
        case PROPELLER_OP_DS:
        case PROPELLER_OP_DS_15_1:
        case PROPELLER_OP_DS_15_2:
        {
          if (i == 0)
          {
            snprintf(instruction, length, "%s%s 0x%x, 0x%x", condition, table_propeller[n].instr, d, s);
          }
            else
          {
            snprintf(instruction, length, "%s%s 0x%x, #0x%x", condition, table_propeller[n].instr, d, s);
          }
          strcat(instruction, effects);
          return 4;
        }
        case PROPELLER_OP_S:
        {
          if (i == 0)
          {
            snprintf(instruction, length, "%s%s 0x%x", condition, table_propeller[n].instr, s);
          }
            else
          {
            snprintf(instruction, length, "%s%s #0x%x", condition, table_propeller[n].instr, s);
          }
          strcat(instruction, effects);
          return 4;
        }
        case PROPELLER_OP_D:
        {
          snprintf(instruction, length, "%s%s 0x%x", condition, table_propeller[n].instr, d);
          strcat(instruction, effects);
          return 4;
        }
        case PROPELLER_OP_IMMEDIATE:
        {
          snprintf(instruction, length, "%s%s #0x%x", condition, table_propeller[n].instr, s);
          strcat(instruction, effects);
          return 4;
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

  snprintf(instruction, length, "%s???", condition);

  return 4;
}

void list_output_propeller(
  AsmContext *asm_context,
  uint32_t start,
  uint32_t end)
{
  int cycles_min, cycles_max;
  uint32_t opcode;
  char instruction[128];
  char bytes[16];
  int count;

  Memory *memory = &asm_context->memory;

  fprintf(asm_context->list, "\n");

  while (start < end)
  {
    count = disasm_propeller(
      memory,
      start,
      instruction,
      sizeof(instruction),
      &cycles_min,
      &cycles_max);

    opcode = memory->read32(start);
    snprintf(bytes, sizeof(bytes), "0x%08x", opcode);

    fprintf(asm_context->list, "0x%04x: %-16s %-40s cycles: ", start / 4, bytes, instruction);

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
    if (cycles_max < 1)
    {
      fprintf(asm_context->list, "%d or %d\n", cycles_min, -cycles_max);
    }
      else
    {
      fprintf(asm_context->list, "%d-%d\n", cycles_min, cycles_max);
    }

    start += count;
  }
}

void disasm_range_propeller(
  Memory *memory,
  uint32_t flags,
  uint32_t start,
  uint32_t end)
{
  char instruction[128];
  char bytes[16];
  int cycles_min = 0,cycles_max = 0;

  printf("\n");

  printf("%-7s %-5s %-40s Cycles\n", "Addr", "Opcode", "Instruction");
  printf("------- ------ ----------------------------------       ------\n");

  while (start <= end)
  {
    disasm_propeller(
      memory,
      start,
      instruction,
      sizeof(instruction),
      &cycles_min,
      &cycles_max);

    snprintf(bytes, sizeof(bytes), "0x%08x", memory->read32(start));
    printf("0x%04x: %-16s %-40s ", start / 4, bytes, instruction);

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
    if (cycles_max < 1)
    {
      printf("%d or %d\n", cycles_min, -cycles_max);
    }
      else
    {
      printf("%d-%d\n", cycles_min, cycles_max);
    }

    start = start + 4;
  }
}

