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

/**                              gelim_solver.c
 *                                                                            
 * Maintain sets of equivalent and opposite valued variables using inverted
 * trees and path compression when joining.  Equivalences can only be set,
 * not removed.  Assumes variables are numbered (that is, of type int).
 * 
 * class Equiv API:
 *    Equiv (int inp, int out, int True, int False): Constructor, inp is the
 *    maximum numbered input variable, True is the number of the variable
 *    representing value true, False is the number of the variable representing
 *    value false.
 *
**/

#include "sbsat.h"

// Figure 1.
// 0-1 matrix representing a system of linear functions.  Vectors are packed
// into 32 bit words.  The example below shows a system of 46 variables.  It
// uses two words per function.  Columns are indexed on variables, lowest on
// the left, and rows on functions.  The first row in the example below
// represents the clause - operator "+" is the xor operator
//            (x[2] + x[4] + x[12] + x[19] + x[30] + x[45])
// because the "last column" (rightmost bit of rightmost word - most
// significant bit) is 0.  The second row in the example below represents
//            (x[3] + x[4] + x[5] + x[10] + x[21] + x[44] + 1)
// where the extra "1" is due to a "1" in the last column of the right word.
// The stored form of the matrix has been subjected to gaussian elimination
// and consists of a diagonal component plus an arbitrary component.
//
//            +--first_bit[6] = -1         +--first_bit[36] = 9
//            |                              |
//      INT32 00101000000010000001000000000010 00000000000001000000000000000000 ...
//      INT32 00011100001000000000010000000000 00000000000010000000000000000000 ...
//      INT32 00000000000000000111000000000000 00000000000100000000000000000000 ...
//      INT32 00101000000010000001000000000010 10000000001000000000000000000000 ...
//      INT32 00001000000010000000000000000010 01000000010000000000000000000000 ...
//      INT32 00100000000000000001000000000000 00000000100000000000000000000000 ...
//      INT32 00001000000000000000000000000000 00000001000000000000000000000000 ...
//      INT32 00000000010100000001000000100010 00000010000000000000000000000000 ...
//      INT32 00000000010000000001000000000000 10000100000000000000000000000000 ...
//      INT32 00000100000001000000001000000000 00001000000000000000000000000000 ...
//      INT32 11000000010000000000000000000000 00010000000000000000000000000000 ...
//      INT32 00000000010000000000001000000000 00100000000000000000000000000000 ...
//      INT32 ||<-------anything allowed------->||<-------->||<---unused----->| ...
//      INT32 |                                    diagonal                    
//      INT32 +--equality bit                      component                   
//
// The following additional variables are used:
//   first_bit - If x is a column (variable) in the diagonal component then
//           first_bit[x] is the number of the row which has a "1" in that
//           column.  Otherwise first_bit[x] is -1.
//   mask  - 0-1 masking pattern for columns of the matrix.  If mask[x] is 1
//           then variable x has not been assigned a value and is not a column
//           in the diagonal component.
//   order - list of pointers to vectors used for sorting by means of qsort
//   frame - block of memory containing the number of input variables, number
//           of output variables, the number of words in each 0-1 row (also   
//           called vector), the number of rows (index), the identities of    
//           T (true) and F (false), and "index" number of "row records".     
//           Each "row record" contains a consecutive "vec_size" number of    
//           words containing the 0-1 patterns, the "vec_size" value, the     
//           type of function (even/odd parity), in what column the bit of a  
//           row occupies in the diagonal component, and equivalence class    
//           variables.  Access to these values in the frame variable is      
//           accomplished as follows:                                         
//                                                                            
//           Matrix variables:
//               first_bit: (short int *)&frame[first_bit_ref]
//                   xlist: (short int *)&frame[xlist_ref]
//                   ylist: (short int *)&frame[ylist_ref]
//                  matrix: (char *)&frame[vecs_v_ref]
//                 ith row: (VecType *)&frame[vecs_v_start+i*vecs_rec_bytes]
//      ith row column ptr: *((short int *)&frame[offset+vecs_colm_start])
//              where offset = vecs_v_start + i*vecs_rec_bytes
//        ith row var list: (short int *)&frame[offset+vecs_vlst_start]
//        ith row vec_size: (short int *)&frame[offset+vecs_vsze_start]
//          highest order 1
//       e. first_bit[xord's highest order 1] is set to new row
//       f. mask takes a 0 in same position as xord's highest order 1
//       g. column ptr of the new row set to xord's highest order 1

