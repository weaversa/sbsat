/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2007, University of Cincinnati.  All rights reserved.
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

#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

void PrintAllSmurfStateEntries(SmurfManager *SM) {
  for(SmurfStatesTableStruct *pIter = SM->arrSmurfStatesTableHead; pIter != NULL; pIter = pIter->pNext) {
    uint8_t *pState = (uint8_t *)pIter->arrStatesTable;
    uint8_t *pStateMax = pState + SM->arrCurrSmurfStatesTable->max_size;
    do {
      TypeStateEntry *pTypeState = (TypeStateEntry *)pState;
      uintmax_t size = arrStatesTypeSize[pTypeState->type](pTypeState);
      if(pTypeState->type!=FN_FREE_STATE) {
	arrPrintStateEntry[pTypeState->type](SM, pTypeState);
      }
      pState+=size;
    } while(pState < pStateMax);
  }
}

uint8_t compress_transitions_dot = 1;

void _PrintSmurf_dot(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t *pIndex2Var) {
  if(pTypeState == NULL) {
    fprintf(stdout, " b%p [shape=box, fontname=""Helvetica"",label=""X""];\n", (void *)pTypeState);
    return;
  }

  if(pTypeState->type == FN_SMURF) {
    if(pTypeState->visited == 1) return;
    pTypeState->visited = 1;

    if(compress_transitions_dot == 1) {
      PrintSmurfStateEntry_inferences_compressed_dot(SM, pTypeState, pIndex2Var);
    } else {
      PrintSmurfStateEntry_dot(SM, pTypeState, pIndex2Var);
    }
    SmurfStateEntry *pSmurfState = (SmurfStateEntry *)pTypeState;
    SmurfStateTransition *pSmurfTransition = (SmurfStateTransition *)(pSmurfState + 1);
    for(uintmax_t i = 0; i < pSmurfState->nNumVariables; i++) {
      _PrintSmurf_dot(SM, pSmurfTransition->pVarIsTrueTransition, pIndex2Var);
      _PrintSmurf_dot(SM, pSmurfTransition->pVarIsFalseTransition, pIndex2Var);
      pSmurfTransition++;
    }
  } else if(compress_transitions_dot == 1 && (pTypeState->type == FN_INFERENCE || pTypeState->type == FN_DROPPEDVAR || pTypeState->type == FN_XORFACTOR)) {
    TypeStateEntry *pNextState = pTypeState;
    //The FN_SMURF printer will display the inferences and dropped vars in a compressed
    //form, so we skip them here.
    
    while(pNextState && (pNextState->type == FN_INFERENCE ||
			 pNextState->type == FN_DROPPEDVAR ||
			 pNextState->type == FN_XORFACTOR)) {
      if(pNextState->type == FN_INFERENCE)
	pNextState = ((InferenceStateEntry *)pNextState)->pTransition;
      else if(pNextState->type == FN_DROPPEDVAR) {
	pNextState = ((DroppedVarStateEntry *)pNextState)->pTransition;
      } else {
	assert(pNextState->type == FN_XORFACTOR);
	pNextState = ((XORFACTORStateEntry *)pNextState)->pTransition;
      }
    }

    _PrintSmurf_dot(SM, pNextState, pIndex2Var);

  } else if(pTypeState->type == FN_INFERENCE) {
    InferenceStateEntry *pInferenceState = (InferenceStateEntry *)pTypeState;
    arrPrintStateEntry_dot[pTypeState->type](SM, pTypeState, pIndex2Var);
    _PrintSmurf_dot(SM, pInferenceState->pTransition, pIndex2Var);
  } else if(pTypeState->type == FN_DROPPEDVAR) {
    DroppedVarStateEntry *pDroppedVarState = (DroppedVarStateEntry *)pTypeState;
    arrPrintStateEntry_dot[pTypeState->type](SM, pTypeState, pIndex2Var);
    _PrintSmurf_dot(SM, pDroppedVarState->pTransition, pIndex2Var);
  } else if(pTypeState->type == FN_XORFACTOR) {
    XORFACTORStateEntry *pXORFACTORState = (XORFACTORStateEntry *)pTypeState;
    arrPrintStateEntry_dot[pTypeState->type](SM, pTypeState, pIndex2Var);
    _PrintSmurf_dot(SM, pXORFACTORState->pTransition, pIndex2Var);
  } else {
    //Leaf node
    arrPrintStateEntry_dot[pTypeState->type](SM, pTypeState, pIndex2Var);
  }
}

