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

enum {
  MITOSIS_FLAG_NUMBER,
  MAX_FLAG_NUMBER
};

uintmax_t bdd_flag_numbers[MAX_FLAG_NUMBER] = {0};

uintmax_t increment_and_return_bdd_flag_number(int bdd_flag) {
  bdd_flag_numbers[bdd_flag]++;
  if (bdd_flag_numbers[bdd_flag] == INT_MAX) {
    bdd_flag_numbers[bdd_flag] = 0;
  }
  return bdd_flag_numbers[bdd_flag];
}

//Should really write my own qsort rather than use this. SEAN!!!
int bddperm_compfunc (const void *x, const void *y) {
   uintmax_t pp, qq;
   
   pp = *(const uintmax_t *) x;
   qq = *(const uintmax_t *) y;
   if (BM_main->dd->perm[pp] < BM_main->dd->perm[qq])
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

void _BDD_Support(uintmax_t *length, uintmax_t *max, uintmax_t **tempint, DdNode *f);

//Populate a list with variables occurring in BDD f.

void BDD_Support(uintmax_t *length, uintmax_t *max, uintmax_t **tempint, DdNode *f) {
  *length=0;
  _BDD_Support(length, max, tempint, Cudd_Regular(f));
  for (uintmax_t i = 0; i < *length; i++) {
    // clear the flag
    sym_reset_flag((*tempint)[i]);
  }
  ddClearFlag(Cudd_Regular(f));
}

//Populate a list with variables occurring in BDD f and g (with f occuring first).

void BDD_Support2(uintmax_t *length, uintmax_t *max, uintmax_t **tempint, DdNode *f, DdNode *g) {
  *length=0;

  _BDD_Support(length, max, tempint, Cudd_Regular(f));
  _BDD_Support(length, max, tempint, Cudd_Regular(g));
  for (uintmax_t i = 0; i < *length; i++) {
    // clear the flag
    sym_reset_flag((*tempint)[i]);
  }

  ddClearFlag(Cudd_Regular(f));
  ddClearFlag(Cudd_Regular(g));
}

void _BDD_Support(uintmax_t *length, uintmax_t *max, uintmax_t **tempint, DdNode *f) {
  if (cuddIsConstant(f) || Cudd_IsComplement(f->next)) return;
  
  /* Mark as visited. */
  f->next = Cudd_Not(f->next);
  
  if (sym_is_flag(f->index) == 0) {
    if (*length >= *max) {
      *tempint = (uintmax_t*)sbsat_recalloc(*(void**)tempint, *max, *max+100, sizeof(uintmax_t), 9, "tempint");
      *max += 100;
    }
    (*tempint)[*length] = f->index;
    sym_set_flag(f->index);
    (*length)++;
  }
  _BDD_Support (length, max, tempint, cuddT(f));
  _BDD_Support (length, max, tempint, Cudd_Regular(cuddE(f)));
}

uintmax_t _highest_var_in_BDD(DdNode *f) {
  if (cuddIsConstant(f) || Cudd_IsComplement(f->next)) return 0;

  /* Mark as visited. */
  f->next = Cudd_Not(f->next);

  uintmax_t max1 = _highest_var_in_BDD(cuddT(f));
  uintmax_t max2 = _highest_var_in_BDD(Cudd_Regular(cuddE(f)));
  uintmax_t max3 = f->index;
  if(max1 > max2) {
    if(max1 > max3) return max1;
    return max3;
  } else if(max3 > max2) {
    return max3;
  } else return max2;
}

uintmax_t highest_var_in_BDD(DdNode *f) {
  uintmax_t max = _highest_var_in_BDD(Cudd_Regular(f));
  ddClearFlag(Cudd_Regular(f));
  return max;

}

DdNode *Mitosis_Recur(DdManager *dd, DdNode *f, uint32_t *map, DdNode *unique) {
  DdNode *F = Cudd_Regular(f);
  DdNode *res;

  if (cuddIsConstant(F)) return f;

  if (F->ref > 1 && (res = cuddCacheLookup2(dd, (DdNode* (*)(DdManager*, DdNode*, DdNode*))Mitosis_Recur, f, unique)) != NULL) {
    return(res);
  }

  /* Split and recur on children of this node. */
  DdNode *T = Mitosis_Recur(dd, cuddT(F), map, unique);
  if (T == NULL) return(NULL);
  cuddRef(T);
  DdNode *E = Mitosis_Recur(dd, cuddE(F), map, unique);
  if (E == NULL) {
    Cudd_IterDerefBdd(dd, T);
    return(NULL);
  }
  cuddRef(E);
  
  /* Move variable that should be in this position to this position
  ** by retrieving the single var BDD for that variable, and calling
  ** cuddBddIteRecur with the T and E we just created.
  */

  uint32_t index = map[F->index];
  res = cuddBddIteRecur(dd, dd->vars[index], T, E);
  if (res == NULL) {
    Cudd_IterDerefBdd(dd, T);
    Cudd_IterDerefBdd(dd, E);
    return(NULL);
  }
  cuddRef(res);
  Cudd_IterDerefBdd(dd, T);
  Cudd_IterDerefBdd(dd, E);
  cuddDeref(res);
  
  if (F->ref > 1)
    cuddCacheInsert2(dd, (DdNode* (*)(DdManager*, DdNode*, DdNode*)) Mitosis_Recur, f, unique, Cudd_NotCond(res, F != f));

  return(Cudd_NotCond(res, F != f));
}

DdNode *Mitosis(DdManager *dd, DdNode *f, uint32_t *map) {
  DdNode *unique = (DdNode *)increment_and_return_bdd_flag_number(MITOSIS_FLAG_NUMBER);
  return Mitosis_Recur(dd, f, map, unique);    
}

DdNode *_clone_BDD(DdManager *dd, DdManager *dd_old, DdNode *f, st_table *table) {
  if (f == DD_ONE(dd_old)) return DD_ONE(dd);
  if (f == Cudd_Not(DD_ONE(dd_old))) return Cudd_Not(DD_ONE(dd));

  DdNode *F = Cudd_Regular(f);
  DdNode *T, *E;

  DdNode *cached;
  if(st_lookup(table, f, &cached)) {
    return cached;
  }

  T = cuddT(F); E = cuddE(F);
  if(Cudd_IsComplement(f)) {
    T = Cudd_Not(T); E = Cudd_Not(E);
  }

  DdNode *r = _clone_BDD(dd, dd_old, T, table); Cudd_Ref(r);
  DdNode *e = _clone_BDD(dd, dd_old, E, table); Cudd_Ref(e);

  DdNode *ret = Cudd_bddIte(dd, Cudd_bddIthVar(dd, F->index), r, e);
  Cudd_Ref(ret);
  Cudd_IterDerefBdd(dd, r);
  Cudd_IterDerefBdd(dd, e);

  if(st_add_direct(table, (char *)f, (char *)ret) == ST_OUT_OF_MEM) {
    assert(0);
    exit(0);
  }
  
  cuddDeref(ret);
  return ret;
}

DdNode *clone_BDD(DdManager *dd, DdManager *dd_old, DdNode *f) {
  if(sbsat_turn_off_bdds) return DD_ONE(dd);
  Cudd_AutodynDisable(dd);

  st_table *table;
  table = st_init_table(st_ptrcmp,st_ptrhash);
  if (table == NULL) {
    return NULL;

  }

  DdNode *ret = _clone_BDD(dd, dd_old, f, table);
  
  st_free_table(table);

  Cudd_AutodynEnable(dd, CUDD_REORDER_SAME);
  return ret;
}
  
DdNode *build_and_BDD(DdManager *dd, intmax_t *variables, uintmax_t length) {
  if(sbsat_turn_off_bdds) return DD_ONE(dd);

  Cudd_AutodynDisable(dd);
  DdNode *tmp_lit;
  DdNode *bTemp;
  DdNode *clause = DD_ONE(dd);
  cuddRef(clause);
  for(intmax_t y = length-1; y >=0; y--) {
    tmp_lit = Cudd_NotCond(Cudd_bddIthVar(dd, imaxabs(variables[y])), variables[y]<0);
    clause = Cudd_bddAnd(dd, tmp_lit, bTemp = clause); cuddRef(clause);
    Cudd_IterDerefBdd(dd, bTemp);
  }
  
  Cudd_AutodynEnable(dd, CUDD_REORDER_SAME);
  
  cuddDeref(clause);
  return clause;
}

DdNode *build_or_BDD(DdManager *dd, intmax_t *variables, uintmax_t length) {
  if(sbsat_turn_off_bdds) return DD_ONE(dd);

  Cudd_AutodynDisable(dd);
  DdNode *tmp_lit;
  DdNode *bTemp;
  DdNode *clause = Cudd_Not(DD_ONE(dd));
  cuddRef(clause);
  for(intmax_t y = length-1; y >=0; y--) {
    tmp_lit = Cudd_NotCond(Cudd_bddIthVar(dd, imaxabs(variables[y])), variables[y]<0);
    clause = Cudd_bddOr(dd, tmp_lit, bTemp = clause); cuddRef(clause);
    Cudd_IterDerefBdd(dd, bTemp);
  }
  
  Cudd_AutodynEnable(dd, CUDD_REORDER_SAME);
  
  cuddDeref(clause);
  return clause;
}

DdNode *build_xor_BDD(DdManager *dd, intmax_t *variables, uintmax_t length) {
  if(sbsat_turn_off_bdds) return DD_ONE(dd);

  Cudd_AutodynDisable(dd);
  
  DdNode *tmp_lit;
  DdNode *bTemp;
  DdNode *xor_clause = Cudd_Not(DD_ONE(dd));
  cuddRef(xor_clause);
  for(intmax_t y = length-1; y >=0; y--) {
    tmp_lit = Cudd_NotCond(Cudd_bddIthVar(dd, imaxabs(variables[y])), variables[y]<0);
    xor_clause = Cudd_bddXor(dd, tmp_lit, bTemp = xor_clause); cuddRef(xor_clause);
    Cudd_IterDerefBdd(dd, bTemp);
  }
  
  Cudd_AutodynEnable(dd, CUDD_REORDER_SAME);
  
  cuddDeref(xor_clause);
  return xor_clause;
}

DdNode *build_and_equals_BDD(DdManager *dd, intmax_t equal_var, intmax_t *variables, uintmax_t length) {
  if(sbsat_turn_off_bdds) return DD_ONE(dd);

  Cudd_AutodynDisable(dd);
  
  DdNode *tmp_lit;
  DdNode *bTemp;
  DdNode *cube = DD_ONE(dd); cuddRef(cube);
  for(intmax_t y = length-1; y >=0; y--) {
    if(imaxabs(variables[y]) == equal_var) continue;
    tmp_lit = Cudd_NotCond(Cudd_bddIthVar(dd, imaxabs(variables[y])), variables[y]<0);
    bTemp = Cudd_bddAnd(dd, tmp_lit, cube);
    if(bTemp == NULL) {
      Cudd_IterDerefBdd(dd, cube);
      return NULL;
    }
    cuddRef(bTemp);
    Cudd_IterDerefBdd(dd, cube);
    cube = bTemp;
  }
  
  tmp_lit = Cudd_NotCond(Cudd_bddIthVar(dd, imaxabs(equal_var)), equal_var>0);
  bTemp = Cudd_bddXor(dd, tmp_lit, cube);

  if(bTemp == NULL) {
    Cudd_IterDerefBdd(dd, cube);
    return NULL;
  }
  cuddRef(bTemp);
  Cudd_IterDerefBdd(dd, cube);
  cube = bTemp;
 
  Cudd_AutodynEnable(dd, CUDD_REORDER_SAME);
  
  cuddDeref(cube);
  return cube;
}

DdNode *build_or_equals_BDD(DdManager *dd, intmax_t equal_var, intmax_t *variables, uintmax_t length) {
  if(sbsat_turn_off_bdds) return DD_ONE(dd);

  Cudd_AutodynDisable(dd);
  
  DdNode *tmp_lit;
  DdNode *bTemp;
  DdNode *clause = Cudd_Not(DD_ONE(dd)); cuddRef(clause);
  for(intmax_t y = length-1; y >=0; y--) {
    if(imaxabs(variables[y]) == equal_var) continue;
    tmp_lit = Cudd_NotCond(Cudd_bddIthVar(dd, imaxabs(variables[y])), variables[y]<0);
    bTemp = Cudd_bddOr(dd, tmp_lit, clause);
    if(bTemp == NULL) {
      Cudd_IterDerefBdd(dd, clause);
      return NULL;
    }
    cuddRef(bTemp);
    Cudd_IterDerefBdd(dd, clause);
    clause = bTemp;
  }

  tmp_lit = Cudd_NotCond(Cudd_bddIthVar(dd, imaxabs(equal_var)), equal_var>0);
  bTemp = Cudd_bddXor(dd, tmp_lit, clause);

  if(bTemp == NULL) {
    Cudd_IterDerefBdd(dd, clause);
    return NULL;
  }
  cuddRef(bTemp);

  Cudd_IterDerefBdd(dd, clause);
  clause = bTemp;
 
  Cudd_AutodynEnable(dd, CUDD_REORDER_SAME);
  
  cuddDeref(clause);
  return clause;
}

DdNode *build_ite_equals_BDD(DdManager *dd, intmax_t equal_var, intmax_t v1, intmax_t v2, intmax_t v3) {
  if(sbsat_turn_off_bdds) return DD_ONE(dd);

  Cudd_AutodynDisable(dd);
  
  DdNode *tmp_lit;
  DdNode *bTemp;
  DdNode *v1_dd = Cudd_NotCond(Cudd_bddIthVar(dd, imaxabs(v1)), v1<0);
  DdNode *v2_dd = Cudd_NotCond(Cudd_bddIthVar(dd, imaxabs(v2)), v2<0);
  DdNode *v3_dd = Cudd_NotCond(Cudd_bddIthVar(dd, imaxabs(v3)), v3<0);
  DdNode *ite_equals = Cudd_bddIte(dd, v1_dd, v2_dd, v3_dd);
  cuddRef(ite_equals);
  
  tmp_lit = Cudd_NotCond(Cudd_bddIthVar(dd, imaxabs(equal_var)), equal_var<0);
  ite_equals = Cudd_Not(Cudd_bddXor(dd, tmp_lit, bTemp = ite_equals)); cuddRef(ite_equals);
  Cudd_IterDerefBdd(dd, bTemp);
  
  Cudd_AutodynEnable(dd, CUDD_REORDER_SAME);
  
  cuddDeref(ite_equals);
  return ite_equals;
}

DdNode *build_majv_equals_BDD(DdManager *dd, intmax_t equal_var, intmax_t v1, intmax_t v2, intmax_t v3) {
  if(sbsat_turn_off_bdds) return DD_ONE(dd);

  Cudd_AutodynDisable(dd);
  
  DdNode *tmp_lit;
  DdNode *bTemp;
  DdNode *v1_dd = Cudd_NotCond(Cudd_bddIthVar(dd, imaxabs(v1)), v1<0);
  DdNode *v2_dd = Cudd_NotCond(Cudd_bddIthVar(dd, imaxabs(v2)), v2<0);
  DdNode *v3_dd = Cudd_NotCond(Cudd_bddIthVar(dd, imaxabs(v3)), v3<0);
  DdNode *or_bdd = Cudd_bddOr(dd, v2_dd, v3_dd); cuddRef(or_bdd);
  DdNode *and_bdd = Cudd_bddAnd(dd, v2_dd, v3_dd); cuddRef(and_bdd);
  DdNode *majv_equals = Cudd_bddIte(dd, v1_dd, or_bdd, and_bdd); cuddRef(majv_equals);
  Cudd_IterDerefBdd(dd, or_bdd);
  Cudd_IterDerefBdd(dd, and_bdd);
  
  tmp_lit = Cudd_NotCond(Cudd_bddIthVar(dd, imaxabs(equal_var)), equal_var<0);
  majv_equals = Cudd_Not(Cudd_bddXor(dd, tmp_lit, bTemp = majv_equals)); cuddRef(majv_equals);
  Cudd_IterDerefBdd(dd, bTemp);
  
  Cudd_AutodynEnable(dd, CUDD_REORDER_SAME);
  
  cuddDeref(majv_equals);
  return majv_equals;
}

DdNode *build_cardinality_BDD(DdManager *dd, uintmax_t min, uintmax_t max, uintmax_t *variables, uintmax_t length) {
  if(sbsat_turn_off_bdds) return DD_ONE(dd);

  uintmax_t i;
  intmax_t j;
  DdNode *bTemp, *result;
  if(max < min) return Cudd_Not(DD_ONE(dd));
  Cudd_AutodynDisable(dd);
  //It is a good idea to sort 'variables' according to dd->perm before building a cardinality BDD.
  //for(i = 1; ((uintmax_t) i) < length; i++) assert(dd->perm[variables[i-1]] < dd->perm[variables[i]]);
  DdNode **arr = (DdNode **)sbsat_calloc(length+1, sizeof(DdNode*), 9, "create_cardinality_BDD");
  arr[0] = Cudd_NotCond(DD_ONE(dd), (0 >= min)==0); cuddRef(arr[0]);
  for(i = 1; i <= length; i++) {
    DdNode *prev_node = Cudd_NotCond(DD_ONE(dd), (i >= min && i <= max)==0); cuddRef(prev_node);
    if(prev_node != arr[0]) {
      arr[0] = Cudd_bddIte(dd, Cudd_bddIthVar(dd, variables[length-1]), prev_node, bTemp = arr[0]); cuddRef(arr[0]);
      Cudd_IterDerefBdd(dd, bTemp);
    }
    for(j = 1; j < (intmax_t) i; j++) {
      if(arr[j] != arr[j-1]) {
	arr[j] = Cudd_bddIte(dd, Cudd_bddIthVar(dd, variables[(length-1)-j]), arr[j-1], bTemp = arr[j]); cuddRef(arr[j]);
	Cudd_IterDerefBdd(dd, bTemp);
      }
    }
    for(j = i; j > 0; j--)
      arr[j] = arr[j-1];
    arr[0] = prev_node;
  }
  result = arr[length];

  for(i = 0; i < length; i++) {
    Cudd_IterDerefBdd(dd, arr[i]);
  }
  
  Cudd_AutodynEnable(dd, CUDD_REORDER_SAME);
  free(arr);
  
  Cudd_Deref(result);
  return result;
}

/****************************************************************************************************/

uintmax_t add_and_BDD(BDDManager *BM, intmax_t *variables, uintmax_t length) {
  return add_BDD_to_manager(BM, build_and_BDD(BM->dd, variables, length));
}

uintmax_t add_or_BDD(BDDManager *BM, intmax_t *variables, uintmax_t length) {
  return add_BDD_to_manager(BM, build_or_BDD(BM->dd, variables, length));
}

uintmax_t add_xor_BDD(BDDManager *BM, intmax_t *variables, uintmax_t length) {
  return add_BDD_to_manager(BM, build_xor_BDD(BM->dd, variables, length));
}

uintmax_t add_and_equals_BDD(BDDManager *BM, intmax_t equal_var, intmax_t *variables, uintmax_t length) {
  return add_BDD_to_manager(BM, build_and_equals_BDD(BM->dd, equal_var, variables, length));
}

uintmax_t add_or_equals_BDD(BDDManager *BM, intmax_t equal_var, intmax_t *variables, uintmax_t length) {
  return add_BDD_to_manager(BM, build_or_equals_BDD(BM->dd, equal_var, variables, length));
}

uintmax_t add_ite_equals_BDD(BDDManager *BM, intmax_t equal_var, intmax_t v1, intmax_t v2, intmax_t v3) {
  return add_BDD_to_manager(BM, build_ite_equals_BDD(BM->dd, equal_var, v1, v2, v3));
}

uintmax_t add_majv_equals_BDD(BDDManager *BM, intmax_t equal_var, intmax_t v1, intmax_t v2, intmax_t v3) {
  return add_BDD_to_manager(BM, build_majv_equals_BDD(BM->dd, equal_var, v1, v2, v3));
}

uintmax_t add_cardinality_BDD(BDDManager *BM, uintmax_t min, uintmax_t max, uintmax_t *variables, uintmax_t length) {
  return add_BDD_to_manager(BM, build_cardinality_BDD(BM->dd, min, max, variables, length));
}

DdNode *precache_conjunctive_BDD_overlap(BDDManager *BM, uintmax_t bdd0, uintmax_t bdd1, uintmax_t var_limit) {
  DdNode *precache_cube = Cudd_ReadOne(BM->dd); Cudd_Ref(precache_cube);
  
  uintmax_t bdd0_i = 0;
  uintmax_t bdd1_i = 0;
  
  while(bdd0_i < BM->Support[bdd0].nLength && bdd1_i < BM->Support[bdd1].nLength) {
    if(BM->Support[bdd0].pList[bdd0_i] == BM->Support[bdd1].pList[bdd1_i]) {
      uintmax_t bdd_var = BM->Support[bdd0].pList[bdd0_i];
      if(bdd_var >= BM->ExQuantProtectedVars.nLength || BM->ExQuantProtectedVars.pList[bdd_var] == 0) {
	if(BM->VarBDDMap[bdd_var].nLength <= var_limit) {
	  DdNode *bTemp = Cudd_bddAnd(BM->dd, Cudd_bddIthVar(BM->dd, bdd_var), precache_cube); Cudd_Ref(bTemp);
	  Cudd_IterDerefBdd(BM->dd, precache_cube);
	  precache_cube = bTemp;
	}
      }
      bdd0_i++; bdd1_i++;
    } else if(BM->Support[bdd0].pList[bdd0_i] <= BM->Support[bdd1].pList[bdd1_i]) {
      bdd0_i++;
    } else {
      bdd1_i++;
    }
  }
  return precache_cube;
}

uintmax_t compute_BDD_overlap(BDDManager *BM, uintmax_t bdd0, uintmax_t bdd1, uintmax_t var_limit) {
  uintmax_t overlap = 0;
  
  uintmax_t bdd0_i = 0;
  uintmax_t bdd1_i = 0;
  
  while(bdd0_i < BM->Support[bdd0].nLength && bdd1_i < BM->Support[bdd1].nLength) {
    if(BM->Support[bdd0].pList[bdd0_i] == BM->Support[bdd1].pList[bdd1_i]) {
      if(BM->VarBDDMap[BM->Support[bdd0].pList[bdd0_i]].nLength > var_limit) {
	overlap++;
      }
      bdd0_i++; bdd1_i++;
    } else if(BM->Support[bdd0].pList[bdd0_i] <= BM->Support[bdd1].pList[bdd1_i]) {
      bdd0_i++;
    } else {
      bdd1_i++;
    }
  }
  return overlap;
}

uintmax_t compute_BDD_union(BDDManager *BM, uintmax_t bdd0, uintmax_t bdd1, uintmax_t var_limit) {
  uintmax_t bdd_union = 0;
  
  uintmax_t bdd0_i = 0;
  uintmax_t bdd1_i = 0;
  
  while(bdd0_i < BM->Support[bdd0].nLength && bdd1_i < BM->Support[bdd1].nLength) {
    if(BM->Support[bdd0].pList[bdd0_i] == BM->Support[bdd1].pList[bdd1_i]) {
      if(BM->VarBDDMap[BM->Support[bdd0].pList[bdd0_i]].nLength > var_limit) {
	bdd_union++;
      }
      bdd0_i++; bdd1_i++;
    } else if(BM->Support[bdd0].pList[bdd0_i] <= BM->Support[bdd1].pList[bdd1_i]) {
      bdd_union++;
      bdd0_i++;
    } else {
      bdd_union++;
      bdd1_i++;
    }
  }
  bdd_union += BM->Support[bdd0].nLength - bdd0_i;
  bdd_union += BM->Support[bdd1].nLength - bdd1_i;
  return bdd_union;
}

DdNode *Cudd_bddRestrictLimit(DdManager *manager, DdNode *f, DdNode *c, unsigned int limit) {
  DdNode *res;
  unsigned int saveLimit = manager->maxLive;
  
  manager->maxLive = (manager->keys - manager->dead) +
    (manager->keysZ - manager->deadZ) + limit;
  do {
    manager->reordered = 0;
    res = Cudd_bddRestrict(manager, f, c);
  } while (manager->reordered == 1);
  manager->maxLive = saveLimit;

  return(res);
}

DdNode *Cudd_bddNPAndLimit(DdManager *manager, DdNode *f, DdNode *c, unsigned int limit) {
  DdNode *res;
  unsigned int saveLimit = manager->maxLive;
  
  manager->maxLive = (manager->keys - manager->dead) +
    (manager->keysZ - manager->deadZ) + limit;
  do {
    manager->reordered = 0;
    res = Cudd_bddNPAnd(manager, f, c);
  } while (manager->reordered == 1);
  manager->maxLive = saveLimit;

  return(res);
}

DdNode *Cudd_bddLICompactionLimit(DdManager *manager, DdNode *f, DdNode *c, unsigned int limit) {
  DdNode *res;
  unsigned int saveLimit = manager->maxLive;
  
  manager->maxLive = (manager->keys - manager->dead) +
    (manager->keysZ - manager->deadZ) + limit;
  do {
    manager->reordered = 0;
    res = Cudd_bddLICompaction(manager, f, c);
  } while (manager->reordered == 1);
  manager->maxLive = saveLimit;

  return(res);
}

//Super slow for large lists.
DdNode *quadratic_bdd(BDDManager *BM, uintmax_t_list vars) {
  //xor(and(c1, x1), and(c2, x2), and(c3, x3), and(c4, x4), cT)

  intmax_t i;
  DdNode *quadratic = Cudd_bddIthVar(BM->dd, BM->nHighestVarInABDD+1);
  Cudd_Ref(quadratic);
  for(i = vars.nLength-1; i >= 0; i--) {
    DdNode *conjunct = Cudd_bddAnd(BM->dd, Cudd_bddIthVar(BM->dd, BM->nHighestVarInABDD+1+i), Cudd_bddIthVar(BM->dd, vars.pList[i]));
    Cudd_Ref(conjunct);

    DdNode *bTemp = Cudd_bddXor(BM->dd, quadratic, conjunct);
    Cudd_Ref(bTemp);
    Cudd_IterDerefBdd(BM->dd, conjunct);
    Cudd_IterDerefBdd(BM->dd, quadratic);
    quadratic = bTemp;
  }

  Cudd_Deref(quadratic);
  return quadratic;
}

//Assuming vars is sorted based on the current bdd variable ordering
DdNode *_find_all_linear_factors(DdManager *dd, DdNode *f, DdNode *vars, uintmax_t outvar) {
  DdNode *one, *F, *T, *E, *r, *e, *bTemp, *vars_next, *ret;

  one = DD_ONE(dd);
  
  if(f == one) return Cudd_Not(Cudd_bddIthVar(dd, outvar));
  if(f == Cudd_Not(one)) return Cudd_Not(one);

  assert(!Cudd_IsConstant(f));

  F = Cudd_Regular(f);
  T = cuddT(F); E = cuddE(F);
  if(Cudd_IsComplement(f)) {
    T = Cudd_Not(T); E = Cudd_Not(E);
  }

  //  if(T == one && E != Cudd_Not(one)) return one; //SEAN!!! Correct???
  //  if(E == one && T != Cudd_Not(one)) return one; //SEAN!!! Correct???

  ret = cuddCacheLookup2(dd, (DdNode* (*)(DdManager *, DdNode *, DdNode *))_find_all_linear_factors, f, vars);
  if(ret != NULL) {
    return ret;
  }

  assert(vars->index == F->index);

  uintmax_t back_length = 0;
  vars_next = cuddT(vars);
  while(vars_next->index != Cudd_Regular(T)->index) {
    back_length++;
    vars_next = cuddT(vars_next);
  }

  r = _find_all_linear_factors(dd, T, vars_next, outvar);

  if(r == NULL) return NULL; //reorder or gc happened
  assert(r != one);
  Cudd_Ref(r);

  if(r != Cudd_Not(one)) {
    vars_next = cuddT(vars);
    uintmax_t i;
    for(i = 0; i < back_length; i++) {
      bTemp = Cudd_bddOr(dd, r, Cudd_bddIthVar(dd, vars_next->index));
      Cudd_Ref(bTemp);
      Cudd_IterDerefBdd(dd, r);
      r = bTemp;
      vars_next = cuddT(vars_next);
    }
  }

  //if(T == Cudd_Not(E)) { //optimization
  //SEAN!!! There is some optimization I could make here. Just hard to figure out what it should be.
  //}

  back_length = 0;
  vars_next = cuddT(vars);
  while(vars_next->index != Cudd_Regular(E)->index) {
    back_length++;
    vars_next = cuddT(vars_next);
  }

  e = _find_all_linear_factors(dd, E, vars_next, outvar);
  if(e == NULL) return NULL; //reorder or gc happened
  assert(e != one);
  Cudd_Ref(e);

  if(e != Cudd_Not(one)) {
    vars_next = cuddT(vars);
    uintmax_t i;
    for(i = 0; i < back_length; i++) {
      bTemp = Cudd_bddOr(dd, e, Cudd_bddIthVar(dd, vars_next->index));
      Cudd_Ref(bTemp);
      Cudd_IterDerefBdd(dd, e);
      e = bTemp;
      vars_next = cuddT(vars_next);
    }
  }

  DdNode *rt, *re, *et, *ee, *reet, *rtee;

  re = Cudd_Cofactor(dd, r, Cudd_Not(Cudd_bddIthVar(dd, outvar)));
  Cudd_Ref(re);
  assert((r == Cudd_Not(one)) || (re != r));
  et = Cudd_Cofactor(dd, e, Cudd_bddIthVar(dd, outvar));
  Cudd_Ref(et);
  assert((e == Cudd_Not(one)) || (et != e));

  reet = Cudd_bddOr(dd, re, et);
  if(reet == NULL) {
    Cudd_IterDerefBdd(dd, r);  
    Cudd_IterDerefBdd(dd, e);
    Cudd_IterDerefBdd(dd, et);
    return NULL;
  }
  Cudd_Ref(reet);
  Cudd_IterDerefBdd(dd, re);
  Cudd_IterDerefBdd(dd, et);

  rt = Cudd_Cofactor(dd, r, Cudd_bddIthVar(dd, outvar));
  if(rt == NULL) {
    Cudd_IterDerefBdd(dd, r);  
    Cudd_IterDerefBdd(dd, e);
    Cudd_IterDerefBdd(dd, reet);
    return NULL;
  }
  Cudd_Ref(rt);

  ee = Cudd_Cofactor(dd, e, Cudd_Not(Cudd_bddIthVar(dd, outvar)));
  if(ee == NULL) {
    Cudd_IterDerefBdd(dd, r);  
    Cudd_IterDerefBdd(dd, e);
    Cudd_IterDerefBdd(dd, rt);
    Cudd_IterDerefBdd(dd, reet);
    return NULL;
  }
  Cudd_Ref(ee);  

  bTemp = Cudd_bddOr(dd, r, e);
  if(bTemp == NULL) {
    Cudd_IterDerefBdd(dd, r);  
    Cudd_IterDerefBdd(dd, e);
    Cudd_IterDerefBdd(dd, rt);
    Cudd_IterDerefBdd(dd, ee);
    Cudd_IterDerefBdd(dd, reet);
    return NULL;
  }
  Cudd_Ref(bTemp);
  Cudd_IterDerefBdd(dd, r);  
  Cudd_IterDerefBdd(dd, e);
  e = bTemp;

  rtee = Cudd_bddOr(dd, rt, ee);
  if(rtee == NULL) {
    Cudd_IterDerefBdd(dd, rt);
    Cudd_IterDerefBdd(dd, ee);
    Cudd_IterDerefBdd(dd, reet);
    Cudd_IterDerefBdd(dd, e);
    return NULL;
  }
  Cudd_Ref(rtee);
  Cudd_IterDerefBdd(dd, rt);
  Cudd_IterDerefBdd(dd, ee);

  r = Cudd_bddIte(dd, Cudd_bddIthVar(dd, outvar), reet, rtee);
  if(r == NULL) {
    Cudd_IterDerefBdd(dd, reet);
    Cudd_IterDerefBdd(dd, rtee);
    Cudd_IterDerefBdd(dd, e);
    return NULL;
  }
  Cudd_Ref(r);
  Cudd_IterDerefBdd(dd, reet);
  Cudd_IterDerefBdd(dd, rtee);

  ret = Cudd_bddIte(dd, Cudd_bddIthVar(dd, F->index), r, e);
  if(ret == NULL) {
    Cudd_IterDerefBdd(dd, r);
    Cudd_IterDerefBdd(dd, e);
    return NULL;
  }
  Cudd_Ref(ret);
  Cudd_IterDerefBdd(dd, r);
  Cudd_IterDerefBdd(dd, e);

  cuddCacheInsert2(dd, (DdNode* (*)(DdManager *, DdNode *, DdNode *))_find_all_linear_factors, f, vars, ret);

  Cudd_Deref(ret);

  return ret;
}

DdNode *find_all_linear_factors(DdManager *dd, DdNode *f, intmax_t_list *support, uintmax_t outvar) {
  if(Cudd_IsConstant(f)) return DD_ONE(dd);

  DdNode *conj = build_and_BDD(dd, support->pList, support->nLength);
  Cudd_Ref(conj);
  
  Cudd_AutodynDisable(dd); //NECESSARY!

  DdNode *factors;
  do {
    assert(Cudd_Regular(f)->index == conj->index);
    factors = _find_all_linear_factors(dd, f, conj, outvar);
  } while (factors == NULL);
  Cudd_Ref(factors);
  Cudd_IterDerefBdd(dd, conj);

  Cudd_AutodynEnable(dd, CUDD_REORDER_SAME);

  if(factors != DD_ONE(dd)) {
    //Remove the True factor

    conj = DD_ONE(dd);
    cuddRef(conj);
    for(uintmax_t i = 0; i < support->nLength; i++) {
      DdNode *bTemp = Cudd_bddAnd(dd, Cudd_Not(Cudd_bddIthVar(dd, support->pList[i])), conj);
      Cudd_Ref(bTemp);
      Cudd_IterDerefBdd(dd, conj);
      conj = bTemp;
    }
    
    DdNode *conj_ext = Cudd_bddAnd(dd, Cudd_bddIthVar(dd, outvar), conj);
    Cudd_Ref(conj_ext);
    Cudd_IterDerefBdd(dd, conj);
    
    DdNode *bTemp = Cudd_bddOr(dd, factors, conj_ext);
    Cudd_Ref(bTemp);
    assert(factors != bTemp);
    Cudd_IterDerefBdd(dd, factors);
    Cudd_IterDerefBdd(dd, conj_ext);
    factors = bTemp;
  }  

  Cudd_Deref(factors);
  return factors;  
}

DdNode *create_first_factor(DdManager *dd, DdNode *lin_factors, uintmax_t outvar) {
  Cudd_AutodynDisable(dd);
  
  DdNode *factor = Cudd_Not(DD_ONE(dd));
  Cudd_Ref(factor);

  DdNode *LF = Cudd_Regular(lin_factors);
  uint8_t neg = LF != lin_factors;
  while(!Cudd_IsConstant(LF)) {
    if(LF->index == outvar) { //LS is the 'T' coefficient, try to choose 'else' case first
      if(Cudd_IsConstant(cuddE(LF))) { //Go away from constant
	if(Cudd_IsConstant(cuddT(LF))) { //Both constant, choose the False node
	  if(cuddE(LF) == Cudd_NotCond(DD_ONE(dd), neg==0)) {
	    //Choose the 'else' case first
	    LF = cuddE(LF);
	    neg = neg ^ Cudd_IsComplement(LF); //Not really necessary since we are at the bottom of the BDD
	    LF = Cudd_Regular(LF);
	  } else {
	    //Choose the 'then' case - this is the last possible factor because 'T' is a factor of everything
	    assert(cuddT(LF) == Cudd_NotCond(DD_ONE(dd), neg==0));
	    factor = Cudd_Not(factor); //XOR the factor w/ 'T'
	    LF = cuddT(LF);
	  }
	} else {
	  //'else' is constant. 'then' is not constant. Choose 'then'
	  factor = Cudd_Not(factor); //XOR the factor w/ 'T'
	  LF = cuddT(LF);
	}
      } else {
	//'else' is not constant, choose 'else'
	LF = cuddE(LF);
	neg = neg ^ Cudd_IsComplement(LF);
	LF = Cudd_Regular(LF);
      }
    } else {
      //LF is not the 'T' coefficient, try to choose 'then' case first
      if(Cudd_IsConstant(cuddT(LF))) { //'then' case is constant, choose the 'else' case
	if(Cudd_IsConstant(cuddE(LF))) { //Both constant, choose the False node
	  if(cuddT(LF) == Cudd_NotCond(DD_ONE(dd), neg==0)) {
	    //Choose the 'then' case first
	    //XOR the factor w/ LF->index;
	    DdNode *bTemp = Cudd_bddXor(dd, factor, Cudd_bddIthVar(dd, LF->index));
	    Cudd_Ref(bTemp); Cudd_IterDerefBdd(dd, factor); factor = bTemp;
	    LF = cuddT(LF);
	  } else {
	    //Choose the 'else' case last
	    assert(cuddE(LF) == Cudd_NotCond(DD_ONE(dd), neg==0));
	    LF = cuddE(LF);
	    neg = neg ^ Cudd_IsComplement(LF); //Not really necessary since we are at the bottom of the BDD
	    LF = Cudd_Regular(LF);
	  }
	} else {
	  //'then' is constant. 'else' is not constant. Choose 'else'
	  LF = cuddE(LF);
	  neg = neg ^ Cudd_IsComplement(LF); //Not really necessary since we are at the bottom of the BDD
	  LF = Cudd_Regular(LF);
	}
      } else {
	//'then' is not constant. Choose 'then'
	//XOR the factor w/ LF->index;
	DdNode *bTemp = Cudd_bddXor(dd, factor, Cudd_bddIthVar(dd, LF->index));
	Cudd_Ref(bTemp); Cudd_IterDerefBdd(dd, factor); factor = bTemp;
	LF = cuddT(LF);
      }
    }
  }

    /*
    if(!Cudd_IsConstant(cuddE(LF))) {
      LF = cuddE(LF);
      neg = neg ^ Cudd_IsComplement(LF);
      LF = Cudd_Regular(LF);
    } else {
      if(LF->index == outvar) {
	if(cuddT(LF) == Cudd_NotCond(DD_ONE(dd), neg==0)) {
	  factor = Cudd_Not(factor);
	}
      } else {
	//Var is in the factor
	DdNode *bTemp = Cudd_bddXor(dd, factor, Cudd_bddIthVar(dd, LF->index));
	Cudd_Ref(bTemp); Cudd_IterDerefBdd(dd, factor); factor = bTemp;
      }
      
      LF = cuddT(LF);
    }
    */

  //}

  Cudd_AutodynEnable(dd, CUDD_REORDER_SAME);
  
  cuddDeref(factor);
  return factor;
}

DdNode *better_gcf(BDDManager *BM, DdNode *f, DdNode *c) {
  uintmax_t length;
  BDD_Support2(&length, &(BM->uTempList.nLength_max), &(BM->uTempList.pList), c, f);

  Cudd_AutodynDisable(BM->dd);

  for(uintmax_t i = 0 ; i < length; i++) {
    uint32_t_list_add(&BM->uTempList32, BM->uTempList.pList[i], BM->dd->perm[(length-1)-i]);
  }
  
  DdNode *drop_c = Cudd_bddPermute(BM->dd, c, (int *)BM->uTempList32.pList);
  Cudd_Ref(drop_c);

  DdNode *drop_f = Cudd_bddPermute(BM->dd, f, (int *)BM->uTempList32.pList);
  Cudd_Ref(drop_f);

  DdNode *drop_res = Cudd_bddConstrain(BM->dd, drop_f, drop_c);
  Cudd_Ref(drop_res);

  for(uintmax_t i = 0 ; i < length; i++) {
    uint32_t_list_add(&BM->uTempList32, BM->dd->perm[(length-1)-i], BM->uTempList.pList[i]);
  }

  Cudd_AutodynEnable(BM->dd, CUDD_REORDER_SAME);

  DdNode *res = Cudd_bddPermute(BM->dd, drop_res, (int *)BM->uTempList32.pList);
  Cudd_Ref(res);
  
  Cudd_IterDerefBdd(BM->dd, drop_res);
  Cudd_IterDerefBdd(BM->dd, drop_f);
  Cudd_IterDerefBdd(BM->dd, drop_c);
  
  Cudd_Deref(res);
  return res;
}

DdNode *XORXORsRecur(DdManager * dd, DdNode * f, DdNode * g) {
  DdNode *fv, *G, *gv;
  DdNode *one, *zero, *r, *t, *e;
  unsigned int topf, topg, index;
  
  one = DD_ONE(dd);
  zero = Cudd_Not(one);
  
  /* Terminal cases. */
  if (f == g) return(zero);
  if (f == Cudd_Not(g)) return(one);
  if (f > g) { /* Try to increase cache efficiency and simplify tests. */
    DdNode *tmp = f;
    f = g;
    g = tmp;
  }
  if (g == zero) return(f);
  if (g == one) return(Cudd_Not(f));
  if (Cudd_IsComplement(f)) {
    f = Cudd_Not(f);
    g = Cudd_Not(g);
  }
  /* Now the first argument is regular. */
  if (f == one) return(Cudd_Not(g));
  
  /* At this point f and g are not constant. */
  
  /* Check cache. */
  //r = cuddCacheLookup2(dd, XORXORsRecur, f, g);
  //if (r != NULL) return(r);
  
  /* Here we can skip the use of cuddI, because the operands are known
  ** to be non-constant.
  */
  topf = dd->perm[f->index];
  G = Cudd_Regular(g);
  topg = dd->perm[G->index];
  
  /* Compute cofactors. */
  if (topf <= topg) {
    index = f->index;
    fv = cuddT(f);
  } else {
    index = G->index;
    fv = f;
  }
  
  if (topg <= topf) {
    gv = cuddT(G);
    if (Cudd_IsComplement(g)) {
      gv = Cudd_Not(gv);
    }
  } else {
    gv = g;
  }
  
  t = XORXORsRecur(dd, fv, gv);
  if (t == NULL) return(NULL);
  cuddRef(t);
  
  if(topf == topg) {
    r = t;
  } else {
    e = Cudd_Not(t);
    cuddRef(e);
  
    if (Cudd_IsComplement(t)) {
      r = cuddUniqueInter(dd,(int)index,Cudd_Not(t),Cudd_Not(e));
      if (r == NULL) {
	Cudd_IterDerefBdd(dd, t);
	Cudd_IterDerefBdd(dd, e);
	return(NULL);
      }
      r = Cudd_Not(r);
    } else {
      r = cuddUniqueInter(dd,(int)index,t,e);
      if (r == NULL) {
	Cudd_IterDerefBdd(dd, t);
	Cudd_IterDerefBdd(dd, e);
	return(NULL);
      }
    }

    cuddDeref(e);
  }
  cuddDeref(t);

  //cuddCacheInsert2(dd, XORXORsRecur, f, g, r);
  return(r);
}

DdNode *XORXORs(DdManager * dd, DdNode * f, DdNode * g) {
  Cudd_AutodynDisable(dd);  
  DdNode *res = XORXORsRecur(dd,f,g);
  Cudd_AutodynEnable(dd, CUDD_REORDER_SAME);
  return res;
}

DdNode *safe_assign0(DdManager *dd, DdNode *f, DdNode *v) {
  DdNode *one, *res, *F, *Fv, *Fnv;
  
  one = DD_ONE(dd);
  if(Cudd_IsConstant(f)) return one;

  /* Check cache. */
  res = cuddCacheLookup2(dd, safe_assign0, f, v);
  if (res != NULL) return res;

  F = Cudd_Regular(f);
  unsigned int topf = dd->perm[F->index];
  unsigned int topv = dd->perm[v->index];

  if(topf == topv) {
    Fv = cuddT(F); Fnv = cuddE(F);
    if(Cudd_IsComplement(f)) {
      Fv = Cudd_Not(Fv);
      Fnv = Cudd_Not(Fnv);
    }

    //maybe leq here
    DdNode *r = Cudd_bddIteConstant(dd, Fv, Cudd_Not(Fnv), Cudd_Not(one));
    DdNode *e = Cudd_bddIteConstant(dd, Cudd_Not(Fv), Fnv, Cudd_Not(one));

    if(r == Cudd_Not(one)) {
      if(e == Cudd_Not(one)) {
	assert(0);
	res = one;
      } else {
	res = Cudd_Not(v);
      }
    } else if(e == Cudd_Not(one)) {
      res = v;
    } else {
      res = Cudd_Not(one);
    }    

    cuddCacheInsert2(dd, safe_assign0, f, v, res);
    return res;
  }

  Fv = cuddT(F); Fnv = cuddE(F);
  if(Cudd_IsComplement(f)) {
    Fv = Cudd_Not(Fv);
    Fnv = Cudd_Not(Fnv);
  }

  DdNode *r = safe_assign0(dd, Fv, v);
  if(r == Cudd_Not(one)) res = Cudd_Not(one);
  else res = Cudd_bddAnd(dd, r, safe_assign0(dd, Fnv, v));

  cuddCacheInsert2(dd, safe_assign0, f, v, res);
  return res;
}

DdNode *set_variable(DdManager *dd, DdNode *pCurrentBDD, uintmax_t var, uint8_t polarity) {
  DdNode *bTemp = Cudd_Cofactor(dd, pCurrentBDD, Cudd_NotCond(Cudd_bddIthVar(dd, var), 1-polarity));
  if(bTemp == NULL) {
    assert(0);
  }
  return bTemp;
}

mpz_t *_count_true_paths(DdManager *dd, DdNode *f, uintmax_t_list *support, uintmax_t level, st_table *table) {

  mpz_t *ret; //check the hash table
  ret = (mpz_t *)malloc(sizeof(mpz_t));
  mpz_init(*ret);
  
  if(f == DD_ONE(dd)) {
    mpz_setbit(*ret, support->nLength - level);
    return ret;
  }
  if(f == Cudd_Not(DD_ONE(dd))) {
    return ret;
  }
  
  DdNode *F = Cudd_Regular(f);  
  DdNode *T, *E;

  uintmax_t misses = level;
  while(F->index != support->pList[level]) level++;
  assert(level < support->nLength);

  mpz_t *dummy;
  if(st_lookup(table, f, &dummy)) {
    mpz_set(*ret, *dummy);
    mpz_mul_2exp(*ret, *ret, level-misses);
    return ret;
  }

  T = cuddT(F); E = cuddE(F);
  if(Cudd_IsComplement(f)) {
    T = Cudd_Not(T); E = Cudd_Not(E);
  }

  mpz_t *r = _count_true_paths(dd, T, support, level+1, table);
  mpz_t *e = _count_true_paths(dd, E, support, level+1, table);
  mpz_add(*ret, *r, *e);
  mpz_clear(*r); mpz_clear(*e);
  free(r); free(e);

  //Store result to hash table
  mpz_t *hash_value = (mpz_t *)malloc(sizeof(mpz_t));
  mpz_init_set(*hash_value, *ret);
  if(st_add_direct(table, (char *)f, (char *)hash_value) == ST_OUT_OF_MEM) {
    mpz_clear(*hash_value);
    free(hash_value);
  }

  mpz_mul_2exp(*ret, *ret, level-misses);

  /*
  printBDD(dd, f);
  char *count = mpz_get_str(NULL, 10, *ret);
  fprintf(stdout, "Number of paths to true is %s (%ju %ju %ju)\n", count, support->nLength, level, misses);
  free(count);
  */

  return ret;
}

enum st_retval count_true_paths_Stfree(char * key, char * value, char * arg) {
  mpz_t *d;
  d = (mpz_t *)value;
  mpz_clear(*d);
  free(d);
  return(ST_CONTINUE);
}

mpz_t *count_true_paths(BDDManager *BM, DdNode *f, uintmax_t_list *support) {
  uintmax_t_list_copy(&BM->uTempList, support);
  qsort(BM->uTempList.pList, BM->uTempList.nLength, sizeof(uintmax_t), bddperm_compfunc);

  st_table *table;
  table = st_init_table(st_ptrcmp,st_ptrhash);
  if (table == NULL) {
    return NULL;
  }
  
  mpz_t *num =_count_true_paths(BM->dd, f, &BM->uTempList, 0, table);
  ddClearFlag(Cudd_Regular(f));
  
  st_foreach(table, count_true_paths_Stfree, NULL); //Free the mpz_t objects left in the CUDD hash table
  st_free_table(table);

  return num;
}

mpz_t *count_BDD_solutions(BDDManager *BM) {

  //First, reset command line flags that affect clustering
  int old_pairwise_bdd_reduction = pairwise_bdd_reduction;
  int old_bdd_cluster_limit = bdd_cluster_limit;
  uint8_t old_apply_BDD_inferences = apply_BDD_inferences;
  uint8_t old_apply_BDD_equivalences = apply_BDD_equivalences;
  pairwise_bdd_reduction = PAIRWISE_BDD_REDUCTION_NONE;
  bdd_cluster_limit = -1;
  apply_BDD_inferences = 0;
  apply_BDD_equivalences = 0;

  //Cluster until each variable occurs in only one BDD.
  uint8_t ret = VE(BM, CHeuristics[5], -1);
  if((ret != NO_ERROR) && (ret != SAT) && (ret != CLUSTER_LIMIT)) {
    if(ret == UNSAT) {
      mpz_t *count = (mpz_t *)malloc(sizeof(mpz_t));
      mpz_init(*count);
      return count; //zero count
    } else {
      //fprintf(stderr, "||%ju||\n", ret);
      return NULL;
    }
  }

  //find_all_early_quantification_vars(BM); //just in case
  //perform_early_quantification(BM);

  //Turn the command line flags back to user preferences
  pairwise_bdd_reduction = old_pairwise_bdd_reduction;
  bdd_cluster_limit = old_bdd_cluster_limit;
  apply_BDD_inferences = old_apply_BDD_inferences;
  apply_BDD_equivalences = old_apply_BDD_equivalences;

  //Calculate the number of missing 'protected' variables
  uintmax_t missing_vars_count = 0;
  DdNode *orig_bdd = NULL;
  if(BM->ExQuantProtectVars && early_quantify_BDD_vars) {
    if(apply_BDD_equivalences) {
      fprintf(stderr, "Warning: Equivalence reasoning can cause an incorrect number of solutions to be produced\n");
    }
    
    for(uintmax_t i = 1; i < BM->ExQuantProtectedVars.nLength; i++) {
      if(BM->ExQuantProtectedVars.pList[i] == 1 && BM->VarBDDMap[i].nLength == 0) {
	//Variable is protected and has been removed somehow
	intmax_t equiv = get_equiv(BM->EQManager, i);
	if(equiv == i) {
	  //Has not been set and is missing, double the solution count
	  missing_vars_count++;
	} else if(equiv == BM->EQManager->True) {
	  //Do nothing
	} else if(equiv == BM->EQManager->False) {
	  //Do nothing
	} else {
	  //Variable has been set equal to some other variable
	  if(BM->ExQuantProtectedVars.pList[imaxabs(equiv)] == 1) {
	    //Variable is equal to some other protected variable
	    //What to do?
	  } else {
	    //Variable is equal to a not protected variable
	    //???
	  }
	}
      } else if(BM->ExQuantProtectedVars.pList[i] == 0) {
	assert(BM->VarBDDMap[i].nLength == 0);
      }
    }
  }

  //fprintf(stderr, "||%ju||", missing_vars_count);

  //2^missing_vars_count
  mpz_t *count = (mpz_t *)malloc(sizeof(mpz_t));
  mpz_init(*count);
  mpz_setbit(*count, missing_vars_count);

  //Calculate the number of solutions of all the remaining BDDs,
  //multiplying the counts together
  for(uintmax_t i = 0; i < BM->nNumBDDs; i++) {
    for(uintmax_t j = 0; j < BM->Support[i].nLength; j++) {
      //Assert that every variable remaining occurs in only one BDD
      assert(BM->VarBDDMap[BM->Support[i].pList[j]].nLength == 1);
    }

    mpz_t *count_i = count_true_paths(BM, BM->BDDList[i], &BM->Support[i]);
    mpz_mul(*count, *count, *count_i);
    mpz_clear(*count_i);
    free(count_i);
  }
  
  return count;
}
