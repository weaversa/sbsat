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

#include "sbsat.h"

double ***arrNEGMINMAXWghts = NULL;
uintmax_t *arrMaxNEGMINMAXTrue = NULL;
uintmax_t *arrMaxNEGMINMAXFalse = NULL;

uintmax_t nNEGMINMAXWghtsSize = 0;

//---------------------------------------------------------------

double LSGBNEGMINMAXCOUNTERStateGetHeurScore(SmurfManager *SM, TypeStateEntry *pTypeState) {
  NEGMINMAXCOUNTERStateEntry *pNEGMINMAXCOUNTERState = (NEGMINMAXCOUNTERStateEntry *)pTypeState;
  assert(pNEGMINMAXCOUNTERState->type == FN_NEGMINMAXCOUNTER);

  NEGMINMAXStateEntry *pNEGMINMAXState = pNEGMINMAXCOUNTERState->pNEGMINMAXState;

  uintmax_t nNumVariables = pNEGMINMAXCOUNTERState->nNumVariables;
  uintmax_t nNumTrue = pNEGMINMAXCOUNTERState->nNumTrue;
  uintmax_t nMin = nNumTrue > pNEGMINMAXState->nMin ? 0 : pNEGMINMAXState->nMin - nNumTrue;
  uintmax_t nMax = pNEGMINMAXState->nMax - nNumTrue;
  uintmax_t nDiff = nMax - nMin; // Find out the length of the diff (max-min)
  
  if(nNEGMINMAXWghtsSize < nDiff+1) {
    arrNEGMINMAXWghts = (double ***)sbsat_recalloc(arrNEGMINMAXWghts, nNEGMINMAXWghtsSize, nDiff+1, sizeof(double **), 2, "arrNEGMINMAXWghts");
    arrMaxNEGMINMAXTrue = (uintmax_t *)sbsat_recalloc(arrMaxNEGMINMAXTrue, nNEGMINMAXWghtsSize, nDiff+1, sizeof(uintmax_t), 2, "arrMaxNEGMINMAXTrue");
    arrMaxNEGMINMAXFalse = (uintmax_t *)sbsat_recalloc(arrMaxNEGMINMAXFalse, nNEGMINMAXWghtsSize, nDiff+1, sizeof(uintmax_t), 2, "arrMaxNEGMINMAXFalse");
    nNEGMINMAXWghtsSize = nDiff+1;
  }

  uint8_t recompute_true = 0;
  uintmax_t old_true = 0;
  if (arrMaxNEGMINMAXTrue[nDiff] < (nMax + 2)) {
    recompute_true = 1;
    old_true = arrMaxNEGMINMAXTrue[nDiff];
    arrMaxNEGMINMAXTrue[nDiff] = nMax + 2;
  }
  
  int recompute_false = 0;
  int old_false = 0;
  if (arrMaxNEGMINMAXFalse[nDiff] < ((nNumVariables - nMin) + 2)) {
    recompute_false = 1;
    old_false = arrMaxNEGMINMAXFalse[nDiff];
    arrMaxNEGMINMAXFalse[nDiff] = (nNumVariables - nMin) + 2;
  }
  
  if(recompute_true>0 || recompute_false>0) {
    intmax_t i = (intmax_t)nDiff;
    if(recompute_true > 0)
      arrNEGMINMAXWghts[i] = (double **)sbsat_recalloc(arrNEGMINMAXWghts[i], old_true, arrMaxNEGMINMAXTrue[i], sizeof(double *), 2, "arrNEGMINMAXWghts[i]");
    
    for(intmax_t j=0; j<arrMaxNEGMINMAXTrue[i]; j++) {
      arrNEGMINMAXWghts[i][j] = (double *)sbsat_recalloc(arrNEGMINMAXWghts[i][j], old_false, arrMaxNEGMINMAXFalse[i], sizeof(double), 2, "arrNEGMINMAXWghts[i][j]");
      arrNEGMINMAXWghts[i][j][0] = SM->JHEURISTIC_K_TRUE; // diff = i
      if (j==0) {
	for(intmax_t m=old_false==0?1:old_false; m<arrMaxNEGMINMAXFalse[i]; m++) {
	  arrNEGMINMAXWghts[i][0][m] = SM->JHEURISTIC_K_TRUE;
	}
      } else {
	for(intmax_t m=old_false==0?1:old_false; m<arrMaxNEGMINMAXFalse[i]; m++) {
	  if(j < i+2) {
	    if(m < i+2) {
	      arrNEGMINMAXWghts[i][j][m] = SM->JHEURISTIC_K_TRUE;
	    } else if (m == i+2) {
	      arrNEGMINMAXWghts[i][j][m] = SM->JHEURISTIC_K_INF * (double)j;
	    } else arrNEGMINMAXWghts[i][j][m] = (arrNEGMINMAXWghts[i][j-1][m] + arrNEGMINMAXWghts[i][j][m-1]) / (2.0 * SM->JHEURISTIC_K);
	  } else if (m < i+2 && j == i+2) {
	    arrNEGMINMAXWghts[i][j][m] = SM->JHEURISTIC_K_INF * (double)m;
	  } else arrNEGMINMAXWghts[i][j][m] = (arrNEGMINMAXWghts[i][j-1][m] + arrNEGMINMAXWghts[i][j][m-1]) / (2.0 * SM->JHEURISTIC_K);
	}
      }
    }
  }

  return arrNEGMINMAXWghts[nDiff][nMax+1][(nNumVariables - nMin)+1];
}

