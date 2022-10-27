/*
   Copyright (C) 2009 Sean Weaver

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

#include "sbsat.h"
#include <sys/time.h>
#include <sys/resource.h>

extern char temp_dir[256];

uintmax_t sbsat_max_ram;
float sbsat_max_runtime;

void *_sbsat_calloc(uintmax_t x, uintmax_t y, int dbg_lvl, const char *for_what) {
   //fprintf(stderr, "sbsat_calloc(x=%d, y=%d, dbg_lvl=%d, for_what=%s)\n", x, y, dbg_lvl, for_what);
   void *p = NULL;
   uintmax_t r = x;
   r *= y;
   if (r >= INT_MAX) {
         fprintf(stderr, "ERROR: Unable to allocate %ju (%ju * %ju) bytes for %s\n", x*y, x, y, for_what); 
         exit(1); 
   }
   if (x==0 || y==0) {
      dm2_printf2("WARNING: 0 bytes allocation for %s\n", for_what); 
   } else {
      p=calloc(x,y);
      if (!p) {
         fprintf(stderr, "ERROR: Unable to allocate %ju (%ju * %ju) bytes for %s\n", x*y, x, y, for_what); 
         exit(1); 
      } 
   }
   DM_2(
         if ((DEBUG_LVL&15) >= dbg_lvl) 
         fprintf(stddbg, "Allocated %ld bytes for %s\n", (long)(x*y), for_what); 
      );
   return p;
}

void *_sbsat_malloc(uintmax_t x, uintmax_t y, int dbg_lvl, const char *for_what) {
   //fprintf(stderr, "sbsat_malloc(x=%d, y=%d, dbg_lvl=%d, for_what=%s)\n", x, y, dbg_lvl, for_what);
   void *p = NULL;
   uintmax_t r = x;
   r *= y;
   if (r >= INT_MAX) {
         fprintf(stderr, "ERROR: Unable to allocate %ju (%ju * %ju) bytes for %s\n", x*y, x, y, for_what); 
         exit(1); 
   }
   if (x==0 || y==0) {
      dm2_printf2("WARNING: 0 bytes allocation for %s\n", for_what); 
   } else {
      p=malloc(x*y);
      if (!p) {
         fprintf(stderr, "ERROR: Unable to allocate %ju (%ju * %ju) bytes for %s\n", x*y, x, y, for_what); 
         exit(1); 
      } 
   }
   DM_2(
         if ((DEBUG_LVL&15) >= dbg_lvl) 
         fprintf(stddbg, "Allocated %ld bytes for %s\n", (long)(x*y), for_what); 
      );
   return p;
}

void _sbsat_free(void **ptr) {
  if (*ptr != NULL) {
    free(*ptr);
    *ptr = NULL;
  }
}

void *_sbsat_recalloc(void *ptr, uintmax_t oldx, uintmax_t x, uintmax_t y, int dbg_lvl, const char *for_what) {
   //fprintf(stderr, "sbsat_recalloc(ptr, oldx=%d, x=%d, y=%d, dbg_lvl=%d, for_what=%s)\n", oldx, x, y, dbg_lvl, for_what);
   void *p = NULL;
   uintmax_t r = x;
   r *= y;
   if (r >= INT_MAX) {
         fprintf(stderr, "ERROR: Unable to allocate %ju (%ju * %ju) bytes for %s\n", x*y, x, y, for_what); 
         exit(1); 
   }
//   assert(oldx<x);
   if (x==0 || y==0) {
      dm2_printf2("WARNING: 0 bytes allocation for %s\n", for_what); 
   } else {
      p=realloc(ptr, x*y);
      if (!p) {
         fprintf(stderr, "ERROR: Unable to allocate %ju bytes for %s\n", x*y, for_what); 
         exit(1); 
      } 
      void *pp = (char*)p+oldx*y;
      if(oldx < x) memset(pp, 0, (x-oldx)*y);
   }
   DM_2(
         if ((DEBUG_LVL&15) >= dbg_lvl) 
         fprintf(stddbg, "ReAllocated %ju bytes for %s\n", x*y, for_what); 
      );
   return p;
}

void *_sbsat_realloc(void *ptr, uintmax_t oldx, uintmax_t x, uintmax_t y, int dbg_lvl, const char *for_what) {
   //fprintf(stderr, "sbsat_realloc(ptr, oldx=%d, x=%d, y=%d, dbg_lvl=%d, for_what=%s)\n", oldx, x, y, dbg_lvl, for_what);
   void *p = NULL;
   uintmax_t r = x;
   r *= y;
   if (r >= INT_MAX) {
         fprintf(stderr, "ERROR: Unable to allocate %ju (%ju * %ju) bytes for %s\n", x*y, x, y, for_what); 
	 assert(0);
         exit(1); 
   }
   //assert(oldx<x);
   if (x==0 || y==0) {
      dm2_printf2("WARNING: 0 bytes allocation for %s\n", for_what); 
   } else {
      p=realloc(ptr, x*y);
      if (!p) {
         fprintf(stderr, "ERROR: Unable to allocate %ju bytes for %s\n", x*y, for_what); 
	 assert(0);
         exit(1); 
      } 
      //void *pp = (char*)p+oldx*y;
      //memset(pp, 0, (x-oldx)*y);
   }
   DM_2(
         if ((DEBUG_LVL&15) >= dbg_lvl) 
         fprintf(stddbg, "ReAllocated %ju bytes for %s\n", x*y, for_what); 
      );
   return p;
}

//Returns run time in seconds.
double
get_runtime ()
{
  struct rusage ru;
  getrusage (RUSAGE_SELF, &ru);
  return ((double) ru.ru_utime.tv_sec * 1000 + ru.ru_utime.tv_usec / 1000 +
           ru.ru_stime.tv_sec * 1000 + ru.ru_stime.tv_usec / 1000) / 1000;
}

//#define LINUX_GET_MEMUSAGE
#ifdef LINUX_GET_MEMUSAGE
uintmax_t
get_memusage ()
{
   int pid = getpid();
   char stat_buf[256];
   char buffer[2001];
   sprintf(stat_buf, "/proc/%d/status", pid);
   FILE *fin=fopen (stat_buf, "r");
   if (!fin) return 0;

   /* read the file in one swing */
   fread(buffer, 2000, 1, fin);
   fclose(fin);

   buffer[2000] = 0;

   char *c=buffer;

   /* find the end of the filename */
   //while (*c!=0 && *c!=')') c++;
   //if (*c == 0) return 0;

   /* find the start of the memory info */
   int space_counter=0;
   while (space_counter < 9) 
    {
      if (*c == ' ') space_counter++;
      c++;
    }

   *c='*';

   uintmax_t memalloc=0;
   if (1!=sscanf(c+1, "%ju", &memalloc)) 
	  memalloc=0;
   //fprintf(stderr, "stat(%s):\n%s\nreturn: %ld", stat_buf, buffer, memalloc);
   return memalloc;
}
#else
uintmax_t
get_memusage ()
{
  struct rusage ru; //SEAN!!! This does not work right...overflow problems at 2gigs.
  getrusage (RUSAGE_SELF, &ru);
  return 0x00000000FFFFFFFF & (ru.ru_maxrss/*+ru.ru_ixrss+ru.ru_idrss+ru.ru_isrss*/);
}
#endif

