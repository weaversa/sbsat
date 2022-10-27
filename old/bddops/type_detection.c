/*
 *    Copyright (C) 2010 Sean Weaver
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

uintmax_t functionTypeLimits[FN_TYPE_MAX];
int functionTypeLimits_all;
uint8_t ignore_limits = 0;

uintmax_t isOR(DdManager *dd, DdNode *bdd) {
  uint8_t neg = Cudd_IsComplement(bdd);
  bdd = Cudd_Regular(bdd);
  if (cuddIsConstant(bdd)) return 0;
  uintmax_t size = 0;
  while(bdd != Cudd_NotCond(DD_ONE(dd), neg==0)) {
    if(cuddT(bdd) == Cudd_NotCond(DD_ONE(dd), neg==1)) {
      bdd = cuddE(bdd);
      neg = neg ^ Cudd_IsComplement(bdd);
      bdd = Cudd_Regular(bdd);
    } else if(Cudd_Regular(cuddE(bdd)) == Cudd_NotCond(DD_ONE(dd), neg ^ Cudd_IsComplement(cuddE(bdd)))) {
      bdd = cuddT(bdd);
    } else return 0;
    size++;
  }
  return size;
}

uintmax_t isAND(DdManager *dd, DdNode *bdd) {
  uint8_t neg = Cudd_IsComplement(bdd);
  bdd = Cudd_Regular(bdd);
  if (cuddIsConstant(bdd)) return 0;
  uintmax_t size = 0;
  while(bdd != Cudd_NotCond(DD_ONE(dd), neg==1)) {
    if(cuddT(bdd) == Cudd_NotCond(DD_ONE(dd), neg==0)) {
      bdd = cuddE(bdd);
      neg = neg ^ Cudd_IsComplement(bdd);
      bdd = Cudd_Regular(bdd);
    } else if(Cudd_Regular(cuddE(bdd)) == Cudd_NotCond(DD_ONE(dd), neg == Cudd_IsComplement(cuddE(bdd)))) {
      bdd = cuddT(bdd);
    } else return 0;
    size++;
  }
  return size;
}

uintmax_t isXOR(DdManager *dd, DdNode *bdd) {
  bdd = Cudd_Regular(bdd);
  if(cuddIsConstant(bdd)) return 0;
  uintmax_t size = 0;
  for(; !cuddIsConstant(bdd); bdd = cuddT(bdd)) {
    if(cuddT(bdd) != Cudd_Not(cuddE(bdd))) return 0;
    size++;
  }
  return size;
}

//returns True for NO
//returns positive equality variable for AND_EQU
//returns negative equality variable for OR_EQU
DdNode *isAND_EQU(DdManager *dd, DdNode *bdd, uintmax_t_list *bdd_vars) {
  if(cuddIsConstant(Cudd_Regular(bdd))) return NULL;
  if(bdd_vars->nLength < 3) return NULL;
  uintmax_t nNumNodes = Cudd_DagSize(bdd);
  
  DdNode *one = DD_ONE(dd);
  
  if(nNumNodes <= bdd_vars->nLength+2) {
    assert(nNumNodes >= bdd_vars->nLength+1);
    for(uintmax_t x = 0; x < bdd_vars->nLength; x++) {
      DdNode *equ_var = Cudd_bddIthVar(dd, bdd_vars->pList[x]);
      if(isXDependent(dd, bdd, equ_var) == one) {
	DdNode *t_equ_bdd = Cudd_Cofactor(dd, bdd, equ_var); cuddRef(t_equ_bdd);
	if(isAND(dd, t_equ_bdd)) {
	  Cudd_IterDerefBdd(dd, t_equ_bdd);
	  return equ_var;
	} else if(isOR(dd, t_equ_bdd)) {
	  Cudd_IterDerefBdd(dd, t_equ_bdd);
	  return Cudd_Not(equ_var);
	}
	Cudd_IterDerefBdd(dd, t_equ_bdd);
      }
    }
  }

  //This BDD is not an AND/OR Equals BDD
  return DD_ONE(dd);
}

uintmax_t getMAX(DdManager* dd, DdNode *bdd) {
  uintmax_t max = -1;
  bdd = Cudd_Regular(bdd);
  do {
    bdd = cuddT(bdd);
    max++;
  } while (!cuddIsConstant(bdd));
  
  return max;
}

uintmax_t getMIN(DdManager *dd, DdNode *bdd, uintmax_t bdd_len) {
  DdNode *one;
  uintmax_t min = -1;

  uint8_t neg = Cudd_IsComplement(bdd);
  bdd = Cudd_Regular(bdd);
  one = DD_ONE(dd);

  do {
    bdd = cuddE(bdd);
    neg = neg ^ Cudd_IsComplement(bdd);
    bdd = Cudd_Regular(bdd);
    min++;
  } while (!cuddIsConstant(bdd));
  if(bdd == Cudd_NotCond(one, neg)) {
     min = 0;
  } else {
    min = bdd_len - min;
  }

  return min;
}

uint8_t isMINMAX(DdManager* dd, DdNode *bdd, uintmax_t_list *bdd_vars) {
  DdNode *one, *zero, *tmp_bdd;
  uintmax_t max, min;

  tmp_bdd = Cudd_Regular(bdd);
  if(cuddIsConstant(tmp_bdd)) return 0;
  if(tmp_bdd == bdd) return 0;
  
  one = DD_ONE(dd);
  zero = Cudd_Not(one);
  
  uint8_t neg = Cudd_IsComplement(bdd);
  max = -1;
  do {
    tmp_bdd = cuddT(tmp_bdd);
    max++;
  } while (!cuddIsConstant(tmp_bdd));
  assert(tmp_bdd == one);
  
  tmp_bdd = Cudd_Regular(bdd);
  min = -1;
  do {
    tmp_bdd = cuddE(tmp_bdd);
    neg = neg ^ Cudd_IsComplement(tmp_bdd);
    tmp_bdd = Cudd_Regular(tmp_bdd);
    min++;
  } while (!cuddIsConstant(tmp_bdd));
  if(tmp_bdd == Cudd_NotCond(one, neg)) min = 0;
  else if(tmp_bdd != Cudd_NotCond(zero, neg)) return 0;
  else min = bdd_vars->nLength - min;
  
  if(min > max) return 0;
  
  //Now have the min and max
  
  //A slow way to do it!
  //fprintf(stderr, "min=%d, max=%d, len=%d\n", min, max, bdd_vars->nLength);
  
  tmp_bdd = build_cardinality_BDD(dd, min, max, bdd_vars->pList, bdd_vars->nLength); cuddRef(tmp_bdd);

  if(bdd == tmp_bdd) {
    Cudd_IterDerefBdd(dd, tmp_bdd);
    return 1;
  } else {
    Cudd_IterDerefBdd(dd, tmp_bdd);
    return 0;
  }
}

uint8_t isNEGMINMAX(DdManager *dd, DdNode *bdd, uintmax_t_list *bdd_vars) {
  //SEAN!!! Could combine isMINMAX and isNEGMINMAX
  return isMINMAX(dd, Cudd_Not(bdd), bdd_vars);
}

//returns False for NO
//returns True for YES
DdNode *isXDependent(DdManager *dd, DdNode *bdd, DdNode *X) {
  DdNode *cache_res;
  
  bdd = Cudd_Regular(bdd);
  if(cuddIsConstant(bdd)) return Cudd_Not(DD_ONE(dd));
  
  uintmax_t top_bdd = dd->perm[bdd->index];
  uintmax_t top_X = dd->perm[X->index];
  if(top_bdd > top_X) return Cudd_Not(DD_ONE(dd));
  
  if(top_bdd == top_X) {
    if(Cudd_Regular(cuddE(bdd)) == Cudd_Regular(cuddT(bdd)))
      return DD_ONE(dd);
  }
  
  cache_res = cuddCacheLookup2(dd, isXDependent, bdd, X);
  if(cache_res!=NULL) {
    return cache_res;
  }
  
  if(isXDependent(dd, cuddT(bdd), X) == Cudd_Not(DD_ONE(dd))) {
    cuddCacheInsert2(dd, isXDependent, bdd, X, Cudd_Not(DD_ONE(dd)));
    return Cudd_Not(DD_ONE(dd));
  }
  
  if(isXDependent(dd, cuddE(bdd), X) == Cudd_Not(DD_ONE(dd))) {
    cuddCacheInsert2(dd, isXDependent, bdd, X, Cudd_Not(DD_ONE(dd)));
    return Cudd_Not(DD_ONE(dd));
  }
  
  cuddCacheInsert2(dd, isXDependent, bdd, X, DD_ONE(dd));
  return DD_ONE(dd);
}

//returns True for NO
//returns a dependent variable otherwise
DdNode *find_dep_var(DdManager *dd, DdNode *bdd, uintmax_t_list *bdd_vars) {
  DdNode *equ_var, *one;
  
  one = DD_ONE(dd);
  for(uintmax_t x = 0; x < bdd_vars->nLength; x++) {
    equ_var = Cudd_bddIthVar(dd, bdd_vars->pList[x]);
    if(isXDependent(dd, bdd, equ_var) == one) {
      //      if(Cudd_bddVarIsDependent(dd, bdd, equ_var)) { //This function is too agressive
      //We only want to find variables x in F of the form:
      //x = F(...)
      //         i.e.
      //         DdNode *t_equ_bdd = Cudd_Cofactor(dd, bdd, equ_var); cuddRef(t_equ_bdd);
      //         DdNode *f_equ_bdd = Cudd_Cofactor(dd, bdd, Cudd_Not(equ_var)); cuddRef(f_equ_bdd);
      //         assert(t_equ_bdd == Cudd_Not(f_equ_bdd));
      
      return equ_var;
    }
  }
  return one;
}

//returns True for NO
//returns the xor of dependent variables or the reduced bdd otherwise (depending on the ret_vars flag)
DdNode *find_all_dep_vars(DdManager *dd, DdNode *bdd, uintmax_t_list *bdd_vars, uint8_t ret_vars) {
  DdNode *equ_var, *one, *ret;

  uintmax_t nNumFound = 0;
  Cudd_Ref(bdd);

  ret = one = DD_ONE(dd);
  Cudd_Ref(ret);
  for(uintmax_t x = 0; x < bdd_vars->nLength; x++) {
    equ_var = Cudd_bddIthVar(dd, bdd_vars->pList[x]);
    if(isXDependent(dd, bdd, equ_var) == one) {
      //      if(Cudd_bddVarIsDependent(dd, bdd, equ_var)) { //This function is too agressive
      //We only want to find variables x in F of the form:
      //x = F(...)
      //         i.e.
      //         DdNode *t_equ_bdd = Cudd_Cofactor(dd, bdd, equ_var); cuddRef(t_equ_bdd);
      //         DdNode *f_equ_bdd = Cudd_Cofactor(dd, bdd, Cudd_Not(equ_var)); cuddRef(f_equ_bdd);
      //         assert(t_equ_bdd == Cudd_Not(f_equ_bdd));

      nNumFound++;
      DdNode *bTemp;
      if(ret_vars) {
	bTemp = Cudd_bddXor(dd, equ_var, ret);
	Cudd_Ref(bTemp);
	Cudd_IterDerefBdd(dd, ret);
	ret = bTemp;
      }
      bTemp = Cudd_Cofactor(dd, bdd, Cudd_Not(equ_var));
      Cudd_Ref(bTemp);
      Cudd_IterDerefBdd(dd, bdd);
      bdd = bTemp;
    }
  }

  if(ret_vars) {
    if(nNumFound > 1) {
      Cudd_Deref(ret);
      return ret;
    } else {
      Cudd_IterDerefBdd(dd, ret);
      Cudd_Ref(one);
      return one;
    }
  } else {
    Cudd_Deref(ret);
    if(nNumFound > 1) {
      Cudd_Deref(bdd);
      return bdd;
    } else {
      Cudd_IterDerefBdd(dd, bdd);
      return one;
    }
  }
}

//returns True for NO
//returns the or of disjunctive variables or the reduced bdd otherwise (depending on the ret_vars flag)
DdNode *find_all_disjunctive_vars(DdManager *dd, DdNode *bdd, uintmax_t_list *bdd_vars, uint8_t ret_vars) {
  DdNode *or_var, *one, *ret;

  uintmax_t nNumFound = 0;
  Cudd_Ref(bdd);

  ret = one = DD_ONE(dd);
  Cudd_Ref(ret);
  for(uintmax_t x = 0; x < bdd_vars->nLength; x++) {
    or_var = Cudd_bddIthVar(dd, bdd_vars->pList[x]);
    
    DdNode *t_equ_bdd = Cudd_Cofactor(dd, bdd, or_var); cuddRef(t_equ_bdd);
    DdNode *f_equ_bdd = Cudd_Cofactor(dd, bdd, Cudd_Not(or_var)); cuddRef(f_equ_bdd);

    if(t_equ_bdd == one || f_equ_bdd == one) {
      nNumFound++;
      DdNode *bTemp;
      if(ret_vars) {
	bTemp = Cudd_bddOr(dd, or_var, ret);
	Cudd_Ref(bTemp);
	Cudd_IterDerefBdd(dd, ret);
	ret = bTemp;
      }
      Cudd_IterDerefBdd(dd, bdd);
      if(t_equ_bdd == one) {
	bdd = f_equ_bdd;
      } else {
	bdd = t_equ_bdd;
      }
      Cudd_Ref(bdd);
    }
    Cudd_IterDerefBdd(dd, t_equ_bdd);
    Cudd_IterDerefBdd(dd, f_equ_bdd);
  }

  if(ret_vars) {
    Cudd_Deref(ret);
    return ret;
  } else {
    Cudd_Deref(ret);
    Cudd_Deref(bdd);
    return bdd;
  }
}

//Based on CUDD_bddConstrainDecomp
DdNode *_find_equals_factor(DdManager *dd, DdNode *f) {
  DdNode *F, *fv, *fvn;
  DdNode *fAbs;
  DdNode *result;
  DdNode *ok;
  
  if (Cudd_IsConstant(f)) return DD_ONE(dd);
  /* Compute complements of cofactors. */
  F = Cudd_Regular(f);
  fv = cuddT(F);
  fvn = cuddE(F);
  if (F == f) {
    fv = Cudd_Not(fv);
    fvn = Cudd_Not(fvn);
  }
  /* Compute abstraction of top variable. */
  fAbs = cuddBddAndRecur(dd, fv, fvn);

  if (fAbs == NULL) {
    return NULL;
  }

  cuddRef(fAbs);
  fAbs = Cudd_Not(fAbs);
  /* Recursively find the next abstraction and the components of the
  ** decomposition. */
  ok = _find_equals_factor(dd, fAbs);
  if (ok != DD_ONE(dd)) {
    cuddRef(ok);
    Cudd_IterDerefBdd(dd,fAbs);
    cuddDeref(ok);
    return ok; //ok is either NULL or of the form x = f(...)
  }
  /* Compute the component of the decomposition corresponding to the
  ** top variable and return it. */
  result = cuddBddConstrainRecur(dd, f, fAbs);
  if (result == NULL) {
    Cudd_IterDerefBdd(dd,fAbs);
    return NULL;
  }
  cuddRef(result);
  Cudd_IterDerefBdd(dd,fAbs);

  DdNode *equ_var = Cudd_bddIthVar(dd, Cudd_Regular(result)->index);
  if(isXDependent(dd, result, equ_var) == DD_ONE(dd)) {
    cuddDeref(result);
    return result;
  }
  Cudd_IterDerefBdd(dd, result);

  return DD_ONE(dd);
}

