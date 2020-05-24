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

#ifndef FN_XOR_S_H
#define FN_XOR_S_H

// XOR State Initialization and Deletion
void initXORStateType();
void FreeXORStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState);

// XOR State Building and Garbage collection
TypeStateEntry *CreateXORState(SmurfManager *SM, DdNode *pXORBDD);
uintmax_t ComputeXORStateSize(TypeStateEntry *pTypeState);
void SetXORStateVisitedFlag(TypeStateEntry *pTypeState);
void UnsetXORStateVisitedFlag(TypeStateEntry *pTypeState);
void CleanUpXORState(TypeStateEntry *pTypeState);

// XOR State Traversing
uint8_t ApplyInferenceToXOR(SmurfManager *SM, uintmax_t nVbleIndex, uint8_t bPolarity, SmurfInfoStruct *pSmurfInfo);
uint8_t ApplyInferenceToXOR_gelim(SmurfManager *SM, uintmax_t nVbleIndex, uint8_t bPolarity, SmurfInfoStruct *pSmurfInfo);

// XOR State Printing
void PrintXORStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState);
void PrintXORStateEntry_dot(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t *pIndex2Var);

// XOR State Heuristic Scoring and Garbage Collection
double LSGBXORStateGetHeurScore(SmurfManager *SM, TypeStateEntry *pTypeState);
double LSGBXORStateGetVarHeurScore(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t nVbleIndex, uint8_t bPolarity);
void LSGBXORStateFree();
void CalculateXORStateLSGBHeuristic(SmurfManager *SM, SmurfInfoStruct *pSmurfInfo);

#endif
