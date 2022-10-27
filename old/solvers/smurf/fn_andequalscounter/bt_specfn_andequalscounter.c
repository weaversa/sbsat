#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

void SetANDEQUALSCOUNTERStateVisitedFlag(TypeStateEntry *pTypeState) {
  ANDEQUALSCOUNTERStateEntry *pANDEQUALSCOUNTERState = (ANDEQUALSCOUNTERStateEntry *)pTypeState;
  assert(pANDEQUALSCOUNTERState->type == FN_ANDEQUALSCOUNTER);

  if(pANDEQUALSCOUNTERState->visited==0) {
    pANDEQUALSCOUNTERState->visited = 1;

    arrSetStateVisitedFlag[pANDEQUALSCOUNTERState->pTransition->type](pANDEQUALSCOUNTERState->pTransition);
    arrSetStateVisitedFlag[pANDEQUALSCOUNTERState->pORCOUNTERState->type]((TypeStateEntry *)pANDEQUALSCOUNTERState->pORCOUNTERState);
    arrSetStateVisitedFlag[pANDEQUALSCOUNTERState->pANDEQUALSState->type]((TypeStateEntry *)pANDEQUALSCOUNTERState->pANDEQUALSState);
  }
}

void UnsetANDEQUALSCOUNTERStateVisitedFlag(TypeStateEntry *pTypeState) {
  ANDEQUALSCOUNTERStateEntry *pANDEQUALSCOUNTERState = (ANDEQUALSCOUNTERStateEntry *)pTypeState;
  assert(pANDEQUALSCOUNTERState->type == FN_ANDEQUALSCOUNTER);

  if(pANDEQUALSCOUNTERState->visited==1) {
    pANDEQUALSCOUNTERState->visited = 0;

    arrUnsetStateVisitedFlag[pANDEQUALSCOUNTERState->pTransition->type](pANDEQUALSCOUNTERState->pTransition);
    arrUnsetStateVisitedFlag[pANDEQUALSCOUNTERState->pORCOUNTERState->type]((TypeStateEntry *)pANDEQUALSCOUNTERState->pORCOUNTERState);
    arrUnsetStateVisitedFlag[pANDEQUALSCOUNTERState->pANDEQUALSState->type]((TypeStateEntry *)pANDEQUALSCOUNTERState->pANDEQUALSState);
  }
}

void CleanUpANDEQUALSCOUNTERState(TypeStateEntry *pTypeState) {
  ANDEQUALSCOUNTERStateEntry *pANDEQUALSCOUNTERState = (ANDEQUALSCOUNTERStateEntry *)pTypeState;
  assert(pANDEQUALSCOUNTERState->type == FN_ANDEQUALSCOUNTER);
}

