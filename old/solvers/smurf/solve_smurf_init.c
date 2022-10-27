#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

int turn_off_smurf_normalization = 0;
int use_dropped_var_smurfs = 0;
int gelim_smurfs = 0;
int precompute_smurfs = 12;
char sHeuristic = 'j';
float JHEURISTIC_K;
float JHEURISTIC_K_TRUE;
float JHEURISTIC_K_INF;
float JHEURISTIC_K_UNKNOWN;

InitStateType arrInitStateType[] = STATE_INIT_LIST;
FreeStateType arrFreeStateType[] = STATE_FREE_LIST;

uint8_t arrGetStateType[NUM_SMURF_TYPES]; //Node, Transition, or Leaf

ApplyInferenceToState arrApplyInferenceToState[NUM_SMURF_TYPES];
CreateStateEntry arrCreateStateEntry[NUM_SMURF_TYPES];
SetStateVisitedFlag arrSetStateVisitedFlag[NUM_SMURF_TYPES];
UnsetStateVisitedFlag arrUnsetStateVisitedFlag[NUM_SMURF_TYPES];
CleanUpStateEntry arrCleanUpStateEntry[NUM_SMURF_TYPES];
FreeStateEntry arrFreeStateEntry[NUM_SMURF_TYPES];
CalculateStateSize arrStatesTypeSize[NUM_SMURF_TYPES];
PrintStateEntry arrPrintStateEntry[NUM_SMURF_TYPES];
PrintStateEntry_dot arrPrintStateEntry_dot[NUM_SMURF_TYPES];
CalculateStateHeuristic arrCalculateStateHeuristic[NUM_SMURF_TYPES];
SetStateHeuristicScore arrSetStateHeuristicScore[NUM_SMURF_TYPES];
GetStateHeuristicScore arrGetStateHeuristicScore[NUM_SMURF_TYPES];
GetVarHeuristicScore arrGetVarHeuristicScore[NUM_SMURF_TYPES];
FreeHeuristicScore arrFreeHeuristicScore[NUM_SMURF_TYPES];

//This function needs to be defined for use w/ an external solver
uint8_t (*EnqueueInference_hook)(SmurfManager *SM, SmurfInfoStruct *pSmurfInfo, uintmax_t nVariable, uint8_t bPolarity);

uintmax_t ComputeFreeStateSize(TypeStateEntry *pTypeState) {
  return sizeof(uint8_t);
}

uintmax_t ComputeTypeStateSize(TypeStateEntry *pTypeState) {
  return sizeof(TypeStateEntry);
}

void FreeFreeStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState) {
  //Do nothing
}

