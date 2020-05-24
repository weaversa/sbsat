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

/* The symbol table: a chain of `struct symrec'.  */
symrec **sym_table = NULL;
symrec *symtmp_table = NULL;
symrec *sym_hash_table=NULL;

#define SYM_HASH_SIZE 5000
#define SYMTMP_TABLE_SIZE 5000
#define SYM_TABLE_SIZE 5000

uintmax_t sym_table_idx = 0;
uintmax_t sym_table_max = 0;
uintmax_t symtmp_table_idx = 0;
uintmax_t symtmp_table_max = 0;
uint8_t sym_all_int_flag = 1;

symrec *tputsym_truefalse(uint8_t sym_type);
void fill_symrec_with_id(symrec *ptr, uint8_t sym_type, uintmax_t id);
void free_symrec_name_pool_chain();
void free_symrec_rec_pool_chain();
void free_symtmp_table();
void symrec_rec_pool_alloc();
void create_all_syms(uintmax_t start, uintmax_t sym_max_id);

void sym_init() {
  sym_hash_table = (symrec*)sbsat_calloc(SYM_HASH_SIZE, sizeof(symrec), 9, "sym_hash_table");
  sym_table_max = SYM_TABLE_SIZE;
  sym_table = (symrec**)sbsat_calloc(SYM_TABLE_SIZE, sizeof(symrec*), 9, "sym_table");
  sym_table_idx = 0;
  symrec_rec_pool_alloc();
  create_all_syms(0, 0);
}

void sym_free() {
   sbsat_free((void**)&sym_hash_table);
   sbsat_free((void**)&sym_table);
   free_symrec_name_pool_chain();
   free_symrec_rec_pool_chain();
   free_symtmp_table();
}

symrec *sym_hash(char *name) {
  uint32_t i=0;
  uint32_t hash=0;
  while (name[i] && i<10) {
    hash += name[i];
    i++;
  }
  hash = hash % SYM_HASH_SIZE;
  return sym_hash_table+hash;
}

uintmax_t i_putsym(char *sym_name, uint8_t sym_type) {
  uintmax_t i = i_getsym(sym_name, sym_type);
  if (i!=0) return i;
  putsym(sym_name, sym_type);
  return i_getsym(sym_name, sym_type);
}

uintmax_t get_or_putsym_check(char *sym_name, uint8_t sym_type, uintmax_t id) {
   uintmax_t i = i_putsym(sym_name, sym_type);
   assert(i==id);
   return i;
}

void fill_symrec_with_id(symrec *ptr, uint8_t sym_type, uintmax_t id) {
  if (id >= sym_table_max) {
    sym_table = (symrec**)sbsat_recalloc((void*)sym_table, sym_table_max,
					 sym_table_max+SYM_TABLE_SIZE, sizeof(symrec*), 9, "sym_table_idx");
    sym_table_max += SYM_TABLE_SIZE;
  }
  if (id > sym_table_idx) sym_table_idx = id;
  
  ptr->id = id;
  ptr->flag = 0;
  ptr->sym_type = sym_type;
  sym_table[id] = ptr;
}

void fill_symrec(symrec *ptr, int sym_type) {
  sym_table_idx++;
  fill_symrec_with_id(ptr, sym_type, sym_table_idx);
}

/* ------------------- name rec pool ------------------- */

char *symrec_name_pool = NULL;
uintmax_t symrec_name_pool_max = 0;
uintmax_t symrec_name_pool_index = 0;
typedef struct _symrec_name_pool_chain_type {
   char *pool;
   struct _symrec_name_pool_chain_type * next;
} symrec_name_pool_chain_type;
symrec_name_pool_chain_type * symrec_name_pool_chain_head=NULL;
symrec_name_pool_chain_type * symrec_name_pool_chain=NULL;

void free_symrec_name_pool_chain() {
   symrec_name_pool_chain_type * next = symrec_name_pool_chain_head;
   while(next) {
      symrec_name_pool_chain_type *current = next;
      next = next->next;
      free(current->pool);
      free(current);
   }
   symrec_name_pool_chain_head = NULL;
   symrec_name_pool_chain = NULL;
   symrec_name_pool_max = 0;
   symrec_name_pool_index = 0;
}