DdNode *find_equals_factor(DdManager *dd, DdNode *f) {
  DdNode *result;
  do {
    dd->reordered = 0;
    result = _find_equals_factor(dd,f);
  } while (dd->reordered == 1);
  if(result == NULL) return DD_ONE(dd);
  return result;
}

uintmax_t findandret_fnType(BDDManager *BM, DdNode *bdd) {
  
  if(cuddIsConstant(Cudd_Regular(bdd))) {
    BM->uTempList.nLength = 0;
    return FN_TYPE_UNSURE;
  }
  
  BDD_Support(&(BM->uTempList.nLength), &(BM->uTempList.nLength_max), &(BM->uTempList.pList), bdd);
  
  //if (BM->uTempList.nLength != 0) qsort (BM->uTempList.pList, BM->uTempList.nLength, sizeof(uintmax_t), compfunc); //SEAN!!! why sort this list?
  
  if((ignore_limits || BM->uTempList.nLength >= functionTypeLimits[FN_TYPE_OR]) && isOR(BM->dd, bdd)) {
    return FN_TYPE_OR;
  }
  
  if((ignore_limits || BM->uTempList.nLength >= functionTypeLimits[FN_TYPE_AND]) && isAND(BM->dd, bdd)) {
    return FN_TYPE_AND;
  }
  
  if((ignore_limits || BM->uTempList.nLength >= functionTypeLimits[FN_TYPE_XOR]) && isXOR(BM->dd, bdd)) {
    return FN_TYPE_XOR;
  }
  
  if((ignore_limits || BM->uTempList.nLength >= functionTypeLimits[FN_TYPE_MINMAX]) && isMINMAX(BM->dd, bdd, &BM->uTempList)) {
    return FN_TYPE_MINMAX;
  }
  
  if((ignore_limits || BM->uTempList.nLength >= functionTypeLimits[FN_TYPE_NEGMINMAX]) && isNEGMINMAX(BM->dd, bdd, &BM->uTempList)) {
    return FN_TYPE_NEGMINMAX;
  }

  DdNode *depvar = find_dep_var(BM->dd, bdd, &BM->uTempList);
  if(depvar != DD_ONE(BM->dd)) {
    BM->equ_var = depvar;
    return FN_TYPE_DEP_VAR;
  }

  /*
  DdNode **conjuncts = Cudd_bddConstrainDecomp(BM->dd, bdd);
  for(uintmax_t v = 0; v < BM->nNumVariables; v++) {
    if(conjuncts[v] != DD_ONE(BM->dd)) {
      DdNode *equ_var = Cudd_bddIthVar(BM->dd, v);
      if(isXDependent(BM->dd, conjuncts[v], equ_var) == DD_ONE(BM->dd)) {
	fprintf(stdout, "\n%s(%ju) equ decomp\n", s_name(v), v);
	//printBDD(BM->dd, conjuncts[v]); fprintf(stdout, "\n");
      } else {
	//fprintf(stdout, "\n%s(%ju) normal decomp\n", s_name(v), v);
	//printBDD(BM->dd, conjuncts[v]); fprintf(stdout, "\n");
      }
    }
    Cudd_IterDerefBdd(BM->dd, conjuncts[v]);
  }
  */

  if(ignore_limits || BM->uTempList.nLength >= functionTypeLimits[FN_TYPE_AND_EQU]) {
    DdNode *equ_var = isAND_EQU(BM->dd, bdd, &BM->uTempList);
    BM->equ_var = equ_var;
    if(equ_var != DD_ONE(BM->dd)) {
      if(Cudd_IsComplement(equ_var)) {
	BM->equ_var = Cudd_Not(BM->equ_var);
	return FN_TYPE_OR_EQU;
      } else {
	return FN_TYPE_AND_EQU;
      } //else - bdd is not an AND_EQU function
    }
  }
  
  return FN_TYPE_UNSURE;
}
