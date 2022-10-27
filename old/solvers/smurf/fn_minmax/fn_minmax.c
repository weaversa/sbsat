#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

// MINMAX State

void initMINMAXStateType() {
  arrCreateStateEntry[FN_MINMAX] = CreateMINMAXState;
  arrStatesTypeSize[FN_MINMAX] = ComputeMINMAXStateSize;
  arrSetStateVisitedFlag[FN_MINMAX] = SetMINMAXStateVisitedFlag;
  arrUnsetStateVisitedFlag[FN_MINMAX] = UnsetMINMAXStateVisitedFlag;
  arrCleanUpStateEntry[FN_MINMAX] = CleanUpMINMAXState;

  //arrApplyInferenceToState[FN_MINMAX] = ApplyInferenceToMINMAX;

  arrPrintStateEntry[FN_MINMAX] = PrintMINMAXStateEntry;
  arrPrintStateEntry_dot[FN_MINMAX] = PrintMINMAXStateEntry_dot;
  
  arrFreeStateEntry[FN_MINMAX] = FreeMINMAXStateEntry;

  //arrCalculateStateHeuristic[FN_MINMAX] = CalculateMINMAXLSGBHeuristic;
  //arrSetStateHeuristicScore[FN_MINMAX] = LSGBMINMAXStateSetHeurScore;
  //arrGetStateHeuristicScore[FN_MINMAX] = LSGBMINMAXStateGetHeurScore;
}

uintmax_t ComputeMINMAXStateSize(TypeStateEntry *pTypeState) {
  MINMAXStateEntry *pMINMAXState = (MINMAXStateEntry *)pTypeState;
  assert(pMINMAXState->type == FN_MINMAX);
  return sizeof(MINMAXStateEntry) + (sizeof(uint8_t) * pMINMAXState->nNumVariables);
}

void PrintMINMAXStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {
  MINMAXStateEntry *pMINMAXState = (MINMAXStateEntry *)pTypeState;
  assert(pMINMAXState->type == FN_MINMAX);

  /*
  fprintf(stdout, "MINMAX %p, nvars=%u, pol=", (void *)pMINMAXState, pMINMAXState->nNumVariables);
  for(uint32_t i = 0; i < pMINMAXState->nNumVariables; i++)
    fprintf(stdout, "%u ", pMINMAXState->bPolarity[i]);
  fprintf(stdout, "\b, refs=%d\n", Cudd_Regular(pMINMAXState->pMINMAXBDD)->ref);
  */
}

void PrintMINMAXStateEntry_dot(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t *pIndex2Var) {
  if(pTypeState->visited == 1) return;
  pTypeState->visited = 1;
}

void FreeMINMAXStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {
  DdNode *pMINMAXBDD = ((MINMAXStateEntry *)pTypeState)->pMINMAXBDD;
  if(pMINMAXBDD != NULL) {
    if(Cudd_IsComplement(pMINMAXBDD)) {
      Cudd_Not(pMINMAXBDD)->pStateNeg = NULL;
    } else {
      pMINMAXBDD->pStatePos = NULL;
    }
    Cudd_IterDerefBdd(SM->dd, pMINMAXBDD);
  }
}
