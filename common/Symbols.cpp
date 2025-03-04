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

#include "common/MemoryPool.h"
#include "common/Symbols.h"

int symbols_init(Symbols *symbols)
{
  symbols->memory_pool = NULL;
  symbols->locked = 0;
  symbols->in_scope = 0;
  symbols->debug = 0;
  symbols->current_scope = 0;

  return 0;
}

void symbols_free(Symbols *symbols)
{
  memory_pool_free(symbols->memory_pool);
  symbols->memory_pool = NULL;
}

SymbolsData *symbols_find(Symbols *symbols, const char *name)
{
  MemoryPool *memory_pool = symbols->memory_pool;
  int ptr;

  // Check local scope.
  if (symbols->in_scope != 0)
  {
    while (memory_pool != NULL)
    {
      ptr = 0;

      while (ptr < memory_pool->ptr)
      {
        SymbolsData *symbols_data =
          (SymbolsData *)(memory_pool->buffer + ptr);

        if (symbols->current_scope == symbols_data->scope &&
            strcmp(symbols_data->name, name) == 0)
        {
          return symbols_data;
        }

        ptr += symbols_data->len + sizeof(SymbolsData);
      }

      memory_pool = memory_pool->next;
    }

    memory_pool = symbols->memory_pool;
  }

  // Check global scope.
  while (memory_pool != NULL)
  {
    ptr = 0;

    while (ptr < memory_pool->ptr)
    {
      SymbolsData *symbols_data = (SymbolsData *)(memory_pool->buffer + ptr);

      if (symbols_data->scope == 0 && strcmp(symbols_data->name, name) == 0)
      {
        return symbols_data;
      }

      ptr += symbols_data->len + sizeof(SymbolsData);
    }

    memory_pool = memory_pool->next;
  }

  return NULL;
}

int symbols_append(Symbols *symbols, const char *name, uint32_t address)
{
  int token_len;
  MemoryPool *memory_pool = symbols->memory_pool;
  SymbolsData *symbols_data;

#ifdef DEBUG
//printf("symbols_append(%s, %d);\n", name, address);
#endif

  if (symbols->locked == 1) { return 0; }

  symbols_data = symbols_find(symbols, name);

  if (symbols_data != NULL)
  {
    // For unit test.  Probably a better way to do this.
    if (symbols->debug == 1)
    {
      symbols_data->address = address;
      return 0;
    }

    if (symbols->in_scope == 0 || symbols_data->scope == symbols->current_scope)
    {
      printf("Error: Label '%s' already defined.\n", name);
      return -1;
    }
  }

  token_len = strlen(name) + 1;

  // Check if size of new label is bigger than 255.
  if (token_len > 255)
  {
    printf("Error: Label '%s' is too big.\n", name);
    return -1;
  }

  // If there is no pool, add one.
  if (memory_pool == NULL)
  {
    memory_pool = memory_pool_add((NakenHeap *)symbols, SYMBOLS_HEAP_SIZE);
  }

  // Find a pool that has enough area at the end to add this address.
  // If none can be found, alloc a new one.
  while (1)
  {
     if (memory_pool->ptr + token_len + (int)sizeof(SymbolsData) < memory_pool->len)
     {
       break;
     }

     if (memory_pool->next == NULL)
     {
       memory_pool->next = memory_pool_add((NakenHeap *)symbols, SYMBOLS_HEAP_SIZE);
     }

     memory_pool = memory_pool->next;
  }

  // Divide by bytes_per_address (for AVR8 and dsPIC).
  //address = address / asm_context->bytes_per_address;

  // Set the new label/address entry.
  symbols_data =
    (SymbolsData *)(memory_pool->buffer + memory_pool->ptr);

  memcpy(symbols_data->name, name, token_len);
  symbols_data->len = token_len;
  symbols_data->flag_rw = 0;
  symbols_data->flag_export = 0;
  symbols_data->address = address;
  symbols_data->scope = symbols->in_scope == 0 ? 0 : symbols->current_scope;

  memory_pool->ptr += token_len + sizeof(SymbolsData);

  return 0;
}