/*
struct XORGElimTableStruct {
	char *frame;
	int *first_bit; // first_bit[i]: pointer to vector on diagonal with first 1 bit at column i
   unsigned long *mask; // 0 bits are columns of the diagonalized submatrix or assigned variables
   int num_vectors;
 };
*/

#define BITS_PER_BYTE 8

uint8_t (*GEEnqueueInference_hook)(uintptr_t *EM, uintptr_t *UNUSED, uintmax_t nVariable, uint8_t bPolarity);
uintptr_t *GEEM;

void printrow(uintmax_t_list *row);
void printXORGElimTable(XORGElimTableStruct *x);

void printLinear(XORGElimTableStruct *x);
void printLinearN (XORGElimTableStruct *x);
void PrintXORGElimVector(uintmax_t *pVector);

static uintmax_t bits_in_16bits [0x1u << 16];

char *gelim_preload_file = NULL;

// Iterated bitcount iterates over each bit. The while condition sometimes helps
//  terminate the loop earlier
static inline uintmax_t iterated_bitcount(uintmax_t n) {
  uintmax_t count=0;
  while(n) {
    count += n & 0x1u;
    n >>= 1 ;
  }
  return count;
}

static inline void compute_bits_in_16bits () {
  for(uintmax_t i = 0; i < (uintmax_t)(0x1u<<16); i++)
    bits_in_16bits[i] = iterated_bitcount(i) ;
}

#ifdef SBSAT_64BIT
inline static uintmax_t precomputed16_bitcount(uintmax_t n){
  // works only for 64-bits
  // fprintf(stderr, " n = %llx\n", n);
  return bits_in_16bits [n & 0xffffu]
    +  bits_in_16bits [(n >> 16) & 0xffffu] 
    +  bits_in_16bits [(n >> 32) & 0xffffu] 
    +  bits_in_16bits [(n >> 48) & 0xffffu];
}
#else
static inline int precomputed16_bitcount (uintmax_t n){
  // works only for 32-bits
  // fprintf(stderr, " n = %lx\n", n);
  return bits_in_16bits [n & 0xffffu]
    +  bits_in_16bits [(n >> 16) & 0xffffu];
}
#endif

void initXORGElimTable(uintmax_t nVars, uint8_t (*_EnqueueInference_hook)(uintptr_t *EM, uintptr_t *UNUSED, uintmax_t nVariable, uint8_t bPolarity), uintptr_t *EM){
  compute_bits_in_16bits();
  
  GEEnqueueInference_hook = _EnqueueInference_hook;
  GEEM = EM; //External Manager
}

void allocXORGElimTable(XORGElimTableStruct *x, uintmax_t nRows, uintmax_t nVariables){
  if(x->rows == NULL) {
    x->nRows = 0;
    x->nRows_max = nRows;
    x->nVariables = nVariables+1;
    uintmax_t_list_alloc(&x->tmp_row, 10000, 10000);
    x->first_bit = (uintmax_t *)sbsat_calloc(x->nVariables+1, sizeof(uintmax_t), 9, "GE->first_bit");
    for(uintmax_t i = 0; i <= x->nVariables; i++) x->first_bit[i] = ~(uintmax_t)0;
    x->rows = (uintmax_t_list *)sbsat_calloc(x->nRows_max, sizeof(uintmax_t_list), 9, "x->rows");
    for(uintmax_t i = 0; i < nRows; i++) {
      uintmax_t_list_alloc(&x->rows[i], 10, 10);
    }
  } else if(x->nRows_max < nRows) {
    if(x->tmp_row.pList == NULL)
      uintmax_t_list_alloc(&x->tmp_row, 10000, 10000);
    x->rows = (uintmax_t_list *)sbsat_recalloc(x->rows, x->nRows_max, nRows, sizeof(uintmax_t_list), 9, "GE->rows realloc");
    for(uintmax_t i = x->nRows_max; i < nRows; i++) {
      uintmax_t_list_alloc(&x->rows[i], 10, 10);
    }
    x->nRows_max = nRows;
  }
}

