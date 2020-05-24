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

int early_quantify_BDD_vars = 0;
uint8_t in_perform_early_quantification = 0;

uint8_t add_potential_early_quantification_var(BDDManager *BM, uintmax_t var) {
  uint8_t ret = NO_ERROR;
  if(early_quantify_BDD_vars && BM->read_input_finished) {
    if(var <= BM->ExQuantProtectedVars.nLength || BM->ExQuantProtectedVars.pList[var] == 0) {
      if(imaxabs(get_equiv(BM->EQManager, var)) == (intmax_t)var) { //And var is not in inference queue
	//SEAN!!! This is an insufficient block - should have a similar 'VarQuantified' array
	ret = uintmax_t_list_push(&BM->PossibleExQuantVars, var);
      }
      if(ret!=NO_ERROR) return ret;
    }
  }
  return ret;
}

uint8_t find_all_early_quantification_vars(BDDManager *BM) {
  uint8_t ret = NO_ERROR;
  if(early_quantify_BDD_vars && BM->read_input_finished) {
    for(intmax_t v = 0; v < (intmax_t)BM->nNumVariables; v++) {
      if(BM->VarBDDMap[v].nLength == 1) { //Var occurs in only one BDD
	ret = add_potential_early_quantification_var(BM, v);
	if(ret != NO_ERROR) return ret;
      }
    }
  }

  return ret;
}

uint8_t perform_early_quantification(BDDManager *BM) {
  uint8_t ret = NO_ERROR;
  if(early_quantify_BDD_vars && in_perform_early_quantification==0) {
    in_perform_early_quantification = 1;

    for(uintmax_t i = 0; i < BM->PossibleExQuantVars.nLength; i++) {
      uintmax_t var = BM->PossibleExQuantVars.pList[i];
      uintmax_t bdd_loc = BM->VarBDDMap[var].pList[0];
      uintmax_t length = BM->Support[bdd_loc].nLength;
      DdNode *ExQuantCube = Cudd_ReadOne(BM->dd); Cudd_Ref(ExQuantCube);
      for(uintmax_t j = 0; j < length; j++) {
	uintmax_t bdd_var = BM->Support[bdd_loc].pList[j];
	if(bdd_var >= BM->ExQuantProtectedVars.nLength || BM->ExQuantProtectedVars.pList[bdd_var] == 0) {
	  if(BM->VarBDDMap[bdd_var].nLength == 1) {
	    if((get_equiv(BM->EQManager, bdd_var) != BM->EQManager->True) &&
	       (get_equiv(BM->EQManager, bdd_var) != BM->EQManager->False)) {
	      d2_printf2("{*%s}", s_name(bdd_var));
	      DdNode *bTemp = Cudd_bddAnd(BM->dd, Cudd_bddIthVar(BM->dd, bdd_var), ExQuantCube); Cudd_Ref(bTemp);
	      Cudd_IterDerefBdd(BM->dd, ExQuantCube);
	      ExQuantCube = bTemp;
	    }
	  }
	}
      }
      if(ExQuantCube != Cudd_ReadOne(BM->dd)) {
	DdNode *new_bdd = Cudd_bddExistAbstract(BM->dd, BM->BDDList[bdd_loc], ExQuantCube);

	ret = replace_BDD_in_manager(BM, new_bdd, bdd_loc);

	if(ret!=NO_ERROR) return ret;
      }

      Cudd_IterDerefBdd(BM->dd, ExQuantCube);
    }
    BM->PossibleExQuantVars.nLength = 0;
    in_perform_early_quantification = 0;
  }
  return ret;
}