uint8_t ApplyInferenceToANDEQUALSCOUNTER(SmurfManager *SM, uintmax_t nVbleIndex, uint8_t bPolarity, SmurfInfoStruct *pSmurfInfo) {
  ANDEQUALSCOUNTERStateEntry *pANDEQUALSCOUNTERState = (ANDEQUALSCOUNTERStateEntry *)pSmurfInfo->pCurrentState;
  assert(pANDEQUALSCOUNTERState->type == FN_ANDEQUALSCOUNTER);

  TypeStateEntry *pNextState;

  if(nVbleIndex == pANDEQUALSCOUNTERState->pANDEQUALSState->nLHS) {
    //Setting the head
    if(pANDEQUALSCOUNTERState->pANDEQUALSState->bAND_or_OR == bPolarity) {
      uintmax_t nNumVariables, nNumBDDVariables;
      if(pANDEQUALSCOUNTERState->pORCOUNTERState->type == FN_ORCOUNTER) {
	nNumVariables = pANDEQUALSCOUNTERState->pORCOUNTERState->nNumVariables;
	nNumBDDVariables = pANDEQUALSCOUNTERState->pORCOUNTERState->pORState->nNumBDDVariables;
      } else {
	assert(pANDEQUALSCOUNTERState->pORCOUNTERState->type == FN_OR);
	nNumVariables = 2;
	nNumBDDVariables = ((ORStateEntry *)pANDEQUALSCOUNTERState->pORCOUNTERState)->nNumBDDVariables;
      }

      //Monkey w/ lemma - not necessary, just an optimization
      uintmax_t nLemmaTop = pSmurfInfo->pLemma->size - (nNumBDDVariables - nNumVariables);
      assert(nLemmaTop < pSmurfInfo->nNumVariables);
      literal_undoer_push(&SM->pLemmaLiteral_undoer, &pSmurfInfo->pLemma->data[nLemmaTop]);
      literal_undoer_push(&SM->pLemmaLiteral_undoer, &pSmurfInfo->pLemma->data[nLemmaTop+1]);
      pSmurfInfo->pLemma->data[nLemmaTop] = (literal)nVbleIndex;

      d9_printf3("ANDEQUALC Assigning pLemma[%ju] = %jd because of head\n", nLemmaTop, bPolarity?-(literal)pSmurfInfo->pIndex2Var[nVbleIndex]:(literal)pSmurfInfo->pIndex2Var[nVbleIndex]);
      pSmurfInfo->pLemma->size = nLemmaTop+1;
      
      uint8_t *barrPolarity = pANDEQUALSCOUNTERState->pANDEQUALSState->pORState->bPolarity;
      uintmax_t nNumElts = pANDEQUALSCOUNTERState->pANDEQUALSState->pORState->nNumVariables;
      for(uintmax_t i = 0; i < nNumElts; i++) {
	uint8_t _bPolarity = barrPolarity[i];
	if(_bPolarity!=2 && SM->pTrail[pSmurfInfo->pIndex2Var[i]]==0) {
	  uint8_t ret = EnqueueInference_hook(SM, pSmurfInfo, i, !_bPolarity);
	  if(ret != NO_ERROR) return ret;
	}
      }
      //Smurf is satisfied
      pNextState = (TypeStateEntry *)SM->pTrueSmurfState;
    } else {
      //Transition to ORCOUNTERState
      clausePush(pSmurfInfo->pLemma, (literal)nVbleIndex);
      pNextState = (TypeStateEntry *)pANDEQUALSCOUNTERState->pORCOUNTERState;
    }
  } else {
    uintmax_t nNumElts = pANDEQUALSCOUNTERState->pANDEQUALSState->pORState->nNumVariables;
    if(nNumElts <= nVbleIndex)
      return NO_ERROR; //Var not in Smurf
    
    uint8_t _bPolarity = pANDEQUALSCOUNTERState->pANDEQUALSState->pORState->bPolarity[nVbleIndex];
    if(_bPolarity == 2)
      return NO_ERROR; //Var not in Smurf
    
    if(bPolarity != _bPolarity) {
      //Normal Transition
      clausePush(pSmurfInfo->pLemma, (literal)nVbleIndex);
      pNextState = (TypeStateEntry *)pANDEQUALSCOUNTERState->pTransition;
    } else {
      //Infer head
      uintmax_t nNumVariables, nNumBDDVariables;
      if(pANDEQUALSCOUNTERState->pORCOUNTERState->type == FN_ORCOUNTER) {
	nNumVariables = pANDEQUALSCOUNTERState->pORCOUNTERState->nNumVariables;
	nNumBDDVariables = pANDEQUALSCOUNTERState->pORCOUNTERState->pORState->nNumBDDVariables;
      } else {
	assert(pANDEQUALSCOUNTERState->pORCOUNTERState->type == FN_OR);
	nNumVariables = 2;// ((ORStateEntry *)pANDEQUALSCOUNTERState->pORCOUNTERState)->nNumBDDVariables;
	nNumBDDVariables = ((ORStateEntry *)pANDEQUALSCOUNTERState->pORCOUNTERState)->nNumBDDVariables;
      }

      //Monkey w/ lemma - not necessary, just an optimization
      uintmax_t nLemmaTop = pSmurfInfo->pLemma->size - (nNumBDDVariables - nNumVariables);
      assert(nLemmaTop < pSmurfInfo->nNumVariables);
      literal_undoer_push(&SM->pLemmaLiteral_undoer, &pSmurfInfo->pLemma->data[nLemmaTop]);
      literal_undoer_push(&SM->pLemmaLiteral_undoer, &pSmurfInfo->pLemma->data[nLemmaTop+1]);
      pSmurfInfo->pLemma->data[nLemmaTop] = (literal)nVbleIndex;

      d9_printf3("ANDEQUALC Assigning pLemma[%ju] = %jd because of tail->head\n", nLemmaTop, bPolarity?-(literal)pSmurfInfo->pIndex2Var[nVbleIndex]:(literal)pSmurfInfo->pIndex2Var[nVbleIndex]);
      pSmurfInfo->pLemma->size = nLemmaTop+1;
      
      uint8_t ret = EnqueueInference_hook(SM, pSmurfInfo, pANDEQUALSCOUNTERState->pANDEQUALSState->nLHS, !pANDEQUALSCOUNTERState->pANDEQUALSState->bAND_or_OR);
      if(ret != NO_ERROR) return ret;

      //Smurf is satisfied
      pNextState = (TypeStateEntry *)SM->pTrueSmurfState;
    }
  }

  pSmurfInfo->pCurrentState = pNextState;
  if(arrGetStateType[pNextState->type] == SM_TRANSITION_TYPE)
    return arrApplyInferenceToState[pNextState->type](SM, nVbleIndex, bPolarity, pSmurfInfo);

  return NO_ERROR;
}
