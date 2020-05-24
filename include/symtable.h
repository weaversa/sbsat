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

#ifndef SYMTABLE_H
#define SYMTABLE_H
/* Data type for links in the chain of symbols.      */
struct symrec
{
   char *name;  /* name of symbol          */
   uintmax_t name_int;
   struct symrec *next;    /* link field              */
   uintmax_t id;
   uint8_t flag;
   uint8_t sym_type;
};

typedef struct symrec symrec;

#define SYM_ANY   0
#define SYM_VAR   1
#define SYM_FN    2
#define SYM_OTHER 3

// flags
#define SYM_FLAG_UNRAVEL 0x10

/* The symbol table: a chain of `struct symrec'.     */
//extern symrec **sym_table;

void      sym_init();
void      sym_free();
symrec   *putsym(char *, uint8_t sym_type);
symrec   *getsym(char *);
uintmax_t i_getsym(char *, uint8_t sym_type);
uintmax_t i_getsym_int(intmax_t var_int, uint8_t sym_type);
uintmax_t i_putsym_int(intmax_t var_int, uint8_t sym_type);
uintmax_t i_putsym(char *, uint8_t sym_type);
symrec *putsym_with_id(char *sym_name, uint8_t sym_type, uintmax_t id);
symrec   *s_getsym(char *, uint8_t sym_type);
char     *s_name(uintmax_t);
symrec   *tputsym(uint8_t sym_type);
symrec   *getsym_i(uintmax_t id);
void      print_symtable();
uintmax_t get_or_putsym_check(char *sym_name, uint8_t sym_type, uintmax_t id);

uint8_t _sym_is_flag(uintmax_t id);
extern symrec **sym_table;
#define sym_is_flag(id) (sym_table[id]->flag & SYM_FLAG_UNRAVEL)
void sym_set_flag(uintmax_t id);
void sym_reset_flag(uintmax_t id); 
void sym_clear_all_flag();
uint8_t sym_all_int();


/* reg expressions */
typedef struct {
   uintmax_t last_id;
   regex_t rg;
} t_myregex;

uint8_t   sym_regex_init(t_myregex *rg, char *exp);
uintmax_t sym_regex(t_myregex *rg);
uint8_t   sym_regex_free(t_myregex *rg);
void create_all_syms(uintmax_t start, uintmax_t sym_max_id);

#endif
