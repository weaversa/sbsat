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

#ifndef FN_MINMAXCOUNTER_S_H
#define FN_MINMAXCOUNTER_S_H

// MINMAXCOUNTER State Initialization and Deletion
void initMINMAXCOUNTERStateType();
void FreeMINMAXCOUNTERStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState);

// MINMAXCOUNTER State Building and Garbage collection
TypeStateEntry *CreateMINMAXCOUNTERState(SmurfManager *SM, DdNode *pMINMAXCOUNTERBDD);
uintmax_t ComputeMINMAXCOUNTERStateSize(TypeStateEntry *pTypeState);
void SetMINMAXCOUNTERStateVisitedFlag(TypeStateEntry *pTypeState);
void UnsetMINMAXCOUNTERStateVisitedFlag(TypeStateEntry *pTypeState);
void CleanUpMINMAXCOUNTERState(TypeStateEntry *pTypeState);

// MINMAXCOUNTER State Traversing
uint8_t ApplyInferenceToMINMAXCOUNTER(SmurfManager *SM, uintmax_t nVbleIndex, uint8_t bPolarity, SmurfInfoStruct *pSmurfInfo);

// MINMAXCOUNTER State Printing
void PrintMINMAXCOUNTERStateEntry(SmurfManager *SM, TypeStateEntry *pTypeState);
void PrintMINMAXCOUNTERStateEntry_dot(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t *pIndex2Var);

// MINMAXCOUNTER State Heuristic Scoring and Garbage Collection
double LSGBMINMAXCOUNTERStateGetHeurScore(SmurfManager *SM, TypeStateEntry *pTypeState);
double LSGBMINMAXCOUNTERStateGetVarHeurScore(SmurfManager *SM, TypeStateEntry *pTypeState, uintmax_t nVbleIndex, uint8_t bPolarity);
void LSGBMINMAXCOUNTERStateFree();
extern double ***arrMINMAXWghts;
void CalculateMINMAXCOUNTERStateLSGBHeuristic(SmurfManager *SM, SmurfInfoStruct *pSmurfInfo);

#endif