void preloadXORGelimTable(XORGElimTableStruct *x) {
  assert(x->nRows == 0);
  if(gelim_preload_file == NULL) {
    d9_printf1("No XOR table to preload...\n");
    return;
  }
  FILE *xorfile;
  if((xorfile = fopen(gelim_preload_file, "r")) == NULL) {
    d9_printf1("XOR table preload file failed to open...\n");
    return;
  }
  uint32_t nRows, nVariables, nLength, nVar;
  if(fread(&nRows,sizeof(uint32_t),1,xorfile) == EOF) {
    d9_printf1("XOR table preload file failed while reading nRows...\n");
    return;
  }
  assert(nRows > 0);
  if(fread(&nVariables,sizeof(uint32_t),1,xorfile) == EOF) {
    d9_printf1("XOR table preload file failed while reading nVariables...\n");
    return;
  }
  assert(nVariables > 0);
  allocXORGElimTable(x, nRows, nVariables);

  uintmax_t i, j;
  for(i = 0; i < nRows; i++) {
    if(fread(&nLength,sizeof(uint32_t),1,xorfile) == EOF) {
      d9_printf2("XOR table preload file failed while reading length of row %ju...\n", i);
      return;
    }
    x->rows[i].nLength = 0;
    for(j = 0; j < nLength; j++) {
      if(fread(&nVar,sizeof(uint32_t),1,xorfile) == EOF) {
	d9_printf3("XOR table preload file failed while reading variable %ju of row %ju...\n", j, i);
	return;
      }
      uintmax_t_list_push(&x->rows[i], nVar);
      if(j == nLength-1) assert(nVar < 2);
    }
    assert(x->rows[i].pList[0] <= nVariables);
    assert(x->first_bit[x->rows[i].pList[0]] == ~(uintmax_t)0);
    x->first_bit[x->rows[i].pList[0]] = i;
    qsort(x->rows[i].pList, nLength-1, sizeof(uintmax_t), compfunc);
  }
  //assert(feof(xorfile));
  fclose(xorfile);
  x->nRows = nRows;
}

void deleteXORGElimTable(XORGElimTableStruct *x) { 
  uintmax_t i;
  if(gelim_preload_file != NULL) {
    sbsat_free((void **)&gelim_preload_file);
    gelim_preload_file = NULL;
  }
  if(x!=NULL) {
    uintmax_t_list_free(&x->tmp_row);
    sbsat_free((void **)&x->first_bit);
    for(i = 0; i < x->nRows; i++)
      uintmax_t_list_free(&x->rows[i]);
    sbsat_free((void **)&x->rows);
  }
}
 
// Push a copy of this frame into the frame of another level
void pushXORGElimTable(XORGElimTableStruct *curr, XORGElimTableStruct *dest) {
  allocXORGElimTable(dest, curr->nRows_max, curr->nVariables);
  
  memcpy(dest->first_bit, curr->first_bit, (curr->nVariables+1) * sizeof(uintmax_t));
  for(uintmax_t i = 0; i < curr->nRows; i++) {
    uintmax_t_list_copy(&dest->rows[i], &curr->rows[i]);
  }
  dest->nRows = curr->nRows;
}

void XORRows(uintmax_t_list *res, uintmax_t_list *x,  uintmax_t_list *y) {
  uintmax_t xi = 0, yi = 0;
  res->nLength = 0;
  while(xi < x->nLength-1 && yi < y->nLength-1) {
    if(x->pList[xi] < y->pList[yi]) {
      uintmax_t_list_push(res, x->pList[xi++]);
    } else if(x->pList[xi] > y->pList[yi]) {
      uintmax_t_list_push(res, y->pList[yi++]);
    } else {
      xi++; yi++;
    }
  }

  while(xi < x->nLength-1) {
    uintmax_t_list_push(res, x->pList[xi++]);
  }

  while(yi < y->nLength-1) {
    uintmax_t_list_push(res, y->pList[yi++]);
  }

  uintmax_t_list_push(res, x->pList[xi] ^ y->pList[yi]);
  //d2_printf2("|%ju|", res->nLength);
}