void symrec_name_pool_alloc(uintmax_t min) {
   symrec_name_pool_chain_type *tmp_symrec_name_pool_chain = 
     (symrec_name_pool_chain_type*)sbsat_calloc(1, sizeof(symrec_name_pool_chain_type), 9, "symrec_name_pool_chain");
   if (symrec_name_pool_chain == NULL) {
      symrec_name_pool_chain_head = symrec_name_pool_chain = tmp_symrec_name_pool_chain;
   } else {
      symrec_name_pool_chain->next = tmp_symrec_name_pool_chain;
      symrec_name_pool_chain = tmp_symrec_name_pool_chain;
   }
   symrec_name_pool_max = 100000;
   if (symrec_name_pool_max < min) symrec_name_pool_max = min;
   symrec_name_pool_chain->pool = (char*)sbsat_calloc(symrec_name_pool_max, sizeof(char), 9, "symrec_name_pool");
   symrec_name_pool = symrec_name_pool_chain->pool;
   symrec_name_pool_index = 0;
}

char *symrec_name_alloc(char *name) {
   uintmax_t len = strlen(name)+1;
   if (symrec_name_pool_index + len >= symrec_name_pool_max) {
      symrec_name_pool_alloc(len);
   }
   strcpy(symrec_name_pool + symrec_name_pool_index, name);
   symrec_name_pool_index += len;
   return (symrec_name_pool + symrec_name_pool_index - len);
}

/* ------------------- sym rec pool ------------------ */

symrec *symrec_rec_pool = NULL;
int symrec_rec_pool_max = 0;
int symrec_rec_pool_index = 0;
typedef struct _symrec_rec_pool_chain_type {
   symrec *pool;
   struct _symrec_rec_pool_chain_type * next;
} symrec_rec_pool_chain_type;
symrec_rec_pool_chain_type * symrec_rec_pool_chain_head=NULL;
symrec_rec_pool_chain_type * symrec_rec_pool_chain=NULL;

void free_symrec_rec_pool_chain() {
   symrec_rec_pool_chain_type * next = symrec_rec_pool_chain_head;
   while(next) {
      symrec_rec_pool_chain_type *current = next;
      next = next->next;
      free(current->pool);
      free(current);
   }
   symrec_rec_pool_chain_head = NULL;
   symrec_rec_pool_chain = NULL;
   symrec_rec_pool_max = 0;
   symrec_rec_pool_index = 0;
}

void symrec_rec_pool_alloc() {
   symrec_rec_pool_chain_type *tmp_symrec_rec_pool_chain = 
     (symrec_rec_pool_chain_type*)sbsat_calloc(1, sizeof(symrec_rec_pool_chain_type), 9, "symrec_rec_pool_chain");
   if (symrec_rec_pool_chain == NULL) {
      symrec_rec_pool_chain_head = symrec_rec_pool_chain = tmp_symrec_rec_pool_chain;
   } else {
      symrec_rec_pool_chain->next = tmp_symrec_rec_pool_chain;
      symrec_rec_pool_chain = tmp_symrec_rec_pool_chain;
   }
   symrec_rec_pool_max = 100000;
   symrec_rec_pool_chain->pool = (symrec*)sbsat_calloc(symrec_rec_pool_max, sizeof(symrec), 9, "symrec_rec_pool");
   symrec_rec_pool = symrec_rec_pool_chain->pool;
   symrec_rec_pool_index = 0;
}

symrec *symrec_rec_alloc() {
   if (symrec_rec_pool_index + 1 >= symrec_rec_pool_max) {
      symrec_rec_pool_alloc();
   }
   symrec_rec_pool_index += 1;
   return (symrec_rec_pool + symrec_rec_pool_index - 1);
}

/* ----------------------------------------------------------- */

symrec *putsym_with_id(char *sym_name, uint8_t sym_type, uintmax_t id) {
   symrec *ptr;
   symrec *tmp_sym_table = sym_hash(sym_name);
   ptr = symrec_rec_alloc();
   ptr->name = symrec_name_alloc(sym_name);
   ptr->next = (struct symrec *)tmp_sym_table->next;
   tmp_sym_table->next = ptr;
   fill_symrec_with_id(ptr, sym_type, id);
   return ptr;
}

symrec *putsym(char *sym_name, uint8_t sym_type) {
   symrec *ptr;
   //{int id; if (sscanf(sym_name, "%d", &id)==1 && id!=0) return putsym_with_id(sym_name, sym_type, id); }
   symrec *tmp_sym_table = sym_hash(sym_name);
   ptr = symrec_rec_alloc();
   ptr->name = symrec_name_alloc(sym_name);
   if (sym_all_int_flag && sscanf(sym_name, "%ju", &(ptr->name_int)) == 0) sym_all_int_flag = 0;
   ptr->next = (struct symrec *)tmp_sym_table->next;
   tmp_sym_table->next = ptr;
   fill_symrec(ptr, sym_type);
   return ptr;
}