void PrintSmurf_dot(SmurfManager *SM, uintmax_t nSmurfIndex) {
  fprintf(stdout, "digraph Smurf {\n");
  //fprintf(stdout, " graph [concentrate=false, rankdir=TB];\n");
  fprintf(stdout, " graph [concentrate=false, rankdir=TB, nodesep=\"0.25\", ranksep=\"2.25\", ordering=in];\n");
  //fprintf(stdout, " graph [concentrate=true, rankdir=TB];\n");
  //fprintf(stdout, " graph [concentrate=false, ordering=in, rankdir=TB, nodesep=\"0.25\", ranksep=\"1.5\"];\n");
  //fprintf(stdout, " graph [concentrate=false, nodesep=\"0.30\", ordering=in, rankdir=TB, ranksep=\"0.5\"];\n");
  //fprintf(stdout, " graph [concentrate=true, nodesep=\"0.30\", ordering=in, rankdir=LR, ranksep=\"2.25\"];\n");

  fprintf(stdout, " b%p [shape=box, fontname=""Helvetica"",label=""True""];\n", (void *)SM->pTrueSmurfState);

  _PrintSmurf_dot(SM, SM->arrSmurfStates.pList[nSmurfIndex].pCurrentState, SM->arrSmurfStates.pList[nSmurfIndex].pIndex2Var);
  fprintf(stdout, "}\n");

  //Clear visited flags
  UnsetAllStatesVisitedFlag(SM);
}

//Functions for printing (via dot) the corresponding SMURFS for sets of BDDs.
uint8_t PrintSmurfs(BDDManager *BM, DdNode **bdds, uintmax_t nLength) {
  uint8_t ret = NO_ERROR;

  uintmax_t bddi = BM->nNumBDDs;
  for(uintmax_t i = 0; i < nLength; i++) {
    ret = add_BDD_to_manager(BM, bdds[i]);
    if(ret != NO_ERROR) return ret;
  }
  
  SmurfManager *SM = Init_SmurfManager(BM);
  
  for(uintmax_t i = bddi; i < nLength+bddi; i++) {
    ret = AddBDDToSmurfManager(SM, BM->BDDList[i]);
    if(ret != NO_ERROR) return ret;
    PrintSmurf_dot(SM, i-bddi);
  }

  for(uintmax_t i = bddi; i < nLength+bddi; i++)
    remove_BDD_from_manager(BM, bddi);

  ret = Final_SimpleSmurfSolver(SM);
  if(ret != NO_ERROR) return ret;

  return ret;
}

