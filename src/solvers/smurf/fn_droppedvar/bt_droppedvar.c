#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

void SetDroppedVarStateVisitedFlag(TypeStateEntry *pTypeState) {
  DroppedVarStateEntry *pDroppedVarState = (DroppedVarStateEntry *)pTypeState;
  assert(pDroppedVarState->type == FN_DROPPEDVAR);

  while(pDroppedVarState != NULL &&
        pDroppedVarState->type == FN_DROPPEDVAR &&
        pDroppedVarState->visited == 0) {
    pDroppedVarState->visited = 1;
    pDroppedVarState = (DroppedVarStateEntry *)pDroppedVarState->pTransition;
  }
  if(pDroppedVarState != NULL && pDroppedVarState->type != FN_SMURF && pDroppedVarState->visited != 1)
    arrSetStateVisitedFlag[pDroppedVarState->type]((TypeStateEntry *)pDroppedVarState);
}

void UnsetDroppedVarStateVisitedFlag(TypeStateEntry *pTypeState) {
  DroppedVarStateEntry *pDroppedVarState = (DroppedVarStateEntry *)pTypeState;
  assert(pDroppedVarState->type == FN_DROPPEDVAR);

  while(pDroppedVarState != NULL &&
        pDroppedVarState->type == FN_DROPPEDVAR &&
        pDroppedVarState->visited == 1) {
    pDroppedVarState->visited = 0;
    pDroppedVarState = (DroppedVarStateEntry *)pDroppedVarState->pTransition;
  }
  if(pDroppedVarState != NULL && pDroppedVarState->type != FN_SMURF && pDroppedVarState->visited != 0)
    arrUnsetStateVisitedFlag[pDroppedVarState->type]((TypeStateEntry *)pDroppedVarState);
}

void CleanUpDroppedVarState(TypeStateEntry *pTypeState) {
  DroppedVarStateEntry *pDroppedVarState = (DroppedVarStateEntry *)pTypeState;
  assert(pDroppedVarState->type == FN_DROPPEDVAR);

  if(pDroppedVarState->visited == 0) return;
  
  if(pDroppedVarState->pTransition && pDroppedVarState->pTransition->visited == 0)
    pDroppedVarState->pTransition = NULL;
}
