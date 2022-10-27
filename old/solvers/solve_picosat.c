#include "sbsat.h"
#include "solver.h"

extern void picosat_enter (void);
extern void picosat_leave (void);

//Utils

uint8_t add_ORBDDs_to_picosat(BDDManager *BM) {
  uint8_t ret = NO_ERROR;
  DdManager *dd = BM->dd;

  int old_in_perform_bdd_reductions = BM->in_perform_bdd_reductions;
  BM->in_perform_bdd_reductions = 1;

  for(uintmax_t i = 0; i < BM->nNumBDDs; i++) {
    uintmax_t size;
    if((size = isOR(dd, BM->BDDList[i]))) {
      //Move clause over to picosat
      d7_printf3("moving OR BDD %ju with %ju literals over to picosat\n", i, size);
      DdNode *bdd = BM->BDDList[i];
      
      uint8_t neg = Cudd_IsComplement(bdd);
      bdd = Cudd_Regular(bdd);
      assert(!cuddIsConstant(bdd));
      while(bdd != Cudd_NotCond(DD_ONE(dd), neg==0)) {
	uintmax_t v = bdd->index;
	if(cuddT(bdd) == Cudd_NotCond(DD_ONE(dd), neg==1)) {
	  picosat_add((int)v);
	  bdd = cuddE(bdd);
	  neg = neg ^ Cudd_IsComplement(bdd);
	  bdd = Cudd_Regular(bdd);
	} else if(Cudd_Regular(cuddE(bdd)) == Cudd_NotCond(DD_ONE(dd), neg ^ Cudd_IsComplement(cuddE(bdd)))) {
	  picosat_add(-(int)v);
	  bdd = cuddT(bdd);
	} else assert(0);
      }
      picosat_add((int)0); //end of clause
      remove_BDD_from_manager(BM, i);
    }
  }

  ret = compress_BDD_manager(BM);

  BM->in_perform_bdd_reductions = old_in_perform_bdd_reductions;

  return ret;
}

void create_picosat_clause_from_TypeState(SmurfManager *SM, intmax_t nInfVar, uintmax_t nVbleIndex, SmurfInfoStruct *pSmurfInfo, ClsStruct *picosat_clause, uintmax_t nLemmaLength) {
  PicosatManager *PM = (PicosatManager *)SM->ESSM;

  clause *smurf_clause = pSmurfInfo->pLemma; //funcsat clause

  literal temp_lits[nLemmaLength+1];
  literal *old_lits = smurf_clause->data;
  uintmax_t old_size = smurf_clause->size;
  smurf_clause->data = temp_lits;  
  smurf_clause->size = nLemmaLength;
  
  for(uintmax_t i = 0; i < smurf_clause->size; i++) {
    smurf_clause->data[i] = old_lits[i];
  }
  minimize_conflict_clause(SM, smurf_clause, pSmurfInfo, nInfVar, nVbleIndex);

  if(picosat_clause->clause == NULL || smurf_clause->size > picosat_clause->max_size) {
    picosat_clause->clause = (Cls *)sbsat_recalloc(picosat_clause->clause, picosat_clause->max_size, bytes_clause(smurf_clause->size, 0), 1, 9, "picosat_clause->clause");
    picosat_clause->max_size = smurf_clause->size;
  }

  assert(picosat_clause->clause->used == 0); //The literal is already inferred
  Lit **p = picosat_clause->clause->lits;

  for(uintmax_t i = 0; i < smurf_clause->size; i++) {
    d7_printf2("%jd ", smurf_clause->data[i]);
    (*p++) = int2lit(smurf_clause->data[i]);
  }
  d7_printf1("\n");

  picosat_clause->clause->size = smurf_clause->size;

  smurf_clause->size = old_size;
  smurf_clause->data = old_lits;

  //  dumpclsnl(picosat_clause->clause);

}

uint8_t EnqueueInference_picosat(SmurfManager *SM, SmurfInfoStruct *pSmurfInfo, uintmax_t nVbleIndex, uint8_t bPolarity) {
  PicosatManager *PM = (PicosatManager *)SM->ESSM;
  uintmax_t nVariable = pSmurfInfo->pIndex2Var[nVbleIndex];  
  assert(nVariable != 0);

  d7_printf3("      Inferring %jd at level %ju\n",
	     bPolarity?(intmax_t)nVariable:-(intmax_t)nVariable, SM->nCurrentDecisionLevel);

  uint8_t value_correct;
  Lit *literal = int2lit(bPolarity?nVariable:-nVariable);
  if(literal->val == 0) {
    value_correct = -1;
  } else if(literal->val == 1) {
    value_correct = 1; //Value currently unassigned
  } else {
    value_correct = 0;
  }

  if(value_correct==1) {
    //Value is already inferred the correct value
    d7_printf2("      Inference %jd already inferred\n", bPolarity?(intmax_t)nVariable:-(intmax_t)nVariable);
    return NO_ERROR;
  } else if(value_correct==0) {
    //Conflict Detected
    d7_printf2("      Conflict when adding %jd to the inference queue\n", bPolarity?(intmax_t)nVariable:-(intmax_t)nVariable);

    create_picosat_clause_from_TypeState(SM, bPolarity?nVariable:-nVariable, nVbleIndex, pSmurfInfo, &PM->memoized_conflict_clause, pSmurfInfo->pLemma->size);
    *PM->conflict = PM->memoized_conflict_clause.clause;

    return UNSAT; //SEAN!!! Make a BACKTRACK return type or something
  } else {
    //Inference is not in inference queue, insert it and create a conflict clause
    InferenceStruct *pInference = &SM->arrInferenceInfo.pList[nVariable];
    pInference->nVbleIndex = nVbleIndex;
    pInference->bPolarity = bPolarity;
    pInference->pSmurfInfo = pSmurfInfo;
    if(pSmurfInfo != NULL) { //Not a choicepoint
      pInference->saved_nLemmaLength = pSmurfInfo->pLemma->size;
      assert(pSmurfInfo->pLemma->size < pSmurfInfo->nNumVariables);
    }
    sbsat_stats[STAT_NUM_INFERENCES]++;

    create_picosat_clause_from_TypeState(SM, bPolarity?nVariable:-nVariable, nVbleIndex, pSmurfInfo, &PM->memoized_clauses[nVariable], pInference->saved_nLemmaLength);
    assign_forced(literal, PM->memoized_clauses[nVariable].clause);
  }

  return NO_ERROR;
}