uint8_t check_limits() {
  uintmax_t memusage = get_memusage()/1000000;
  if(memusage > sbsat_stats[STAT_MAX_RAM_USAGE])
    sbsat_stats[STAT_MAX_RAM_USAGE] = memusage;
  double runtime = get_runtime();

  d6_printf4("Runtime: %4.3fs, using %juM and %ju SmurfStates built\n", runtime, memusage, sbsat_stats[STAT_STATES_BUILT]);
  if(SM_main && SM_main->ESSM) {
    if(SM_main->ESSM_type == 'f') {
      d6_printf3("Propagations (%ju) per second: %4.3Lf\n", sbsat_stats[STAT_NUM_PROPAGATIONS], ((long double)sbsat_stats[STAT_NUM_PROPAGATIONS]) / (runtime - sbsat_stats_f[STAT_F_SOLVER_START_TIME]));
      d6_printf3("Backtracks (%ju) per second: %4.3Lf\n", sbsat_stats[STAT_NUM_BACKTRACKS], ((long double)sbsat_stats[STAT_NUM_BACKTRACKS]) / (runtime - sbsat_stats_f[STAT_F_SOLVER_START_TIME]));
      d6_printf3("Learned Clauses (%ju) per second: %4.3Lf\n",
		 (uintmax_t)((funcsat *)(SM_main->ESSM))->numLearnedClauses,
		 ((long double)((funcsat *)(SM_main->ESSM))->numLearnedClauses) / (runtime - sbsat_stats_f[STAT_F_SOLVER_START_TIME]));
    } else if(SM_main->ESSM_type == 'p') {
      d6_printf3("Propagations (%ju) per second: %4.3Lf\n",
		 (uintmax_t)*((PicosatManager *)(SM_main->ESSM))->propagations,
		 ((long double)*((PicosatManager *)(SM_main->ESSM))->propagations) / (runtime - sbsat_stats_f[STAT_F_SOLVER_START_TIME]));
      d6_printf3("Backtracks (%ju) per second: %4.3Lf\n",
		 (uintmax_t)*((PicosatManager *)(SM_main->ESSM))->conflicts,
		 ((long double)*((PicosatManager *)(SM_main->ESSM))->conflicts) / (runtime - sbsat_stats_f[STAT_F_SOLVER_START_TIME]));
    } else {
      d6_printf3("Propagations (%ju) per second: %4.3Lf\n", sbsat_stats[STAT_NUM_PROPAGATIONS], ((long double)sbsat_stats[STAT_NUM_PROPAGATIONS]) / (runtime - sbsat_stats_f[STAT_F_SOLVER_START_TIME]));
      d6_printf3("Backtracks (%ju) per second: %4.3Lf\n", sbsat_stats[STAT_NUM_BACKTRACKS], ((long double)sbsat_stats[STAT_NUM_BACKTRACKS]) / (runtime - sbsat_stats_f[STAT_F_SOLVER_START_TIME]));
    }
  }
  if((sbsat_max_ram!=0) && (memusage >= sbsat_max_ram)) {
    if(memusage >= sbsat_max_ram) { //If we are still in swap
      d2_printf3("SBSAT is using %juM which is greater than max of %juM\n", memusage, sbsat_max_ram);
      sbsat_stats[STAT_ERROR_NUM] = MEM_ERR;
      return MEM_ERR;
    }
  }

  if((sbsat_max_runtime!=0.0) && (runtime >= sbsat_max_runtime)) {
    d2_printf3("SBSAT has taken %4.3fs which is greater than max of %4.3fs\n", runtime, sbsat_max_runtime);
    sbsat_stats[STAT_ERROR_NUM] = TIMEOUT_ERR;
    return TIMEOUT_ERR;
  }

  return NO_ERROR;
}

