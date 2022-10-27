#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

void SetXORFACTORStateVisitedFlag(TypeStateEntry *pTypeState) {
  XORFACTORStateEntry *pXORFACTORState = (XORFACTORStateEntry *)pTypeState;
  assert(pXORFACTORState->type == FN_XORFACTOR);

  if(pXORFACTORState->visited==0) {
    pXORFACTORState->visited = 1;

    arrSetStateVisitedFlag[pXORFACTORState->pXORFactor->type](pXORFACTORState->pXORFactor);
    arrSetStateVisitedFlag[pXORFACTORState->pTransition->type](pXORFACTORState->pTransition);
  }
}

void UnsetXORFACTORStateVisitedFlag(TypeStateEntry *pTypeState) {
  XORFACTORStateEntry *pXORFACTORState = (XORFACTORStateEntry *)pTypeState;
  assert(pXORFACTORState->type == FN_XORFACTOR);

  if(pXORFACTORState->visited==1) {
    pXORFACTORState->visited = 0;

    arrUnsetStateVisitedFlag[pXORFACTORState->pXORFactor->type](pXORFACTORState->pXORFactor);
    arrUnsetStateVisitedFlag[pXORFACTORState->pTransition->type](pXORFACTORState->pTransition);
  }
}

void CleanUpXORFACTORState(TypeStateEntry *pTypeState) {

}

uint8_t ApplyInferenceToXORFACTOR(SmurfManager *SM, uintmax_t nVbleIndex, uint8_t bPolarity, SmurfInfoStruct *pSmurfInfo) {
  XORFACTORStateEntry *pXORFACTORState = (XORFACTORStateEntry *)pSmurfInfo->pCurrentState;
  assert(pXORFACTORState->type == FN_XORFACTOR);
  
  assert(gelim_smurfs);

  TypeStateEntry *pNextState = pXORFACTORState->pXORFactor;
  pSmurfInfo->pCurrentState = pNextState;
  uint8_t ret = arrApplyInferenceToState[pNextState->type](SM, nVbleIndex, bPolarity, pSmurfInfo);
  if(ret != NO_ERROR) return ret;
  
  pNextState = pXORFACTORState->pTransition;

  pSmurfInfo->pCurrentState = pNextState;
  if(arrGetStateType[pNextState->type] == SM_TRANSITION_TYPE)
    return arrApplyInferenceToState[pNextState->type](SM, nVbleIndex, bPolarity, pSmurfInfo);
  
  return NO_ERROR;
}
