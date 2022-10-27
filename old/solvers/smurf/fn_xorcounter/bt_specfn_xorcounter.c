#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

void SetXORCOUNTERStateVisitedFlag(TypeStateEntry *pTypeState) {
  XORCOUNTERStateEntry *pXORCOUNTERState = (XORCOUNTERStateEntry *)pTypeState;
  assert(pXORCOUNTERState->type == FN_XORCOUNTER);

  if(pXORCOUNTERState->visited==0) {
    pXORCOUNTERState->visited = 1;

    arrSetStateVisitedFlag[pXORCOUNTERState->pVarIsTrueTransition->type](pXORCOUNTERState->pVarIsTrueTransition);
    arrSetStateVisitedFlag[pXORCOUNTERState->pVarIsFalseTransition->type](pXORCOUNTERState->pVarIsFalseTransition);
  }
}

void UnsetXORCOUNTERStateVisitedFlag(TypeStateEntry *pTypeState) {
  XORCOUNTERStateEntry *pXORCOUNTERState = (XORCOUNTERStateEntry *)pTypeState;
  assert(pXORCOUNTERState->type == FN_XORCOUNTER);

  if(pXORCOUNTERState->visited==1) {
    pXORCOUNTERState->visited = 0;

    arrUnsetStateVisitedFlag[pXORCOUNTERState->pVarIsTrueTransition->type](pXORCOUNTERState->pVarIsTrueTransition);
    arrUnsetStateVisitedFlag[pXORCOUNTERState->pVarIsFalseTransition->type](pXORCOUNTERState->pVarIsFalseTransition);
  }
}

void CleanUpXORCOUNTERState(TypeStateEntry *pTypeState) {
  XORCOUNTERStateEntry *pXORCOUNTERState = (XORCOUNTERStateEntry *)pTypeState;
  assert(pXORCOUNTERState->type == FN_XORCOUNTER);
}

uint8_t ApplyInferenceToXORCOUNTER(SmurfManager *SM, uintmax_t nVbleIndex, uint8_t bPolarity, SmurfInfoStruct *pSmurfInfo) {
  XORCOUNTERStateEntry *pXORCOUNTERState = (XORCOUNTERStateEntry *)pSmurfInfo->pCurrentState;
  assert(pXORCOUNTERState->type == FN_XORCOUNTER);
  
  XORStateEntry *pXORState = pXORCOUNTERState->pXORState;

  if(pXORState->nNumVariables <= nVbleIndex) //Var not in Smurf
    return NO_ERROR;

  if(pXORState->bExists[nVbleIndex] == 0)
    return NO_ERROR; //Var not in Smurf

  TypeStateEntry *pNextState;

  if(bPolarity) {
    //Follow True transition
    pNextState = pXORCOUNTERState->pVarIsTrueTransition;
  } else {
    //Follow False transition
    pNextState = pXORCOUNTERState->pVarIsFalseTransition;
  }

  clausePush(pSmurfInfo->pLemma, (literal)nVbleIndex);

  pSmurfInfo->pCurrentState = pNextState;
  if(arrGetStateType[pNextState->type] == SM_TRANSITION_TYPE)
    return arrApplyInferenceToState[pNextState->type](SM, nVbleIndex, bPolarity, pSmurfInfo);
  
  return NO_ERROR;
}

uint8_t ApplyInferenceToXORCOUNTER_gelim(SmurfManager *SM, uintmax_t nVbleIndex, uint8_t bPolarity, SmurfInfoStruct *pSmurfInfo) {
  XORCOUNTERStateEntry *pXORCOUNTERState = (XORCOUNTERStateEntry *)pSmurfInfo->pCurrentState;
  assert(pXORCOUNTERState->type == FN_XORCOUNTER);
  
  //Fill up row w/ unset literals
  uintmax_t nNumVariables = pXORCOUNTERState->nNumVariables;
  uintmax_t nNumFound = 0;
  uint8_t *bExists = pXORCOUNTERState->pXORState->bExists;
  for(uintmax_t i = 0; nNumFound < nNumVariables; i++) {
    assert(i < pXORCOUNTERState->pXORState->nNumVariables);
    if(bExists[i] && SM->pTrail[pSmurfInfo->pIndex2Var[i]]==0) {
      SM->uTempList1.pList[nNumFound] = pSmurfInfo->pIndex2Var[i];
      nNumFound++;
    }
  }

  uint8_t ret = addRowXORGElimTable(SM->XM[SM->nCurrentDecisionLevel], nNumFound, SM->uTempList1.pList, pXORCOUNTERState->pXORState->bParity);
  if(ret != NO_ERROR) return ret;
  
  //Smurf is satisfied
  pSmurfInfo->pCurrentState = (TypeStateEntry *)SM->pTrueSmurfState;

  return NO_ERROR;
}
