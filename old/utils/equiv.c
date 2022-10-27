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

#include "sbsat.h"

// Figure 1.
// Equivalence classes are maintained using inverted trees.  These are        
// adjusted when truth values are assigned to variables.  The following       
// diagram illustrates the structure of the data.  An explanation follows.    
// Please note that not all links are shown in this example.                  
//                                                                            
// equiv_lft[4]=3 |=======| equiv_rgt[4]=8                                    
//      +---------*   4   *---------+                                         
//      |    +--->|=======|<---+    |                       +--> -1           
//      |    |  equiv_cnt[3]   |    |                       |                 
//      +->|=*=|    =6       |=*=|<-+  equiv_fwd[2]=8     |=*=|               
//         * 3 |             * 8 |<-----------+     +-----* 6 |<-------+      
//      +->|=*=|<-+          |=*=|<-+         |     |     |=*=|<-+     |      
//      |         |                 |         |     |       |    |     |      
//      |         |                 |         |     |   +---|----|-----|----+ 
//      |         |                 |         |     |   |   |    |     |    | 
//    |=*=|     |=*=|             |=*=|     |=*=|   | +-|---|->|=*=| |=*=|  | 
//    * 4 |     * 1 |<--------+   * 9 |     * 2 |   | | +---|--* 10| * 12|<-+ 
//    |=*=|<-+  |=*=|<-+      |   |=*=|<-+  |=*=|   | |   +-|->|=*=| |=*=|<-+ 
//           |         |      |          |          | |   |  \              | 
//           |         |      |          |          | |   |   \-------------+ 
//         |=*=|     |=*=|  |=*=|      |=*=|        | | |=*=|                 
//         * 13|     * 5 |  * 11|      * 0 |        | +-* 7 |                 
//         |=*=|     |=*=|  |=*=|      |=*=|        +-->|=*=|                 
//                                           equiv_bck[6]=7                   
//                                                                            
//  For this example: variables 8 and 3 have opposite value, variables 1 and  
//    4 have the same value, it is not known whether variables 10 and 13      
//    must have the same or opposite values, there are no known variables     
//    which must have value opposite to that of variable 7.                   
//                                                                            
//  LEGEND                                                                    
//                                                                            
//  *---->    These are data links.  The "*" is the point of origin (an index 
//            into some array) and the ">" is the target (the value of the    
//            array element.  Names are given to some of the links in the     
//            diagram for illustration purposes.  Explanations are given      
//            below.                                                          
//                                                                            
// |=======|  These are "super" nodes which connect two equivalence classes   
// * 1   2 *  with the interpretation that variables of one class have value  
// |=======|  opposite to the variables of the other class.  There are two    
//            pointers denoted by "*".  For super node "x":                   
//              1. equiv_lft[x] points to the root of one equivalence class   
//              2. equiv_rgt[x] points to the root of the other.              
//            There is no particular significance to the value of x since     
//            super nodes are taken from an available pool of super nodes,    
//            as needed.                                                      
//     1                                                                      
//   |=*=|    These are the variables arranged in inverted trees to identify  
// 2 *   |    sets or equivalence classes of variables that must have the     
//   |=*=|    same value.  There are three pointers denoted by "*".  Each is  
//     3      explained for variable "x":                                     
//              1. equiv_fwd[x] has a value which is that of another variable 
//                 in the equivalence class, or is a negative number.  If it  
//                 is a negative number then x is the root of the equivalence 
//                 class.  If it is a negative number less than -1 then it is 
//                 also pointing to a "super" node and there exists a class   
//                 variables of opposite value to x and its equivalence class.
//              2. equiv_bck[x] points to another variable in the class       
//                 containing x.  Such pointers form a linked list linking    
//                 all variables of an equivalence class starting at the root.
//                 This enables a quick response to a query such as "what     
//                 variables are equivalent to x?".                           
//              3. equiv_end[x] points to a variable in the equivalence class 
//                 which is at the end of the linked list formed by equiv_bck.
//                 This is used to merge two classes quickly when variables   
//                 of different classes have been discovered to be equivalent.
//             In addition to the pointers, there is a value equiv_cnt[x]     
//             which associates with variable x: this is valid only if x is a 
//             root of an equivalence class and is then the number of         
//             variables in the equivalence class.  This is used to perform   
//             path compression when merging and to answer queries about the  
//             size of an equivalence class.                                  
//                                                                            
// NOTE: Not all links are shown in the diagram                               

