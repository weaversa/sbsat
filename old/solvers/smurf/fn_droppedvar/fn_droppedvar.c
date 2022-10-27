#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

// DroppedVar State
void initDroppedVarStateType() {
  arrStatesTypeSize[FN_DROPPEDVAR] = ComputeDroppedVarStateSize;
  arrSetStateVisitedFlag[FN_DROPPEDVAR] = SetDroppedVarStateVisitedFlag;
  arrUnsetStateVisitedFlag[FN_DROPPEDVAR] = UnsetDroppedVarStateVisitedFlag;
  arrCleanUpStateEntry[FN_DROPPEDVAR] = CleanUpDroppedVarState;
  arrPrintStateEntry[FN_DROPPEDVAR] = PrintDroppedVarStateEntry;
  arrPrintStateEntry_dot[FN_DROPPEDVAR] = PrintDroppedVarStateEntry_dot;
  arrFreeStateEntry[FN_DROPPEDVAR] = FreeDroppedVarStateEntry;
}

uintmax_t ComputeDroppedVarStateSize(TypeStateEntry *pTypeState) {
  return (uintmax_t)sizeof(DroppedVarStateEntry);
}

void PrintDroppedVarStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {
  DroppedVarStateEntry *pDroppedVarState = (DroppedVarStateEntry *)pTypeState;
  fprintf(stdout, "DV %p, v=%ju, next=%p\n",
	  (void *)pDroppedVarState,
	  pDroppedVarState->nVar,
	  (void *)pDroppedVarState->pTransition);
}

uint8_t print_dropped_vars = 1;

void PrintDroppedVarStateEntry_dot(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t *pIndex2Var) {
  if(pTypeState->visited == 1) return;
  pTypeState->visited = 1;

  DroppedVarStateEntry *pDroppedVarState = (DroppedVarStateEntry *)pTypeState;
  fprintf(stdout, " b%p:s->b%p [style=solid,tailclip=false,label=",
	  (void *)pDroppedVarState,
	  (void *)pDroppedVarState->pTransition);
  if(print_dropped_vars) {
    fprintf(stdout, "\"%s\"]\n", s_name(pIndex2Var[pDroppedVarState->nVar]));
  } else fprintf(stdout, "\"\"]\n");

  fprintf(stdout, " b%p [shape=\"invtriangle\",label=\"D\"]\n", (void *)pDroppedVarState);
}

void PrintDroppedVarChain_dot(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t *pIndex2Var) {
  DroppedVarStateEntry *pDroppedVarState = (DroppedVarStateEntry *)pTypeState;
  while(pDroppedVarState && pDroppedVarState->type == FN_DROPPEDVAR) {
    if(print_dropped_vars) fprintf(stdout, " %s", s_name(pIndex2Var[pDroppedVarState->nVar]));
    pDroppedVarState = (DroppedVarStateEntry *)pDroppedVarState->pTransition;
  }
}

void FreeDroppedVarStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {

}
