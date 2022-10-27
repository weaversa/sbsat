/*
 *    Copyright (C) 2011 Sean Weaver
 * 
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2, or (at your option)
 *    any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software Foundation,
 *    Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * */

#include "sbsat.h"

int Cudd_var_ordering_algType = 0;

int sbsat_turn_off_bdds = 0;

char preset_variables_string[4096]="";

void force_preset_variables(BDDManager *BM) {
  intmax_t iter = 0;
  intmax_t str_length = strlen(preset_variables_string);
  intmax_t begin = iter;
  while(iter < str_length) {
    while(preset_variables_string[iter] == ' ' && iter < str_length) iter++;
    if(iter == str_length) break;
    uint8_t sign;
    char p = preset_variables_string[iter];
    if(p == '+') {
      sign = 1;
    } else if(p == '-') {
      sign = 0;
    } else {
      fprintf (stderr, "\nExpected '+' or '-' in --preset-variables, found '%c'", p);
      fprintf (stderr, "\nAll variables must be preceeded by either '+' or '-'...exiting\n");
      exit (1);
    }
    iter++;
    begin = iter;
    p = preset_variables_string[iter];
    while (((p >= 'a') && (p <= 'z')) || ((p >= 'A') && (p <= 'Z'))
	   || (p == '_') || ((p >= '0') && (p <= '9'))) {
      iter++;
      p = preset_variables_string[iter];
      if(iter == str_length) break;
    }
    if((p != ' ' && p != 0) || begin == iter) {
      fprintf(stderr, "\nUnexpected character '%c' in --preset-variables...exiting\n", p);
      exit(1);
    }
    preset_variables_string[iter] = 0;
    intmax_t intnum;
    intnum = i_getsym(preset_variables_string+begin, SYM_VAR);
    d7_printf3("{%jd %s}", intnum, preset_variables_string+begin);
    if(intnum >= BM->nNumVariables) {
      fprintf(stderr, "\nUnknown variable %s found in --preset-variables...exiting\n", preset_variables_string+begin);
      exit(1);
    } else if(intnum == 0) {
      fprintf(stderr, "\nVariable %s cannot be used in --preset-variables...exiting\n", preset_variables_string+begin);
      exit(1);
    }
    if(sign==0) intnum = -intnum;
    
    add_and_BDD(BM, &intnum, 1);
    
    if(iter < str_length) preset_variables_string[iter] = ' ';
  }
}

int8_t var_bddcmp(DdManager *dd, intmax_t x, intmax_t y) {
  if(imaxabs(x) == imaxabs(y))
    return 0;
  if(dd->perm[imaxabs(x)] < dd->perm[imaxabs(y)])
    return -1;
  else return 1;      
}

uintmax_t count_num_BDD_variables(BDDManager *BM) {
  uintmax_t count = 0;
  for(uintmax_t i = 0; i < BM->nNumVariables; i++)
    if(BM->VarBDDMap[i].nLength > 0) count++;
  return count;
}

uint8_t resize_num_BDDs(BDDManager *BM, uintmax_t n_bdds) {
  uint8_t ret = NO_ERROR;
  
  if (n_bdds < BM->nNumBDDs_max) return ret;
  
  int delta = n_bdds - BM->nNumBDDs_max;
  if (delta < 10000) n_bdds = BM->nNumBDDs_max + 10000;
  BM->BDDList = (DdNode **)sbsat_recalloc((void*)BM->BDDList, BM->nNumBDDs_max,
					   n_bdds, sizeof(DdNode*), 9, "BDDList");
  if(BM->BDDList==NULL) return MEM_ERR;

  BM->Support = (uintmax_t_list *)sbsat_recalloc((void*)BM->Support, BM->nNumBDDs_max,
						 n_bdds, sizeof(uintmax_t_list), 9, "BDD Support");
  
  if(BM->Support==NULL) return MEM_ERR;

  for(uintmax_t v = BM->nNumBDDs_max; v < n_bdds; v++) {
    BM->Support[v].nLength = 0;
    BM->Support[v].nLength_max = 0;
    BM->Support[v].pList = NULL;
  }

  BM->nNumBDDs_max = n_bdds;
  return ret;
}

