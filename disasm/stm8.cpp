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

#include "disasm/stm8.h"
#include "table/stm8.h"

#define READ_RAM16(a) \
  ((memory->read8(a) << 8) | \
   (memory->read8(a + 1)))

#define READ_RAM24(a) \
  ((memory->read8(a + 0) << 16) | \
   (memory->read8(a + 1) << 8)  | \
   (memory->read8(a + 2)))

#define SINGLE_OPCODE(pre, op, cycles, size, instr) \
  if (opcode == op && prefix == pre) \
  { \
    strcpy(instruction, instr); \
    *cycles_min = cycles; \
    *cycles_max = cycles; \
    return size; \
  }

static void get_instruction(char *instr, int instr_enum)
{
  int n;

  n = 0;
  while (table_stm8[n].instr != NULL)
  {
    if (table_stm8[n].instr_enum == instr_enum)
    {
      strcpy(instr, table_stm8[n].instr);
      break;
    }
    n++;
  }
}

void add_reg(char *instr, int reg)
{
  switch (reg)
  {
    case OP_REG_A:
      strcat(instr, "A");
      break;
    case OP_REG_X:
      strcat(instr, "X");
      break;
    case OP_REG_Y:
      strcat(instr, "Y");
      break;
    case OP_REG_XL:
      strcat(instr, "XL");
      break;
    case OP_REG_YL:
      strcat(instr, "YL");
      break;
    case OP_REG_XH:
      strcat(instr, "XH");
      break;
    case OP_REG_YH:
      strcat(instr, "YH");
      break;
    case OP_REG_CC:
      strcat(instr, "CC");
      break;
    case OP_SP:
      strcat(instr, "SP");
      break;
    case OP_NONE:
    default:
      strcat(instr, "?");
      break;
  }
}

