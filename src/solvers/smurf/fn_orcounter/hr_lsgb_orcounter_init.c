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

double *arrLSGBORCOUNTERWeight = NULL;
uintmax_t nORCOUNTERNumVariables = 0;

//---------------------------------------------------------------

void initLSGBORCOUNTERState(SmurfManager *SM, uintmax_t nNumVariables) {
  if(nNumVariables > nORCOUNTERNumVariables) {
    if(nORCOUNTERNumVariables == 0) { //Array needs to be initalized
      arrLSGBORCOUNTERWeight = (double *)sbsat_calloc(nNumVariables+1, sizeof(double), 9, "arrLSGBORCOUNTERWeight");
      arrLSGBORCOUNTERWeight[0] = SM->JHEURISTIC_K_TRUE;
      arrLSGBORCOUNTERWeight[1] = SM->JHEURISTIC_K_INF + SM->JHEURISTIC_K_TRUE;
      nORCOUNTERNumVariables = 1;
    } else
      arrLSGBORCOUNTERWeight = (double *)sbsat_realloc(arrLSGBORCOUNTERWeight, nORCOUNTERNumVariables, nNumVariables+1, sizeof(double), 9, "arrLSGBORCOUNTERWeight");
    
    for (uintmax_t i = nORCOUNTERNumVariables+1; i <= nNumVariables; i++) {
      arrLSGBORCOUNTERWeight[i] = (arrLSGBORCOUNTERWeight[i-1] + SM->JHEURISTIC_K_TRUE) / (2.0 * SM->JHEURISTIC_K);
    }
    nORCOUNTERNumVariables = nNumVariables;
  }
}

double LSGBORCOUNTERStateGetHeurScore(SmurfManager *SM, TypeStateEntry *pTypeState) {
  ORCOUNTERStateEntry *pORCOUNTERState = (ORCOUNTERStateEntry *)pTypeState;
  uintmax_t nNumVariables = pORCOUNTERState->nNumVariables;
  
  initLSGBORCOUNTERState(SM, nNumVariables);

  return arrLSGBORCOUNTERWeight[nNumVariables];
}

double LSGBORCOUNTERStateGetVarHeurScore(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t nVbleIndex, uint8_t bPolarity) {
  ORCOUNTERStateEntry *pORCOUNTERState = (ORCOUNTERStateEntry *)pTypeState;
  assert(pORCOUNTERState->type == FN_ORCOUNTER);
  
  ORStateEntry *pORState = pORCOUNTERState->pORState;
  assert(pORState->type == FN_OR);
  
  if(pORState->nNumVariables <= nVbleIndex) //Var not in Smurf
    return 0.0;
  
  uint8_t _bPolarity = pORState->bPolarity[nVbleIndex];
  if(_bPolarity == 2)
    return 0.0; //Var not in Smurf
  
  if(bPolarity == _bPolarity) {
    //Literal is being assigned positively
    //Smurf will become satisfied w/ no inference
    return SM->JHEURISTIC_K_TRUE;
  } else {
    //Literal is being assigned negatively
    return arrGetStateHeuristicScore[pORCOUNTERState->pTransition->type](SM, pORCOUNTERState->pTransition);
  }
}

void LSGBORCOUNTERStateFree(SmurfManager *SM) {
  if(arrLSGBORCOUNTERWeight!=NULL) sbsat_free((void **)&arrLSGBORCOUNTERWeight);
  nORCOUNTERNumVariables = 0;
}