int compfunc (const void *x, const void *y) {
   uintmax_t pp, qq;
   
   pp = *(const uintmax_t *) x;
   qq = *(const uintmax_t *) y;
   if (pp < qq)
     return -1;
   if (pp == qq)
#ifndef FORCE_STABLE_QSORT
     return 0;
#else
     {
        if (x < y) return -1;
        else if (x > y) return 1;
        else return 0;
     }
#endif
   return 1;
}

int revdbl_indexcompfunc(const void *x, const void *y) {
   dbl_index pp, qq;
   
   pp = *(const dbl_index *) x;
   qq = *(const dbl_index *) y;
   if (pp.value > qq.value)
     return -1;
   if (pp.value == qq.value)
#ifndef FORCE_STABLE_QSORT
     return 0;
#else
     {
        if (x < y) return -1;
        else if (x > y) return 1;
        else return 0;
     }
#endif
   return 1;
}

int dbl_indexcompfunc(const void *x, const void *y) {
   dbl_index pp, qq;
   
   pp = *(const dbl_index *) x;
   qq = *(const dbl_index *) y;
   if (pp.value < qq.value)
     return -1;
   if (pp.value == qq.value)
#ifndef FORCE_STABLE_QSORT
     return 0;
#else
     {
        if (x < y) return -1;
        else if (x > y) return 1;
        else return 0;
     }
#endif
   return 1;
}

int doublecompfunc(const void *x, const void *y) {
   double pp, qq;
   
   pp = *(const double *) x;
   qq = *(const double *) y;
   if (pp < qq)
     return -1;
   if (pp == qq)
#ifndef FORCE_STABLE_QSORT
     return 0;
#else
     {
        if (x < y) return -1;
        else if (x > y) return 1;
        else return 0;
     }
#endif
   return 1;
}

int revcompfunc(const void *x, const void *y) {
   uintmax_t pp, qq;
   
   pp = *(const uintmax_t *) x;
   qq = *(const uintmax_t *) y;
   if (pp > qq)
     return -1;
   if (pp == qq)
#ifndef FORCE_STABLE_QSORT
     return 0;
#else
     {
        if (x > y) return -1;
        else if (x < y) return 1;
        else return 0;
     }
#endif
   return 1;
}

