#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

// ANDEQUALSCOUNTER State

void initANDEQUALSCOUNTERStateType() {
  arrGetStateType[FN_ANDEQUALSCOUNTER] = SM_NODE_TYPE;

  arrCreateStateEntry[FN_ANDEQUALSCOUNTER] = CreateANDEQUALSCOUNTERState;
  arrStatesTypeSize[FN_ANDEQUALSCOUNTER] = ComputeANDEQUALSCOUNTERStateSize;
  arrSetStateVisitedFlag[FN_ANDEQUALSCOUNTER] = SetANDEQUALSCOUNTERStateVisitedFlag;
  arrUnsetStateVisitedFlag[FN_ANDEQUALSCOUNTER] = UnsetANDEQUALSCOUNTERStateVisitedFlag;
  arrCleanUpStateEntry[FN_ANDEQUALSCOUNTER] = CleanUpANDEQUALSCOUNTERState;
  arrApplyInferenceToState[FN_ANDEQUALSCOUNTER] = ApplyInferenceToANDEQUALSCOUNTER;
  arrPrintStateEntry[FN_ANDEQUALSCOUNTER] = PrintANDEQUALSCOUNTERStateEntry;
  arrPrintStateEntry_dot[FN_ANDEQUALSCOUNTER] = PrintANDEQUALSCOUNTERStateEntry_dot;
  arrFreeStateEntry[FN_ANDEQUALSCOUNTER] = FreeANDEQUALSCOUNTERStateEntry;
  arrCalculateStateHeuristic[FN_ANDEQUALSCOUNTER] = CalculateANDEQUALSCOUNTERStateLSGBHeuristic;
  //arrSetStateHeuristicScore[FN_ANDEQUALSCOUNTER] = LSGBANDEQUALSCOUNTERStateSetHeurScore;
  arrGetStateHeuristicScore[FN_ANDEQUALSCOUNTER] = LSGBANDEQUALSCOUNTERStateGetHeurScore;
  arrGetVarHeuristicScore[FN_ANDEQUALSCOUNTER] = LSGBANDEQUALSCOUNTERStateGetVarHeurScore;
  arrFreeHeuristicScore[FN_ANDEQUALSCOUNTER] = LSGBANDEQUALSCOUNTERStateFree;
}

uintmax_t ComputeANDEQUALSCOUNTERStateSize(TypeStateEntry *pTypeState) {
  ANDEQUALSCOUNTERStateEntry *pANDEQUALSCOUNTERState = (ANDEQUALSCOUNTERStateEntry *)pTypeState;
  assert(pANDEQUALSCOUNTERState->type == FN_ANDEQUALSCOUNTER);
  return sizeof(ANDEQUALSCOUNTERStateEntry);
}

void PrintANDEQUALSCOUNTERStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {
  ANDEQUALSCOUNTERStateEntry *pANDEQUALSCOUNTERState = (ANDEQUALSCOUNTERStateEntry *)pTypeState;
  assert(pANDEQUALSCOUNTERState->type == FN_ANDEQUALSCOUNTER);

  //  fprintf(stdout, "AE %p, lhs=%ju, %s, or=%p, refs=%d\n", (void *)pANDEQUALSCOUNTERState,
  //	  pANDEQUALSCOUNTERState->nLHS, pANDEQUALSCOUNTERState->bAND_or_OR?"and=":"or=", (void *)pANDEQUALSCOUNTERState->pORState,
  //	     Cudd_Regular(pANDEQUALSCOUNTERState->pANDEQUALSCOUNTERBDD)->ref);
  //  PrintORStateEntry(SM, (TypeStateEntry *)pANDEQUALSCOUNTERState->pORState);
}

