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

#ifndef SBSAT_FORMATS_H
#define SBSAT_FORMATS_H

#define I_CNF 1;

extern uint8_t input_format;
extern void *input_object;
extern uint8_t output_format;

uint8_t read_input(BDDManager *BM);
uint8_t write_output(BDDManager *BM);

//CNF format specific types, variables, and functions
typedef struct store {
  uintmax_t length;
  uintmax_t *num;
} store;

typedef struct Clause {
  uintmax_t length;
  intmax_t *variables;
  uint8_t subsumed;
  uint8_t flag;
} Clause;

typedef struct CNF_Struct {
  uintmax_t nNumVariables;
  uintmax_t nNumClauses;
  Clause *pClauses;
  uintmax_t nNumVariables_nosym;
} CNF_Struct;

extern int find_cnf_gates_flag; //SEAN!!! why are these ints?
extern int cluster_cnf_flag;
extern int clause_cluster_overlap;
extern int clause_cluster_max_length;
extern int write_gates_flag;
extern int write_cnf_flag;
uint8_t free_clauses(CNF_Struct *CNF);
uint8_t read_cnf(CNF_Struct *CNF);
uint8_t process_cnf(CNF_Struct *CNF, BDDManager *BM);
uint8_t write_clause(CNF_Struct *CNF, uintmax_t clause_num);
uint8_t write_cnf(CNF_Struct *CNF);
uint8_t compress_variables(CNF_Struct *CNF);

//ITE format specifc types, variables, and functions
uint8_t iteloop (BDDManager *BM);

uint8_t process_aiger(BDDManager *BM);
  
#endif
