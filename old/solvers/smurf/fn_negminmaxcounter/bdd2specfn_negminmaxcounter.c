#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

//precondition is that SM->uTempList1 contains the support of
//pNEGMINMAXCOUNTERBDD, which is true if ReadSmurfStateIntoTable was the caller
TypeStateEntry *CreateNEGMINMAXCOUNTERState(SmurfManager *SM, DdNode *pNEGMINMAXCOUNTERBDD) {
  NEGMINMAXCOUNTERStateEntry *pNEGMINMAXCOUNTERState = NULL;
  
  //Build NEGMINMAX state
  NEGMINMAXStateEntry *pNEGMINMAXState = (NEGMINMAXStateEntry *)CreateNEGMINMAXState(SM, pNEGMINMAXCOUNTERBDD);
  if(pNEGMINMAXState) {
    //Build a lattice of NEGMINMAXCOUNTER state

    //Init some local variables
    intmax_t nNumVariables = pNEGMINMAXState->nNumBDDVariables;
    intmax_t min = pNEGMINMAXState->nMin;
    intmax_t max = pNEGMINMAXState->nMax;

    NEGMINMAXCOUNTERStateEntry *pMMHead = (NEGMINMAXCOUNTERStateEntry *)SM->pTrueSmurfState;

    //Build the lines
    intmax_t nNumTrue, nNumVarsLeft;
    for(nNumTrue = max; nNumTrue >= 0; nNumTrue--) { //NumTrue
      NEGMINMAXCOUNTERStateEntry *pMMNext = (NEGMINMAXCOUNTERStateEntry *)SM->pTrueSmurfState;
      for(nNumVarsLeft = min-nNumTrue; nNumVarsLeft <= nNumVariables-nNumTrue; nNumVarsLeft++) {
	if(nNumVarsLeft < 2) continue;
	if(((nNumVarsLeft + nNumTrue) <= max) && (nNumTrue >= min-1)) continue; //over the min and can't make it over max
	if((nNumTrue >= min) && ((nNumTrue + nNumVarsLeft) <= max+1)) continue; //under the max and can't make it under the min
	if((nNumTrue+nNumVarsLeft) < min) continue; //stuck under the min

	uint8_t ret = check_SmurfStatesTableSize(SM, (uintmax_t)sizeof(NEGMINMAXCOUNTERStateEntry));
	if(ret != NO_ERROR) {assert(0); return NULL;}
	
	NEGMINMAXCOUNTERStateEntry *pNewState = (NEGMINMAXCOUNTERStateEntry *)SM->pSmurfStatesTableTail;
	chomp_SmurfStatesTable(SM, (uintmax_t)sizeof(NEGMINMAXCOUNTERStateEntry));
	sbsat_stats[STAT_NEGMINMAX_STATES_BUILT]+=1;
	sbsat_stats[STAT_STATES_BUILT]+=1;
	
	pNewState->type                  = FN_NEGMINMAXCOUNTER;
	pNewState->visited               = 0;
	pNewState->nNumVariables         = nNumVarsLeft;
	pNewState->nNumTrue              = nNumTrue;
	pNewState->pVarIsFalseTransition = (TypeStateEntry *)pMMNext;
	pNewState->pVarIsTrueTransition  = (TypeStateEntry *)SM->pTrueSmurfState;
	pNewState->pNEGMINMAXState          = pNEGMINMAXState; 
	
	pMMNext = pNewState;
      }
      if(pMMNext != (NEGMINMAXCOUNTERStateEntry *)SM->pTrueSmurfState) {
	pMMNext->pVarIsTrueTransition = (TypeStateEntry *)pMMHead;
	pMMHead = pMMNext;
      }
    }

    pNEGMINMAXCOUNTERState = pMMHead;
    
    //Zip the lines together    
    for(; pMMHead != (NEGMINMAXCOUNTERStateEntry *)SM->pTrueSmurfState; pMMHead = (NEGMINMAXCOUNTERStateEntry *)pMMHead->pVarIsTrueTransition) {
      NEGMINMAXCOUNTERStateEntry *pMMLine1 = (NEGMINMAXCOUNTERStateEntry *)pMMHead->pVarIsFalseTransition;
      NEGMINMAXCOUNTERStateEntry *pMMLine2 = (NEGMINMAXCOUNTERStateEntry *)pMMHead->pVarIsTrueTransition;
      if(pMMLine2 == (NEGMINMAXCOUNTERStateEntry *)SM->pTrueSmurfState) break;
      pMMLine2 = (NEGMINMAXCOUNTERStateEntry *)pMMLine2->pVarIsFalseTransition;
      while((pMMLine1 != (NEGMINMAXCOUNTERStateEntry *)SM->pTrueSmurfState) &&
	    (pMMLine2 != (NEGMINMAXCOUNTERStateEntry *)SM->pTrueSmurfState)) {
	//	assert(pMMLine1->nNumVariables > pMMLine2->nNumVariables);
	pMMLine1->pVarIsTrueTransition = (TypeStateEntry *)pMMLine2;
	pMMLine1 = (NEGMINMAXCOUNTERStateEntry *)pMMLine1->pVarIsFalseTransition;
	pMMLine2 = (NEGMINMAXCOUNTERStateEntry *)pMMLine2->pVarIsFalseTransition;
      }
    }

  }
  
  //Leaf node, just return
  return (TypeStateEntry *)pNEGMINMAXCOUNTERState;
}
