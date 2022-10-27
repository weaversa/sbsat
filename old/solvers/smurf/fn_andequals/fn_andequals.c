#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

// ANDEQUALS State

void initANDEQUALSStateType() {
  if(gelim_smurfs) arrGetStateType[FN_ANDEQUALS] = SM_TRANSITION_TYPE;
  else arrGetStateType[FN_ANDEQUALS] = SM_NODE_TYPE;

  arrCreateStateEntry[FN_ANDEQUALS] = CreateANDEQUALSState;
  arrStatesTypeSize[FN_ANDEQUALS] = ComputeANDEQUALSStateSize;
  arrSetStateVisitedFlag[FN_ANDEQUALS] = SetANDEQUALSStateVisitedFlag;
  arrUnsetStateVisitedFlag[FN_ANDEQUALS] = UnsetANDEQUALSStateVisitedFlag;
  arrCleanUpStateEntry[FN_ANDEQUALS] = CleanUpANDEQUALSState;

  if(gelim_smurfs) arrApplyInferenceToState[FN_ANDEQUALS] = ApplyInferenceToANDEQUALS_gelim;
  else arrApplyInferenceToState[FN_ANDEQUALS] = ApplyInferenceToANDEQUALS;

  arrPrintStateEntry[FN_ANDEQUALS] = PrintANDEQUALSStateEntry;
  arrPrintStateEntry_dot[FN_ANDEQUALS] = PrintANDEQUALSStateEntry_dot;
  arrFreeStateEntry[FN_ANDEQUALS] = FreeANDEQUALSStateEntry;
  arrCalculateStateHeuristic[FN_ANDEQUALS] = CalculateANDEQUALSStateLSGBHeuristic;
  //arrSetStateHeuristicScore[FN_ANDEQUALS] = LSGBANDEQUALSStateSetHeurScore;
  arrGetStateHeuristicScore[FN_ANDEQUALS] = LSGBANDEQUALSStateGetHeurScore;
  arrGetVarHeuristicScore[FN_ANDEQUALS] = LSGBANDEQUALSStateGetVarHeurScore;
  arrFreeHeuristicScore[FN_ANDEQUALS] = LSGBANDEQUALSStateFree;
}

uintmax_t ComputeANDEQUALSStateSize(TypeStateEntry *pTypeState) {
  ANDEQUALSStateEntry *pANDEQUALSState = (ANDEQUALSStateEntry *)pTypeState;
  assert(pANDEQUALSState->type == FN_ANDEQUALS);
  return sizeof(ANDEQUALSStateEntry);
}

void PrintANDEQUALSStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {
  ANDEQUALSStateEntry *pANDEQUALSState = (ANDEQUALSStateEntry *)pTypeState;
  assert(pANDEQUALSState->type == FN_ANDEQUALS);

  fprintf(stdout, "AE %p, lhs=%ju, %s, or=%p, refs=%d\n", (void *)pANDEQUALSState,
	  pANDEQUALSState->nLHS, pANDEQUALSState->bAND_or_OR?"and=":"or=", (void *)pANDEQUALSState->pORState,
	     Cudd_Regular(pANDEQUALSState->pANDEQUALSBDD)->ref);
  PrintORStateEntry(SM, (TypeStateEntry *)pANDEQUALSState->pORState);
}

void PrintANDEQUALSStateEntry_dot(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t *pIndex2Var) {
  if(pTypeState->visited == 1) return;
  pTypeState->visited = 1;

  ANDEQUALSStateEntry *pANDEQUALSState = (ANDEQUALSStateEntry *)pTypeState;
  //  ORStateEntry *pORState = pANDEQUALSState->pORState;
  //  arrPrintStateEntry_dot[pORState->type](SM, (TypeStateEntry *)pORState, pIndex2Var);

  fprintf(stdout, " b%p->b%p [style=solid,label=\" \\overline{%s} : \\overline{x_1} \"]\n", (void *)pANDEQUALSState, (void *)SM->pTrueSmurfState, s_name(pIndex2Var[pANDEQUALSState->nLHS]));
  fprintf(stdout, " b%p->b%p [style=dotted,label=\" %s : x_1 \"]\n", (void *)pANDEQUALSState, (void *)SM->pTrueSmurfState, s_name(pIndex2Var[pANDEQUALSState->nLHS]));
  fprintf(stdout, " b%p->b%p [style=solid,label=\" x_1 : %s \"]\n", (void *)pANDEQUALSState, (void *)SM->pTrueSmurfState, s_name(pIndex2Var[pANDEQUALSState->nLHS]));
  fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{x_1} : \\overline{%s} \"]\n", (void *)pANDEQUALSState, (void *)SM->pTrueSmurfState, s_name(pIndex2Var[pANDEQUALSState->nLHS]));
  fprintf(stdout, " b%p [shape=\"ellipse\",label=\"%s \\oplus x_1 \\oplus {\\tt T}\"]\n", (void *)pANDEQUALSState, s_name(pIndex2Var[pANDEQUALSState->nLHS]));
}

void FreeANDEQUALSStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {
  DdNode *pANDEQUALSBDD = ((ANDEQUALSStateEntry *)pTypeState)->pANDEQUALSBDD;
  if(pANDEQUALSBDD != NULL) {
    if(Cudd_IsComplement(pANDEQUALSBDD)) {
      Cudd_Not(pANDEQUALSBDD)->pStateNeg = NULL;
    } else {
      pANDEQUALSBDD->pStatePos = NULL;
    }
    Cudd_IterDerefBdd(SM->dd, pANDEQUALSBDD);
  }
}
