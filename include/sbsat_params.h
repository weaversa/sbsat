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

#ifndef SBSAT_PARAMS_H
#define SBSAT_PARAMS_H

//#include <stdio.h>
//#include <assert.h>
//#include <string.h>
//#include <ctype.h>
//#include <stdlib.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <unistd.h>

//parameter types 

#define P_PRE_CHAR 1
#define P_PRE_INT 2
#define P_PRE_LONG 3
#define P_PRE_FLOAT 4
#define P_PRE_STRING 5
#define P_NONE 10
#define P_CHAR 11
#define P_INT 12
#define P_LONG 13
#define P_FLOAT 14
#define P_STRING 15
#define P_FN 20
#define P_FN_CHAR 21
#define P_FN_INT 22
#define P_FN_LONG 23
#define P_FN_FLOAT 24
#define P_FN_STRING 25

typedef void(*p_fn)(void);
typedef void(*p_fn_char)(char);
typedef void(*p_fn_int)(int);
typedef void(*p_fn_long)(long);
typedef void(*p_fn_float)(float);
typedef void(*p_fn_string)(char *);

/*
 * variable types
 */

#define VAR_CMDLINE 1
#define VAR_INI 2
#define VAR_DUMP 4
#define VAR_CHECK 8

/*
 * predefined standard
 */

#define VAR_NORMAL (VAR_CMDLINE+VAR_INI+VAR_DUMP)
#define VAR_CMD VAR_CMDLINE

typedef
union {
  char s[256]; /* has to be the first */
  char c;
  int i;
  long l;
  float f;
} 
t_value;

// cmdline and ini file options
// can add values of internal variable for dump (read/only)
//
// l_opt -- (dash)letter option (should be just one letter)
// w_opt -- (dash)(dash)word option and ini option (no spaces please)
// p_target -- pointer to the variable
// p_type -- P_NONE, P_INT, P_FLOAT, P_STRING
// p_value -- if p_num == 0 the variable is assigned this value if found
// p_defa -- default value
// var_type -- should not get exported to the ini file
// p_src -- parameter default=0, parameter from the ini file=1, cmd line=2
//          change the parameter only if this number is lower than the new one
// desc_opt -- description of this option for help and ini file
//

typedef struct {
  void *p_target;

  char l_opt[32];
  char w_opt[256];
  short p_type;
  t_value p_value;
  t_value p_defa;
  short var_type; 
  short p_src;
  char desc_opt[256];
} t_opt;

/* designated initializers are not supported on some platforms */
/* workaround is to initialize the value as string and then convert it to
 * the appropriate type
 * This work for all parameters that are specifiable on the cmd line
 * It is restrictive in predefined parameters since they can not be initialized
 * to already present constant (making it imposible to print internal constant
 * this way)
 */
//#define HAVE_DES_INITIALIZERS
#ifdef HAVE_DES_INITIALIZERS
#define V(x,y) {x}
#else
#define V(x,y) {y}
#endif

extern t_opt options[];

void show_ini();
void show_help();
void show_version();
void show_competition_version();

void init_options();
void read_cmd(int argc, char *argv[]);
void fix_ini_filename();
void read_ini(char*);
void dump_params();
t_opt *lookup_keyword(char *key);
t_opt *lookup_short_keyword(char *key);
void set_param_int(char *param, int value);
void change_defa_param_int(char *param, int value);


#endif