int disasm_stm8(
  Memory *memory,
  uint32_t address,
  char *instr,
  int length,
  int *cycles_min,
  int *cycles_max)
{
  uint8_t opcode;
  uint8_t prefix = 0;
  int8_t offset;
  char temp[128];
  int count = 1;
  int n;

  instr[0] = 0;

  *cycles_min = -1;
  *cycles_max = -1;

  opcode = memory->read8(address);

  if (opcode == 0x90 || opcode == 0x91 || opcode == 0x92 || opcode == 0x72)
  {
    prefix = opcode;
    opcode = memory->read8(address + count);
    count++;
  }

//printf("prefix=%x  opcode=%x\n", prefix, opcode);

  n = 0;
  while (table_stm8_opcodes[n].instr_enum != STM8_NONE)
  {
    if (table_stm8_opcodes[n].prefix == prefix)
    {
      if (table_stm8_opcodes[n].opcode == opcode)
      {
        break;
      }

      if (prefix != 0 && (table_stm8_opcodes[n].opcode & 0xf0) == 0x10 &&
          table_stm8_opcodes[n].opcode == (opcode & 0xf1))
      {
        break;
      }

      if (prefix == 0x72 && (table_stm8_opcodes[n].opcode & 0xf0) == 0x00 &&
          table_stm8_opcodes[n].opcode == (opcode & 0xf1))
      {
        break;
      }
    }
    n++;
  }

  // If instruction can't be found return ???.
  if (table_stm8_opcodes[n].instr_enum == STM8_NONE)
  {
    strcpy(instr, "???");
    return count;
  }

  *cycles_min = table_stm8_opcodes[n].cycles_min;
  *cycles_max = table_stm8_opcodes[n].cycles_max;

  get_instruction(instr, table_stm8_opcodes[n].instr_enum);

  if (table_stm8_opcodes[n].type != OP_NONE) { strcat(instr, " "); }

  if (table_stm8_opcodes[n].dest != 0)
  {
    add_reg(instr, table_stm8_opcodes[n].dest);

    if (table_stm8_opcodes[n].type != OP_NONE &&
        table_stm8_opcodes[n].type != OP_SINGLE_REGISTER)
    {
      strcat(instr, ", ");
    }
  }

  switch (table_stm8_opcodes[n].type)
  {
    case OP_NONE:
      break;
    case OP_NUMBER8:
      snprintf(temp, sizeof(temp), "#$%02x", memory->read8(address + count));
      strcat(instr, temp);
      count++;
      break;
    case OP_NUMBER16:
      snprintf(temp, sizeof(temp), "#$%x", READ_RAM16(address + count));
      strcat(instr, temp);
      count += 2;
      break;
    case OP_ADDRESS8:
      snprintf(temp, sizeof(temp), "$%02x", memory->read8(address + count));
      strcat(instr, temp);
      count++;
      break;
    case OP_ADDRESS16:
      snprintf(temp, sizeof(temp), "$%x", READ_RAM16(address + count));
      strcat(instr, temp);
      count += 2;
      break;
    case OP_ADDRESS24:
      snprintf(temp, sizeof(temp), "$%x", READ_RAM24(address + count));
      strcat(instr, temp);
      count += 3;
      break;
    case OP_INDEX_X:
      strcat(instr, "(X)");
      break;
    case OP_OFFSET8_INDEX_X:
      snprintf(temp, sizeof(temp), "($%02x,X)", memory->read8(address + count));
      strcat(instr, temp);
      count++;
      break;
    case OP_OFFSET16_INDEX_X:
      snprintf(temp, sizeof(temp), "($%x,X)", READ_RAM16(address + count));
      strcat(instr, temp);
      count += 2;
      break;
    case OP_OFFSET24_INDEX_X:
      snprintf(temp, sizeof(temp), "($%x,X)", READ_RAM24(address + count));
      strcat(instr, temp);
      count += 3;
      break;
    case OP_INDEX_Y:
      strcat(instr, "(Y)");
      break;
    case OP_OFFSET8_INDEX_Y:
      snprintf(temp, sizeof(temp), "($%02x,Y)", memory->read8(address + count));
      strcat(instr, temp);
      count++;
      break;
    case OP_OFFSET16_INDEX_Y:
      snprintf(temp, sizeof(temp), "($%x,Y)", READ_RAM16(address + count));
      strcat(instr, temp);
      count += 2;
      break;
    case OP_OFFSET24_INDEX_Y:
      snprintf(temp, sizeof(temp), "($%x,Y)", READ_RAM24(address + count));
      strcat(instr, temp);
      count += 3;
      break;
    case OP_OFFSET8_INDEX_SP:
      snprintf(temp, sizeof(temp), "($%02x,SP)", memory->read8(address + count));
      strcat(instr, temp);
      count++;
      break;
    case OP_INDIRECT8:
      snprintf(temp, sizeof(temp), "[$%02x.w]", memory->read8(address + count));
      strcat(instr, temp);
      count++;
      break;
    case OP_INDIRECT16:
      snprintf(temp, sizeof(temp), "[$%x.w]", READ_RAM16(address + count));
      strcat(instr, temp);
      count += 2;
      break;
    case OP_INDIRECT16_E:
      snprintf(temp, sizeof(temp), "[$%x.e]", READ_RAM16(address + count));
      strcat(instr, temp);
      count += 2;
      break;
    case OP_INDIRECT8_X:
      snprintf(temp, sizeof(temp), "([$%02x.w],X)", memory->read8(address + count));
      strcat(instr, temp);
      count++;
      break;
    case OP_INDIRECT16_X:
      snprintf(temp, sizeof(temp), "([$%x.w],X)", READ_RAM16(address + count));
      strcat(instr, temp);
      count += 2;
      break;
    case OP_INDIRECT16_E_X:
      snprintf(temp, sizeof(temp), "([$%x.e],X)", READ_RAM16(address + count));
      strcat(instr, temp);
      count += 2;
      break;
    case OP_INDIRECT8_Y:
      snprintf(temp, sizeof(temp), "([$%02x.w],Y)", memory->read8(address + count));
      strcat(instr, temp);
      count++;
      break;
    case OP_INDIRECT16_E_Y:
      snprintf(temp, sizeof(temp), "([$%x.e],Y)", READ_RAM16(address + count));
      strcat(instr, temp);
      count += 2;
      break;
    case OP_ADDRESS_BIT:
      snprintf(temp, sizeof(temp), "$%x, #%d", READ_RAM16(address + count), (opcode & 0x0e) >> 1);
      strcat(instr, temp);
      count += 2;
      break;
    case OP_ADDRESS_BIT_LOOP:
      offset = (int8_t)memory->read8(address + count + 2);
      snprintf(temp, sizeof(temp), "$%x, #%d, $%x  (offset=%d)", READ_RAM16(address + count), (opcode & 0x0e) >> 1, (address + count + 3) + offset, offset);
      strcat(instr, temp);
      count += 3;
      break;
    case OP_RELATIVE:
      offset = (int8_t)memory->read8(address + count);
      snprintf(temp, sizeof(temp), "$%x  (offset=%d)", (address + count + 1) + offset, offset);
      strcat(instr, temp);
      count++;
      break;
    case OP_SINGLE_REGISTER:
    case OP_TWO_REGISTERS:
      break;
    case OP_ADDRESS16_NUMBER8:
      snprintf(temp, sizeof(temp), "$%x, #$%02x", READ_RAM16(address + count + 1), memory->read8(address + count));
      strcat(instr, temp);
      count += 3;
      break;
    case OP_ADDRESS8_ADDRESS8:
      snprintf(temp, sizeof(temp), "$%x, $%02x", memory->read8(address + count + 1), memory->read8(address + count));
      strcat(instr, temp);
      count += 2;
      break;
    case OP_ADDRESS16_ADDRESS16:
      snprintf(temp, sizeof(temp), "$%x, $%x", READ_RAM16(address + count + 2), READ_RAM16(address + count));
      strcat(instr, temp);
      count += 4;
      break;
    default:
      strcpy(instr, "???");
      break;
  }

  if (table_stm8_opcodes[n].src != 0)
  {
    if (table_stm8_opcodes[n].type != OP_NONE &&
        table_stm8_opcodes[n].type != OP_TWO_REGISTERS)
    {
      strcat(instr, ",");
    }

    add_reg(instr, table_stm8_opcodes[n].src);
  }

  return count;
}