int symbols_set(Symbols *symbols, const char *name, uint32_t address)
{
  SymbolsData *symbols_data = NULL;

  symbols_data = symbols_find(symbols, name);

  if (symbols_data == NULL)
  {
    if (symbols_append(symbols, name, address) != 0)
    {
      return -1;
    }

    symbols_data = symbols_find(symbols, name);
    symbols_data->scope = 0;
    symbols_data->flag_rw = 1;
  }
    else
  if (symbols_data->flag_rw == 1)
  {
    symbols_data->address = address;
  }
    else
  {
    return -1;
  }

  return 0;
}

int symbols_export(Symbols *symbols, const char *name)
{
  SymbolsData *symbols_data = symbols_find(symbols, name);

  if (symbols_data == NULL) { return -1; }

  if (symbols_data->scope != 0)
  {
    printf("Error: Cannot export local variable '%s'\n", name);
    return -1;
  }

  symbols_data->flag_export = 1;

  return 0;
}

void symbols_lock(Symbols *symbols)
{
  symbols->locked = 1;
}

int symbols_lookup(Symbols *symbols, const char *name, uint32_t *address)
{
  SymbolsData *symbols_data = symbols_find(symbols, name);

  if (symbols_data == NULL)
  {
    *address = 0;
    return -1;
  }

  *address = symbols_data->address;

  return 0;
}

int symbols_iterate(Symbols *symbols, SymbolsIter *iter)
{
  MemoryPool *memory_pool = symbols->memory_pool;

  if (iter->end_flag == 1) { return -1; }
  if (iter->memory_pool == NULL)
  {
    iter->memory_pool = symbols->memory_pool;
    iter->ptr = 0;
  }

  while (memory_pool != NULL)
  {
    if (iter->ptr < memory_pool->ptr)
    {
      SymbolsData * symbols_data =
        (SymbolsData *)(memory_pool->buffer + iter->ptr);

      iter->address = symbols_data->address;
      iter->name = symbols_data->name;
      iter->ptr = iter->ptr + symbols_data->len + sizeof(SymbolsData);
      iter->flag_export = symbols_data->flag_export;
      iter->scope = symbols_data->scope;
      iter->count++;

      return 0;
    }

    memory_pool = memory_pool->next;
  }

  iter->end_flag = 1;

  return -1;
}

int symbols_print(Symbols *symbols, FILE *out)
{
  SymbolsIter iter;

  memset(&iter, 0, sizeof(iter));

  fprintf(out, "%30s ADDRESS  SCOPE\n", "LABEL");

  while (symbols_iterate(symbols, &iter) != -1)
  {
    fprintf(out, "%30s %08x %d%s\n", iter.name, iter.address, iter.scope, iter.flag_export == 1 ? " EXPORTED" : "");
  }

  fprintf(out, " -> Total symbols: %d\n\n", iter.count);

  return 0;
}

int symbols_count(Symbols *symbols)
{
  MemoryPool *memory_pool = symbols->memory_pool;
  int ptr;
  int count = 0;

  while (memory_pool != NULL)
  {
    ptr = 0;
    while (ptr < memory_pool->ptr)
    {
      SymbolsData *symbols_data =
        (SymbolsData *)(memory_pool->buffer + ptr);
      ptr += symbols_data->len + sizeof(SymbolsData);
      count++;
    }

    memory_pool = memory_pool->next;
  }

  return count;
}

int symbols_export_count(Symbols *symbols)
{
  MemoryPool *memory_pool = symbols->memory_pool;
  int ptr;
  int count = 0;

  while (memory_pool != NULL)
  {
    ptr = 0;
    while (ptr < memory_pool->ptr)
    {
      SymbolsData *symbols_data = (SymbolsData *)(memory_pool->buffer + ptr);
      ptr += symbols_data->len + sizeof(SymbolsData);
      if (symbols_data->flag_export == 1) { count++; }
    }

    memory_pool = memory_pool->next;
  }

  return count;
}

int symbols_scope_start(Symbols *symbols)
{
  if (symbols->in_scope == 1)
  {
    return -1;
  }

  symbols->in_scope = 1;
  symbols->current_scope++;

  return 0;
}

int symbols_scope_reset(Symbols *symbols)
{
  symbols->current_scope = 0;

  return 0;
}

int symbols_scope_end(Symbols *symbols)
{
  symbols->in_scope = 0;

  return 0;
}

