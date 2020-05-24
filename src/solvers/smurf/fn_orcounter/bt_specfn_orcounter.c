#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

void SetORCOUNTERStateVisitedFlag(TypeStateEntry *pTypeState) {
  ORCOUNTERStateEntry *pORCOUNTERState = (ORCOUNTERStateEntry *)pTypeState;
  assert(pORCOUNTERState->type == FN_ORCOUNTER);

  if(pORCOUNTERState->visited==0) {
    pORCOUNTERState->visited = 1;

    arrSetStateVisitedFlag[pORCOUNTERState->pTransition->type](pORCOUNTERState->pTransition);
  }
}

void UnsetORCOUNTERStateVisitedFlag(TypeStateEntry *pTypeState) {
  ORCOUNTERStateEntry *pORCOUNTERState = (ORCOUNTERStateEntry *)pTypeState;
  assert(pORCOUNTERState->type == FN_ORCOUNTER);

  if(pORCOUNTERState->visited==1) {
    pORCOUNTERState->visited = 0;

    arrUnsetStateVisitedFlag[pORCOUNTERState->pTransition->type](pORCOUNTERState->pTransition);
  }
}

void CleanUpORCOUNTERState(TypeStateEntry *pTypeState) {
  ORCOUNTERStateEntry *pORCOUNTERState = (ORCOUNTERStateEntry *)pTypeState;
  assert(pORCOUNTERState->type == FN_ORCOUNTER);
}

uint8_t ApplyInferenceToORCOUNTER(SmurfManager *SM, uintmax_t nVbleIndex, uint8_t bPolarity, SmurfInfoStruct *pSmurfInfo) {
  ORCOUNTERStateEntry *pORCOUNTERState = (ORCOUNTERStateEntry *)pSmurfInfo->pCurrentState;
  assert(pORCOUNTERState->type == FN_ORCOUNTER);
  
  ORStateEntry *pORState = pORCOUNTERState->pORState;

  if(pORState->nNumVariables <= nVbleIndex)
    return NO_ERROR; //Var not in Smurf

  uint8_t _bPolarity = pORState->bPolarity[nVbleIndex];
  if(_bPolarity == 2)
    return NO_ERROR; //Var not in Smurf

  TypeStateEntry *pNextState;

  if(bPolarity == _bPolarity) {
    //Smurf satisfied
    pNextState = (TypeStateEntry *)SM->pTrueSmurfState;
  } else {
    //Follow transition
    clausePush(pSmurfInfo->pLemma, (literal)nVbleIndex);
    pNextState = pORCOUNTERState->pTransition;
  }

  pSmurfInfo->pCurrentState = pNextState;
  if(arrGetStateType[pNextState->type] == SM_TRANSITION_TYPE)
    return arrApplyInferenceToState[pNextState->type](SM, nVbleIndex, bPolarity, pSmurfInfo);

  return NO_ERROR;
}
