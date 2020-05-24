#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

void SetANDEQUALSStateVisitedFlag(TypeStateEntry *pTypeState) {
  ANDEQUALSStateEntry *pANDEQUALSState = (ANDEQUALSStateEntry *)pTypeState;
  assert(pANDEQUALSState->type == FN_ANDEQUALS);

  if(pANDEQUALSState->visited==0) {
    pANDEQUALSState->visited = 1;

    arrSetStateVisitedFlag[pANDEQUALSState->pORState->type]((TypeStateEntry *)pANDEQUALSState->pORState);
  }
}

void UnsetANDEQUALSStateVisitedFlag(TypeStateEntry *pTypeState) {
  ANDEQUALSStateEntry *pANDEQUALSState = (ANDEQUALSStateEntry *)pTypeState;
  assert(pANDEQUALSState->type == FN_ANDEQUALS);

  if(pANDEQUALSState->visited==1) {
    pANDEQUALSState->visited = 0;

    arrUnsetStateVisitedFlag[pANDEQUALSState->pORState->type]((TypeStateEntry *)pANDEQUALSState->pORState);
  }
}

void CleanUpANDEQUALSState(TypeStateEntry *pTypeState) {
  ANDEQUALSStateEntry *pANDEQUALSState = (ANDEQUALSStateEntry *)pTypeState;
  assert(pANDEQUALSState->type == FN_ANDEQUALS);
}

uint8_t ApplyInferenceToANDEQUALS(SmurfManager *SM, uintmax_t nVbleIndex, uint8_t bPolarity, SmurfInfoStruct *pSmurfInfo) {
  ANDEQUALSStateEntry *pANDEQUALSState = (ANDEQUALSStateEntry *)pSmurfInfo->pCurrentState;
  assert(pANDEQUALSState->type == FN_ANDEQUALS);
  //This state is basically an EQU or XOR, depending on bAND_or_OR.

  if(nVbleIndex == pANDEQUALSState->nLHS) {
    //Setting the head
    if(pANDEQUALSState->bAND_or_OR == bPolarity) {
      //Monkey w/ lemma - not necessary, just an optimization
      uintmax_t nLemmaTop = pSmurfInfo->pLemma->size - (pANDEQUALSState->pORState->nNumBDDVariables - 1);
      literal_undoer_push(&SM->pLemmaLiteral_undoer, &pSmurfInfo->pLemma->data[nLemmaTop]);
      literal_undoer_push(&SM->pLemmaLiteral_undoer, &pSmurfInfo->pLemma->data[nLemmaTop+1]);
      pSmurfInfo->pLemma->data[nLemmaTop] = (literal)nVbleIndex;
      
      d9_printf3("ANDEQUAL Assigning pLemma[%ju] = %jd because of head\n", nLemmaTop, bPolarity?-(literal)pSmurfInfo->pIndex2Var[nVbleIndex]:(literal)pSmurfInfo->pIndex2Var[nVbleIndex]);
      
      pSmurfInfo->pLemma->size = nLemmaTop+1;
    } else {
      clausePush(pSmurfInfo->pLemma, (literal)nVbleIndex);
    }      

    uint8_t *barrPolarity = pANDEQUALSState->pORState->bPolarity;
    for(uintmax_t i = 0; 1; i++) {
      uint8_t _bPolarity = barrPolarity[i];
      if(_bPolarity!=2 && SM->pTrail[pSmurfInfo->pIndex2Var[i]]==0) {
	uint8_t ret = EnqueueInference_hook(SM, pSmurfInfo, i, _bPolarity ^ (pANDEQUALSState->bAND_or_OR == bPolarity));
	if(ret != NO_ERROR) return ret;
	break; //Should only be one variable
      }
    }
    //Smurf is satisfied
    pSmurfInfo->pCurrentState = (TypeStateEntry *)SM->pTrueSmurfState;
  } else {
    uintmax_t nNumVariables = pANDEQUALSState->pORState->nNumVariables;
    if(nNumVariables <= nVbleIndex)
      return NO_ERROR; //Var not in Smurf
    
    uint8_t _bPolarity = pANDEQUALSState->pORState->bPolarity[nVbleIndex];
    if(_bPolarity == 2)
      return NO_ERROR; //Var not in Smurf
    
    //Infer head

    if(bPolarity != _bPolarity) {
      clausePush(pSmurfInfo->pLemma, (literal)nVbleIndex);
    } else {    
      //Monkey w/ lemma - not necessary, just an optimization
      uintmax_t nLemmaTop = pSmurfInfo->pLemma->size - (pANDEQUALSState->pORState->nNumBDDVariables - 1);
      literal_undoer_push(&SM->pLemmaLiteral_undoer, &pSmurfInfo->pLemma->data[nLemmaTop]);
      literal_undoer_push(&SM->pLemmaLiteral_undoer, &pSmurfInfo->pLemma->data[nLemmaTop+1]);
      pSmurfInfo->pLemma->data[nLemmaTop] = (literal)nVbleIndex;

      d9_printf3("ANDEQUAL Assigning pLemma[%ju] = %jd because of tail\n", nLemmaTop, bPolarity?-(literal)pSmurfInfo->pIndex2Var[nVbleIndex]:(literal)pSmurfInfo->pIndex2Var[nVbleIndex]);
      
      pSmurfInfo->pLemma->size = nLemmaTop+1;
    }
    
    uint8_t ret = EnqueueInference_hook(SM, pSmurfInfo, pANDEQUALSState->nLHS, pANDEQUALSState->bAND_or_OR ^ (bPolarity==_bPolarity));
    if(ret != NO_ERROR) return ret;
    
    //Smurf is satisfied
    pSmurfInfo->pCurrentState = (TypeStateEntry *)SM->pTrueSmurfState;
  }

  return NO_ERROR;
}


uint8_t ApplyInferenceToANDEQUALS_gelim(SmurfManager *SM, uintmax_t nVbleIndex, uint8_t bPolarity, SmurfInfoStruct *pSmurfInfo) {
  ANDEQUALSStateEntry *pANDEQUALSState = (ANDEQUALSStateEntry *)pSmurfInfo->pCurrentState;
  assert(pANDEQUALSState->type == FN_ANDEQUALS);
  //This state is basically an EQU or XOR, depending on bAND_or_OR.
  
  //Could monkey w/ lemma - not necessary, just an optimization SEAN!!!???
  
  uint8_t *barrPolarity = pANDEQUALSState->pORState->bPolarity;
  uintmax_t i;
  for(i = 0; 1; i++) {
    uint8_t _bPolarity = barrPolarity[i];
    if(_bPolarity!=2 && SM->pTrail[pSmurfInfo->pIndex2Var[i]]==0) {
      break; //Should only be one variable
    }
  }
  SM->uTempList1.pList[0] = pSmurfInfo->pIndex2Var[i];
  SM->uTempList1.pList[1] = pSmurfInfo->pIndex2Var[pANDEQUALSState->nLHS];

  uint8_t ret = addRowXORGElimTable(SM->XM[SM->nCurrentDecisionLevel], 2, SM->uTempList1.pList, pANDEQUALSState->bAND_or_OR ^ barrPolarity[i]);
  if(ret != NO_ERROR) return ret;
  
  //Smurf is satisfied
  pSmurfInfo->pCurrentState = (TypeStateEntry *)SM->pTrueSmurfState;

  return NO_ERROR;
}