uint8_t resize_num_variables(BDDManager *BM, uintmax_t n_vars) {
  uint8_t ret = NO_ERROR;

  if (n_vars < BM->nNumVariables_max) return ret;

  if(BM->read_input_finished == 1) {
    fprintf(stderr, "Cannot resize variables once preprocessing has begun. The equivalence manager could be inconsistent otherwise. If you need to do this, clone the manager via bdd_manager_clone and then bdd_read_input_finished.\n");
    assert(0);
    exit(0);
  }
  
  int delta = n_vars - BM->nNumVariables_max;
  if (delta < 10000) n_vars = BM->nNumVariables_max + 10000;
  
  /* n_vars -- numinp */
  BM->VarBDDMap = (uintmax_t_list*)sbsat_recalloc((void*)BM->VarBDDMap, BM->nNumVariables_max,
						n_vars, sizeof(uintmax_t_list), 9, "VarBDDMap");
  if(BM->VarBDDMap==NULL) return MEM_ERR;

  for(uintmax_t v = BM->nNumVariables_max; v < n_vars; v++) {
    uintmax_t_list_alloc(&BM->VarBDDMap[v], 2, 2);
  }

  BM->nNumVariables_max = n_vars;

  return ret;
}

void print_variable_occurrence_lists(BDDManager *BM) {
  fprintf(stdout, "\n");
  for(uintmax_t v = 0; v < BM->nNumVariables; v++) {
    fprintf(stdout, "%ju:", v);
    for(uintmax_t y = 0; y < BM->VarBDDMap[v].nLength; y++) {
      fprintf(stdout, " %ju", BM->VarBDDMap[v].pList[y]);
    }
    fprintf(stdout, "\n");
  }
}

void _check_bdd(DdManager *dd, DdNode *bdd);

void check_bdd(DdManager *dd, DdNode *bdd) {
  _check_bdd(dd, Cudd_Regular(bdd));
  ddClearFlag(Cudd_Regular(bdd));
}

void _check_bdd(DdManager *dd, DdNode *bdd) {
  assert(bdd->ref != 0);  
  if(cuddIsConstant(bdd) || Cudd_IsComplement(bdd->next)) return;
  bdd->next = Cudd_Not(bdd->next);

  assert(Cudd_Regular(bdd)->ref > 0);
  
  DdNode *bddT = cuddT(bdd);
  DdNode *bddE = Cudd_Regular(cuddE(bdd));

  if(!cuddIsConstant(bddT)) assert(dd->perm[bdd->index] < dd->perm[bddT->index]);
  if(!cuddIsConstant(bddE)) assert(dd->perm[bdd->index] < dd->perm[bddE->index]);

  _check_bdd(dd, bddT);
  _check_bdd(dd, bddE);
}

void check_BDDList(BDDManager *BM) {
  for(uintmax_t i = 0; i < BM->nNumBDDs; i++)
    check_bdd(BM->dd, BM->BDDList[i]);
}

uint8_t perform_bdd_reductions(BDDManager *BM) {
  uint8_t ret = NO_ERROR;

  if(BM->read_input_finished && BM->in_perform_bdd_reductions==0) {
    BM->in_perform_bdd_reductions = 1;

    ret = apply_inferences_and_equivalences(BM);
    if(ret != NO_ERROR) return ret;

    ret = perform_early_quantification(BM);
    if(ret != NO_ERROR) return ret;

    BM->in_perform_bdd_reductions = 0;
  }

  return ret;
}

uint8_t add_bdd_to_occurrence_list(BDDManager *BM, uintmax_t bdd_loc, uintmax_t v) {
  uint8_t ret = NO_ERROR;

  if(v > BM->nHighestVarInABDD) BM->nHighestVarInABDD = v;

  ret = uintmax_t_list_push(&BM->VarBDDMap[v], bdd_loc);
  if(ret!=NO_ERROR) return ret;

  if(early_quantify_BDD_vars && BM->VarBDDMap[v].nLength == 1) {
    ret = add_potential_early_quantification_var(BM, v);
    if(ret != NO_ERROR) return ret;
  }

  return ret;
}

uint8_t remove_bdd_from_occurrence_list(BDDManager *BM, uintmax_t bdd_loc, uintmax_t v) {
  uint8_t ret = NO_ERROR;
  
  uintmax_t length = BM->VarBDDMap[v].nLength;
  uintmax_t *varmap = BM->VarBDDMap[v].pList;
  
  if(length == 0) {
    fprintf(stderr, "Tried to remove a BDD(%ju) that does not exist in the appropriate occurrence list\n", bdd_loc);
    return BDD_GENERIC_ERR;
  }
  
  uint8_t found = 0;
  for(uintmax_t y = 0; y < length; y++) {
    if(varmap[y] == bdd_loc) {
      varmap[y] = varmap[length-1];
      length--;
      if(length == 1) {
	ret = add_potential_early_quantification_var(BM, v);
	if(ret != NO_ERROR) return ret;
      }
      found = 1;
      break;
    }
  }
  
  if(found==0) {
    fprintf(stderr, "Tried to remove a BDD(%ju) that does not exist in the appropriate occurrence list\n", bdd_loc);
    return BDD_GENERIC_ERR;
  }
  
  BM->VarBDDMap[v].nLength = length;

  return ret;
}

