#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

// MINMAXCOUNTER State

void initMINMAXCOUNTERStateType() {
  arrGetStateType[FN_MINMAXCOUNTER] = SM_NODE_TYPE;

  arrCreateStateEntry[FN_MINMAXCOUNTER] = CreateMINMAXCOUNTERState;
  arrStatesTypeSize[FN_MINMAXCOUNTER] = ComputeMINMAXCOUNTERStateSize;
  arrSetStateVisitedFlag[FN_MINMAXCOUNTER] = SetMINMAXCOUNTERStateVisitedFlag;
  arrUnsetStateVisitedFlag[FN_MINMAXCOUNTER] = UnsetMINMAXCOUNTERStateVisitedFlag;
  arrCleanUpStateEntry[FN_MINMAXCOUNTER] = CleanUpMINMAXCOUNTERState;
  arrApplyInferenceToState[FN_MINMAXCOUNTER] = ApplyInferenceToMINMAXCOUNTER;
  arrPrintStateEntry[FN_MINMAXCOUNTER] = PrintMINMAXCOUNTERStateEntry;
  arrPrintStateEntry_dot[FN_MINMAXCOUNTER] = PrintMINMAXCOUNTERStateEntry_dot;
  arrFreeStateEntry[FN_MINMAXCOUNTER] = FreeMINMAXCOUNTERStateEntry;
  arrCalculateStateHeuristic[FN_MINMAXCOUNTER] = CalculateMINMAXCOUNTERStateLSGBHeuristic;
  //arrSetStateHeuristicScore[FN_MINMAXCOUNTER] = LSGBMINMAXCOUNTERStateSetHeurScore;
  arrGetStateHeuristicScore[FN_MINMAXCOUNTER] = LSGBMINMAXCOUNTERStateGetHeurScore;
  arrGetVarHeuristicScore[FN_MINMAXCOUNTER] = LSGBMINMAXCOUNTERStateGetVarHeurScore;
  arrFreeHeuristicScore[FN_MINMAXCOUNTER] = LSGBMINMAXCOUNTERStateFree;
}

uintmax_t ComputeMINMAXCOUNTERStateSize(TypeStateEntry *pTypeState) {
  MINMAXCOUNTERStateEntry *pMINMAXCOUNTERState = (MINMAXCOUNTERStateEntry *)pTypeState;
  assert(pMINMAXCOUNTERState->type == FN_MINMAXCOUNTER);
  return sizeof(MINMAXCOUNTERStateEntry);
}

void PrintMINMAXCOUNTERStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {
  MINMAXCOUNTERStateEntry *pMINMAXCOUNTERState = (MINMAXCOUNTERStateEntry *)pTypeState;
  assert(pMINMAXCOUNTERState->type == FN_MINMAXCOUNTER);

  /*
  fprintf(stdout, "MINMAXCOUNTER %p, nvars=%u, pol=", (void *)pMINMAXCOUNTERState, pMINMAXCOUNTERState->nNumVariables);
  for(uint32_t i = 0; i < pMINMAXCOUNTERState->nNumVariables; i++)
    fprintf(stdout, "%u ", pMINMAXCOUNTERState->bPolarity[i]);
  fprintf(stdout, "\b, refs=%d\n", Cudd_Regular(pMINMAXCOUNTERState->pMINMAXCOUNTERBDD)->ref);
  */
}

