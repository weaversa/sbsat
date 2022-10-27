/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2012, University of Cincinnati.  All rights reserved.
 By using this software the USER indicates that he or she has read,
 understood and will comply with the following:

 --- University of Cincinnati hereby grants USER nonexclusive permission
 to use, copy and/or modify this software for internal, noncommercial,
 research purposes only. Any distribution, including commercial sale
 or license, of this software, copies of the software, its associated
 documentation and/or modifications of either is strictly prohibited
 without the prior consent of University of Cincinnati.  Title to copyright
 to this software and its associated documentation shall at all times
 remain with University of Cincinnati.  Appropriate copyright notice shall
 be placed on all software copies, and a complete copy of this notice
 shall be included in all copies of the associated documentation.
 No right is  granted to use in advertising, publicity or otherwise
 any trademark,  service mark, or the name of University of Cincinnati.


 --- This software and any associated documentation is provided "as is"

 UNIVERSITY OF CINCINNATI MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS
 OR IMPLIED, INCLUDING THOSE OF MERCHANTABILITY OR FITNESS FOR A
 PARTICULAR PURPOSE, OR THAT  USE OF THE SOFTWARE, MODIFICATIONS, OR
 ASSOCIATED DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS,
 TRADEMARKS OR OTHER INTELLECTUAL PROPERTY RIGHTS OF A THIRD PARTY.

 University of Cincinnati shall not be liable under any circumstances for
 any direct, indirect, special, incidental, or consequential damages
 with respect to any claim by USER or any third party on account of
 or arising from the use, or inability to use, this software or its
 associated documentation, even if University of Cincinnati has been advised
 of the possibility of those damages.
*********************************************************************/

#ifndef SMURF_SOLVER_H
#define SMURF_SOLVER_H

#include "smurfs.h"

#define SMURF_TABLE_SIZE 0x1000000

#define STATE_INIT_LIST {                       \
  initSmurfStateType,                           \
  initInferenceStateType,                       \
  initDroppedVarStateType,                      \
  initORStateType,                              \
  initORCOUNTERStateType,                       \
  initXORStateType,                             \
  initXORCOUNTERStateType,                      \
  initMINMAXStateType,                          \
  initMINMAXCOUNTERStateType,                   \
  initNEGMINMAXStateType,                       \
  initNEGMINMAXCOUNTERStateType,                \
  initXORFACTORStateType,                       \
  initANDEQUALSStateType,                       \
  initANDEQUALSCOUNTERStateType,                \
  NULL }

      //      initXORGElimStateType,                
      //      initNEGMINMAXStateType,                
      //      initNEGMINMAXCounterStateType,        

#define STATE_FREE_LIST {                       \
  NULL }
    //  LSGBORFree,				
    //  LSGBXORFree,				
    //  LSGBMINMAXFree,				
    //  LSGBNEGMINMAXFree,                        


typedef struct SmurfStatesTableStruct {
  uint32_t curr_size;
  uint32_t max_size;
  uint8_t *arrStatesTable; //Pointer to a table of smurf states.
  struct SmurfStatesTableStruct *pNext;
} SmurfStatesTableStruct;

typedef struct SmurfOccurrenceStruct {
  SmurfInfoStruct *pSmurfInfo;
  uintmax_t nVbleIndex;
} SmurfOccurrenceStruct;

create_list_headers(SmurfOccurrenceStruct);
create_list_headers(SmurfOccurrenceStruct_list);

typedef struct InferenceStruct {
  uintmax_t nVbleIndex;
  uint8_t bPolarity;
  SmurfInfoStruct *pSmurfInfo;
  uintmax_t saved_nLemmaLength;
} InferenceStruct;

create_list_headers(InferenceStruct);

typedef struct SmurfStack {
  uintmax_t nVarChoiceCurrLevel; //Index to array of size nNumVars
  uintmax_t nNumFreeVars;
  uintmax_t nNumSmurfsSatisfied;
  uintmax_t nHeuristicPlaceholder;
  TypeStateEntry **arrSmurfStates;   //Pointer to array of size nNumSmurfs
} SmurfStack;