uint8_t add_bdd_to_occurrence_lists(BDDManager *BM, uintmax_t bdd_loc) {
  uint8_t ret = NO_ERROR;

  uintmax_t nVars = Cudd_ReadSize(BM->dd); //returns 1 + highest numbered BDD index
  ret = resize_num_variables(BM, nVars);
  if(ret!=NO_ERROR) {
    fprintf(stderr, "Error: dd returned error %d when allocating more variables.\n", ret);
    return ret;
  }
  
  BM->nNumVariables = nVars;
  
  uintmax_t length = BM->Support[bdd_loc].nLength;
  uintmax_t *support = BM->Support[bdd_loc].pList;
  
  //Add bdd to each variable's occurrence list
  for(uintmax_t i = 0; i < length; i++) {
    uintmax_t v = support[i];
    ret = add_bdd_to_occurrence_list(BM, bdd_loc, v);
    if(ret != NO_ERROR) return ret;
  }
  
  return ret;
}

uint8_t remove_bdd_from_occurrence_lists(BDDManager *BM, uintmax_t bdd_loc) {
  uint8_t ret = NO_ERROR;

  uintmax_t length = BM->Support[bdd_loc].nLength;
  uintmax_t *support = BM->Support[bdd_loc].pList;

  //remove bdd from each variable's occurrence list
  for(uintmax_t i = 0; i < length; i++) {
    uintmax_t v = support[i];
    ret = remove_bdd_from_occurrence_list(BM, bdd_loc, v);
    if(ret != NO_ERROR) return ret;
  }
  
  return ret;
}

uint8_t update_support_and_variable_occurrence_list(BDDManager *BM, uintmax_t bdd_loc) {
  uint8_t ret = NO_ERROR;

  DdNode *bdd = BM->BDDList[bdd_loc];
  BDD_Support(&BM->uTempList.nLength, &(BM->uTempList.nLength_max), &(BM->uTempList.pList), bdd);
  qsort(BM->uTempList.pList, BM->uTempList.nLength, sizeof(uintmax_t), compfunc);
  
  uintmax_t *oldlist = BM->Support[bdd_loc].pList;
  uintmax_t oldlen = BM->Support[bdd_loc].nLength;
  uintmax_t *newlist = BM->uTempList.pList;
  uintmax_t newlen = BM->uTempList.nLength;

  uintmax_t o = 0;
  uintmax_t n = 0;
  while(o < oldlen && n < newlen) {
    if(oldlist[o] == newlist[n]) { o++; n++; continue; }
    if(oldlist[o] < newlist[n]) { //remove oldlist[o]
      ret = remove_bdd_from_occurrence_list(BM, bdd_loc, oldlist[o]);
      if(ret != NO_ERROR) return ret;
      o++;
    } else { //oldlist[o] > newlist[n] //add newlist[n]
      ret = add_bdd_to_occurrence_list(BM, bdd_loc, newlist[n]);
      if(ret != NO_ERROR) return ret;
      n++;
    }
  }

  //leftovers
  if(o < oldlen) {
    //remove all olds
    for(; o < oldlen; o++) {
      ret = remove_bdd_from_occurrence_list(BM, bdd_loc, oldlist[o]);
      if(ret != NO_ERROR) return ret;
    }
  } else { //n < newlen
    //add all news
    for(; n < newlen; n++) {
      ret = add_bdd_to_occurrence_list(BM, bdd_loc, newlist[n]);
      if(ret != NO_ERROR) return ret;
    }
  }

  uintmax_t_list_copy(&BM->Support[bdd_loc], &BM->uTempList);

  ret = check_BDD_for_safe_assignment(BM, bdd_loc);
  if(ret != NO_ERROR) return ret;

  return ret;
}

uint8_t add_BDD_support(BDDManager *BM, uintmax_t bdd_loc) {
  uint8_t ret = NO_ERROR;

  DdNode *bdd = BM->BDDList[bdd_loc];
  BDD_Support(&BM->Support[bdd_loc].nLength, &(BM->Support[bdd_loc].nLength_max), &(BM->Support[bdd_loc].pList), bdd);
  qsort(BM->Support[bdd_loc].pList, BM->Support[bdd_loc].nLength, sizeof(uintmax_t), compfunc);

  ret = check_BDD_for_safe_assignment(BM, bdd_loc);
  if(ret != NO_ERROR) return ret;

  return ret;
}

