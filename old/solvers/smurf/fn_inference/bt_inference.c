#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

void SetInferenceStateVisitedFlag(TypeStateEntry *pTypeState) {
  InferenceStateEntry *pInferenceState = (InferenceStateEntry *)pTypeState;
  assert(pInferenceState->type == FN_INFERENCE);

  while(pInferenceState != NULL && pInferenceState->type == FN_INFERENCE && pInferenceState->visited == 0) {
    pInferenceState->visited = 1;
    pInferenceState = (InferenceStateEntry *)pInferenceState->pTransition;
  }
  if(pInferenceState != NULL && pInferenceState->type != FN_SMURF && pInferenceState->visited != 1)
    arrSetStateVisitedFlag[pInferenceState->type]((TypeStateEntry *)pInferenceState);
}

void UnsetInferenceStateVisitedFlag(TypeStateEntry *pTypeState) {
  InferenceStateEntry *pInferenceState = (InferenceStateEntry *)pTypeState;
  assert(pInferenceState->type == FN_INFERENCE);

  while(pInferenceState != NULL && pInferenceState->type == FN_INFERENCE && pInferenceState->visited == 1) {
    pInferenceState->visited = 0;
    pInferenceState = (InferenceStateEntry *)pInferenceState->pTransition;
  }
  if(pInferenceState != NULL && pInferenceState->type != FN_SMURF && pInferenceState->visited != 0)
    arrUnsetStateVisitedFlag[pInferenceState->type]((TypeStateEntry *)pInferenceState);
}

void CleanUpInferenceState(TypeStateEntry *pTypeState) {
  InferenceStateEntry *pInferenceState = (InferenceStateEntry *)pTypeState;
  assert(pInferenceState->type == FN_INFERENCE);

  if(pInferenceState->visited == 0) return;
  
  if(pInferenceState->pTransition && pInferenceState->pTransition->visited == 0)
    pInferenceState->pTransition = NULL;
}

uint8_t ApplyInferenceToInference(SmurfManager *SM, uintmax_t nVbleIndex, uint8_t bPolarity, SmurfInfoStruct *pSmurfInfo) {
  InferenceStateEntry *pInferenceState = (InferenceStateEntry *)pSmurfInfo->pCurrentState;
  assert(pInferenceState->type == FN_INFERENCE);

  uint8_t ret = EnqueueInference_hook(SM, pSmurfInfo, pInferenceState->nTransitionVar, pInferenceState->bTransitionPol);
  if(ret != NO_ERROR) {assert(ret == 113); return ret;}

  TypeStateEntry *pNextState = pInferenceState->pTransition;
  
  if(pNextState == NULL) {
    DdNode *pNextBDD = Cudd_Cofactor(SM->dd, pInferenceState->pInferenceBDD, Cudd_NotCond(Cudd_bddIthVar(SM->dd, pInferenceState->nTransitionVar), !pInferenceState->bTransitionPol));
    Cudd_Ref(pNextBDD);
    pNextState = pInferenceState->pTransition = ReadSmurfStateIntoTable(SM, pNextBDD);
    assert(pInferenceState->pTransition != NULL);
    Cudd_IterDerefBdd(SM->dd, pNextBDD);
  }

  pSmurfInfo->pCurrentState = pNextState;
  if(arrGetStateType[pNextState->type] == SM_TRANSITION_TYPE)
    return arrApplyInferenceToState[pNextState->type](SM, nVbleIndex, bPolarity, pSmurfInfo);

  return NO_ERROR;
}  
