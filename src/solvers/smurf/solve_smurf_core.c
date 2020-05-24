#include "sbsat.h"
#include "solver.h"

//Assume that M is initially empty
//Assume P is ordered according to dd->perm
//depth is initially 0
//G is not DD_ONE(dd)
void BDDmininf(DdManager *dd, DdNode *G, DdNode *P, uintmax_t_list *M) {
  if(G == Cudd_Not(DD_ONE(dd))) return;

  assert(G != DD_ONE(dd));
  assert(!cuddIsConstant(P));

  DdNode *zero = Cudd_Not(DD_ONE(dd));

  DdNode *g = Cudd_Regular(G);
  DdNode *Gv, *Gnv;
  DdNode *p = Cudd_Regular(P);
  DdNode *Pv;

  assert(Cudd_bddIteConstant(dd, G, P, Cudd_Not(DD_ONE(dd))) == zero);

  if(g->index == p->index) {
    if(cuddT(p) == Cudd_NotCond(DD_ONE(dd), !Cudd_IsComplement(P))) {
      Pv = Cudd_NotCond(cuddE(p), Cudd_IsComplement(P));
      Gv = Cudd_NotCond(cuddE(g), Cudd_IsComplement(G));
      Gnv = Cudd_NotCond(cuddT(g), Cudd_IsComplement(G));
    } else {
      assert(cuddE(p) == Cudd_NotCond(DD_ONE(dd), !Cudd_IsComplement(P)));
      Pv = Cudd_NotCond(cuddT(p), Cudd_IsComplement(P));
      Gv = Cudd_NotCond(cuddT(g), Cudd_IsComplement(G));
      Gnv = Cudd_NotCond(cuddE(g), Cudd_IsComplement(G));
    }
    
    DdNode *is_false = Cudd_bddIteConstant(dd, Pv, Gnv, Cudd_Not(DD_ONE(dd)));
    if(is_false != Cudd_Not(DD_ONE(dd))) {
      //p->index is necessary
      uintmax_t_list_push(M, p->index);
      BDDmininf(dd, Gv, Pv, M);
    } else {
      //g->index is not necessary, quantify it away from G.
      DdNode *G_exist = Cudd_bddOr(dd, Gv, Gnv); Cudd_Ref(G_exist);
      BDDmininf(dd, G_exist, Pv, M);
      Cudd_IterDerefBdd(dd, G_exist);
    }
  } else if(dd->perm[g->index] < dd->perm[p->index]) {
    //g->index is not necessary, quantify it away from G.
    DdNode *G_exist = Cudd_bddExistAbstract(dd, G, Cudd_bddIthVar(dd, g->index));
    Cudd_Ref(G_exist);
    BDDmininf(dd, G_exist, P, M);
    Cudd_IterDerefBdd(dd, G_exist);
  } else {
    //p->index is not in G. Pass it over.
    if(cuddT(p) == Cudd_NotCond(DD_ONE(dd), !Cudd_IsComplement(P))) {
      Pv = Cudd_NotCond(cuddE(p), Cudd_IsComplement(P));
    } else {
      assert(cuddE(p) == Cudd_NotCond(DD_ONE(dd), !Cudd_IsComplement(P)));
      Pv = Cudd_NotCond(cuddT(p), Cudd_IsComplement(P));
    }
    BDDmininf(dd, G, Pv, M);
  }
}

void _minimize_conflict_clause(SmurfManager *SM, SmurfInfoStruct *pSmurfInfo, clause *c, DdNode *bdd, uint8_t bInfPolarity) {
  Cudd_AutodynDisable(SM->dd);

  SM->uTempList1.nLength = 0;

  intmax_t nInfVar = c->data[--c->size];
  bdd = Cudd_Cofactor(SM->dd, bdd, Cudd_NotCond(Cudd_bddIthVar(SM->dd, nInfVar), bInfPolarity));
  Cudd_Ref(bdd);

  DdNode *tmp_lit;
  DdNode *bTemp;
  DdNode *P = DD_ONE(SM->dd);
  cuddRef(P);
  for(uintmax_t i = 0; i < c->size; i++) {
    uint8_t pos = SM->pTrail[pSmurfInfo->pIndex2Var[c->data[i]]]==1;
    //tmp_lit = Cudd_NotCond(Cudd_bddIthVar(SM->dd, c->data[i]), i==c->size-1?bInfPolarity:!pos);
    tmp_lit = Cudd_NotCond(Cudd_bddIthVar(SM->dd, c->data[i]), !pos);
    //d7_printf1("\n"); printBDD(SM->dd, tmp_lit);
    P = Cudd_bddAnd(SM->dd, tmp_lit, bTemp = P); cuddRef(P);
    Cudd_IterDerefBdd(SM->dd, bTemp);
  }

  BDDmininf(SM->dd, bdd, P, &SM->uTempList1);

  Cudd_IterDerefBdd(SM->dd, P);
  Cudd_IterDerefBdd(SM->dd, bdd);
  c->size++;

  d7_printf1("reduced clause: ")
  for(uintmax_t i = 0; i < SM->uTempList1.nLength; i++) {
    d7_printf2("%jd ", SM->pTrail[pSmurfInfo->pIndex2Var[SM->uTempList1.pList[i]]]==1?-(literal)pSmurfInfo->pIndex2Var[SM->uTempList1.pList[i]]:(literal)pSmurfInfo->pIndex2Var[SM->uTempList1.pList[i]]);
  }
  d7_printf2("%jd \n", bInfPolarity?pSmurfInfo->pIndex2Var[nInfVar]:-pSmurfInfo->pIndex2Var[nInfVar]);
  
  Cudd_AutodynEnable(SM->dd, CUDD_REORDER_SAME);
}

