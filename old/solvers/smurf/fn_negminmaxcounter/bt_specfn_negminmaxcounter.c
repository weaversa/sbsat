#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

void SetNEGMINMAXCOUNTERStateVisitedFlag(TypeStateEntry *pTypeState) {
  NEGMINMAXCOUNTERStateEntry *pNEGMINMAXCOUNTERState = (NEGMINMAXCOUNTERStateEntry *)pTypeState;
  assert(pNEGMINMAXCOUNTERState->type == FN_NEGMINMAXCOUNTER);

  if(pNEGMINMAXCOUNTERState->visited==0) {
    pNEGMINMAXCOUNTERState->visited = 1;

    arrSetStateVisitedFlag[pNEGMINMAXCOUNTERState->pVarIsTrueTransition->type](pNEGMINMAXCOUNTERState->pVarIsTrueTransition);
    arrSetStateVisitedFlag[pNEGMINMAXCOUNTERState->pVarIsFalseTransition->type](pNEGMINMAXCOUNTERState->pVarIsFalseTransition);
    arrSetStateVisitedFlag[pNEGMINMAXCOUNTERState->pNEGMINMAXState->type]((TypeStateEntry *)pNEGMINMAXCOUNTERState->pNEGMINMAXState);
  }
}

void UnsetNEGMINMAXCOUNTERStateVisitedFlag(TypeStateEntry *pTypeState) {
  NEGMINMAXCOUNTERStateEntry *pNEGMINMAXCOUNTERState = (NEGMINMAXCOUNTERStateEntry *)pTypeState;
  assert(pNEGMINMAXCOUNTERState->type == FN_NEGMINMAXCOUNTER);

  if(pNEGMINMAXCOUNTERState->visited==1) {
    pNEGMINMAXCOUNTERState->visited = 0;

    arrUnsetStateVisitedFlag[pNEGMINMAXCOUNTERState->pVarIsTrueTransition->type](pNEGMINMAXCOUNTERState->pVarIsTrueTransition);
    arrUnsetStateVisitedFlag[pNEGMINMAXCOUNTERState->pVarIsFalseTransition->type](pNEGMINMAXCOUNTERState->pVarIsFalseTransition);
    arrUnsetStateVisitedFlag[pNEGMINMAXCOUNTERState->pNEGMINMAXState->type]((TypeStateEntry *)pNEGMINMAXCOUNTERState->pNEGMINMAXState);
  }
}

void CleanUpNEGMINMAXCOUNTERState(TypeStateEntry *pTypeState) {

}

uint8_t ApplyInferenceToNEGMINMAXCOUNTER(SmurfManager *SM, uintmax_t nVbleIndex, uint8_t bPolarity, SmurfInfoStruct *pSmurfInfo) {
  NEGMINMAXCOUNTERStateEntry *pNEGMINMAXCOUNTERState = (NEGMINMAXCOUNTERStateEntry *)pSmurfInfo->pCurrentState;
  assert(pNEGMINMAXCOUNTERState->type == FN_NEGMINMAXCOUNTER);

  NEGMINMAXStateEntry *pNEGMINMAXState = pNEGMINMAXCOUNTERState->pNEGMINMAXState;
  
  if(pNEGMINMAXState->nNumVariables <= nVbleIndex) //Var not in Smurf
    return NO_ERROR;
  
  if(pNEGMINMAXState->bExists[nVbleIndex] == 0)
    return NO_ERROR; //Var not in Smurf

  clausePush(pSmurfInfo->pLemma, (literal)nVbleIndex);
    
  uintmax_t nNumVariables = pNEGMINMAXCOUNTERState->nNumVariables;
  uintmax_t nNumTrue = pNEGMINMAXCOUNTERState->nNumTrue;
  uintmax_t nMin = nNumTrue > pNEGMINMAXState->nMin ? 0 : pNEGMINMAXState->nMin - nNumTrue;
  uintmax_t nMax = pNEGMINMAXState->nMax - nNumTrue;

  if((bPolarity  && (nNumVariables <= nMax && nMin==2)) ||
     (!bPolarity && (nNumVariables == nMax+1 && nMin == 1))) {
    //Infer remaining variables to False
    uintmax_t nNumElts = pNEGMINMAXState->nNumVariables;
    assert(pNEGMINMAXCOUNTERState->pVarIsTrueTransition == (TypeStateEntry *)SM->pTrueSmurfState);
    uint8_t *bExists = pNEGMINMAXState->bExists;
    uintmax_t i = 0;
    for(; i < nNumElts; i++) {
      if(bExists[i] && SM->pTrail[pSmurfInfo->pIndex2Var[i]]==0) {
	uint8_t ret = EnqueueInference_hook(SM, pSmurfInfo, i, 0);
	if(ret != NO_ERROR) return ret;
      }
    }
  } else if((!bPolarity && (nMin == 0 && nNumVariables == nMax+2)) ||
            (bPolarity  && (nNumVariables == nMax+1 && nMin == 1))) {
    //Infer remaining variables to True
    uintmax_t nNumElts = pNEGMINMAXState->nNumVariables;
    assert(pNEGMINMAXCOUNTERState->pVarIsFalseTransition == (TypeStateEntry *)SM->pTrueSmurfState);
    uint8_t *bExists = pNEGMINMAXState->bExists;
    uintmax_t i = 0;
    for(; i < nNumElts; i++) {
      if(bExists[i] && SM->pTrail[pSmurfInfo->pIndex2Var[i]]==0) {
	uint8_t ret = EnqueueInference_hook(SM, pSmurfInfo, i, 1);
	if(ret != NO_ERROR) return ret;
      }
    }
  }

  //Transition to the next state
  TypeStateEntry *pNextState = bPolarity?pNEGMINMAXCOUNTERState->pVarIsTrueTransition:pNEGMINMAXCOUNTERState->pVarIsFalseTransition;

  pSmurfInfo->pCurrentState = pNextState;
  if(arrGetStateType[pNextState->type] == SM_TRANSITION_TYPE)
    return arrApplyInferenceToState[pNextState->type](SM, nVbleIndex, bPolarity, pSmurfInfo);

  return NO_ERROR;
}
