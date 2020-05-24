#include "sbsat.h"
#include "solver.h"

#define REASON_SMURF_TY 1
int minimize_smurf_clauses = 1;

void makeWatchable(funcsat *f, uintptr_t ix);
void addWatch(funcsat *f, uintptr_t ix);

//Functions that Funcsat needs defined
static clause *get_reason_clause_from_smurf(funcsat *FS, literal l) {
  SmurfManager *SM = (SmurfManager *)FS->conf->user;
  InferenceStruct *pInference = &SM->arrInferenceInfo.pList[abs(l)];
  SmurfInfoStruct *pSmurfInfo = pInference->pSmurfInfo;
  clause *smurf_clause = pSmurfInfo->pLemma;

  //for sanity's sake.
  l = pInference->bPolarity?pSmurfInfo->pIndex2Var[pInference->nVbleIndex]:-pSmurfInfo->pIndex2Var[pInference->nVbleIndex];

  literal temp_lits[pInference->saved_nLemmaLength+1];
  literal *old_lits = smurf_clause->data;
  uintmax_t old_size = smurf_clause->size;
  smurf_clause->data = temp_lits;  
  smurf_clause->size = pInference->saved_nLemmaLength;
  
  for(uintmax_t i = 0; i < smurf_clause->size; i++) {
    smurf_clause->data[i] = old_lits[i];
  }
  minimize_conflict_clause(SM, smurf_clause, pSmurfInfo, l, pInference->nVbleIndex);


  clause *test = clause_head_alloc_from_clause(FS, smurf_clause);
  addWatch(FS, test);

  smurf_clause->size = old_size;
  smurf_clause->data = old_lits;

  return test;
}

uint8_t EnqueueInference_funcsat(SmurfManager *SM, SmurfInfoStruct *pSmurfInfo, uintmax_t nVbleIndex, uint8_t bPolarity) {
  funcsat *FS = (funcsat *)SM->ESSM;
  uintmax_t nVariable = pSmurfInfo->pIndex2Var[nVbleIndex];
  d7_printf4("      Inferring %jd at level %ju to level %u\n",
	     bPolarity?(intmax_t)nVariable:-(intmax_t)nVariable, FS->propq, FS->trail.size);

  mbool previous_lit_value;
  literal lit = bPolarity?(literal)nVariable:-(literal)nVariable;
  d7_printf2("\n%jd\n", lit);
  if (FS->level.data[nVariable] == -1) previous_lit_value = unknown;
  else previous_lit_value = (lit == FS->trail.data[FS->model.data[nVariable]]);

  if(previous_lit_value == true) {
    //Value is already inferred the correct value
    d7_printf2("      Inference %jd already inferred\n", bPolarity?(intmax_t)nVariable:-(intmax_t)nVariable);
    return NO_ERROR;
  } else if(previous_lit_value == false) {
    //Conflict Detected
    d7_printf2("      Conflict when adding %jd to the inference queue\n", bPolarity?(intmax_t)nVariable:-(intmax_t)nVariable);
    //InferenceStruct *pInference = &SM->arrInferenceInfo.pList[nVariable];

    uintmax_t size = pSmurfInfo->pLemma->size;
    clause *reason = SM->conflictClause;
    reason->size = 0;
    clause *smurf_clause = pSmurfInfo->pLemma;

    for(uintmax_t i = 0; i < size; i++) {
      clausePush(reason, smurf_clause->data[i]);
    }
        
    minimize_conflict_clause(SM, reason, pSmurfInfo, lit, nVbleIndex);
      
    FS->conflict_clause = clause_head_alloc_from_clause(FS, reason);
    addWatch(FS, FS->conflict_clause); //Unnecessary?
    
    return UNSAT; //SEAN!!! Make a BACKTRACK return type or something
  } else if(previous_lit_value == unknown) {
    //Inference is not in inference queue, insert it.
    //Also save lemma info in case a lemma needs to be created later
    //InferenceStruct *pInference = &SM->arrInferenceInfo.pList[FS->trail.size];
    
    InferenceStruct *pInference = &SM->arrInferenceInfo.pList[nVariable];
    pInference->nVbleIndex = nVbleIndex;
    pInference->bPolarity = bPolarity;
    pInference->pSmurfInfo = pSmurfInfo;
    if(pSmurfInfo != NULL) { //Not a choicepoint
      pInference->saved_nLemmaLength = pSmurfInfo->pLemma->size;
      assert(pSmurfInfo->pLemma->size < pSmurfInfo->nNumVariables);
    }
    sbsat_stats[STAT_NUM_INFERENCES]++;

    //Insert inference into funcsat's inference queue

    //Funcsat should provide this for me
    assert(FS->reason_infos_freelist < FS->reason_infos->size);
    uintptr_t reason = FS->reason_infos_freelist;
    struct reason_info *r = &FS->reason_infos->data[reason];
    d7_printf3("queueing reason clause at index %ju (next is %ju)\n", reason, r->ty);
    FS->reason_infos_freelist = r->ty;
    r->ty = REASON_SMURF_TY;
    r->cls = 0;

    trailPush(FS, lit, reason);
  }

  return NO_ERROR;
}