SmurfManager *Init_SmurfManager(BDDManager *BM) {
  Cudd_AutodynDisable(BM->dd);
  Cudd_AutodynEnable(BM->dd, CUDD_REORDER_NONE); //This isn't really
						 //the right way to do
						 //this.

  //Initialize the various function types
  arrStatesTypeSize[FN_FREE_STATE] = ComputeFreeStateSize;
  arrStatesTypeSize[FN_TYPE_STATE] = ComputeTypeStateSize;
  arrFreeStateEntry[FN_FREE_STATE] = FreeFreeStateEntry;
  for(uintmax_t i = 0; arrInitStateType[i]!=NULL; i++)
    arrInitStateType[i]();

  //Allocate a new SmurfManager
  SmurfManager *SM = (SmurfManager *)sbsat_calloc(1, sizeof(SmurfManager), 9, "SmurfManager");

  SM->nNumSmurfs = 0;
  SM->nNumVariables = BM->nNumVariables;
  
  //Allocate the SmurfStatesTable
  SM->arrSmurfStatesTableHead = (SmurfStatesTableStruct *)sbsat_calloc(1, sizeof(SmurfStatesTableStruct), 9, "SM->arrSmurfStatesTableHead");
  SM->arrSmurfStatesTableHead->arrStatesTable = (uint8_t *)sbsat_calloc(SMURF_TABLE_SIZE, 1, 9, "SM->arrSmurfStatesTableHead->arrStatesTable");
  SM->arrSmurfStatesTableHead->curr_size = sizeof(SmurfStateEntry); //For pTrueSmurfState
  SM->arrSmurfStatesTableHead->max_size = SMURF_TABLE_SIZE;
  SM->arrSmurfStatesTableHead->pNext = NULL;
  
  SM->arrCurrSmurfStatesTable = SM->arrSmurfStatesTableHead;

  //arrSmurfStatesTable[0] is reserved for the pTrueSmurfState
  SM->pTrueSmurfState = (SmurfStateEntry *)SM->arrCurrSmurfStatesTable->arrStatesTable;
  SM->pTrueSmurfState->type = FN_SMURF;
  SM->pTrueSmurfState->visited = 0;
  SM->pTrueSmurfState->nNumVariables = 0;
  SM->pTrueSmurfState->pSmurfBDD = DD_ONE(BM->dd); Cudd_Ref(DD_ONE(BM->dd));
  DD_ONE(BM->dd)->pStatePos = SM->pTrueSmurfState;
  sbsat_stats[STAT_SMURF_STATES_BUILT]+=1;
  sbsat_stats[STAT_STATES_BUILT]+=1;
  
  SM->pSmurfStatesTableTail = (uint8_t *)(SM->pTrueSmurfState + 1);

  //Initialize the occurrence lists.
  SmurfOccurrenceStruct_list_list_alloc(&SM->arrVariableOccurrences, SM->nNumVariables+1, 10);
  for(uintmax_t i = 0; i <= SM->nNumVariables; i++) {
    SmurfOccurrenceStruct_list_alloc(&SM->arrVariableOccurrences.pList[i], BM->VarBDDMap[i].nLength, 1);
  }
  SmurfInfoStruct_list_alloc(&SM->arrSmurfStates, BM->nNumBDDs, 10);

  SM->arrInitialSmurfStates = (TypeStateEntry **)sbsat_calloc(BM->nNumBDDs, sizeof(TypeStateEntry *), 9, "arrInitialSmurfStates");

  //Initialize list for inference information
  InferenceStruct_list_alloc(&SM->arrInferenceInfo, SM->nNumVariables+1, 10);

  SM->pTrail = sbsat_calloc(SM->nNumVariables+1, sizeof(int8_t), 9, "SM->pTrail");

  SM->BM = BM;
  SM->dd = BM->dd;
  SM->ESSM = NULL;

  if(gelim_smurfs) { //initialize the Gaussian Elimination manager
    initXORGElimTable(SM->nNumVariables, (EnqueueInference_hook_gelim) EnqueueInference_smurf, (uintptr_t *)SM);
    SM->XM = (XORGElimTableStruct **)sbsat_calloc(SM->nNumVariables+1, sizeof(XORGElimTableStruct*), 9, "Stack of Gaussian Elimination managers");
    for(uintmax_t i = 0; i <= SM->nNumVariables; i++) {
      SM->XM[i] = (XORGElimTableStruct *)sbsat_calloc(1, sizeof(XORGElimTableStruct), 9, "Gaussian Elimination manager");
    }
    allocXORGElimTable(SM->XM[0], BM->nNumBDDs, SM->nNumVariables);
    preloadXORGelimTable(SM->XM[0]);
  }

  SM->JHEURISTIC_K = JHEURISTIC_K;
  if(SM->JHEURISTIC_K == 0.0)
    SM->JHEURISTIC_K = 0.001;
  SM->JHEURISTIC_K_TRUE = JHEURISTIC_K_TRUE;
  SM->JHEURISTIC_K_INF = JHEURISTIC_K_INF;
  SM->JHEURISTIC_K_UNKNOWN = JHEURISTIC_K_UNKNOWN;
  
  SM->arrPosVarHeurWghts = (double *)sbsat_malloc(SM->nNumVariables+1, sizeof(double), 9, "SM->arrPosVarHeurWghts");
  SM->arrNegVarHeurWghts = (double *)sbsat_malloc(SM->nNumVariables+1, sizeof(double), 9, "SM->arrNegVarHeurWghts");
  
  SM->nHeuristicPlaceholder = 1; //Variable 0 doesn't exist

  SM->pStaticHeurOrder = (uintmax_t *)sbsat_calloc(SM->nNumVariables+1, sizeof(uintmax_t), 9, "SM->pStaticHeurOrder");

  CHeuristics[ve_heuristic](BM, SM->pStaticHeurOrder);

  //SEAN!!! A funcsat specific thing. Will have to change w/ picosat integration
  SM->conflictClause = clauseAlloc(SM->nNumVariables+1);

  uintmax_t_list_alloc(&SM->uTempList1, 10, 10);
  uintmax_t_list_alloc(&SM->uTempList2, 10, 10);
  uint32_t_list_alloc(&SM->uTempList32, 10, 10);

  TypeStateEntry_pundoer_alloc(&SM->pCurrentState_pundoer, 1000, 1000);
  uint32_t_undoer_alloc(&SM->nLemmaLength_undoer, 1000, 1000);
  literal_undoer_alloc(&SM->pLemmaLiteral_undoer, 1000, 1000);
  double_undoer_alloc(&SM->VarHeurWghts_undoer, 1000, 1000);
  int8_t_undoer_alloc(&SM->pTrail_undoer, 1000, 1000);
  uintmax_t_undoer_alloc(&SM->generic_uintmax_t_undoer, 1000, 1000);

  SM->nCurrentDecisionLevel = 0;

  return SM;
}