SmurfManager *Init_SmurfSolver_picosat(BDDManager *BM, PicosatManager *PM) {
  uint8_t ret = NO_ERROR;

  sbsat_stats_f[STAT_F_SMURF_BUILDING_TIME] = get_runtime();

  picosat_init ();
  picosat_enter ();

  if((ret = add_ORBDDs_to_picosat(BM)) != NO_ERROR)
    return NULL;

  EnqueueInference_hook = EnqueueInference_picosat;

  //Initialize the Smurf Manager
  SmurfManager *SM = Init_SmurfManager(BM);

  SM->ESSM_type = 'p';
  SM->ESSM = (void *)PM;

  //Compute Smurfs
  ret = ReadAllSmurfsIntoTable(SM, BM);
  if(ret != NO_ERROR) {
    fprintf(stderr, "Problem reading Smurfs into table (%u)\n", ret);
    return NULL;
  }

  sbsat_stats_f[STAT_F_SMURF_BUILDING_TIME] = get_runtime() - sbsat_stats_f[STAT_F_SMURF_BUILDING_TIME];

  SmurfStatePush((uintptr_t *)SM); //This is to get SBSAT's decision level lined up
                      //with picosat's 'level'

  PM->propagations = &propagations;
  PM->conflicts = &conflicts;
  PM->conflict = &conflict;
  PM->vars = vars;
  PM->lits = lits;
  PM->level = &level;
  PM->memoized_clauses = (ClsStruct *)sbsat_calloc(SM->nNumVariables+1, sizeof(ClsStruct), 9, "PM->memoized_clauses");

  PM->user = (uintptr_t *)SM;

  PM->ApplyInference_hook = ApplyNextInferenceToStates;
  PM->PopState_hook = SmurfBacktrack;
  PM->PushState_hook = SmurfStatePush;
  if(sHeuristic == 'j') PM->Decide_hook = LSGBHeuristic;
  else if(sHeuristic == 's') PM->Decide_hook = StaticHeuristic;
  else if(sHeuristic == 'r') PM->Decide_hook = RandomHeuristic;
  else if(sHeuristic == 'n') PM->Decide_hook = NULLHeuristic;
  else PM->Decide_hook = NULL;

  picosat_adjust(SM->nNumVariables); //Set the highest numbered variable in Picosat
  picosat_set_seed(random_seed); //Set the random seed

  for(uintmax_t i = 0; i < SM->nNumSmurfs; i++) {
    SmurfInfoStruct *pSmurfInfo = &SM->arrSmurfStates.pList[i];
    if(pSmurfInfo->nNumVariables > 2) 
      loadded++; //picosat heuristic needs this variable initialized
  }

  for(uintmax_t i = 0; i < SM->nNumVariables; i++) {
    SmurfOccurrenceStruct_list pSmurfOccurrences = SM->arrVariableOccurrences.pList[i];
    for(uintmax_t j = 0; j < pSmurfOccurrences.nLength; j++) {
      int old_level = vars[i].level;
      vars[i].level = 1;
      inc_score(&vars[i]);
      vars[i].level = old_level;
    }
  }
  
  return SM;
}

int picosatSolve(PicosatManager *PM) {
  uint8_t ret = NO_ERROR;

  SmurfManager *SM = (SmurfManager *)PM->user;

  //Push inital inferences from Smurfs
  for(uintmax_t i = 0; i < SM->nNumSmurfs; i++) {
    TypeStateEntry *pTypeState = SM->arrSmurfStates.pList[i].pCurrentState;
    if(arrGetStateType[pTypeState->type] == SM_TRANSITION_TYPE) {
      d9_printf2("Following initial transition of Smurf %ju\n", i);
      if(arrApplyInferenceToState[pTypeState->type](SM, 0, 0, &SM->arrSmurfStates.pList[i]) == UNSAT) {
	d9_printf1("Formula found unsat via initial transitions\n");
	SmurfBacktrack((uintptr_t *)SM, -1);
	return UNSAT;
      }
    }
  }

  int decision_limit = -1;
  ret = picosat_sat(decision_limit, PM);

  if(ret == PICOSAT_UNSATISFIABLE) {
    ret = UNSAT;
  } else if(ret == PICOSAT_SATISFIABLE) {
    ret = SAT;
  } else {
    ret = SAT_UNKNOWN;
  }

  picosat_leave ();
  picosat_reset ();

  SmurfBacktrack((uintptr_t *)SM, -1);

  return ret;
}
