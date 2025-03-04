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
#include <stdint.h>

#include "disasm/arm.h"
#include "table/arm.h"

//#define READ_RAM(a) memory_read_m(memory, a)
#define ARM_NIB(n) ((opcode >> n) & 0xf)

// NOTE "" is AL
static const char *arm_cond[] =
{
  "eq", "ne", "cs", "cc",
  "mi", "pl", "vs", "vc",
  "hi", "ls", "ge", "lt",
  "gt", "le", "", "nv"
};

#if 0
const char *arm_alu_ops[] =
{
  "and", "eor", "sub", "rsb",
  "add", "adc", "sbc", "rsc",
  "tst", "teq", "cmp", "cmn",
  "orr", "mov", "bic", "mvn"
};
#endif

const char *arm_shift[] =
{
  "lsl", "lsr", "asr", "ror"
};

static const char *arm_reg[] =
{
  "r0", "r1", "r2", "r3",
  "r4", "r5", "r6", "r7",
  "r8", "r9", "r10", "r11",
  "r12", "sp", "lr", "pc"
};

static int compute_immediate(int immediate)
{
  int rotate = immediate >> 8;
  immediate &= 0xff;

  return immediate << (rotate << 1);
#if 0
  int shift=(immediate>>8)*2;
  int shift_mask=(1<<(shift+1))-1;
  immediate=immediate&0xff;

  //printf("immediate=%d shift=%d shift_mask=%x\n", immediate, shift, shift_mask);

  if (shift==0) { return immediate; }
  return ((immediate&shift_mask)<<(32-shift))|(immediate>>shift);
#endif
}

static void arm_calc_shift(char *temp, int length, int shift, int reg)
{
  if ((shift & 1) == 1)
  {
    snprintf(temp, length, "%s, %s %s",
      arm_reg[reg],
      arm_shift[(shift >> 1) & 0x3],
      arm_reg[shift >> 4]);
  }
    else
  {
    int shift_amount = shift >> 3;
    if (shift_amount != 0)
    {
      snprintf(temp, length, "%s, %s #0x%x",
        arm_reg[reg],
        arm_shift[(shift >> 1) & 0x3],
        shift >> 3);
    }
      else
    {
      snprintf(temp, length, "%s", arm_reg[reg]);
    }
  }
}

static void arm_register_list(char *instruction, int opcode)
{
  char temp[16];
  //int first = 33;
  int count = 0;
  int n;

  for (n = 0; n < 16; n++)
  {
    if ((opcode & 1) == 1)
    {
      snprintf(temp, sizeof(temp), "r%d", n);
      if (count != 0) { strcat(instruction, ", "); }
      strcat(instruction, temp);
      //first = n;

      count++;
    }

    opcode >>= 1;
  }
}

static void process_alu_3(
  char *instruction,
  int length,
  uint32_t opcode,
  int index)
{
  int i = (opcode >> 25) & 1;
  int s = (opcode >> 20) & 1;
  int operand2 = opcode & 0xfff;
  char opcode2[32];

  if (i == 0)
  {
    arm_calc_shift(opcode2, sizeof(opcode2), operand2 >> 4, operand2 & 0xf);
  }
    else
  {
    snprintf(opcode2, sizeof(opcode2), "#0x%x {#0x%02x, %d}",
      compute_immediate(opcode & 0xfff), opcode & 0xff, (opcode & 0xf00) >> 7);
  }

  if ((opcode & table_arm[index].mask) == 0x01a00000)
  {
    snprintf(instruction, length, "%s%s%s %s, %s",
      table_arm[index].instr,
      arm_cond[ARM_NIB(28)],
      (s == 1) ? "s" : "",
      arm_reg[ARM_NIB(12)],
      opcode2);
  }
    else
  {
    snprintf(instruction, length, "%s%s%s %s, %s, %s",
      table_arm[index].instr,
      arm_cond[ARM_NIB(28)],
      (s == 1) ? "s" : "",
      arm_reg[ARM_NIB(12)],
      arm_reg[ARM_NIB(16)],
      opcode2);
  }
}

