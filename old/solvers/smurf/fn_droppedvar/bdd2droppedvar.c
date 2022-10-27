#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

DroppedVarStateEntry *CreateOneDroppedVarState(SmurfManager *SM, uintmax_t nVar) {
  uint8_t ret = check_SmurfStatesTableSize(SM, (uintmax_t)sizeof(DroppedVarStateEntry));
  if(ret != NO_ERROR) {assert(0); return NULL;}
  
  DroppedVarStateEntry *pNewState = (DroppedVarStateEntry *)SM->pSmurfStatesTableTail;
  chomp_SmurfStatesTable(SM, (uintmax_t)sizeof(DroppedVarStateEntry));
  sbsat_stats[STAT_DROPPEDVAR_STATES_BUILT]+=1;
  sbsat_stats[STAT_STATES_BUILT]+=1;

  pNewState->type           = FN_DROPPEDVAR;
  pNewState->visited        = 0;
  pNewState->nVar           = nVar;
  pNewState->pTransition    = NULL;
  
  return pNewState;
}

TypeStateEntry *CreateDroppedVarState(SmurfManager *SM, DdNode *pOldBDD, DdNode *pNewBDD) {
  if(Cudd_Regular(pNewBDD) == DD_ONE(SM->dd)) return (TypeStateEntry *)SM->pTrueSmurfState;

  DroppedVarStateEntry *pHeadState = NULL;
  DroppedVarStateEntry *pTailState = NULL;

  //Create support for old bdd.
  BDD_Support(&SM->uTempList1.nLength, &(SM->uTempList1.nLength_max), &(SM->uTempList1.pList), pOldBDD);
  qsort(SM->uTempList1.pList, SM->uTempList1.nLength, sizeof(uintmax_t), compfunc);
  uintmax_t *pOldSupport = SM->uTempList1.pList;
  uintmax_t nNumOldElts = SM->uTempList1.nLength;

  //Create support for new bdd.
  BDD_Support(&SM->uTempList2.nLength, &(SM->uTempList2.nLength_max), &(SM->uTempList2.pList), pNewBDD);
  qsort(SM->uTempList2.pList, SM->uTempList2.nLength, sizeof(uintmax_t), compfunc);
  uintmax_t *pNewSupport = SM->uTempList2.pList;
  uintmax_t nNumNewElts = SM->uTempList2.nLength;

  assert(nNumOldElts > nNumNewElts); //Expecting variables to be removed, never added.
  assert(nNumNewElts > 0);

  uintmax_t tail = nNumOldElts-1;

  intmax_t i_new = (intmax_t) nNumNewElts-1;
  intmax_t i_old = (intmax_t) nNumOldElts-1;
  for(; i_old >= 0; i_old--) {
    if(i_new < 0 || pOldSupport[i_old] != pNewSupport[i_new]){
      //lost variable pOldSupport[i_old] - create a dropped state
      uintmax_t droppedvar;
      //if(compress_smurfs) droppedvar = i_old;
      //else
      droppedvar = pOldSupport[i_old];
      DroppedVarStateEntry *pNewState = CreateOneDroppedVarState(SM, droppedvar);
      if(pTailState == NULL) {
	pHeadState = pTailState = pNewState;
      } else {
	pTailState->pTransition = (TypeStateEntry *)pNewState;
	pTailState = pNewState;
      }
      pOldSupport[i_old] = pOldSupport[tail];
      tail--;
    } else {
      i_new--;
    }
  }

  assert(pHeadState != NULL);
  assert(pTailState != NULL);
  
  //Compression
  if(0) {
    for(uintmax_t i = 0; i < nNumNewElts; i++) {
      if(1) {
	uint32_t_list_add(&SM->uTempList32, pNewSupport[i], i);
      } else {
	uint32_t_list_add(&SM->uTempList32, pOldSupport[i], i);
      }
    }

    //DdNode *pNewBDD2 = Cudd_bddPermute(SM->dd, pNewBDD, (int *)SM->uTempList32.pList);
    //Cudd_Ref(pNewBDD2);

    //pNewBDD = Mitosis(SM->dd, pNewBDD, SM->uTempList32.pList); //faster - I forget why I commented this out - it's probably wrong somehow
    pNewBDD = Cudd_bddPermute(SM->dd, pNewBDD, (int *)SM->uTempList32.pList);
    Cudd_Ref(pNewBDD); //iterDeref before returning

    //assert(pNewBDD2 == pNewBDD);
    //Cudd_IterDerefBdd(SM->dd, pNewBDD2);

  }

  pTailState->pTransition = ReadSmurfStateIntoTable(SM, pNewBDD);

  //Compression
  if(0) Cudd_IterDerefBdd(SM->dd, pNewBDD);

  return (TypeStateEntry *)pHeadState;
}