//Utils
uint8_t add_ORBDDs_to_funcsat(BDDManager *BM, funcsat *FS) {
  uint8_t ret = NO_ERROR;
  DdManager *dd = BM->dd;

  int old_in_perform_bdd_reductions = BM->in_perform_bdd_reductions;
  BM->in_perform_bdd_reductions = 1;

  for(uintmax_t i = 0; i < BM->nNumBDDs; i++) {
    uintmax_t size;
    if((size = isOR(dd, BM->BDDList[i]))) {
      //Move clause over to funcsat
      d7_printf3("moving OR BDD %ju with %ju literals over to funcsat\n", i, size);
      clause *c = clauseAlloc(size);
      DdNode *bdd = BM->BDDList[i];
      
      uint8_t neg = Cudd_IsComplement(bdd);
      bdd = Cudd_Regular(bdd);
      assert(!cuddIsConstant(bdd));
      while(bdd != Cudd_NotCond(DD_ONE(dd), neg==0)) {
	uintmax_t v = bdd->index;
	if(cuddT(bdd) == Cudd_NotCond(DD_ONE(dd), neg==1)) {
	  clausePush(c, (literal)v);
	  bdd = cuddE(bdd);
	  neg = neg ^ Cudd_IsComplement(bdd);
	  bdd = Cudd_Regular(bdd);
	} else if(Cudd_Regular(cuddE(bdd)) == Cudd_NotCond(DD_ONE(dd), neg ^ Cudd_IsComplement(cuddE(bdd)))) {
	  clausePush(c, -(literal)v);
	  bdd = cuddT(bdd);
	} else assert(0);
      }
      remove_BDD_from_manager(BM, i);

      if (FS_UNSAT == funcsatAddClause(FS, c)) {
	BM->in_perform_bdd_reductions = old_in_perform_bdd_reductions;
	return UNSAT;
      }
    }
  }

  ret = compress_BDD_manager(BM);

  BM->in_perform_bdd_reductions = old_in_perform_bdd_reductions;

  return ret;
}

void init_funcsat_with_Smurf_heuristic(SmurfManager *SM, funcsat *FS) {
  uintmax_t orig_varInc = FS->varInc;
  for(uintmax_t i = 1; i < SM->nNumVariables; i++) {
    double hscore = 0.0;
    hscore += get_heuristic_score_for_var(SM, i, 0);
    hscore += get_heuristic_score_for_var(SM, i, 1); 

    d9_printf3("Variable %ju has Johnson score %lf\n", i, hscore);
    
    FS->varInc = hscore;
    varBumpScore(FS, i);
  }

  FS->varInc = orig_varInc;
}

