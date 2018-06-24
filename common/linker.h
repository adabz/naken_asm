/**
 *  naken_asm assembler.
 *  Author: Michael Kohn
 *   Email: mike@mikekohn.net
 *     Web: http://www.mikekohn.net/
 * License: GPLv3
 *
 * Copyright 2010-2018 by Michael Kohn
 *
 */

#ifndef _LINKER_H
#define _LINKER_H

enum
{
  IMPORT_TYPE_AR,
  IMPORT_TYPE_OBJ,
};

struct _imports
{
  struct _imports *next;
  int type;
  int size;
  uint8_t code[];
};

struct _linker
{
  struct _imports *imports;
  uint8_t *symbol_list_buffer;
  uint32_t symbol_list_buffer_size;
  uint32_t symbol_list_buffer_end;
};

struct _symbol_list
{
  struct _imports *imports;
  char name[];
};

int linker_add_file(struct _linker *linker, const char *filename);

int linker_search_code_from_symbol(
  struct _linker *linker,
  const char *symbol);

uint8_t *linker_find_code_from_symbol(
  struct _linker *linker,
  const char *symbol,
  uint32_t *function_size);

const char *linker_find_name_from_offset(
  struct _linker *linker,
  uint32_t offset);

void linker_print_symbol_list(struct _linker *linker);
void linker_free(struct _linker *linker);

#endif

