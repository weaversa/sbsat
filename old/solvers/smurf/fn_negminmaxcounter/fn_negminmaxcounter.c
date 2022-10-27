#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

// NEGMINMAXCOUNTER State

void initNEGMINMAXCOUNTERStateType() {
  arrGetStateType[FN_NEGMINMAXCOUNTER] = SM_NODE_TYPE;

  arrCreateStateEntry[FN_NEGMINMAXCOUNTER] = CreateNEGMINMAXCOUNTERState;
  arrStatesTypeSize[FN_NEGMINMAXCOUNTER] = ComputeNEGMINMAXCOUNTERStateSize;
  arrSetStateVisitedFlag[FN_NEGMINMAXCOUNTER] = SetNEGMINMAXCOUNTERStateVisitedFlag;
  arrUnsetStateVisitedFlag[FN_NEGMINMAXCOUNTER] = UnsetNEGMINMAXCOUNTERStateVisitedFlag;
  arrCleanUpStateEntry[FN_NEGMINMAXCOUNTER] = CleanUpNEGMINMAXCOUNTERState;
  arrApplyInferenceToState[FN_NEGMINMAXCOUNTER] = ApplyInferenceToNEGMINMAXCOUNTER;
  arrPrintStateEntry[FN_NEGMINMAXCOUNTER] = PrintNEGMINMAXCOUNTERStateEntry;
  arrPrintStateEntry_dot[FN_NEGMINMAXCOUNTER] = PrintNEGMINMAXCOUNTERStateEntry_dot;
  arrFreeStateEntry[FN_NEGMINMAXCOUNTER] = FreeNEGMINMAXCOUNTERStateEntry;
  arrCalculateStateHeuristic[FN_NEGMINMAXCOUNTER] = CalculateNEGMINMAXCOUNTERStateLSGBHeuristic;
  //arrSetStateHeuristicScore[FN_NEGMINMAXCOUNTER] = LSGBNEGMINMAXCOUNTERStateSetHeurScore;
  arrGetStateHeuristicScore[FN_NEGMINMAXCOUNTER] = LSGBNEGMINMAXCOUNTERStateGetHeurScore;
  arrGetVarHeuristicScore[FN_NEGMINMAXCOUNTER] = LSGBNEGMINMAXCOUNTERStateGetVarHeurScore;
  arrFreeHeuristicScore[FN_NEGMINMAXCOUNTER] = LSGBNEGMINMAXCOUNTERStateFree;
}

uintmax_t ComputeNEGMINMAXCOUNTERStateSize(TypeStateEntry *pTypeState) {
  NEGMINMAXCOUNTERStateEntry *pNEGMINMAXCOUNTERState = (NEGMINMAXCOUNTERStateEntry *)pTypeState;
  assert(pNEGMINMAXCOUNTERState->type == FN_NEGMINMAXCOUNTER);
  return sizeof(NEGMINMAXCOUNTERStateEntry);
}

void PrintNEGMINMAXCOUNTERStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {
  NEGMINMAXCOUNTERStateEntry *pNEGMINMAXCOUNTERState = (NEGMINMAXCOUNTERStateEntry *)pTypeState;
  assert(pNEGMINMAXCOUNTERState->type == FN_NEGMINMAXCOUNTER);

  /*
  fprintf(stdout, "NEGMINMAXCOUNTER %p, nvars=%u, pol=", (void *)pNEGMINMAXCOUNTERState, pNEGMINMAXCOUNTERState->nNumVariables);
  for(uint32_t i = 0; i < pNEGMINMAXCOUNTERState->nNumVariables; i++)
    fprintf(stdout, "%u ", pNEGMINMAXCOUNTERState->bPolarity[i]);
  fprintf(stdout, "\b, refs=%d\n", Cudd_Regular(pNEGMINMAXCOUNTERState->pNEGMINMAXCOUNTERBDD)->ref);
  */
}

