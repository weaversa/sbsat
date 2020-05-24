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

double *arrLSGBXORCOUNTERWeight = NULL;
uintmax_t nXORCOUNTERNumVariables = 0;

//---------------------------------------------------------------

double LSGBXORCOUNTERStateGetHeurScore(SmurfManager *SM, TypeStateEntry *pTypeState) {
  XORCOUNTERStateEntry *pXORCOUNTERState = (XORCOUNTERStateEntry *)pTypeState;
  uintmax_t nNumVariables = pXORCOUNTERState->nNumVariables;
  
  if(nNumVariables > nXORCOUNTERNumVariables) {
    if(nXORCOUNTERNumVariables == 0) { //Array needs to be initalized
      arrLSGBXORCOUNTERWeight = (double *)sbsat_calloc(nNumVariables+1, sizeof(double), 9, "arrLSGBXORCOUNTERWeight");
      arrLSGBXORCOUNTERWeight[0] = SM->JHEURISTIC_K_TRUE;
      arrLSGBXORCOUNTERWeight[1] = SM->JHEURISTIC_K_INF + SM->JHEURISTIC_K_TRUE;
      nXORCOUNTERNumVariables = 1;
    } else
      arrLSGBXORCOUNTERWeight = (double *)sbsat_realloc(arrLSGBXORCOUNTERWeight, nXORCOUNTERNumVariables, nNumVariables+1, sizeof(double), 9, "arrLSGBXORCOUNTERWeight");
    
    for (uintmax_t i = nXORCOUNTERNumVariables+1; i <= nNumVariables; i++) {
      arrLSGBXORCOUNTERWeight[i] = arrLSGBXORCOUNTERWeight[i-1] / SM->JHEURISTIC_K;
    }
    nXORCOUNTERNumVariables = nNumVariables;
  }
  return arrLSGBXORCOUNTERWeight[nNumVariables];
}

double LSGBXORCOUNTERStateGetVarHeurScore(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t nVbleIndex, uint8_t bPolarity) {
  XORCOUNTERStateEntry *pXORCOUNTERState = (XORCOUNTERStateEntry *)pTypeState;
  assert(pXORCOUNTERState->type == FN_XORCOUNTER);
  
  XORStateEntry *pXORState = pXORCOUNTERState->pXORState;
  assert(pXORState->type == FN_XOR);
  
  if(pXORState->nNumVariables <= nVbleIndex) //Var not in Smurf
    return 0.0;
  
  if(pXORState->bExists[nVbleIndex] == 0)
    return 0.0; //Var not in Smurf
  
  if(bPolarity) {
    return arrGetStateHeuristicScore[pXORCOUNTERState->pVarIsTrueTransition->type](SM, pXORCOUNTERState->pVarIsTrueTransition);
  } else {
    return arrGetStateHeuristicScore[pXORCOUNTERState->pVarIsFalseTransition->type](SM, pXORCOUNTERState->pVarIsFalseTransition);
  }
}

void LSGBXORCOUNTERStateFree(SmurfManager *SM) {
  if(arrLSGBXORCOUNTERWeight!=NULL) sbsat_free((void **)&arrLSGBXORCOUNTERWeight);
  nXORCOUNTERNumVariables = 0;
}