typedef struct SmurfManager {
  uintmax_t nNumSmurfs;
  uintmax_t nNumVariables;

  SmurfStatesTableStruct *arrSmurfStatesTableHead; //Pointer to the table of all smurf states
  SmurfStatesTableStruct *arrCurrSmurfStatesTable; //Pointer to the current table of smurf states
  uint8_t *pSmurfStatesTableTail;                  //Pointer to the next open byte of the arrSmurfStatesTable

  SmurfStateEntry *pTrueSmurfState;

  SmurfOccurrenceStruct_list_list arrVariableOccurrences;
  SmurfInfoStruct_list arrSmurfStates;
  InferenceStruct_list arrInferenceInfo;

  TypeStateEntry **arrInitialSmurfStates;

  int8_t *pTrail; //The trail (does not contain information about the inference queue)
    
  BDDManager *BM;  //BDD Manager for printing and other misc. things
  DdManager *dd;   //CUDD BDD Manager
  uintptr_t *ESSM; //external SAT solver manager
  char ESSM_type;

  XORGElimTableStruct **XM; //Stack of Gaussian Elimination Managers
  
  double JHEURISTIC_K;
  double JHEURISTIC_K_TRUE;
  double JHEURISTIC_K_INF;
  double JHEURISTIC_K_UNKNOWN;
  double *arrPosVarHeurWghts;
  double *arrNegVarHeurWghts;
  uintmax_t nHeuristicPlaceholder;
  uintmax_t *pStaticHeurOrder;

  clause *conflictClause;

  uintmax_t_list uTempList1;
  uintmax_t_list uTempList2;
  uint32_t_list uTempList32;

  TypeStateEntry_pundoer pCurrentState_pundoer;
  uint32_t_undoer nLemmaLength_undoer;
  literal_undoer pLemmaLiteral_undoer;
  double_undoer VarHeurWghts_undoer;
  int8_t_undoer pTrail_undoer;
  uintmax_t_undoer generic_uintmax_t_undoer;

  //For holding state of the Smurfs in a non-incremental way.
  uintmax_t nCurrentDecisionLevel;
  SmurfStack *arrSmurfStack; //Pointer to array of size nNumVariables

} SmurfManager;
void Alloc_SmurfStack(SmurfManager *SM, uintmax_t destination);

#include "fn_smurf.h"
#include "fn_inference.h"
#include "fn_droppedvar.h"
#include "fn_or.h"
#include "fn_orcounter.h"
#include "fn_xor.h"
#include "fn_xorcounter.h"
#include "fn_minmax.h"
#include "fn_minmaxcounter.h"
#include "fn_negminmax.h"
#include "fn_negminmaxcounter.h"
#include "fn_xorfactor.h"
#include "fn_andequals.h"
#include "fn_andequalscounter.h"


//Prototypes for building and freeing Smurfs
TypeStateEntry *ReadSmurfStateIntoTable(SmurfManager *SM, DdNode *pCurrentBDD);
uint8_t AddBDDToSmurfManager(SmurfManager *SM, DdNode *bdd);
uint8_t check_SmurfStatesTableSize(SmurfManager *SM, uintmax_t size);
void chomp_SmurfStatesTable(SmurfManager *SM, uintmax_t size);
void FreeSmurfStatesTable(SmurfManager *SM);
void FreeSmurfStateEntries(SmurfManager *SM);
void UnsetAllStatesVisitedFlag(SmurfManager *SM);
void GarbageCollectSmurfStatesTable(SmurfManager *SM);

//Prototypes for displaying Smurfs
void PrintAllSmurfStateEntries(SmurfManager *SM);
void _PrintSmurf_dot(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t *pIndex2Var);
void PrintSmurf_dot(SmurfManager *SM, uintmax_t nSmurfIndex);

extern uint8_t (*IsVariableAssigned_hook)(SmurfManager *SM, uintmax_t nVariable);
extern uint8_t (*EnqueueInference_hook)(SmurfManager *SM, SmurfInfoStruct *pSmurfInfo, uintmax_t nVariable, uint8_t bPolarity);

//Core functions

void SmurfStatePush(uintptr_t *_SM);
uint8_t SmurfBacktrack(uintptr_t *_SM, uintmax_t new_level);
uint8_t ApplyNextInferenceToStates(uintptr_t *_SM, uintmax_t nVariable, uint8_t bPolarity);

