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

int safe_BDD_assignments = 0;

uint8_t find_safe_assignment(BDDManager *BM, uintmax_t v) {
  uint8_t ret = NO_ERROR;

  if(safe_BDD_assignments && BM->read_input_finished) {
    if((get_equiv(BM->EQManager, v) != BM->EQManager->True) &&
       (get_equiv(BM->EQManager, v) != BM->EQManager->False)) { //Not sure this condition is necessary

      DdNode *one = DD_ONE(BM->dd);
      DdNode *safe_assign = one; //This will only ever be True or a single literal - no need to reference
      DdNode *var = Cudd_bddIthVar(BM->dd, v);
      for(uintmax_t i = 0; i < BM->VarBDDMap[v].nLength && safe_assign != Cudd_Not(one); i++) {
	uintmax_t bdd_loc = BM->VarBDDMap[v].pList[i];
	DdNode *bTemp = safe_assign0(BM->dd, BM->BDDList[bdd_loc], var);
	safe_assign = Cudd_bddAnd(BM->dd, bTemp, safe_assign);
      }
      inference_item inference;
      if(!Cudd_IsConstant(safe_assign)) {
	if(safe_assign == var) {
	  d2_printf2("\n\n{*%d=T}\n\n", var->index);
	  inference.lft = var->index;
	  inference.rgt = 0;
	  ret = save_inference(BM, &inference);
	  if(ret != NO_ERROR) {
	    return ret;
	  }
	} else {
	  assert(safe_assign == Cudd_Not(var));
	  d2_printf2("\n\n{*%d=F}\n\n", var->index);
	  inference.lft = -(intmax_t)var->index;
	  inference.rgt = 0;
	  ret = save_inference(BM, &inference);
	  if(ret != NO_ERROR) {
	    return ret;
	  }
	}
      }
    }
  }

  return ret;
}

uint8_t check_BDD_for_safe_assignment(BDDManager *BM, uintmax_t bdd_loc) {
  uint8_t ret = NO_ERROR;

  if(safe_BDD_assignments && BM->read_input_finished) {

    uintmax_t length = BM->Support[bdd_loc].nLength;
    uintmax_t *support = BM->Support[bdd_loc].pList;
    
    for(uintmax_t i = 0; i < length; i++) {
      uintmax_t v = support[i];
      ret = find_safe_assignment(BM, v);
      if(ret != NO_ERROR) return ret;
    }
  }

  return ret;
}

uint8_t find_all_safe_assignments(BDDManager *BM) {
  uint8_t ret = NO_ERROR;

  if(safe_BDD_assignments && BM->read_input_finished) {
    for(uintmax_t v = 0; v < BM->nNumVariables; v++) {
      find_safe_assignment(BM, v);
    }
  }

  return ret;
}