Equiv_Manager_Struct *equiv_init(uintmax_t nNumVariables) {
  Equiv_Manager_Struct *EQM = (Equiv_Manager_Struct *)sbsat_calloc(1, sizeof(Equiv_Manager_Struct), 9, "Equiv Manager");

  EQM->nNumVariables = (intmax_t) nNumVariables;

  EQM->True = EQM->nNumVariables+1;
  EQM->False = EQM->True+1;
  uintmax_t size = EQM->False+1;
  
  uintmax_t equiv_fwd_sze = sizeof(uintmax_t)*(size);
  uintmax_t equiv_bck_sze = sizeof(uintmax_t)*(size);
  uintmax_t equiv_end_sze = sizeof(uintmax_t)*(size);
  uintmax_t equiv_cnt_sze = sizeof(uintmax_t)*(size);
  uintmax_t equiv_min_sze = sizeof(uintmax_t)*(size);
  uintmax_t equiv_rgt_sze = sizeof(uintmax_t)*(size);
  uintmax_t equiv_lft_sze = sizeof(uintmax_t)*(size);
  uintmax_t xlist_sze     = sizeof(uintmax_t)*nNumVariables;
  uintmax_t ylist_sze     = sizeof(uintmax_t)*nNumVariables;
 
  uintmax_t equiv_fwd_ref = 0;
  uintmax_t equiv_bck_ref = equiv_fwd_ref + equiv_fwd_sze;
  uintmax_t equiv_end_ref = equiv_bck_ref + equiv_bck_sze;
  uintmax_t equiv_cnt_ref = equiv_end_ref + equiv_end_sze;
  uintmax_t equiv_min_ref = equiv_cnt_ref + equiv_cnt_sze;
  uintmax_t equiv_rgt_ref = equiv_min_ref + equiv_min_sze;
  uintmax_t equiv_lft_ref = equiv_rgt_ref + equiv_rgt_sze;
  uintmax_t xlist_ref     = equiv_lft_ref + equiv_lft_sze;
  uintmax_t ylist_ref     = xlist_ref     + xlist_sze;
  
  EQM->frame_size = ylist_ref + ylist_sze;		
  
  EQM->frame = (char *)sbsat_calloc(1, EQM->frame_size, 9, "Equiv Manager memory frame");
  
  EQM->equiv_idx = 3;
  
  // Use equiv for inverted trees with path compression 
  EQM->equiv_fwd = (intmax_t *)(EQM->frame+equiv_fwd_ref);
  EQM->equiv_cnt = (intmax_t *)(EQM->frame+equiv_cnt_ref);
  EQM->equiv_min = (intmax_t *)(EQM->frame+equiv_min_ref);
  EQM->equiv_bck = (intmax_t *)(EQM->frame+equiv_bck_ref); 
  EQM->equiv_end = (intmax_t *)(EQM->frame+equiv_end_ref); 
  EQM->equiv_rgt = (intmax_t *)(EQM->frame+equiv_rgt_ref);
  EQM->equiv_lft = (intmax_t *)(EQM->frame+equiv_lft_ref); 
  
  for(uintmax_t i=0; i < size; i++) {
    EQM->equiv_fwd[i] = EQUIV_NULL;
    EQM->equiv_cnt[i] = 1;
    EQM->equiv_min[i] = i;
    EQM->equiv_bck[i] = EQUIV_NULL;
    EQM->equiv_end[i] = i;
    EQM->equiv_rgt[i] = EQUIV_NULL;
    EQM->equiv_lft[i] = EQUIV_NULL;
  }
  EQM->equiv_fwd[EQM->True] = -2;  // Create a super node for T and F immediately 
  EQM->equiv_fwd[EQM->False] = -2;
  EQM->equiv_rgt[2] = EQM->True;
  EQM->equiv_lft[2] = EQM->False;
  
  EQM->xlist = (intmax_t *)(EQM->frame+xlist_ref); 
  EQM->ylist = (intmax_t *)(EQM->frame+ylist_ref);  

  return EQM;
}