uint8_t remove_BDD_support(BDDManager *BM, uintmax_t bdd_loc) {
  uint8_t ret = NO_ERROR;

  BM->Support[bdd_loc].nLength = 0;

  return ret;
}

uintmax_t BDDManager_GC(BDDManager *BM) {
  uintmax_t numBDDNodesGarbageCollected = (uintmax_t) cuddGarbageCollect(BM->dd, 1);
  d6_printf2("BDD GC removed %ju BDD nodes\n", numBDDNodesGarbageCollected);
  return numBDDNodesGarbageCollected;
}

DdNode *get_BDD_from_manager(BDDManager *BM, uintmax_t bdd_loc) {
  return BM->BDDList[bdd_loc];
}

uint8_t add_BDD_to_manager(BDDManager *BM, DdNode *bdd) {
  uint8_t ret = NO_ERROR;

  if(BM == NULL) return BDD_NOT_INIT_ERR;
  if(bdd == NULL) return BDD_IS_NULL_ERR;

  cuddRef(bdd);

  uintmax_t bdd_loc = BM->nNumBDDs;

  ret = resize_num_BDDs(BM, bdd_loc);
  if(ret!=NO_ERROR) return 0;
  
  //Invariant: BDDs are always added to the end of the BDD list
  //To remove free spots (True BDDs), use compress_BDD_manager(BDDManager *BM)

  assert(BM->BDDList[bdd_loc] == NULL);
  BM->BDDList[bdd_loc] = bdd;
  BM->nNumBDDs++;

  if(BM->BDDList[bdd_loc] == Cudd_Not(Cudd_ReadOne(BM->dd))) {
    d2_printf1("UNSAT\n");
    return UNSAT;
  }

  ret = add_BDD_support(BM, bdd_loc);
  if(ret != NO_ERROR) return ret;

  ret = add_bdd_to_occurrence_lists(BM, bdd_loc);
  if(ret != NO_ERROR) return ret;

  ret = find_and_add_inferences_and_equivalences(BM, bdd_loc);
  if(ret == FOUND_INFERENCE) ret = NO_ERROR;
  if(ret != NO_ERROR) return ret;

  ret = perform_bdd_reductions(BM);
  if(ret != NO_ERROR) return ret;

  return ret;
}

uint8_t replace_BDD_in_manager(BDDManager *BM, DdNode *new_bdd, uintmax_t bdd_loc) {
  uint8_t ret = NO_ERROR;

  if(BM == NULL) return BDD_NOT_INIT_ERR;
  if(new_bdd == NULL) return BDD_IS_NULL_ERR;

  if(new_bdd == BM->BDDList[bdd_loc]) {
    return ret; //No replacement is needed
  }

  Cudd_Ref(new_bdd);
  Cudd_IterDerefBdd(BM->dd, BM->BDDList[bdd_loc]);
  BM->BDDList[bdd_loc] = new_bdd;

  if(BM->BDDList[bdd_loc] == Cudd_Not(Cudd_ReadOne(BM->dd))) {
    d2_printf1("UNSAT\n");
    return UNSAT;
  }

  ret = update_support_and_variable_occurrence_list(BM, bdd_loc);
  if(ret != NO_ERROR) return ret;

  ret = find_and_add_inferences_and_equivalences(BM, bdd_loc);
  if(ret == FOUND_INFERENCE) ret = NO_ERROR;
  if(ret != NO_ERROR) return ret;

  ret = perform_bdd_reductions(BM);
  if(ret != NO_ERROR) return ret;

  //  Cudd_ReduceHeap(BM->dd, CUDD_REORDER_SAME, 0);

  return ret;
}

uint8_t remove_BDD_from_manager(BDDManager *BM, uintmax_t bdd_loc) {
  uint8_t ret = NO_ERROR;

  if(BM == NULL) return BDD_NOT_INIT_ERR;

  ret = remove_bdd_from_occurrence_lists(BM, bdd_loc);
  if(ret != NO_ERROR) return ret;
  
  ret = remove_BDD_support(BM, bdd_loc);
  if(ret != NO_ERROR) return ret;

  Cudd_IterDerefBdd(BM->dd, BM->BDDList[bdd_loc]);  

  BM->BDDList[bdd_loc] = Cudd_ReadOne(BM->dd);
  cuddRef(BM->BDDList[bdd_loc]);
  //Could maintain a queue of free spots.

  ret = perform_bdd_reductions(BM);
  if(ret != NO_ERROR) return ret;
  
  return ret;
}

