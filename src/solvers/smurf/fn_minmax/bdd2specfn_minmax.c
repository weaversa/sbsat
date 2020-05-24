#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

//precondition is that SM->uTempList1 contains the support of
//pMINMAXBDD, which is true if ReadSmurfStateIntoTable was the caller
TypeStateEntry *CreateMINMAXState(SmurfManager *SM, DdNode *pMINMAXBDD) {
  MINMAXStateEntry *pMINMAXState = NULL;
  
  if((SM->uTempList1.nLength >= functionTypeLimits[FN_TYPE_MINMAX]) && isMINMAX(SM->dd, pMINMAXBDD, &SM->uTempList1)) {
    Cudd_Ref(pMINMAXBDD);
    
    //Build MINMAX state
    uintmax_t nNumBDDVars = SM->uTempList1.nLength;
    uintmax_t nNumElts = SM->uTempList1.pList[nNumBDDVars-1]+1;
    assert(nNumElts > 0);

    uint8_t ret = check_SmurfStatesTableSize(SM, (uintmax_t)sizeof(MINMAXStateEntry) + (sizeof(uint8_t) * nNumElts));
    if(ret != NO_ERROR) {assert(0); return NULL;}
    
    pMINMAXState = (MINMAXStateEntry *)SM->pSmurfStatesTableTail;
    chomp_SmurfStatesTable(SM, (uintmax_t)sizeof(MINMAXStateEntry) + (sizeof(uint8_t) * nNumElts));
    sbsat_stats[STAT_MINMAX_STATES_BUILT]+=1;
    sbsat_stats[STAT_STATES_BUILT]+=1;
    
    pMINMAXState->type             = FN_MINMAX;
    pMINMAXState->visited          = 0;
    pMINMAXState->nNumVariables    = nNumElts;
    pMINMAXState->nNumBDDVariables = nNumBDDVars;
    pMINMAXState->nMin             = getMIN(SM->dd, pMINMAXBDD, nNumBDDVars);
    pMINMAXState->nMax             = getMAX(SM->dd, pMINMAXBDD);
    pMINMAXState->bExists          = (uint8_t *)(pMINMAXState+1);
    pMINMAXState->pMINMAXBDD       = pMINMAXBDD;
    
    assert(pMINMAXState->nMin <= pMINMAXState->nMax);

    uintmax_t nBDDVar = 0;
    for(uintmax_t nVbleIndex = 0; nVbleIndex < nNumElts; nVbleIndex++) {
      if(nVbleIndex == SM->uTempList1.pList[nBDDVar]) {
	//Variable is in the BDD
	pMINMAXState->bExists[nVbleIndex] = 1;
	nBDDVar++;
      } else {
	//Variable is not in the BDD
	pMINMAXState->bExists[nVbleIndex] = 0;
      }
    }
    assert(nBDDVar == nNumBDDVars);
  }
  
  //Leaf node, just return
  return (TypeStateEntry *)pMINMAXState;
}
