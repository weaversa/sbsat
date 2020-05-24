#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

void SetSmurfStateVisitedFlag(TypeStateEntry *pTypeState) {
  SmurfStateEntry *pSmurfState = (SmurfStateEntry *)pTypeState;
  SmurfStateTransition *pSmurfTransition = (SmurfStateTransition *)(pSmurfState + 1);
  assert(pSmurfState->type == FN_SMURF);
  pSmurfState->visited = 1;

  for(uintmax_t i = 0; i < pSmurfState->nNumVariables; i++) {
    TypeStateEntry *pTrueTransition = pSmurfTransition->pVarIsTrueTransition;
    if(pSmurfTransition->pVarIsTrueTransition != NULL &&
       pTrueTransition->visited == 0 &&
       pTrueTransition->type != FN_SMURF) {
      arrSetStateVisitedFlag[pTrueTransition->type](pTrueTransition);
    }
    TypeStateEntry *pFalseTransition = pSmurfTransition->pVarIsFalseTransition;
    if(pSmurfTransition->pVarIsFalseTransition != NULL &&
       pFalseTransition->visited == 0 &&
       pFalseTransition->type != FN_SMURF) {
      arrSetStateVisitedFlag[pFalseTransition->type](pFalseTransition);
    }
    pSmurfTransition++;
  }
}

void UnsetSmurfStateVisitedFlag(TypeStateEntry *pTypeState) {
  SmurfStateEntry *pSmurfState = (SmurfStateEntry *)pTypeState;
  SmurfStateTransition *pSmurfTransition = (SmurfStateTransition *)(pSmurfState + 1);
  uintmax_t nNumVariables = pSmurfState->nNumVariables;
  assert(pSmurfState->type == FN_SMURF);
  pSmurfState->visited = 0;

  for(uintmax_t i = 0; i < nNumVariables; i++) {
    TypeStateEntry *pTrueTransition = pSmurfTransition->pVarIsTrueTransition;
    if(pSmurfTransition->pVarIsTrueTransition != NULL &&
       pTrueTransition->visited == 1 &&
       pTrueTransition->type != FN_SMURF) {
      arrUnsetStateVisitedFlag[pTrueTransition->type](pTrueTransition);
    }
    TypeStateEntry *pFalseTransition = pSmurfTransition->pVarIsFalseTransition;
    if(pSmurfTransition->pVarIsFalseTransition != NULL &&
       pFalseTransition->visited == 1 &&
       pFalseTransition->type != FN_SMURF) {
      arrUnsetStateVisitedFlag[pFalseTransition->type](pFalseTransition);
    }
    pSmurfTransition++;
  }
}

void CleanUpSmurfState(TypeStateEntry *pTypeState) {
  SmurfStateEntry *pSmurfState = (SmurfStateEntry *)pTypeState;
  SmurfStateTransition *pSmurfTransition = (SmurfStateTransition *)(pSmurfState + 1);
  uintmax_t nNumVariables = pSmurfState->nNumVariables;
  assert(pSmurfState->type == FN_SMURF);
  if(pSmurfState->visited == 0) return;
  
  for(uintmax_t i = 0; i < nNumVariables; i++) {
    if(pSmurfTransition->pVarIsTrueTransition &&
       ((TypeStateEntry *)pSmurfTransition->pVarIsTrueTransition)->visited == 0) {
      pSmurfTransition->pVarIsTrueTransition = NULL;
    }
    if(pSmurfTransition->pVarIsFalseTransition &&
       ((TypeStateEntry *)pSmurfTransition->pVarIsFalseTransition)->visited == 0) {
      pSmurfTransition->pVarIsFalseTransition = NULL;
    }
    pSmurfTransition++;
  }
}

uint8_t ApplyInferenceToSmurf(SmurfManager *SM, uintmax_t nVbleIndex, uint8_t bPolarity, SmurfInfoStruct *pSmurfInfo) {
  SmurfStateEntry *pSmurfState = (SmurfStateEntry *)pSmurfInfo->pCurrentState;
  assert(pSmurfState->type == FN_SMURF);

  if(pSmurfState->nNumVariables <= nVbleIndex) //Var not in Smurf or Smurf is the True state
    return NO_ERROR;

  TypeStateEntry *pNextState;
  SmurfStateTransition *pSmurfTransition = ((SmurfStateTransition *)(pSmurfState + 1)) + nVbleIndex;
  if(bPolarity == 0) {
    //Get the False transition
    pNextState = pSmurfTransition->pVarIsFalseTransition;
    if(pNextState == NULL) {
      DdNode *pNextBDD = Cudd_Cofactor(SM->dd, pSmurfState->pSmurfBDD, Cudd_Not(Cudd_bddIthVar(SM->dd, nVbleIndex)));
      Cudd_Ref(pNextBDD);
      pNextState = pSmurfTransition->pVarIsFalseTransition = ReadSmurfStateIntoTable(SM, pNextBDD);
      assert(pSmurfTransition->pVarIsFalseTransition != NULL);
      Cudd_IterDerefBdd(SM->dd, pNextBDD);
    }
  } else {
    //Get the True transition
    pNextState = pSmurfTransition->pVarIsTrueTransition;
    if(pNextState == NULL) {
      DdNode *pNextBDD = Cudd_Cofactor(SM->dd, pSmurfState->pSmurfBDD, Cudd_bddIthVar(SM->dd, nVbleIndex));
      Cudd_Ref(pNextBDD);
      pNextState = pSmurfTransition->pVarIsTrueTransition = ReadSmurfStateIntoTable(SM, pNextBDD);
      assert(pSmurfTransition->pVarIsTrueTransition != NULL);
      Cudd_IterDerefBdd(SM->dd, pNextBDD);
    }
  }

  if(pNextState == (TypeStateEntry *)pSmurfState)
    return NO_ERROR; //Var not in Smurf

  clausePush(pSmurfInfo->pLemma, (literal)nVbleIndex);

  pSmurfInfo->pCurrentState = pNextState;
  if(arrGetStateType[pNextState->type] == SM_TRANSITION_TYPE)
    return arrApplyInferenceToState[pNextState->type](SM, nVbleIndex, bPolarity, pSmurfInfo);
  
  return NO_ERROR;
}