void equiv_free(Equiv_Manager_Struct *EQM) {
  sbsat_free((void **) &EQM->frame);
  sbsat_free((void **) &EQM);
}

uintmax_t get_equiv(Equiv_Manager_Struct *EQM, uintmax_t x) {
  uintmax_t a;
  for(a=x; EQM->equiv_fwd[a] >= 0; a=EQM->equiv_fwd[a]) {}
  return EQM->equiv_min[a];
}

uintmax_t get_oppos(Equiv_Manager_Struct *EQM, uintmax_t x) {
  uintmax_t a;
  intmax_t root_super;

  for(a=x; EQM->equiv_fwd[a] >= 0; a=EQM->equiv_fwd[a]) {}
  if(EQM->equiv_fwd[a] == EQUIV_NULL) return x;

  root_super = -EQM->equiv_fwd[a];
  a = (EQM->equiv_rgt[root_super] == a) ? EQM->equiv_lft[root_super] : EQM->equiv_rgt[root_super];

  return EQM->equiv_min[a];
}

//Make x and y have the same value
uint8_t equiv_insertEquiv(Equiv_Manager_Struct *EQM, intmax_t x, intmax_t y) {
  intmax_t xidx=0, yidx=0, a, b, i;
  intmax_t lft, rgt, tmp;
  
  intmax_t nNumVariables = EQM->nNumVariables;
  intmax_t *equiv_fwd = EQM->equiv_fwd;
  intmax_t *equiv_cnt = EQM->equiv_cnt;
  intmax_t *equiv_min = EQM->equiv_min;
  intmax_t *equiv_bck = EQM->equiv_bck;
  intmax_t *equiv_end = EQM->equiv_end;  
  intmax_t *equiv_rgt = EQM->equiv_rgt;
  intmax_t *equiv_lft = EQM->equiv_lft;
  intmax_t *xlist = EQM->xlist;
  intmax_t *ylist = EQM->ylist;

  for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]) xlist[xidx++] = a;
  for (b=y ; equiv_fwd[b] >= 0 ; b=equiv_fwd[b]) ylist[yidx++] = b;
  if (a != b) { // x and y are in different equivalence classes   
    // if true: consistent 
    if (!(equiv_fwd[a] == equiv_fwd[b] && equiv_fwd[a] <= -2)) { 
      
      lft = equiv_min[a];
      rgt = equiv_min[b];
      // if a has a super node and a is not T or F then see if other
      // side of equiv class tree structure has a min smaller than a's
      if (equiv_fwd[a] <= -2 && lft < nNumVariables) {
	if (equiv_lft[-equiv_fwd[a]] == a) {
	  if ((tmp = equiv_min[equiv_rgt[-equiv_fwd[a]]]) < lft)
	    lft = -tmp;
	} else {
	  if ((tmp = equiv_min[equiv_lft[-equiv_fwd[a]]]) < lft) 
	    lft = -tmp;
	}
      }
      // if b has a super node and b is not T or F then see if other
      // side of equiv class tree structure has a min smaller than b's
      if (equiv_fwd[b] <= -2 && rgt < nNumVariables) {
	if (equiv_lft[-equiv_fwd[b]] == b) {
	  if ((tmp = equiv_min[equiv_rgt[-equiv_fwd[b]]]) < rgt) 
	    rgt = -tmp;
	} else {
	  if ((tmp = equiv_min[equiv_lft[-equiv_fwd[b]]]) < rgt) 
	    rgt = -tmp;
	}
      }
      
      if(lft < 0 && rgt < EQM->True) {
	lft = -lft;
	rgt = -rgt;
      }
      
      EQM->lft_result = lft;
      EQM->rgt_result = rgt;
      
      intmax_t root = (equiv_cnt[a] < equiv_cnt[b]) ? b : a;
      intmax_t auxx = (equiv_cnt[a] < equiv_cnt[b]) ? a : b;
      if (equiv_bck[root] == EQUIV_NULL) {
	equiv_bck[root] = auxx;
      } else {
	equiv_bck[equiv_end[root]] = auxx;
      }
      equiv_end[root] = equiv_end[auxx];    // due to self loop on "end" 
      equiv_cnt[root] += equiv_cnt[auxx];
      if ((equiv_min[root] < nNumVariables && 
	   equiv_min[auxx] < equiv_min[root]) || 
	  equiv_min[auxx] >= nNumVariables) 
	equiv_min[root] = equiv_min[auxx];
      intmax_t auxx_super = -equiv_fwd[auxx]; // Should be a pos number 
      intmax_t root_super = -equiv_fwd[root]; // Should be a pos number 
      equiv_fwd[auxx] = root;
      
      // Straighten out the opposite classes     
      // Case 0: Auxx has no super - do nothing  
      if (auxx_super > 1) {
	// Case 1: Root has no super but auxx does 
	if (root_super == 1) {
	  if (equiv_lft[auxx_super] == auxx) {
	    equiv_lft[auxx_super] = root;
	  } else {
	    equiv_rgt[auxx_super] = root;
	  }
	  equiv_fwd[root] = -auxx_super;
	} else if (root_super > 1) {
	  // Case 2: Root and auxx have a super, they must be different
	  intmax_t auxx_opp = (equiv_lft[auxx_super] == auxx) ?
	    equiv_rgt[auxx_super] : equiv_lft[auxx_super];
	  intmax_t root_opp = (equiv_lft[root_super] == root) ?
	    equiv_rgt[root_super] : equiv_lft[root_super];
	  // Join opp trees and attach to opposite of new equiv class 
	  if (equiv_cnt[auxx_opp] < equiv_cnt[root_opp]) {
	    equiv_fwd[auxx_opp] = root_opp;
	    if (equiv_bck[root_opp] == -1) {
	      equiv_bck[root_opp] = auxx_opp;
	    } else {
	      equiv_bck[equiv_end[root_opp]] = auxx_opp;
	    }
	    equiv_end[root_opp] = equiv_end[auxx_opp];
	    equiv_cnt[root_opp] += equiv_cnt[auxx_opp];
	    if ((equiv_min[root_opp] < nNumVariables && 
		 equiv_min[auxx_opp] < equiv_min[root_opp]) ||
		equiv_min[auxx_opp] >= nNumVariables) 
	      equiv_min[root_opp] = equiv_min[auxx_opp];
	    equiv_fwd[root_opp] = -root_super;
	  } else {
	    equiv_fwd[root_opp] = auxx_opp;
	    if (equiv_bck[auxx_opp] == -1) {
	      equiv_bck[auxx_opp] = root_opp;
	    } else {
	      equiv_bck[equiv_end[auxx_opp]] = root_opp;
	    }
	    equiv_end[auxx_opp] = equiv_end[root_opp];
	    equiv_cnt[auxx_opp] += equiv_cnt[root_opp];
	    if ((equiv_min[auxx_opp] < nNumVariables && 
		 equiv_min[root_opp] < equiv_min[auxx_opp]) ||
		equiv_min[root_opp] >= nNumVariables)
	      equiv_min[auxx_opp] = equiv_min[root_opp]; 
	    equiv_fwd[auxx_opp] = -root_super;
	    if (equiv_rgt[root_super] == root) {
	      equiv_lft[root_super] = auxx_opp;
	    } else {
	      equiv_rgt[root_super] = auxx_opp;
	    }
	  }
	}
      }
      for (i=0 ; i < xidx ; i++) equiv_fwd[xlist[i]] = root;
      for (i=0 ; i < yidx ; i++) equiv_fwd[ylist[i]] = root;
    } else { // otherwise there is an inconsistency                  
      EQM->lft_result = EQM->True;
      EQM->rgt_result = EQM->False;
      return UNSAT;
    }
  } else return EQUIV_NO_CHANGE; // Already in same class - no change 
  return EQUIV_CHANGE;
}

