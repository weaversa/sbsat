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

#ifndef BDD_MANAGEMENT_H
#define BDD_MANAGEMENT_H

typedef struct BDDManager{
  DdManager *dd;   
  uintmax_t nNumBDDs;
  uintmax_t nNumBDDs_max;
  DdNode **BDDList;
  uintmax_t_list *Support;
  uintmax_t nNumVariables;
  uintmax_t nNumVariables_max;
  uintmax_t nHighestVarInABDD;
  uintmax_t_list *VarBDDMap;
  uintmax_t_list uTempList;
  uint32_t_list uTempList32;
  uintmax_t_list PossibleExQuantVars;
  uintmax_t_list ExQuantProtectedVars;
  uint8_t ExQuantProtectVars;
  void_queue *InferenceQueue;
  DdNode *equ_var;
  uint8_t read_input_finished;
  Equiv_Manager_Struct *EQManager;
  inference_pool *infer_pool;
  uint8_t in_perform_bdd_reductions;
} BDDManager;

extern int Cudd_var_ordering_algType;
extern BDDManager *BM_main;

extern int sbsat_turn_off_bdds;

extern char preset_variables_string[4096];

//Init, free, and verification procedures
uint8_t bdd_init(BDDManager *BM, uintmax_t nNumVars);
uint8_t bdd_manager_clone_inferences(BDDManager *BM, BDDManager *BM_old);
uint8_t bdd_manager_clone(BDDManager *BM, BDDManager *BM_old);
uint8_t bdd_read_input_finished(BDDManager *BM);
uint8_t bdd_free(BDDManager *BM);
void check_bdd(DdManager *dd, DdNode *new_bdd);
int8_t var_bddcmp(DdManager *dd, intmax_t x, intmax_t y);
uintmax_t count_num_BDD_variables(BDDManager *BM);

//Garbage collect BDD nodes
uintmax_t BDDManager_GC(BDDManager *BM);

//BDDList Management
uint8_t add_BDD_to_manager(BDDManager *BM, DdNode *bdd);
uint8_t replace_BDD_in_manager(BDDManager *BM, DdNode *new_bdd, uintmax_t bdd_loc);
uint8_t remove_BDD_from_manager(BDDManager *BM, uintmax_t bdd_loc);
uint8_t compress_BDD_manager(BDDManager *BM);
uint8_t cluster_BDDs(BDDManager *BM, uintmax_t bdd0, uintmax_t bdd1, unsigned int limit);
DdNode *get_BDD_from_manager(BDDManager *BM, uintmax_t bdd_loc);

//BDD Inferences
extern int apply_BDD_inferences;
extern int apply_BDD_equivalences;

//To turn equivalences off, can either (turn off apply_BDD_equivalences
//AND turn on apply_BDD_inferences) OR (turn off find_equivalences).

uint8_t save_inference(BDDManager *BM, inference_item *inference);
uint8_t find_and_add_inferences_and_equivalences(BDDManager *BM, uintmax_t bdd_loc);
uint8_t find_and_add_all_inferences_and_equivalences(BDDManager *BM);
uint8_t apply_inferences_and_equivalences(BDDManager *BM);

//BDD Quantification
extern int early_quantify_BDD_vars;

uint8_t add_potential_early_quantification_var(BDDManager *BM, uintmax_t var);
uint8_t find_all_early_quantification_vars(BDDManager *BM);
uint8_t perform_early_quantification(BDDManager *BM);

//BDD Safe Assignments
extern int safe_BDD_assignments;

uint8_t find_safe_assignment(BDDManager *BM, uintmax_t v);
uint8_t check_BDD_for_safe_assignment(BDDManager *BM, uintmax_t bdd_loc);
uint8_t find_all_safe_assignments(BDDManager *BM);

#endif
