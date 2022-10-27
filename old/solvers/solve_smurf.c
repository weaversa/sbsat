#include "sbsat.h"
#include "solver.h"

char solver_flag = 's';

//Functions that SBSAT needs defined
inline
uint8_t IsVariableAssigned_smurf(SmurfManager *SM, uintmax_t nVariable) {
  return SM->pTrail[nVariable] != 0;
}

inline
uint8_t EnqueueInference_smurf(SmurfManager *SM, SmurfInfoStruct *pSmurfInfo, uintmax_t nVariable, uint8_t bPolarity) {
  /*
  funcsat *FS = (funcsat *)SM->ESSM;
  d7_printf4("      Inferring %jd at level %ju to level %u\n",
	     bPolarity?(intmax_t)nVariable:-(intmax_t)nVariable, FS->propq, FS->trail.size);

  literal lit = bPolarity?(literal)nVariable:-(literal)nVariable;
  mbool previous_lit_value;
  if (FS->level.data[nVariable] == -1) previous_lit_value = unknown;
  else previous_lit_value = (lit == FS->trail.data[FS->model.data[nVariable]]);

  if(previous_lit_value == true) {
    //Value is already inferred the correct value
    d7_printf2("      Inference %jd already inferred\n", bPolarity?(intmax_t)nVariable:-(intmax_t)nVariable);
    return NO_ERROR;
  } else if(previous_lit_value == false) {
    //Conflict Detected
    d7_printf2("      Conflict when adding %jd to the inference queue\n", bPolarity?(intmax_t)nVariable:-(intmax_t)nVariable);

    return UNSAT; //SEAN!!! Make a BACKTRACK return type or something
  } else if(previous_lit_value == unknown) {
    //Inference is not in inference queue, insert it.
    
    trailPush(FS, lit, UINTPTR_MAX);
  }
  */
  return NO_ERROR;
}

uint8_t EnqueueInference_smurf_idx(SmurfManager *SM, SmurfInfoStruct *pSmurfInfo, uintmax_t nVbleIndex, uint8_t bPolarity) {
  return EnqueueInference_smurf(SM, pSmurfInfo, pSmurfInfo->pIndex2Var[nVbleIndex], bPolarity);
}

static inline
uint8_t SmurfBCP(SmurfManager *SM) {
  uint8_t ret = NO_ERROR;
  /*
  funcsat *FS = (funcsat *)SM->ESSM;
  while (FS->propq < FS->trail.size) {
    literal l = FS->trail.data[FS->propq];
    if((ret = ApplyNextInferenceToStates((uintptr_t *)SM, imaxabs(l), l > 0)) != FS_UNKNOWN)
      break;
    FS->propq++;
  }
  */
  return ret;
}

uint8_t SmurfSolve(SmurfManager *SM) {
  /*
  funcsat *FS = (funcsat *)SM->ESSM;

  if (FS_UNSAT == (FS->lastResult = startSolving(FS))) goto Done;

  //Push inital inferences
  for(uintmax_t i = 0; i < SM->nNumSmurfs; i++) {
    TypeStateEntry *pTypeState = SM->arrSmurfStates.pList[i].pCurrentState;
    if(arrGetStateType[pTypeState->type] == SM_TRANSITION_TYPE) {
      d9_printf2("Following initial transition of Smurf %ju\n", i);
      if(arrApplyInferenceToState[pTypeState->type](SM, 0, 0, &SM->arrSmurfStates.pList[i]) == UNSAT) {
	d9_printf1("Formula found unsat via initial transitions\n");
	goto Unsat;
      }
    }
  }

  if (SmurfBCP(SM) == FS_UNSAT) {
    d9_printf1("Formula found unsat via initial BCP\n");
    goto Unsat;
  }

  while (!FS->conf->isResourceLimitHit(FS, FS->conf->user)) {
    if (SmurfBCP(SM) == FS_UNSAT) {
      if (0 == FS->decisionLevel) goto Unsat;

      bool foundDecision = 0;
      literal flipme;
      while (!foundDecision) {
	foundDecision = isDecision(FS, fs_lit2var(flipme = clausePeek(&FS->trail)));
	trailPop(FS, NULL);
      }
      trailPush(FS, -flipme, UINTPTR_MAX);

      continue;
    }

    intmax_t decision;
    if(sHeuristic == 'j') decision = LSGBHeuristic(SM);
    else if(sHeuristic == 's') decision = StaticHeuristic(SM);
    else if(sHeuristic == 'r') decision = RandomHeuristic(SM);
    else decision = NULLHeuristic(SM);

    if(decision == 0) {
      FS->lastResult = FS_SAT;
      goto Done;
    }

    ++FS->numDecisions;//, FS->pctdepth /= 2.0, FS->correction = 1;
    trailPush(FS, decision, UINTPTR_MAX);
    FS->level.data[imaxabs(decision)] = (intmax_t)++FS->decisionLevel;
    if(FS->MakeDecision_hook) FS->MakeDecision_hook((uintptr_t *)FS->conf->user);
    FS->decisions.data[imaxabs(decision)] = FS->decisionLevel;

    d9_printf2("Heuristic choosing: %jd\n", clausePeek(&FS->trail));
  }

Unsat:
  FS->lastResult = FS_UNSAT;

Done:
  assert(FS->lastResult != FS_SAT || FS->trail.size == FS->numVars);

  if(FS->lastResult == FS_SAT) {
    //Potentially print solution

    if(0) {
      fprintf(foutputfile, "v ");
      for(intmax_t i = 1; i < FS->trail.size; i++) {
	uint8_t lit_pos = FS->trail.data[FS->model.data[i]]>0;
	if(lit_pos) {
	  fprintf(foutputfile, "%s ", s_name(i));
	} else {
	  fprintf(foutputfile, "-%s ", s_name(i));
	}
      }
      fprintf(foutputfile, "0\n");
    } else {
      for(intmax_t i = 1; i < FS->trail.size; i++) {
	uint8_t lit_pos = FS->trail.data[FS->model.data[i]]>0;
	if(lit_pos) {
	  fprintf(foutputfile, "1");
	} else {
	  fprintf(foutputfile, "0");
	}
      }
      fprintf(foutputfile, "\n");
    }
  }

  return FS->lastResult;
  */
}

//This initializes a few things then calls the main initialization function - ReadAllSmurfsIntoTable();
SmurfManager *Init_SmurfSolver(BDDManager *BM) {
  uint8_t ret = NO_ERROR;

  sbsat_stats_f[STAT_F_SMURF_BUILDING_TIME] = get_runtime();

  //  FS->Backtrack_hook = SmurfBacktrack;
  //  FS->BCP_hook = ApplyNextInferenceToStates;
  //  FS->MakeDecision_hook = SmurfStatePush;

  EnqueueInference_hook = EnqueueInference_smurf_idx;
  
  //Initialize the Smurf Manager
  SmurfManager *SM = Init_SmurfManager(BM);

  //SM->ESSM = (uintptr_t *)FS;
  //FS->conf->user = (void *)SM;

  //Compute Smurfs
  ret = ReadAllSmurfsIntoTable(SM, BM);
  if(ret != NO_ERROR) {
    fprintf(stderr, "Problem reading Smurfs into table (%u)\n", ret);
    return NULL;
  }

  sbsat_stats_f[STAT_F_SMURF_BUILDING_TIME] = get_runtime() - sbsat_stats_f[STAT_F_SMURF_BUILDING_TIME];

  SmurfStatePush((uintptr_t *)SM); //This is to get SBSAT's decision level lined up
                      //with funcsat's 'level'

  return SM;
}
