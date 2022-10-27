#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

// NEGMINMAX State

void initNEGMINMAXStateType() {
  arrCreateStateEntry[FN_NEGMINMAX] = CreateNEGMINMAXState;
  arrStatesTypeSize[FN_NEGMINMAX] = ComputeNEGMINMAXStateSize;
  arrSetStateVisitedFlag[FN_NEGMINMAX] = SetNEGMINMAXStateVisitedFlag;
  arrUnsetStateVisitedFlag[FN_NEGMINMAX] = UnsetNEGMINMAXStateVisitedFlag;
  arrCleanUpStateEntry[FN_NEGMINMAX] = CleanUpNEGMINMAXState;

  //arrApplyInferenceToState[FN_NEGMINMAX] = ApplyInferenceToNEGMINMAX;

  arrPrintStateEntry[FN_NEGMINMAX] = PrintNEGMINMAXStateEntry;
  arrPrintStateEntry_dot[FN_NEGMINMAX] = PrintNEGMINMAXStateEntry_dot;
  
  arrFreeStateEntry[FN_NEGMINMAX] = FreeNEGMINMAXStateEntry;

  //arrCalculateStateHeuristic[FN_NEGMINMAX] = CalculateNEGMINMAXLSGBHeuristic;
  //arrSetStateHeuristicScore[FN_NEGMINMAX] = LSGBNEGMINMAXStateSetHeurScore;
  //arrGetStateHeuristicScore[FN_NEGMINMAX] = LSGBNEGMINMAXStateGetHeurScore;
}

uintmax_t ComputeNEGMINMAXStateSize(TypeStateEntry *pTypeState) {
  NEGMINMAXStateEntry *pNEGMINMAXState = (NEGMINMAXStateEntry *)pTypeState;
  assert(pNEGMINMAXState->type == FN_NEGMINMAX);
  return sizeof(NEGMINMAXStateEntry) + (sizeof(uint8_t) * pNEGMINMAXState->nNumVariables);
}

void PrintNEGMINMAXStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {
  NEGMINMAXStateEntry *pNEGMINMAXState = (NEGMINMAXStateEntry *)pTypeState;
  assert(pNEGMINMAXState->type == FN_NEGMINMAX);

  /*
  fprintf(stdout, "NEGMINMAX %p, nvars=%u, pol=", (void *)pNEGMINMAXState, pNEGMINMAXState->nNumVariables);
  for(uint32_t i = 0; i < pNEGMINMAXState->nNumVariables; i++)
    fprintf(stdout, "%u ", pNEGMINMAXState->bPolarity[i]);
  fprintf(stdout, "\b, refs=%d\n", Cudd_Regular(pNEGMINMAXState->pNEGMINMAXBDD)->ref);
  */
}

void PrintNEGMINMAXStateEntry_dot(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t *pIndex2Var) {
  if(pTypeState->visited == 1) return;
  pTypeState->visited = 1;
}

void FreeNEGMINMAXStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {
  DdNode *pNEGMINMAXBDD = ((NEGMINMAXStateEntry *)pTypeState)->pNEGMINMAXBDD;
  if(pNEGMINMAXBDD != NULL) {
    if(Cudd_IsComplement(pNEGMINMAXBDD)) {
      Cudd_Not(pNEGMINMAXBDD)->pStateNeg = NULL;
    } else {
      pNEGMINMAXBDD->pStatePos = NULL;
    }
    Cudd_IterDerefBdd(SM->dd, pNEGMINMAXBDD);
  }
}