uint8_t rediagonalizeXORGElimTable(XORGElimTableStruct *x, uintmax_t_list *row, uintmax_t row_index) {
  assert(row != NULL);
  uintmax_t row_size = row->nLength;
  assert(row_size > 0);
  if(row_size == 1) {
    if(row->pList[0] == 1) return 0;
    return 2;
  }

  // Find column owner for this row
  //uintmax_t first_var = row->pList[0];
  uintmax_t first_var = row->pList[row_size-2];
  //uintmax_t first_var = row->pList[0];

  //d2_printf3("{%ju, %ju}", first_var, min);

  // Open up a new diagonal column
  x->first_bit[first_var] = row_index;
  
  //Look for second 1. If doesn't exist --> row gives inference.
  if(row_size == 2) {
    uint8_t ret = GEEnqueueInference_hook(GEEM, NULL, first_var, row->pList[1]);
    if(ret == UNSAT) {
      return 0;
    }
  }

  // Cancel all 1's in the new column. Currently looks at *all* vectors!
  for(uintmax_t i=0 ; i < x->nRows; i++) {
    assert(x->rows[i].pList != NULL);

    if(i == row_index) continue; //skip the new row

    //Does x->rows[i] have first_var?
    uint8_t has_first_var = 0;
    intmax_t min = 0;
    intmax_t max = x->rows[i].nLength-1;
    while (max >= min) {
      intmax_t mid = min + ((max - min) / 2);
      if(x->rows[i].pList[mid] == first_var) {
	has_first_var = 1;
	break;
      } else if (x->rows[i].pList[mid] < first_var) {
	min = mid + 1;
      } else {
	max = mid - 1;
      }
    }

    /*    for(uintmax_t j=0; j < x->rows[i].nLength-1; j++) {
      if(x->rows[i].pList[j] == first_var) {
	has_first_var = 1;
	break;
      }
      }*/
    if(!has_first_var) continue;

    D_7(
    d7_printf1("XORing:\n");
    printrow(row); printrow(&x->rows[i]);
    d7_printf1("\n");
    );

    XORRows(&x->tmp_row, &x->rows[i], row);
    uintmax_t_list_copy(&x->rows[i], &x->tmp_row);

    D_7(d2_printf1("c    ");printrow(&x->rows[i]); d2_printf3(" row %ju, var %ju\n", i, first_var););

    if((x->rows[i].nLength == 1) && (x->rows[i].pList[0] == 0)) {
      return 0; //Inconsistent
    }
    
    if(x->rows[i].nLength == 2) {
      //Inference
      uint8_t ret = GEEnqueueInference_hook(GEEM, NULL, x->rows[i].pList[0], x->rows[i].pList[1]);
      if(ret == UNSAT) {
	return 0; //Inconsistent
      }
    }
  }

  return 1;
}

