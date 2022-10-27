#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

// Inference State
void initInferenceStateType() {
  arrGetStateType[FN_INFERENCE] = SM_TRANSITION_TYPE;

  arrCreateStateEntry[FN_INFERENCE] = CreateInferenceState;
  arrStatesTypeSize[FN_INFERENCE] = ComputeInferenceStateSize;
  arrSetStateVisitedFlag[FN_INFERENCE] = SetInferenceStateVisitedFlag;
  arrUnsetStateVisitedFlag[FN_INFERENCE] = UnsetInferenceStateVisitedFlag;
  arrCleanUpStateEntry[FN_INFERENCE] = CleanUpInferenceState;
  arrApplyInferenceToState[FN_INFERENCE] = ApplyInferenceToInference;
  arrPrintStateEntry[FN_INFERENCE] = PrintInferenceStateEntry;
  arrPrintStateEntry_dot[FN_INFERENCE] = PrintInferenceStateEntry_dot;
  arrFreeStateEntry[FN_INFERENCE] = FreeInferenceStateEntry;
  //arrCalculateStateHeuristic[FN_INFERENCE] = NULL;
  //arrSetStateHeuristicScore[FN_INFERENCE] = LSGBInferenceSetHeurScore;
  arrGetStateHeuristicScore[FN_INFERENCE] = LSGBInferenceGetStateHeurScore;
  arrGetVarHeuristicScore[FN_INFERENCE] = LSGBInferenceGetVarHeurScore;
  arrFreeHeuristicScore[FN_INFERENCE] = LSGBInferenceFree;
}

uintmax_t ComputeInferenceStateSize(TypeStateEntry *pTypeState) {
  return (uintmax_t)sizeof(InferenceStateEntry);
}

/*
TypeStateEntry *pGetNextSmurfStateFromInference(InferenceStateEntry *pInferenceState) {
  void *pNextState = pInferenceState;
  void *pPrevState = NULL;
  while(pNextState!=NULL && ((TypeStateEntry *)pNextState)->type == FN_INFERENCE) {
    //Follow the transtion to the next SmurfState
    pPrevState = pNextState;
    pNextState = ((InferenceStateEntry *)pNextState)->pTransition;
  }
  
  if(pNextState == NULL) {
    assert(((TypeStateEntry *)pPrevState)->type == FN_INFERENCE);
    ((InferenceStateEntry *)pPrevState)->pTransition = pNextState = ReadSmurfStateIntoTable(
	       set_variable(((InferenceStateEntry *)pPrevState)->pInferenceBDD,
	       arrSimpleSolver2IteVarMap[((InferenceStateEntry *)pPrevState)->nTransitionVar],
	       ((InferenceStateEntry *)pPrevState)->bPolarity),
	       NULL, 0);
    assert(((TypeStateEntry *)pNextState)->type==FN_FREE_STATE);
  }
  
  return (TypeStateEntry *)pNextState;
}
*/

void PrintInferenceStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {
  InferenceStateEntry *pInferenceState = (InferenceStateEntry *)pTypeState;
  fprintf(stdout, "IN %p, v=%jd, next=%p\n",
	  (void *)pInferenceState,
	  pInferenceState->bTransitionPol?(intmax_t)pInferenceState->nTransitionVar:-(intmax_t)pInferenceState->nTransitionVar,
	  (void *)pInferenceState->pTransition);
}

uint8_t print_inferences = 1;

void PrintInferenceStateEntry_dot(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t *pIndex2Var) {
  if(pTypeState->visited == 1) return;
  pTypeState->visited = 1;

  InferenceStateEntry *pInferenceState = (InferenceStateEntry *)pTypeState;

  if(pInferenceState->bTransitionPol==0) {
    fprintf(stdout, " b%p->b%p [style=dashed,label=",
	    (void *)pInferenceState,
	    (void *)pInferenceState->pTransition);
    if(print_inferences) {
      fprintf(stdout, "\"\\overline{%s}\"]\n", s_name(pIndex2Var[pInferenceState->nTransitionVar]));
    } else fprintf(stdout, "\"]\n");
  } else {
    fprintf(stdout, " b%p->b%p [style=solid,label=",
	    (void *)pInferenceState,
	    (void *)pInferenceState->pTransition);
    if(print_inferences) {
      fprintf(stdout, "\"%s\"]\n", s_name(pIndex2Var[pInferenceState->nTransitionVar]));
    } else fprintf(stdout, "\"]\n");
  }
  fprintf(stdout, " b%p [shape=\"circle\",label=\"I\"]\n", (void *)pInferenceState);
}