int abscompfunc(const void *x, const void *y) {
   intmax_t pp, qq;
   
   pp = imaxabs(*(const intmax_t *) x);
   qq = imaxabs(*(const intmax_t *) y);
   if (pp < qq)
     return -1;
   if (pp == qq)
#ifndef FORCE_STABLE_QSORT
     return 0;
#else
     {
        if (x < y) return -1;
        else if (x > y) return 1;
        else return 0;
     }
#endif
   return 1;
}

int absrevcompfunc(const void *x, const void *y) {
   intmax_t pp, qq;
   
   pp = imaxabs(*(const intmax_t *) x);
   qq = imaxabs(*(const intmax_t *) y);
   if (pp > qq)
     return -1;
   if (pp == qq)
#ifndef FORCE_STABLE_QSORT
     return 0;
#else
     {
        if (x < y) return -1;
        else if (x > y) return 1;
        else return 0;
     }
#endif
  return 1;
}

int randlistcompfunc(const void *x, const void *y) {
  rand_list pp, qq;
  
  pp = *(const rand_list *) x;
  qq = *(const rand_list *) y;
  if (pp.size < qq.size)
    return -1;
  if (pp.size == qq.size) {
    if(pp.prob < qq.prob)
      return -1;
    if(pp.prob > qq.prob)
      return 1;
    return 0;
  }
  return 1;
}



void memcpy_sbsat( void* d, void* s, int size ) {
	memcpy(d,s,size);
	return;
}

void
get_freefile(char *basename, char *file_dir, char *filename, int filename_max) {
	struct stat buf;
	int filename_len=0;
	
	if (file_dir != NULL && *file_dir != 0) 
	  strncpy(filename, file_dir, filename_max);	
	else
	  strcpy(filename, ".");
	
	strncat(filename, "/", filename_max-strlen(filename)-5);
	strncat(filename, basename, filename_max-strlen(filename)-5);	
	filename_len = strlen(filename);
	
	/* if it does not exist => it is good and it returns */
	if (stat(filename, &buf)!=0) return;
	
	/* otherwise we just append a number */
	  {
		  int i = 1;
		  while (i<1000000) {
			  sprintf(filename+filename_len, "%d", i);
			  if (stat(filename, &buf)==0) i++;
			  else return;
		  }
	  }
	printf("Please delete old files from %s\n", filename);
	exit (1);
}

char *sbsat_basename(char *filename) {
   char *ptr = strrchr(filename, '/');
   if (ptr == NULL) ptr = filename;
   else ptr = ptr+1;
   return ptr;
}

void sbsat_strncpy(char *str_dst, char *str_src, int len) {
   strncpy(str_dst, str_src, len);
   str_dst[len] = 0;
}

int signs_idx = 0;
//#define signs_max 6
//char signs[signs_max] = "/|\\- ";
#define signs_max 4
char signs[signs_max+1] = "/|\\-";
int last_print_roller = 0;

void print_roller() {
   if (last_print_roller) {
      fprintf(stddbg, "\b%c", signs[signs_idx++]);
   } else {
      fprintf(stddbg, "%c", signs[signs_idx++]);
      last_print_roller = 1; 
   }
   if (signs[signs_idx] == 0) signs_idx = 0;
}

void print_nonroller() {
   last_print_roller = 0; 
}

void print_roller_init() {
   last_print_roller = 0; 
   signs_idx = 0;
}

VectorManager *initVectorManager(uint64_t block_size, uint64_t initial_num_blocks) {
  VectorManager *x = (VectorManager *)sbsat_calloc(1, sizeof(VectorManager), 9, "VectorManager");
  x->block_size = block_size;
  x->num_blocks = initial_num_blocks;
  x->next_block = 0;
  x->pool = (uint64_t *)sbsat_calloc(initial_num_blocks * block_size, sizeof(uint64_t), 9, "x->pool");
  return x;
}

uint64_t *get_next_vector(VectorManager *x) {
  if(x->next_block == x->num_blocks) {
    x->pool = (uint64_t *)sbsat_recalloc(x->pool, x->num_blocks * x->block_size, (x->num_blocks*2) * x->block_size, sizeof(uint64_t), 9, "x->pool");
    x->num_blocks*=2;
  }
  return x->pool + (x->block_size * x->next_block++);
}

void freeVectorManager(VectorManager *x) {
  sbsat_free((void **)&x->pool);
  sbsat_free((void **)&x);
}
