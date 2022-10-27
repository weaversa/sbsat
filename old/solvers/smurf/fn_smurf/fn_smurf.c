#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

// Smurf State

void initSmurfStateType() {
  arrGetStateType[FN_SMURF] = SM_NODE_TYPE;

  arrCreateStateEntry[FN_SMURF] = CreateSmurfState;
  arrStatesTypeSize[FN_SMURF] = ComputeSmurfStateSize;
  arrSetStateVisitedFlag[FN_SMURF] = SetSmurfStateVisitedFlag;
  arrUnsetStateVisitedFlag[FN_SMURF] = UnsetSmurfStateVisitedFlag;
  arrCleanUpStateEntry[FN_SMURF] = CleanUpSmurfState;
  arrApplyInferenceToState[FN_SMURF] = ApplyInferenceToSmurf;
  arrPrintStateEntry[FN_SMURF] = PrintSmurfStateEntry;
  arrPrintStateEntry_dot[FN_SMURF] = _PrintSmurf_dot;
  arrFreeStateEntry[FN_SMURF] = FreeSmurfStateEntry;
  arrCalculateStateHeuristic[FN_SMURF] = CalculateSmurfLSGBHeuristic;
  //arrSetStateHeuristicScore[FN_SMURF] = LSGBSmurfSetHeurScore;
  arrGetStateHeuristicScore[FN_SMURF] = LSGBSumNodeWeights;
  arrGetVarHeuristicScore[FN_SMURF] = LSGBVarWeight;
  arrFreeHeuristicScore[FN_SMURF] = LSGBSmurfFree;
}

uintmax_t ComputeSmurfStateSize(TypeStateEntry *pTypeState) {
  SmurfStateEntry *pSmurfState = (SmurfStateEntry *)pTypeState;
  return (uintmax_t)sizeof(SmurfStateEntry) + (sizeof(SmurfStateTransition) * pSmurfState->nNumVariables);
}

void PrintSmurfStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {
  SmurfStateEntry *pSmurfState = (SmurfStateEntry *)pTypeState;
  if(pSmurfState == SM->pTrueSmurfState) {
    fprintf(stdout, "T  %p, refs=%d\n", (void *)pSmurfState, Cudd_Regular(pSmurfState->pSmurfBDD)->ref);
  } else {    
    SmurfStateTransition *pSmurfTransition = (SmurfStateTransition *)(pSmurfState + 1);
    for(uintmax_t i = 0; i < pSmurfState->nNumVariables; i++) {
      fprintf(stdout, "SM %p, v=%ju, v=T:%p, v=F:%p, refs=%d\n",
		 (void *)pSmurfState,
                 i,
		 (void *)pSmurfTransition->pVarIsTrueTransition,
		 (void *)pSmurfTransition->pVarIsFalseTransition,
		 Cudd_Regular(pSmurfState->pSmurfBDD)->ref);
      pSmurfTransition++;
    }
  }
}

uint8_t print_smurf_transitions = 1;

void PrintSmurfStateEntry_dot(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t *pIndex2Var) {
  SmurfStateEntry *pSmurfState = (SmurfStateEntry *)pTypeState;
  if(pSmurfState == SM->pTrueSmurfState) {
    fprintf(stdout, " b%p [shape=box, fontname=""Helvetica"",label=""True""];\n", (void *)pSmurfState);
  } else {    
    fprintf(stdout, " b%p [shape=\"ellipse\", label=\"", (void *)pSmurfState);

    uintmax_t old_ignore_limits = ignore_limits;
    ignore_limits = 1;
    //Lift normalized BDD to pretty print the state label
    for(uintmax_t i = 0; i < pSmurfState->nNumVariables; i++) {
      uint32_t_list_add(&SM->uTempList32, i, pIndex2Var[i]);
    }
    DdNode *lifted_bdd = Cudd_bddPermute(SM->dd, pSmurfState->pSmurfBDD, (int *)SM->uTempList32.pList);
    Cudd_Ref(lifted_bdd);
    printBDD_ReduceSpecFunc_latex(SM->BM, lifted_bdd, stdout);
    Cudd_IterDerefBdd(SM->dd, lifted_bdd);
    ignore_limits = old_ignore_limits;

    fprintf(stdout, "\"];\n");

    SmurfStateTransition *pSmurfTransition = (SmurfStateTransition *)(pSmurfState + 1);
    for(uintmax_t i = 0; i < pSmurfState->nNumVariables; i++) {
      TypeStateEntry *pNextState = pSmurfTransition->pVarIsTrueTransition;
      if(pNextState != pTypeState) { //Don't print loops
	fprintf(stdout, " b%p->b%p [style=solid,label=",
		(void *)pSmurfState,
		(void *)pSmurfTransition->pVarIsTrueTransition);
	if(print_smurf_transitions) {
	  fprintf(stdout, "\"%s\"]\n", s_name(pIndex2Var[i]));
	} else fprintf(stdout, "\"\"]\n");
      }	

      pNextState = pSmurfTransition->pVarIsFalseTransition;
      if(pNextState != pTypeState) { //Don't print loops
	fprintf(stdout, " b%p->b%p [style=dotted,label=",
		(void *)pSmurfState,
		(void *)pSmurfTransition->pVarIsFalseTransition);
	if(print_smurf_transitions) {
	  fprintf(stdout, "\"\\overline{%s}\"]\n", s_name(pIndex2Var[i]));
	} else fprintf(stdout, "\"\"]\n");
      }

      pSmurfTransition++;
    }
  }
}

