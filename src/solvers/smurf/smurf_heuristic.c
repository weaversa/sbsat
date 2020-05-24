#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

inline
double get_heuristic_score_for_var(SmurfManager *SM, uintmax_t nVariable, uint8_t bPolarity) {
  double score = 0.0;
  
  SmurfOccurrenceStruct_list pSmurfOccurrences = SM->arrVariableOccurrences.pList[nVariable];
  for(uintmax_t i = 0; i < pSmurfOccurrences.nLength; i++) {
    SmurfOccurrenceStruct pSmurfOccurrence = pSmurfOccurrences.pList[i];
    TypeStateEntry *pTypeState = pSmurfOccurrence.pSmurfInfo->pCurrentState;
    uintmax_t nVbleIndex = pSmurfOccurrence.nVbleIndex;
    score += arrGetVarHeuristicScore[pTypeState->type](SM, pTypeState, nVbleIndex, bPolarity);
  }

  d9_printf4("Heuristic score for %ju = %d is %lf\n", nVariable, bPolarity, score);

  return score;
}

inline
void populateLSGBScores(SmurfManager *SM) {
  //Way #1
  memset(SM->arrPosVarHeurWghts, 0, sizeof(double)*SM->nNumVariables);
  memset(SM->arrNegVarHeurWghts, 0, sizeof(double)*SM->nNumVariables);
  //TypeStateEntry **arrSmurfStates = SM->arrSmurfStack[SM->nCurrentDecisionLevel].arrSmurfStates;
  for(uintmax_t i = 0; i < SM->nNumSmurfs; i++) {
    //if(arrSmurfStates[i] == (TypeStateEntry *)SM->pTrueSmurfState) continue;
    SmurfInfoStruct *pSmurfInfo = &SM->arrSmurfStates.pList[i];
    if(pSmurfInfo->pCurrentState == (TypeStateEntry *)SM->pTrueSmurfState) continue;
    //pSmurfInfo->pCurrentState = arrSmurfStates[i];
    arrCalculateStateHeuristic[pSmurfInfo->pCurrentState->type](SM, pSmurfInfo);
  }
  
  //Way #2
  //for(uintmax_t i = 0; i <= SM->nNumVariables; i++) {
  //SM->arrPosVarHeurWghts[i] = get_heuristic_score_for_var(SM, i, 1);
  //SM->arrNegVarHeurWghts[i] = get_heuristic_score_for_var(SM, i, 0);
  //}
}

inline
void SmurfStateHeuristicUpdate(SmurfManager *SM, TypeStateEntry *pOldState, SmurfInfoStruct *pSmurfInfo) {
  TypeStateEntry *pCurrentState = pSmurfInfo->pCurrentState;
  if(pOldState == pCurrentState) return;

  double_undoer *VarHeurWghts_undoer = &SM->VarHeurWghts_undoer;

  double *arrPosVarHeurWghts = SM->arrPosVarHeurWghts;
  double *arrNegVarHeurWghts = SM->arrNegVarHeurWghts;
  uintmax_t *pIndex2Var = pSmurfInfo->pIndex2Var;
  uint32_t nNumVariables = pSmurfInfo->nNumVariables;

  // Save old heuristic influence
  // Remove the old state's heuristic influence and add the new state's heuristic influence
  for(uintmax_t i = 0; i < nNumVariables; i++) {
    if(SM->pTrail[pIndex2Var[i]] == 0) {
      uintmax_t nVar = pIndex2Var[i];
      double score = arrGetVarHeuristicScore[pOldState->type](SM, pOldState, i, 1);
      if(score > 0.0) {
	score -= arrGetVarHeuristicScore[pCurrentState->type](SM, pCurrentState, i, 1);
	double_undoer_push(VarHeurWghts_undoer, &arrPosVarHeurWghts[nVar]);
	arrPosVarHeurWghts[nVar] -= score;
	
	double_undoer_push(VarHeurWghts_undoer, &arrNegVarHeurWghts[nVar]);
	arrNegVarHeurWghts[nVar] += arrGetVarHeuristicScore[pCurrentState->type](SM, pCurrentState, i, 0) - arrGetVarHeuristicScore[pOldState->type](SM, pOldState, i, 0);
      }
    }
  }
}

//int (*SmurfSolverHeuristic) () = NULL;