void create_all_syms(uintmax_t start, uintmax_t sym_max_id) {
   for(uintmax_t i=start;i<=sym_max_id;i++) {
      if (i>sym_table_idx ||
          sym_table[i] == NULL) {
         char num[20]; //0xFFFFFFFFFFFFFFFF needs 20 characters in base 10.
         sprintf(num, "%ju", i);
         putsym_with_id(num, SYM_VAR, i);
      }
   }
}

symrec *getsym(char *sym_name) {
   symrec *ptr;
   //int sym_name_int=0; if (sscanf(sym_name, "%d", &sym_name_int) == 0) sym_name_int = 0;
   for (ptr = sym_hash(sym_name)->next; ptr != (symrec *) 0;
        ptr = (symrec *)ptr->next) {
      /*
       if (sym_name_int > 0 && ptr->name_int > 0)
       if (sym_name_int == ptr->name_int) 
       return ptr;
       else
       continue;
       */
      if (strcasecmp(ptr->name, sym_name) == 0) {
         return ptr;
      }
   }
   return 0;
}

symrec *getsym_i(uintmax_t id) {
  if (sym_table == NULL) return NULL;
  if (id > sym_table_idx) return NULL;
  /*  -- enable if vars are missing
   if (sym_table[id] == NULL) {
     char sym_name[32];
     sprintf(sym_name, "%d", id);
     if (getsym(sym_name) != NULL) { // bummer
         exit();
     }
     putsym_id(sym_name, SYM_VAR, id);
     assert(sym_table[id] != NULL);
  }
  */
  return sym_table[id];
}

char *s_name(uintmax_t id) {
  if (sym_table == NULL) return NULL;
  if (id > sym_table_idx) return NULL;
  symrec *ptr = sym_table[id];
  if (ptr == NULL) return NULL;
  return ptr->name;
}

uintmax_t i_getsym (char *sym_name, uint8_t sym_type) {
   symrec *ptr = getsym(sym_name);
   if (ptr) {
      if (ptr->sym_type != sym_type && sym_type != 0)
        return -ptr->id;
      else
        return ptr->id;
   }
   ptr = putsym(sym_name, sym_type);
   return ptr->id;
}

uintmax_t i_getsym_int(intmax_t var_int, uint8_t sym_type) {
   char sym_name[64];
   intmax_t sign=var_int<0?-1:1;
   var_int *= sign; // abs value
   assert(var_int > 0);
   sprintf(sym_name, "%ju", var_int);
   return sign*i_getsym(sym_name, sym_type);
}

uintmax_t i_putsym_int(intmax_t var_int, uint8_t sym_type) {
   char sym_name[64];
   intmax_t sign=var_int<0?-1:1;
   var_int *= sign; // abs value
   assert(var_int > 0);
   sprintf(sym_name, "%ju", var_int);
   return sign*i_putsym(sym_name, sym_type);
}

symrec *s_getsym(char *sym_name, uint8_t sym_type) {
   symrec *ptr = getsym(sym_name);
   if (ptr) {
      if (ptr->sym_type != sym_type && sym_type != 0) 
        return NULL;
      else
        return ptr;
   } 
   ptr = putsym(sym_name, sym_type);
   assert(ptr->id != 0);
   return ptr;
}

typedef struct _symtmp_table_pools_type {
   symrec *pool;
   struct _symtmp_table_pools_type * next;
} symtmp_table_pools_type;

symtmp_table_pools_type * symtmp_table_pools_head=NULL;
symtmp_table_pools_type * symtmp_table_pools=NULL;

void symtmp_table_alloc() {
   symtmp_table_pools_type * tmp_symtmp_table_pools = (symtmp_table_pools_type*)sbsat_calloc(1, sizeof(symtmp_table_pools_type), 9, "symtmp_table_pools");
   if (symtmp_table_pools_head == NULL) {
      symtmp_table_pools_head = 
        symtmp_table_pools = tmp_symtmp_table_pools;
   } else {
      symtmp_table_pools->next = tmp_symtmp_table_pools;
      symtmp_table_pools = symtmp_table_pools->next;
   }
   /* cannot reallocate  -- pointers exist to this pool */
   symtmp_table_max = SYMTMP_TABLE_SIZE;
   symtmp_table = (symrec*)sbsat_calloc(symtmp_table_max, sizeof(symrec), 9, "symtmp_table");
   symtmp_table_idx = 0;
   symtmp_table_pools->pool = symtmp_table;
}

