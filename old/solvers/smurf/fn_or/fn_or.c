#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

// OR State

void initORStateType() {
  arrGetStateType[FN_OR] = SM_NODE_TYPE;

  arrCreateStateEntry[FN_OR] = CreateORState;
  arrStatesTypeSize[FN_OR] = ComputeORStateSize;
  arrSetStateVisitedFlag[FN_OR] = SetORStateVisitedFlag;
  arrUnsetStateVisitedFlag[FN_OR] = UnsetORStateVisitedFlag;
  arrCleanUpStateEntry[FN_OR] = CleanUpORState;
  arrApplyInferenceToState[FN_OR] = ApplyInferenceToOR;
  arrPrintStateEntry[FN_OR] = PrintORStateEntry;
  arrPrintStateEntry_dot[FN_OR] = PrintORStateEntry_dot;
  arrFreeStateEntry[FN_OR] = FreeORStateEntry;
  arrCalculateStateHeuristic[FN_OR] = CalculateORStateLSGBHeuristic;
  //arrSetStateHeuristicScore[FN_OR] = LSGBORStateSetHeurScore;
  arrGetStateHeuristicScore[FN_OR] = LSGBORStateGetHeurScore;
  arrGetVarHeuristicScore[FN_OR] = LSGBORStateGetVarHeurScore;
  arrFreeHeuristicScore[FN_OR] = LSGBORStateFree;
}

uintmax_t ComputeORStateSize(TypeStateEntry *pTypeState) {
  ORStateEntry *pORState = (ORStateEntry *)pTypeState;
  assert(pORState->type == FN_OR);
  return sizeof(ORStateEntry) + (sizeof(uint8_t) * pORState->nNumVariables);
}

void PrintORStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {
  ORStateEntry *pORState = (ORStateEntry *)pTypeState;
  assert(pORState->type == FN_OR);

  fprintf(stdout, "OR %p, nvars=%ju, pol=", (void *)pORState, pORState->nNumVariables);
  for(uint32_t i = 0; i < pORState->nNumVariables; i++)
    fprintf(stdout, "%u ", pORState->bPolarity[i]);
  fprintf(stdout, "\b, refs=%d\n", Cudd_Regular(pORState->pORBDD)->ref);
}

void PrintORStateEntry_dot(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t *pIndex2Var) {
  if(pTypeState->visited == 1) return;
  pTypeState->visited = 1;

  ORStateEntry *pORState = (ORStateEntry *)pTypeState;
  fprintf(stdout, " b%p->b%p [style=solid,label=\" x_1 \"]\n", (void *)pORState, (void *)SM->pTrueSmurfState);
  fprintf(stdout, " b%p->b%p [style=solid,label=\" x_2 \"]\n", (void *)pORState, (void *)SM->pTrueSmurfState);
  fprintf(stdout, " b%p->b%p [style=solid,label=\" \\overline{x_1} : x_2 \"]\n", (void *)pORState, (void *)SM->pTrueSmurfState);
  fprintf(stdout, " b%p->b%p [style=solid,label=\" \\overline{x_2} : x_1 \"]\n", (void *)pORState, (void *)SM->pTrueSmurfState);
  fprintf(stdout, " b%p [shape=\"ellipse\",label=\"x_1 \\vee x_2\"]\n", (void *)pORState);
}

void FreeORStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {
  DdNode *pORBDD = ((ORStateEntry *)pTypeState)->pORBDD;
  if(pORBDD != NULL) {
    if(Cudd_IsComplement(pORBDD)) {
      Cudd_Not(pORBDD)->pStateNeg = NULL;
    } else {
      pORBDD->pStatePos = NULL;
    }
    Cudd_IterDerefBdd(SM->dd, pORBDD);
  }
}