uint8_t compress_BDD_manager(BDDManager *BM) {
  uint8_t ret = NO_ERROR;

  if(BM == NULL) return BDD_NOT_INIT_ERR;

  DdNode *one = Cudd_ReadOne(BM->dd);

  //Turn BDD reductions off
  int old_in_perform_bdd_reductions = BM->in_perform_bdd_reductions;
  BM->in_perform_bdd_reductions = 1;

  for(intmax_t i = 0; i < (intmax_t)BM->nNumBDDs; i++) {
    if(BM->BDDList[i] == one) {
      ret = replace_BDD_in_manager(BM, BM->BDDList[BM->nNumBDDs-1], i);
      if(ret != NO_ERROR) return ret;
      ret = remove_BDD_from_manager(BM, BM->nNumBDDs-1);
      if(ret != NO_ERROR) return ret;
      //remove_BDD_from_manager puts a ref'd True node in the BDDList.
      Cudd_IterDerefBdd(BM->dd, BM->BDDList[BM->nNumBDDs-1]);
      BM->BDDList[BM->nNumBDDs-1] = NULL;
      BM->nNumBDDs--;
      i--;
      if(i > 0) assert(BM->BDDList[0] != one);
    }
  }

  BM->in_perform_bdd_reductions = old_in_perform_bdd_reductions;

  if(BM->nNumBDDs > 0) assert(BM->BDDList[0] != one);

  return ret;
}

uint8_t cluster_and_quantify = 1;
int pairwise_bdd_reduction = PAIRWISE_BDD_REDUCTION_NONE;
int bdd_cluster_limit = -1;
unsigned long bdd_clustering_time_limit = 0; //Not sure this works that well