TypeStateEntry *ReadSmurfStateIntoTable(SmurfManager *SM, DdNode *pCurrentBDD) {
  sbsat_stats[STAT_SMURF_STATES_EXAMINED]++;

  uint8_t BDD_is_negated = Cudd_IsComplement(pCurrentBDD);
  TypeStateEntry *pNewState = (TypeStateEntry *)(BDD_is_negated?Cudd_Not(pCurrentBDD)->pStateNeg:pCurrentBDD->pStatePos);
  if(pNewState==NULL)  { //Smurf not already in the table
    BDD_Support(&SM->uTempList1.nLength, &(SM->uTempList1.nLength_max), &(SM->uTempList1.pList), pCurrentBDD);
    qsort(SM->uTempList1.pList, SM->uTempList1.nLength, sizeof(uintmax_t), compfunc);
    
    for(uintmax_t i = BEGIN_FUNCTIONAL_SMURFS+1; i!=END_FUNCTIONAL_SMURFS; i++)
      if((pNewState = arrCreateStateEntry[i](SM, pCurrentBDD))) break;
    assert(pNewState != NULL);
    
    if(BDD_is_negated) Cudd_Not(pCurrentBDD)->pStateNeg = pNewState;
    else pCurrentBDD->pStatePos = pNewState;

  }
  
  assert(((TypeStateEntry *)pNewState)->type!=FN_FREE_STATE);

  return pNewState;
}