void minimize_conflict_clause(SmurfManager *SM, clause *smurf_clause, SmurfInfoStruct *pSmurfInfo, intmax_t nInfVar, uintmax_t nVbleIndex) {
  d7_printf2("Creating reason clause for %jd: ", nInfVar);
  for(uintmax_t i = 0; i < smurf_clause->size; i++) {
    d7_printf2("%jd ", SM->pTrail[pSmurfInfo->pIndex2Var[smurf_clause->data[i]]]==1?-pSmurfInfo->pIndex2Var[smurf_clause->data[i]]:pSmurfInfo->pIndex2Var[smurf_clause->data[i]]);
  }
  d7_printf2("%jd\n", nInfVar);

  if(minimize_smurf_clauses && smurf_clause->size > 2 && SM->arrInitialSmurfStates[pSmurfInfo->nSmurfIndex]->type == FN_SMURF) {
    clausePush(smurf_clause, nVbleIndex);
    _minimize_conflict_clause(SM, pSmurfInfo, smurf_clause, ((SmurfStateEntry *)SM->arrInitialSmurfStates[pSmurfInfo->nSmurfIndex])->pSmurfBDD, nInfVar>0);
    smurf_clause->size = SM->uTempList1.nLength;
    for(uintmax_t i = 0; i < SM->uTempList1.nLength; i++) {
      smurf_clause->data[i] = SM->pTrail[pSmurfInfo->pIndex2Var[SM->uTempList1.pList[i]]]==1?-(literal)pSmurfInfo->pIndex2Var[SM->uTempList1.pList[i]]:(literal)pSmurfInfo->pIndex2Var[SM->uTempList1.pList[i]];
    }
  } else {
    for(uintmax_t i = 0; i < smurf_clause->size; i++) {
      smurf_clause->data[i] = SM->pTrail[pSmurfInfo->pIndex2Var[smurf_clause->data[i]]]==1?-(literal)pSmurfInfo->pIndex2Var[smurf_clause->data[i]]:(literal)pSmurfInfo->pIndex2Var[smurf_clause->data[i]];
    }
  }
  
  smurf_clause->data[smurf_clause->size++] = nInfVar;

  d7_printf1("final clause: ");
  for(uintmax_t i = 0; i < smurf_clause->size; i++) {
    d7_printf2("%jd ", smurf_clause->data[i]);
  }
  d7_printf1("\n");
}