void PrintSmurfStateEntry_inferences_compressed_dot(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t *pIndex2Var) {
  SmurfStateEntry *pSmurfState = (SmurfStateEntry *)pTypeState;
  if(pSmurfState == SM->pTrueSmurfState) {
    fprintf(stdout, " b%p [shape=box, fontname=""Helvetica"",label=""True""];\n", (void *)pSmurfState);
  } else {    
    fprintf(stdout, " b%p [shape=\"ellipse\", label=\"", (void *)pSmurfState);

    uintmax_t old_ignore_limits = ignore_limits;
    ignore_limits = 1;    //Lift normalized BDD to pretty print the state label
    for(uintmax_t i = 0; i < pSmurfState->nNumVariables; i++) {
      uint32_t_list_add(&SM->uTempList32, i, pIndex2Var[i]);
    }
    DdNode *lifted_bdd = Cudd_bddPermute(SM->dd, pSmurfState->pSmurfBDD, (int *)SM->uTempList32.pList);
    Cudd_Ref(lifted_bdd);
    printBDD_ReduceSpecFunc_latex(SM->BM, lifted_bdd, stdout);
    Cudd_IterDerefBdd(SM->dd, lifted_bdd);
    ignore_limits = old_ignore_limits;

    fprintf(stdout, "\"];\n");

    SmurfStateTransition *pSmurfTransition = (SmurfStateTransition *)(pSmurfState + 1);
    for(uintmax_t i = 0; i < pSmurfState->nNumVariables; i++) {
      TypeStateEntry *pNextState = pSmurfTransition->pVarIsTrueTransition;
      if(pNextState != pTypeState) { //Don't print loops
	while(pNextState && (pNextState->type == FN_INFERENCE ||
			     pNextState->type == FN_DROPPEDVAR ||
			     pNextState->type == FN_XORFACTOR)) {
	  if(pNextState->type == FN_INFERENCE) {
	    pNextState = ((InferenceStateEntry *)pNextState)->pTransition;
	  } else if(pNextState->type == FN_DROPPEDVAR) {
	    pNextState = ((DroppedVarStateEntry *)pNextState)->pTransition;
	  } else {
	    assert(pNextState->type == FN_XORFACTOR);
	    pNextState = ((XORFACTORStateEntry *)pNextState)->pTransition;
	  }
	}
	fprintf(stdout, " b%p->b%p [style=solid,label=",
		(void *)pSmurfState,
		(void *)pNextState);
	if(print_smurf_transitions) {
	  fprintf(stdout, "\"%s", s_name(pIndex2Var[i]));
	} else fprintf(stdout, "\"");

	pNextState = pSmurfTransition->pVarIsTrueTransition;
	
	PrintInferenceChain_dot(SM, pNextState, pIndex2Var);
	fprintf(stdout, "\"]\n");
      }

      if(pNextState != pTypeState) { //Don't print loops
	pNextState = pSmurfTransition->pVarIsFalseTransition;
	while(pNextState && (pNextState->type == FN_INFERENCE ||
			     pNextState->type == FN_DROPPEDVAR ||
			     pNextState->type == FN_XORFACTOR)) {
	  if(pNextState->type == FN_INFERENCE) {
	    pNextState = ((InferenceStateEntry *)pNextState)->pTransition;
	  } else if(pNextState->type == FN_DROPPEDVAR) {
	    pNextState = ((DroppedVarStateEntry *)pNextState)->pTransition;
	  } else {
	    assert(pNextState->type == FN_XORFACTOR);
	    pNextState = ((XORFACTORStateEntry *)pNextState)->pTransition;
	  }
	}
	fprintf(stdout, " b%p->b%p [style=dotted,label=",
		(void *)pSmurfState,
		(void *)pNextState);
	if(print_smurf_transitions) {
	  fprintf(stdout, "\"\\overline{%s}", s_name(pIndex2Var[i]));
	} else fprintf(stdout, "\"");
	
	pNextState = pSmurfTransition->pVarIsFalseTransition;
	
	PrintInferenceChain_dot(SM, pNextState, pIndex2Var);
	fprintf(stdout, "\"]\n");
      }

      pSmurfTransition++;
    }
  }
}

void FreeSmurfStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {
  DdNode *pSmurfBDD = ((SmurfStateEntry *)pTypeState)->pSmurfBDD;
  if(pSmurfBDD != NULL) {
    if(Cudd_IsComplement(pSmurfBDD)) {
      Cudd_Not(pSmurfBDD)->pStateNeg = NULL;
    } else {
      pSmurfBDD->pStatePos = NULL;
    }
    Cudd_IterDerefBdd(SM->dd, pSmurfBDD);
  }
}
