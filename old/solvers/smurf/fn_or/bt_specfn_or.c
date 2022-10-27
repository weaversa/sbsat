#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

void SetORStateVisitedFlag(TypeStateEntry *pTypeState) {
  ORStateEntry *pORState = (ORStateEntry *)pTypeState;
  assert(pORState->type == FN_OR);

  if(pORState->visited==0) {
    pORState->visited = 1;
  }
}

void UnsetORStateVisitedFlag(TypeStateEntry *pTypeState) {
  ORStateEntry *pORState = (ORStateEntry *)pTypeState;
  assert(pORState->type == FN_OR);

  if(pORState->visited==1) {
    pORState->visited = 0;
  }
}

void CleanUpORState(TypeStateEntry *pTypeState) {
  ORStateEntry *pORState = (ORStateEntry *)pTypeState;
  assert(pORState->type == FN_OR);
}

uint8_t ApplyInferenceToOR(SmurfManager *SM, uintmax_t nVbleIndex, uint8_t bPolarity, SmurfInfoStruct *pSmurfInfo) {
  ORStateEntry *pORState = (ORStateEntry *)pSmurfInfo->pCurrentState;
  assert(pORState->type == FN_OR);

  d9_printf3("Checking OR Smurf %ju %ju\n", nVbleIndex, pORState->nNumVariables);

  if(pORState->nNumVariables <= nVbleIndex)
    return NO_ERROR; //Var not in Smurf

  uint8_t _bPolarity = pORState->bPolarity[nVbleIndex];
  if(_bPolarity == 2)
    return NO_ERROR; //Var not in Smurf

  if(bPolarity != _bPolarity) {
    clausePush(pSmurfInfo->pLemma, (literal)nVbleIndex);
    
    //Infer remaining var
    uint8_t *barrPolarity = pORState->bPolarity;
        
    for(uintmax_t i = 0; 1; i++) {
      if(barrPolarity[i] != 2 && SM->pTrail[pSmurfInfo->pIndex2Var[i]]==0) {
	uint8_t ret = EnqueueInference_hook(SM, pSmurfInfo, i, pORState->bPolarity[i]); 
	if(ret != NO_ERROR) return ret;
	break;
      }
    }
  }
  
  //Smurf is satisfied
  pSmurfInfo->pCurrentState = (TypeStateEntry *)SM->pTrueSmurfState;

  return NO_ERROR;
}