uint8_t IsVariableAssigned_smurf(SmurfManager *SM, uintmax_t nVariable);
uint8_t EnqueueInference_smurf(SmurfManager *SM, SmurfInfoStruct *pSmurfInfo, uintmax_t nVariable, uint8_t bPolarity);

void populateLSGBScores(SmurfManager *SM);
double get_heuristic_score_for_var(SmurfManager *SM, uintmax_t nVariable, uint8_t bPolarity);
void SmurfStateHeuristicUpdate(SmurfManager *SM, TypeStateEntry *pOldState, SmurfInfoStruct *pSmurfInfo);
intmax_t LSGBHeuristic(SmurfManager *SM);
intmax_t NULLHeuristic(SmurfManager *SM);
intmax_t StaticHeuristic(SmurfManager *SM);
intmax_t RandomHeuristic(SmurfManager *SM);

void CalculateSmurfLSGBHeuristic(SmurfManager *SM, SmurfInfoStruct *pSmurfInfo); //SEAN!!! yuck

void minimize_conflict_clause(SmurfManager *SM, clause *smurf_clause, SmurfInfoStruct *pSmurfInfo, intmax_t nInfVar, uintmax_t nVbleIndex);

typedef void (*InitStateType)();
extern InitStateType arrInitStateType[];

typedef void (*FreeStateType)();
extern FreeStateType arrFreeStateType[];

extern uint8_t arrGetStateType[NUM_SMURF_TYPES]; //Node, Transition, or Leaf

typedef uint8_t (*ApplyInferenceToState)(SmurfManager *SM, uintmax_t nVbleIndex, uint8_t bPolarity, SmurfInfoStruct *pSmurfInfo);
extern ApplyInferenceToState arrApplyInferenceToState[NUM_SMURF_TYPES];

typedef TypeStateEntry *(*CreateStateEntry)(SmurfManager *SM, DdNode *pBDD);
extern CreateStateEntry arrCreateStateEntry[NUM_SMURF_TYPES];

typedef void (*FreeStateEntry)(SmurfManager *SM, TypeStateEntry *pTypeState);
extern FreeStateEntry arrFreeStateEntry[NUM_SMURF_TYPES];

typedef void (*SetStateVisitedFlag)(TypeStateEntry *pTypeState);
extern SetStateVisitedFlag arrSetStateVisitedFlag[NUM_SMURF_TYPES];

typedef void (*UnsetStateVisitedFlag)(TypeStateEntry *pTypeState);
extern UnsetStateVisitedFlag arrUnsetStateVisitedFlag[NUM_SMURF_TYPES];

typedef void (*CleanUpStateEntry)(TypeStateEntry *pTypeState);
extern CleanUpStateEntry arrCleanUpStateEntry[NUM_SMURF_TYPES];

typedef uintmax_t (*CalculateStateSize)(TypeStateEntry *pTypeState);
extern CalculateStateSize arrStatesTypeSize[NUM_SMURF_TYPES];

typedef void (*PrintStateEntry)(SmurfManager *SM, TypeStateEntry *pTypeState);
extern PrintStateEntry arrPrintStateEntry[NUM_SMURF_TYPES];

typedef void (*PrintStateEntry_dot)(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t *pIndex2Var);
extern PrintStateEntry_dot arrPrintStateEntry_dot[NUM_SMURF_TYPES];

typedef void (*CalculateStateHeuristic)(SmurfManager *SM, SmurfInfoStruct *pSmurfInfo);
extern CalculateStateHeuristic arrCalculateStateHeuristic[NUM_SMURF_TYPES];

typedef void (*SetStateHeuristicScore)(TypeStateEntry *pTypeState);
extern SetStateHeuristicScore arrSetStateHeuristicScore[NUM_SMURF_TYPES];

typedef double (*GetStateHeuristicScore)(SmurfManager *SM, TypeStateEntry *pTypeState);
extern GetStateHeuristicScore arrGetStateHeuristicScore[NUM_SMURF_TYPES];

typedef double (*GetVarHeuristicScore)(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t nVbleIndex, uint8_t bPolarity);
extern GetVarHeuristicScore arrGetVarHeuristicScore[NUM_SMURF_TYPES];

typedef void (*FreeHeuristicScore)(SmurfManager *SM);
extern FreeHeuristicScore arrFreeHeuristicScore[NUM_SMURF_TYPES];

#endif