void PrintMINMAXCOUNTERStateEntry_dot(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t *pIndex2Var) {
  if(pTypeState->visited == 1) return;
  pTypeState->visited = 1;

  MINMAXCOUNTERStateEntry *pMINMAXCOUNTERState = (MINMAXCOUNTERStateEntry *)pTypeState;

  MINMAXStateEntry *pMINMAXState = pMINMAXCOUNTERState->pMINMAXState;
  uintmax_t nNumVariables = pMINMAXCOUNTERState->nNumVariables;
  uintmax_t nNumTrue = pMINMAXCOUNTERState->nNumTrue;
  uintmax_t min = nNumTrue > pMINMAXState->nMin ? 0 : pMINMAXState->nMin - nNumTrue;
  uintmax_t max = pMINMAXState->nMax - nNumTrue;

  //Print True transition
  if(max == 1) {
    assert(pMINMAXCOUNTERState->pVarIsTrueTransition == (TypeStateEntry *)SM->pTrueSmurfState);
    //Infer remaining variables to False
    if(nNumVariables == 2) {
      fprintf(stdout, " b%p->b%p [style=solid,label=\" x_1 : \\overline{x_2} \"]\n", (void *)pMINMAXCOUNTERState, (void *)pMINMAXCOUNTERState->pVarIsTrueTransition);
      fprintf(stdout, " b%p->b%p [style=solid,label=\" x_2 : \\overline{x_1} \"]\n", (void *)pMINMAXCOUNTERState, (void *)pMINMAXCOUNTERState->pVarIsTrueTransition);
    } else {
      fprintf(stdout, " b%p->b%p [style=solid,label=\" xi : \\overline{x_1} .. \\overline{x_%ju} \"]\n", (void *)pMINMAXCOUNTERState, (void *)pMINMAXCOUNTERState->pVarIsTrueTransition, nNumVariables);
    }
  } else {
    //No inferences
    if(nNumVariables == 2) {
      assert(pMINMAXCOUNTERState->pVarIsTrueTransition == (TypeStateEntry *)SM->pTrueSmurfState);
      fprintf(stdout, " b%p->b%p [style=solid,label=\" x_1 \"]\n", (void *)pMINMAXCOUNTERState, (void *)pMINMAXCOUNTERState->pVarIsTrueTransition);
      fprintf(stdout, " b%p->b%p [style=solid,label=\" x_2 \"]\n", (void *)pMINMAXCOUNTERState, (void *)pMINMAXCOUNTERState->pVarIsTrueTransition);
    } else {
      fprintf(stdout, " b%p->b%p [style=solid,label=\" xi \"]\n", (void *)pMINMAXCOUNTERState, (void *)pMINMAXCOUNTERState->pVarIsTrueTransition);
    }
  }

  //Print False transition
  if(nNumVariables-1 == min) {
    assert(pMINMAXCOUNTERState->pVarIsFalseTransition == (TypeStateEntry *)SM->pTrueSmurfState);
    //Infer remaining variables to True
    if(nNumVariables == 2) {
      fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{x_1} : x_2 \"]\n", (void *)pMINMAXCOUNTERState, (void *)pMINMAXCOUNTERState->pVarIsFalseTransition);
      fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{x_2} : x_1 \"]\n", (void *)pMINMAXCOUNTERState, (void *)pMINMAXCOUNTERState->pVarIsFalseTransition);
    } else {
      fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{x_i} : x_1 .. x_%ju \"]\n", (void *)pMINMAXCOUNTERState, (void *)pMINMAXCOUNTERState->pVarIsFalseTransition, nNumVariables);
    }
  } else {
    //No inferences
    if(nNumVariables == 2) {
      assert(pMINMAXCOUNTERState->pVarIsFalseTransition == (TypeStateEntry *)SM->pTrueSmurfState);
      fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{x_1} \"]\n", (void *)pMINMAXCOUNTERState, (void *)pMINMAXCOUNTERState->pVarIsFalseTransition);
      fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{x_2} \"]\n", (void *)pMINMAXCOUNTERState, (void *)pMINMAXCOUNTERState->pVarIsFalseTransition);
    } else {
      fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{x_i} \"]\n", (void *)pMINMAXCOUNTERState, (void *)pMINMAXCOUNTERState->pVarIsFalseTransition);
    }
  }
  
  if(min == 0) {
    if(nNumVariables == 2) {
      fprintf(stdout, " b%p [shape=\"ellipse\", label=\" x_1 + x_2 \\leq %ju \"]\n", (void *)pMINMAXCOUNTERState, max);
    } else {
      fprintf(stdout, " b%p [shape=\"ellipse\", label=\" x_1 + .. + x_%ju \\leq %ju \"]\n", (void *)pMINMAXCOUNTERState, nNumVariables, max);
    }
  } else if(nNumVariables <= max) {
    if(nNumVariables == 2) {
      fprintf(stdout, " b%p [shape=\"ellipse\", label=\" %ju \\leq x_1 + x_2 \"]\n", (void *)pMINMAXCOUNTERState, min);
    } else {
      fprintf(stdout, " b%p [shape=\"ellipse\", label=\" %ju \\leq x_1 + .. + x_%ju \"]\n", (void *)pMINMAXCOUNTERState, min, nNumVariables);
    }
  } else {
    if(nNumVariables == 2) {
      fprintf(stdout, " b%p [shape=\"ellipse\", label=\" %ju \\leq x_1 + x_2 \\leq %ju \"]\n", (void *)pMINMAXCOUNTERState, min, max);
    } else {
      fprintf(stdout, " b%p [shape=\"ellipse\", label=\" %ju \\leq x_1 + .. + x_%ju \\leq %ju \"]\n", (void *)pMINMAXCOUNTERState, min, nNumVariables, max);
    }
  }

  pTypeState = pMINMAXCOUNTERState->pVarIsTrueTransition;
  arrPrintStateEntry_dot[pTypeState->type](SM, pTypeState, pIndex2Var);
  
  pTypeState = pMINMAXCOUNTERState->pVarIsFalseTransition;
  arrPrintStateEntry_dot[pTypeState->type](SM, pTypeState, pIndex2Var);
}

void FreeMINMAXCOUNTERStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {

}
