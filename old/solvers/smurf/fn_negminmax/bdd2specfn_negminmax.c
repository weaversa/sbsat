#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

//precondition is that SM->uTempList1 contains the support of
//pNEGMINMAXBDD, which is true if ReadSmurfStateIntoTable was the caller
TypeStateEntry *CreateNEGMINMAXState(SmurfManager *SM, DdNode *pNEGMINMAXBDD) {
  NEGMINMAXStateEntry *pNEGMINMAXState = NULL;

  if((SM->uTempList1.nLength >= functionTypeLimits[FN_TYPE_NEGMINMAX]) && isNEGMINMAX(SM->dd, pNEGMINMAXBDD, &SM->uTempList1)) {
    Cudd_Ref(pNEGMINMAXBDD);
    
    //Build NEGMINMAX state
    uintmax_t nNumBDDVars = SM->uTempList1.nLength;
    uintmax_t nNumElts = SM->uTempList1.pList[nNumBDDVars-1]+1;
    assert(nNumElts > 0);

    uint8_t ret = check_SmurfStatesTableSize(SM, (uintmax_t)sizeof(NEGMINMAXStateEntry) * (sizeof(uint8_t) * nNumElts));
    if(ret != NO_ERROR) {assert(0); return NULL;}
    
    pNEGMINMAXState = (NEGMINMAXStateEntry *)SM->pSmurfStatesTableTail;
    chomp_SmurfStatesTable(SM, (uintmax_t)sizeof(NEGMINMAXStateEntry) + (sizeof(uint8_t) * nNumElts));
    sbsat_stats[STAT_NEGMINMAX_STATES_BUILT]+=1;
    sbsat_stats[STAT_STATES_BUILT]+=1;
    
    pNEGMINMAXState->type             = FN_NEGMINMAX;
    pNEGMINMAXState->visited          = 0;
    pNEGMINMAXState->nNumVariables    = nNumElts;
    pNEGMINMAXState->nNumBDDVariables = nNumBDDVars;
    pNEGMINMAXState->nMin             = getMIN(SM->dd, Cudd_Not(pNEGMINMAXBDD), nNumBDDVars);
    pNEGMINMAXState->nMax             = getMAX(SM->dd, Cudd_Not(pNEGMINMAXBDD));
    pNEGMINMAXState->bExists          = (uint8_t *)(pNEGMINMAXState+1);
    pNEGMINMAXState->pNEGMINMAXBDD    = pNEGMINMAXBDD;
    
    assert(pNEGMINMAXState->nMin <= pNEGMINMAXState->nMax);

    uintmax_t nBDDVar = 0;
    for(uintmax_t nVbleIndex = 0; nVbleIndex < nNumElts; nVbleIndex++) {
      if(nVbleIndex == SM->uTempList1.pList[nBDDVar]) {
	//Variable is in the BDD
	pNEGMINMAXState->bExists[nVbleIndex] = 1;
	nBDDVar++;
      } else {
	//Variable is not in the BDD
	pNEGMINMAXState->bExists[nVbleIndex] = 0;
      }
    }
    assert(nBDDVar == nNumBDDVars);    
  }

  //Leaf node, just return
  return (TypeStateEntry *)pNEGMINMAXState;
}
