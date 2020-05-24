#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

TypeStateEntry *CreateORCOUNTERState(SmurfManager *SM, DdNode *pORCOUNTERBDD) {
  ORCOUNTERStateEntry *pORCOUNTERState = NULL;

  //Build OR state
  ORStateEntry *pORState = (ORStateEntry *)CreateORState(SM, pORCOUNTERBDD);
  if(pORState) {
    pORCOUNTERState = (ORCOUNTERStateEntry *)pORState;

    //Build SM->uTempList1.nLength ORCOUNTER states
    uintmax_t i;
    uintmax_t nNumVariables = SM->uTempList1.nLength;
    for(i = 3; i <= nNumVariables; i++) {
      uint8_t ret = check_SmurfStatesTableSize(SM, (uintmax_t)sizeof(ORCOUNTERStateEntry));
      if(ret != NO_ERROR) {assert(0); return NULL;}
      
      ORCOUNTERStateEntry *pNextState = (ORCOUNTERStateEntry *)SM->pSmurfStatesTableTail;
      chomp_SmurfStatesTable(SM, (uintmax_t)sizeof(ORCOUNTERStateEntry));
      sbsat_stats[STAT_OR_STATES_BUILT]+=1;
      sbsat_stats[STAT_STATES_BUILT]+=1;
      
      pNextState->type          = FN_ORCOUNTER;
      pNextState->visited       = 0;
      pNextState->nNumVariables = i;
      pNextState->pTransition   = (TypeStateEntry *)pORCOUNTERState;
      pNextState->pORState      = pORState;
      
      pORCOUNTERState = pNextState;
    }

    assert(pORCOUNTERState->type == FN_OR ||
	 pORCOUNTERState->type == FN_ORCOUNTER);
  }

  //Leaf node, just return
  return (TypeStateEntry *)pORCOUNTERState;
}