uint8_t cluster_BDDs(BDDManager *BM, uintmax_t bdd0, uintmax_t bdd1, unsigned int limit) {
  uint8_t ret = NO_ERROR;

  assert(bdd0 < BM->nNumBDDs && bdd1 < BM->nNumBDDs);

  if(bdd_cluster_limit > -1) {
    int bdd_union = (int)compute_BDD_union(BM, bdd0, bdd1, (BM->read_input_finished && early_quantify_BDD_vars)?2:0);
    if(bdd_union > bdd_cluster_limit) return CLUSTER_LIMIT;
  }

  //Cube of vars only existing in these two BDDs
  //Really speeds up some benchmarks
  DdNode *overlap_cube = precache_conjunctive_BDD_overlap(BM, bdd0, bdd1, 2);
  DdNode *cluster;

  if(BM->read_input_finished && cluster_and_quantify && early_quantify_BDD_vars) {
    //Cudd_bddClippingAndAbstract //SEAN!!! Try this
    if(bdd_clustering_time_limit) {
      Cudd_ResetStartTime(BM->dd);
      Cudd_SetTimeLimit(BM->dd, limit);
      cluster = Cudd_bddAndAbstract(BM->dd, BM->BDDList[bdd0], BM->BDDList[bdd1], overlap_cube);
      Cudd_UnsetTimeLimit(BM->dd);
    } else {
      cluster = Cudd_bddAndAbstractLimit(BM->dd, BM->BDDList[bdd0], BM->BDDList[bdd1], overlap_cube, limit);
    }

    if(cluster != NULL) {
      if(BM->read_input_finished) d3_printf3("{%ju %ju}", bdd0, bdd1);
      DdNode *bdd = overlap_cube;
      uint8_t neg = Cudd_IsComplement(bdd);
      bdd = Cudd_Regular(bdd);
      while(bdd != Cudd_NotCond(DD_ONE(BM->dd), neg==1)) {
	if(Cudd_Regular(cuddE(bdd)) == Cudd_NotCond(DD_ONE(BM->dd), neg == Cudd_IsComplement(cuddE(bdd)))) {
	  assert(BM->VarBDDMap[bdd->index].nLength == 2);
	  d3_printf2("{*%s}", s_name(bdd->index));
	  bdd = cuddT(bdd);
	} else assert(0);
      }
    }
  } else {
    //Cudd_bddClippingAnd //SEAN!!! Try this
    if(bdd_clustering_time_limit) {
      Cudd_ResetStartTime(BM->dd);
      Cudd_SetTimeLimit(BM->dd, limit);
      cluster = Cudd_bddAnd(BM->dd, BM->BDDList[bdd0], BM->BDDList[bdd1]);
      Cudd_UnsetTimeLimit(BM->dd);
    } else {
      cluster = Cudd_bddAndLimit(BM->dd, BM->BDDList[bdd0], BM->BDDList[bdd1], limit);
    }
    if(cluster != NULL)
      if(BM->read_input_finished) d3_printf3("{%ju %ju}", bdd0, bdd1);
  }

  Cudd_IterDerefBdd(BM->dd, overlap_cube);
  
  if(cluster == NULL) { //hit limit, consider reductions
    if(pairwise_bdd_reduction == PAIRWISE_BDD_REDUCTION_NONE)
      return CLUSTER_LIMIT;
    
    if(pairwise_bdd_reduction == PAIRWISE_BDD_REDUCTION_RESTRICT) {
      ret = replace_BDD_in_manager(BM, Cudd_bddRestrictLimit(BM->dd, BM->BDDList[bdd0], BM->BDDList[bdd1], limit>>1), bdd0);
      if(ret != BDD_IS_NULL_ERR) {
	if(ret != NO_ERROR) return ret;
	ret = replace_BDD_in_manager(BM, Cudd_bddRestrictLimit(BM->dd, BM->BDDList[bdd1], BM->BDDList[bdd0], limit>>1), bdd1);
      }
    } else if(pairwise_bdd_reduction == PAIRWISE_BDD_REDUCTION_LICOMPACTION) {
      ret = replace_BDD_in_manager(BM, Cudd_bddLICompactionLimit(BM->dd, BM->BDDList[bdd0], BM->BDDList[bdd1], limit>>1), bdd0);
      if(ret != BDD_IS_NULL_ERR) {
	if(ret != NO_ERROR) return ret;
	ret = replace_BDD_in_manager(BM, Cudd_bddLICompactionLimit(BM->dd, BM->BDDList[bdd1], BM->BDDList[bdd0], limit>>1), bdd1);
      }
    } else if(pairwise_bdd_reduction == PAIRWISE_BDD_REDUCTION_NPAND) {
      ret = replace_BDD_in_manager(BM, Cudd_bddNPAndLimit(BM->dd, BM->BDDList[bdd0], BM->BDDList[bdd1], 0), bdd0);
      if(ret != BDD_IS_NULL_ERR) {
	if(ret != NO_ERROR) return ret;
	ret = replace_BDD_in_manager(BM, Cudd_bddNPAndLimit(BM->dd, BM->BDDList[bdd1], BM->BDDList[bdd0], 0), bdd1);
      }
    } else {
      //Unknown reduction method
      return CLUSTER_LIMIT;
    }

    if(ret != BDD_IS_NULL_ERR) {
      if(ret != NO_ERROR) return ret;
      
      if(BM->read_input_finished) {
	d3_printf1("|");
	//Compress the BDDList in case _some_ BDD became True either by inference or other reduction
	ret = compress_BDD_manager(BM);
	if(ret != NO_ERROR) return ret;
      }
    }
    return CLUSTER_LIMIT;
  }

  Cudd_Ref(cluster);

  if(cluster == Cudd_Not(DD_ONE(BM->dd))) {
    d2_printf1("UNSAT\n");
    Cudd_IterDerefBdd(BM->dd, cluster);
    return UNSAT;
  }
  
  //Turn BDD reductions off
  int old_in_perform_bdd_reductions = BM->in_perform_bdd_reductions;
  BM->in_perform_bdd_reductions = 1;

  ret = replace_BDD_in_manager(BM, cluster, bdd0);
  Cudd_IterDerefBdd(BM->dd, cluster);
  if(ret != NO_ERROR) return ret;

  //Turn BDD reductions back on
  BM->in_perform_bdd_reductions = old_in_perform_bdd_reductions;

  assert(BM->Support[bdd0].nLength <= (uintmax_t) bdd_cluster_limit);

  ret = remove_BDD_from_manager(BM, bdd1);
  if(ret != NO_ERROR) return ret;
  
  if(BM->read_input_finished) {
    ret = compress_BDD_manager(BM);
    if(ret != NO_ERROR) return ret;
  }

  if(BM->read_input_finished) d3_printf2("[%ju]", BM->nNumBDDs);

  return ret;
}