void init_funcsat_with_Lemma_heuristic(SmurfManager *SM, funcsat *FS) {
  uintmax_t orig_varInc = FS->varInc;
  for(uintmax_t i = 1; i < SM->nNumSmurfs; i++) {
    SmurfInfoStruct *pSmurfInfo = &SM->arrSmurfStates.pList[i];
    FS->varInc += 2.*(1./(double)pSmurfInfo->nNumVariables);

    for(uintmax_t j = 0; j < pSmurfInfo->nNumVariables; j++) {
      varBumpScore(FS, pSmurfInfo->pIndex2Var[j]);
    }
  }

  FS->varInc = orig_varInc;
}

//This initializes a few things then calls the main initialization function - ReadAllSmurfsIntoTable();
SmurfManager *Init_SmurfSolver_funcsat(BDDManager *BM, funcsat *FS) {
  uint8_t ret = NO_ERROR;

  sbsat_stats_f[STAT_F_SMURF_BUILDING_TIME] = get_runtime();

  //Tie funcsat to the Smurfs
  funcsatResize(FS, BM->nNumVariables-1); //SEAN!!! eh?
  funcsatAddReasonHook(FS, REASON_SMURF_TY, get_reason_clause_from_smurf);
  funcsatAddReasonHook(FS, REASON_SMURF_TY, get_reason_clause_from_smurf); //SEAN!!! only because it's broke currently
    
  ret = add_ORBDDs_to_funcsat(BM, FS);
  if(ret != NO_ERROR) {
    //fprintf(stderr, "Problem adding clauses to funcsat (%u)\n", ret);
    return NULL;
  }
  
  FS->Backtrack_hook = SmurfBacktrack;
  FS->BCP_hook = ApplyNextInferenceToStates;
  FS->MakeDecision_hook = SmurfStatePush;
  if(sHeuristic == 'j') FS->ExternalHeuristic_hook = LSGBHeuristic;
  else if(sHeuristic == 's') FS->ExternalHeuristic_hook = StaticHeuristic;
  else if(sHeuristic == 'r') FS->ExternalHeuristic_hook = RandomHeuristic;
  else if(sHeuristic == 'n') FS->ExternalHeuristic_hook = NULLHeuristic;
  else FS->ExternalHeuristic_hook = NULL;

  EnqueueInference_hook = EnqueueInference_funcsat;
  
  //Initialize the Smurf Manager
  SmurfManager *SM = Init_SmurfManager(BM);

  SM->ESSM = (uintptr_t *)FS;
  SM->ESSM_type = 'f';
  FS->conf->user = (void *)SM;

  //Compute Smurfs
  ret = ReadAllSmurfsIntoTable(SM, BM);
  if(ret != NO_ERROR) {
    fprintf(stderr, "Problem reading Smurfs into table (%u)\n", ret);
    return NULL;
  }

  SmurfStatePush(SM);

  //Push inital inferences
  for(uintmax_t i = 0; i < SM->nNumSmurfs; i++) {
    TypeStateEntry *pTypeState = SM->arrSmurfStates.pList[i].pCurrentState;
    if(arrGetStateType[pTypeState->type] == SM_TRANSITION_TYPE) {
      d9_printf2("Following initial transition of Smurf %ju\n", i);
      if(arrApplyInferenceToState[pTypeState->type](SM, 0, 0, &SM->arrSmurfStates.pList[i]) == UNSAT) {
	d9_printf1("Formula found unsat via initial transitions\n");
	FS->lastResult = FS_UNSAT;
	return SM;
      }
    }
  }

  //init_funcsat_with_Smurf_heuristic(SM, FS);
  init_funcsat_with_Lemma_heuristic(SM, FS);

  sbsat_stats_f[STAT_F_SMURF_BUILDING_TIME] = get_runtime() - sbsat_stats_f[STAT_F_SMURF_BUILDING_TIME];

  return SM;
}