inline
intmax_t LSGBHeuristic(SmurfManager *SM) {
  int nBestVble = 0;
  double fMaxWeight = 0.0;
  double fVbleWeight;

  populateLSGBScores(SM);
  
  // Determine the variable with the highest weight:
  // 
  // Initialize to the lowest indexed variable whose value is uninstantiated.

  for(uintmax_t i = SM->nHeuristicPlaceholder; i <= SM->nNumVariables; i++) {
    if((SM->pTrail[i]==0) && (SM->arrVariableOccurrences.pList[i].nLength!=0)) {
      d9_printf4("Heuristic score for %ju = %d is %lf\n", i, 1, SM->arrPosVarHeurWghts[i]);
      d9_printf4("Heuristic score for %ju = %d is %lf\n", i, 0, SM->arrNegVarHeurWghts[i]);
      nBestVble = i;
      fMaxWeight = (1.0 + SM->arrPosVarHeurWghts[i]) * (1.0 + SM->arrNegVarHeurWghts[i]);
      SM->nHeuristicPlaceholder = i;
      break;
    }
  }
  
  // Search through the remaining uninstantiated variables.
  for(uintmax_t i = nBestVble + 1; i <= SM->nNumVariables; i++) {
    if((SM->pTrail[i]==0) && (SM->arrVariableOccurrences.pList[i].nLength!=0)) {
      d9_printf4("Heuristic score for %ju = %d is %lf\n", i, 1, SM->arrPosVarHeurWghts[i]);
      d9_printf4("Heuristic score for %ju = %d is %lf\n", i, 0, SM->arrNegVarHeurWghts[i]);
      fVbleWeight = (1.0 + SM->arrPosVarHeurWghts[i]) *	(1.0 + SM->arrNegVarHeurWghts[i]);
      if(fVbleWeight > fMaxWeight) {
	fMaxWeight = fVbleWeight;
	nBestVble = i;
      }
    }
  }

  if(nBestVble != 0) return (SM->arrPosVarHeurWghts[nBestVble] >= SM->arrNegVarHeurWghts[nBestVble])?nBestVble:-nBestVble;

  for(uintmax_t i = SM->nHeuristicPlaceholder; i <= SM->nNumVariables; i++) {
    if(SM->pTrail[i]==0) {
      d9_printf4("Heuristic score for %ju = %d is %lf\n", i, 1, SM->arrPosVarHeurWghts[i]);
      d9_printf4("Heuristic score for %ju = %d is %lf\n", i, 0, SM->arrNegVarHeurWghts[i]);
      nBestVble = i;
      fMaxWeight = (1.0 + SM->arrPosVarHeurWghts[i]) * (1.0 + SM->arrNegVarHeurWghts[i]);
      SM->nHeuristicPlaceholder = i;
      break;
    }
  }
  
  // Search through the remaining uninstantiated variables.
  for(uintmax_t i = nBestVble + 1; i <= SM->nNumVariables; i++) {
    if(SM->pTrail[i]==0) {
      d9_printf4("Heuristic score for %ju = %d is %lf\n", i, 1, SM->arrPosVarHeurWghts[i]);
      d9_printf4("Heuristic score for %ju = %d is %lf\n", i, 0, SM->arrNegVarHeurWghts[i]);
      fVbleWeight = (1.0 + SM->arrPosVarHeurWghts[i]) *	(1.0 + SM->arrNegVarHeurWghts[i]);
      if(fVbleWeight > fMaxWeight) {
	fMaxWeight = fVbleWeight;
	nBestVble = i;
      }
    }
  }

  return (SM->arrPosVarHeurWghts[nBestVble] >= SM->arrNegVarHeurWghts[nBestVble])?nBestVble:-nBestVble;
}

inline
intmax_t NULLHeuristic(SmurfManager *SM) {
  // Initialize to the lowest indexed variable whose value is uninstantiated.
  uintmax_t nBestVble = 0;
  
  for(uintmax_t i = SM->nHeuristicPlaceholder; i <= SM->nNumVariables; i++) {
    if((SM->pTrail[i]==0) && (SM->arrVariableOccurrences.pList[i].nLength!=0)) {
      nBestVble = i;
      SM->nHeuristicPlaceholder = i;
      return -nBestVble;
    }
  }

  for(uintmax_t i = 0; i <= SM->nNumVariables; i++) {
    if(SM->pTrail[i]==0) {
      nBestVble = i;
      SM->nHeuristicPlaceholder = i;
      break;
    }
  }
  
  return -nBestVble;
}

inline
intmax_t StaticHeuristic(SmurfManager *SM) {
  // Initialize to the lowest ordered variable whose value is uninstantiated.
  uintmax_t nBestVble = 0;

  for(uintmax_t i = SM->nHeuristicPlaceholder; i <= SM->nNumVariables; i++) {
    nBestVble = SM->pStaticHeurOrder[i];
    if(nBestVble == 0) continue;
    if((SM->pTrail[nBestVble]==0) && (SM->arrVariableOccurrences.pList[nBestVble].nLength!=0)) {
      SM->nHeuristicPlaceholder = i;
      return -nBestVble;
    }
  }

  for(uintmax_t i = 0; i <= SM->nNumVariables; i++) {
    nBestVble = SM->pStaticHeurOrder[i];
    if(nBestVble == 0) continue;
    if(SM->pTrail[nBestVble]==0) {
      SM->nHeuristicPlaceholder = i;
      break;
    }
  }

  return -nBestVble;
}

inline //Well, it's not great random...but it's something
intmax_t RandomHeuristic(SmurfManager *SM) {
  //intmax_t nBestVble = (random()%SM->nNumVariables)+1;
  intmax_t nBestVble = SM->nNumVariables;
  intmax_t loop = nBestVble;
  //uint8_t dir = random()%2;
  uint8_t dir = 0;

  while(1) {
    if(SM->pTrail[nBestVble]==0) {
      return random()%2?nBestVble:-nBestVble;
    }
    if(dir) {
      if(++nBestVble > SM->nNumVariables) nBestVble = 1;
    } else {
      if(--nBestVble == 0) nBestVble = SM->nNumVariables;
    }
    if(nBestVble == loop) break;
  }

  return 0;
}
