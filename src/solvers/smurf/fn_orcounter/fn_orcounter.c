#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

// ORCOUNTER State

void initORCOUNTERStateType() {
  arrGetStateType[FN_ORCOUNTER] = SM_NODE_TYPE;

  arrCreateStateEntry[FN_ORCOUNTER] = CreateORCOUNTERState;
  arrStatesTypeSize[FN_ORCOUNTER] = ComputeORCOUNTERStateSize;
  arrSetStateVisitedFlag[FN_ORCOUNTER] = SetORCOUNTERStateVisitedFlag;
  arrUnsetStateVisitedFlag[FN_ORCOUNTER] = UnsetORCOUNTERStateVisitedFlag;
  arrCleanUpStateEntry[FN_ORCOUNTER] = CleanUpORCOUNTERState;
  arrApplyInferenceToState[FN_ORCOUNTER] = ApplyInferenceToORCOUNTER;
  arrPrintStateEntry[FN_ORCOUNTER] = PrintORCOUNTERStateEntry;
  arrPrintStateEntry_dot[FN_ORCOUNTER] = PrintORCOUNTERStateEntry_dot;
  arrFreeStateEntry[FN_ORCOUNTER] = FreeORCOUNTERStateEntry;
  arrCalculateStateHeuristic[FN_ORCOUNTER] = CalculateORCOUNTERStateLSGBHeuristic;
  //arrSetStateHeuristicScore[FN_ORCOUNTER] = LSGBORCOUNTERStateSetHeurScore;
  arrGetStateHeuristicScore[FN_ORCOUNTER] = LSGBORCOUNTERStateGetHeurScore;
  arrGetVarHeuristicScore[FN_ORCOUNTER] = LSGBORCOUNTERStateGetVarHeurScore;
  arrFreeHeuristicScore[FN_ORCOUNTER] = LSGBORCOUNTERStateFree;
}

uintmax_t ComputeORCOUNTERStateSize(TypeStateEntry *pTypeState) {
  ORCOUNTERStateEntry *pORCOUNTERState = (ORCOUNTERStateEntry *)pTypeState;
  assert(pORCOUNTERState->type == FN_ORCOUNTER);
  return sizeof(ORCOUNTERStateEntry);
}

void PrintORCOUNTERStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {
  ORCOUNTERStateEntry *pORCOUNTERState = (ORCOUNTERStateEntry *)pTypeState;
  assert(pORCOUNTERState->type == FN_ORCOUNTER);

  fprintf(stdout, "ORCOUNTER %p, nvars=%ju, next=%p, orstate=%p", (void *)pORCOUNTERState, pORCOUNTERState->nNumVariables, pORCOUNTERState->pTransition, pORCOUNTERState->pORState);
}

void PrintORCOUNTERStateEntry_dot(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t *pIndex2Var) {
  if(pTypeState->visited == 1) return;
  pTypeState->visited = 1;

  ORCOUNTERStateEntry *pORCOUNTERState = (ORCOUNTERStateEntry *)pTypeState;
  fprintf(stdout, " b%p->b%p [style=solid,label=\" x_i \"]\n", (void *)pORCOUNTERState, (void *)SM->pTrueSmurfState);
  fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{x_i} \"]\n", (void *)pORCOUNTERState, (void *)pORCOUNTERState->pTransition);
  fprintf(stdout, " b%p [shape=\"ellipse\",label=\"x_1 \\vee .. \\vee x_%ju \"]\n", (void *)pORCOUNTERState, pORCOUNTERState->nNumVariables);

  pTypeState = pORCOUNTERState->pTransition;
  arrPrintStateEntry_dot[pTypeState->type](SM, pTypeState, pIndex2Var);
}

void FreeORCOUNTERStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {

}
