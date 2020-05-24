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

#ifndef GELIM_SOLVER_H
#define GELIM_SOLVER_H

#include "sbsat.h"

typedef struct XORGElimTableStruct {
  uintmax_t nRows;
  uintmax_t nRows_max;
  uintmax_t nVariables;
  uintmax_t_list tmp_row;
  uintmax_t *first_bit;
  uintmax_t_list *rows;
} XORGElimTableStruct;

typedef uint8_t (*EnqueueInference_hook_gelim)(uintptr_t *EM, uintptr_t *UNUSED, uintmax_t nVariable, uint8_t bPolarity);

void initXORGElimTable(uintmax_t nVars, EnqueueInference_hook_gelim _EnqueueInference_hook, uintptr_t *EM);

void allocXORGElimTable(XORGElimTableStruct *x, uintmax_t nRows, uintmax_t nVariables);

void pushXORGElimTable(XORGElimTableStruct *curr, XORGElimTableStruct *dest);

uint8_t addRowXORGElimTable (XORGElimTableStruct *x, uintmax_t nVars, uintmax_t *pnVarlist, uint8_t bParity);

uint8_t ApplyInferenceToXORGElimTable (XORGElimTableStruct *x, uintmax_t nVar, uint8_t bValue);

void deleteXORGElimTable (XORGElimTableStruct *x);

void preloadXORGelimTable(XORGElimTableStruct *x);

extern char *gelim_preload_file;

#endif