void free_symtmp_table() {
   symtmp_table_pools_type * next = symtmp_table_pools_head;
   while (next) {
      symtmp_table_pools_type * current = next;
      next = next->next;
      sbsat_free((void**)&current->pool);
      sbsat_free((void**)&current);
   }
   symtmp_table_pools_head = NULL;
   symtmp_table_pools = NULL;
   symtmp_table_max = 0;
   symtmp_table_idx = 0;
}

symrec *tputsym_truefalse(uint8_t sym_type) {
   if (symtmp_table_idx >= symtmp_table_max) {
      symtmp_table_alloc();
   }
   symrec *ptr = &(symtmp_table[symtmp_table_idx]);
   fill_symrec(ptr, sym_type);
   symtmp_table_idx++;
   return ptr;
}

symrec *tputsym(uint8_t sym_type) {
   if (symtmp_table_idx >= symtmp_table_max) {
      symtmp_table_alloc();
   }
   symrec *ptr = &(symtmp_table[symtmp_table_idx]);
   fill_symrec(ptr, sym_type);
   symtmp_table_idx++;
   return ptr;
}

void print_symtable() {
   for(uintmax_t i=0;i <= sym_table_idx;i++) {
      symrec *ptr = getsym_i(i);
      if (ptr)
        printf("%ju \"%s\" \n", i, (ptr->name?ptr->name:""));
   } 
}

/* set myregex = NULL */
/* call sym_regex_init(&myregex, "expression") */
/* call sym_regex(&myregex) returns the first id or 0 */
/* call sym_regex_free(&myregex) */

uint8_t sym_regex_init(t_myregex *rg, char *sym_exp) {
   assert(rg != NULL);
   memset(rg, 0, sizeof(t_myregex));
   rg->last_id = 1;
   return regcomp(&(rg->rg), sym_exp, REG_NOSUB+REG_EXTENDED+REG_ICASE);
   /* REG_EXTENDED REG_ICASE REG_NEWLINE */
}

uintmax_t sym_regex(t_myregex *rg) {
   assert(rg != NULL);
   for(;rg->last_id <= sym_table_idx; rg->last_id++) {
      symrec *ptr = getsym_i(rg->last_id);
      if (ptr && ptr->name &&
          regexec(&(rg->rg), ptr->name, 0, NULL, 0) == 0) {
         /* found one */
         rg->last_id++;
         return rg->last_id-1;
      }
   }
   return 0;
}

uint8_t sym_regex_free(t_myregex *rg) {
   assert(rg != NULL);
   regfree(&(rg->rg));
   memset(rg, 0, sizeof(t_myregex));
   return 0;
}

/* here is the test -- to use it call this from e.g. read_input */
void sym_regex_test(char *reg) {
   t_myregex myrg;
   sym_regex_init(&myrg, reg);
   uintmax_t id = sym_regex(&myrg);
   while (id) {
      /* found variable and the variable id is id */
      fprintf(stderr, "%ju %s\n", id, getsym_i(id)->name);
      id = sym_regex(&myrg);
   }
   sym_regex_free(&myrg);
   exit(1);
}

void sym_reset_flag(uintmax_t id) {
   assert(getsym_i(id)->flag & SYM_FLAG_UNRAVEL);
   getsym_i(id)->flag &= ~SYM_FLAG_UNRAVEL;
}

void sym_set_flag(uintmax_t id) {
   assert((getsym_i(id)->flag & SYM_FLAG_UNRAVEL) == 0);
   getsym_i(id)->flag |= SYM_FLAG_UNRAVEL;
}

uint8_t _sym_is_flag(uintmax_t id) {
   return getsym_i(id)->flag & SYM_FLAG_UNRAVEL;
}

void sym_clear_all_flag() {
   uintmax_t i;
   for(i=0;i <= sym_table_idx;i++)
     sym_table[i]->flag = 0;
}

uint8_t sym_all_int() {
   uintmax_t i;
   for(i=0;i <= sym_table_idx;i++) {
      uintmax_t num;
      if (sym_table[i] == NULL) continue;
      if (sym_table[i]->name == NULL) return 0;
      if (sscanf(sym_table[i]->name, "%ju", &num)==0) return 0;
   }
   return 1;
}
