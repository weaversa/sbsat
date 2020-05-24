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

#ifndef SBSAT_VARS_H
#define SBSAT_VARS_H

//Error conditions may be between 0 and 255
#define NO_ERROR 0
#define ERR_IO_INIT 101
#define ERR_IO_READ 102
#define ERR_IO_UNEXPECTED_CHAR 103
#define IO_CNF_HEADER 104
#define MEM_ERR 105
#define MEM_SWAP_ERR 106
#define TIMEOUT_ERR 107
#define BDD_FREE_ERR 108
#define BDD_GENERIC_ERR 109
#define BDD_IS_NULL_ERR 110
#define BDD_NOT_INIT_ERR 111
#define ERR_SIG_INIT 112
#define UNSAT 113
#define SAT 114
#define SAT_UNKNOWN 115
#define FOUND_INFERENCE 116
#define CLUSTER_LIMIT 117
#define ERR_SIG_USER_QUIT 118

void params_consistency_check ();
int params_parse_cmd_line(int argc, char *argv[]);
void params_dump();

extern char inputfile[256];
extern char outputfile[256];
extern FILE *finputfile;
extern FILE *foutputfile;

extern int PARAMS_DUMP;

extern char temp_dir[256];

extern char comment[1024];

extern char temp_dir[256];
extern char ini_filename[256];
extern char input_result_filename[256];

extern int random_seed;

extern int break_after_input;
extern int do_not_solve;
extern int count_BDD_solutions_flag;

#endif
