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

int apply_BDD_inferences = 0;
int apply_BDD_equivalences = 0;
uint8_t in_apply_inferences = 0;


uint8_t reduce_inference(Equiv_Manager_Struct *EQM, inference_item *inference) {
  uint8_t ret = NO_ERROR;

  if(inference->rgt == 0) {
    if(inference->lft > 0) {
      ret = equiv_insertEquiv(EQM, inference->lft, EQM->True);
      if(ret != EQUIV_CHANGE) {
	//Skip over inference already applied or UNSAT
	return ret;
      }	else if(EQM->lft_result < EQM->True) { //Probably unnecessary...
	inference->lft = EQM->lft_result; //I don't think EQM->lft_result will ever be T or F
      } else {
	assert(0);
      }
    } else {
      ret = equiv_insertEquiv(EQM, -inference->lft, EQM->False);
      if(ret != EQUIV_CHANGE) {
	//Skip over inference already applied or UNSAT
	return ret;
      } else if(EQM->lft_result < EQM->True) { //Probably unnecessary...
	inference->lft = -EQM->lft_result; //I don't think EQM->lft_result will ever be T or F
      } else {
	assert(0);
      }
    }
  } else { //inference->rgt != 0
    if(inference->rgt > 0) {
      ret = equiv_insertEquiv(EQM, inference->lft, inference->rgt);
      if(ret != EQUIV_CHANGE) {
	//Skip over inference already applied or UNSAT
      }	else if(EQM->lft_result == EQM->True) {
	inference->lft = EQM->rgt_result;
	inference->rgt = 0;
      } else if(EQM->rgt_result == EQM->True) {
	inference->lft = EQM->lft_result;
	inference->rgt = 0;
      } else if(EQM->lft_result == EQM->False) {
	inference->lft = -EQM->rgt_result;
	inference->rgt = 0;
      } else if(EQM->rgt_result == EQM->False) {
	inference->lft = -EQM->lft_result;
	inference->rgt = 0;
      } else if(imaxabs(EQM->lft_result) < imaxabs(EQM->rgt_result)) {
	if(EQM->lft_result < 0) {
	  inference->lft = -EQM->lft_result;
	  inference->rgt = -EQM->rgt_result;
	} else {
	  inference->lft = EQM->lft_result;
	  inference->rgt = EQM->rgt_result;
	}
      } else {
	if(EQM->rgt_result < 0) {
	  inference->lft = -EQM->rgt_result;
	  inference->rgt = -EQM->lft_result;
	} else {
	  inference->lft = EQM->rgt_result;
	  inference->rgt = EQM->lft_result;
	}
      }
    } else { //inference->rgt < 0
      ret = equiv_insertOppos(EQM, inference->lft, -inference->rgt);
      if(ret != EQUIV_CHANGE) {
	//Skip over inference already applied or UNSAT
      } else if(EQM->lft_result == EQM->True) {
	inference->lft = -EQM->rgt_result;
	inference->rgt = 0;
      } else if(EQM->rgt_result == EQM->True) {
	inference->lft = -EQM->lft_result;
	inference->rgt = 0;
      } else if(EQM->lft_result == EQM->False) {
	inference->lft = EQM->rgt_result;
	inference->rgt = 0;
      } else if(EQM->rgt_result == EQM->False) {
	inference->lft = EQM->lft_result;
	inference->rgt = 0;
      } else if(imaxabs(EQM->lft_result) < imaxabs(EQM->rgt_result)) {
	if(EQM->lft_result < 0) {
	  inference->lft = -EQM->lft_result;
	  inference->rgt = EQM->rgt_result;
	} else {
	  inference->lft = EQM->lft_result;
	  inference->rgt = -EQM->rgt_result;
	}
      } else {
	if(EQM->rgt_result < 0) {
	  inference->lft = -EQM->rgt_result;
	  inference->rgt = EQM->lft_result;
	} else {
	  inference->lft = EQM->rgt_result;
	  inference->rgt = -EQM->lft_result;
	}
      }
    }
  }

  return ret;
}

