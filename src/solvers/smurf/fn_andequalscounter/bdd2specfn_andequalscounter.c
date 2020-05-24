#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

TypeStateEntry *CreateANDEQUALSCOUNTERState(SmurfManager *SM, DdNode *pANDEQUALSCOUNTERBDD) {
  ANDEQUALSCOUNTERStateEntry *pANDEQUALSCOUNTERState = NULL;

  //Build ANDEQUALS state
  ANDEQUALSStateEntry *pANDEQUALSState = (ANDEQUALSStateEntry *)CreateANDEQUALSState(SM, pANDEQUALSCOUNTERBDD);
  if(pANDEQUALSState) {
    //Build ANDEQUALSCOUNTER states
    uintmax_t i;
    ORCOUNTERStateEntry *pORCOUNTERState = (ORCOUNTERStateEntry *)pANDEQUALSState->pORState;
    ANDEQUALSCOUNTERStateEntry *pPrevState = NULL;

    uintmax_t nNumVariables;
    if(pORCOUNTERState->type == FN_ORCOUNTER) {
      nNumVariables = pORCOUNTERState->nNumVariables;
    } else {
      assert(pORCOUNTERState->type == FN_OR);
      nNumVariables = ((ORStateEntry *)pORCOUNTERState)->nNumBDDVariables;
    }

    for(i = nNumVariables+1; i > 2; i--) {
      uint8_t ret = check_SmurfStatesTableSize(SM, (uintmax_t)sizeof(ANDEQUALSCOUNTERStateEntry));
      if(ret != NO_ERROR) {assert(0); return NULL;}

      ANDEQUALSCOUNTERStateEntry *pNextState = (ANDEQUALSCOUNTERStateEntry *)SM->pSmurfStatesTableTail;
      chomp_SmurfStatesTable(SM, (uintmax_t)sizeof(ANDEQUALSCOUNTERStateEntry));
      sbsat_stats[STAT_ANDEQUALS_STATES_BUILT]+=1;
      sbsat_stats[STAT_STATES_BUILT]+=1;
      
      pNextState->type            = FN_ANDEQUALSCOUNTER;
      pNextState->visited         = 0;
      pNextState->pORCOUNTERState = pORCOUNTERState;
      pNextState->pANDEQUALSState = pANDEQUALSState;      
      
      if(i > 3) {
	assert(pORCOUNTERState->type == FN_ORCOUNTER);
	pORCOUNTERState = (ORCOUNTERStateEntry *)pORCOUNTERState->pTransition;
      }
      
      if(pANDEQUALSCOUNTERState == NULL) {
	pANDEQUALSCOUNTERState = pPrevState = pNextState;
      } else {
	pPrevState->pTransition = (TypeStateEntry *)pNextState;
	pPrevState = pNextState;
      }
    }
    assert(pANDEQUALSCOUNTERState != NULL);
    pPrevState->pTransition = (TypeStateEntry *)pANDEQUALSState;

    pANDEQUALSState->pORState = (ORStateEntry *)pORCOUNTERState;
    assert(pORCOUNTERState->type == FN_OR);
  }

  //Leaf node, just return
  return (TypeStateEntry *)pANDEQUALSCOUNTERState;
}