double LSGBNEGMINMAXCOUNTERStateGetVarHeurScore(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t nVbleIndex, uint8_t bPolarity) {
  NEGMINMAXCOUNTERStateEntry *pNEGMINMAXCOUNTERState = (NEGMINMAXCOUNTERStateEntry *)pTypeState;
  assert(pNEGMINMAXCOUNTERState->type == FN_NEGMINMAXCOUNTER);

  NEGMINMAXStateEntry *pNEGMINMAXState = pNEGMINMAXCOUNTERState->pNEGMINMAXState;
  
  if(pNEGMINMAXState->nNumVariables <= nVbleIndex)
    return 0.0; //Var not in Smurf
  
  if(pNEGMINMAXState->bExists[nVbleIndex] == 0)
    return 0.0; //Var not in Smurf

  uintmax_t nNumVariables = pNEGMINMAXCOUNTERState->nNumVariables;
  uintmax_t nNumTrue = pNEGMINMAXCOUNTERState->nNumTrue;
  uintmax_t nMin = nNumTrue > pNEGMINMAXState->nMin ? 0 : pNEGMINMAXState->nMin - nNumTrue;
  uintmax_t nMax = pNEGMINMAXState->nMax - nNumTrue;

  if(((bPolarity  && (nNumVariables <= nMax && nMin==2)) ||
      (!bPolarity && (nNumVariables == nMax+1 && nMin == 1))) ||
     ((!bPolarity && (nMin == 0 && nNumVariables == nMax+2)) ||
      (bPolarity  && (nNumVariables == nMax+1 && nMin == 1)))) {
    //Infer remaining variables
    return ((double)(nNumVariables-1)) * SM->JHEURISTIC_K_INF;
  } else {
    TypeStateEntry *pNextState = bPolarity?pNEGMINMAXCOUNTERState->pVarIsTrueTransition:pNEGMINMAXCOUNTERState->pVarIsFalseTransition;
    return arrGetStateHeuristicScore[pNextState->type](SM, pNextState);
  }
}

void LSGBNEGMINMAXCOUNTERStateFree(SmurfManager *SM) {
  for(uintmax_t i = 0; i < nNEGMINMAXWghtsSize; i++) {
    if(arrMaxNEGMINMAXTrue[i] != 0) {
      for(uintmax_t j = 0; j < arrMaxNEGMINMAXTrue[i]; j++)
	sbsat_free((void **)&arrNEGMINMAXWghts[i][j]);
      sbsat_free((void **)&arrNEGMINMAXWghts[i]);
    }
  }
  
  if(arrNEGMINMAXWghts!=NULL) sbsat_free((void **)&arrNEGMINMAXWghts);
  if(arrMaxNEGMINMAXTrue!=NULL) sbsat_free((void **)&arrMaxNEGMINMAXTrue);
  if(arrMaxNEGMINMAXFalse!=NULL) sbsat_free((void **)&arrMaxNEGMINMAXFalse);
  
  nNEGMINMAXWghtsSize = 0;
}