// Make x and y have opposite value.
uint8_t equiv_insertOppos(Equiv_Manager_Struct *EQM, intmax_t x, intmax_t y) {
  intmax_t xidx=0, yidx=0, a, b, lft, rgt, tmp, i;
  
  intmax_t nNumVariables = EQM->nNumVariables;
  intmax_t *equiv_fwd = EQM->equiv_fwd;
  intmax_t *equiv_cnt = EQM->equiv_cnt;
  intmax_t *equiv_min = EQM->equiv_min;
  intmax_t *equiv_bck = EQM->equiv_bck;
  intmax_t *equiv_end = EQM->equiv_end;  
  intmax_t *equiv_rgt = EQM->equiv_rgt;
  intmax_t *equiv_lft = EQM->equiv_lft;
  intmax_t *xlist = EQM->xlist;
  intmax_t *ylist = EQM->ylist;

  for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]) xlist[xidx++] = a;
  for (b=y ; equiv_fwd[b] >= 0 ; b=equiv_fwd[b]) ylist[yidx++] = b;
  if (a == b) {  // Inconsistent 
    EQM->lft_result = EQM->True;
    EQM->rgt_result = EQM->False;
    return UNSAT;
  }
  if (equiv_fwd[a] == equiv_fwd[b] && equiv_fwd[a] <= -2) { // Already OK 
    return EQUIV_NO_CHANGE;
  }
  
  lft = -equiv_min[a];
  rgt = -equiv_min[b];
  
  // if a has a super node and a is not T or F then see if other
  // side of equiv class tree structure has a min smaller than a's
  if (equiv_fwd[a] <= -2 && -lft < nNumVariables) {
    if (equiv_lft[-equiv_fwd[a]] == a) {
      if ((tmp = equiv_min[equiv_rgt[-equiv_fwd[a]]]) < -lft) lft = tmp;
    } else {
      if ((tmp = equiv_min[equiv_lft[-equiv_fwd[a]]]) < -lft) lft = tmp;
    }
  } else if (-lft >= nNumVariables) {
    if (-lft == EQM->False) lft = EQM->True;
  }
  // if b has a super node and b is not T or F then see if other
  // side of equiv class tree structure has a min smaller than b's
  if (equiv_fwd[b] <= -2 && -rgt < nNumVariables) {
    if (equiv_lft[-equiv_fwd[b]] == b) {
      if ((tmp = equiv_min[equiv_rgt[-equiv_fwd[b]]]) < -rgt) rgt = tmp;
    } else {
      if ((tmp = equiv_min[equiv_lft[-equiv_fwd[b]]]) < -rgt) rgt = tmp;
    }
  } else if (-rgt >= nNumVariables) {
    if (-rgt == EQM->True) rgt = EQM->False;  //Iffy Here Sean!
    if (-rgt == EQM->False) rgt = EQM->True;
  }
  
  if(lft < 0 && rgt < EQM->True) {
    lft = -lft;
    rgt = -rgt;
  }
  EQM->lft_result = lft;
  EQM->rgt_result = rgt;		
  
  intmax_t a_super = -equiv_fwd[a]; // Should be a positive number 
  intmax_t b_super = -equiv_fwd[b]; // Should be a positive number 
  
  // If neither class a nor b has a super 
  if (a_super == 1 && b_super == 1) {
    equiv_lft[EQM->equiv_idx] = a;
    equiv_rgt[EQM->equiv_idx] = b;
    equiv_fwd[a] = -EQM->equiv_idx;
    equiv_fwd[b] = -EQM->equiv_idx;
    EQM->equiv_idx++;
    for (i=0 ; i < xidx ; i++) equiv_fwd[xlist[i]] = a;
    for (i=0 ; i < yidx ; i++) equiv_fwd[ylist[i]] = b;
  } else if (a_super > 1 && b_super == 1) {
    // If b has no super but a does - merge b with a's opposite 
    intmax_t r = (equiv_rgt[a_super] == a) ?
      equiv_lft[a_super] : equiv_rgt[a_super];
    intmax_t root = (equiv_cnt[r] < equiv_cnt[b]) ? b : r;
    intmax_t auxx = (equiv_cnt[r] < equiv_cnt[b]) ? r : b;
    if (equiv_bck[root] == -1) {
      equiv_bck[root] = auxx;
    } else {
      equiv_bck[equiv_end[root]] = auxx;
    }
    equiv_end[root] = equiv_end[auxx]; // because of self loop on "end" 
    equiv_cnt[root] += equiv_cnt[auxx];
    if ((equiv_min[root] < nNumVariables && 
	 equiv_min[auxx] < equiv_min[root]) ||
	equiv_min[auxx] >= nNumVariables) 
      equiv_min[root] = equiv_min[auxx];
    equiv_fwd[auxx] = root;
    equiv_fwd[root] = -a_super;
    if (equiv_rgt[a_super] == a) equiv_lft[a_super] = root;
    else equiv_rgt[a_super] = root;
    for (i=0 ; i < xidx ; i++) equiv_fwd[xlist[i]] = a;
    for (i=0 ; i < yidx ; i++) equiv_fwd[ylist[i]] = root;
  } else if (a_super == 1 && b_super > 1) {
    // If b has a super but a does not - merge a with b's opposite 
    intmax_t r = (equiv_rgt[b_super] == b) ?
      equiv_lft[b_super] : equiv_rgt[b_super];
    intmax_t root = (equiv_cnt[r] < equiv_cnt[a]) ? a : r;
    intmax_t auxx = (equiv_cnt[r] < equiv_cnt[a]) ? r : a;
    if (equiv_bck[root] == -1) {
      equiv_bck[root] = auxx;
    } else {
      equiv_bck[equiv_end[root]] = auxx;
    }
    equiv_end[root] = equiv_end[auxx]; // because of self loop on "end" 
    equiv_cnt[root] += equiv_cnt[auxx];
    if ((equiv_min[root] < nNumVariables && 
	 equiv_min[auxx] < equiv_min[root]) ||
	equiv_min[auxx] >= nNumVariables) 
      equiv_min[root] = equiv_min[auxx];
    equiv_fwd[auxx] = root;
    equiv_fwd[root] = -b_super;
    if (equiv_rgt[b_super] == b) equiv_lft[b_super] = root;
    else equiv_rgt[b_super] = root;
    for (i=0 ; i < xidx ; i++) equiv_fwd[xlist[i]] = root;
    for (i=0 ; i < yidx ; i++) equiv_fwd[ylist[i]] = b;
  } else {
    // If a and b have a super 
    intmax_t rb = (equiv_rgt[b_super] == b) ?
      equiv_lft[b_super] : equiv_rgt[b_super];
    intmax_t ra = (equiv_rgt[a_super] == a) ?
      equiv_lft[a_super] : equiv_rgt[a_super];  // ra, rb are opposites
    
    intmax_t root1 = (equiv_cnt[rb] < equiv_cnt[a]) ? a : rb;
    intmax_t auxx1 = (equiv_cnt[rb] < equiv_cnt[a]) ? rb : a;
    if (equiv_bck[root1] == -1) {
      equiv_bck[root1] = auxx1;
    } else {
      equiv_bck[equiv_end[root1]] = auxx1;
    }
    equiv_end[root1] = equiv_end[auxx1]; // because of self loop on "end"
    equiv_cnt[root1] += equiv_cnt[auxx1];
    if ((equiv_min[root1] < nNumVariables && 
	 equiv_min[auxx1] < equiv_min[root1]) ||
	equiv_min[auxx1] >= nNumVariables) 
      equiv_min[root1] = equiv_min[auxx1];
    equiv_fwd[auxx1] = root1;
    
    intmax_t root2 = (equiv_cnt[ra] < equiv_cnt[b]) ? b : ra;
    intmax_t auxx2 = (equiv_cnt[ra] < equiv_cnt[b]) ? ra : b;
    if (equiv_bck[root2] == -1) {
      equiv_bck[root2] = auxx2;
    } else {
      equiv_bck[equiv_end[root2]] = auxx2;
    }
    equiv_end[root2] = equiv_end[auxx2]; // because of self loop on "end" 
    equiv_cnt[root2] += equiv_cnt[auxx2];
    if ((equiv_min[root2] < nNumVariables && 
	 equiv_min[auxx2] < equiv_min[root2]) || 
	equiv_min[auxx2] >= nNumVariables) 
      equiv_min[root2] = equiv_min[auxx2];
    equiv_fwd[auxx2] = root2;
    
    equiv_fwd[root1] = -a_super;
    equiv_fwd[root2] = -a_super;			
    equiv_rgt[a_super] = root1;
    equiv_lft[a_super] = root2;
    for (i=0 ; i < xidx ; i++) equiv_fwd[xlist[i]] = root1;
    for (i=0 ; i < yidx ; i++) equiv_fwd[ylist[i]] = root2;
  }
  return EQUIV_CHANGE;
}

