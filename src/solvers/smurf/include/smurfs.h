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

#ifndef SMURFS_H
#define SMURFS_H

/* List of possible smurf function types */
enum {
  FN_FREE_STATE, //Must be 0
  FN_TYPE_STATE,
  FN_DROPPEDVAR, //Start of non-functional matching states
  FN_OR,
  FN_XOR,
  FN_ANDEQUALS,
  FN_MINMAX,
  FN_NEGMINMAX,
  BEGIN_FUNCTIONAL_SMURFS,
  FN_INFERENCE,  //First functional state
  FN_XORFACTOR,
  FN_ORCOUNTER,
  FN_XORCOUNTER,
  FN_ANDEQUALSCOUNTER,
  FN_MINMAXCOUNTER,
  FN_NEGMINMAXCOUNTER,
  FN_SMURF,      //The SMURF should be the last functional state
  END_FUNCTIONAL_SMURFS,
  NUM_SMURF_TYPES,
};

/* State types */
enum {
  SM_NODE_TYPE,
  SM_TRANSITION_TYPE
};

#ifdef SBSAT_64BIT
#define TSE_SO_BITS 58
#define MAX_BITSm1 63
#else
#define TSE_SO_BITS 26
#define MAX_BITSm1 31
#endif

#define TSE_data()                  \
  uint8_t type:7;                   \
  uint8_t visited:1;                \

//Structures and functions for the simpleSolver

typedef struct TypeStateEntry {
  TSE_data();
} TypeStateEntry;

typedef struct SmurfStateEntry {
  TSE_data(); //FN_SMURF

  uintmax_t nNumVariables;
  DdNode *pSmurfBDD; //Used when building smurfs on the fly.
} SmurfStateEntry;

typedef struct SmurfStateTransition {
  double fHeurWghtOfFalseTransition;
  double fHeurWghtOfTrueTransition;

  TypeStateEntry *pVarIsFalseTransition;
  TypeStateEntry *pVarIsTrueTransition;
} SmurfStateTransition;

typedef struct InferenceStateEntry {
  TSE_data(); //FN_INFERENCE

  uintmax_t nTransitionVar:MAX_BITSm1;
  uint8_t bTransitionPol:1;

  TypeStateEntry *pTransition;
  DdNode *pInferenceBDD; //Used when building smurfs on the fly.
} InferenceStateEntry;

typedef struct DroppedVarStateEntry {
  TSE_data(); //FN_DROPPEDVAR
  
  uintmax_t nVar;
  TypeStateEntry *pTransition;
} DroppedVarStateEntry;

typedef struct ORStateEntry {
  TSE_data(); //FN_OR
  
  uintmax_t nNumVariables;
  uintmax_t nNumBDDVariables;
  uint8_t *bPolarity;
  DdNode *pORBDD;
} ORStateEntry;

typedef struct ORCOUNTERStateEntry {
  TSE_data(); //FN_ORCOUNTER
  
  uintmax_t nNumVariables;
  TypeStateEntry *pTransition; //Either FN_OR or FN_ORCOUNTER
  ORStateEntry *pORState;
} ORCOUNTERStateEntry;

typedef struct XORStateEntry {
  TSE_data(); //FN_XOR
  
  uintmax_t nNumVariables;
  uint8_t bParity;
  uint8_t *bExists;
  DdNode *pXORBDD;
} XORStateEntry;

typedef struct XORCOUNTERStateEntry {
  TSE_data(); //FN_XORCOUNTER

  uintmax_t nNumVariables;
  TypeStateEntry *pVarIsTrueTransition;
  TypeStateEntry *pVarIsFalseTransition;
  XORStateEntry *pXORState;
} XORCOUNTERStateEntry;

typedef struct ANDEQUALSStateEntry {
  TSE_data(); //FN_ANDEQUALS
  
  uintmax_t nLHS; //Left Hand Side = AND(...)
  uint8_t bAND_or_OR;
  ORStateEntry *pORState;
  DdNode *pANDEQUALSBDD;
} ANDEQUALSStateEntry;

typedef struct ANDEQUALSCOUNTERStateEntry {
  TSE_data(); //FN_ANDEQUALSCOUNTER
  
  TypeStateEntry *pTransition;
  ORCOUNTERStateEntry *pORCOUNTERState;
  ANDEQUALSStateEntry *pANDEQUALSState;
} ANDEQUALSCOUNTERStateEntry;

typedef struct MINMAXStateEntry {
  TSE_data(); //FN_MINMAX

  uintmax_t nNumVariables;
  uintmax_t nNumBDDVariables;
  uintmax_t nMin;
  uintmax_t nMax;
  uint8_t *bExists;
  DdNode *pMINMAXBDD;
} MINMAXStateEntry;

typedef struct MINMAXCOUNTERStateEntry {
  TSE_data(); //FN_MINMAXCOUNTER

  uintmax_t nNumVariables;
  uintmax_t nNumTrue;
  TypeStateEntry *pVarIsTrueTransition;
  TypeStateEntry *pVarIsFalseTransition;

  MINMAXStateEntry *pMINMAXState;
} MINMAXCOUNTERStateEntry;

typedef struct NEGMINMAXStateEntry {
  TSE_data(); //FN_NEGMINMAX

  uintmax_t nNumVariables;
  uintmax_t nNumBDDVariables;
  uintmax_t nMin;
  uintmax_t nMax;
  uint8_t *bExists;
  DdNode *pNEGMINMAXBDD;
} NEGMINMAXStateEntry;

typedef struct NEGMINMAXCOUNTERStateEntry {
  TSE_data(); //FN_NEGMINMAXCOUNTER

  uintmax_t nNumVariables;
  uintmax_t nNumTrue;
  TypeStateEntry *pVarIsTrueTransition;
  TypeStateEntry *pVarIsFalseTransition;

  NEGMINMAXStateEntry *pNEGMINMAXState;
} NEGMINMAXCOUNTERStateEntry;

typedef struct XORFACTORStateEntry {
  TSE_data(); //FN_XORFACTOR

  TypeStateEntry *pXORFactor;
  TypeStateEntry *pTransition;
  DdNode *pXORFACTORBDD;
} XORFACTORStateEntry;


typedef struct SmurfInfoStruct SmurfInfoStruct;
struct SmurfInfoStruct {
  uintmax_t nNumVariables;
  TypeStateEntry *pCurrentState;
  clause *pLemma;
  uintmax_t *pIndex2Var;
  uintmax_t nSmurfIndex;
};

create_list_headers(SmurfInfoStruct);
create_pundoer_headers(TypeStateEntry);
create_undoer_headers(literal);

#endif