static void process_alu_2(
  char *instruction,
  int length,
  uint32_t opcode,
  int index,
  int use_d)
{
  int i = (opcode >> 25) & 1;
  int s = (opcode >> 20) & 1;
  int operand2 = opcode & 0xfff;
  char opcode2[32];
  int reg_offset = (use_d == 1) ? 12 : 16;

  if (i == 0)
  {
    arm_calc_shift(opcode2, sizeof(opcode2), operand2 >> 4, operand2 & 0xf);
  }
    else
  {
    snprintf(opcode2, sizeof(opcode2), "#0x%x {#0x%02x, %d}",
      compute_immediate(opcode & 0xfff), opcode & 0xff, (opcode & 0xf00) >> 7);
  }

  if ((opcode & table_arm[index].mask) == 0x01a00000)
  {
    snprintf(instruction, length, "%s%s%s %s, %s",
      table_arm[index].instr,
      arm_cond[ARM_NIB(28)],
      (s == 1) ? "s" : "",
      arm_reg[ARM_NIB(reg_offset)],
      opcode2);
  }
    else
  {
    snprintf(instruction, length, "%s%s%s %s, %s",
      table_arm[index].instr,
      arm_cond[ARM_NIB(28)],
      (s == 1) ? "s" : "",
      arm_reg[ARM_NIB(reg_offset)],
      opcode2);
  }
}

static void process_mul(char *instruction, int length, uint32_t opcode)
{
  int a = (opcode >> 21) & 1;
  int s = (opcode >> 20) & 1;

  if (a == 0)
  {
    snprintf(instruction, length, "mul%s%s %s, %s, %s",
      arm_cond[ARM_NIB(28)],
      (s == 1) ? "s" : "",
      arm_reg[ARM_NIB(16)],
      arm_reg[ARM_NIB(0)],
      arm_reg[ARM_NIB(8)]);
  }
    else
  {
    snprintf(instruction, length, "mla%s%s %s, %s, %s, %s",
      arm_cond[ARM_NIB(28)],
      (s == 1) ? "s" : "",
      arm_reg[ARM_NIB(16)],
      arm_reg[ARM_NIB(0)],
      arm_reg[ARM_NIB(8)],
      arm_reg[ARM_NIB(12)]);
  }
}

static void process_swap(char *instruction, int length, uint32_t opcode)
{
  int b = (opcode >> 22) & 1;

  snprintf(instruction, length, "swp%s%s %s, %s, [%s]",
    arm_cond[ARM_NIB(28)],
    (b == 1) ? "b" : "",
    arm_reg[ARM_NIB(12)],
    arm_reg[ARM_NIB(0)],
    arm_reg[ARM_NIB(16)]);
}

static void process_mrs(char *instruction, int length, uint32_t opcode)
{
  int ps = (opcode >> 22) & 1;

  snprintf(instruction, length, "mrs%s %s, %s",
    arm_cond[ARM_NIB(28)],
    arm_reg[ARM_NIB(12)],
    (ps == 1) ? "SPSR" : "CPSR");
}

static void process_msr_all(char *instruction, int length, uint32_t opcode)
{
  int ps = (opcode >> 22) & 1;

  snprintf(instruction, length, "msr%s %s, %s",
    arm_cond[ARM_NIB(28)],
    (ps == 1) ? "SPSR" : "CPSR",
    arm_reg[ARM_NIB(0)]);
}

static void process_msr_flag(char *instruction, int length, uint32_t opcode)
{
  int i = (opcode >> 25) & 1;
  int ps = (opcode >> 22) & 1;

  if (i == 0)
  {
    snprintf(instruction, length, "msr%s %s_flg, %s",
      arm_cond[ARM_NIB(28)],
      (ps == 1) ? "SPSR" : "CPSR",
      arm_reg[ARM_NIB(0)]);
  }
    else
  {
    snprintf(instruction, length, "msr%s %s_flg, #%d {#%d, %d}",
      arm_cond[ARM_NIB(28)],
      (ps == 1) ? "SPSR" : "CPSR",
      compute_immediate(opcode & 0xfff),
      opcode & 0xff,
      (opcode&0xf00) >> 7);
  }
}

