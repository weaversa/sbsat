#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

// XORFACTOR State

void initXORFACTORStateType() {
  arrGetStateType[FN_XORFACTOR] = SM_TRANSITION_TYPE;

  arrCreateStateEntry[FN_XORFACTOR] = CreateXORFACTORState;
  arrStatesTypeSize[FN_XORFACTOR] = ComputeXORFACTORStateSize;
  arrSetStateVisitedFlag[FN_XORFACTOR] = SetXORFACTORStateVisitedFlag;
  arrUnsetStateVisitedFlag[FN_XORFACTOR] = UnsetXORFACTORStateVisitedFlag;
  arrCleanUpStateEntry[FN_XORFACTOR] = CleanUpXORFACTORState;

  arrApplyInferenceToState[FN_XORFACTOR] = ApplyInferenceToXORFACTOR;

  arrPrintStateEntry[FN_XORFACTOR] = PrintXORFACTORStateEntry;
  arrPrintStateEntry_dot[FN_XORFACTOR] = PrintXORFACTORStateEntry_dot;
  arrFreeStateEntry[FN_XORFACTOR] = FreeXORFACTORStateEntry;
  arrCalculateStateHeuristic[FN_XORFACTOR] = CalculateXORFACTORStateLSGBHeuristic;
  //arrSetStateHeuristicScore[FN_XORFACTOR] = LSGBXORFACTORStateSetHeurScore;
  arrGetStateHeuristicScore[FN_XORFACTOR] = LSGBXORFACTORStateGetHeurScore;
  arrGetVarHeuristicScore[FN_XORFACTOR] = LSGBXORFACTORStateGetVarHeurScore;
  arrFreeHeuristicScore[FN_XORFACTOR] = LSGBXORFACTORStateFree;
}

uintmax_t ComputeXORFACTORStateSize(TypeStateEntry *pTypeState) {
  XORFACTORStateEntry *pXORFACTORState = (XORFACTORStateEntry *)pTypeState;
  assert(pXORFACTORState->type == FN_XORFACTOR);
  return sizeof(XORFACTORStateEntry);
}

void PrintXORFACTORStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {
  XORFACTORStateEntry *pXORFACTORState = (XORFACTORStateEntry *)pTypeState;
  assert(pXORFACTORState->type == FN_XORFACTOR);
  fprintf(stdout, "XF %p, xor=%p, next=%p, refs=%d\n", (void *)pXORFACTORState, (void *)pXORFACTORState->pTransition, (void *)pXORFACTORState->pXORFactor, Cudd_Regular(pXORFACTORState->pXORFACTORBDD)->ref);
}

void PrintXORFACTORStateEntry_dot(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t *pIndex2Var) {
  if(pTypeState->visited == 1) return;
  pTypeState->visited = 1;
  XORFACTORStateEntry *pXORFACTORState = (XORFACTORStateEntry *)pTypeState;

  fprintf(stdout, " b%p [shape=\"ellipse\",label=\"", (void *)pXORFACTORState);

  BDD_Support(&SM->uTempList1.nLength, &(SM->uTempList1.nLength_max), &(SM->uTempList1.pList), pXORFACTORState->pXORFACTORBDD);
  //Just need the length :-)
  
  uintmax_t old_ignore_limits = ignore_limits;
  ignore_limits = 1;
  //Lift normalized BDD to pretty print the state label
  for(uintmax_t i = 0; i < SM->uTempList1.nLength; i++) {
    uint32_t_list_add(&SM->uTempList32, i, pIndex2Var[i]);
  }
  DdNode *lifted_bdd = Cudd_bddPermute(SM->dd, pXORFACTORState->pXORFACTORBDD, (int *)SM->uTempList32.pList);
  Cudd_Ref(lifted_bdd);
  printBDD_ReduceSpecFunc_latex(SM->BM, lifted_bdd, stdout);
  Cudd_IterDerefBdd(SM->dd, lifted_bdd);
  ignore_limits = old_ignore_limits;

  fprintf(stdout, "\"];\n");

  //Print the factor
  pTypeState = (TypeStateEntry *)pXORFACTORState->pXORFactor;
  TypeStateEntry *pNextState = pTypeState;
  //The printer will display the inferences and dropped vars in a compressed
  //form, so we skip them here.
  while(pNextState && (pNextState->type == FN_INFERENCE || pNextState->type == FN_DROPPEDVAR)) {
    if(pNextState->type == FN_INFERENCE)
      pNextState = ((InferenceStateEntry *)pNextState)->pTransition;
    else {
      assert(pNextState->type == FN_DROPPEDVAR);
      pNextState = ((DroppedVarStateEntry *)pNextState)->pTransition;
    }
  }
  fprintf(stdout, " b%p->b%p [style=dashed, label=\" factor ", (void *)pXORFACTORState, (void *)pNextState);
  PrintInferenceChain_dot(SM, pTypeState, pIndex2Var);
  fprintf(stdout, "\"]\n");
  _PrintSmurf_dot(SM, pNextState, pIndex2Var);


  //Print the rest of the Smurf
  pTypeState = pXORFACTORState->pTransition;
  pNextState = pTypeState;
  //The printer will display the inferences and dropped vars in a compressed
  //form, so we skip them here.
  while(pNextState && (pNextState->type == FN_INFERENCE || pNextState->type == FN_DROPPEDVAR)) {
    if(pNextState->type == FN_INFERENCE)
      pNextState = ((InferenceStateEntry *)pNextState)->pTransition;
    else {
      assert(pNextState->type == FN_DROPPEDVAR);
      pNextState = ((DroppedVarStateEntry *)pNextState)->pTransition;
    }
  }
  fprintf(stdout, " b%p->b%p [style=solid, label=\"", (void *)pXORFACTORState, (void *)pNextState);
  PrintInferenceChain_dot(SM, pTypeState, pIndex2Var);
  fprintf(stdout, "\"]\n");
  _PrintSmurf_dot(SM, pNextState, pIndex2Var);
}

void FreeXORFACTORStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {
  DdNode *pXORFACTORBDD = ((XORFACTORStateEntry *)pTypeState)->pXORFACTORBDD;
  if(pXORFACTORBDD != NULL) {
    if(Cudd_IsComplement(pXORFACTORBDD)) {
      Cudd_Not(pXORFACTORBDD)->pStateNeg = NULL;
    } else {
      pXORFACTORBDD->pStatePos = NULL;
    }
    Cudd_IterDerefBdd(SM->dd, pXORFACTORBDD);
  }
}