// Add row to the matrix
uint8_t addRowXORGElimTable (XORGElimTableStruct *x, uintmax_t nVars, uintmax_t *pnVarList, uint8_t bParity) {
 
  d7_printf3("    Adding a row (%ju) to the addRowXORGElimTable %d\n", x->nRows, bParity);

  if(x->nRows >= x->nRows_max) {
    allocXORGElimTable(x, x->nRows+1, x->nVariables);
    //assert(0); // Cannot add any more vectors to the matrix
  }

  // Build the next free row
  x->rows[x->nRows].nLength = 0;
  for(uintmax_t i = 0; i < nVars; i++)
    uintmax_t_list_push(&x->rows[x->nRows], pnVarList[i]);
  uintmax_t_list_push(&x->rows[x->nRows], !bParity);

  D_7(printrow(&x->rows[x->nRows]););
 
  // Add the existing such row to the new vector.
  // Eliminate all 1's of the new vector in the current diagonal matrix
  for(uintmax_t i = 0; i < nVars; i++) {
    uintmax_t v;
    if ((v = x->first_bit[pnVarList[i]]) != ~(uintmax_t)0) {
      //Remove variable in diagonal
      D_7(
      d1_printf1("XORing:\n");
      printrow(&x->rows[x->nRows]); printrow(&x->rows[v]);
      d1_printf1("\n");
      );

      XORRows(&x->tmp_row, &x->rows[x->nRows], &x->rows[v]);
      uintmax_t_list_copy(&x->rows[x->nRows], &x->tmp_row);

      D_7(d2_printf1("a    ");printrow(&x->rows[x->nRows]);d2_printf3(" row %ju, var %ju\n", v, pnVarList[i]););
    }
  }
  
  //printLinearN(x);
  
  uintmax_t ret = rediagonalizeXORGElimTable(x, &x->rows[x->nRows], x->nRows);

  if(ret == 1) {
    // Insert the new row
    x->nRows++;
  }

  D_7(printXORGElimTable(x););

  if(ret == 0) {
    return UNSAT;
  }
  return NO_ERROR;
}

uint8_t ApplyInferenceToXORGElimTable(XORGElimTableStruct *x, uintmax_t nVar, uint8_t bValue) {
  uintmax_t pnVarlist[1];
  pnVarlist[0] = nVar;

  d7_printf1("    Checking the XORGETable\n");
  uint8_t ret = addRowXORGElimTable(x, 1, pnVarlist, !bValue);

  return ret; // Normal ending
}

void printrow(uintmax_t_list *row) {
  assert(row!=NULL);
  assert(row->pList!=NULL);
  d2_printf1("xor(");
  uintmax_t i;
  for(i = 0; i < row->nLength-1; i++) {
    d2_printf2(" %ju", row->pList[i]);
  }
  d2_printf2(" %c)\n", row->pList[i]?'T':'F');
}

void printXORGElimTable(XORGElimTableStruct *x) {
  uintmax_t i;
  for(i = 0; i < x->nRows; i++) {
    printrow(&x->rows[i]);
  }
  d2_printf1("\n");
}