inline
uint8_t ApplyNextInferenceToStates(uintptr_t *_SM, uintmax_t nVariable, uint8_t bPolarity) {
  uint8_t ret = FS_UNKNOWN;
  SmurfManager *SM = (SmurfManager *)_SM;

  TypeStateEntry_pundoer *pCurrentState_pundoer = &SM->pCurrentState_pundoer;
  uint32_t_undoer *nLemmaLength_undoer = &SM->nLemmaLength_undoer;

  int8_t_undoer_push(&SM->pTrail_undoer, SM->pTrail + nVariable);
  SM->pTrail[nVariable] = bPolarity?1:-1;

  d7_printf2("  Applying inference %jd\n", bPolarity?nVariable:-nVariable);
  sbsat_stats[STAT_NUM_PROPAGATIONS]++;

  SmurfOccurrenceStruct_list pSmurfOccurrences = SM->arrVariableOccurrences.pList[nVariable];
  for(uintmax_t i = 0; i < pSmurfOccurrences.nLength; i++) {
    SmurfInfoStruct *pSmurfInfo = pSmurfOccurrences.pList[i].pSmurfInfo;
    uintmax_t nVbleIndex = pSmurfOccurrences.pList[i].nVbleIndex;

    d7_printf3("    Checking Smurf %ju (State %p)\n", pSmurfInfo->nSmurfIndex, (uintptr_t *)pSmurfInfo->pCurrentState);
    //d7_printf2("    Checking Smurf %ju\n", pSmurfInfo->nSmurfIndex);

    //Save old state to undoer stack
    TypeStateEntry *pOldState = pSmurfInfo->pCurrentState;

    TypeStateEntry_pundoer_push(pCurrentState_pundoer, &pSmurfInfo->pCurrentState);
    uint32_t_undoer_push(nLemmaLength_undoer, &pSmurfInfo->pLemma->size);

    //Apply Inference to Smurf
    
    ret = arrApplyInferenceToState[pOldState->type](SM, nVbleIndex, bPolarity, pSmurfInfo);

    if(ret == UNSAT) return FS_UNSAT;
    if(ret != NO_ERROR) assert(0);

    //If there was not a change, forget the undoer stack push
    if(pSmurfInfo->pCurrentState == pOldState) {
      TypeStateEntry_pundoer_forget(pCurrentState_pundoer);
      uint32_t_undoer_forget(nLemmaLength_undoer);
      d9_printf2("      no change, %p\n", (uintptr_t *)pSmurfInfo->pCurrentState);
    } else {
      d9_printf3("      pushing %ju, %p\n", pSmurfInfo->nSmurfIndex, (uintptr_t *)pSmurfInfo->pCurrentState);
      //Potentially update the heuristic
      //SEAN!!! Make this contingent on which heuristic is being used
      //if(((funcsat *)(SM->ESSM))->decisionLevel < 8)
      //SmurfStateHeuristicUpdate(SM, pOldState, pSmurfInfo);
    }
  }    

  if(gelim_smurfs) {
    ret = ApplyInferenceToXORGElimTable(SM->XM[SM->nCurrentDecisionLevel], nVariable, bPolarity);
    if(ret == UNSAT) return FS_UNSAT; //SEAN!!! bleck on return values.
  }

  return FS_UNKNOWN;
}

inline
void SmurfStatePush(uintptr_t *_SM) {
  SmurfManager *SM = (SmurfManager *)_SM;
  
  d7_printf2("  Pushing SmurfState at level %ju\n", SM->nCurrentDecisionLevel);
  TypeStateEntry_pundoer_push_marker(&SM->pCurrentState_pundoer);
  uint32_t_undoer_push_marker(&SM->nLemmaLength_undoer);
  literal_undoer_push_marker(&SM->pLemmaLiteral_undoer);
  double_undoer_push_marker(&SM->VarHeurWghts_undoer);
  int8_t_undoer_push_marker(&SM->pTrail_undoer);

  uintmax_t_undoer_push(&SM->generic_uintmax_t_undoer, &SM->nHeuristicPlaceholder);

  if(gelim_smurfs) pushXORGElimTable(SM->XM[SM->nCurrentDecisionLevel], SM->XM[SM->nCurrentDecisionLevel+1]);
  SM->nCurrentDecisionLevel++;
}

inline
uint8_t SmurfBacktrack(uintptr_t *_SM, uintmax_t new_level) {
  new_level++;
  uint8_t ret = FS_UNKNOWN;
  SmurfManager *SM = (SmurfManager *)_SM;

  if(SM->nCurrentDecisionLevel==1) if(new_level == 1) return ret;

  assert(new_level < SM->nCurrentDecisionLevel);

  do {
    SM->nCurrentDecisionLevel--;
    d7_printf2("  Backtracking to level %ju\n", SM->nCurrentDecisionLevel);

    sbsat_stats[STAT_NUM_BACKTRACKS]++;  

    TypeStateEntry_pundoer_undo_to_last_marker(&SM->pCurrentState_pundoer);
    uint32_t_undoer_undo_to_last_marker(&SM->nLemmaLength_undoer);
    literal_undoer_undo_to_last_marker(&SM->pLemmaLiteral_undoer);
    double_undoer_undo_to_last_marker(&SM->VarHeurWghts_undoer);
    int8_t_undoer_undo_to_last_marker(&SM->pTrail_undoer);
    
    uintmax_t_undoer_undo(&SM->generic_uintmax_t_undoer);
  } while(SM->nCurrentDecisionLevel != new_level);

  return ret;
}