/*

// Returns 0 if no equivalence or error, 1 if equivalent, -1 if opposite
int Equiv::isEquiv (int x, int y) {
  int a, b;
  
  if (x < 0 || y < 0) return 0;
  for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]);
  for (b=y ; equiv_fwd[b] >= 0 ; b=equiv_fwd[b]);
  if (a == b) return 1;
  if (equiv_fwd[a] == equiv_fwd[b] && equiv_fwd[a] < -1) return EQUIV_NULL;
  return 0;
}

// Returns a -1 terminated list of vars equivalent to x or EQUIV_NULL on error
intmax_t *equivalent (int x) {
  int a, i=0;
  
  if (x < 0) { equiv_res[0] = EQUIV_NULL; return equiv_res; }
  for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]);
  while (a >= 0) {
    if (a != x) equiv_res[i++] = a;
    a = equiv_bck[a];
  }
  equiv_res[i] = EQUIV_NULL;
  return equiv_res;
}

// Returns a -1 terminated list of vars opposite to x or EQUIV_NULL on error
int *Equiv::opposite (int x) {
  int a, i=0;
  
  if (x < 0) { equiv_res[0] = EQUIV_NULL; return equiv_res; }
  for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]);
  if (equiv_fwd[a] == EQUIV_NULL) {
    equiv_res[0] = EQUIV_NULL;
  } else {
    int root_super = -equiv_fwd[a];
    a = (equiv_rgt[root_super] == a) ?
      equiv_lft[root_super] : equiv_rgt[root_super];
    while (a >= 0) {
      if (a != x) equiv_res[i++] = a;
      a = equiv_bck[a];
    }
    equiv_res[i] = EQUIV_NULL;
  }
  return equiv_res;
}

// Returns -1 terminated list of variables equivalent to T or F 
int *Equiv::valueOfTrue ()  { return equivalent(Tr); }
int *Equiv::valueOfFalse () { return equivalent(Fa); }

// Returns the number of other variables equivalent to x 
int Equiv::equivCount (int x) {
  int a;
  for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]);
  return equiv_cnt[a] - 1;
}

// Returns the number of other variables opposite to x 
int Equiv::opposCount (int x) {
  int a;
  for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]);
  int s = -equiv_fwd[a];
  int other = (equiv_lft[s] == a) ? equiv_rgt[s] : equiv_lft[s];
  int count = 0;
  for(a=other; a!=EQUIV_NULL; a=equiv_bck[a]) count++;
  return count;
  //return equiv_cnt[other] - 1;
}

void Equiv::printEquivalences () {
  int x;
  
  for (int i=0 ; i < nNumVariables ; i++) {
    if (equiv_fwd[i] < 0) {
      if (equiv_min[i] == Tr)
	cout << "[T] ";
      else if (equiv_min[i] == Fa)
	cout << "[F] ";
      else
	cout << "[" << equiv_min[i] << "] ";
      if (i == Tr) cout << "T "; else 
	if (i == Fa) cout << "F "; else
	  cout << i << " ";
      for (x=equiv_bck[i] ; x != EQUIV_NULL ; x=equiv_bck[x]) {
	if (x == Tr) cout << "T "; else
	  if (x == Fa) cout << "F "; else
	    cout << x << " "; 
      }
      cout << "|| ";
      if (equiv_fwd[i] < -1) {
	int super = -equiv_fwd[i];
	int root = (equiv_lft[super] == i) ? 
	  equiv_rgt[super] : equiv_lft[super];
	if (equiv_min[root] == Tr)
	  cout << "[T] ";
	else if (equiv_min[root] == Fa)
	  cout << "[F] ";
	else
	  cout << "[" << equiv_min[root] << "] ";
	if (root == Tr) cout << "T "; else
	  if (root == Fa) cout << "F "; else
	    cout << root << " ";
	for (x=equiv_bck[root] ; x != EQUIV_NULL ; x=equiv_bck[x]) {
	  if (x == Tr) cout << "T "; else
	    if (x == Fa) cout << "F "; else
	      cout << x << " ";
	}
      } 
      cout << "\n";
      flush(cout);
    }
  }
  if (equiv_fwd[Tr] == equiv_fwd[Fa] && equiv_fwd[Tr] != -1) {
    // Tr 
    cout << "T ";
    for (x=equiv_bck[Tr] ; x != EQUIV_NULL ; x=equiv_bck[x])
      cout << x << " ";
    cout << "|| F ";
    // Fa 
    for (x=equiv_bck[Fa] ; x != EQUIV_NULL ; x=equiv_bck[x])
      cout << x << " ";
    cout << "\n";
  }
  cout << "-------------------------\n";
}

void Equiv::printEquivalence (int x) {
  int a;
  for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]);
  for ( ; a != EQUIV_NULL ; a=equiv_bck[a]) cout << a << " ";
  cout << "\n";
}

void Equiv::printOpposite (int x) {
  int a;
  for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]);
  int super = -equiv_fwd[a];
  int root = (equiv_lft[super] == a) ? equiv_rgt[super] : equiv_lft[super];
  for (a=root ; a != EQUIV_NULL ; a=equiv_bck[a]) cout << a << " ";
  cout << "\n";
}

void Equiv::printNoLinksToRoot () {
  cout << "Links: ";
  for (int i=0 ; i < nNumVariables ; i++) {
    int j=0;
    for (int a=i ; equiv_fwd[a] >= 0 ; j++, a=equiv_fwd[a]);
    cout << j << " ";
  }
  cout << "\n";
}

void Equiv::printEquivClassCounts () {
  cout << "EqCnt (Var:cnt): ";
  for (int i=0 ; i < nNumVariables ; i++) {
    if (equiv_fwd[i] < 0) {
      cout << i << ":" << equiv_cnt[i] << " ";
    }
  }
  cout << "\n";		
}

void Equiv::printEquivVarCount () {
  cout << "EqVarCnt (Var:cnt): ";
  for (int i=0 ; i < nNumVariables ; i++) {
    cout << i << ":" << equivCount(i) << " ";
  }
  cout << "\n";
}

void Equiv::printOpposVarCount () {
  cout << "OpVarCount (Var:cnt): ";
  for (int i=0 ; i < nNumVariables ; i++) {
    cout << i << ":" << opposCount(i) << " ";
  }
  cout << "\n";
}

void Equiv::printWhetherEquiv () {
  int i;
  cout << "Equivalents:\n    ";
  for (i=0 ; i < nNumVariables ; i++) printf("%3d",i);
  cout << "\n";
  cout << "     ---------------------------------------------------------------\n";
  for (i=0 ; i < nNumVariables ; i++) {
    printf("%-3d:",i);
    for (int j=0 ; j < nNumVariables ; j++) {
      if (i == j) printf("   "); else printf("%3d",isEquiv(i,j));
    }
    printf("\n");
  }
}
*/
