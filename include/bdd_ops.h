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

#ifndef BDD_OPS_H
#define BDD_OPS_H

#include "sbsat.h"

int bddperm_compfunc (const void *x, const void *y);

void BDD_Support2(uintmax_t *length, uintmax_t *max, uintmax_t **tempint, DdNode *f, DdNode *g);
void BDD_Support(uintmax_t *length, uintmax_t *max, uintmax_t **tempint, DdNode *func);
uintmax_t highest_var_in_BDD(DdNode *f);
DdNode *Mitosis(DdManager *dd, DdNode *f, uint32_t *map);
DdNode *clone_BDD(DdManager *dd, DdManager *dd_old, DdNode *f);

DdNode *build_and_BDD(DdManager *dd, intmax_t *variables, uintmax_t length);
DdNode *build_or_BDD(DdManager *dd, intmax_t *variables, uintmax_t length);
DdNode *build_xor_BDD(DdManager *dd, intmax_t *variables, uintmax_t length);
DdNode *build_and_equals_BDD(DdManager *dd, intmax_t equal_var, intmax_t *variables, uintmax_t length);
DdNode *build_or_equals_BDD(DdManager *dd, intmax_t equal_var, intmax_t *variables, uintmax_t length);
DdNode *build_ite_equals_BDD(DdManager *dd, intmax_t equal_var, intmax_t v1, intmax_t v2, intmax_t v3);
DdNode *build_majv_equals_BDD(DdManager *dd, intmax_t equal_var, intmax_t v1, intmax_t v2, intmax_t v3);
DdNode *build_cardinality_BDD(DdManager *dd, uintmax_t min, uintmax_t max, uintmax_t *variables, uintmax_t length);

/****************************************************************************************************/

uintmax_t add_and_BDD(BDDManager *BM, intmax_t *variables, uintmax_t length);
uintmax_t add_or_BDD(BDDManager *BM, intmax_t *variables, uintmax_t length);
uintmax_t add_xor_BDD(BDDManager *BM, intmax_t *variables, uintmax_t length);
uintmax_t add_and_equals_BDD(BDDManager *BM, intmax_t equal_var, intmax_t *variables, uintmax_t length);
uintmax_t add_or_equals_BDD(BDDManager *BM, intmax_t equal_var, intmax_t *variables, uintmax_t length);
uintmax_t add_ite_equals_BDD(BDDManager *BM, intmax_t equal_var, intmax_t v1, intmax_t v2, intmax_t v3);
uintmax_t add_majv_equals_BDD(BDDManager *BM, intmax_t equal_var, intmax_t v1, intmax_t v2, intmax_t v3);
uintmax_t add_cardinality_BDD(BDDManager *BM, uintmax_t min, uintmax_t max, uintmax_t *variables, uintmax_t length);

DdNode *precache_conjunctive_BDD_overlap(BDDManager *BM, uintmax_t bdd0, uintmax_t bdd1, uintmax_t var_limit);
uintmax_t compute_BDD_overlap(BDDManager *BM, uintmax_t bdd0, uintmax_t bdd1, uintmax_t var_limit);
uintmax_t compute_BDD_union(BDDManager *BM, uintmax_t bdd0, uintmax_t bdd1, uintmax_t var_limit);

DdNode *Cudd_bddRestrictLimit(DdManager *manager, DdNode *f, DdNode *c, unsigned int limit);
DdNode *Cudd_bddNPAndLimit(DdManager *manager, DdNode *f, DdNode *c, unsigned int limit);
DdNode *Cudd_bddLICompactionLimit(DdManager *manager, DdNode *f, DdNode *c, unsigned int limit);
DdNode *quadratic_bdd(BDDManager *BM, uintmax_t_list vars);
DdNode *find_all_linear_factors(DdManager *dd, DdNode *f, intmax_t_list *support, uintmax_t outvar);
DdNode *create_first_factor(DdManager *dd, DdNode *lin_factors, uintmax_t outvar);
DdNode *better_gcf(BDDManager *BM, DdNode *f, DdNode *c);
DdNode *XORXORs(DdManager * dd, DdNode * f, DdNode * g);
DdNode *safe_assign0(DdManager *dd, DdNode *f, DdNode *v);

DdNode *set_variable(DdManager *dd, DdNode *pCurrentBDD, uintmax_t var, uint8_t polarity);

mpz_t *count_true_paths(BDDManager *BM, DdNode *f, uintmax_t_list *support);
mpz_t *count_BDD_solutions(BDDManager *BM);

#endif
