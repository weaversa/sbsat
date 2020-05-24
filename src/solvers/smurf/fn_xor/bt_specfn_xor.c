#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

void SetXORStateVisitedFlag(TypeStateEntry *pTypeState) {
  XORStateEntry *pXORState = (XORStateEntry *)pTypeState;
  assert(pXORState->type == FN_XOR);

  if(pXORState->visited==0) {
    pXORState->visited = 1;
  }
}

void UnsetXORStateVisitedFlag(TypeStateEntry *pTypeState) {
  XORStateEntry *pXORState = (XORStateEntry *)pTypeState;
  assert(pXORState->type == FN_XOR);

  if(pXORState->visited==1) {
    pXORState->visited = 0;
  }
}

void CleanUpXORState(TypeStateEntry *pTypeState) {

}

uint8_t ApplyInferenceToXOR(SmurfManager *SM, uintmax_t nVbleIndex, uint8_t bPolarity, SmurfInfoStruct *pSmurfInfo) {
  XORStateEntry *pXORState = (XORStateEntry *)pSmurfInfo->pCurrentState;
  assert(pXORState->type == FN_XOR);
  
  if(pXORState->nNumVariables <= nVbleIndex) //Var not in Smurf
    return NO_ERROR;
  
  if(pXORState->bExists[nVbleIndex] == 0)
    return NO_ERROR; //Var not in Smurf

  clausePush(pSmurfInfo->pLemma, (literal)nVbleIndex);
  
  //Infer remaining var
  uint8_t *bExists = pXORState->bExists;
  for(uintmax_t i = 0; 1; i++) {
    if(bExists[i] && SM->pTrail[pSmurfInfo->pIndex2Var[i]]==0) {
      uint8_t ret = EnqueueInference_hook(SM, pSmurfInfo, i, bPolarity == pXORState->bParity);
      if(ret != NO_ERROR) return ret;
      break;
    }
  }
  
  //Smurf is satisfied
  pSmurfInfo->pCurrentState = (TypeStateEntry *)SM->pTrueSmurfState;

  return NO_ERROR;
}

uint8_t ApplyInferenceToXOR_gelim(SmurfManager *SM, uintmax_t nVbleIndex, uint8_t bPolarity, SmurfInfoStruct *pSmurfInfo) {
  XORStateEntry *pXORState = (XORStateEntry *)pSmurfInfo->pCurrentState;
  assert(pXORState->type == FN_XOR);
  
  //Fill up row w/ unset literals
  uintmax_t nNumFound = 0;
  uint8_t *bExists = pXORState->bExists;
  for(uintmax_t i = 0; nNumFound < 2; i++) {
    if(bExists[i] && SM->pTrail[pSmurfInfo->pIndex2Var[i]]==0) {
      SM->uTempList1.pList[nNumFound] = pSmurfInfo->pIndex2Var[i];
      nNumFound++;
    }
  }

  uint8_t ret = addRowXORGElimTable(SM->XM[SM->nCurrentDecisionLevel], 2, SM->uTempList1.pList, pXORState->bParity);
  if(ret != NO_ERROR) return ret;
  
  //Smurf is satisfied
  pSmurfInfo->pCurrentState = (TypeStateEntry *)SM->pTrueSmurfState;

  return NO_ERROR;
}
