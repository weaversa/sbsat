#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

TypeStateEntry *CreateXORState(SmurfManager *SM, DdNode *pXORBDD) {
  XORStateEntry *pXORState = NULL;

  if((SM->uTempList1.nLength >= functionTypeLimits[FN_TYPE_XOR]) && isXOR(SM->dd, pXORBDD)) {
    Cudd_Ref(pXORBDD);
    
    uint8_t neg = Cudd_IsComplement(pXORBDD);
    uint8_t parity = neg == (SM->uTempList1.nLength&1);

    //Build XOR state
    uintmax_t nNumBDDVars = SM->uTempList1.nLength;
    uintmax_t nNumElts = SM->uTempList1.pList[nNumBDDVars-1]+1;
    assert(nNumElts > 0);

    uint8_t ret = check_SmurfStatesTableSize(SM, (uintmax_t)sizeof(XORStateEntry) + (sizeof(uint8_t)*nNumElts));
    if(ret != NO_ERROR) {assert(0); return NULL;}
    
    pXORState = (XORStateEntry *)SM->pSmurfStatesTableTail;
    chomp_SmurfStatesTable(SM, (uintmax_t)sizeof(XORStateEntry) + (sizeof(uint8_t) * nNumElts));
    sbsat_stats[STAT_XOR_STATES_BUILT]+=1;
    sbsat_stats[STAT_STATES_BUILT]+=1;
    
    pXORState->type          = FN_XOR;
    pXORState->visited       = 0;
    pXORState->nNumVariables = nNumElts;
    pXORState->bExists       = (uint8_t *)(pXORState+1);
    pXORState->bParity       = parity;
    pXORState->pXORBDD       = pXORBDD;

    uintmax_t nBDDVar = 0;
    for(uintmax_t nVbleIndex = 0; nVbleIndex < nNumElts; nVbleIndex++) {
      if(nVbleIndex == SM->uTempList1.pList[nBDDVar]) {
	//Variable is in the BDD
	pXORState->bExists[nVbleIndex] = 1;
	nBDDVar++;
      } else {
	//Variable is not in the BDD
	pXORState->bExists[nVbleIndex] = 0;
      }
    }
    assert(nBDDVar == nNumBDDVars);
  }

  //Leaf node, just return
  return (TypeStateEntry *)pXORState;
}
