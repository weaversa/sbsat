#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

TypeStateEntry *CreateInferenceState(SmurfManager *SM, DdNode *pInferenceBDD) {
  InferenceStateEntry *pInferenceState = NULL;

  DdNode *pInferenceCube = Cudd_FindEssential(SM->dd, pInferenceBDD);
  assert(pInferenceCube != NULL);
  Cudd_Ref(pInferenceCube);
  if(pInferenceCube != DD_ONE(SM->dd)) {
    Cudd_Ref(pInferenceBDD);
    
    uint8_t ret = check_SmurfStatesTableSize(SM, (uintmax_t)sizeof(InferenceStateEntry));
    if(ret != NO_ERROR) {assert(0); return NULL;}
    
    pInferenceState = (InferenceStateEntry *)SM->pSmurfStatesTableTail;
    chomp_SmurfStatesTable(SM, (uintmax_t)sizeof(InferenceStateEntry));
    sbsat_stats[STAT_INFERENCE_STATES_BUILT]+=1;
    sbsat_stats[STAT_STATES_BUILT]+=1;
    
    pInferenceState->type          = FN_INFERENCE;
    pInferenceState->visited       = 0;
    pInferenceState->pInferenceBDD = pInferenceBDD;
    
    uint8_t neg = Cudd_IsComplement(pInferenceCube);
    DdNode* pInfCubeRegularBDD = Cudd_Regular(pInferenceCube);
    pInferenceState->nTransitionVar = (uintmax_t)pInfCubeRegularBDD->index;
    //Move to the next precomputed inference by traversing away from False nodes
    DdNode *pNextInferenceCube;
    if(cuddT(pInfCubeRegularBDD) == Cudd_NotCond(DD_ONE(SM->dd), !neg)) {
      pNextInferenceCube = Cudd_NotCond(cuddE(pInfCubeRegularBDD), neg);
      pInferenceState->bTransitionPol = 0;
    } else {
      pNextInferenceCube = Cudd_NotCond(cuddT(pInfCubeRegularBDD), neg);
      pInferenceState->bTransitionPol = 1;
    }
 
    assert(pNextInferenceCube != Cudd_Not(DD_ONE(SM->dd)));
    
    DdNode *pNextBDD = Cudd_Cofactor(SM->dd, pInferenceBDD, Cudd_NotCond(Cudd_bddIthVar(SM->dd, pInfCubeRegularBDD->index), !pInferenceState->bTransitionPol));
    Cudd_Ref(pNextBDD);
    pInferenceState->pTransition = ReadSmurfStateIntoTable(SM, pNextBDD);
    Cudd_IterDerefBdd(SM->dd, pNextBDD);
  }
  
  Cudd_IterDerefBdd(SM->dd, pInferenceCube);

  return (TypeStateEntry *)pInferenceState;
}