void list_output_stm8(
  AsmContext *asm_context,
  uint32_t start,
  uint32_t end)
{
  int cycles_min,cycles_max,count;
  char instruction[128];
  int n;

  Memory *memory = &asm_context->memory;

  fprintf(asm_context->list, "\n");

  while (start < end)
  {
    count = disasm_stm8(
      memory,
      start,
      instruction,
      sizeof(instruction),
      &cycles_min,
      &cycles_max);

    fprintf(asm_context->list, "0x%04x:", start);

    for (n = 0; n < 5; n++)
    {
      if (n < count)
      {
        fprintf(asm_context->list, " %02x", memory->read8(start + n));
      }
        else
      {
        fprintf(asm_context->list, "   ");
      }
    }

    fprintf(asm_context->list, "  %-40s cycles: ", instruction);

    if (cycles_min == cycles_max)
    { fprintf(asm_context->list, "%d\n", cycles_min); }
      else
    { fprintf(asm_context->list, "%d-%d\n", cycles_min, cycles_max); }

    start += count;
  }
}

void disasm_range_stm8(
  Memory *memory,
  uint32_t flags,
  uint32_t start,
  uint32_t end)
{
  char instruction[128];
  int cycles_min = 0, cycles_max = 0;
  int count;
  int n;

  printf("\n");

  printf("%-7s  %-5s         %-40s Cycles\n", "Addr", "Opcode", "Instruction");
  printf("-------  ------------   ----------------------------------       ------\n");

  while (start <= end)
  {
    count = disasm_stm8(
      memory,
      start,
      instruction,
      sizeof(instruction),
      &cycles_min,
      &cycles_max);

    printf("0x%04x: ", start);

    for (n = 0; n < 5; n++)
    {
      if (n < count)
      {
        printf(" %02x", memory->read8(start + n));
      }
        else
      {
        printf("   ");
      }
    }

    printf(" %-40s ", instruction);

    if (cycles_min < 1)
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

    start = start + count;
  }
}