static void process_ldr_str(
  char *instruction,
  int length,
  uint32_t opcode,
  int index,
  uint32_t address)
{
  int w = (opcode >> 21) & 1;
  int b = (opcode >> 22) & 1;
  int u = (opcode >> 23) & 1;
  int pr = (opcode >> 24) & 1;
  int i = (opcode >> 25) & 1;
  int offset = opcode & 0xfff;
  int rn = ARM_NIB(16);
  char temp[64];

  if (i == 0)
  {
    if (offset == 0)
    {
      snprintf(temp, sizeof(temp), "[%s]", arm_reg[rn]);
    }
      else
    {
      if (pr == 1)
      {
        if (u == 0) { offset = -offset; }

        if (rn != 15)
        {
          snprintf(temp, sizeof(temp), "[%s, #%d]", arm_reg[rn], offset);
        }
          else
        {
          snprintf(temp, sizeof(temp), "[%s, #%d] ; 0x%04x",
            arm_reg[rn], offset, address + 8 + offset);
        }
      }
        else
      {
        snprintf(temp, sizeof(temp), "[%s], #%s%d", arm_reg[rn], (u == 0) ? "-" : "", offset);
      }
    }
  }
    else
  {
    int shift = offset >> 4;
    int is_reg = shift & 0x1;
    int type = (shift >> 1) & 0x3;
    int rm = offset & 0xf;
    int rs = (shift >> 4) & 0xf;

#if 0
printf("shift=%d is_reg=%d type=%d rm=%d rs=%d\n",
  shift, is_reg, type, rm, rs);
#endif

    if (is_reg == 1)
    {
      if (pr == 1)
      {
        snprintf(temp, sizeof(temp), "[%s, %s, %s %s]",
          arm_reg[rn], arm_reg[rm], arm_shift[type], arm_reg[rs]);
      }
        else
      {
        snprintf(temp, sizeof(temp), "[%s], %s, %s %s",
          arm_reg[rn], arm_reg[rm], arm_shift[type], arm_reg[rs]);
      }
    }
      else
    {
      int shift_amount = shift >> 3;

      if (pr == 1)
      {
        if (shift_amount != 0)
        {
          snprintf(temp, sizeof(temp), "[%s, %s, %s #%d]",
            arm_reg[rn], arm_reg[rm], arm_shift[type], shift_amount);
        }
          else
        {
          snprintf(temp, sizeof(temp), "[%s, %s]", arm_reg[rn], arm_reg[rm]);
        }
      }
        else
      {
        if (shift_amount != 0)
        {
          snprintf(temp, sizeof(temp), "[%s], %s, %s #%d",
            arm_reg[rn], arm_reg[rm], arm_shift[type], shift_amount);
        }
          else
        {
          snprintf(temp, sizeof(temp), "[%s], %s", arm_reg[rn], arm_reg[rm]);
        }
      }
    }
  }

  snprintf(instruction, length, "%s%s%s %s, %s%s",
    table_arm[index].instr,
    arm_cond[ARM_NIB(28)],
    (b == 0) ? "" : "b",
    arm_reg[ARM_NIB(12)],
    temp,
    (w == 0) ? "" : "!");
}

static void process_undefined(char *instruction, int length, uint32_t opcode)
{
  // hmm.. why?
  strcpy(instruction, "???");
}

static void process_ldm_stm(
  char *instruction,
  int length,
  uint32_t opcode,
  int index)
{
  const char *pru_str[] = { "db", "ib", "da", "ia" };
  int cond = (opcode >> 28) & 0xf;
  int w = (opcode >> 21) & 1;
  int s = (opcode >> 22) & 1;
  int pru = (opcode >> 23)  &0x3;

  //snprintf(instruction, length, "%s%s%s %s%s, {",
  snprintf(instruction, length, "%s%s%s %s%s, {",
    table_arm[index].instr,
    arm_cond[cond],
    pru_str[pru],
    //(s == 1) ? "s" : "",
    arm_reg[ARM_NIB(16)],
    (w == 1) ? "!" : "");

  arm_register_list(instruction, opcode);

  strcat(instruction, "}");

  if (s == 1) { strcat(instruction, "^"); }
}

