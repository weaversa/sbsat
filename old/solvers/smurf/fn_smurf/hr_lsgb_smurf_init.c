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

double LSGBSumNodeWeights(SmurfManager *SM, TypeStateEntry *pTypeState) {
  SmurfStateEntry *pSmurfState = (SmurfStateEntry *)pTypeState;

  if((SmurfStateEntry *)pTypeState == SM->pTrueSmurfState) return SM->JHEURISTIC_K_TRUE;

  double fHeurWghtOfSmurf = 0.0;
  uintmax_t nNumVariables = pSmurfState->nNumVariables;
  uintmax_t nNumRealVars = 0;

  SmurfStateTransition *pSmurfTransition = (SmurfStateTransition *)(pSmurfState + 1);
  for(uintmax_t nVbleIndex = 0; nVbleIndex < nNumVariables; nVbleIndex++) {
    if(pSmurfTransition->pVarIsFalseTransition != (TypeStateEntry *)pSmurfState) {
      nNumRealVars++;
      fHeurWghtOfSmurf += pSmurfTransition->fHeurWghtOfFalseTransition;
      fHeurWghtOfSmurf += pSmurfTransition->fHeurWghtOfTrueTransition;
    }
    pSmurfTransition++;
  }

  assert(nNumRealVars > 0);

  fHeurWghtOfSmurf = fHeurWghtOfSmurf / (((double)nNumRealVars) * 2.0 * SM->JHEURISTIC_K);

  return fHeurWghtOfSmurf;
}

double LSGBVarWeight(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t nVbleIndex, uint8_t bPolarity) {
  SmurfStateEntry *pSmurfState = (SmurfStateEntry *)pTypeState;

  if(pSmurfState->nNumVariables <= nVbleIndex) //Var not in Smurf or Smurf is the True state
    return SM->JHEURISTIC_K_TRUE;

  SmurfStateTransition *pSmurfTransition = ((SmurfStateTransition *)(pSmurfState + 1)) + nVbleIndex;
  if(bPolarity == 0) {
    //Return the False transition value
    return pSmurfTransition->fHeurWghtOfFalseTransition;
  } else {
    //Return the True transition value
    return pSmurfTransition->fHeurWghtOfTrueTransition;
  }

}

void LSGBSmurfFree(SmurfManager *SM) {
  //Do nothing
}