void PrintInferenceChain_dot(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t *pIndex2Var) {
  uint8_t colon_printed = 0;
  while(pTypeState && (pTypeState->type == FN_INFERENCE ||
		       pTypeState->type == FN_DROPPEDVAR ||
		       pTypeState->type == FN_XORFACTOR)) {
    if(pTypeState->type == FN_INFERENCE) {
      InferenceStateEntry *pInferenceState = (InferenceStateEntry *)pTypeState;
      if(print_inferences) {
	if(colon_printed == 0) {
	  fprintf(stdout, " :");
	  colon_printed = 1;
	}
	fprintf(stdout, " ");
	if(pInferenceState->bTransitionPol == 0) fprintf(stdout, "\\overline{");
	fprintf(stdout, "%s", s_name(pIndex2Var[pInferenceState->nTransitionVar]));
	if(pInferenceState->bTransitionPol == 0) fprintf(stdout, "}");
      }

      //Follow transition
      pTypeState = pInferenceState->pTransition;
    } else if(pTypeState->type == FN_DROPPEDVAR) {
      DroppedVarStateEntry *pDroppedVarState = (DroppedVarStateEntry *)pTypeState;

      if(print_dropped_vars) {
	if(colon_printed == 0) {
	  fprintf(stdout, " :");
	  colon_printed = 1;
	}
	fprintf(stdout, " ");
	fprintf(stdout, "[%s]", s_name(pIndex2Var[pDroppedVarState->nVar]));
      }

      //Follow transition
      pTypeState = pDroppedVarState->pTransition;
    } else if(pTypeState->type == FN_XORFACTOR) {
      XORFACTORStateEntry *pXORFACTORState = (XORFACTORStateEntry *)pTypeState;
      if(print_inferences) {
	if(colon_printed == 0) {
	  fprintf(stdout, " :");
	  colon_printed = 1;
	}
	fprintf(stdout, " ");

	DdNode *xorbdd;
	uintmax_t nLength;
	if(pXORFACTORState->pXORFactor->type == FN_XOR) {
	  xorbdd = ((XORStateEntry *)pXORFACTORState->pXORFactor)->pXORBDD;
	  nLength = ((XORStateEntry *)pXORFACTORState->pXORFactor)->nNumVariables;
	} else {
	  assert(pXORFACTORState->pXORFactor->type == FN_XORCOUNTER);
	  xorbdd = ((XORCOUNTERStateEntry *)pXORFACTORState->pXORFactor)->pXORState->pXORBDD;
	  nLength = ((XORCOUNTERStateEntry *)pXORFACTORState->pXORFactor)->pXORState->nNumVariables;
	}

	uintmax_t old_ignore_limits = ignore_limits;
	ignore_limits = 1;
	//Lift normalized BDD to pretty print the state label
	SM->uTempList1.nLength = nLength;
	for(uintmax_t i = 0; i < SM->uTempList1.nLength; i++) {
	  uint32_t_list_add(&SM->uTempList32, i, pIndex2Var[i]);
	}

	DdNode *lifted_bdd = Cudd_bddPermute(SM->dd, xorbdd, (int *)SM->uTempList32.pList);
	Cudd_Ref(lifted_bdd);
	//fprintf(stdout, "(");
	printBDD_ReduceSpecFunc_latex(SM->BM, lifted_bdd, stdout);
	//fprintf(stdout, ")");
	Cudd_IterDerefBdd(SM->dd, lifted_bdd);
	ignore_limits = old_ignore_limits;
      }

      //Follow transition
      pTypeState = pXORFACTORState->pTransition;
    }
  }
}

void FreeInferenceStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {
  DdNode *pInferenceBDD = ((InferenceStateEntry *)pTypeState)->pInferenceBDD;
  if(pInferenceBDD != NULL) {
    if(Cudd_IsComplement(pInferenceBDD)) {
      Cudd_Not(pInferenceBDD)->pStateNeg = NULL;
    } else {
      pInferenceBDD->pStatePos = NULL;
    }
    Cudd_IterDerefBdd(SM->dd, pInferenceBDD);
  }
}
