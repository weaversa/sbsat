#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

int find_xor_factors;

TypeStateEntry *CreateXORFACTORState(SmurfManager *SM, DdNode *pXORFACTORBDD) {
  XORFACTORStateEntry *pXORFACTORState = NULL;

  if(!find_xor_factors || SM->uTempList1.nLength < functionTypeLimits[FN_TYPE_XOR]) return (TypeStateEntry *)pXORFACTORState; 

  DdNode *factors = find_all_linear_factors(SM->dd, pXORFACTORBDD, (intmax_t_list *)&SM->uTempList1, SM->BM->nHighestVarInABDD+1);
  if(factors == NULL) return (TypeStateEntry *)pXORFACTORState;
  Cudd_Ref(factors);

  if(factors != DD_ONE(SM->dd)) {
    DdNode *factor = create_first_factor(SM->dd, factors, SM->BM->nHighestVarInABDD+1);
    Cudd_Ref(factor);

    DdNode *should_be_zero = Cudd_bddAnd(SM->dd, pXORFACTORBDD, Cudd_Not(factor));
    assert(should_be_zero == Cudd_Not(DD_ONE(SM->dd)));
    
    //Build XORFACTOR state
    Cudd_Ref(pXORFACTORBDD);

    uint8_t ret = check_SmurfStatesTableSize(SM, (uintmax_t)sizeof(XORFACTORStateEntry));
    if(ret != NO_ERROR) {assert(0); return NULL;}
  
    pXORFACTORState = (XORFACTORStateEntry *)SM->pSmurfStatesTableTail;
    chomp_SmurfStatesTable(SM, (uintmax_t)sizeof(XORFACTORStateEntry));
    sbsat_stats[STAT_XORFACTOR_STATES_BUILT]+=1;
    sbsat_stats[STAT_STATES_BUILT]+=1;
    
    pXORFACTORState->type             = FN_XORFACTOR;
    pXORFACTORState->visited          = 0;
    find_xor_factors = 0; //YUCK!!! This is being done to refresh SM->uTempList1
    pXORFACTORState->pXORFactor       = (TypeStateEntry *)ReadSmurfStateIntoTable(SM, factor);
    find_xor_factors = 1; //YUCK!!!
    //CreateXORCOUNTERState(SM, factor);
    assert(pXORFACTORState->pXORFactor != NULL);
    pXORFACTORState->pXORFACTORBDD    = pXORFACTORBDD;

    //Remove this factor from the current BDD and create transition
    //DdNode *bTemp = Cudd_bddConstrain(SM->dd, pXORFACTORBDD, factor);
    DdNode *bTemp = better_gcf(SM->BM, pXORFACTORBDD, factor);
    Cudd_Ref(bTemp);
    Cudd_IterDerefBdd(SM->dd, factor);
    assert(bTemp != pXORFACTORBDD);

    pXORFACTORState->pTransition = ReadSmurfStateIntoTable(SM, bTemp);
    Cudd_IterDerefBdd(SM->dd, bTemp);
  }

  Cudd_IterDerefBdd(SM->dd, factors);

  return (TypeStateEntry *)pXORFACTORState;
}