void PrintNEGMINMAXCOUNTERStateEntry_dot(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t *pIndex2Var) {
  if(pTypeState->visited == 1) return;
  pTypeState->visited = 1;

  NEGMINMAXCOUNTERStateEntry *pNEGMINMAXCOUNTERState = (NEGMINMAXCOUNTERStateEntry *)pTypeState;

  NEGMINMAXStateEntry *pNEGMINMAXState = pNEGMINMAXCOUNTERState->pNEGMINMAXState;
  uintmax_t nNumVariables = pNEGMINMAXCOUNTERState->nNumVariables;
  uintmax_t nNumTrue = pNEGMINMAXCOUNTERState->nNumTrue;
  uintmax_t min = nNumTrue > pNEGMINMAXState->nMin ? 0 : pNEGMINMAXState->nMin - nNumTrue;
  uintmax_t max = pNEGMINMAXState->nMax - nNumTrue;

  //Print True transition
  if(nNumVariables <= max && min==2) {
    //Infer remaining variables to False
    assert(pNEGMINMAXCOUNTERState->pVarIsTrueTransition == (TypeStateEntry *)SM->pTrueSmurfState); 
    if(nNumVariables == 2) {
      fprintf(stdout, " b%p->b%p [style=solid,label=\" x_1 : \\overline{x_2} \"]\n", (void *)pNEGMINMAXCOUNTERState, (void *)pNEGMINMAXCOUNTERState->pVarIsTrueTransition);
      fprintf(stdout, " b%p->b%p [style=solid,label=\" x_2 : \\overline{x_1} \"]\n", (void *)pNEGMINMAXCOUNTERState, (void *)pNEGMINMAXCOUNTERState->pVarIsTrueTransition);
    } else {
      fprintf(stdout, " b%p->b%p [style=solid,label=\" xi : \\overline{x_1} .. \\overline{x_%ju} \"]\n", (void *)pNEGMINMAXCOUNTERState, (void *)pNEGMINMAXCOUNTERState->pVarIsTrueTransition, nNumVariables);
    }
  } else if(nNumVariables == max+1 && min == 1) {
    //Infer remaining variables to True
    assert(pNEGMINMAXCOUNTERState->pVarIsTrueTransition == (TypeStateEntry *)SM->pTrueSmurfState); 
    if(nNumVariables == 2) {
      fprintf(stdout, " b%p->b%p [style=solid,label=\" x_1 : x_2 \"]\n", (void *)pNEGMINMAXCOUNTERState, (void *)pNEGMINMAXCOUNTERState->pVarIsTrueTransition);
      fprintf(stdout, " b%p->b%p [style=solid,label=\" x_2 : x_1 \"]\n", (void *)pNEGMINMAXCOUNTERState, (void *)pNEGMINMAXCOUNTERState->pVarIsTrueTransition);
    } else {
      fprintf(stdout, " b%p->b%p [style=solid,label=\" xi : x_1 .. x_%ju \"]\n", (void *)pNEGMINMAXCOUNTERState, (void *)pNEGMINMAXCOUNTERState->pVarIsTrueTransition, nNumVariables);
    }
  } else {
    //No inferences
    if(nNumVariables == 2) {
      assert(pNEGMINMAXCOUNTERState->pVarIsTrueTransition == (TypeStateEntry *)SM->pTrueSmurfState);
      fprintf(stdout, " b%p->b%p [style=solid,label=\" x_1 \"]\n", (void *)pNEGMINMAXCOUNTERState, (void *)pNEGMINMAXCOUNTERState->pVarIsTrueTransition);
      fprintf(stdout, " b%p->b%p [style=solid,label=\" x_2 \"]\n", (void *)pNEGMINMAXCOUNTERState, (void *)pNEGMINMAXCOUNTERState->pVarIsTrueTransition);
    } else {
      fprintf(stdout, " b%p->b%p [style=solid,label=\" xi \"]\n", (void *)pNEGMINMAXCOUNTERState, (void *)pNEGMINMAXCOUNTERState->pVarIsTrueTransition);
    }
  }

  //Print False transition
  if(min == 0 && nNumVariables == max+2) {
    assert(pNEGMINMAXCOUNTERState->pVarIsFalseTransition == (TypeStateEntry *)SM->pTrueSmurfState);
    //Infer remaining variables to True
    if(nNumVariables == 2) {
      fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{x_1} : x_2 \"]\n", (void *)pNEGMINMAXCOUNTERState, (void *)pNEGMINMAXCOUNTERState->pVarIsFalseTransition);
      fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{x_2} : x_1 \"]\n", (void *)pNEGMINMAXCOUNTERState, (void *)pNEGMINMAXCOUNTERState->pVarIsFalseTransition);
    } else {
      fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{x_i} : x_1 .. x_%ju \"]\n", (void *)pNEGMINMAXCOUNTERState, (void *)pNEGMINMAXCOUNTERState->pVarIsFalseTransition, nNumVariables);
    }
  } else if(nNumVariables == max+1 && min == 1) {
    //Infer remaining variables to False
    assert(pNEGMINMAXCOUNTERState->pVarIsFalseTransition == (TypeStateEntry *)SM->pTrueSmurfState); 
    if(nNumVariables == 2) {
      fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{x_1} : \\overline{x_2} \"]\n", (void *)pNEGMINMAXCOUNTERState, (void *)pNEGMINMAXCOUNTERState->pVarIsFalseTransition);
      fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{x_2} : \\overline{x_1} \"]\n", (void *)pNEGMINMAXCOUNTERState, (void *)pNEGMINMAXCOUNTERState->pVarIsFalseTransition);
    } else {
      fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{x_i} : \\overline{x_1} .. \\overline{x_%ju} \"]\n", (void *)pNEGMINMAXCOUNTERState, (void *)pNEGMINMAXCOUNTERState->pVarIsFalseTransition, nNumVariables);
    }
  } else {
    //No inferences
    if(nNumVariables == 2) {
      assert(pNEGMINMAXCOUNTERState->pVarIsFalseTransition == (TypeStateEntry *)SM->pTrueSmurfState);
      fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{x_1} \"]\n", (void *)pNEGMINMAXCOUNTERState, (void *)pNEGMINMAXCOUNTERState->pVarIsFalseTransition);
      fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{x_2} \"]\n", (void *)pNEGMINMAXCOUNTERState, (void *)pNEGMINMAXCOUNTERState->pVarIsFalseTransition);
    } else {
      fprintf(stdout, " b%p->b%p [style=dotted,label=\" \\overline{x_i} \"]\n", (void *)pNEGMINMAXCOUNTERState, (void *)pNEGMINMAXCOUNTERState->pVarIsFalseTransition);
    }
  }

  if(min == 0) {
    if(nNumVariables == 2) {
      fprintf(stdout, " b%p [shape=\"ellipse\", label=\"x_1 + x_2 > %ju\"]\n", (void *)pNEGMINMAXCOUNTERState, max);
    } else {
      fprintf(stdout, " b%p [shape=\"ellipse\", label=\"x_1 + .. + x_%ju > %ju\"]\n", (void *)pNEGMINMAXCOUNTERState, nNumVariables, max);
    }
  } else if(nNumVariables <= max) {
    if(nNumVariables == 2) {
      fprintf(stdout, " b%p [shape=\"ellipse\", label=\"x_1 + x_2 < %ju\"]\n", (void *)pNEGMINMAXCOUNTERState, min);
    } else {
      fprintf(stdout, " b%p [shape=\"ellipse\", label=\"x_1 + .. + x_%ju < %ju\"]\n", (void *)pNEGMINMAXCOUNTERState, nNumVariables, min);
    }
  } else {
    if(nNumVariables == 2) {
      fprintf(stdout, " b%p [shape=\"ellipse\", label=\"\\overline{%ju \\leq x_1 + x_2 \\leq %ju}\"]\n", (void *)pNEGMINMAXCOUNTERState, min, max);
    } else {
      fprintf(stdout, " b%p [shape=\"ellipse\", label=\"\\overline{%ju \\leq x_1 + .. + x_%ju \\leq %ju}\"]\n", (void *)pNEGMINMAXCOUNTERState, min, nNumVariables, max);
    }
  }

  pTypeState = pNEGMINMAXCOUNTERState->pVarIsTrueTransition;
  arrPrintStateEntry_dot[pTypeState->type](SM, pTypeState, pIndex2Var);
  
  pTypeState = pNEGMINMAXCOUNTERState->pVarIsFalseTransition;
  arrPrintStateEntry_dot[pTypeState->type](SM, pTypeState, pIndex2Var);
}

void FreeNEGMINMAXCOUNTERStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {

}