static void process_branch(
  char *instruction,
  int length,
  uint32_t opcode,
  uint32_t address)
{
  int l = (opcode >> 24) & 1;

  int32_t offset = (opcode & 0xffffff);
  if ((offset & (1 << 23)) != 0) { offset |= 0xff000000; }
  offset <<= 2;

  // address+8 (to allow for the pipeline)
  snprintf(instruction, length, "%s%s 0x%02x (%d)",
    (l == 0) ? "b" : "bl",
    arm_cond[ARM_NIB(28)],
    (address + 8) + offset, offset);
}

static void process_branch_exchange(
  char *instruction,
  int length,
  uint32_t opcode)
{
  snprintf(instruction, length, "bx%s %s", arm_cond[ARM_NIB(28)], arm_reg[ARM_NIB(0)]);
}

static void process_swi(char *instruction, int length, uint32_t opcode)
{
  snprintf(instruction, length, "swi%s", arm_cond[ARM_NIB(28)]);
}

static void process_co_swi(char *instruction, int length, uint32_t opcode)
{
}

static void process_co_transfer(char *instruction, int length, uint32_t opcode)
{
}

static void process_co_op_mask(char *instruction, int length, uint32_t opcode)
{
  snprintf(instruction, length, "cdp%s %d, %d, cr%d, cr%d, cr%d, %d",
    arm_cond[ARM_NIB(28)], ARM_NIB(8), ARM_NIB(20), ARM_NIB(12),
    ARM_NIB(16), ARM_NIB(0), (opcode>>5)&0x7);
}

static void process_co_transfer_mask(
  char *instruction,
  int length,
  uint32_t opcode)
{
  int ls = (opcode >> 20) & 1;
  int w = (opcode >> 21) & 1;
  int n = (opcode >> 22) & 1;
  int u = (opcode >> 23) & 1;
  int pr = (opcode >> 24) & 1;
  int offset = opcode & 0xff;

  if (offset == 0)
  {
    snprintf(instruction, length, "%s%s%s %d, cr%d, [r%d]",
      (ls == 1) ? "ldc" : "stc",
      arm_cond[ARM_NIB(28)],
      (n == 1) ? "l" : "",
      ARM_NIB(8), ARM_NIB(12), ARM_NIB(16));
  }
    else
  if (pr == 1)
  {
    snprintf(instruction, length, "%s%s%s %d, cr%d, [r%d, #%s%d]%s",
      (ls == 1) ? "ldc" : "stc",
      arm_cond[ARM_NIB(28)],
      (n == 1) ? "l" : "",
      ARM_NIB(8), ARM_NIB(12), ARM_NIB(16),
      (u == 0) ? "-" : "",
      offset,
      (w == 1) ? "!" : "");
  }
    else
  {
    snprintf(instruction, length, "%s%s%s %d, cr%d, [r%d], #%s%d%s",
      (ls == 1) ? "ldc" : "stc",
      arm_cond[ARM_NIB(28)],
      (n == 1) ? "l" : "",
      ARM_NIB(8), ARM_NIB(12), ARM_NIB(16),
      (u == 0) ? "-" : "",
      offset,
      (w == 1) ? "!" : "");
  }
}

