#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

void SetMINMAXCOUNTERStateVisitedFlag(TypeStateEntry *pTypeState) {
  MINMAXCOUNTERStateEntry *pMINMAXCOUNTERState = (MINMAXCOUNTERStateEntry *)pTypeState;
  assert(pMINMAXCOUNTERState->type == FN_MINMAXCOUNTER);

  if(pMINMAXCOUNTERState->visited==0) {
    pMINMAXCOUNTERState->visited = 1;

    arrSetStateVisitedFlag[pMINMAXCOUNTERState->pVarIsTrueTransition->type](pMINMAXCOUNTERState->pVarIsTrueTransition);
    arrSetStateVisitedFlag[pMINMAXCOUNTERState->pVarIsFalseTransition->type](pMINMAXCOUNTERState->pVarIsFalseTransition);
    arrSetStateVisitedFlag[pMINMAXCOUNTERState->pMINMAXState->type]((TypeStateEntry *)pMINMAXCOUNTERState->pMINMAXState);
  }
}

void UnsetMINMAXCOUNTERStateVisitedFlag(TypeStateEntry *pTypeState) {
  MINMAXCOUNTERStateEntry *pMINMAXCOUNTERState = (MINMAXCOUNTERStateEntry *)pTypeState;
  assert(pMINMAXCOUNTERState->type == FN_MINMAXCOUNTER);

  if(pMINMAXCOUNTERState->visited==1) {
    pMINMAXCOUNTERState->visited = 0;

    arrUnsetStateVisitedFlag[pMINMAXCOUNTERState->pVarIsTrueTransition->type](pMINMAXCOUNTERState->pVarIsTrueTransition);
    arrUnsetStateVisitedFlag[pMINMAXCOUNTERState->pVarIsFalseTransition->type](pMINMAXCOUNTERState->pVarIsFalseTransition);
    arrUnsetStateVisitedFlag[pMINMAXCOUNTERState->pMINMAXState->type]((TypeStateEntry *)pMINMAXCOUNTERState->pMINMAXState);
  }
}

void CleanUpMINMAXCOUNTERState(TypeStateEntry *pTypeState) {

}

uint8_t ApplyInferenceToMINMAXCOUNTER(SmurfManager *SM, uintmax_t nVbleIndex, uint8_t bPolarity, SmurfInfoStruct *pSmurfInfo) {
  MINMAXCOUNTERStateEntry *pMINMAXCOUNTERState = (MINMAXCOUNTERStateEntry *)pSmurfInfo->pCurrentState;
  assert(pMINMAXCOUNTERState->type == FN_MINMAXCOUNTER);

  MINMAXStateEntry *pMINMAXState = pMINMAXCOUNTERState->pMINMAXState;
  
  if(pMINMAXState->nNumVariables <= nVbleIndex)
    return NO_ERROR; //Var not in Smurf
  
  if(pMINMAXState->bExists[nVbleIndex] == 0)
    return NO_ERROR; //Var not in Smurf

  clausePush(pSmurfInfo->pLemma, (literal)nVbleIndex);
    
  uintmax_t nNumVariables = pMINMAXCOUNTERState->nNumVariables;
  uintmax_t nNumTrue = pMINMAXCOUNTERState->nNumTrue;
  uintmax_t nMin = nNumTrue > pMINMAXState->nMin ? 0 : pMINMAXState->nMin - nNumTrue;
  uintmax_t nMax = pMINMAXState->nMax - nNumTrue;

  if(bPolarity && (nMax==1)) {
    //Infer remaining variables to False
    uintmax_t nNumElts = pMINMAXState->nNumVariables;
    assert(pMINMAXCOUNTERState->pVarIsTrueTransition == (TypeStateEntry *)SM->pTrueSmurfState);
    uint8_t *bExists = pMINMAXState->bExists;
    uintmax_t i = 0;
    for(; i < nNumElts; i++) {
      if(bExists[i] && SM->pTrail[pSmurfInfo->pIndex2Var[i]]==0) {
	uint8_t ret = EnqueueInference_hook(SM, pSmurfInfo, i, 0);
	if(ret != NO_ERROR) return ret;
      }
    }
  } else if(!bPolarity && (nNumVariables-1 == nMin)) {
    //Infer remaining variables to True
    uintmax_t nNumElts = pMINMAXState->nNumVariables;
    assert(pMINMAXCOUNTERState->pVarIsFalseTransition == (TypeStateEntry *)SM->pTrueSmurfState);
    uint8_t *bExists = pMINMAXState->bExists;
    uintmax_t i = 0;
    for(; i < nNumElts; i++) {
      if(bExists[i] && SM->pTrail[pSmurfInfo->pIndex2Var[i]]==0) {
	uint8_t ret = EnqueueInference_hook(SM, pSmurfInfo, i, 1);
	if(ret != NO_ERROR) return ret;
      }
    }
  }

  //Transition to the next state
  TypeStateEntry *pNextState = bPolarity?pMINMAXCOUNTERState->pVarIsTrueTransition:pMINMAXCOUNTERState->pVarIsFalseTransition;

  pSmurfInfo->pCurrentState = pNextState;
  if(arrGetStateType[pNextState->type] == SM_TRANSITION_TYPE)
    return arrApplyInferenceToState[pNextState->type](SM, nVbleIndex, bPolarity, pSmurfInfo);

  return NO_ERROR;
}
