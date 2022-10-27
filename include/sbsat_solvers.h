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

#ifndef SBSAT_SOLVERS_H
#define SBSAT_SOLVERS_H

/*
  
enum {
   NULL_HEURISTIC,
   JOHNSON_HEURISTIC,
   C_LEMMA_HEURISTIC,
   JOHNSON_LEMMA_HEURISTIC,
   INTERACTIVE_HEURISTIC,
   STATE_HEURISTIC
};

typedef struct _t_solution_info {
  int *arrElts;
  int nNumElts;
  struct _t_solution_info *next;
} t_solution_info;

extern t_solution_info *solution_info;
extern t_solution_info *solution_info_head;

extern SmurfManager *SM_main;

SmurfManager *Init_SmurfManager(BDDManager *BM);
uint8_t ReadAllSmurfsIntoTable(SmurfManager *SM, BDDManager *BM);
uint8_t Final_SimpleSmurfSolver(SmurfManager *SM);

uint8_t SmurfSolve(SmurfManager *SM);
int picosatSolve();

void PrintAllSmurfStateEntries();
uint8_t PrintSmurfs(BDDManager *BM, DdNode **bdds, uintmax_t nLength);

SmurfManager *Init_SmurfSolver(BDDManager *BM);
SmurfManager *Init_SmurfSolver_picosat(BDDManager *BM, PicosatManager *PM);

extern char solver_flag;
extern int turn_off_smurf_normalization;
extern int use_dropped_var_smurfs;
extern int find_xor_factors;
extern int gelim_smurfs;
extern int precompute_smurfs;
extern int minimize_smurf_clauses;

extern char sHeuristic;
extern float JHEURISTIC_K;
extern float JHEURISTIC_K_TRUE;
extern float JHEURISTIC_K_INF;
extern float JHEURISTIC_K_UNKNOWN;

*/

#endif
