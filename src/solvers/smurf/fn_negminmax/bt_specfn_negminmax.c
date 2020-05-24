#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

void SetNEGMINMAXStateVisitedFlag(TypeStateEntry *pTypeState) {
  NEGMINMAXStateEntry *pNEGMINMAXState = (NEGMINMAXStateEntry *)pTypeState;
  assert(pNEGMINMAXState->type == FN_NEGMINMAX);

  if(pNEGMINMAXState->visited==0) {
    pNEGMINMAXState->visited = 1;
  }
}

void UnsetNEGMINMAXStateVisitedFlag(TypeStateEntry *pTypeState) {
  NEGMINMAXStateEntry *pNEGMINMAXState = (NEGMINMAXStateEntry *)pTypeState;
  assert(pNEGMINMAXState->type == FN_NEGMINMAX);

  if(pNEGMINMAXState->visited==1) {
    pNEGMINMAXState->visited = 0;
  }
}

void CleanUpNEGMINMAXState(TypeStateEntry *pTypeState) {
  NEGMINMAXStateEntry *pNEGMINMAXState = (NEGMINMAXStateEntry *)pTypeState;
  assert(pNEGMINMAXState->type == FN_NEGMINMAX);
}