uint8_t save_inference(BDDManager *BM, inference_item *inference) {
  uint8_t ret = NO_ERROR;
  ret = reduce_inference(BM->EQManager, inference);
  if(ret == EQUIV_CHANGE) {      
    if(inference->rgt == 0) {
      if(inference->lft < 0) {
	d2_printf2("{%s=F}", s_name(-inference->lft));
      } else {
	d2_printf2("{%s=T}", s_name(inference->lft));
      }
    } else {
      if(inference->rgt < 0) {
	d2_printf3("{%s=-%s}", s_name(inference->lft), s_name(-inference->rgt));
      } else {
	d2_printf3("{%s=%s}", s_name(inference->lft), s_name(inference->rgt));
      }
      assert(inference->lft > 0);
    }
    enqueue_x(BM->InferenceQueue, (void *)inference->lft);
    enqueue_x(BM->InferenceQueue, (void *)inference->rgt);
  } else if(ret == UNSAT) {
    d2_printf1("UNSAT\n");
    return UNSAT;
  } else assert(ret == EQUIV_NO_CHANGE);

  ret = NO_ERROR;
  return ret;
}

uint8_t find_and_add_inferences(BDDManager *BM, uintmax_t bdd_loc) {
  uint8_t ret = NO_ERROR;
  
  if(apply_BDD_inferences && BM->read_input_finished) {
    
    DdManager *dd = BM->dd;
    DdNode *bdd = BM->BDDList[bdd_loc];
    
    DdNode *inference_cube = Cudd_FindEssential(dd, bdd);
    if(inference_cube == NULL) return ret;

    if(inference_cube == Cudd_ReadOne(dd)) {
      return ret;
    }

    Cudd_Ref(inference_cube);
    
    bdd = inference_cube;

    uint8_t neg = Cudd_IsComplement(bdd);
    bdd = Cudd_Regular(bdd);
    while(bdd != Cudd_NotCond(Cudd_ReadOne(dd), neg==1)) {
      if(cuddT(bdd) == Cudd_NotCond(Cudd_ReadOne(dd), neg==0)) {
	inference_item inference;
	inference.lft = -(intmax_t)bdd->index;
	inference.rgt = 0;
	ret = save_inference(BM, &inference);
	if(ret != NO_ERROR) {
	  Cudd_IterDerefBdd(dd, inference_cube);
	  return ret;
	}
	neg = neg ^ Cudd_IsComplement(cuddE(bdd));
	bdd = Cudd_Regular(cuddE(bdd));
      } else if(Cudd_Regular(cuddE(bdd)) == Cudd_NotCond(Cudd_ReadOne(dd), neg == Cudd_IsComplement(cuddE(bdd)))) {
	inference_item inference;
	inference.lft = bdd->index;
	inference.rgt = 0;
	ret = save_inference(BM, &inference);
	if(ret != NO_ERROR) {
	  Cudd_IterDerefBdd(dd, inference_cube);
	  return ret;
	}
	bdd = cuddT(bdd);
      } else assert(0);
    }
    
    Cudd_IterDerefBdd(dd, inference_cube);

    return FOUND_INFERENCE;
  }

  return ret;
}