/*
void print_dot_inference(int var) {
  if(just_flipped_choicepoint < 0) {
    just_flipped_choicepoint = -just_flipped_choicepoint; //skip past the old choicepoint
  } else {
    fprintf(stdout, " i%d [shape=\"circle\",label=\"%d\"];\n", ite_counters[UNIQUE_NUM], var);
    if(ite_counters[UNIQUE_NUM] != 1) {
      if(just_flipped_choicepoint > 0) {
	fprintf(stdout, " i%d -> i%d [style=solid,label=\"%c\"];\n", just_flipped_choicepoint, ite_counters[UNIQUE_NUM], var>0?'+':'-');
	just_flipped_choicepoint = 0;
      } else {
	fprintf(stdout, " i%d -> i%d [style=solid,label=\"%c\"];\n", ite_counters[UNIQUE_NUM]-1, ite_counters[UNIQUE_NUM], var>0?'+':'-');
      }
    }
  }
}

void PrintAllXORSmurfStateEntries() {
  void **arrSmurfStates = SM->arrSmurfStack[SM->nCurrSearchTreeLevel].arrSmurfStates;
  
  for(int i = 0; i < SM->nNumSmurfs; i++) {
    if(arrSmurfStates[i] == pTrueSimpleSmurfState) continue;
    void *pState = arrSmurfStates[i];
    if(((TypeStateEntry *)pState)->type == FN_XOR_COUNTER) {
      PrintXORStateEntry_formatted((void *)((XORCounterStateEntry *)pState)->pXORState);
    } else if(((TypeStateEntry *)pState)->type == FN_XOR) {
      PrintXORStateEntry_formatted(pState);
    }
  }
  d2_printf1("\n");
}

extern int solutions_overflow;
void DisplaySimpleStatistics(uintmax_t nNumChoicePts, uintmax_t nNumBacktracks, uintmax_t nNumBackjumps) {
  d2_printf2("Choice Points: %ju", nNumChoicePts);
  d2_printf3(", Backtracks: %ju, Backjumps: %ju \n",
	     nNumBacktracks, nNumBackjumps);
}

ITE_INLINE
void CalculateSimpleSolverProgress(int *_whereAmI, int *_total) {
  int whereAmI=0;
  int total=0;
  int soft_count=14;
  int hard_count=28;
  int count=0;
  int nInfQueueTail = 0;
  
  if(SM->nCurrSearchTreeLevel >= 0)
    nInfQueueTail = SM->arrSmurfStack[SM->nCurrSearchTreeLevel].nNumFreeVars;
  
  int nCurrSearchTreeLevel = 0;
  for(int i = nInfQueueStart; i < nInfQueueTail && (count<soft_count || (count < hard_count && whereAmI==0)); i++) {
    int nBranchLit = SM->arrInferenceQueue[i];
    if(SM->arrInferenceDeclaredAtLevel[abs(nBranchLit)] < 0 ||
       (SM->arrInferenceDeclaredAtLevel[abs(nBranchLit)] == 0 && add_one_display==1)) {
      //This variable is an old choicepoint
      whereAmI *= 2;
      whereAmI += 1;
      total *= 2;
      total += 1;
      count++;
    } else if(nBranchLit == SM->arrInferenceQueue[SM->arrSmurfStack[nCurrSearchTreeLevel].nNumFreeVars]) {
      //This variable is a current choicepoint
      whereAmI *= 2;
      total *= 2;
      total += 1;
      nCurrSearchTreeLevel++;
      count++;
    }		
  }
  *_whereAmI = whereAmI;
  *_total    = total;
}

void DisplaySimpleSolverBacktrackInfo() {
  fSimpleSolverEndTime = ite_counters_f[CURRENT_TIME] = get_runtime();
  double fTotalDurationInSecs = fSimpleSolverEndTime - fSimpleSolverStartTime;
  double fDurationInSecs = fSimpleSolverEndTime - fSimpleSolverPrevEndTime;
  double fBacktracksPerSec = BACKTRACKS_PER_STAT_REPORT / (fDurationInSecs>0?fDurationInSecs:0.001);
  fSimpleSolverPrevEndTime = fSimpleSolverEndTime;
  
  d2_printf2("Time: %4.3fs. ", fTotalDurationInSecs);
  d2_printf3("Backtracks: %ld (%4.3f per sec) ",
	     (long)ite_counters[NUM_BACKTRACKS], fBacktracksPerSec);
  
  int whereAmI = 0;
  int total = 0;
  double progress = 0.0;
  CalculateSimpleSolverProgress(&whereAmI, &total);
  if (total == 0) total=1;
  progress = ite_counters_f[PROGRESS] = (float)whereAmI*100/total;
  //d2_printf3("Progress: %p/%p        ", whereAmI, total);
  d2_printf1("Progress: ");
  char number[10];
  char back[10] = "\b\b\b\b\b\b\b\b\b";
  sprintf(number, "% 3.2f%%", progress);
  //sprintf(number, " ?");
  back[strlen(number)]=0;
  if ((DEBUG_LVL&15) == 1) {
    fprintf(stderr, "%s%s", number, back);
  } else {
    D_1(
	d0_printf3("%s%s", number, back);
	fflush(stddbg);
	)
      }
  
  d2_printf1("\n Choices (total, dependent" );
  dC_printf4("c %4.3fs. Progress %s Choices: %lld\n", fTotalDurationInSecs, number, ite_counters[NUM_CHOICE_POINTS]);
  if (backjumping) d2_printf1(", backjumped");
  d2_printf3("): (%lld, %lld", ite_counters[NUM_CHOICE_POINTS], ite_counters[HEU_DEP_VAR]);
  if (backjumping) d2_printf2(", %lld", ite_counters[NUM_TOTAL_BACKJUMPS]);
  d2_printf1(")");
  
  d2_printf1("\n");
  d2_printf1(" Inferences by ");
  if(ite_counters[INF_SMURF]>0) d2_printf2("smurfs: %lld; ", ite_counters[INF_SMURF]);
  if(ite_counters[INF_SPEC_FN_OR]>0) d2_printf2("ORs: %lld; ", ite_counters[INF_SPEC_FN_OR]);
  if(ite_counters[INF_SPEC_FN_XOR]>0)	d2_printf2("XORs: %lld; ", ite_counters[INF_SPEC_FN_XOR]);
  if(ite_counters[INF_BB_GELIM]>0) d2_printf2("GELIM: %lld; ", ite_counters[INF_BB_GELIM]);
  if(ite_counters[INF_SPEC_FN_MINMAX]>0)	d2_printf2("MINMAXs: %lld; ", ite_counters[INF_SPEC_FN_MINMAX]);
  if(ite_counters[INF_SPEC_FN_NEGMINMAX]>0)	d2_printf2("NEGMINMAXs: %lld; ", ite_counters[INF_SPEC_FN_NEGMINMAX]);
  if(ite_counters[INF_LEMMA]>0) d2_printf2("lemmas: %lld; ", ite_counters[INF_LEMMA]);
  d2_printf1("\n");
  
  d2_printf1(" Backtracks by ");
  if(ite_counters[ERR_BT_SMURF]>0) d2_printf2("smurfs: %lld; ", ite_counters[ERR_BT_SMURF]);
  if(ite_counters[ERR_BT_SPEC_FN_OR]>0) d2_printf2("ORs: %lld; ", ite_counters[ERR_BT_SPEC_FN_OR]);
  if(ite_counters[ERR_BT_SPEC_FN_XOR]>0) d2_printf2("XORs: %lld; ", ite_counters[ERR_BT_SPEC_FN_XOR]);
  if(ite_counters[ERR_BT_BB_GELIM]>0) d2_printf2("GELIM: %lld; ", ite_counters[ERR_BT_BB_GELIM]);
  if(ite_counters[ERR_BT_SPEC_FN_MINMAX]>0) d2_printf2("MINMAXs: %lld; ", ite_counters[ERR_BT_SPEC_FN_MINMAX]);
  if(ite_counters[ERR_BT_SPEC_FN_NEGMINMAX]>0) d2_printf2("NEGMINMAXs: %lld; ", ite_counters[ERR_BT_SPEC_FN_NEGMINMAX]);
  if(ite_counters[ERR_BT_LEMMA]>0) d2_printf2("lemmas: %lld; ", ite_counters[ERR_BT_LEMMA]);
  d2_printf1("\n");
  if (backjumping) d2_printf3(" Backjumps: %ld (avg bj len: %.1f)\n",
			      (long)ite_counters[NUM_TOTAL_BACKJUMPS],
			      (float)ite_counters[NUM_TOTAL_BACKJUMPS]/(1+ite_counters[NUM_BACKJUMPS]));
  if (autarky) d2_printf3(" Autarkies: %ld (avg au len: %.1f)\n",
			  (long)ite_counters[NUM_TOTAL_AUTARKIES],
			  (float)ite_counters[NUM_TOTAL_AUTARKIES]/(1+ite_counters[NUM_AUTARKIES]));
  if (max_solutions != 1 && solutions_overflow==0) { d2_printf3(" Solutions found: %lld/%lld\n", ite_counters[NUM_SOLUTIONS], max_solutions); }
  else if(solutions_overflow) d2_printf2(" Solutions found: > %lld\n", ~(((long long)1)<<(long long)63));
  
  d2_printf1("\n");
}

void DisplaySimpleSolverBacktrackInfo_gnuplot() {
  fSimpleSolverEndTime = ite_counters_f[CURRENT_TIME];
  double fTotalDurationInSecs = fSimpleSolverEndTime - fSimpleSolverStartTime;
  double fDurationInSecs = fSimpleSolverEndTime - fSimpleSolverPrevEndTime;
  double fBacktracksPerSec = BACKTRACKS_PER_STAT_REPORT / (fDurationInSecs>0?fDurationInSecs:0.001);
  
  fprintf(fd_csv_trace_file, "%4.3f ", fTotalDurationInSecs);
  fprintf(fd_csv_trace_file, "%ld %4.3f ",
	  (long)ite_counters[NUM_BACKTRACKS], fBacktracksPerSec);
  
  int whereAmI = 0;
  int total = 0;
  double progress = 0.0;
  CalculateSimpleSolverProgress(&whereAmI, &total);
  if (total == 0) total=1;
  progress = ite_counters_f[PROGRESS] = (float)whereAmI*100/total;
  fprintf(fd_csv_trace_file, "%4.3f ", progress);
  
  fprintf(fd_csv_trace_file, "%lld %lld ", ite_counters[NUM_CHOICE_POINTS], ite_counters[HEU_DEP_VAR]);
  fprintf(fd_csv_trace_file, "%lld ", ite_counters[NUM_INFERENCES]);
  fprintf(fd_csv_trace_file, "%lld ", ite_counters[NUM_BACKTRACKS]);
  fprintf(fd_csv_trace_file, "%ld %ld\n", (long)ite_counters[NUM_SOLUTIONS], (long)max_solutions);
}
*/
