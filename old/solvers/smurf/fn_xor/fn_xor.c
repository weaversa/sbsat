#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

// XOR State

void initXORStateType() {
  if(gelim_smurfs) arrGetStateType[FN_XOR] = SM_TRANSITION_TYPE;
  else arrGetStateType[FN_XOR] = SM_NODE_TYPE;

  arrCreateStateEntry[FN_XOR] = CreateXORState;
  arrStatesTypeSize[FN_XOR] = ComputeXORStateSize;
  arrSetStateVisitedFlag[FN_XOR] = SetXORStateVisitedFlag;
  arrUnsetStateVisitedFlag[FN_XOR] = UnsetXORStateVisitedFlag;
  arrCleanUpStateEntry[FN_XOR] = CleanUpXORState;

  if(gelim_smurfs) arrApplyInferenceToState[FN_XOR] = ApplyInferenceToXOR_gelim;
  else arrApplyInferenceToState[FN_XOR] = ApplyInferenceToXOR;

  arrPrintStateEntry[FN_XOR] = PrintXORStateEntry;
  arrPrintStateEntry_dot[FN_XOR] = PrintXORStateEntry_dot;
  arrFreeStateEntry[FN_XOR] = FreeXORStateEntry;
  arrCalculateStateHeuristic[FN_XOR] = CalculateXORStateLSGBHeuristic;
  //arrSetStateHeuristicScore[FN_XOR] = LSGBXORStateSetHeurScore;
  arrGetStateHeuristicScore[FN_XOR] = LSGBXORStateGetHeurScore;
  arrGetVarHeuristicScore[FN_XOR] = LSGBXORStateGetVarHeurScore;
  arrFreeHeuristicScore[FN_XOR] = LSGBXORStateFree;
}

uintmax_t ComputeXORStateSize(TypeStateEntry *pTypeState) {
  XORStateEntry *pXORState = (XORStateEntry *)pTypeState;
  assert(pXORState->type == FN_XOR);
  return sizeof(XORStateEntry) + (sizeof(uint8_t) * pXORState->nNumVariables);
}

void PrintXORStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {
  XORStateEntry *pXORState = (XORStateEntry *)pTypeState;
  assert(pXORState->type == FN_XOR);

  d2_printf5("XOR %p, nvars=%ju, par=%u, refs=%d\n", (void *)pXORState, pXORState->nNumVariables, pXORState->bParity, Cudd_Regular(pXORState->pXORBDD)->ref);
}

void PrintXORStateEntry_dot(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t *pIndex2Var) {
  if(pTypeState->visited == 1) return;
  pTypeState->visited = 1;

  XORStateEntry *pXORState = (XORStateEntry *)pTypeState;
  if(!pXORState->bParity) {
    fprintf(stdout, " b%p->b%p [style=solid,label=\" x_1 : \\overline{x_2}\"]\n", (void *)pXORState, (void *)SM->pTrueSmurfState);
    fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{x_1} : x_2 \"]\n", (void *)pXORState, (void *)SM->pTrueSmurfState);
    fprintf(stdout, " b%p->b%p [style=solid,label=\" x_2 : \\overline{x_1} \"]\n", (void *)pXORState, (void *)SM->pTrueSmurfState);
    fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{x_2} : x_1 \"]\n", (void *)pXORState, (void *)SM->pTrueSmurfState);
    fprintf(stdout, " b%p [shape=\"ellipse\",label=\"x_1 \\oplus x_2\"]\n", (void *)pXORState);
  } else {
    fprintf(stdout, " b%p->b%p [style=solid,label=\" x_1 : x_2\"]\n", (void *)pXORState, (void *)SM->pTrueSmurfState);
    fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{x_1} : \\overline{x_2} \"]\n", (void *)pXORState, (void *)SM->pTrueSmurfState);
    fprintf(stdout, " b%p->b%p [style=solid,label=\" x_2 : x_1 \"]\n", (void *)pXORState, (void *)SM->pTrueSmurfState);
    fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{x_2} : \\overline{x_1} \"]\n", (void *)pXORState, (void *)SM->pTrueSmurfState);
    fprintf(stdout, " b%p [shape=\"ellipse\",label=\"x_1 \\oplus x_2 \\oplus {\\tt T}\"]\n", (void *)pXORState);
  }
}

void FreeXORStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {
  DdNode *pXORBDD = ((XORStateEntry *)pTypeState)->pXORBDD;
  if(pXORBDD != NULL) {
    if(Cudd_IsComplement(pXORBDD)) {
      Cudd_Not(pXORBDD)->pStateNeg = NULL;
    } else {
      pXORBDD->pStatePos = NULL;
    }
    Cudd_IterDerefBdd(SM->dd, pXORBDD);
  }
}