int disasm_arm(
  Memory *memory,
  uint32_t address,
  char *instruction,
  int length,
  int *cycles_min,
  int *cycles_max)
{
  uint32_t opcode;

  *cycles_min = -1;
  *cycles_max = -1;
  opcode = memory->read32(address);
  //printf("%08x: opcode=%08x\n", address, opcode);

  int n = 0;
  while (table_arm[n].instr != NULL)
  {
    if ((opcode & table_arm[n].mask) == table_arm[n].opcode)
    {
      //*cycles_min=table_arm[n].cycles;
      //*cycles_max=table_arm[n].cycles;

      switch (table_arm[n].type)
      {
        case OP_ALU_3:
          //*cycles_min = 2;
          //*cycles_max = 2;
          process_alu_3(instruction, length, opcode, n);
          return 4;
        case OP_ALU_2_N:
          process_alu_2(instruction, length, opcode, n, 0);
          return 4;
        case OP_ALU_2_D:
          process_alu_2(instruction, length, opcode, n, 1);
          return 4;
        case OP_MULTIPLY:
          process_mul(instruction, length, opcode);
          return 4;
        case OP_SWAP:
          process_swap(instruction, length, opcode);
          return 4;
        case OP_MRS:
          process_mrs(instruction, length, opcode);
          return 4;
        case OP_MSR_ALL:
          process_msr_all(instruction, length, opcode);
          return 4;
        case OP_MSR_FLAG:
          process_msr_flag(instruction, length, opcode);
          return 4;
        case OP_LDR_STR:
          process_ldr_str(instruction, length, opcode, n, address);
          return 4;
        case OP_UNDEFINED:
          process_undefined(instruction, length, opcode);
          return 4;
        case OP_LDM_STM:
          process_ldm_stm(instruction, length, opcode, n);
          return 4;
        case OP_BRANCH:
          process_branch(instruction, length, opcode, address);
          //*cycles_max = 3;
          return 4;
        case OP_BRANCH_EXCHANGE:
          process_branch_exchange(instruction, length, opcode);
          return 4;
        case OP_SWI:
          process_swi(instruction, length, opcode);
          return 4;
        case OP_CO_SWI:
          process_co_swi(instruction, length, opcode);
          return 4;
        case OP_CO_TRANSFER:
          process_co_transfer(instruction, length, opcode);
          return 4;
        case OP_CO_OP_MASK:
          process_co_op_mask(instruction, length, opcode);
          return 4;
        case OP_CO_TRANSFER_MASK:
          process_co_transfer_mask(instruction, length, opcode);
          return 4;
        default:
          strcpy(instruction, "???");
          break;
      }
    }

    n++;
  }

  return 4;
}

void list_output_arm(
  AsmContext *asm_context,
  uint32_t start,
  uint32_t end)
{
  int cycles_min,cycles_max;
  char instruction[128];
  int count;

  Memory *memory = &asm_context->memory;

  while (start < end)
  {
    uint32_t opcode = memory->read32(start);

    fprintf(asm_context->list, "\n");
    count = disasm_arm(
      memory,
      start,
      instruction,
      sizeof(instruction),
      &cycles_min,
      &cycles_max);

    fprintf(asm_context->list, "0x%04x: 0x%08x %-40s cycles: ", start, opcode, instruction);

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

    start += count;
  }
}

void disasm_range_arm(
  Memory *memory,
  uint32_t flags,
  uint32_t start,
  uint32_t end)
{
  char instruction[128];
  int cycles_min = 0,cycles_max = 0;
  uint32_t opcode;

  printf("\n");

  printf("%-7s %-9s %-40s Cycles\n", "Addr", "Opcode", "Instruction");
  printf("------- --------  ----------------------------------       ------\n");

  while (start <= end)
  {
    disasm_arm(
      memory,
      start,
      instruction,
      sizeof(instruction),
      &cycles_min,
      &cycles_max);

    opcode = memory->read32(start);

    if (cycles_min < 1)
    {
      printf("0x%04x: %08x  %-40s ?\n", start, opcode, instruction);
    }
      else
    if (cycles_min == cycles_max)
    {
      printf("0x%04x: %08x  %-40s %d\n", start, opcode, instruction, cycles_min);
    }
      else
    {
      printf("0x%04x: %08x  %-40s %d-%d\n", start, opcode, instruction, cycles_min, cycles_max);
    }

    start = start + 4;
  }
}

