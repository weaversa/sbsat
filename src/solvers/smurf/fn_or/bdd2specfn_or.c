#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

TypeStateEntry *CreateORState(SmurfManager *SM, DdNode *pORBDD) {
  ORStateEntry *pORState = NULL;

  if((SM->uTempList1.nLength >= functionTypeLimits[FN_TYPE_OR] ||
     (SM->uTempList1.nLength >= functionTypeLimits[FN_TYPE_AND_EQU]-1)) &&
     isOR(SM->dd, pORBDD)) {
    Cudd_Ref(pORBDD);

    //Build OR state

    uintmax_t nNumBDDVars = SM->uTempList1.nLength;
    uintmax_t nNumElts = SM->uTempList1.pList[nNumBDDVars-1]+1;
    assert(nNumElts > 0);
    
    uint8_t ret = check_SmurfStatesTableSize(SM, (uintmax_t)sizeof(ORStateEntry) + (sizeof(uint8_t)*nNumElts));
    if(ret != NO_ERROR) {assert(0); return NULL;}
    
    pORState = (ORStateEntry *)SM->pSmurfStatesTableTail;
    chomp_SmurfStatesTable(SM, (uintmax_t)sizeof(ORStateEntry) + (sizeof(uint8_t)*nNumElts));
    sbsat_stats[STAT_OR_STATES_BUILT]+=1;
    sbsat_stats[STAT_STATES_BUILT]+=1; 
   
    pORState->type             = FN_OR;
    pORState->visited          = 0;
    pORState->nNumVariables    = nNumElts;
    pORState->nNumBDDVariables = nNumBDDVars;
    pORState->bPolarity        = (uint8_t *)(pORState+1);
    pORState->pORBDD           = pORBDD;
    
    uintmax_t nBDDVar = 0;
    DdNode *one = DD_ONE(SM->dd);
    for(uintmax_t nVbleIndex = 0; nVbleIndex < nNumElts; nVbleIndex++) {
      if(nVbleIndex == SM->uTempList1.pList[nBDDVar]) {
	//Variable is in the BDD
	DdNode *bTemp = Cudd_Cofactor(SM->dd, pORBDD, Cudd_bddIthVar(SM->dd, SM->uTempList1.pList[nBDDVar]));
	pORState->bPolarity[nVbleIndex] = (bTemp == one);
	Cudd_Ref(bTemp); Cudd_IterDerefBdd(SM->dd, bTemp);
	nBDDVar++;
      } else {
	//Variable is not in the BDD
	pORState->bPolarity[nVbleIndex] = 2;
      }
    }
    assert(nBDDVar == nNumBDDVars);
  }
  
  //Leaf node, just return
  return (TypeStateEntry *)pORState;
}
