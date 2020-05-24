#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

//precondition is that SM->uTempList1 contains the support of
//pMINMAXCOUNTERBDD, which is true if ReadSmurfStateIntoTable was the caller
TypeStateEntry *CreateMINMAXCOUNTERState(SmurfManager *SM, DdNode *pMINMAXCOUNTERBDD) {
  MINMAXCOUNTERStateEntry *pMINMAXCOUNTERState = NULL;
  
  //Build MINMAX state
  MINMAXStateEntry *pMINMAXState = (MINMAXStateEntry *)CreateMINMAXState(SM, pMINMAXCOUNTERBDD);
  if(pMINMAXState) {
    //Build a lattice of MINMAXCOUNTER state

    //Init some local variables
    intmax_t nNumVariables = pMINMAXState->nNumBDDVariables;
    intmax_t min = pMINMAXState->nMin;
    intmax_t max = pMINMAXState->nMax;

    MINMAXCOUNTERStateEntry *pMMHead = (MINMAXCOUNTERStateEntry *)SM->pTrueSmurfState;

    //Build the lines
    intmax_t nNumTrue, nNumVarsLeft;
    for(nNumTrue = max; nNumTrue >= 0; nNumTrue--) {
      MINMAXCOUNTERStateEntry *pMMNext = (MINMAXCOUNTERStateEntry *)SM->pTrueSmurfState;
      for(nNumVarsLeft = min-nNumTrue; nNumVarsLeft <= nNumVariables-nNumTrue; nNumVarsLeft++) {
	if(nNumVarsLeft < 2) continue;
	if((nNumTrue + nNumVarsLeft) <= min) continue; //under the min and can't make it over
	if(nNumTrue >= max) continue; //over the max
	if((nNumTrue >= min) && ((nNumTrue + nNumVarsLeft) <= max)) continue; //stuck in bounds

	uint8_t ret = check_SmurfStatesTableSize(SM, (uintmax_t)sizeof(MINMAXCOUNTERStateEntry));
	if(ret != NO_ERROR) {assert(0); return NULL;}
	
	MINMAXCOUNTERStateEntry *pNewState = (MINMAXCOUNTERStateEntry *)SM->pSmurfStatesTableTail;
	chomp_SmurfStatesTable(SM, (uintmax_t)sizeof(MINMAXCOUNTERStateEntry));
	sbsat_stats[STAT_MINMAX_STATES_BUILT]+=1;
	sbsat_stats[STAT_STATES_BUILT]+=1;
	
	pNewState->type                  = FN_MINMAXCOUNTER;
	pNewState->visited               = 0;
	pNewState->nNumVariables         = nNumVarsLeft;
	pNewState->nNumTrue              = nNumTrue;
	pNewState->pVarIsFalseTransition = (TypeStateEntry *)pMMNext;
	pNewState->pVarIsTrueTransition  = (TypeStateEntry *)SM->pTrueSmurfState;
	pNewState->pMINMAXState          = pMINMAXState; 
	
	pMMNext = pNewState;
      }
      if(pMMNext != (MINMAXCOUNTERStateEntry *)SM->pTrueSmurfState) {
	pMMNext->pVarIsTrueTransition = (TypeStateEntry *)pMMHead;
	pMMHead = pMMNext;
      }
    }

    pMINMAXCOUNTERState = pMMHead;
    
    //Zip the lines together    
    for(; pMMHead != (MINMAXCOUNTERStateEntry *)SM->pTrueSmurfState; pMMHead = (MINMAXCOUNTERStateEntry *)pMMHead->pVarIsTrueTransition) {
      MINMAXCOUNTERStateEntry *pMMLine1 = (MINMAXCOUNTERStateEntry *)pMMHead->pVarIsFalseTransition;
      MINMAXCOUNTERStateEntry *pMMLine2 = (MINMAXCOUNTERStateEntry *)pMMHead->pVarIsTrueTransition;
      if(pMMLine2 == (MINMAXCOUNTERStateEntry *)SM->pTrueSmurfState) break;
      pMMLine2 = (MINMAXCOUNTERStateEntry *)pMMLine2->pVarIsFalseTransition;
      while((pMMLine1 != (MINMAXCOUNTERStateEntry *)SM->pTrueSmurfState) &&
	    (pMMLine2 != (MINMAXCOUNTERStateEntry *)SM->pTrueSmurfState)) {
	pMMLine1->pVarIsTrueTransition = (TypeStateEntry *)pMMLine2;
	pMMLine1 = (MINMAXCOUNTERStateEntry *)pMMLine1->pVarIsFalseTransition;
	pMMLine2 = (MINMAXCOUNTERStateEntry *)pMMLine2->pVarIsFalseTransition;
      }
    }

  }
  
  //Leaf node, just return
  return (TypeStateEntry *)pMINMAXCOUNTERState;
}
