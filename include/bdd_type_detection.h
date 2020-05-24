/*
  *     sbsat - SBSAT is a state-based Satisfiability solver.
  *  
  *     Copyright (C) 2010 Sean Weaver
  *  
  *     This program is free software; you can redistribute it and/or modify
  *     it under the terms of the GNU General Public License as published by
  *     the Free Software Foundation; either version 2, or (at your option)
  *     any later version.
  *  
  *     This program is distributed in the hope that it will be useful,
  *     but WITHOUT ANY WARRANTY; without even the implied warranty of
  *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *     GNU General Public License for more details.
  *  
  *     You should have received a copy of the GNU General Public License
  *     along with this program; if not, write to the Free Software Foundation,
  *     Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
  * */

#ifndef BDD_TYPE_DETECTION_H
#define BDD_TYPE_DETECTION_H

#include "sbsat.h"

enum {
  FN_TYPE_UNSURE=0,     /* 0 */
  FN_TYPE_AND,          /* 1 */
  FN_TYPE_NAND,         /* 2 */
  FN_TYPE_OR,           /* 3 */
  FN_TYPE_NOR,          /* 4 */
  FN_TYPE_XOR,          /* 5 */
  FN_TYPE_EQU,          /* 6  = XNOR */
  FN_TYPE_RIMP,         /* 7 */
  FN_TYPE_RNIMP,        /* 8 */
  FN_TYPE_LIMP,         /* 9 */
  FN_TYPE_LNIMP,        /* 10 */
  FN_TYPE_ITE,          /* 11 */
  FN_TYPE_NITE,         /* 12 */
  FN_TYPE_NEW_INT_LEAF, /* 13 */
  FN_TYPE_IMPAND,       /* 14 *///x-> a & b & c
  FN_TYPE_IMPOR,        /* 15 *///x-> a v b v c
  FN_TYPE_PLAINOR,      /* 16 *///a v b v c
  FN_TYPE_PLAINAND,     /* 17 *///a & b & c
  FN_TYPE_PLAINXOR,     /* 18 *///a + b + c
  FN_TYPE_MINMAX,       /* 19 */
  FN_TYPE_NEGMINMAX,    /* 20 */
  FN_TYPE_XDD,          /* 21 */
  FN_TYPE_DEP_VAR,      /* 22 */
     
  FN_TYPE_EQU_BASE=30,      /* equ_base + 0 */
  FN_TYPE_AND_EQU,          /* equ_base + 1 */
  FN_TYPE_NAND_EQU,         /* equ_base + 2 */
  FN_TYPE_OR_EQU,           /* equ_base + 3 */
  FN_TYPE_NOR_EQU,          /* equ_base + 4 */
  FN_TYPE_XOR_EQU,          /* equ_base + 5 */
  FN_TYPE_EQU_EQU,          /* equ_base + 6  = XNOR */
  FN_TYPE_RIMP_EQU,         /* equ_base + 7 */
  FN_TYPE_RNIMP_EQU,        /* equ_base + 8 */
  FN_TYPE_LIMP_EQU,         /* equ_base + 9 */
  FN_TYPE_LNIMP_EQU,        /* equ_base + 10 */
  FN_TYPE_ITE_EQU,          /* equ_base + 11 */
  FN_TYPE_NITE_EQU,         /* equ_base + 12 */
  FN_TYPE_AND_EQUAL_EQU,    /* equ_base + 13 */
  FN_TYPE_NEW_INT_LEAF_EQU, /* equ_base + 14 */
  FN_TYPE_IMPAND_EQU,       /* equ_base + 15 *///x-> a & b & c
  FN_TYPE_IMPOR_EQU,        /* equ_base + 16 *///x-> a v b v c
  FN_TYPE_PLAINOR_EQU,      /* equ_base + 17 *///a v b v c
  FN_TYPE_PLAINXOR_EQU,     /* equ_base + 18 *///a + b + c
  FN_TYPE_MINMAX_EQU,       /* equ_base + 19 */
  FN_TYPE_NEG_MINMAX_EQU,   /* equ_base + 20 */
  FN_TYPE_MAX
};

extern uintmax_t functionTypeLimits[FN_TYPE_MAX];
extern int functionTypeLimits_all;
extern uint8_t ignore_limits;

uint8_t isSingleton(DdNode *bdd);
uintmax_t isOR(DdManager *dd, DdNode *bdd);
uintmax_t isAND(DdManager *dd, DdNode *bdd);
uintmax_t isXOR(DdManager *dd, DdNode *bdd);
uint8_t isMINMAX(DdManager* dd, DdNode *bdd, uintmax_t_list *bdd_vars);
uint8_t isNEGMINMAX(DdManager* dd, DdNode *bdd, uintmax_t_list *bdd_vars);
DdNode *isAND_EQU(DdManager *dd, DdNode *bdd, uintmax_t_list *bdd_vars);
DdNode *isXDependent(DdManager *dd, DdNode *bdd, DdNode *X);
DdNode *find_dep_var(DdManager *dd, DdNode *bdd, uintmax_t_list *bdd_vars);
DdNode *find_all_dep_vars(DdManager *dd, DdNode *bdd, uintmax_t_list *bdd_vars, uint8_t ret_vars);
DdNode *find_all_disjunctive_vars(DdManager *dd, DdNode *bdd, uintmax_t_list *bdd_vars, uint8_t ret_vars);
DdNode *find_equals_factor(DdManager *dd, DdNode *f);

uintmax_t getMAX(DdManager *dd, DdNode *bdd);
uintmax_t getMIN(DdManager *dd, DdNode *bdd, uintmax_t bdd_len);

uintmax_t findandret_fnType(BDDManager *BM, DdNode *bdd);

#endif