uint8_t _collect_inferences_from_factor_bdd(BDDManager *BM, DdNode *factors, DdNode *vars, intmax_t var1, intmax_t var2, int8_t cT, uintmax_t outvar) {
  uint8_t ret = NO_ERROR;

  DdNode *one = DD_ONE(BM->dd);

  if(factors == one) return ret;
  if(factors == Cudd_Not(one) && vars==one) {
    //Factor found
    inference_item inf;
    assert(cT >= 0);
    if(var1 == ~0) return NO_ERROR; //this is the 1=1 useless factor
    if(var2 == ~0) {
      if(cT == 0) {
	//Single variable positive inference
	inf.lft = var1;
      } else {
	//Single variable negative inference
	assert(cT == 1);
	inf.lft = -var1;
      }
      inf.rgt = 0;
    } else {
      assert(cT >= 0);
      inf.lft = var1;
      if(cT == 0) {
	//Xor
	inf.rgt = -var2;
      } else {
	assert(cT == 1);
	//Equivalence
	inf.rgt = var2;
      }
    }
    ret = save_inference(BM, &inf);
    return ret;
  }

  DdNode *factorsr = Cudd_Regular(factors);
  
  if(factorsr->index == outvar) {
    assert(cT == -1);
    ret = _collect_inferences_from_factor_bdd(BM, Cudd_NotCond(cuddT(factorsr), factors != factorsr), vars, var1, var2, 1, outvar);
    if(ret != NO_ERROR) return ret;
    ret = _collect_inferences_from_factor_bdd(BM, Cudd_NotCond(cuddE(factorsr), factors != factorsr), vars, var1, var2, 0, outvar);
    if(ret != NO_ERROR) return ret;
  } else if(var1 == ~0) {
    if(vars->index == factorsr->index) {
      ret = _collect_inferences_from_factor_bdd(BM, Cudd_NotCond(cuddT(factorsr), factors != factorsr), cuddT(vars), factorsr->index, var2, cT, outvar);
      if(ret != NO_ERROR) return ret;
      ret = _collect_inferences_from_factor_bdd(BM, Cudd_NotCond(cuddE(factorsr), factors != factorsr), cuddT(vars), var1, var2, cT, outvar);
      if(ret != NO_ERROR) return ret;
    } else {
      ret = _collect_inferences_from_factor_bdd(BM, factors, cuddT(vars), vars->index, var2, cT, outvar);
      if(ret != NO_ERROR) return ret;
      ret = _collect_inferences_from_factor_bdd(BM, factors, cuddT(vars), var1, var2, cT, outvar);
      if(ret != NO_ERROR) return ret;
    }
  } else if(var2 == ~0) {
    assert(var1 != ~0);
    if(vars->index == factorsr->index) {
      ret = _collect_inferences_from_factor_bdd(BM, Cudd_NotCond(cuddT(factorsr), factors != factorsr), cuddT(vars), var1, factorsr->index, cT, outvar);
      if(ret != NO_ERROR) return ret;
      ret = _collect_inferences_from_factor_bdd(BM, Cudd_NotCond(cuddE(factorsr), factors != factorsr), cuddT(vars), var1, var2, cT, outvar);
      if(ret != NO_ERROR) return ret;
    } else {
      ret = _collect_inferences_from_factor_bdd(BM, factors, cuddT(vars), var1, vars->index, cT, outvar);
      if(ret != NO_ERROR) return ret;
      ret = _collect_inferences_from_factor_bdd(BM, factors, cuddT(vars), var1, var2, cT, outvar);
      if(ret != NO_ERROR) return ret;
    }
  } else {
    //All vars are set, only follow Else path
    if(vars->index == factorsr->index) {
      ret = _collect_inferences_from_factor_bdd(BM, Cudd_NotCond(cuddE(factorsr), factors != factorsr), cuddT(vars), var1, var2, cT, outvar);
      if(ret != NO_ERROR) return ret;
    } else {
      ret = _collect_inferences_from_factor_bdd(BM, factors, cuddT(vars), var1, var2, cT, outvar);
      if(ret != NO_ERROR) return ret;
    }
  }

  return ret;
}

uint8_t collect_inferences_from_factor_bdd(BDDManager *BM, uintmax_t bdd_loc, DdNode *factors) {
  uint8_t ret = NO_ERROR;

  DdNode *conj = build_and_BDD(BM->dd, (intmax_t *)BM->Support[bdd_loc].pList, BM->Support[bdd_loc].nLength);
  Cudd_Ref(conj);
  
  ret = _collect_inferences_from_factor_bdd(BM, factors, conj, ~0, ~0, -1, BM->nHighestVarInABDD+1);

  Cudd_IterDerefBdd(BM->dd, conj);
  
  return ret;
}

uint8_t find_and_add_inferences_and_equivalences(BDDManager *BM, uintmax_t bdd_loc) {
  uint8_t ret = NO_ERROR;

  if(apply_BDD_inferences && BM->read_input_finished) {
    ret = find_and_add_inferences(BM, bdd_loc);
    if(ret == FOUND_INFERENCE) ret = NO_ERROR;
    if(ret != NO_ERROR) return ret;
  
    if(apply_BDD_equivalences) {
      DdNode *factors = find_all_linear_factors(BM->dd, BM->BDDList[bdd_loc], (intmax_t_list *)&BM->Support[bdd_loc], BM->nHighestVarInABDD+1);
      assert(factors != NULL);
      if(factors == NULL) return BDD_GENERIC_ERR;
      Cudd_Ref(factors);
      
      ret = collect_inferences_from_factor_bdd(BM, bdd_loc, factors);
      Cudd_IterDerefBdd(BM->dd, factors);
    }
  }
  
  return ret;
}