/*
void printframeSize () {
  fprintf(stdout, "frame: %ju\n", frame_size);
}

uint8_t isMaskZero(XORGElimTableStruct *x) {
  for (uintmax_t i=0 ; i < vec_size ; i++) {
    if(x->mask[i]!=0) return 0;
  }
  return 1;
}

uint8_t isVectorZero(XORGElimTableStruct *x, uintmax_t *pVector) {
  for (uintmax_t word=0 ; word < vec_size; word++) {
    if (pVector[word] & x->mask[word]) return 0;
  }
  return 1;
}

uintmax_t nNumActiveXORGElimVectors(XORGElimTableStruct *x) {
  uintmax_t nNumActiveVectors = 0;
  for (int i=0 ; i < x->num_vectors; i++) {
    if(!isVectorZero(x, (uintmax_t *)(x->frame + (vecs_v_ref+i*vecs_rec_bytes))))
      nNumActiveVectors++;
  }
  return nNumActiveVectors;
}		  

void printMask (XORGElimTableStruct *x) {
  d2_printf2("mask (%ju", vec_size*sizeof(uintmax_t)*BITS_PER_BYTE);
  d2_printf1(" bits):\n     ");
  PrintXORGElimVector(x->mask);
  d2_printf1("\n");
}

void PrintXORGElimVector(uintmax_t *pVector) {
  for (uintmax_t word=0 ; word < vec_size; word++) {
    for(uintmax_t bit = word==0?1:0; bit < (uintmax_t)(sizeof(uintmax_t)*BITS_PER_BYTE); bit++) {
      if(pVector[word] & (((uintmax_t)1) << bit)) {
	d2_printf1("1");
      } else { d2_printf1("0"); }
    }
  }
  
  d2_printf1(".");
  if (pVector[0]&1) {
    d2_printf1("1");
  } else { d2_printf1("0"); }
}

void PrintMaskXORGElimVector(XORGElimTableStruct *x, uintmax_t *pVector) {
  for (uintmax_t word=0 ; word < vec_size; word++) {
    for(uintmax_t bit = word==0?1:0; bit < (uintmax_t)(sizeof(uintmax_t)*BITS_PER_BYTE); bit++) {
      if(pVector[word] & (((uintmax_t)1) << bit) & x->mask[word]) {
	d2_printf1("1");
      } else { d2_printf1("0"); }
    }
  }
  
  d2_printf1(".");
  if (pVector[0]&1) {
    d2_printf1("1");
  } else { d2_printf1("0"); }
}

void printLinearN (XORGElimTableStruct *x) {
  printMask(x);
  d2_printf1("Vectors:\n");
  for (uintmax_t i=0 ; i < x->num_vectors; i++) {
    d2_printf1("     ");
    uintmax_t *vn = (uintmax_t *)(x->frame + (vecs_v_ref+i*vecs_rec_bytes));
    PrintXORGElimVector((void *)vn);
    d2_printf1("     ");
    PrintMaskXORGElimVector(x, (void *)vn);
    d2_printf1("\n");		
  }
  d2_printf1(" +-----+     +-----+     +-----+     +-----+     +-----+\n");
}

void printLinear (XORGElimTableStruct *x) {
  uintmax_t xlate[512];
  intmax_t rows[512];
  //printLinearN(x);
  for (uintmax_t i=0 ; i < 512 ; i++) xlate[i] = i;
  uintmax_t j=0;
  for (intmax_t i=no_inp_vars-1 ; i >= 0 ; i--)
    if (x->first_bit[i] != -1) {
      rows[j] = x->first_bit[i]; xlate[j++] = i;
    }
  
  rows[j] = -1;
  for (intmax_t i=no_inp_vars-1 ; i >= 0 ; i--)
    if (x->first_bit[i] == -1) xlate[j++] = i;
  
  fprintf(stdout, "x->mask (%lu bits):\n", vec_size*sizeof(uintmax_t)*BITS_PER_BYTE);
  for (uintmax_t i=0 ; i < no_inp_vars ; i++) {
    uintmax_t word = xlate[i]/(sizeof(uintmax_t)*BITS_PER_BYTE);
    uintmax_t bit  = xlate[i] % (sizeof(uintmax_t)*BITS_PER_BYTE);
    if(x->mask[word] & ((uintmax_t)1 << bit)) fprintf(stdout, "1"); else fprintf(stdout, "0");
  }
  fprintf(stdout, "\n");
  
  fprintf(stdout, "Vectors:\n");
  for (uintmax_t i=0 ; rows[i] >= 0 ; i++) {
    fprintf(stdout, "     ");
    uintmax_t *vn = (uintmax_t *)(((uintmax_t *)x->frame)+vecs_v_ref+rows[i]*vecs_rec_bytes);
    for (j=0 ; j < no_inp_vars ; j++) {
      uintmax_t word = xlate[j]/(sizeof(uintmax_t)*BITS_PER_BYTE);
      uintmax_t bit  = xlate[j] % (sizeof(uintmax_t)*BITS_PER_BYTE);
      if (vn[word] & (1 << bit)) fprintf(stdout, "1"); else fprintf(stdout, "0");
    }
    fprintf(stdout, ".");
    if (vn[0]&1)
      fprintf(stdout, "1"); else fprintf(stdout, "0");
    fprintf(stdout, "     ");
    for (j=0 ; j < no_inp_vars ; j++) {
      uintmax_t word = xlate[j]/(sizeof(uintmax_t)*BITS_PER_BYTE);
      uintmax_t bit  = xlate[j] % (sizeof(uintmax_t)*BITS_PER_BYTE);
      if (vn[word] & x->mask[word] & ((uintmax_t)1 << bit)) fprintf(stdout, "1"); else fprintf(stdout, "0");
    }
    fprintf(stdout, ".");
    if (vn[0]&1)
      fprintf(stdout, "1"); else fprintf(stdout, "0");
    fprintf(stdout, "\n");
  }
  fprintf(stdout, "========================================================\n");
}
*/