void PrintANDEQUALSCOUNTERStateEntry_dot(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t *pIndex2Var) {
  if(pTypeState->visited == 1) return;
  pTypeState->visited = 1;

  ANDEQUALSCOUNTERStateEntry *pANDEQUALSCOUNTERState = (ANDEQUALSCOUNTERStateEntry *)pTypeState;
  ANDEQUALSStateEntry *pANDEQUALSState = pANDEQUALSCOUNTERState->pANDEQUALSState;
  ORCOUNTERStateEntry *pORCOUNTERState = pANDEQUALSCOUNTERState->pORCOUNTERState;
  arrPrintStateEntry_dot[pORCOUNTERState->type](SM, (TypeStateEntry *)pORCOUNTERState, pIndex2Var);
  uintmax_t nNumVariables;
  if(pANDEQUALSCOUNTERState->pORCOUNTERState->type == FN_ORCOUNTER) {
    nNumVariables = pANDEQUALSCOUNTERState->pORCOUNTERState->nNumVariables;
  } else {
    assert(pANDEQUALSCOUNTERState->pORCOUNTERState->type == FN_OR);
    nNumVariables = 2;
  }

  //SEAN!!! THESE ARE WRONG!!!
  if(pANDEQUALSState->bAND_or_OR) {
    fprintf(stdout, " b%p->b%p [style=solid,label=\" %s : x_1..x_%ju \"]\n", (void *)pANDEQUALSCOUNTERState, (void *)SM->pTrueSmurfState, s_name(pIndex2Var[pANDEQUALSState->nLHS]), nNumVariables);
    fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{%s} \"]\n", (void *)pANDEQUALSCOUNTERState, (void *)pANDEQUALSCOUNTERState->pORCOUNTERState, s_name(pIndex2Var[pANDEQUALSState->nLHS]));
    fprintf(stdout, " b%p->b%p [style=solid,label=\" x_i \"]\n", (void *)pANDEQUALSCOUNTERState, (void *)pANDEQUALSCOUNTERState->pTransition);
    fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{x_i} : \\overline{%s} \"]\n", (void *)pANDEQUALSCOUNTERState, (void *)SM->pTrueSmurfState, s_name(pIndex2Var[pANDEQUALSState->nLHS]));
    fprintf(stdout, " b%p [shape=\"ellipse\",label=\"%s \\oplus (\\overline{x_1} \\vee .. \\vee \\overline{x_%ju})\"]\n", (void *)pANDEQUALSCOUNTERState, s_name(pIndex2Var[pANDEQUALSState->nLHS]), nNumVariables);
  } else {
    fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{%s} : \\overline{x_1}..\\overline{x_%ju} \"]\n", (void *)pANDEQUALSCOUNTERState, (void *)SM->pTrueSmurfState, s_name(pIndex2Var[pANDEQUALSState->nLHS]), nNumVariables);
    fprintf(stdout, " b%p->b%p [style=solid,label=\" %s \"]\n", (void *)pANDEQUALSCOUNTERState, (void *)pANDEQUALSCOUNTERState->pORCOUNTERState, s_name(pIndex2Var[pANDEQUALSState->nLHS]));
    fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{x_i} \"]\n", (void *)pANDEQUALSCOUNTERState, (void *)pANDEQUALSCOUNTERState->pTransition);
    fprintf(stdout, " b%p->b%p [style=solid,label=\" x_i : %s \"]\n", (void *)pANDEQUALSCOUNTERState, (void *)SM->pTrueSmurfState, s_name(pIndex2Var[pANDEQUALSState->nLHS]));
    fprintf(stdout, " b%p [shape=\"ellipse\",label=\"%s \\oplus (\\overline{x_1} \\wedge ..\\wedge \\overline{x_%ju})\"]\n", (void *)pANDEQUALSCOUNTERState, s_name(pIndex2Var[pANDEQUALSState->nLHS]), nNumVariables);
  }

  pTypeState = pANDEQUALSCOUNTERState->pTransition;
  arrPrintStateEntry_dot[pTypeState->type](SM, pTypeState, pIndex2Var);
}

void FreeANDEQUALSCOUNTERStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {

}
