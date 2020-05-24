/*
   sbsat - SBSAT is a state-based Satisfiability solver.

   Copyright (C) 2006 Sean Weaver

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef SBSAT_UTILS_H
#define SBSAT_UTILS_H

#include "sbsat.h"

extern uintmax_t sbsat_max_ram;
extern float sbsat_max_runtime;

#define sbsat_free(n) _sbsat_free(n)
#define sbsat_calloc(x,y,dbglvl,forwhat) _sbsat_calloc(x,y,dbglvl,forwhat)
#define sbsat_malloc(x,y,dbglvl,forwhat) _sbsat_malloc(x,y,dbglvl,forwhat)
#define sbsat_recalloc(ptr,oldx,x,y,dbglvl,forwhat) _sbsat_recalloc(ptr,oldx,x,y,dbglvl,forwhat)
#define sbsat_realloc(ptr,oldx,x,y,dbglvl,forwhat) _sbsat_realloc(ptr,oldx,x,y,dbglvl,forwhat)

void _sbsat_free(void **ptr);
void *_sbsat_calloc(uintmax_t x, uintmax_t y, int dbg_lvl, const char *for_what);
void *_sbsat_malloc(uintmax_t x, uintmax_t y, int dbg_lvl, const char *for_what);
void *_sbsat_recalloc(void *ptr, uintmax_t oldx, uintmax_t x, uintmax_t y, int dbg_lvl, const char *for_what);
void *_sbsat_realloc(void *ptr, uintmax_t oldx, uintmax_t x, uintmax_t y, int dbg_lvl, const char *for_what);
void memcpy_sbsat(void*, void*, int);

char *sbsat_basename(char *filename);
void sbsat_strncpy(char *str_dst, char *str_src, int len);

void get_freefile(char *basename, char *file_dir, char *filename, int filename_max);

int init_terminal_out();
int init_terminal_in();
void free_terminal_out();
void free_terminal_in();

uint8_t set_signal_timer();
double get_runtime();
uintmax_t get_memusage();
uint8_t check_limits();

//qsort compare functions
int revdbl_indexcompfunc(const void *x, const void *y);
int dbl_indexcompfunc(const void *x, const void *y);
int doublecompfunc(const void *x, const void *y);
int compfunc(const void *x, const void *y);
int revcompfunc(const void *x, const void *y);
int abscompfunc(const void *, const void *);
int absrevcompfunc(const void *x, const void *y);
int randlistcompfunc(const void *x, const void *y);

typedef struct rand_list {
  uintmax_t num;
  uintmax_t size;
  double prob;
} rand_list;

int check_gzip (char *filename);
FILE *zread(char *filename, int zip);

typedef struct VectorManager {
  uint64_t block_size;
  uint64_t num_blocks;
  uint64_t next_block;
  uint64_t *pool;
} VectorManager;

VectorManager *initVectorManager(uint64_t block_size, uint64_t initial_num_blocks);
uint64_t *get_next_vector(VectorManager *x);
void freeVectorManager(VectorManager *x);

#endif
