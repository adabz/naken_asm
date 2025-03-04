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

#ifndef NAKEN_ASM_SIMULATE_RISCV_H
#define NAKEN_ASM_SIMULATE_RISCV_H

#include <unistd.h>

#include "simulate/Simulate.h"

class SimulateRiscv : public Simulate
{
public:
  SimulateRiscv(Memory *memory);
  virtual ~SimulateRiscv();

  static Simulate *init(Memory *memory);

  virtual void reset();
  virtual void push(uint32_t value);
  virtual int set_reg(const char *reg_string, uint32_t value);
  virtual uint32_t get_reg(const char *reg_string);
  virtual void set_pc(uint32_t value);
  virtual void dump_registers();
  virtual int run(int max_cycles, int step);

private:
  int16_t reg[32];
};

#endif