uint8_t AddBDDToSmurfManager(SmurfManager *SM, DdNode *bdd) {
  uint8_t ret = NO_ERROR;

  if(bdd == DD_ONE(SM->dd)) return ret;
  if(bdd == Cudd_Not(SM->dd)) return UNSAT;

  uintmax_t nSmurfIndex = SM->nNumSmurfs;
  SM->nNumSmurfs++;

  BDD_Support(&SM->uTempList1.nLength, &(SM->uTempList1.nLength_max), &(SM->uTempList1.pList), bdd);
  //qsort(SM->uTempList1.pList, SM->uTempList1.nLength, sizeof(uintmax_t), compfunc);
  qsort(SM->uTempList1.pList, SM->uTempList1.nLength, sizeof(uintmax_t), bddperm_compfunc);

  for(uintmax_t i = 0; i < SM->uTempList1.nLength; i++)
    uintmax_t_list_add(&SM->uTempList2, i, i);
  SM->uTempList2.nLength = SM->uTempList1.nLength;
  qsort(SM->uTempList2.pList, SM->uTempList2.nLength, sizeof(uintmax_t), bddperm_compfunc);

  SmurfInfoStruct smurf_info;
  //Set up smurf info for the new state machine
  smurf_info.nNumVariables = SM->uTempList1.nLength;
  smurf_info.pIndex2Var = (uintmax_t *)sbsat_malloc(smurf_info.nNumVariables, sizeof(uintmax_t), 9, "smurf_info.pIndex2Var");

  //Create the generic funcsat lemma for this smurf
  smurf_info.pLemma = clauseAlloc(smurf_info.nNumVariables);
  
  smurf_info.nSmurfIndex = nSmurfIndex;
  for(uintmax_t i = 0; i < SM->uTempList1.nLength; i++) {
    d9_printf3("mapping %ju to %ju\n", SM->uTempList1.pList[i], SM->uTempList2.pList[i]);
    assert(SM->uTempList1.pList[i] <= SM->nNumVariables);
    //smurf_info.pIndex2Var[i] = SM->uTempList1.pList[i];
    smurf_info.pIndex2Var[SM->uTempList2.pList[i]] = SM->uTempList1.pList[i]; //map down here
  }
  SmurfInfoStruct_list_push(&SM->arrSmurfStates, smurf_info);
  assert(SM->nNumSmurfs == SM->arrSmurfStates.nLength);

  //Add new state machine to the occurrence lists
  SmurfOccurrenceStruct smurf_occurrence;
  for(uintmax_t i = 0; i < SM->uTempList1.nLength; i++) {
    uintmax_t loc = SM->uTempList2.pList[i];
    uintmax_t v = smurf_info.pIndex2Var[SM->uTempList2.pList[i]];
    assert(v <= SM->nNumVariables && v <= SM->arrVariableOccurrences.nLength_max);
    smurf_occurrence.pSmurfInfo = &SM->arrSmurfStates.pList[nSmurfIndex];
    smurf_occurrence.nVbleIndex = loc;
    SmurfOccurrenceStruct_list_push(&SM->arrVariableOccurrences.pList[v], smurf_occurrence);
  }

  //Normalize the BDD to get more sharing among Smurfs - this is no longer optional
  if(!turn_off_smurf_normalization) {
    for(uintmax_t i = 0; i < SM->uTempList1.nLength; i++) {
      //uint32_t_list_add(&SM->uTempList32, SM->uTempList1.pList[i], i);
      uint32_t_list_add(&SM->uTempList32, SM->uTempList1.pList[i], SM->uTempList2.pList[i]);
    }
    bdd = Cudd_bddPermute(SM->dd, bdd, (int *)SM->uTempList32.pList);
    Cudd_Ref(bdd);
  }

  //Generate state machine for the bdd
  SM->arrInitialSmurfStates[nSmurfIndex] = SM->arrSmurfStates.pList[nSmurfIndex].pCurrentState = ReadSmurfStateIntoTable(SM, bdd);

  if(!turn_off_smurf_normalization)
    Cudd_IterDerefBdd(SM->dd, bdd);
  
  return ret;
}

