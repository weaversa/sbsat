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

double *arrLSGBANDEQUALSCOUNTERWeight = NULL;
uintmax_t nANDEQUALSCOUNTERNumVariables = 0;

//---------------------------------------------------------------

double LSGBANDEQUALSCOUNTERStateGetHeurScore(SmurfManager *SM, TypeStateEntry *pTypeState) {
  ANDEQUALSCOUNTERStateEntry *pANDEQUALSCOUNTERState = (ANDEQUALSCOUNTERStateEntry *)pTypeState;
  uintmax_t nNumVariables;
  if(pANDEQUALSCOUNTERState->pORCOUNTERState->type == FN_ORCOUNTER) {
    nNumVariables = 1 + pANDEQUALSCOUNTERState->pORCOUNTERState->nNumVariables;
  } else {
    assert(pANDEQUALSCOUNTERState->pORCOUNTERState->type == FN_OR);
    nNumVariables = 3;
  }  

  if(nNumVariables > nANDEQUALSCOUNTERNumVariables) {
    if(nANDEQUALSCOUNTERNumVariables == 0) { //Array needs to be initalized
      arrLSGBANDEQUALSCOUNTERWeight = (double *)sbsat_malloc(nNumVariables+1, sizeof(double), 9, "arrLSGBANDEQUALSCOUNTERWeight");
      arrLSGBANDEQUALSCOUNTERWeight[0] = SM->JHEURISTIC_K_TRUE;
      arrLSGBANDEQUALSCOUNTERWeight[1] = SM->JHEURISTIC_K_INF + SM->JHEURISTIC_K_TRUE;
      nANDEQUALSCOUNTERNumVariables = 1;
    } else {
      arrLSGBANDEQUALSCOUNTERWeight = (double *)sbsat_realloc(arrLSGBANDEQUALSCOUNTERWeight, nANDEQUALSCOUNTERNumVariables, nNumVariables+1, sizeof(double), 9, "arrLSGBANDEQUALSCOUNTERWeight");
    }

    //Force update the ORCOUNTER heuristic arrays
    initLSGBORCOUNTERState(SM, nNumVariables-1);
        
    for (uintmax_t i = nANDEQUALSCOUNTERNumVariables+1; i <= nNumVariables; i++) {
      arrLSGBANDEQUALSCOUNTERWeight[i] = ((((double)(i-1)) * arrLSGBANDEQUALSCOUNTERWeight[i-1]) //rhs->true
					  + ((((double)(i-1)) * SM->JHEURISTIC_K_INF) + SM->JHEURISTIC_K_TRUE) //lhs->true
					  + (((double)(i-1)) * (SM->JHEURISTIC_K_INF + SM->JHEURISTIC_K_TRUE)) //rhs->false
					  + arrLSGBORCOUNTERWeight[i-1]) / (2.0 * ((double)i) * SM->JHEURISTIC_K); //lhs->false
    }
    nANDEQUALSCOUNTERNumVariables = nNumVariables;
  }

  return arrLSGBANDEQUALSCOUNTERWeight[nNumVariables];
}

double LSGBANDEQUALSCOUNTERStateGetVarHeurScore(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t nVbleIndex, uint8_t bPolarity) {
  ANDEQUALSCOUNTERStateEntry *pANDEQUALSCOUNTERState = (ANDEQUALSCOUNTERStateEntry *)pTypeState;
  assert(pANDEQUALSCOUNTERState->type == FN_ANDEQUALSCOUNTER);

  if(nVbleIndex == pANDEQUALSCOUNTERState->pANDEQUALSState->nLHS) {
    //Setting the head
    if(pANDEQUALSCOUNTERState->pANDEQUALSState->bAND_or_OR == bPolarity) {
      uintmax_t nNumVariables;
      if(pANDEQUALSCOUNTERState->pORCOUNTERState->type == FN_ORCOUNTER) {
	nNumVariables = pANDEQUALSCOUNTERState->pORCOUNTERState->nNumVariables;
      } else {
	assert(pANDEQUALSCOUNTERState->pORCOUNTERState->type == FN_OR);
	nNumVariables = 2;
      }
      return (((double)nNumVariables) * SM->JHEURISTIC_K_INF) + SM->JHEURISTIC_K_TRUE;
    } else {
      //Transition to ORCOUNTERState
      return arrGetStateHeuristicScore[pANDEQUALSCOUNTERState->pORCOUNTERState->type](SM, (TypeStateEntry *)pANDEQUALSCOUNTERState->pORCOUNTERState);
    }
  } else {
    uintmax_t nNumElts = pANDEQUALSCOUNTERState->pANDEQUALSState->pORState->nNumVariables;
    if(nNumElts <= nVbleIndex)
      return 0.0; //Var not in Smurf
    
    uint8_t _bPolarity = pANDEQUALSCOUNTERState->pANDEQUALSState->pORState->bPolarity[nVbleIndex];
    if(_bPolarity == 2)
      return 0.0; //Var not in Smurf
    
    if(bPolarity != _bPolarity) {
      //Normal Transition
      return arrGetStateHeuristicScore[pANDEQUALSCOUNTERState->pTransition->type](SM, (TypeStateEntry *)pANDEQUALSCOUNTERState->pTransition);
    } else {
      //Infer head
      return SM->JHEURISTIC_K_INF + SM->JHEURISTIC_K_TRUE;
    }
  }
}

void LSGBANDEQUALSCOUNTERStateFree(SmurfManager *SM) {
  if(arrLSGBANDEQUALSCOUNTERWeight!=NULL) sbsat_free((void **)&arrLSGBANDEQUALSCOUNTERWeight);
  nANDEQUALSCOUNTERNumVariables = 0;
}
