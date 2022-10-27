#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

TypeStateEntry *CreateSmurfState(SmurfManager *SM, DdNode *pCurrentBDD) {
  Cudd_Ref(pCurrentBDD);

  uintmax_t nNumBDDVars = SM->uTempList1.nLength;
  assert(nNumBDDVars > 0);
  uintmax_t nNumElts = SM->uTempList1.pList[nNumBDDVars-1]+1;
  assert(nNumElts > 0);

  uint8_t ret = check_SmurfStatesTableSize(SM, (uintmax_t)sizeof(SmurfStateEntry) + (sizeof(SmurfStateTransition) * nNumElts));
  if(ret != NO_ERROR) {assert(0); return NULL;}
  
  SmurfStateEntry *pNewState = (SmurfStateEntry *)SM->pSmurfStatesTableTail;
  chomp_SmurfStatesTable(SM, (uintmax_t)sizeof(SmurfStateEntry) + (sizeof(SmurfStateTransition) * nNumElts));
  sbsat_stats[STAT_SMURF_STATES_BUILT]+=1;
  sbsat_stats[STAT_STATES_BUILT]+=1;

  pNewState->type             = FN_SMURF;
  pNewState->visited          = 0;
  pNewState->pSmurfBDD        = pCurrentBDD;
  pNewState->nNumVariables    = nNumElts;

  uintmax_t nBDDVar = 0;
  SmurfStateTransition *pNewTransition = (SmurfStateTransition *)(pNewState + 1);
  for(uintmax_t nVbleIndex = 0; nVbleIndex < nNumElts; nVbleIndex++) {
    if(nVbleIndex == SM->uTempList1.pList[nBDDVar]) {
      //Variable is in the BDD
      pNewTransition->pVarIsFalseTransition = NULL;
      pNewTransition->pVarIsTrueTransition = NULL;
      pNewTransition->fHeurWghtOfFalseTransition = SM->JHEURISTIC_K_UNKNOWN;
      pNewTransition->fHeurWghtOfTrueTransition = SM->JHEURISTIC_K_UNKNOWN;
      nBDDVar++;
    } else {
      //Variable is not in the BDD
      pNewTransition->pVarIsFalseTransition = (TypeStateEntry *)pNewState;
      pNewTransition->pVarIsTrueTransition = (TypeStateEntry *)pNewState;
      pNewTransition->fHeurWghtOfFalseTransition = 0.0;
      pNewTransition->fHeurWghtOfTrueTransition = 0.0;
    }
    pNewTransition++;
  }
  
  pNewTransition = (SmurfStateTransition *)(pNewState + 1);

  for(uintmax_t nVbleIndex = 0; nVbleIndex < nNumElts; nVbleIndex++) {
    //Compute the SmurfState w/ the transition var set to False
    if(pNewTransition->pVarIsFalseTransition != (TypeStateEntry *)pNewState) {
      if(nNumElts <= precompute_smurfs) {
	DdNode *pNextBDD = Cudd_Cofactor(SM->dd, pCurrentBDD, Cudd_Not(Cudd_bddIthVar(SM->dd, nVbleIndex)));
	Cudd_Ref(pNextBDD);
	pNewTransition->pVarIsFalseTransition = ReadSmurfStateIntoTable(SM, pNextBDD);
	assert(pNewTransition->pVarIsFalseTransition != NULL);
	Cudd_IterDerefBdd(SM->dd, pNextBDD);
	pNewTransition->fHeurWghtOfFalseTransition = arrGetStateHeuristicScore[pNewTransition->pVarIsFalseTransition->type](SM, pNewTransition->pVarIsFalseTransition);
      } else {
	pNewTransition->pVarIsFalseTransition = NULL;
	pNewTransition->fHeurWghtOfFalseTransition = 0.0;
      }
      
      //Compute the SmurfState w/ the transition var set to True
      if(nNumElts <= precompute_smurfs) {
	DdNode *pNextBDD = Cudd_Cofactor(SM->dd, pCurrentBDD, Cudd_bddIthVar(SM->dd, nVbleIndex));
	Cudd_Ref(pNextBDD);
	pNewTransition->pVarIsTrueTransition = ReadSmurfStateIntoTable(SM, pNextBDD);
	assert(pNewTransition->pVarIsTrueTransition != NULL);
	Cudd_IterDerefBdd(SM->dd, pNextBDD);
	pNewTransition->fHeurWghtOfTrueTransition = arrGetStateHeuristicScore[pNewTransition->pVarIsTrueTransition->type](SM, pNewTransition->pVarIsTrueTransition);
      } else {
	pNewTransition->pVarIsTrueTransition = NULL;
	pNewTransition->fHeurWghtOfTrueTransition = 0.0;
      }
    }

    pNewTransition++;
  }

  return (TypeStateEntry *)pNewState;
}
