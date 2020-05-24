#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

void SetMINMAXStateVisitedFlag(TypeStateEntry *pTypeState) {
  MINMAXStateEntry *pMINMAXState = (MINMAXStateEntry *)pTypeState;
  assert(pMINMAXState->type == FN_MINMAX);

  if(pMINMAXState->visited==0) {
    pMINMAXState->visited = 1;
  }
}

void UnsetMINMAXStateVisitedFlag(TypeStateEntry *pTypeState) {
  MINMAXStateEntry *pMINMAXState = (MINMAXStateEntry *)pTypeState;
  assert(pMINMAXState->type == FN_MINMAX);

  if(pMINMAXState->visited==1) {
    pMINMAXState->visited = 0;
  }
}

void CleanUpMINMAXState(TypeStateEntry *pTypeState) {
  MINMAXStateEntry *pMINMAXState = (MINMAXStateEntry *)pTypeState;
  assert(pMINMAXState->type == FN_MINMAX);
}
