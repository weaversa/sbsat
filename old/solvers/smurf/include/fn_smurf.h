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

#ifndef FN_SMURF_S_H
#define FN_SMURF_S_H

// Smurf State Initialization and Deletion
void initSmurfStateType();
void FreeSmurfStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState);

// Smurf State Building and Garbage collection
TypeStateEntry *CreateSmurfState(SmurfManager *SM, DdNode *pCurrentBDD);
uintmax_t ComputeSmurfStateSize(TypeStateEntry *pTypeState);
void SetSmurfStateVisitedFlag(TypeStateEntry *pTypeState);
void UnsetSmurfStateVisitedFlag(TypeStateEntry *pTypeState);
void CleanUpSmurfState(TypeStateEntry *pTypeState);

// Smurf State Traversing
uint8_t ApplyInferenceToSmurf(SmurfManager *SM, uintmax_t nVbleIndex, uint8_t bPolarity, SmurfInfoStruct *pSmurfInfo);

// Smurf State Printing
extern uint8_t print_smurf_transitions;
void PrintSmurfStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState);
void PrintSmurfStateEntry_dot(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t *pIndex2Var);
void PrintSmurfStateEntry_inferences_compressed_dot(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t *pIndex2Var);

// Smurf State Heuristic Scoring and Garbage Collection
double LSGBSumNodeWeights(SmurfManager *SM, TypeStateEntry *pTypeState);
double LSGBVarWeight(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t nVbleIndex, uint8_t bPolarity);
void LSGBSmurfFree(SmurfManager *SM);
void CalculateSmurfLSGBHeuristic(SmurfManager *SM, SmurfInfoStruct *pSmurfInfo);

#endif