uint8_t bdd_init(BDDManager *BM, uintmax_t nNumVars) {
  uint8_t ret = NO_ERROR;
  
  BM->dd = Cudd_Init(nNumVars, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, sbsat_max_ram*1048576 /* megabytes to bytes */);
  if(BM->dd == NULL) return MEM_ERR;

  Cudd_ResetStartTime(BM->dd);

  Cudd_ReorderingType algType;
  switch(Cudd_var_ordering_algType) {
  case 0: algType = CUDD_REORDER_NONE; break;
  case 1: algType = CUDD_REORDER_RANDOM_PIVOT; break;
  case 2: algType = CUDD_REORDER_RANDOM; break;
  case 3: algType = CUDD_REORDER_SIFT; break;
  case 4: algType = CUDD_REORDER_SIFT_CONVERGE; break;
  case 5: algType = CUDD_REORDER_SYMM_SIFT; break;
  case 6: algType = CUDD_REORDER_SYMM_SIFT_CONV; break;
  case 7: algType = CUDD_REORDER_WINDOW2; break;
  case 8: algType = CUDD_REORDER_WINDOW3; break;
  case 9: algType = CUDD_REORDER_WINDOW4; break;
  case 10: algType = CUDD_REORDER_WINDOW2_CONV; break;
  case 11: algType = CUDD_REORDER_WINDOW3_CONV; break;
  case 12: algType = CUDD_REORDER_WINDOW4_CONV; break;
  case 13: algType = CUDD_REORDER_GROUP_SIFT; break;
  case 14: algType = CUDD_REORDER_GROUP_SIFT_CONV; break;
  case 15: algType = CUDD_REORDER_ANNEALING; break;
  case 16: algType = CUDD_REORDER_GENETIC; break;
  case 17: algType = CUDD_REORDER_LAZY_SIFT; break;
    //    case 18: algType = CUDD_REORDER_LINEAR; break;
    //    case 19: algType = CUDD_REORDER_LINEAR_CONVERGE; break;
    //    case 20: algType = CUDD_REORDER_EXACT; break;
  default: fprintf(stderr, "Invalid BDD variable ordering heuristic specified (%d)...choosing none.\n", Cudd_var_ordering_algType);
    algType = CUDD_REORDER_NONE; break;
  }

  BM->dd->autoMethod = algType;
  Cudd_AutodynEnable(BM->dd, algType);
  
  BM->nNumBDDs = 0;
  BM->nNumBDDs_max = 0;
  BM->BDDList = NULL;
  BM->Support = NULL;
  BM->nNumVariables = 0;
  BM->nNumVariables_max = 0;
  BM->nHighestVarInABDD = 0;
  BM->VarBDDMap = NULL;
  ret = uintmax_t_list_alloc(&BM->uTempList, 10, 10); if(ret != NO_ERROR) return ret;
  ret = uint32_t_list_alloc(&BM->uTempList32, 10, 10); if(ret != NO_ERROR) return ret;
  ret = uintmax_t_list_alloc(&BM->PossibleExQuantVars, 10, 10); if(ret != NO_ERROR) return ret;
  ret = uintmax_t_list_alloc(&BM->ExQuantProtectedVars, 10, 10); if(ret != NO_ERROR) return ret;
  BM->ExQuantProtectVars = 0;
  BM->InferenceQueue = queue_init();
  BM->equ_var = 0;
  BM->read_input_finished = 0;
  BM->EQManager = NULL;
  BM->infer_pool = NULL;
  BM->in_perform_bdd_reductions = 0;

  ret = resize_num_variables(BM, nNumVars+1);
  if(ret != NO_ERROR) return ret;

  return ret;
}

uint8_t bdd_manager_clone_inferences(BDDManager *BM, BDDManager *BM_old) {
  uint8_t ret = NO_ERROR;
  inference_item inference;
  for(intmax_t i = 0; i < BM_old->nNumVariables; i++) {
    if(BM->VarBDDMap[i].nLength > 0) {
      intmax_t v = get_equiv(BM_old->EQManager, i);
      if(v == i) {
	v = get_oppos(BM_old->EQManager, i);
	if(v != i && apply_BDD_equivalences && (BM->VarBDDMap[v].nLength > 0)) {
	  inference.lft = i;
	  inference.rgt = -v;
	  ret = save_inference(BM, &inference);
	  if(ret != NO_ERROR) return ret;
	}
      } else {
	inference.lft = i;
	if(v == BM_old->EQManager->True) {
	  inference.rgt = BM->EQManager->True;
	  ret = save_inference(BM, &inference);
	  if(ret != NO_ERROR) return ret;
	} else if(v == BM_old->EQManager->False) {
	  inference.rgt = BM->EQManager->False;
	  ret = save_inference(BM, &inference);
	  if(ret != NO_ERROR) return ret;
	} else if(apply_BDD_equivalences && BM->VarBDDMap[v].nLength > 0) {
	  inference.rgt = v;
	  ret = save_inference(BM, &inference);
	  if(ret != NO_ERROR) return ret;
	}
      }
    }
  }
  ret = apply_inferences_and_equivalences(BM);
  if(ret != NO_ERROR) return ret;
  ret = find_all_early_quantification_vars(BM);
  if(ret != NO_ERROR) return ret;
  ret = find_all_safe_assignments(BM);
  if(ret != NO_ERROR) return ret;

  return ret;
}

