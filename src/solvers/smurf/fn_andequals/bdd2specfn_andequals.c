#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

TypeStateEntry *CreateANDEQUALSState(SmurfManager *SM, DdNode *pANDEQUALSBDD) {
  ANDEQUALSStateEntry *pANDEQUALSState = NULL;

  DdNode *equ_var;
  
  if((SM->uTempList1.nLength >= functionTypeLimits[FN_TYPE_AND_EQU]) &&
     (equ_var = isAND_EQU(SM->dd, pANDEQUALSBDD, &SM->uTempList1)) != DD_ONE(SM->dd)) {
    assert(equ_var != NULL);
    Cudd_Ref(pANDEQUALSBDD);

    //Build ANDEQUALS state
    
    uint8_t ret = check_SmurfStatesTableSize(SM, (uintmax_t)sizeof(ANDEQUALSStateEntry));
    if(ret != NO_ERROR) {assert(0); return NULL;}
    
    pANDEQUALSState = (ANDEQUALSStateEntry *)SM->pSmurfStatesTableTail;
    chomp_SmurfStatesTable(SM, (uintmax_t)sizeof(ANDEQUALSStateEntry));
    sbsat_stats[STAT_ANDEQUALS_STATES_BUILT]+=1;
    sbsat_stats[STAT_STATES_BUILT]+=1;
    
    pANDEQUALSState->type            = FN_ANDEQUALS;
    pANDEQUALSState->visited         = 0;
    pANDEQUALSState->nLHS            = Cudd_Regular(equ_var)->index;
    pANDEQUALSState->bAND_or_OR      = !Cudd_IsComplement(equ_var);
    pANDEQUALSState->pANDEQUALSBDD   = pANDEQUALSBDD;

    //Create the OR state
    DdNode *pORBDD = Cudd_NotCond(Cudd_Cofactor(SM->dd, pANDEQUALSBDD, Cudd_Regular(equ_var)), !Cudd_IsComplement(equ_var));
    Cudd_Ref(pORBDD);
    assert(isOR(SM->dd, pORBDD));
    pANDEQUALSState->pORState = (ORStateEntry *)ReadSmurfStateIntoTable(SM, pORBDD);
    //Could be OR or ORCOUNTER - this is fixed by the FN_ANDEQUALSCOUNTER creation code
    assert(pANDEQUALSState->pORState->type == FN_OR ||
	   pANDEQUALSState->pORState->type == FN_ORCOUNTER);
    Cudd_IterDerefBdd(SM->dd, pORBDD);
  }

  //Leaf node, just return
  return (TypeStateEntry *)pANDEQUALSState;
}
