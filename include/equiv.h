/*
 *    Copyright (C) 2011 Sean Weaver
 * 
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2, or (at your option)
 *    any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software Foundation,
 *    Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * */

#ifndef EQUIV_H
#define EQUIV_H

/**                              equivclass.h
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
 *    Result *insertEquiv (int x, int y): Make variable x equivalent to 
 *    variable y.  Return a pair of int values: the left value is the 
 *    smallest numbered variable which is either equivalent to or opposite
 *    of x before the insertion; the right value is the smallest numbered 
 *    variable that is either equivalent to or opposite of y before the
 *    insertion.  The left (right) value takes a negative sign if the 
 *    smallest variable is in the same equivalence class of x (y).
 *
 *    Result *insertOppos (int x, int y): Make variable x opposite to 
 *    variable y.  Return a pair of int values: the left value is the 
 *    smallest numbered variable which is either equivalent to or opposite
 *    of x before the insertion; the right value is the smallest numbered 
 *    variable that is either equivalent to or opposite of y before the
 *    insertion.  The left (right) value takes a negative sign if the 
 *    smallest variable is in the opposite equivalence class of x (y).
 * 
 *    int isEquiv (int x, int y): Returns 0 if no equivalence or error, 
 *    1 if equivalent, -1 if opposite.
 * 
 *    int *equivalent (int x): Returns a -1 terminated list of variables
 *    equivalent to x or null on error.
 * 
 *    int *opposite (int x):  Returns a -1 terminated list of variables
 *    opposite to x or null on error.
 * 
 *    int *valueOfTrue (): Returns -1 terminated list of variables 
 *    equivalent to true.
 *
 *    int *valueOfFalse (): Returns -1 terminated list of variables 
 *    equivalent to false.
 * 
 *    int equivCount (int x): Returns the number of other variables
 *    equivalent to x.
 *
 *    int opposCount (int x): Returns the number of other variables 
 *    opposite to x.
 * 
 *    void printEquivalences (): Print lists of equivalence classes.  A line
 *    in the list such as:
 * 
 *      [F] 16 F 58 99 98 || [T] 73 116 128 T
 * 
 *    means 99, 98, and 16 are equivalent to false and opposite to 73, 116, 
 *    128 and true.  Another example line is:
 * 
 *      [16] 16 99 98 || [73] 73 116 128
 *  
 *    The number in brackets is the variable which represents the equivalent 
 *    class (the root of the inverted tree).
**/

#define EQUIV_NULL     -1
#define EQUIV_CHANGE    1
#define EQUIV_NO_CHANGE 2

typedef struct Equiv_Manager_Struct{
  intmax_t nNumVariables; // The number of input variables 
  intmax_t True;
  intmax_t False;

  uintmax_t frame_size;
  char *frame;
  
  uintmax_t equiv_idx; // How many such nodes are in use.
  
  intmax_t *equiv_fwd; // Structure for maintaining and finding equivalences
  intmax_t *equiv_cnt; // Number of variables in an equiv tree - path comp.
  intmax_t *equiv_min; // 
  intmax_t *equiv_bck; // Linked list downward from root of inverted tree
  intmax_t *equiv_end; // Pointer to end of the bck linked list
  intmax_t *equiv_rgt; // These maintain nodes that point to trees of
  intmax_t *equiv_lft; //   opposite equivalence.
  intmax_t *xlist;     // Temporary space for equivalence operations         
  intmax_t *ylist;     // Temporary space for equivalence operations

  intmax_t lft_result; //lft_result = rgt_result
  intmax_t rgt_result;
} Equiv_Manager_Struct;

Equiv_Manager_Struct *equiv_init(uintmax_t nNumVariables);
void equiv_free(Equiv_Manager_Struct *EQM); 
uintmax_t get_equiv(Equiv_Manager_Struct *EQM, uintmax_t x);
uintmax_t get_oppos(Equiv_Manager_Struct *EQM, uintmax_t x);
uint8_t equiv_insertEquiv(Equiv_Manager_Struct *EQM, intmax_t x, intmax_t y);
uint8_t equiv_insertOppos(Equiv_Manager_Struct *EQM, intmax_t x, intmax_t y);

/*
  // Returns 0 if no equivalence or error, 1 if equivalent, -1 if opposite
  int isEquiv (int x, int y);
  
  // Returns a -1 terminated list of vars equivalent to x or null on error
  int *equivalent (int x);
  
  // Returns a -1 terminated list of vars opposite to x or null on error
  int *opposite (int x);
  
  // Returns -1 terminated list of variables equivalent to T or F 
  int *valueOfTrue ();
  int *valueOfFalse ();
  
  // Returns the number of other variables equivalent to x 
  int equivCount (int x);
  
  // Returns the number of other variables opposite to x 
  int opposCount (int x);
  
  void printEquivalences ();
  
  void printEquivalence (int x);
  
  void printOpposite (int x);
  
  void printNoLinksToRoot ();
  
  void printEquivClassCounts ();
  
  void printEquivVarCount ();
  
  void printOpposVarCount ();
  
  void printWhetherEquiv ();
*/
    
#endif