uint8_t bdd_manager_clone(BDDManager *BM, BDDManager *BM_old) {
  uint8_t ret = NO_ERROR;
  
  ret = bdd_init(BM, 0);
  if(ret != NO_ERROR) return ret;
  
  for(uintmax_t i = 0; i < BM_old->nNumBDDs; i++) {
    ret = add_BDD_to_manager(BM, clone_BDD(BM->dd, BM_old->dd, BM_old->BDDList[i]));
    if(ret != NO_ERROR) return ret;
  }

  return ret;
}

uint8_t bdd_read_input_finished(BDDManager *BM) {
  uint8_t ret;
  
  force_preset_variables(BM);

  BM->read_input_finished = 1;

  BM->infer_pool = init_inference_pool();
  BM->EQManager = equiv_init(BM->nNumVariables);

  ret = find_and_add_all_inferences_and_equivalences(BM);
  if(ret != NO_ERROR) return ret;

  ret = find_all_early_quantification_vars(BM);
  if(ret != NO_ERROR) return ret;

  //SEAN!!! Look at Cudd_Increasing...similar to safe assignments?
  ret = find_all_safe_assignments(BM);
  if(ret != NO_ERROR) return ret;

  //  check_BDDList(BM);

  ret = perform_bdd_reductions(BM);
  if(ret != NO_ERROR) return ret;

  ret = compress_BDD_manager(BM);
  if(ret != NO_ERROR) return ret;

  return ret;
}

uint8_t bdd_free(BDDManager *BM) {
  uint8_t ret = NO_ERROR;
  
  for(uintmax_t x = 0; x < BM->nNumBDDs; x++) {
    Cudd_IterDerefBdd(BM->dd, BM->BDDList[x]);
  }

  //  Cudd_PrintInfo(BM->dd, stderr);
  
  int ref;
  if((ref=Cudd_CheckZeroRef(BM->dd))==0) {
    Cudd_Quit (BM->dd);
  } else {
    fprintf(stderr, "Cudd BDD manager free'd but %d nodes still referenced.\n", ref);
    return BDD_FREE_ERR;
  }
  
  if(BM->BDDList!=NULL) {
    sbsat_free((void**)&(BM->BDDList));
    BM->BDDList = NULL;
    BM->nNumBDDs = 0;
  }
  
  if(BM->Support!=NULL) {
    for(uintmax_t x = 0; x < BM->nNumBDDs_max; x++)
      uintmax_t_list_free(&BM->Support[x]);
    sbsat_free((void**)&(BM->Support));
    BM->Support = NULL;
  }

  if(BM->VarBDDMap!=NULL) {
    for(uintmax_t v = 0; v < BM->nNumVariables_max; v++)
      uintmax_t_list_free(&BM->VarBDDMap[v]);
    sbsat_free((void**)&(BM->VarBDDMap));
    BM->VarBDDMap = NULL;
  }
 
  uintmax_t_list_free(&BM->uTempList);
  uint32_t_list_free(&BM->uTempList32);
  uintmax_t_list_free(&BM->PossibleExQuantVars);
  uintmax_t_list_free(&BM->ExQuantProtectedVars);
  
  while(!queue_is_empty(BM->InferenceQueue)) {
    dequeue(BM->InferenceQueue);
  }
  queue_free(BM->InferenceQueue);

  BM->nNumBDDs = 0;
  BM->nNumBDDs_max = 0;
  
  BM->nNumVariables = 0;
  BM->nNumVariables_max = 0;

  BM->nHighestVarInABDD = 0;

  BM->equ_var = 0;
  BM->read_input_finished = 0;

  if(BM->EQManager) {
    equiv_free(BM->EQManager);
    BM->EQManager = NULL;
  }

  if(BM->infer_pool) {
    free_inference_pool(BM->infer_pool);
    BM->infer_pool = NULL;
  }

  return ret;
}