uint8_t find_and_add_all_inferences_and_equivalences(BDDManager *BM) {
  uint8_t ret = NO_ERROR;

  if(BM->read_input_finished) {
    for(uintmax_t b = 0; b < BM->nNumBDDs; b++) {
      ret = find_and_add_inferences_and_equivalences(BM, b);
      if(ret != NO_ERROR) return ret;
    }
  }
  return ret;
}

uint8_t apply_inferences_and_equivalences(BDDManager *BM) {
  uint8_t ret = NO_ERROR;

  if(apply_BDD_inferences && in_apply_inferences==0) {
    in_apply_inferences = 1;
    
    while(!queue_is_empty(BM->InferenceQueue)) {
      intmax_t inference = (intmax_t)dequeue(BM->InferenceQueue);
      intmax_t equivalence = (intmax_t)dequeue(BM->InferenceQueue);
      
      if(equivalence == 0) {
	inference = imaxabs(inference);
	while(BM->VarBDDMap[inference].nLength > 0) {
	  uintmax_t bdd_loc = BM->VarBDDMap[inference].pList[0];
	  DdNode *inf_cube = Cudd_ReadOne(BM->dd); Cudd_Ref(inf_cube);
	  uintmax_t length = BM->Support[bdd_loc].nLength;
	  uintmax_t *support = BM->Support[bdd_loc].pList;

	  for(intmax_t i = length-1; i >= 0; i--) {
	    uintmax_t v = support[i];
	    //d2_printf2("+%ju ", v);

	    intmax_t equiv = get_equiv(BM->EQManager, v);
	    if(equiv == BM->EQManager->True) {
	      DdNode *bTemp = Cudd_bddAnd(BM->dd, Cudd_bddIthVar(BM->dd, v), inf_cube); Cudd_Ref(bTemp);
	      Cudd_IterDerefBdd(BM->dd, inf_cube);
	      inf_cube = bTemp;
	    } else if(equiv == BM->EQManager->False) {
	      DdNode *bTemp = Cudd_bddAnd(BM->dd, Cudd_Not(Cudd_bddIthVar(BM->dd, v)), inf_cube); Cudd_Ref(bTemp);
	      Cudd_IterDerefBdd(BM->dd, inf_cube);
	      inf_cube = bTemp;
	    } else continue; //If var is not set then continue
	  }
	  
	  assert(inf_cube != DD_ONE(BM->dd));
	  
	  //check_BDDList(BM);

	  uintmax_t varmap_length = BM->VarBDDMap[inference].nLength;
	
    	  ret = replace_BDD_in_manager(BM, Cudd_Cofactor(BM->dd, BM->BDDList[bdd_loc], inf_cube), bdd_loc);
	  Cudd_IterDerefBdd(BM->dd, inf_cube);
	  if(ret != NO_ERROR) {
	    return ret;
	  }

	  assert(varmap_length > BM->VarBDDMap[inference].nLength);

	}
      } else {
	assert(apply_BDD_equivalences);
	DdNode *var = Cudd_NotCond(Cudd_bddIthVar(BM->dd, inference), equivalence < 0);
	equivalence = imaxabs(equivalence);

	assert(inference > 0);
	
	while(BM->VarBDDMap[equivalence].nLength > 0) {
	  uintmax_t bdd_loc = BM->VarBDDMap[equivalence].pList[0];
	  ret = replace_BDD_in_manager(BM, Cudd_bddCompose(BM->dd, BM->BDDList[bdd_loc], var, equivalence), bdd_loc);
	  if(ret != NO_ERROR) {
	    return ret;
	  }
	}
      }
    }
    in_apply_inferences = 0;
  }

  return ret;
}