uint8_t ReadAllSmurfsIntoTable(SmurfManager *SM, BDDManager *BM) {
  uint8_t ret = NO_ERROR;

  //Turn BDD reductions off
  uint8_t old_in_perform_bdd_reductions = BM->in_perform_bdd_reductions;
  BM->in_perform_bdd_reductions = 1;

  //Create the rest of the SmurfState entries
  d2_printf1("\n");
  for(uintmax_t nSmurfIndex = 0; nSmurfIndex < BM->nNumBDDs; nSmurfIndex++) {
    d2_printf4("\rBuilding Smurfs %ju/%ju (%ju)... ", nSmurfIndex+1, BM->nNumBDDs, BM->Support[nSmurfIndex].nLength);
    
    ret = AddBDDToSmurfManager(SM, BM->BDDList[nSmurfIndex]);
    if(ret != NO_ERROR) return ret;

    //Remove BDD from the BDD Manager...maybe this isn't the right thing to do?

    remove_BDD_from_manager(BM, nSmurfIndex);
  }

  BM->in_perform_bdd_reductions = old_in_perform_bdd_reductions;

  ret = compress_BDD_manager(BM);
  if(ret != NO_ERROR) return ret;  

  BDDManager_GC(BM);

  d2_printf2("%ju SmurfStates Built\n", sbsat_stats[STAT_STATES_BUILT]);

  return ret;
}

//Here we do the clean up after the brancher is done - freeing memory etc.
uint8_t Final_SimpleSmurfSolver(SmurfManager *SM) {
  uint8_t ret = NO_ERROR;

  FreeSmurfStateEntries(SM);
  FreeSmurfStatesTable(SM);

  for(uintmax_t i = 0; i < SM->nNumSmurfs; i++) {
    sbsat_free((void **)&SM->arrSmurfStates.pList[i].pIndex2Var);
    clauseFree(SM->arrSmurfStates.pList[i].pLemma);
  }

  sbsat_free((void **)&SM->arrInitialSmurfStates);

  for(uintmax_t i = 0; i < SM->nNumVariables; i++)
    SmurfOccurrenceStruct_list_free(&SM->arrVariableOccurrences.pList[i]);
  SmurfOccurrenceStruct_list_list_free(&SM->arrVariableOccurrences);

  SmurfInfoStruct_list_free(&SM->arrSmurfStates);
  InferenceStruct_list_free(&SM->arrInferenceInfo);

  sbsat_free((void **)&SM->pTrail);

  if(gelim_smurfs) { //free the Gaussian Elimination manager
    for(uintmax_t i = 0; i <= SM->nNumVariables; i++) {
      deleteXORGElimTable(SM->XM[i]);
    }
    sbsat_free((void **)&SM->XM);
  }
  
  //Free memory associated with initalizing the various function types
  for(uintmax_t i = 0; arrFreeStateType[i]!=NULL; i++)
    arrFreeStateType[i]();

  //Free memory associated with initalizing the heuristic scores for various function types
  for(uintmax_t i = 0; arrFreeStateType[i]!=NULL; i++)
    arrFreeHeuristicScore[i](SM);

  sbsat_free((void **)&SM->arrPosVarHeurWghts);
  sbsat_free((void **)&SM->arrNegVarHeurWghts);

  sbsat_free((void **)&SM->pStaticHeurOrder);

  clauseFree(SM->conflictClause);

  uintmax_t_list_free(&SM->uTempList1);
  uintmax_t_list_free(&SM->uTempList2);
  uint32_t_list_free(&SM->uTempList32);

  TypeStateEntry_pundoer_free(&SM->pCurrentState_pundoer);
  uint32_t_undoer_free(&SM->nLemmaLength_undoer);
  literal_undoer_free(&SM->pLemmaLiteral_undoer);
  double_undoer_free(&SM->VarHeurWghts_undoer);
  int8_t_undoer_free(&SM->pTrail_undoer);
  uintmax_t_undoer_free(&SM->generic_uintmax_t_undoer);

  //Cudd_AutodynEnable(SM->dd, CUDD_REORDER_SAME);
  
  sbsat_free((void **)&SM);

  return ret;
}

create_list_type(SmurfInfoStruct);
create_list_type(SmurfOccurrenceStruct);
create_list_type(SmurfOccurrenceStruct_list);
create_list_type(InferenceStruct);

create_pundoer_type(TypeStateEntry);
create_undoer_type(literal);
