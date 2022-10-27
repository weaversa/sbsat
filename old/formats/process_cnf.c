/*
   Copyright (C) 2009 Sean Weaver

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "sbsat.h"

int find_cnf_gates_flag;
int cluster_cnf_flag;
int clause_cluster_overlap;
int clause_cluster_max_length;
int write_gates_flag;
int write_cnf_flag;
                   
uint8_t free_clauses(CNF_Struct *CNF) {
  uint8_t ret = NO_ERROR;
  
  if(CNF->pClauses != NULL) {
    for(uintmax_t x = 0; x < CNF->nNumClauses; x++) {
      if(CNF->pClauses[x].variables != NULL) sbsat_free((void **)&(CNF->pClauses[x].variables));
    }
    sbsat_free((void **)&CNF->pClauses);
  }
  CNF->nNumClauses = 0;
  CNF->nNumVariables = 0;
  
  return ret;
}

int clscompfunc(const void *x, const void *y) {
  Clause pp, qq;
  
  pp = *(const Clause *)x;
  qq = *(const Clause *)y;
  
  //Compare the lengths of the clauses.
  if(pp.length != qq.length )
    return (pp.length < qq.length ? -1 : 1);
  
  //The lengths of pp and qq are the same.
  //Now take a look at the variables in the clauses.
  for(uintmax_t i = 0; i < pp.length; i++) {
    assert(i < pp.length);
    assert(i < qq.length);
    if(imaxabs(pp.variables[i]) != imaxabs(qq.variables[i]))
      return (imaxabs(pp.variables[i]) < imaxabs(qq.variables[i]) ? -1 : 1);
  }
  
  //If the two clauses contain the same variables, then consider the
  //literals and compare again. ( So, no abs() is used here ). This is done to make
  //removal of duplets easy.
  for(uintmax_t i = 0; i < pp.length; i++) {
    assert(i < pp.length);
    assert(i < qq.length);
    if(pp.variables[i] != qq.variables[i])
      return (pp.variables[i] < qq.variables[i] ? -1 : 1);
  }
  
  //Default value if all are equal. ( Thus a duplet... )
#ifndef FORCE_STABLE_QSORT
  return 0;
#else
  {
    if (x < y) return -1;
    else if (x > y) return 1;
    else return 0;
  }
#endif
  return 1;
}

uint8_t compress_variables(CNF_Struct *CNF) {
  uint8_t ret = NO_ERROR;
  
  uintmax_t nNumClauses = CNF->nNumClauses;
  uintmax_t nNumVariables = CNF->nNumVariables;
  Clause *pClauses = CNF->pClauses;
  
  uintmax_t *cnf_var_map = (uintmax_t *)sbsat_calloc(nNumVariables+1, sizeof(uintmax_t), 9, "cnf_var_map");
  
  uintmax_t nNumVariablesFound = 0;
  
  for(uintmax_t x = 0; x < nNumClauses; x++) {
    for(uintmax_t y = 0; y < pClauses[x].length; y++) {
      uintmax_t var = imaxabs(pClauses[x].variables[y]);
      if(cnf_var_map[var] == 0) {
	nNumVariablesFound++;
	char num[20]; //0xFFFFFFFFFFFFFFFF needs 20 characters in base 10.
	sprintf(num, "%ju", var);
	putsym(num, SYM_VAR);
	cnf_var_map[var] = i_getsym(num, SYM_VAR);
      }
      pClauses[x].variables[y] = pClauses[x].variables[y] < 0 ? -cnf_var_map[var] : cnf_var_map[var];
    }
  }
  
  assert(nNumVariablesFound <= nNumVariables);
  
  CNF->nNumVariables = nNumVariablesFound;
  
  d2_printf2("Variable compression removed %ju variables\n", nNumVariables - nNumVariablesFound);
  
  sbsat_free((void **)&cnf_var_map);
  
  return ret;
}

uint8_t remove_dup_lits(CNF_Struct *CNF) {
  uint8_t ret = NO_ERROR;

  uintmax_t nNumClauses = CNF->nNumClauses;
  if(nNumClauses == 0) return ret;

  Clause *pClauses = CNF->pClauses;

  for(uintmax_t x = 0; x < nNumClauses-1; x++) {
    uintmax_t y = 0;
    for(uintmax_t z = 1; z < pClauses[x].length; z++) {
      if(pClauses[x].variables[y] == pClauses[x].variables[z]) {
	//Duplicate literal, remove it
      } else if(pClauses[x].variables[y] == -pClauses[x].variables[z]) {
	//Clause is a tautology, mark for deletion.
	pClauses[x].subsumed = 1;
	break;
      } else {
	pClauses[x].variables[y+1] = pClauses[x].variables[z];
	y++;
      }
    }
    pClauses[x].length = y+1;
  }

  return ret;
}

uint8_t reduce_clauses(CNF_Struct *CNF, BDDManager *BM) {
  uint8_t ret = NO_ERROR;
  
  uintmax_t nNumClauses = CNF->nNumClauses;
  uintmax_t nNumVariables = CNF->nNumVariables;
  Clause *pClauses = CNF->pClauses;
  
  //Mark duplicate clauses
  if(nNumClauses!=0) {
    for(uintmax_t x = 0; x < nNumClauses-1; x++) {
      uint8_t isdup = 0;
      if(pClauses[x].length == pClauses[x+1].length) {
	isdup = 1;
	for(uintmax_t y = 0; y < pClauses[x].length; y++) {
	  if(pClauses[x].variables[y] != pClauses[x+1].variables[y]) {
	    isdup = 0;
	    break;
	  }
	}
      }
      if(isdup == 1) {
	pClauses[x+1].subsumed = 1;
      }
    }
  }
  
  //Remove subsumed clauses
  uintmax_t subs = 0;
  for(uintmax_t x = 0; x < nNumClauses; x++) {
    pClauses[x].length = pClauses[x+subs].length;
    pClauses[x].variables = pClauses[x+subs].variables;
    pClauses[x].subsumed = pClauses[x+subs].subsumed;
    
    if(pClauses[x+subs].subsumed == 1) {
      pClauses[x+subs].length = 0;
      sbsat_free((void **)&pClauses[x+subs].variables);
      pClauses[x+subs].variables = NULL;
      subs++;x--;nNumClauses--;continue;
    }
  }
  
  if(nNumClauses < CNF->nNumClauses) {
    if(nNumClauses == 0) {
      sbsat_free((void **)&CNF->pClauses);
      CNF->pClauses = NULL;
    } else {
      CNF->pClauses = (Clause *)sbsat_realloc(CNF->pClauses, CNF->nNumClauses, nNumClauses, sizeof(Clause), 9, "CNF->pClauses");
    }
  }
  
  uintmax_t numBDDNodesGarbageCollected = BDDManager_GC(BM);
  
  d2_printf3("Simplify removed %ju clauses and %ju BDD nodes\n", subs, numBDDNodesGarbageCollected);
  
  CNF->nNumClauses = nNumClauses;
  CNF->nNumVariables = nNumVariables;

  return ret;
}

uint8_t find_and_add_xors(CNF_Struct *CNF, BDDManager *BM) {
  //Search for XORs - This code designed after a snippet of march_dl by M. Heule
  uint8_t ret = NO_ERROR;
  
  uintmax_t nNumClauses = CNF->nNumClauses;
  Clause *pClauses = CNF->pClauses;
  
  uintmax_t num_xors_found = 0;
  for(uintmax_t x = 0; x < nNumClauses; x++) {
    assert(pClauses[x].length > 0);
    if(pClauses[x].length>1) {
      uintmax_t domain = 1<<(pClauses[x].length-1);
      if(domain<=1) break;
      if(domain+x > nNumClauses) break;
      
      uint8_t cont = 0;
      for(uintmax_t y = 1; y < domain; y++) 
	if(pClauses[x+y].length != pClauses[x].length) {
	  x += (y-1);
	  cont = 1;
	  break;
	}
      if(cont == 1) continue;
      
      for(uintmax_t y = 0; y < pClauses[x].length; y++)
	if(imaxabs(pClauses[x].variables[y]) != imaxabs(pClauses[x+domain-1].variables[y])) {
	  cont = 1;
	  break;
	}
      if(cont == 1) continue;
      
      intmax_t sign = 1;
      for(uintmax_t y = 0; y < pClauses[x].length; y++)
	sign *= pClauses[x].variables[y] < 0 ? -1 : 1;
      
      for(uintmax_t y = 1; y < domain; y++) { //Safety check - probably not needed
	intmax_t tmp = 1;
	for(uintmax_t z = 0; z < pClauses[x+y].length; z++) //pClauses[x+y].length == pClauses[x].length
	  tmp *= pClauses[x+y].variables[z] < 0 ? -1 : 1;
	if(tmp != sign) {
	  cont = 1;
	  break;
	}
      }
      if(cont == 1) continue;
      
      num_xors_found++;
      
      if(write_gates_flag) {
	//Found XOR. Printing it for demonstration purposes.
	fprintf(foutputfile, "xor(%jd", pClauses[x].variables[0]);
	for(uintmax_t y = 1; y < pClauses[x].length; y++) {
	  fprintf(foutputfile, ", %jd", pClauses[x].variables[y]);
	}
	fprintf(foutputfile, ")\n");
      }
      
      ret = add_xor_BDD(BM, pClauses[x].variables, pClauses[x].length);
      if(ret != NO_ERROR) return ret;
      
      //Marking clauses used in the XOR so that they will be removed.
      for(uintmax_t y = 0; y < domain; y++)
	pClauses[x+y].subsumed = 1;
      x += domain-1;
    }
  }
  
  d2_printf2("Found %ju XOR functions\n", num_xors_found);
  sbsat_stats[STAT_NUM_XORS] = num_xors_found;
  
  return ret;
}

uint8_t find_and_add_andequals(CNF_Struct *CNF, BDDManager *BM) {
  //Search for AND= and OR=
  uint8_t ret = NO_ERROR;
  
  uintmax_t nNumClauses = CNF->nNumClauses;
  uintmax_t nNumVariables = CNF->nNumVariables;
  Clause *pClauses = CNF->pClauses;

  uintmax_t *twopos_temp = (uintmax_t *)sbsat_calloc(nNumVariables+1, sizeof(uintmax_t), 9, "twopos_temp");
  uintmax_t *twoneg_temp = (uintmax_t *)sbsat_calloc(nNumVariables+1, sizeof(uintmax_t), 9, "twoneg_temp");
  for(uintmax_t x = 0; x < nNumClauses; x++) {
    if(pClauses[x].length == 2) {
      for(uintmax_t y = 0; y < 2; y++) {
	if(pClauses[x].variables[y] > 0)
	  twopos_temp[pClauses[x].variables[y]]++;
	else
	  twoneg_temp[-pClauses[x].variables[y]]++;
      }
    } else if(pClauses[x].length > 2) break; //Relies on clauses being sorted
  }
  store *two_pos = (store *)sbsat_calloc(nNumVariables+1, sizeof(store), 9, "two_pos");
  store *two_neg = (store *)sbsat_calloc(nNumVariables+1, sizeof(store), 9, "two_neg");

  //two_pos and two_neg are lists that contain all the clauses
  //that are of length 2. two_pos contains every 2 variable clause
  //that has a positive variable, two_neg contains every 2
  //variable clause that has a negative variable. There will most likely
  //be some overlaps in the variable storing.
  //EX)
  //p cnf 3 3
  //2 3 0
  //-2 -3 0
  //-2 3 0
  //
  //two_pos will point to (2:3)   and (-2:3)
  //two_neg will point to (-2:-3) and (-2:3)
  
  //Storing appropriate array sizes...
  for(uintmax_t x = 1; x <= nNumVariables; x++) {
    two_pos[x].num = (uintmax_t *)sbsat_calloc(2*twopos_temp[x], sizeof(uintmax_t), 9, "two_pos[x].num");
    two_neg[x].num = (uintmax_t *)sbsat_calloc(2*twoneg_temp[x], sizeof(uintmax_t), 9, "two_neg[x].num");
  }

  sbsat_free((void **)&twopos_temp);
  sbsat_free((void **)&twoneg_temp);

  //This is where two_pos, two_neg are filled with clauses
  
  for(uintmax_t x = 0; x < nNumClauses; x++) {
    if(pClauses[x].length == 2) {
      if(pClauses[x].variables[0] > 0) {
	uintmax_t y = two_pos[pClauses[x].variables[0]].length;
	two_pos[pClauses[x].variables[0]].num[y] = (uintmax_t)pClauses[x].variables[1];
	two_pos[pClauses[x].variables[0]].num[y+1] = x;
	two_pos[pClauses[x].variables[0]].length+=2;
      } else {
	uintmax_t y = two_neg[-pClauses[x].variables[0]].length;
	two_neg[-pClauses[x].variables[0]].num[y] = (uintmax_t)pClauses[x].variables[1];
	two_neg[-pClauses[x].variables[0]].num[y+1] = x;
	two_neg[-pClauses[x].variables[0]].length+=2;
      }
      if(pClauses[x].variables[1] > 0) {
	uintmax_t y = two_pos[pClauses[x].variables[1]].length;
	two_pos[pClauses[x].variables[1]].num[y] = (uintmax_t)pClauses[x].variables[0];
	two_pos[pClauses[x].variables[1]].num[y+1] = x;
	two_pos[pClauses[x].variables[1]].length+=2;
      } else {
	uintmax_t y = two_neg[-pClauses[x].variables[1]].length;
	two_neg[-pClauses[x].variables[1]].num[y] = (uintmax_t)pClauses[x].variables[0];
	two_neg[-pClauses[x].variables[1]].num[y+1] = x;
	two_neg[-pClauses[x].variables[1]].length+=2;
      }
    } else if(pClauses[x].length > 2) break; //Relies on clauses being sorted
  }
  
  uintmax_t num_andequals_found = 0;
  
  uint8_t find_all = 0; //SEAN!!! Make this a command line option
  //For all clauses greater than length 2
  for(uintmax_t x = 0; x < nNumClauses; x++) {
    if (x%1000 == 1) {
      d4_printf3("\rAND/OR Search CNF %ju/%ju ...                                             ", x, nNumClauses);
    }
    if(pClauses[x].length <= 2) continue;
    if(pClauses[x].subsumed == 1) continue;

    //      fprintf(stdout, "Considering clause %ju: ", x);
    //      write_clause(CNF, x);
    //For each variable in clause x
    for(uintmax_t y = 0; y < pClauses[x].length; y++) {
      //See if the 2-clauses cover clause x on variable v = pClauses[x].variables[y]
      intmax_t v = pClauses[x].variables[y];
      intmax_t vabs = imaxabs(v);
      store *two_v = (v==vabs) ? two_neg : two_pos;
      
      if(two_v[vabs].length < pClauses[x].length-1) continue;
      uint8_t out = 0;
      uintmax_t x_pos = 0;
      uintmax_t v_pos = 0;
      //         fprintf(stdout, "2 clauses %jd: ", v);
      //         for(uintmax_t p = 0; p < two_v[vabs].length; p+=2) fprintf(stdout, "%jd ", (intmax_t)two_v[vabs].num[p]);
      //         fprintf(stdout, "0\n");
      
      while((out==0) && (x_pos < pClauses[x].length)) {
	if(x_pos == y)
	  { x_pos++; }
	else if(v_pos >= two_v[vabs].length)
	  { break; }
	else if(pClauses[x].variables[x_pos] == -(intmax_t)two_v[vabs].num[v_pos])
	  { x_pos++; v_pos+=2; }
	else if(imaxabs(pClauses[x].variables[x_pos]) > imaxabs((intmax_t)two_v[vabs].num[v_pos]))
	  { v_pos+=2; }
	else { out=1; }
      }
      //         fprintf(stdout, "result = %u %ju %ju\n", out, x_pos, v_pos);
      if((out==0) && (x_pos == pClauses[x].length)) {
	//Found a gate
	num_andequals_found++;
	pClauses[x].subsumed = 1;
	x_pos = 0;
	v_pos = 0;
	while((x_pos < pClauses[x].length) && (v_pos < two_v[vabs].length)) {
	  if(x_pos == y)
	    { x_pos++; }
	  else if(pClauses[x].variables[x_pos] == -(intmax_t)two_v[vabs].num[v_pos])
	    { x_pos++; pClauses[two_v[vabs].num[v_pos+1]].subsumed = 1; v_pos+=2;}
	  else if(imaxabs(pClauses[x].variables[x_pos]) > imaxabs((intmax_t)two_v[vabs].num[v_pos]))
	    { v_pos+=2; }
	}
	//Print gate:
        
	if(write_gates_flag) {							
	  if(v==vabs) {                       
	    //Found AND=. Printing it for demonstration purposes.
	    fprintf(foutputfile, "equ(%jd, and(", vabs);
	    for(uintmax_t z = 0; z < pClauses[x].length; z++) {
	      if(pClauses[x].variables[z] != v)
		fprintf(foutputfile, "%jd, ", -pClauses[x].variables[z]);
	    }
	    fprintf(foutputfile, "))\n");
	  } else {
	    //Found OR=. Printing it for demonstration purposes.
	    fprintf(foutputfile, "equ(%jd, or(", vabs);
	    for(uintmax_t z = 0; z < pClauses[x].length; z++) {
	      if(pClauses[x].variables[z] != v)
		fprintf(foutputfile, "%jd, ", pClauses[x].variables[z]);
	    }
	    fprintf(foutputfile, "))\n");
	  }
	}
	if(v==vabs) {
	  //Must negate the list
	  for(uintmax_t z = 0; z < pClauses[x].length; z++)
	    pClauses[x].variables[z] = -pClauses[x].variables[z];
	  ret = add_and_equals_BDD(BM, vabs, pClauses[x].variables, pClauses[x].length);
	  if(ret != NO_ERROR) return ret;
	  for(uintmax_t z = 0; z < pClauses[x].length; z++)
	    pClauses[x].variables[z] = -pClauses[x].variables[z];
	} else {
	  ret = add_or_equals_BDD(BM, vabs, pClauses[x].variables, pClauses[x].length);
	  if(ret != NO_ERROR) return ret;
	}
	if(find_all==0) break;
      }
    }
  }
  
  //Not needed anymore, free them!
  for(uintmax_t x = 1; x < nNumVariables + 1; x++) {
    sbsat_free((void **)&two_pos[x].num);
    sbsat_free((void **)&two_neg[x].num);
  }
  
  sbsat_free((void **)&two_pos);
  sbsat_free((void **)&two_neg);

  d2_printf2("\rFound %ju AND=/OR= functions           \n", num_andequals_found);
  sbsat_stats[STAT_NUM_AND_EQUALS] = num_andequals_found;
  
  return ret;
}

uint8_t find_and_add_iteequals(CNF_Struct *CNF, BDDManager *BM) {
  uint8_t ret = NO_ERROR;
  
  uintmax_t nNumClauses = CNF->nNumClauses;
  uintmax_t nNumVariables = CNF->nNumVariables;
  Clause *pClauses = CNF->pClauses;
  
  uintmax_t num_iteequals_found = 0;
  
  uintmax_t *threepos_temp = (uintmax_t *)sbsat_calloc(nNumVariables+1, sizeof(uintmax_t), 9, "threepos_temp");
  uintmax_t *threeneg_temp = (uintmax_t *)sbsat_calloc(nNumVariables+1, sizeof(uintmax_t), 9, "threeneg_temp");
  for(uintmax_t x = 0; x < nNumClauses; x++) {
    uintmax_t y = pClauses[x].length;
    if(y == 3) {
      for(y = 0; y < 3; y++) {
	if(pClauses[x].variables[y] > 0)
	  threepos_temp[pClauses[x].variables[y]]++;
	else
	  threeneg_temp[-pClauses[x].variables[y]]++;
      }
    } else if(y > 3) break; //Relies on clauses being sorted
  }
  store *three_pos = (store *)sbsat_calloc(nNumVariables+1, sizeof(store), 9, "three_pos");
  store *three_neg = (store *)sbsat_calloc(nNumVariables+1, sizeof(store), 9, "three_neg");
  
  //Store appropriate array sizes to help with memory usage
  for(uintmax_t x = 1; x < nNumVariables+1; x++) {
    three_pos[x].num = (uintmax_t *)sbsat_calloc(threepos_temp[x], sizeof(uintmax_t), 9, "three_pos[x].num");
    three_neg[x].num = (uintmax_t *)sbsat_calloc(threeneg_temp[x], sizeof(uintmax_t), 9, "three_neg[x].num");
  }
  sbsat_free((void **)&threepos_temp);
  sbsat_free((void **)&threeneg_temp);
  
  //Store all clauses with 3 variables so they can be clustered
  uintmax_t count = 0;
  for(uintmax_t x = 0; x < nNumClauses; x++) {
    if(pClauses[x].length == 3) {
      count++;
      for(uint8_t i = 0; i < 3; i++) {
	if(pClauses[x].variables[i] < 0) {
	  three_neg[-pClauses[x].variables[i]].num[three_neg[-pClauses[x].variables[i]].length] = x;
	  three_neg[-pClauses[x].variables[i]].length++;
	} else {
	  three_pos[pClauses[x].variables[i]].num[three_pos[pClauses[x].variables[i]].length] = x;
	  three_pos[pClauses[x].variables[i]].length++;
	}
      }
    } else if(pClauses[x].length > 3) continue; //Relies on clauses being sorted
  }
  
  //v3 is in all clauses
  //if v0 is positive, both v0 positive clauses have v2, just sign changed
  //if v0 is negative, both v0 negative clauses have v1, just sign changed
  //the signs of v1 and v2 are the inverse of the sign of v3
  typedef struct ite_3 {
    uintmax_t pos;
    uintmax_t neg;
    uintmax_t v0;
    uintmax_t v1;
  } ite_3;
  
  uintmax_t v3_1size = 1000;
  uintmax_t v3_2size = 1000;
  ite_3 *v3_1 = (ite_3 *)sbsat_calloc(v3_1size, sizeof(ite_3), 9, "v3_1");
  ite_3 *v3_2 = (ite_3 *)sbsat_calloc(v3_2size, sizeof(ite_3), 9, "v3_2");
  
  uintmax_t v3_1count;
  uintmax_t v3_2count;
  
  for(uintmax_t x = 0; x < nNumVariables+1; x++) {
    v3_1count = 0;
    v3_2count = 0;
    for(uintmax_t i = 0; i < three_pos[x].length; i++) {
      //Finding clauses that have 1 negative variable
      //and clauses that have 2 negative variables
      count = 0;
      for(uint8_t y = 0; y < 3; y++) {
	if(pClauses[three_pos[x].num[i]].variables[y] < 0)
	  count++;
      }
      if(count == 1) {
	v3_1[v3_1count].pos = three_pos[x].num[i];
	v3_1count++;
	if(v3_1count > v3_1size) {
	  v3_1 = (ite_3 *)sbsat_recalloc((void*)v3_1, v3_1size, v3_1size+1000, sizeof(ite_3), 9, "v3_1 recalloc");
	  v3_1size+=1000;
	}
      } else if(count == 2) {
	v3_2[v3_2count].pos = three_pos[x].num[i];
	v3_2count++;
	if(v3_2count > v3_2size) {
	  v3_2 = (ite_3 *)sbsat_recalloc((void*)v3_2, v3_2size, v3_2size+1000, sizeof(ite_3), 9, "v3_2 recalloc");
	  v3_2size+=1000;
	}
      }
    }
    
    //Search through the clauses with 1 negative variable
    //  and try to find counterparts
    for(uintmax_t i = 0; i < v3_1count; i++) {
      uint8_t out = 0;
      for(uintmax_t y = 0; (y < three_neg[x].length) && (!out); y++) {
	v3_1[i].v0 = ~(uintmax_t)0;
	v3_1[i].v1 = ~(uintmax_t)0;
	count = 0;
	for(uint8_t z = 0; z < 3; z++) {
	  if(pClauses[v3_1[i].pos].variables[z] != (intmax_t)x) {
	    for(uint8_t j = 0; j < 3; j++) {
	      if((pClauses[v3_1[i].pos].variables[z] == pClauses[three_neg[x].num[y]].variables[j])
		 &&(pClauses[v3_1[i].pos].variables[z] > 0))
		{
		  count++;
		  v3_1[i].v0 = z;
		} else if(-pClauses[v3_1[i].pos].variables[z] == pClauses[three_neg[x].num[y]].variables[j])
		//&&(pClauses[v3_1[i].pos].variables[z]<0))
		{
		  count++;
		  v3_1[i].v1 = z;
		}
	    }
	  }
	}
	if(count == 2) {
	  //The counterpart clause to v3_1[i].pos is v3_1[i].neg
	  v3_1[i].neg = three_neg[x].num[y];
	  out = 1;
	}
      }
      if(out == 0) {
	v3_1[i].v0 = ~(uintmax_t)0;
	v3_1[i].v1 = ~(uintmax_t)0;
      }
    }
    //Search through the clauses with 2 negative variables
    for(uintmax_t i = 0; i < v3_2count; i++) {
      uint8_t out = 0;
      for(uintmax_t y = 0; (y < three_neg[x].length) && (!out); y++) {
	v3_2[i].v0 = ~(uintmax_t)0;
	v3_2[i].v1 = ~(uintmax_t)0;
	count = 0;
	for(uint8_t z = 0; z < 3; z++) {
	  if(pClauses[v3_2[i].pos].variables[z] != (intmax_t)x) {
	    for(uint8_t j = 0; j < 3; j++) {
	      if((pClauses[v3_2[i].pos].variables[z] == pClauses[three_neg[x].num[y]].variables[j])
		 &&(pClauses[v3_2[i].pos].variables[z] < 0))
		{
		  count++;
		  v3_2[i].v0 = z;
		} else if(-pClauses[v3_2[i].pos].variables[z] == pClauses[three_neg[x].num[y]].variables[j])
		//&&(-pClauses[v3_2[i].pos].variables[z]>0));
		{
		  count++;
		  v3_2[i].v1 = z;
		}
	    }
	  }
	}
	if(count == 2) {
	  //The counterpart clause to v3_2[i].pos is v3_2[i].neg
	  v3_2[i].neg = three_neg[x].num[y];
	  out = 1;
	}
      }
      if(out == 0) {
	v3_2[i].v0 = ~(uintmax_t)0;
	v3_2[i].v1 = ~(uintmax_t)0;
      }
    }
    uint8_t out = 0;
    for(uintmax_t i = 0; (i < v3_1count) && (!out); i++) {
      for(uintmax_t y = 0; (y < v3_2count) && (!out); y++) {
	if((v3_1[i].v0 == ~(uintmax_t)0) || (v3_1[i].v1 == ~(uintmax_t)0)
	   ||(v3_2[y].v0 == ~(uintmax_t)0) || (v3_2[y].v1 == ~(uintmax_t)0))
	  continue;
	if(pClauses[v3_1[i].pos].variables[v3_1[i].v0] == -pClauses[v3_2[y].pos].variables[v3_2[y].v0]) {
	  
	  pClauses[v3_1[i].pos].subsumed = 1;
	  pClauses[v3_1[i].neg].subsumed = 1;
	  pClauses[v3_2[y].pos].subsumed = 1;
	  pClauses[v3_2[y].neg].subsumed = 1;
	  
	  if(write_gates_flag) {
	    //Found ITE=. Printing it for demonstration purposes.
	    fprintf(foutputfile, "equ(%jd, ite(%jd, %jd, %jd))\n", x,
		    pClauses[v3_1[i].pos].variables[v3_1[i].v0],
		    -pClauses[v3_2[y].pos].variables[v3_2[y].v1],
		    -pClauses[v3_1[i].pos].variables[v3_1[i].v1]);
	  }
	  ret = add_ite_equals_BDD(BM, x, pClauses[v3_1[i].pos].variables[v3_1[i].v0],
				   -pClauses[v3_2[y].pos].variables[v3_2[y].v1],
				   -pClauses[v3_1[i].pos].variables[v3_1[i].v1]);
	  if(ret != NO_ERROR) return ret;
	  
	  //Marking clauses used in the ITE= so that they will be removed.
	  for(uintmax_t z = 0; z < three_pos[x].length; z++) {
	    count = 0;
	    for(uint8_t j = 0; j < 3; j++) {
	      if((-pClauses[three_pos[x].num[z]].variables[j] == -pClauses[v3_2[y].pos].variables[v3_2[y].v1])
		 ||(-pClauses[three_pos[x].num[z]].variables[j] == -pClauses[v3_1[i].pos].variables[v3_1[i].v1]))
		count++;
	    }
	    if(count == 2)
	      pClauses[three_pos[x].num[z]].subsumed = 1;
	  }
	  for(uintmax_t z = 0; z < three_neg[x].length; z++)	{
	    count = 0;
	    for(uint8_t j = 0; j < 3; j++) {
	      if((pClauses[three_neg[x].num[z]].variables[j] == -pClauses[v3_2[y].pos].variables[v3_2[y].v1])
		 ||(pClauses[three_neg[x].num[z]].variables[j] == -pClauses[v3_1[i].pos].variables[v3_1[i].v1]))
		count++;
	    }
	    if(count == 2) pClauses[three_neg[x].num[z]].subsumed = 1;
	  }
	  num_iteequals_found++;
	  out = 1;
	}
      }
    }
  }
  for(uintmax_t x = 1; x < nNumVariables+1; x++) {
    sbsat_free((void **)&three_pos[x].num);
    sbsat_free((void **)&three_neg[x].num);
  }
  sbsat_free((void **)&three_pos);
  sbsat_free((void **)&three_neg);
  sbsat_free((void **)&v3_1);
  sbsat_free((void **)&v3_2);
  
  d2_printf2("Found %ju ITE= functions\n", num_iteequals_found);
  sbsat_stats[STAT_NUM_ITE_EQUALS] = num_iteequals_found;

  return ret;
}

uint8_t pattern_majv_equals(Clause clause, intmax_t order[4]) {
  //	fprintf(stderr, "c[%jd %jd %jd]\n", clause.variables[0],clause.variables[1],clause.variables[2]);
  if((clause.variables[0] == order[0] &&
      clause.variables[1] == order[1] &&
      clause.variables[2] == order[2]) ||
     (clause.variables[0] == order[0] &&
      clause.variables[1] == order[1] &&
      clause.variables[2] == order[3]) ||
     (clause.variables[0] == order[0] &&
      clause.variables[1] == order[2] &&
      clause.variables[2] == order[3]) ||
     (clause.variables[0] == order[1] &&
      clause.variables[1] == order[2] &&
      clause.variables[2] == order[3]))
    return 1;
  return 0;
}

uint8_t find_and_add_majvequals(CNF_Struct *CNF, BDDManager *BM) {
  uint8_t ret = NO_ERROR;
  
  uintmax_t nNumClauses = CNF->nNumClauses;
  uintmax_t nNumVariables = CNF->nNumVariables;
  Clause *pClauses = CNF->pClauses;
  
  uintmax_t num_majvequals_found = 0;
  
  uintmax_t *threepos_temp = (uintmax_t *)sbsat_calloc(nNumVariables+1, sizeof(uintmax_t), 9, "threepos_temp");
  uintmax_t *threeneg_temp = (uintmax_t *)sbsat_calloc(nNumVariables+1, sizeof(uintmax_t), 9, "threeneg_temp");
  for(uintmax_t x = 0; x < nNumClauses; x++) {
    uintmax_t y = pClauses[x].length;
    if(y == 3) {
      for(y = 0; y < 3; y++) {
	if(pClauses[x].variables[y] > 0)
	  threepos_temp[pClauses[x].variables[y]]++;
	else
	  threeneg_temp[-pClauses[x].variables[y]]++;
      }
    }
  }
  store *three_pos = (store *)sbsat_calloc(nNumVariables+1, sizeof(store), 9, "three_pos");
  store *three_neg = (store *)sbsat_calloc(nNumVariables+1, sizeof(store), 9, "three_neg");
  
  //Store appropriate array sizes to help with memory usage
  for(uintmax_t x = 1; x < nNumVariables+1; x++) {
    three_pos[x].num = (uintmax_t *)sbsat_calloc(threepos_temp[x], sizeof(uintmax_t), 9, "three_pos[x].num");
    three_neg[x].num = (uintmax_t *)sbsat_calloc(threeneg_temp[x], sizeof(uintmax_t), 9, "three_neg[x].num");
  }
  sbsat_free((void **)&threepos_temp);
  sbsat_free((void **)&threeneg_temp);
  
  //Store all clauses with 3 variables so they can be clustered
  uintmax_t count = 0;
  for(uintmax_t x = 0; x < nNumClauses; x++) {
    if(pClauses[x].length == 3) {
      count++;
      for(uint8_t i = 0; i < 3; i++) {
	if(pClauses[x].variables[i] < 0) {
	  three_neg[-pClauses[x].variables[i]].num[three_neg[-pClauses[x].variables[i]].length] = x;
	  three_neg[-pClauses[x].variables[i]].length++;
	} else {
	  three_pos[pClauses[x].variables[i]].num[three_pos[pClauses[x].variables[i]].length] = x;
	  three_pos[pClauses[x].variables[i]].length++;
	}
      }
    } else if(pClauses[x].length > 3) break; //Relies on clauses being sorted
  }
  
  //v0 is in all six clauses
  //When v0 is positive, all other literals are negative
  //When v0 is negative, all other literals are positive
  //v0 = majv(v1, v2, v3)
  //---------------------
  //v0 -v1 -v2
  //v0 -v1 -v3
  //v0 -v2 -v3
  //-v0 v1 v2
  //-v0 v1 v3
  //-v0 v2 v3
  //---------------------
  
  for(uintmax_t v0 = 0; v0 <= nNumVariables; v0++) {
    if (v0%1000 == 1)
      d4_printf3("\rMAJV Search CNF %ju/%ju ... ", v0, nNumVariables);
    
    uint8_t out=0;
    if(three_pos[v0].length < 3 || three_neg[v0].length < 3) continue;
    for(uintmax_t i = 0; i < three_pos[v0].length-2 && !out; i++) {
      uintmax_t c1 = three_pos[v0].num[i]; //clause number 1
      intmax_t order[4]; //To hold the ordering;
      intmax_t v1, v2;
      if(pClauses[c1].variables[0] == (intmax_t)v0) {
	v1 = pClauses[c1].variables[1];
	v2 = pClauses[c1].variables[2];
      } else if(pClauses[c1].variables[1] == (intmax_t)v0) {
	v1 = pClauses[c1].variables[0];
	v2 = pClauses[c1].variables[2];
      } else { 
	assert(pClauses[c1].variables[2] == (intmax_t)v0);
	v1 = pClauses[c1].variables[0];
	v2 = pClauses[c1].variables[1];
      }
      
      for(uintmax_t j = i+1; j < three_pos[v0].length-1 && !out; j++) {
	uintmax_t c2 = three_pos[v0].num[j]; //clause number 2
	intmax_t v3=0;
	if(pClauses[c2].variables[0] == (intmax_t)v0) {
	  if(pClauses[c2].variables[1] == v1)
	    v3 = pClauses[c2].variables[2];
	  else if(pClauses[c2].variables[2] == v1)
	    v3 = pClauses[c2].variables[1];
	  else if(pClauses[c2].variables[1] == v2)
	    v3 = pClauses[c2].variables[2];
	  else if(pClauses[c2].variables[2] == v2)
	    v3 = pClauses[c2].variables[1];
	  else continue;
	} else if(pClauses[c2].variables[1] == (intmax_t)v0) {
	  if(pClauses[c2].variables[0] == v1)
	    v3 = pClauses[c2].variables[2];
	  else if(pClauses[c2].variables[2] == v1)
	    v3 = pClauses[c2].variables[0];
	  else if(pClauses[c2].variables[0] == v2)
	    v3 = pClauses[c2].variables[2];
	  else if(pClauses[c2].variables[2] == v2)
	    v3 = pClauses[c2].variables[0];
	  else continue;					
	} else {
	  assert(pClauses[c2].variables[2] == (intmax_t)v0);
	  if(pClauses[c2].variables[0] == v1)
	    v3 = pClauses[c2].variables[1];
	  else if(pClauses[c2].variables[1] == v1)
	    v3 = pClauses[c2].variables[0];
	  else if(pClauses[c2].variables[0] == v2)
	    v3 = pClauses[c2].variables[1];
	  else if(pClauses[c2].variables[1] == v2)
	    v3 = pClauses[c2].variables[0];
	  else continue;					
	}
	order[0]=v0; order[1]=v1; order[2]=v2; order[3]=v3;
	qsort(order, 4, sizeof(uintmax_t), abscompfunc);
	if(imaxabs(order[0]) == imaxabs(order[1]) || imaxabs(order[1]) == imaxabs(order[2]) || imaxabs(order[2]) == imaxabs(order[3]))
	  continue;
	//				fprintf(stderr, "[%jd %jd %jd %jd]", order[0], order[1], order[2], order[3]);
	
	uint8_t clause_found=0;
	uintmax_t c3;
	for(uintmax_t k = j+1; k < three_pos[v0].length; k++) {
	  c3 = three_pos[v0].num[k]; //clause number 3
	  if(pattern_majv_equals(pClauses[c3], order)) {
	    clause_found = 1;
	    break;
	  }
	}
	if(!clause_found) continue;
	
	order[0]=-order[0]; order[1]=-order[1]; order[2]=-order[2]; order[3]=-order[3];
	
	clause_found=0;
	uintmax_t c4;
	uintmax_t k1;
	for(k1 = 0; k1 < three_neg[v0].length-2; k1++) {
	  c4 = three_neg[v0].num[k1]; //clause number 4
	  if(pattern_majv_equals(pClauses[c4], order)) {
	    clause_found = 1;
	    break;
	  }
	}
	if(!clause_found) continue;
	
	clause_found=0;
	uintmax_t c5;
	uintmax_t k2;
	for(k2 = k1+1; k2 < three_neg[v0].length-1; k2++) {
	  c5 = three_neg[v0].num[k2]; //clause number 5
	  if(pattern_majv_equals(pClauses[c5], order)) {
	    clause_found = 1;
	    break;
	  }
	}
	if(!clause_found) continue;
	
	clause_found=0;
	uintmax_t c6;
	for(uintmax_t k3 = k2+1; k3 < three_neg[v0].length; k3++) {
	  c6 = three_neg[v0].num[k3]; //clause number 6
	  if(pattern_majv_equals(pClauses[c6], order)) {
	    clause_found = 1;
	    break;
	  }
	}
	if(!clause_found) continue;
	
	if(write_gates_flag) {
	  //Found MAJV=. Printing it for demonstration purposes.
	  fprintf(foutputfile, "equ(%jd, majv(%jd, %jd, %jd))\n", v0, v1, v2, v3);
	}
	
	ret = add_majv_equals_BDD(BM, v0, v1, v2, v3);
	if(ret != NO_ERROR) return ret;
        
	//Marking clauses used in the MAJV= so that they will be removed.
	pClauses[c1].subsumed = 1;
	pClauses[c2].subsumed = 1;
	pClauses[c3].subsumed = 1;
	pClauses[c4].subsumed = 1;
	pClauses[c5].subsumed = 1;
	pClauses[c6].subsumed = 1;
	num_majvequals_found++;
	out=1;
      }
    }
  }
  for(uintmax_t x = 1; x < nNumVariables+1; x++) {
    sbsat_free((void **)&three_pos[x].num);
    sbsat_free((void **)&three_neg[x].num);
  }
  sbsat_free((void **)&three_pos);
  sbsat_free((void **)&three_neg);
  
  d2_printf2("\rFound %ju MAJV= functions             \n", num_majvequals_found);
  sbsat_stats[STAT_NUM_MAJV_EQUALS] = num_majvequals_found;
  
  return ret;
}

uint8_t new_clustering = 0;

uint8_t find_and_add_gates(CNF_Struct *CNF, BDDManager *BM, uintmax_t overlap, uintmax_t max_length) {
  uint8_t ret = NO_ERROR;
  
  if(BM==NULL) return ret;
  
  uintmax_t nNumClauses = CNF->nNumClauses;
  uintmax_t nNumVariables = CNF->nNumVariables;
  Clause *pClauses = CNF->pClauses;
  
  if(nNumClauses == 0) return ret;

  DdNode *clause;
  DdNode *bTemp;
  
  uintmax_t *occurrence_temp = (uintmax_t *)sbsat_calloc(nNumVariables+1, sizeof(uintmax_t), 9, "occurrence_temp");
  
  for(uintmax_t x = 0; x < nNumClauses; x++) {
    if(pClauses[x].length <= 1 || pClauses[x].length > max_length) continue;
    for(uintmax_t y = 0; y < pClauses[x].length; y++) {        
      occurrence_temp[imaxabs(pClauses[x].variables[y])]++;
    }
  }
  
  store *occurrence = (store *)sbsat_calloc(nNumVariables+1, sizeof(store), 9, "occurrence");
  
  for(uintmax_t x = 1; x <= nNumVariables; x++) 
    occurrence[x].num = (uintmax_t *)sbsat_calloc(occurrence_temp[x], sizeof(uintmax_t), 9, "occurrence[x].num");
  
  sbsat_free((void **)&occurrence_temp);
  
  for(uintmax_t x = 0; x < nNumClauses; x++) {
    if(pClauses[x].length <= 1 || pClauses[x].length > max_length) continue;
    for(uintmax_t y = 0; y < pClauses[x].length; y++) {
      uintmax_t v = imaxabs(pClauses[x].variables[y]);
      occurrence[v].num[occurrence[v].length] = x;
      occurrence[v].length++;
    }
  }
  
  uint8_t *seen_before = (uint8_t *)sbsat_calloc(nNumClauses, sizeof(uint8_t), 9, "seen_before");
  uint8_t_undoer seen_before_undoer;
  uint8_t_undoer_alloc(&seen_before_undoer, 100, 100);

  uintmax_t gates_found = 0;
  
  for(uintmax_t x = nNumClauses-1; x > 0; x--) {
    if ((nNumClauses-x)%10000 == 1) {
      d2_printf3("\rSearching for gates related to clause %ju/%ju ... ", nNumClauses-x-1, nNumClauses);
      //			fprintf(stderr, "\n%d\n", cuddGarbageCollect(BM->dd, 1));
    }
    
    if(pClauses[x].subsumed == 1) continue;
    if(pClauses[x].length <= 1) continue;
    if(pClauses[x].length > max_length) continue;

    if(new_clustering) {
      ret = compress_BDD_manager(BM);
      if(ret != NO_ERROR) return ret;
    }

    //fprintf(stderr, "%ju -A \n", x);
    
    uint8_t found_a_gate = 0;
    uintmax_t bdd_num = 0;

    uint8_t_undoer_push_marker(&seen_before_undoer);
    
    for(uintmax_t y = 0; y < pClauses[x].length; y++) {
      uintmax_t v = imaxabs(pClauses[x].variables[y]);
      //fprintf(stderr, "--B %ju\n", v);

      for(uintmax_t z = 0; z < occurrence[v].length; z++) {
	uintmax_t i = occurrence[v].num[z];
	if(x <= i) break;
        
	if(pClauses[i].subsumed == 1) continue;
	if(pClauses[i].length <= 1) continue; //Skip over inferences - no reason to cluster these because they will be applied globally later.
	if(pClauses[i].length > (pClauses[x].length + overlap)) continue;

	//fprintf(stderr, "----C %ju\n", i);

	if(seen_before[i] == 1) continue;
	uint8_t_undoer_push(&seen_before_undoer, seen_before+i);
	seen_before[i] = 1;
        
	uintmax_t out = 0;
	uintmax_t x_pos = 0;
	uintmax_t i_pos = 0;
	while((out<=overlap) && (x_pos < pClauses[x].length) && (i_pos < pClauses[i].length)) {
	  if(imaxabs(pClauses[x].variables[x_pos]) == imaxabs(pClauses[i].variables[i_pos]))
	    { x_pos++; i_pos++; }
	  else if(imaxabs(pClauses[x].variables[x_pos]) < imaxabs(pClauses[i].variables[i_pos]))
	    { x_pos++; }
	  else { i_pos++; out++; }
        }
	out += pClauses[i].length - i_pos;
	if(out<=overlap) {
	  //Clustering BDD i into BDD x
	  //d2_printf3("c:%ju %ju\n", x, i);
	  //write_clause(CNF, x);
	  //write_clause(CNF, i);

	  if(found_a_gate == 0) {
	    ret = add_or_BDD(BM, pClauses[x].variables, pClauses[x].length);
	    if(ret != NO_ERROR) return ret;
	    pClauses[x].subsumed = 1;
	    bdd_num = BM->nNumBDDs-1; //Invariant, BDDs are always added to the end of the BDD list
	  }

	  Cudd_AutodynDisable(BM->dd);
	  clause = build_or_BDD(BM->dd, pClauses[i].variables, pClauses[i].length);
	  cuddRef(clause);
	  
	  unsigned int limit = 10000;
	  clause = Cudd_bddAndLimit(BM->dd, BM->BDDList[bdd_num], bTemp = clause, limit);
	  
	  if(clause != NULL) { //If limit not hit
	    ret = replace_BDD_in_manager(BM, clause, bdd_num);
	    pClauses[i].subsumed = 1;
	    found_a_gate=1;
	  }
	  
	  Cudd_IterDerefBdd(BM->dd, bTemp);
	  Cudd_AutodynEnable(BM->dd, CUDD_REORDER_SAME);
	  if(ret != NO_ERROR) return ret;
	}
	if(found_a_gate && BM->Support[bdd_num].nLength > max_length) break;
      }
      if(found_a_gate && BM->Support[bdd_num].nLength > max_length) break;
    }
    if(found_a_gate==1) gates_found+=1;

    uint8_t_undoer_undo_to_last_marker(&seen_before_undoer);

  }
  
  d2_printf3("\rSearching for gates related to clause %ju/%ju            \n", nNumClauses, nNumClauses);
  
  uint8_t_undoer_free(&seen_before_undoer);
  sbsat_free((void **)&seen_before);
  
  for(uintmax_t x = 1; x <= nNumVariables; x++) {
    sbsat_free((void **)&occurrence[x].num);
  }

  sbsat_free((void **)&occurrence);
 
  d2_printf2("\rFound %ju unknown functions             \n", gates_found);
  sbsat_stats[STAT_NUM_UNKNOWN_CLUSTERS] = gates_found;
  
  return ret;
}

uint8_t add_all_clause_BDDs(CNF_Struct *CNF, BDDManager *BM) {
  uint8_t ret = NO_ERROR;
  
  if(BM==NULL) return ret;
  
  uintmax_t num_single_literal_clauses = 0;

  for(uintmax_t x = 0; x < CNF->nNumClauses; x++) {
    if (x%10000 == 0)
      d2_printf3("\rAdding BDD for clause %ju/%ju ... ", x, CNF->nNumClauses);
    
    if(CNF->pClauses[x].subsumed == 1) continue;
    
    if(CNF->pClauses[x].length == 1) num_single_literal_clauses++;
    
    ret = add_or_BDD(BM, CNF->pClauses[x].variables, CNF->pClauses[x].length);
    if(ret != NO_ERROR) return ret;
    
    CNF->pClauses[x].subsumed = 1;
  }
  
  d2_printf4("\rAdding BDD for clause %ju/%ju (%ju single literal clauses)\n", CNF->nNumClauses, CNF->nNumClauses, num_single_literal_clauses);
  sbsat_stats[STAT_NUM_UNITS] = num_single_literal_clauses;
  sbsat_stats[STAT_NUM_CLAUSES] = CNF->nNumClauses - num_single_literal_clauses;
  
  return ret;
}

uint8_t process_cnf(CNF_Struct *CNF, BDDManager *BM) {
  uint8_t ret = NO_ERROR;
  
  uintmax_t old_nNumClauses = CNF->nNumClauses;

  create_all_syms(0, CNF->nNumVariables+1);
  
  //Sort variables in each clause
  for(uintmax_t x = 0; x < CNF->nNumClauses; x++)
    qsort(CNF->pClauses[x].variables, CNF->pClauses[x].length, sizeof(intmax_t), abscompfunc);
  
  //Sort Clauses
  qsort(CNF->pClauses, CNF->nNumClauses, sizeof(Clause), clscompfunc);

  ret = remove_dup_lits(CNF);
  if(ret != NO_ERROR) return ret;
  
  if(sbsat_turn_off_bdds==0 && Cudd_bddIthVar(BM->dd, CNF->nNumVariables) == NULL) {
    return MEM_ERR;
  }
  
  ret = reduce_clauses(CNF, BM);
  if(ret != NO_ERROR) return ret;

  if(find_cnf_gates_flag) {
    
    ret = find_and_add_xors(CNF, BM);
    if(ret != NO_ERROR) return ret;
    
    //ret = reduce_clauses(CNF, BM);
    //if(ret != NO_ERROR) return ret;
    
    ret = find_and_add_andequals(CNF, BM);
    if(ret != NO_ERROR) return ret;
    
    ret = reduce_clauses(CNF, BM);
    if(ret != NO_ERROR) return ret;
    
    //ret = find_and_add_majvequals(CNF, BM); //I think this is buggy - sat-2010-race/hardware-verification/velev/12pipe_q0_k.cnf.gz
    //if(ret != NO_ERROR) return ret;
    
    //ret = reduce_clauses(CNF, BM);
    //if(ret != NO_ERROR) return ret;
    
    //ret = find_and_add_iteequals(CNF, BM); //I think this is buggy - sat-2010-race/hardware-verification/velev/12pipe_q0_k.cnf.gz
    //if(ret != NO_ERROR) return ret;
    
    //ret = reduce_clauses(CNF, BM);
    //if(ret != NO_ERROR) return ret;
    
    if(cluster_cnf_flag) {
      ret = find_and_add_gates(CNF, BM, clause_cluster_overlap, clause_cluster_max_length);
      if(ret != NO_ERROR) return ret;
      
      ret = reduce_clauses(CNF, BM);
      if(ret != NO_ERROR) return ret;
    }
  }

  if(write_cnf_flag) {
    //Write the left over clauses
    ret = write_cnf(CNF);
    if(ret != NO_ERROR) return ret;
  }

  ret = add_all_clause_BDDs(CNF, BM);
  if(ret != NO_ERROR) return ret;
  
  ret = reduce_clauses(CNF, BM);
  if(ret != NO_ERROR) return ret;
  
  d2_printf2("Number of Variables - %ju\n", CNF->nNumVariables);
  d2_printf2("Compression = %4.2f\n", (double)old_nNumClauses/(double)BM->nNumBDDs);
  d2_printf2("Mem usage = %juM\n", get_memusage()/1000000);

  sbsat_stats[STAT_RAM_USAGE_AFTER_PROCESSING_CNF] = get_memusage()/1000000;
  sbsat_stats[STAT_BDD_NODES_AFTER_PROCESSING_CNF] = BM->dd->keys;
  sbsat_stats[STAT_NUM_BDDS_AFTER_PROCESSING_CNF] = BM->nNumBDDs;

  sbsat_stats_f[STAT_F_CNF_TOTAL_TIME] = get_runtime();
  sbsat_stats_f[STAT_F_CNF_COMPRESSION] = (double)sbsat_stats[STAT_CNF_NUM_CLAUSES] / (double)BM->nNumBDDs;
  sbsat_stats_f[STAT_F_CNF_PROCESS_TIME] = sbsat_stats_f[STAT_F_CNF_TOTAL_TIME] - sbsat_stats_f[STAT_F_CNF_READ_TIME];

  return ret;
}
