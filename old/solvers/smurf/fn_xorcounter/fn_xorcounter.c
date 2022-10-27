#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

// XORCOUNTER State

void initXORCOUNTERStateType() {
  if(gelim_smurfs) arrGetStateType[FN_XORCOUNTER] = SM_TRANSITION_TYPE;
  else arrGetStateType[FN_XORCOUNTER] = SM_NODE_TYPE;

  arrCreateStateEntry[FN_XORCOUNTER] = CreateXORCOUNTERState;
  arrStatesTypeSize[FN_XORCOUNTER] = ComputeXORCOUNTERStateSize;
  arrSetStateVisitedFlag[FN_XORCOUNTER] = SetXORCOUNTERStateVisitedFlag;
  arrUnsetStateVisitedFlag[FN_XORCOUNTER] = UnsetXORCOUNTERStateVisitedFlag;
  arrCleanUpStateEntry[FN_XORCOUNTER] = CleanUpXORCOUNTERState;

  if(gelim_smurfs) arrApplyInferenceToState[FN_XORCOUNTER] = ApplyInferenceToXORCOUNTER_gelim;
  else arrApplyInferenceToState[FN_XORCOUNTER] = ApplyInferenceToXORCOUNTER;

  arrPrintStateEntry[FN_XORCOUNTER] = PrintXORCOUNTERStateEntry;
  arrPrintStateEntry_dot[FN_XORCOUNTER] = PrintXORCOUNTERStateEntry_dot;
  arrFreeStateEntry[FN_XORCOUNTER] = FreeXORCOUNTERStateEntry;
  arrCalculateStateHeuristic[FN_XORCOUNTER] = CalculateXORCOUNTERStateLSGBHeuristic;
  //arrSetStateHeuristicScore[FN_XORCOUNTER] = LSGBXORCOUNTERStateSetHeurScore;
  arrGetStateHeuristicScore[FN_XORCOUNTER] = LSGBXORCOUNTERStateGetHeurScore;
  arrGetVarHeuristicScore[FN_XORCOUNTER] = LSGBXORCOUNTERStateGetVarHeurScore;
  arrFreeHeuristicScore[FN_XORCOUNTER] = LSGBXORCOUNTERStateFree;
}

uintmax_t ComputeXORCOUNTERStateSize(TypeStateEntry *pTypeState) {
  XORCOUNTERStateEntry *pXORCOUNTERState = (XORCOUNTERStateEntry *)pTypeState;
  assert(pXORCOUNTERState->type == FN_XORCOUNTER);
  return sizeof(XORCOUNTERStateEntry);
}

void PrintXORCOUNTERStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {
  XORCOUNTERStateEntry *pXORCOUNTERState = (XORCOUNTERStateEntry *)pTypeState;
  assert(pXORCOUNTERState->type == FN_XORCOUNTER);

  d2_printf5("XORCOUNTER %p, nvars=%ju, T=%p, F=%p\n", (void *)pXORCOUNTERState, pXORCOUNTERState->nNumVariables, pXORCOUNTERState->pVarIsTrueTransition, pXORCOUNTERState->pVarIsFalseTransition);
}

void PrintXORCOUNTERStateEntry_dot(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t *pIndex2Var) {
  if(pTypeState->visited == 1) return;
  pTypeState->visited = 1;

  XORCOUNTERStateEntry *pXORCOUNTERState = (XORCOUNTERStateEntry *)pTypeState;
  fprintf(stdout, " b%p->b%p [style=solid,label=\" x_i \"]\n", (void *)pXORCOUNTERState, (void *)pXORCOUNTERState->pVarIsTrueTransition);
  fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{x_i} \"]\n", (void *)pXORCOUNTERState, (void *)pXORCOUNTERState->pVarIsFalseTransition);
  fprintf(stdout, " b%p [shape=\"ellipse\",label=\"x_1 \\oplus .. \\oplus x_%ju\"]\n", (void *)pXORCOUNTERState, pXORCOUNTERState->nNumVariables);

  pTypeState = pXORCOUNTERState->pVarIsTrueTransition;
  arrPrintStateEntry_dot[pTypeState->type](SM, pTypeState, pIndex2Var);

  pTypeState = pXORCOUNTERState->pVarIsFalseTransition;
  arrPrintStateEntry_dot[pTypeState->type](SM, pTypeState, pIndex2Var);
}

void FreeXORCOUNTERStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {

}
