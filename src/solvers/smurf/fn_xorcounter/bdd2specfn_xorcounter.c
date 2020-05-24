#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

TypeStateEntry *CreateXORCOUNTERState(SmurfManager *SM, DdNode *pXORCOUNTERBDD) {
  XORCOUNTERStateEntry *pXORCOUNTERState = NULL;

  //Build XOR state
  XORStateEntry *pXORState = (XORStateEntry *)CreateXORState(SM, pXORCOUNTERBDD);

  if(pXORState) {
    uintmax_t nNumVariables = SM->uTempList1.nLength;
    if(nNumVariables < 3) {
      pXORCOUNTERState = (XORCOUNTERStateEntry *)pXORState;
    } else {
      //Build SM->uTempList1.nLength XORCOUNTER states for the False edge
      XORCOUNTERStateEntry *pXORCOUNTERFalseState = (XORCOUNTERStateEntry *)pXORState;
      uintmax_t i;
      for(i = 3; i < nNumVariables; i++) {
	uint8_t ret = check_SmurfStatesTableSize(SM, (uintmax_t)sizeof(XORCOUNTERStateEntry));
	if(ret != NO_ERROR) {assert(0); return NULL;}
	
	XORCOUNTERStateEntry *pNextState = (XORCOUNTERStateEntry *)SM->pSmurfStatesTableTail;
	chomp_SmurfStatesTable(SM, (uintmax_t)sizeof(XORCOUNTERStateEntry));
	sbsat_stats[STAT_XOR_STATES_BUILT]+=1;
	sbsat_stats[STAT_STATES_BUILT]+=1;
	
	pNextState->type                    = FN_XORCOUNTER;
	pNextState->visited                 = 0;
	pNextState->nNumVariables           = i;
	pNextState->pVarIsFalseTransition   = (TypeStateEntry *)pXORCOUNTERFalseState;
	pNextState->pXORState               = pXORState;
	
	pXORCOUNTERFalseState = pNextState;
      }
      
      //Build SM->uTempList1.nLength XORCOUNTER states for the True edge
      XORStateEntry *pNEGXORState = (XORStateEntry *)CreateXORState(SM, Cudd_Not(pXORCOUNTERBDD));
      assert(pNEGXORState != NULL);
      XORCOUNTERStateEntry *pXORCOUNTERTrueState = (XORCOUNTERStateEntry *)pNEGXORState;
      for(i = 3; i < nNumVariables; i++) {
	uint8_t ret = check_SmurfStatesTableSize(SM, (uintmax_t)sizeof(XORCOUNTERStateEntry));
	if(ret != NO_ERROR) {assert(0); return NULL;}
	
	XORCOUNTERStateEntry *pNextState = (XORCOUNTERStateEntry *)SM->pSmurfStatesTableTail;
	chomp_SmurfStatesTable(SM, (uintmax_t)sizeof(XORCOUNTERStateEntry));
	sbsat_stats[STAT_XOR_STATES_BUILT]+=1;
	sbsat_stats[STAT_STATES_BUILT]+=1;
	
	pNextState->type                    = FN_XORCOUNTER;
	pNextState->visited                 = 0;
	pNextState->nNumVariables           = i;
	pNextState->pVarIsFalseTransition   = (TypeStateEntry *)pXORCOUNTERTrueState; //This looks like a bug, but it really is 'pVarIsFalseTransition'
	pNextState->pXORState               = pXORState;
	
	pXORCOUNTERTrueState = pNextState;
      }
      
      //Build the top XORCOUNTER state
      uint8_t ret = check_SmurfStatesTableSize(SM, (uintmax_t)sizeof(XORCOUNTERStateEntry));
      if(ret != NO_ERROR) {assert(0); return NULL;}
      
      pXORCOUNTERState = (XORCOUNTERStateEntry *)SM->pSmurfStatesTableTail;
      chomp_SmurfStatesTable(SM, (uintmax_t)sizeof(XORCOUNTERStateEntry));
      sbsat_stats[STAT_XOR_STATES_BUILT]+=1;
      sbsat_stats[STAT_STATES_BUILT]+=1;
      
      pXORCOUNTERState->type                    = FN_XORCOUNTER;
      pXORCOUNTERState->visited                 = 0;
      pXORCOUNTERState->nNumVariables           = i;
      pXORCOUNTERState->pVarIsFalseTransition   = (TypeStateEntry *)pXORCOUNTERFalseState;
      pXORCOUNTERState->pVarIsTrueTransition    = (TypeStateEntry *)pXORCOUNTERTrueState;
      pXORCOUNTERState->pXORState               = pXORState;
      
      //Zip the two sides together
      for(i = nNumVariables-1; i > 2; i--) {
	pXORCOUNTERFalseState->pVarIsTrueTransition = pXORCOUNTERTrueState->pVarIsFalseTransition;
	pXORCOUNTERTrueState->pVarIsTrueTransition = pXORCOUNTERFalseState->pVarIsFalseTransition;
	pXORCOUNTERFalseState = (XORCOUNTERStateEntry *)pXORCOUNTERFalseState->pVarIsFalseTransition;
	pXORCOUNTERTrueState = (XORCOUNTERStateEntry *)pXORCOUNTERTrueState->pVarIsFalseTransition;
      }
    }
  }

  //Leaf node, just return
  return (TypeStateEntry *)pXORCOUNTERState;
}
