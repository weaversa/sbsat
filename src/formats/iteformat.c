/*
 *    Copyright (C) 2010 Sean Weaver
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
 *
*/

#include "sbsat.h"

char getNextIteSymbol (BDDManager *BM, intmax_t *intnum, DdNode **bdd);

int ite_line;

uintmax_t nNumVariables;
uintmax_t nNumBDDs;

typedef struct ite_defines_struct {
  char *string;
  DdNode *bdd;
  int *vlist;
} ite_defines_struct;

ite_defines_struct *ite_defines;

typedef struct initbranch_level {
  char *string;
  double true_inf_weight;
} initbranch_level;

typedef struct initbranch_struct {
  int branch_level;
  int max_initbranch_level;
  int num_initbranch_level;
  initbranch_level *vars;
} initbranch_struct;

initbranch_struct *initbranch;

int max_initbranch;
unsigned int max_varlevel;

double *arrVarTrueInfluences;
int **arrVarChoiceLevels;
int nVarChoiceLevelsMax;
int nVarChoiceLevelsNum;

int max_defines;
int totaldefines;
int max_integers;
char *integers;
int max_macros;
char *macros;
int negate_it;
int expect_integer = 0;

int initbranch_compfunc (const void *x, const void *y) {
  initbranch_struct pp, qq;
  
  pp = *(const initbranch_struct *) x;
  qq = *(const initbranch_struct *) y;
  if (pp.branch_level < qq.branch_level)
    return -1;
  if (pp.branch_level == qq.branch_level)
    return 0;
  return 1;
}

DdNode *putite(BDDManager *BM, intmax_t intnum, DdNode *bdd) {
  char order = getNextIteSymbol (BM, &intnum, &bdd);
  if (order == 'i') { //This becomes unnecessary
    if(intnum >= max_macros) {
      macros = (char *)sbsat_recalloc(macros, max_macros, intnum+2, sizeof(char), 9, "macros");
      max_macros=intnum+2;
    }
    //sprintf(macros, "%d", intnum);
    //		intnum = i_getsym(macros, SYM_VAR);
    //		Here I need to return an integer if it is actually an integer...
    //		for the defines, and InititalBranch, and others that use integers.
    //		Though I could fix those to use integers correctly......maybe.....
    d5_printf2("%s ", macros);
    if(intnum == 0) {
      Cudd_Ref(Cudd_Not(Cudd_ReadOne(BM->dd)));
      return Cudd_Not(Cudd_ReadOne(BM->dd));
    } else if(intnum < 0) {
      DdNode *ret = Cudd_Not(Cudd_bddIthVar(BM->dd, intnum)); Cudd_Ref(ret);
      return ret;
    } else {
      DdNode *ret = Cudd_bddIthVar(BM->dd, intnum); Cudd_Ref(ret);
      return ret;
    }
  }
  if (order == 'b') {
    Cudd_Ref(bdd);
    return (bdd);
  }
  if (order == 'x')
    return NULL;
  if (order == ')')
    return NULL;
  if (order != 'm') {
    fprintf(stderr, "\nFormula parse error...exiting:%d\n", ite_line);
    //I don't think this can actually happen...
    exit (1);
  }
  if (!strcasecmp (macros, "t")) {
    Cudd_Ref(Cudd_ReadOne(BM->dd));
    return Cudd_ReadOne(BM->dd);
  }
  if (!strcasecmp (macros, "f")) {
    Cudd_Ref(Cudd_Not(Cudd_ReadOne(BM->dd)));
    return Cudd_Not(Cudd_ReadOne(BM->dd));
  }
  
  //Search the defines for this word...
  for (int x = 0; x < totaldefines; x++) {
    if (!strcasecmp (macros, ite_defines[x].string)) {
      DdNode **BDDS = (DdNode **)sbsat_malloc(ite_defines[x].vlist[0] + 1, sizeof(DdNode *), 9, "BDDS");;
      for (int v = 1; v <= ite_defines[x].vlist[0]; v++) {
	BDDS[v] = putite(BM, intnum, bdd);
	if(BDDS[v] == NULL) { fprintf(stderr, "\n#define '%s' requires %d arguments, found %d...exiting:%d\n", ite_defines[x].string, ite_defines[x].vlist[0], v-1, ite_line); exit (1); }
      }
      if(putite(BM, intnum, bdd)!=NULL) { fprintf(stderr, "\n#define '%s' requires %d arguments, found > %d...exiting:%d\n", ite_defines[x].string, ite_defines[x].vlist[0], ite_defines[x].vlist[0], ite_line); exit (1); }
      //SEAN!!! Need to figure this operation out.
      DdNode *ret = Cudd_Not(Cudd_ReadOne(BM->dd));//f_mitosis(ite_defines[x].bdd, BDDS, ite_defines[x].vlist);
      Cudd_Ref(ret);
      //			BDDNode * returnbdd = f_apply (ite_defines[x].bdd, BDDS);
      sbsat_free((void **)&BDDS);
      strcpy (macros, ite_defines[x].string);
      return ret;
    }
  } //Must be a variable or a reserved word...
  if (!strcasecmp (macros, "ite")) {
    DdNode * v1, *v2, *v3, *ret;
    v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'ite' expects 3 arguments, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    v2 = putite(BM, intnum, bdd);
    if(v2 == NULL) { fprintf(stderr, "\nKeyword 'ite' expects 3 arguments, found 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    v3 = putite(BM, intnum, bdd);
    if(v3 == NULL) { fprintf(stderr, "\nKeyword 'ite' expects 3 arguments, found 2 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    if(putite(BM, intnum, bdd) != NULL) { fprintf(stderr, "\nKeyword 'ite' expects 3 arguments, found > 3 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    strcpy (macros, "ite");
    ret = Cudd_bddIte (BM->dd, v1, v2, v3);
    Cudd_Ref(ret);
    Cudd_RecursiveDeref(BM->dd, v1);
    Cudd_RecursiveDeref(BM->dd, v2);
    Cudd_RecursiveDeref(BM->dd, v3);
    return ret;
  }
  if (!strcasecmp (macros, "var")) {
    DdNode *v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'var' expects 1 argument, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    if(putite(BM, intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'var' expects 1 argument, found > 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    strcpy (macros, "var");
    return v1;
  }
  if (!strcasecmp (macros, "not")) {
    DdNode *v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'not' expects 1 argument, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    if(putite(BM, intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'not' expects 1 argument, found > 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    strcpy (macros, "not");
    DdNode *ret = Cudd_Not(v1);
    Cudd_Ref(ret);
    Cudd_RecursiveDeref(BM->dd, v1);
    return ret;
  }
  if (!strcasecmp (macros, "initial_branch")) {
    char p = ' ';
    int started_words = 0;
    int branch_level = 0;
    int found_level = 0;
    
    while (p != ')') {
      p = fgetc(finputfile);
      if (p == '\n') { ite_line++; }
      if (p == ';') {
	while (p != '\n') {
	  p = fgetc(finputfile);
	  if (p == EOF) {
	    fprintf(stderr, "\nUnexpected EOF (%s)...exiting:%d\n", macros, ite_line);
	    exit (1);
	  }
	}
	ite_line++;
      }
      if (p == EOF)	{
	fprintf(stderr, "\nUnexpected EOF (%s)...exiting:%d\n",	macros, ite_line);
	exit (1);
      }
      int found_word = 0;
      int i = 0;
      macros[i++] = '^';
      while (((p >= 'a') && (p <= 'z')) || ((p >= 'A') && (p <= 'Z'))
	     || (p == '_') || ((p >= '0') && (p <= '9')) || (p == '[')
	     || (p == ']') || (p == '*') || (p=='?') || (p=='.') || (p=='^')) {
	started_words = 1;
	found_word = 1;
	if(p == '*') macros[i++] = '.';
	macros[i] = p;
	i++;
	if(i >= max_macros) {
	  macros = (char *)sbsat_recalloc(macros, max_macros, max_macros+10, sizeof(char), 9, "macros");
	  max_macros+=10;
	}
	p = fgetc(finputfile);
	if (p == EOF) {
	  fprintf(stderr, "\nUnexpected EOF (%s)...exiting:%d\n",	macros, ite_line);
	  exit (1);
	}
      }
      if(found_word == 1) {
	macros[i] = '$';
	macros[i+1] = 0;
	initbranch[branch_level].vars[initbranch[branch_level].num_initbranch_level].string = (char *)calloc(i+2, sizeof(char));
	strcpy(initbranch[branch_level].vars[initbranch[branch_level].num_initbranch_level].string, macros);
	d5_printf2(" %s", initbranch[branch_level].vars[initbranch[branch_level].num_initbranch_level].string);
	if(p == '%') {
	  d5_printf1("%%");
	  expect_integer = 1;
	  DdNode *v1 = putite(BM, intnum, bdd);
	  if(v1 == NULL) { fprintf(stderr, "\nKeyword 'initial_branch' needs a number between 0 and 100 after a '%%' (%s)...exiting:%d\n", macros, ite_line); exit (1);}
	  
	  expect_integer = 0;
	  double tweight = 0;
	  if(v1 != Cudd_Not(Cudd_ReadOne(BM->dd))) {
	    if (v1 != Cudd_bddIthVar(BM->dd, v1->index)) {
	      fprintf(stderr, "\nKeyword 'initial_branch' needs a number between 0 and 100 after a '%%' (%s)...exiting:%d\n", macros, ite_line);
	      exit (1);
	    }
	    if (v1->index > 100) {
	      fprintf(stderr, "\nKeyword 'initial_branch' needs a number between 0 and 100 after a '%%' (%s)...exiting:%d\n", macros, ite_line);
	      exit (1);
	    }
	    tweight = ((double)v1->index)/100.0;
	    Cudd_RecursiveDeref(BM->dd, v1);
	  }
	  d6_printf2(" %f  ", tweight);
	  p = fgetc(finputfile);
	  if(p == EOF) {
	    fprintf(stderr, "\nUnexpected EOF (%s)...exiting:%d\n", macros, ite_line);
	    exit(1);
	  }
	  if(p == '.') {
	    d5_printf1("\b.");
	    expect_integer = 1;
	    DdNode *v1 = putite(BM, intnum, bdd);
	    expect_integer = 0;
	    double pweight = 0;
	    if(v1!=Cudd_Not(Cudd_ReadOne(BM->dd))) {
	      if(v1 == NULL) { fprintf(stderr, "\nKeyword 'initial_branch' needs a number between 0 and 100 after a '%%' (%s)...exiting:%d\n", macros, ite_line); exit(1); }
	      
	      if(v1 != Cudd_bddIthVar(BM->dd, v1->index)) {
		fprintf(stderr, "\nKeyword 'initial_branch' needs a number between 0 and 100 after a '%%' (%s)...exiting:%d\n", macros, ite_line);
		exit(1);
	      }
	      pweight = (double)v1->index;
	      Cudd_RecursiveDeref(BM->dd, v1);
	      for(unsigned int dec = 0; dec<strlen(macros)/*floor(pweight)!=0*/; dec++) {
		pweight=pweight/10.0;
	      }
	      pweight=pweight/100.0;                                                                                          
	    }
	    tweight+=pweight;
	    d6_printf2(" %f ", tweight);
	    if(tweight > 100) { fprintf(stderr, "\nKeyword 'initial_branch' needs a number between 0 and 100 after a '%%' (%s)...exiting:%d\n", macros, ite_line); exit(1); }
	  } else {
	    ungetc(p, finputfile);
	  }
	  initbranch[branch_level].vars[initbranch[branch_level].num_initbranch_level].true_inf_weight = tweight;
	} else {
	  initbranch[branch_level].vars[initbranch[branch_level].num_initbranch_level].true_inf_weight = 0.5;
	}
	
	initbranch[branch_level].num_initbranch_level++;
	if(initbranch[branch_level].num_initbranch_level >= initbranch[branch_level].max_initbranch_level) {
	  initbranch[branch_level].vars = (initbranch_level *)sbsat_recalloc(initbranch[branch_level].vars, initbranch[branch_level].max_initbranch_level, initbranch[branch_level].max_initbranch_level+10, sizeof(initbranch_level), 9, "initbranch[branch_level].vars");
	  initbranch[branch_level].max_initbranch_level+=10;
	}
      } else if(p == '%') {
	fprintf(stderr, "\nKeyword 'initial_branch' needs '%%' to follow directly after a variable (%s)...exiting:%d\n", macros, ite_line);
	exit (1);
      }
      if(p == '#') {
	d5_printf1("#");
	if(found_level == 1) {
	  fprintf(stderr, "\nKeyword 'initial_branch' can only have one #level associated with it, found multiple (%s)...exiting:%d\n", macros, ite_line);
	  exit (1);
	} else if(started_words == 1) {
	  fprintf(stderr, "\nKeyword 'initial_branch' needs #level to be the first argument (%s)...exiting:%d\n", macros, ite_line);
	  exit (1);
	} else {
	  found_level = 1;  
	  
	  expect_integer = 1;
	  DdNode *v1 = putite(BM, intnum, bdd);
	  if(v1 == NULL) { fprintf(stderr, "\nKeyword 'initial_branch' needs a positive integer as a level after '#' (%s)...exiting:%d\n", macros, ite_line); exit (1); }
	  expect_integer = 0;
	  if (v1 != Cudd_bddIthVar(BM->dd, v1->index)) {
	    fprintf(stderr, "\nKeyword 'initial_branch' needs a positive integer as a level after '#' (%s)...exiting:%d\n", macros, ite_line);
	    exit (1);
	  }
	  if(v1->index > 10000) {
	    fprintf(stderr, "\nKeyword 'initial_branch' needs a positive integer < 10000 as a level after '#' (%s)...exiting:%d\n", macros, ite_line);
	    exit(1);
	  }
	  if(v1->index >= max_varlevel) max_varlevel = v1->index+1;
	  //search for branch_level in initbranch
	  branch_level = -1;
	  for(int i = 0; i < max_initbranch; i++) {
	    if(initbranch[i].branch_level == (int)v1->index) {
	      branch_level = i;
	      break;
	    }
	  }
	  if(branch_level == -1) {
	    for(int i = 0; i < max_initbranch; i++) {
	      if(initbranch[i].branch_level == 0) {
		initbranch[i].branch_level = v1->index;
		branch_level = i;
		break;
	      }
	    }
	  }
	  if(branch_level == -1) {
	    //Create a new initbranch[branch_level]
	    initbranch = (initbranch_struct *)sbsat_recalloc(initbranch, max_initbranch, max_initbranch+1, sizeof(initbranch_struct), 9, "initbranch");
	    branch_level = max_initbranch;
	    max_initbranch++;
	    initbranch[branch_level].branch_level = v1->index;
	    initbranch[branch_level].max_initbranch_level = 10;
	    initbranch[branch_level].num_initbranch_level = 0;
	    initbranch[branch_level].vars = (initbranch_level *)sbsat_recalloc(initbranch[branch_level].vars, 0, initbranch[branch_level].max_initbranch_level, sizeof(initbranch_level), 9, "initbranch[0].vars");
	  }
	  Cudd_RecursiveDeref(BM->dd, v1);
	}
      }
    }
    //ungetc(p, finputfile);
    strcpy (macros, "initial_branch");
    Cudd_Ref(Cudd_ReadOne(BM->dd));
    return Cudd_ReadOne(BM->dd);
  }
  if (!strcasecmp (macros, "define")) {
    if (getNextIteSymbol (BM, &intnum, &bdd) != 'm') {
      fprintf(stderr, "\nUnnamed #define...exiting:%d\n", ite_line);
      exit (1);
    }
    if ((!strcasecmp (macros, "initial_branch")) || (!strcasecmp (macros, "define")) ||
	(!strcasecmp(macros, "t")) || (!strcasecmp(macros, "f")) ||
	(!strcasecmp(macros, "ite")) || (!strcasecmp(macros, "var")) ||
	(!strcasecmp(macros, "not")) || (!strcasecmp(macros, "order")) ||
	(!strcasecmp(macros, "truth_table")) || (!strcasecmp(macros, "mitosis")) ||
	(!strcasecmp(macros, "minmax")) || (!strcasecmp(macros, "exist")) ||
	(!strcasecmp(macros, "universe")) || (!strcasecmp(macros, "safe")) ||
	(!strcasecmp(macros, "safe_eq")) || (!strcasecmp(macros, "safe_func")) ||
	(!strcasecmp(macros, "remove_fps")) || (!strcasecmp(macros, "print_tree")) ||
	(!strcasecmp(macros, "pprint_tree")) || (!strcasecmp(macros, "print_xdd")) ||
	(!strcasecmp(macros, "print_flat_xdd")) || (!strcasecmp(macros, "print_dot_stdout")) ||
	(!strcasecmp(macros, "print_dot")) || (!strcasecmp(macros, "gcf")) ||
	(!strcasecmp(macros, "prune")) || (!strcasecmp(macros, "restrict")) ||
	(!strcasecmp(macros, "strengthen")) || (!strcasecmp(macros, "nimp")) ||
	(!strcasecmp(macros, "nand")) || (!strcasecmp(macros, "nor")) ||
	(!strcasecmp(macros, "equ")) || (!strcasecmp(macros, "imp")) ||
	(!strcasecmp(macros, "xnor")) || (!strcasecmp(macros, "noxor")) ||
	(!strcasecmp(macros, "countT")) || (!strcasecmp(macros, "and")) ||
	(!strcasecmp(macros, "xor")) || (!strcasecmp(macros, "or")) ||
	(!strcasecmp(macros, "gelim_preload_file")) || (!strcasecmp(macros, "same")) ||
	(!strcasecmp(macros, "print_smurf_dot_stdout"))) {
      fprintf(stderr, "\n%s is a reserved word...exiting:%d\n", macros, ite_line);
      exit (1);
    }
    int whereat;
    for (whereat = 0; whereat < totaldefines; whereat++) {
      if (!strcasecmp (macros, ite_defines[whereat].string))
	break;
    }
    
    if(whereat==totaldefines) {
      ite_defines[whereat].string = (char *)sbsat_calloc(strlen(macros)+1, sizeof(char), 9, "ite_defines[].string");
      strcpy (ite_defines[whereat].string, macros);
    }
    
    //      int x;
    //      for(x = 0; macros[x]!=0; x++)
    //        ite_defines[whereat].string[x] = macros[x];
    //      ite_defines[whereat].string[x] = 0;      
    // 
    d4_printf2("#define %s ", ite_defines[whereat].string);
    int v = 0;
    order = getNextIteSymbol (BM, &intnum, &bdd);
    if(order == ')') { fprintf(stderr, "\nKeyword 'define' missing argument before '#' (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    
    int max_vlist = 0;
    ite_defines[whereat].vlist = (int *)sbsat_recalloc(NULL, max_vlist, max_vlist+10, sizeof(int), 9, "ite_defines[whereat].vlist");
    max_vlist+=10;
    
    while (order != '#') {
      if (order == ')') {
	order = getNextIteSymbol (BM, &intnum, &bdd);
	if(order != '#') { fprintf(stderr, "\nProblem with #define %s (%s)...exiting:%d\n", ite_defines[whereat].string, macros, ite_line); exit (1); }
	continue;
      }
      if (order != 'm') { fprintf(stderr, "\nProblem with #define %s (%s)...exiting:%d\n", ite_defines[whereat].string, macros, ite_line); exit (1); }
      v++;
      intnum = i_getsym(macros, SYM_VAR);
      
      if(intnum > (int)nNumVariables) {
	fprintf(stderr, "\nToo many symbols used (%s). Need to increase to greater than %ju...exiting:%d\n", macros, nNumVariables, ite_line);
	exit (1);
      }
      
      for(int iter = 1; iter < v; iter++)
	if(ite_defines[whereat].vlist[iter] == intnum) {
	  fprintf(stderr, "\nCannot use the same variable (%s) twice in argument list in #define %s...exiting:%d\n", getsym_i(intnum)->name, ite_defines[whereat].string, ite_line);
	  exit(1);
	}
      if(v >= max_vlist) {
	ite_defines[whereat].vlist = (int *)sbsat_recalloc(ite_defines[whereat].vlist, max_vlist, max_vlist+10, sizeof(int), 9, "ite_defines[whereat].vlist");
	max_vlist+=10;
      }
      ite_defines[whereat].vlist[v] = intnum;
      order = getNextIteSymbol (BM, &intnum, &bdd);
    }
    ite_defines[whereat].vlist[0] = v;
    ite_defines[whereat].bdd = putite(BM, intnum, bdd);
    if(ite_defines[whereat].bdd == NULL) { fprintf(stderr, "\nKeyword 'define' missing argument after '#' (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    
    uintmax_t *tempint=NULL;
    uintmax_t y = 0;
    uintmax_t tempint_max = 0;
    BDD_Support(&y, &tempint_max, &tempint, ite_defines[whereat].bdd);
    if (y != 0) qsort (tempint, y, sizeof (int), compfunc);
    
    for (uintmax_t i = 0; i < y; i++) {
      uintmax_t found = 0;
      for(int b = 1; b <= ite_defines[whereat].vlist[0]; b++) {
	if((int)tempint[i] == ite_defines[whereat].vlist[b]) 
	  { found = 1; break; }
      }
      if(found == 0) {
	fprintf(stderr, "\nVariable %s is not in argument list for #define %s...exiting:%d\n", getsym_i(tempint[i])->name, ite_defines[whereat].string, ite_line);
	exit(1);
      }
    }
    
    //below: could compare the lengths and just say, "unused variables in the argument list"
    //but that's not as descriptive
    
    for (int b = 1; b <= ite_defines[whereat].vlist[0]; b++) {
      int found = 0;
      for(uintmax_t i = 0; i <y; i++) {
	if((int)tempint[i] == ite_defines[whereat].vlist[b]) 
	  { found = 1; break; }
      }
      if(found == 0) {
	fprintf(stderr, "\nVariable %s is not used in #define %s...exiting:%d\n", getsym_i(ite_defines[whereat].vlist[b])->name, ite_defines[whereat].string, ite_line);
	exit(1);
      }
    }
    
    if (whereat == totaldefines) {
      totaldefines++;
      if(totaldefines >= max_defines) {
	ite_defines = (ite_defines_struct *)sbsat_recalloc(ite_defines, max_defines, max_defines+10, sizeof(ite_defines_struct), 9, "ite_defines");
	max_defines+=10;
      }
    }
    strcpy (macros, "define");
    sbsat_free((void**)&tempint); tempint_max=0;
    return ite_defines[whereat].bdd;
  }
  if (!strcasecmp (macros, "order")) {
    DdNode *v1;
    int count = 0;
    while ((v1 = putite(BM, intnum, bdd))!=NULL) {
      if (v1 != Cudd_bddIthVar(BM->dd, v1->index)) {
	fprintf(stderr, "\nKeyword 'order' expects a list of positive variables terminated by ')'(%s)...exiting:%d\n", macros, ite_line);
	exit (1);
      }
      Cudd_RecursiveDeref(BM->dd, v1);
      count++;
    }
    if(count < 1) {
      fprintf(stderr, "\nKeyword 'order' expects at least 1 argument, found 0 (%s)...exiting:%d\n", macros, ite_line);
      exit (1);
    }
    strcpy (macros, "order");
    Cudd_Ref(Cudd_ReadOne(BM->dd));
    return Cudd_ReadOne(BM->dd);
  }
  if (!strcasecmp(macros, "truth_table")) {
    expect_integer = 1;
    DdNode *v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) {
      fprintf(stderr, "\nKeyword 'truth_table' needs a positive integer as a first argument (%s)...exiting:%d\n", macros, ite_line);
      exit (1);
    }
    expect_integer = 0;
    if (v1 != Cudd_bddIthVar(BM->dd, v1->index)) {
      fprintf(stderr, "\nKeyword 'truth_table' needs a positive integer as a first argument (%s)...exiting:%d\n", macros, ite_line);
      exit (1);
    }			
    int ints[50];
    for(unsigned int i = 1; i <= v1->index; i++) {
      DdNode *v2 = putite(BM, intnum, bdd);
      if (v2 == NULL || v2 != Cudd_bddIthVar(BM->dd, v2->index)) {
	fprintf(stderr, "\nKeyword 'truth_table' needs %d positive integers (%s)...exiting:%d\n", v1->index, macros, ite_line);
	exit (1);
      }			
      ints[i] = v2->index;
      Cudd_RecursiveDeref(BM->dd, v2);
    }
    ints[0] = v1->index;
    char *tv = (char *)calloc((1 << v1->index) +1, sizeof(char));
    order = getNextIteSymbol (BM, &intnum, &bdd);
    if(order == ')') { fprintf(stderr, "\n'Keyword 'truth_table' needs 2^%d, found 0...exiting:%d\n", ints[0], ite_line); exit(1); }
    
    for(long i = 0; i < 1 << v1->index; i++) {
      d5_printf2("%c", macros[i]);
      if(macros[i] == 'T' || macros[i] == 't') {
	tv[i] = '1';
      } else if(macros[i] == 'F' || macros[i] == 'f'){
	tv[i] = '0';
      } else {
	fprintf(stderr, "\nKeyword 'truth_table' needs 2^%d, found %ld...exiting:%d\n", ints[0], i, ite_line);
	exit(1);
      }
      tv[i+1] = 0;
    }
    strcpy (macros, "truth_table");
    fprintf(stderr, "\nKeyword 'truth_table' not currently supported in the 'ite' format...exiting:%d\n", ite_line);
    exit(1);
    DdNode *ret = Cudd_Not(Cudd_ReadOne(BM->dd)); //ReadSmurf (&y, tv, level, &(ints[1]), ints[0]);
    Cudd_Ref(ret);
    Cudd_RecursiveDeref(BM->dd, v1);
    free(tv);
    return ret;
  }
  if (!strcasecmp(macros, "mitosis")) {
    DdNode * v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) fprintf(stderr, "\nKeyword 'mitosis' expects at least 2 arguments, found 0 (%s)...exiting:%d\n", macros, ite_line);
    uintmax_t *tempint=NULL;
    uintmax_t y = 0;
    uintmax_t tempint_max = 0;
    BDD_Support(&y, &tempint_max, &tempint, v1);
    
    if (y != 0) qsort (tempint, y, sizeof (int), compfunc);
    
    for (int i = y; i >= 0; i--)
      tempint[i + 1] = tempint[i];
    tempint[0] = y;
    
    //      struct BDDNodeStruct **functions = new BDDNode*[50];      
    uintmax_t x = 1;
    for (; x <= tempint[0]; x++) {
      DdNode *v2 = (putite(BM, intnum, bdd));
      if(v2 == NULL) { fprintf(stderr, "\nHere, keyword 'mitosis' expects %ju arguments, found %ju (%s)...exiting:%d\n", tempint[0]+1, x, macros, ite_line); exit (1); }
      if (v2 != Cudd_bddIthVar(BM->dd, v2->index)) {
	fprintf(stderr, "\nKeyword 'mitosis' expects a list of positive variables terminated by ')'(%s)...exiting:%d\n", macros, ite_line);
	exit (1);
      }
      Cudd_RecursiveDeref(BM->dd, v2);
    }
    if(putite(BM, intnum, bdd)!=NULL || x != tempint[0]+1) { fprintf(stderr, "\nHere, keyword 'mitosis' expects %juarguments, found %ju (%s)...exiting:%d\n", tempint[0]+1, x+1, macros, ite_line); exit (1); }
    
    strcpy (macros, "mitosis");
    fprintf(stderr, "\nKeyword 'mitosis' not currently supported in the 'ite' format...exiting:%d\n", ite_line);
    exit(1);
    DdNode *ret = Cudd_Not(Cudd_ReadOne(BM->dd));//mitosis (v1, tempint, newBDD);
    Cudd_Ref(ret);
    Cudd_RecursiveDeref(BM->dd, v1);
    sbsat_free((void**)&tempint); tempint_max = 0;
    return ret;
  }
  if (!strcasecmp (macros, "minmax")) {
    int min, max;
    expect_integer = 1;
    DdNode * v1 = putite(BM, intnum, bdd);
    if (v1 == NULL) {
      fprintf(stderr, "\nKeyword 'minmax' needs a positive integer as a first argument (%s)...exiting:%d\n", macros, ite_line);
      exit (1);
    }
    if(v1 == Cudd_Not(Cudd_ReadOne(BM->dd))) min = 0;
    else min = v1->index;
    expect_integer = 1;
    DdNode * v2 = putite(BM, intnum, bdd);
    if (v2 == NULL) {
      fprintf(stderr, "\nKeyword 'minmax' needs a positive integer as a second argument (%s)...exiting:%d\n", macros, ite_line);
      exit (1);
    }
    expect_integer = 0;
    if(v2 == Cudd_Not(Cudd_ReadOne(BM->dd))) max = 0;
    else max = v2->index;
    //if(max<min) {
    //  fprintf(stderr, "\nKeyword 'minmax' cannot have it's second argument (%d) less than it's first argument (%d)...exiting:%d\n", max, min, ite_line);
    //	exit (1);
    //}
    
    int maxarguments = 10;
    int numarguments = 0;
    uintmax_t *var_list = (uintmax_t *)sbsat_calloc(maxarguments, sizeof(uintmax_t), 9, "var_list");
    
    DdNode *v3;
    while((v3 = putite(BM, intnum, bdd))!=NULL) {
      if(numarguments >= maxarguments) {
	var_list = (uintmax_t *)sbsat_recalloc(var_list, maxarguments, maxarguments+10, sizeof(uintmax_t), 9, "var_list");
	maxarguments+=10;
      }
      if(cuddIsConstant(Cudd_Regular(v3))) {
	fprintf(stderr, "\nKeyword 'minmax' needs positive variables as arguments (%s)..exiting:%d\n", macros, ite_line); exit(1);
      }
      if (v3 == Cudd_bddIthVar(BM->dd, v3->index)) {
	var_list[numarguments++] = v3->index;
      } else {	fprintf(stderr, "\nKeyword 'minmax' needs positive variables as arguments (%s)...exiting:%d\n", macros, ite_line); exit(1); }
      Cudd_RecursiveDeref(BM->dd, v3);
    }
    if(max > numarguments) { fprintf(stderr, "\nKeyword 'minmax' cannot have less than the specified max=%d variables (%s)...exiting:%d\n", max, macros, ite_line); exit (1); }
    
    qsort(var_list, numarguments, sizeof(uintmax_t), abscompfunc);
    for(int x = 0; x < numarguments-1; x++)
      if(var_list[x] == var_list[x+1]) {
	fprintf(stderr, "\nFound duplicate variables in the argument list for keyword 'minmax' (%s)...exiting:%d\n", macros, ite_line); exit(1); }
    
    strcpy (macros, "minmax");
    DdNode *ret = build_cardinality_BDD(BM->dd, min, max, var_list, numarguments);
    Cudd_Ref(ret);
    Cudd_RecursiveDeref(BM->dd, v1);
    Cudd_RecursiveDeref(BM->dd, v2);
    sbsat_free((void **)&var_list);
    return ret;
  }
  if (!strcasecmp (macros, "countT")) {
    DdNode *v1 = putite(BM, intnum, bdd);
    if(v1==NULL) { fprintf(stderr, "\nKeyword 'countT' expects at least 1 argument, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    
    int maxarguments = 10;
    int numarguments = 0;
    int *var_list = (int *)sbsat_calloc(maxarguments, sizeof(int), 9, "var_list");
    
    DdNode *v2;
    while((v2 = putite(BM, intnum, bdd))!=NULL) {
      if(numarguments >= maxarguments) {
	var_list = (int *)sbsat_recalloc(var_list, maxarguments, maxarguments+10, sizeof(int), 9, "var_list");
	maxarguments+=10;
      }
      if (v2 == Cudd_bddIthVar(BM->dd, v2->index)) {
	var_list[numarguments++] = v2->index;
      } else {	fprintf(stderr, "\nKeyword 'countT' needs positive variables as arguments (%s)...exiting:%d\n", macros, ite_line); exit (1); }
      Cudd_RecursiveDeref(BM->dd, v2);
    }
    
    qsort(var_list, numarguments, sizeof(int), abscompfunc);
    strcpy (macros, "countT");
    int numT = 0;
    fprintf(stderr, "\nKeyword 'countT' not currently supported in the 'ite' format...exiting:%d\n", ite_line);
    exit(1);
    //Probably something like
    //double Cudd_CountPathsToNonZero(DdNode * node)
    if(numarguments != 0) numT = 0;//count_true_paths(v1, var_list, numarguments);
    fprintf(stdout, "\n|count = %d|\n", numT);
    Cudd_RecursiveDeref(BM->dd, v1);
    sbsat_free((void **)&var_list);
    Cudd_Ref(Cudd_ReadOne(BM->dd));
    return Cudd_ReadOne(BM->dd);
  }
  if (!strcasecmp (macros, "exist")) {
    DdNode * tmp;
    DdNode * v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'exist' expects at least 2 arguments, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    DdNode * v2 = putite(BM, intnum, bdd);
    if(v2 == NULL) { fprintf(stderr, "\nKeyword 'exist' expects at least 2 arguments, found 1 (%s)...exiting:%d\n", macros, ite_line); ;exit (1); }
    if(v2 != Cudd_bddIthVar(BM->dd, v2->index)) {
      fprintf(stderr, "\nKeyword 'exist' expects a list of positive variables following the first argument (%s)...exiting:%d\n", macros, ite_line);
      exit (1);
    }
    DdNode *v3 = v2;
    while((v2 = putite(BM, intnum, bdd))!=NULL) {
      if(v2 != Cudd_bddIthVar(BM->dd, v2->index)) {
	fprintf(stderr, "\nKeyword 'exist' expects a list of positive variables following the first argument (%s)...exiting:%d\n", macros, ite_line);
	exit (1);
      }
      //v1 = xquantify(v1, v2->index);
      tmp = Cudd_bddAnd(BM->dd, v3, v2);
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(BM->dd, v2);
      Cudd_RecursiveDeref(BM->dd, v3);
      v3 = tmp;
    }
    DdNode *ret = Cudd_bddExistAbstract(BM->dd, v1, v3);
    Cudd_Ref(ret);
    Cudd_RecursiveDeref(BM->dd, v1);
    Cudd_RecursiveDeref(BM->dd, v3);
    strcpy (macros, "exist");
    return ret;
  }
  if (!strcasecmp (macros, "universe")) {
    DdNode *tmp;
    DdNode * v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'universe' expects at least 2 arguments, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    DdNode * v2 = putite(BM, intnum, bdd);
    if(v2 == NULL) { fprintf(stderr, "\nKeyword 'universe' expects at least 2 arguments, found 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }		
    if(v2 != Cudd_bddIthVar(BM->dd, v2->index)) {
      fprintf(stderr, "\nKeyword 'universe' expects a list of  positive variable following the first (%s)...exiting:%d\n", macros, ite_line);
      exit (1);
    }
    DdNode *v3 = v2;
    while((v2 = putite(BM, intnum, bdd))!=NULL) {
      if(v2 != Cudd_bddIthVar(BM->dd, v2->index)) {
	fprintf(stderr, "\nKeyword 'universe' expects a list of  positive variable following the first (%s)...exiting:%d\n", macros, ite_line);
	exit (1);
      }
      //v1 = uquantify (v1, v2->index);
      tmp = Cudd_bddAnd(BM->dd, v3, v2);
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(BM->dd, v2);
      Cudd_RecursiveDeref(BM->dd, v3);
      v3 = tmp;
    }
    DdNode *ret = Cudd_bddUnivAbstract(BM->dd, v1, v3);
    Cudd_Ref(ret);
    Cudd_RecursiveDeref(BM->dd, v1);
    Cudd_RecursiveDeref(BM->dd, v3);
    strcpy (macros, "universe");
    return ret;
  }
  if (!strcasecmp (macros, "safe")) {
    DdNode * v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'safe' expects 2 arguments, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    DdNode * v2 = putite(BM, intnum, bdd);
    if(v2 == NULL) { fprintf(stderr, "\nKeyword 'safe' expects 2 arguments, found 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }		
    if (v2 != Cudd_bddIthVar(BM->dd, v2->index)) {
      fprintf(stderr, "\nKeyword 'safe' needs a positive variable as a second argument (%s)...exiting:%d\n", macros, ite_line);
      exit (1);
    }
    if(putite(BM, intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'safe' expects 2 arguments, found > 2 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    strcpy (macros, "safe");
    if (v1 == Cudd_Not(Cudd_ReadOne(BM->dd)))
      return Cudd_Not(Cudd_ReadOne(BM->dd));
    fprintf(stderr, "\nKeyword 'safe' not currently supported in the 'ite' format...exiting:%d\n", ite_line);
    exit(1);
    DdNode *ret = Cudd_Not(Cudd_ReadOne(BM->dd)); //safe_assign (v1, v2->index);
    Cudd_Ref(ret);
    Cudd_RecursiveDeref(BM->dd, v1);
    Cudd_RecursiveDeref(BM->dd, v2);
    return ret;
  }
  if (!strcasecmp (macros, "safe_eq")) {
    DdNode * v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'safe_eq' expects 2 arguments, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    DdNode * v2 = putite(BM, intnum, bdd);
    if(v2 == NULL) { fprintf(stderr, "\nKeyword 'safe_eq' expects 2 arguments, found 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }		
    if (v2 != Cudd_bddIthVar(BM->dd, v2->index)) {
      fprintf(stderr, "\nKeyword 'safe_eq' needs a positive variable as a second argument (%s)...exiting:%d\n", macros, ite_line);
      exit (1);
    }
    if(putite(BM, intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'safe_eq' expects 2 arguments, found > 2 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    strcpy (macros, "safe_eq");
    if (v1 == Cudd_Not(Cudd_ReadOne(BM->dd)))
      return Cudd_Not(Cudd_ReadOne(BM->dd));
    fprintf(stderr, "\nKeyword 'safe_eq' not currently supported in the 'ite' format...exiting:%d\n", ite_line);
    exit(1);
    DdNode *ret = Cudd_Not(Cudd_ReadOne(BM->dd));//safe_assign_eq (v1, v2->index);
    Cudd_Ref(ret);
    Cudd_RecursiveDeref(BM->dd, v1);
    Cudd_RecursiveDeref(BM->dd, v2);
    return ret;
  }
  if (!strcasecmp (macros, "safe_func")) {
    DdNode * v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'safe_func' expects 2 arguments, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    DdNode * v2 = putite(BM, intnum, bdd);
    if(v2 == NULL) { fprintf(stderr, "\nKeyword 'safe_func' expects 2 arguments, found 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    if (v2 != Cudd_bddIthVar(BM->dd, v2->index)) {
      fprintf(stderr, "\nKeyword 'safe_func' needs a positive variable as a second argument (%s)...exiting:%d\n", macros, ite_line);
      exit (1);
    }
    if(putite(BM, intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'safe_func' expects 2 arguments, found > 2 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    strcpy (macros, "safe_func");
    if (v1 == Cudd_Not(Cudd_ReadOne(BM->dd)))
      return Cudd_Not(Cudd_ReadOne(BM->dd));
    fprintf(stderr, "\nKeyword 'safe_func' not currently supported in the 'ite' format...exiting:%d\n", ite_line);
    exit(1);
    DdNode *ret = Cudd_Not(Cudd_ReadOne(BM->dd));//safe_assign_func (v1, v2->index);
    Cudd_Ref(ret);
    Cudd_RecursiveDeref(BM->dd, v1);
    Cudd_RecursiveDeref(BM->dd, v2);
    return ret;
  }
  if (!strcasecmp (macros, "remove_fps")) {
    DdNode * v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'remove_fps' expects 2 arguments, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    DdNode * v2 = putite(BM, intnum, bdd);
    if(v2 == NULL) { fprintf(stderr, "\nKeyword 'remove_fps' expects 2 arguments, found 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }		
    if(putite(BM, intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'remove_fps' expects 2 arguments, found > 2 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    strcpy (macros, "remove_fps");
    fprintf(stderr, "\nKeyword 'remove_fps' not currently supported in the 'ite' format...exiting:%d\n", ite_line);
    exit(1);
    DdNode *ret = Cudd_Not(Cudd_ReadOne(BM->dd));//remove_fps (v1, v2);
    Cudd_Ref(ret);
    Cudd_RecursiveDeref(BM->dd, v1);
    Cudd_RecursiveDeref(BM->dd, v2);
    return ret;
  }
  if (!strcasecmp (macros, "pprint_tree")) {
    DdNode * v1;
    v1 = putite(BM, intnum, bdd);
    if(v1==NULL) { fprintf(stderr, "\nKeyword 'pprint_tree' expects 1 argument, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    if(putite(BM, intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'pprint_tree' expects 1 argument, found > 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    d2_printf1("\n");
    //		for (int i = 0; i < PRINT_TREE_WIDTH; i++)
    //		  d2_printf1("-");
    d2_printf1("\n");
    fprintf(stderr, "\nKeyword 'pprint_tree' not currently supported in the 'ite' format...exiting:%d\n", ite_line);
    exit(1);
    //print_bdd (v1);
    d2_printf1("\n");
    strcpy (macros, "pprint_tree");
    return v1;
  }
  if (!strcasecmp (macros, "print_tree")) {
    DdNode * v1;
    //      int which_zoom = 0;
    v1 = putite(BM, intnum, bdd);
    if(v1==NULL) { fprintf(stderr, "\nKeyword 'print_tree' expects 1 argument, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    if(putite(BM, intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'print_tree' expects 1 argument, found > 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    //printBDDTree (v1, &which_zoom);
    fprintf(stderr, "\nKeyword 'print_tree' not currently supported in the 'ite' format...exiting:%d\n", ite_line);
    exit(1);
    strcpy (macros, "print_tree");
    return v1;
  }
  if (!strcasecmp (macros, "print_xdd")) {
    DdNode * v1;
    v1 = putite(BM, intnum, bdd);
    if(v1==NULL) { fprintf(stderr, "\nKeyword 'print_xdd' expects 1 argument, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    if(putite(BM, intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'print_xdd' expects 1 argument, found > 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    d2_printf1("\n");
    //		for (int i = 0; i < PRINT_TREE_WIDTH; i++)
    //		  d2_printf1("-");
    d2_printf1("\n");
    //print_xdd_d(bdd2xdd(v1));
    fprintf(stderr, "\nKeyword 'print_xdd' not currently supported in the 'ite' format...exiting:%d\n", ite_line);
    exit(1);
    d2_printf1(" = 1\n");
    strcpy (macros, "print_xdd");
    return v1;
  }
  if (!strcasecmp (macros, "print_flat_xdd")) {
    DdNode * v1;
    v1 = putite(BM, intnum, bdd);
    if(v1==NULL) { fprintf(stderr, "\nKeyword 'print_flat_xdd' expects 1 argument, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    if(putite(BM, intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'print_flat_xdd' expects 1 argument, found > 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    d2_printf1("\n");
    //		for (int i = 0; i < PRINT_TREE_WIDTH; i++)
    //		  d2_printf1("-");
    d2_printf1("\n");
    uintmax_t *tempint=NULL;
    uintmax_t y = 0;
    uintmax_t tempint_max = 0;
    BDD_Support(&y, &tempint_max, &tempint, v1);
    if(tempint!=NULL) sbsat_free((void **)&tempint);
    if(y == 0) { d2_printf1("0"); }
    //else { print_flat_xdd(bdd2xdd(v1), y); }
    fprintf(stderr, "\nKeyword 'print_flat_xdd' not currently supported in the 'ite' format...exiting:%d\n", ite_line);
    exit(1);
    d2_printf1(" = 0\n");
    strcpy (macros, "print_flat_xdd");
    return v1;
  }
  if (!strcasecmp (macros, "print_smurf_dot_stdout")) {
    DdNode *v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'print_smurf_dot_stdout' expects at least 1 argument, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    
    int maxarguments = 10;
    int numarguments = 0;
    DdNode **put_dot_bdds = (DdNode **)sbsat_calloc(maxarguments, sizeof(DdNode *), 9, "put_dot_bdds");
    put_dot_bdds[0] = v1;
    numarguments++;
    //    Cudd_RecursiveDeref(BM->dd, v1);
    
    while ((v1 = putite(BM, intnum, bdd))!=NULL) {
      if(numarguments >= maxarguments) {
	put_dot_bdds = (DdNode **)sbsat_recalloc(put_dot_bdds, maxarguments, maxarguments+10, sizeof(DdNode *), 9, "put_dot_bdds");
	maxarguments+=10;
      }
      put_dot_bdds[numarguments++] = v1;
      //Cudd_RecursiveDeref(BM->dd, v1);
    }
    strcpy (macros, "print_smurf_dot_stdout");
    PrintSmurfs(BM, put_dot_bdds, numarguments);
    for(int i = 0; i < numarguments; i++)
      Cudd_IterDerefBdd(BM->dd, put_dot_bdds[i]);
    sbsat_free((void**)&put_dot_bdds);
    Cudd_Ref(Cudd_ReadOne(BM->dd));
    return Cudd_ReadOne(BM->dd);
  }
  if (!strcasecmp (macros, "print_dot_stdout")) {
    DdNode *v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'print_dot_stdout' expects at least 1 argument, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    
    int maxarguments = 10;
    int numarguments = 0;
    DdNode **put_dot_bdds = (DdNode **)sbsat_calloc(maxarguments, sizeof(DdNode *), 9, "put_dot_bdds");
    put_dot_bdds[0] = v1;
    numarguments++;
    
    Cudd_RecursiveDeref(BM->dd, v1);		
    while ((v1 = putite(BM, intnum, bdd))!=NULL) {
      if(numarguments >= maxarguments) {
	put_dot_bdds = (DdNode **)sbsat_recalloc(put_dot_bdds, maxarguments, maxarguments+10, sizeof(DdNode *), 9, "put_dot_bdds");
	maxarguments+=10;
      }
      put_dot_bdds[numarguments++] = v1;
      Cudd_RecursiveDeref(BM->dd, v1);
    }
    strcpy (macros, "print_dot_stdout");
    //printBDDdot_stdout(put_dot_bdds, numarguments);
    fprintf(stderr, "\nKeyword 'print_dot_stdout' not currently supported in the 'ite' format...exiting:%d\n", ite_line);
    exit(1);
    for(int x = 0; x < numarguments; x++)
      Cudd_RecursiveDeref(BM->dd, put_dot_bdds[x]);
    sbsat_free((void**)&put_dot_bdds);
    Cudd_Ref(Cudd_ReadOne(BM->dd));
    return Cudd_ReadOne(BM->dd);
  }
  if (!strcasecmp (macros, "print_dot")) {
    DdNode *v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'print_dot' expects at least 1 argument, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    
    int maxarguments = 10;
    int numarguments = 0;
    DdNode **put_dot_bdds = (DdNode **)sbsat_calloc(maxarguments, sizeof(DdNode *), 9, "put_dot_bdds");
    put_dot_bdds[0] = v1;
    numarguments++;
    //Cudd_RecursiveDeref(BM->dd, v1);
    
    while ((v1 = putite(BM, intnum, bdd))!=NULL) {
      if(numarguments >= maxarguments) {
	put_dot_bdds = (DdNode **)sbsat_recalloc(put_dot_bdds, maxarguments, maxarguments+10, sizeof(DdNode *), 9, "put_dot_bdds");
	maxarguments+=10;
      }
      put_dot_bdds[numarguments++] = v1;			
      //Cudd_RecursiveDeref(BM->dd, v1);
    }
    strcpy (macros, "print_dot");
    
    FILE *fout;
    char filename[256];
    char filename_dot[256];
    strcpy(filename_dot, inputfile);
    strcat(filename_dot, ".dot");
    
    if(strcmp(inputfile, "-")) get_freefile(filename_dot, NULL, filename, 256);
    else filename[0] = 0;
    
    if ((fout = fopen((filename[0]==0)?"output.dot":filename, "wb+")) == NULL)  {
      fprintf(stderr, "Can't open '%s' for writing", filename);
      exit (1);
    }
    
    //printBDDdot_file(put_dot_bdds, numarguments);		
    Cudd_DumpDot(BM->dd, numarguments, put_dot_bdds, NULL, NULL, fout);
    
    fclose(fout);
    
    for(int x = 0; x < numarguments; x++)
      Cudd_RecursiveDeref(BM->dd, put_dot_bdds[x]);
    sbsat_free((void**)&put_dot_bdds);
    Cudd_Ref(Cudd_ReadOne(BM->dd));
    return Cudd_ReadOne(BM->dd);
  }
  if (!strcasecmp (macros, "gelim_preload_file")) {
    order = getNextIteSymbol (BM, &intnum, &bdd);
    if(order != 'm') {
      fprintf(stderr, "\nKeyword 'gelim_preload' takes one filename as an argument, found 0...exiting:%d\n", ite_line);
      exit(0);
    }
    gelim_preload_file = (char *)calloc((strnlen(macros, 100)+1), sizeof(char));
    strncpy(gelim_preload_file, macros, 100);
    order = getNextIteSymbol (BM, &intnum, &bdd);
    if(order != ')') { //need?
      fprintf(stderr, "\nKeyword 'gelim_preload' takes one filename as an argument, found more than 1...exiting:%d\n", ite_line);
      exit(0);
    }
    Cudd_Ref(Cudd_ReadOne(BM->dd));    
    return Cudd_ReadOne(BM->dd);
  } 
  if (!strcasecmp (macros, "same")) {
    DdNode *v1, *v2, *v3, *bTemp;
    v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'same' expects at least 2 arguments, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    v2 = putite(BM, intnum, bdd);
    if(v2 == NULL) { fprintf(stderr, "\nKeyword 'same' expects at least 2 arguments, found 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    v2 = Cudd_bddXnor(BM->dd, v1, bTemp = v2);	Cudd_Ref(v2);
    Cudd_RecursiveDeref(BM->dd, bTemp);
    while ((v3 = putite(BM, intnum, bdd))!=NULL) {
      v3 = Cudd_bddXnor(BM->dd, v1, bTemp = v3); Cudd_Ref(v3);
      Cudd_RecursiveDeref(BM->dd, bTemp);
      
      v2 = Cudd_bddAnd(BM->dd, bTemp = v2, v3); Cudd_Ref(v2);
      Cudd_RecursiveDeref(BM->dd, bTemp);
      Cudd_RecursiveDeref(BM->dd, v3);
    }
    strcpy (macros, "same");
    Cudd_RecursiveDeref(BM->dd, v1);
    return v2;
  }
  if (!strcasecmp (macros, "gcf")) {
    DdNode * v1, *v2;
    v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'gcf' expects 2 arguments, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    v2 = putite(BM, intnum, bdd);
    if(v2 == NULL) { fprintf(stderr, "\nKeyword 'gcf' expects 2 arguments, found 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    if(putite(BM, intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'gcf' expects 2 arguments, found > 2 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    strcpy (macros, "gcf");
    if (v1 == Cudd_Not(Cudd_ReadOne(BM->dd)))
      return Cudd_Not(Cudd_ReadOne(BM->dd));
    DdNode *ret = Cudd_bddConstrain(BM->dd, v1, v2); Cudd_Ref(ret);
    Cudd_RecursiveDeref(BM->dd, v1);
    Cudd_RecursiveDeref(BM->dd, v2);
    return ret;
  }
  if (!strcasecmp (macros, "prune")) { //Same as restrict, below
    DdNode * v1, *v2;
    v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'prune' expects 2 arguments, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    v2 = putite(BM, intnum, bdd);
    if(v2 == NULL) { fprintf(stderr, "\nKeyword 'prune' expects 2 arguments, found 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    if(putite(BM, intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'prune' expects 2 arguments, found > 2 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    strcpy (macros, "prune");
    if (v1 == Cudd_Not(Cudd_ReadOne(BM->dd)))
      return Cudd_Not(Cudd_ReadOne(BM->dd));
    DdNode *ret = Cudd_bddRestrict(BM->dd, v1, v2); Cudd_Ref(ret);
    Cudd_RecursiveDeref(BM->dd, v1);
    Cudd_RecursiveDeref(BM->dd, v2);
    return ret;
  }
  if (!strcasecmp (macros, "restrict")) { //Same as prune, above
    DdNode * v1, *v2;
    v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'restrict' expects 2 arguments, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    v2 = putite(BM, intnum, bdd);
    if(v2 == NULL) { fprintf(stderr, "\nKeyword 'restrict' expects 2 arguments, found 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    if(putite(BM, intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'restrict' expects 2 arguments, found > 2 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    strcpy (macros, "restrict");
    if (v1 == Cudd_Not(Cudd_ReadOne(BM->dd)))
      return Cudd_Not(Cudd_ReadOne(BM->dd));
    DdNode *ret = Cudd_bddRestrict(BM->dd, v1, v2); Cudd_Ref(ret);
    Cudd_RecursiveDeref(BM->dd, v1);
    Cudd_RecursiveDeref(BM->dd, v2);
    return ret;
  }
  if (!strcasecmp (macros, "strengthen")) {
    DdNode * v1, *v2;
    v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'strengthen' expects 2 arguments, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    v2 = putite(BM, intnum, bdd);
    if(v2 == NULL) { fprintf(stderr, "\nKeyword 'strengthen' expects 2 arguments, found 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    if(putite(BM, intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'strengthen' expects 2 arguments, found > 2 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    strcpy (macros, "strengthen");
    if (v1 == Cudd_Not(Cudd_ReadOne(BM->dd)))
      return Cudd_Not(Cudd_ReadOne(BM->dd));
    //strengthen_fun(v1, v2);
    DdNode *ret = Cudd_Not(Cudd_ReadOne(BM->dd)); Cudd_Ref(ret);
    Cudd_RecursiveDeref(BM->dd, v1);
    Cudd_RecursiveDeref(BM->dd, v2);
    return ret;
  }
  if (!strcasecmp (macros, "nimp")) {
    DdNode *v1, *v2, *bTemp;
    v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'nimp' expects at least 2 arguments, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    v2 = putite(BM, intnum, bdd);
    if(v2 == NULL) { fprintf(stderr, "\nKeyword 'nimp' expects at least 2 arguments, found 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    v1 = Cudd_Not(Cudd_bddOr(BM->dd, Cudd_Not(bTemp = v1), v2)); Cudd_Ref(v1);
    Cudd_RecursiveDeref(BM->dd, bTemp);
    Cudd_RecursiveDeref(BM->dd, v2);
    while ((v2 = putite(BM, intnum, bdd))!=NULL) {
      v1 = Cudd_Not(Cudd_bddOr(BM->dd, Cudd_Not(bTemp = v1), v2)); Cudd_Ref(v1);
      Cudd_RecursiveDeref(BM->dd, bTemp);
      Cudd_RecursiveDeref(BM->dd, v2);
    }
    strcpy (macros, "nimp");
    return v1;
  }
  if (!strcasecmp (macros, "nand")) {
    DdNode *v1, *v2, *bTemp;
    v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'nand' expects at least 2 arguments, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    v2 = putite(BM, intnum, bdd);
    if(v2 == NULL) { fprintf(stderr, "\nKeyword 'nand' expects at least 2 arguments, found 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    v1 = Cudd_bddNand(BM->dd, bTemp = v1, v2); Cudd_Ref(v1);
    Cudd_RecursiveDeref(BM->dd, bTemp);
    Cudd_RecursiveDeref(BM->dd, v2);
    while ((v2 = putite(BM, intnum, bdd))!=NULL) {
      v1 = Cudd_bddNand(BM->dd, bTemp = v1, v2); Cudd_Ref(v1);
      Cudd_RecursiveDeref(BM->dd, bTemp);
      Cudd_RecursiveDeref(BM->dd, v2);
    }
    strcpy (macros, "nand");
    return v1;
  }
  if (!strcasecmp (macros, "nor")) {
    DdNode *v1, *v2, *bTemp;
    v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'nor' expects at least 2 arguments, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    v2 = putite(BM, intnum, bdd);
    if(v2 == NULL) { fprintf(stderr, "\nKeyword 'nor' expects at least 2 arguments, found 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    v1 = Cudd_bddNor(BM->dd, bTemp = v1, v2); Cudd_Ref(v1);
    Cudd_RecursiveDeref(BM->dd, bTemp);
    Cudd_RecursiveDeref(BM->dd, v2);
    while ((v2 = putite(BM, intnum, bdd))!=NULL) {
      v1 = Cudd_bddNor(BM->dd, bTemp = v1, v2); Cudd_Ref(v1);
      Cudd_RecursiveDeref(BM->dd, bTemp);
      Cudd_RecursiveDeref(BM->dd, v2);
    }
    strcpy (macros, "nor");
    return v1;
  }
  if (!strcasecmp (macros, "equ") || !strcasecmp (macros, "noxor") || !strcasecmp (macros, "xnor")) {
    DdNode *v1, *v2, *bTemp;
    v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'equ' expects at least 2 arguments, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    v2 = putite(BM, intnum, bdd);
    if(v2 == NULL) { fprintf(stderr, "\nKeyword 'equ' expects at least 2 arguments, found 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    v1 = Cudd_bddXnor(BM->dd, bTemp = v1, v2); Cudd_Ref(v1);
    Cudd_RecursiveDeref(BM->dd, bTemp);
    Cudd_RecursiveDeref(BM->dd, v2);
    while ((v2 = putite(BM, intnum, bdd))!=NULL) {
      v1 = Cudd_bddXnor(BM->dd, bTemp = v1, v2); Cudd_Ref(v1);
      Cudd_RecursiveDeref(BM->dd, bTemp);
      Cudd_RecursiveDeref(BM->dd, v2);
    }
    strcpy (macros, "equ");
    return v1;
  }
  if (!strcasecmp (macros, "imp")) {
    DdNode *v1, *v2, *bTemp;
    v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'imp' expects at least 2 arguments, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    v2 = putite(BM, intnum, bdd);
    if(v2 == NULL) { fprintf(stderr, "\nKeyword 'imp' expects at least 2 arguments, found 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    v1 = Cudd_bddOr(BM->dd, Cudd_Not(bTemp = v1), v2); Cudd_Ref(v1);
    Cudd_RecursiveDeref(BM->dd, bTemp);
    Cudd_RecursiveDeref(BM->dd, v2);
    while ((v2 = putite(BM, intnum, bdd))!=NULL) {
      v1 = Cudd_bddOr(BM->dd, Cudd_Not(bTemp = v1), v2); Cudd_Ref(v1);
      Cudd_RecursiveDeref(BM->dd, bTemp);
      Cudd_RecursiveDeref(BM->dd, v2);
    }
    strcpy (macros, "imp");
    return v1;
  }
  if (!strcasecmp (macros, "and")) {
    DdNode *v1, *v2, *bTemp;
    v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'and' expects at least 2 arguments, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    v2 = putite(BM, intnum, bdd);
    if(v2 == NULL) { fprintf(stderr, "\nKeyword 'and' expects at least 2 arguments, found 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    v1 = Cudd_bddAnd(BM->dd, bTemp = v1, v2); Cudd_Ref(v1);
    Cudd_RecursiveDeref(BM->dd, bTemp);
    Cudd_RecursiveDeref(BM->dd, v2);
    while ((v2 = putite(BM, intnum, bdd))!=NULL) {
      v1 = Cudd_bddAnd(BM->dd, bTemp = v1, v2); Cudd_Ref(v1);
      Cudd_RecursiveDeref(BM->dd, bTemp);
      Cudd_RecursiveDeref(BM->dd, v2);
    }
    strcpy (macros, "and");
    return v1;
  }
  if (!strcasecmp (macros, "xor")) {
    DdNode *v1, *v2, *bTemp;
    v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'xor' expects at least 2 arguments, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    v2 = putite(BM, intnum, bdd);
    if(v2 == NULL) { fprintf(stderr, "\nKeyword 'xor' expects at least 2 arguments, found 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    v1 = Cudd_bddXor(BM->dd, bTemp = v1, v2); Cudd_Ref(v1);
    Cudd_RecursiveDeref(BM->dd, bTemp);
    Cudd_RecursiveDeref(BM->dd, v2);
    while ((v2 = putite(BM, intnum, bdd))!=NULL) {
      v1 = Cudd_bddXor(BM->dd, bTemp = v1, v2); Cudd_Ref(v1);
      Cudd_RecursiveDeref(BM->dd, bTemp);
      Cudd_RecursiveDeref(BM->dd, v2);
    }
    strcpy (macros, "xor");
    return v1;
  }
  if (!strcasecmp (macros, "or")) {
    DdNode *v1, *v2, *bTemp;
    v1 = putite(BM, intnum, bdd);
    if(v1 == NULL) { fprintf(stderr, "\nKeyword 'or' expects at least 2 arguments, found 0 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    v2 = putite(BM, intnum, bdd);
    if(v2 == NULL) { fprintf(stderr, "\nKeyword 'or' expects at least 2 arguments, found 1 (%s)...exiting:%d\n", macros, ite_line); exit (1); }
    v1 = Cudd_bddOr(BM->dd, bTemp = v1, v2); Cudd_Ref(v1);
    Cudd_RecursiveDeref(BM->dd, bTemp);
    Cudd_RecursiveDeref(BM->dd, v2);
    while ((v2 = putite(BM, intnum, bdd))!=NULL) {
      v1 = Cudd_bddOr(BM->dd, bTemp = v1, v2); Cudd_Ref(v1);
      Cudd_RecursiveDeref(BM->dd, bTemp);
      Cudd_RecursiveDeref(BM->dd, v2);
    }
    strcpy (macros, "or");
    return v1;
  }
  
  int v = i_getsym(macros, SYM_VAR);
  
  if(v > (int)nNumVariables) {
    fprintf(stderr, "\nToo many symbols used (%s). Need to increase to greater than %ju...exiting:%d\n", macros, nNumVariables, ite_line);
    exit (1);
  }
  
  if(negate_it == 1) {
    negate_it = 0;
    DdNode *ret = Cudd_Not(Cudd_bddIthVar(BM->dd, v)); Cudd_Ref(ret);
    return ret;
  } else {
    DdNode *ret = Cudd_bddIthVar(BM->dd, v); Cudd_Ref(ret);
    return ret;
  }
  
  fprintf(stderr, "\nUnknown word (%s)...exiting:%d\n", macros,	ite_line);
  exit (1);
  return NULL;
}

char getNextIteSymbol (BDDManager *BM, intmax_t *intnum, DdNode **bdd) {
  int i = 0;
  int p = 0;
  while (1) {
    p = fgetc(finputfile);
    if (p == '\n') { ite_line++; }
    if (p == EOF) {
      //fprintf(stderr, "\nUnexpected EOF (%s)...exiting\n", macros);
      return 'x';
    }
    if (p == '*') {
      fprintf(stderr, "Formula parse error...exiting:%d\n", ite_line);
      exit (1);
    }
    if (p == '!') {
      fprintf(stderr, "Formula parse error, character '!'is invalid. Use '-' or not( ... ) to negate...exiting:%d\n", ite_line);
      exit (1);
    }
    if (p == ')') {
      return ')';
    }
    if (p == ';') {
      while (p != '\n') {
	p = fgetc(finputfile);
	if (p == EOF) {
	  fprintf(stderr, "\nUnexpected EOF...exiting:%d\n", ite_line);
	  exit (1);
	}
      }
      ungetc(p, finputfile);
      continue;
    }
    if (((p >= 'a') && (p <= 'z')) || ((p >= 'A') && (p <= 'Z'))
	|| (p == '_') || ((p >= '0') && (p <= '9')) || (p == '-')) {
      negate_it = 0;
      if (p == '-') {
	negate_it = 1;
	p = fgetc(finputfile);
	if (p == EOF) {
	  fprintf(stderr, "\nUnexpected EOF...exiting:%d\n", ite_line);
	  exit (1);
	}
      }
      if (p == '-') {
	fprintf(stderr, "\nDouble '-' found, variables cannot start with a '-'...exiting:%d\n", ite_line);
	exit (1);
      }
      int is_int = 1;
      while (((p >= 'a') && (p <= 'z')) || ((p >= 'A') && (p <= 'Z'))
	     || (p == '_') || ((p >= '0') && (p <= '9'))) {
	macros[i] = p;
	i++;
	if (((p >= 'a') && (p <= 'z')) || ((p >= 'A') && (p <= 'Z'))
	    || (p == '_')) {
	  is_int = 0;
	}
	if(i >= max_macros) {
	  macros = (char *)sbsat_recalloc(macros, max_macros, max_macros+10, sizeof(char), 9, "macros");
	  max_macros+=10;
	}
	p = fgetc(finputfile);
	if (p == EOF) {
	  fprintf(stderr, "\nUnexpected EOF (%s)...exiting:%d\n",	macros, ite_line);
	  exit (1);
	}
      }
      ungetc (p, finputfile);
      
      macros[i] = 0;
      if(is_int == 1 && expect_integer == 1) {
	expect_integer = 0;
	(*intnum) = atoi (macros);
	if (negate_it) (*intnum) = -(*intnum);
	return 'i';
      } else if(is_int == 0 && expect_integer == 1) {					  
	fprintf(stderr, "\nExpecting an integer, found %s...exiting:%d\n", macros, ite_line);
	exit(1);
      } else {
	if(!strcasecmp(macros, "define")) {
	  fprintf(stderr, "\n'#' expected in front of keyword 'define'...exiting:%d\n", ite_line);
	  exit (1);
	}
      }
      if(negate_it) d5_printf2("-%s ", macros);
      if(!negate_it) d5_printf2("%s ", macros);
      return 'm';
    }
    if (p == '#') {
      p = fgetc(finputfile);
      if (p == EOF) {
	fprintf(stderr, "\nUnexpected EOF...exiting:%d\n", ite_line);
	exit (1);
      }
      if(p == 'd') { //d for #define, no other operator starts with d
	while (((p >= 'a') && (p <= 'z')) || ((p >= 'A') && (p <= 'Z'))
	       || (p == '_') || ((p >= '0') && (p <= '9'))) {
	  macros[i] = p;
	  i++;
	  if(i >= max_macros) {
	    macros = (char *)sbsat_recalloc(macros, max_macros, max_macros+10, sizeof(char), 9, "macros");
	    max_macros+=10;
	  }
	  p = fgetc(finputfile);
	  if (p == EOF) {
	    fprintf(stderr, "\nUnexpected EOF (%s)...exiting:%d\n",	macros, ite_line);
	    exit (1);
	  }
	}
	ungetc (p, finputfile);
	macros[i] = 0;
	if(strcasecmp(macros, "define")) {
	  fprintf(stderr, "\n#define expected, found %s...exiting:%d\n", macros, ite_line);
	  exit (1);
	}
	return 'm';
      } else {
	ungetc(p, finputfile);
	return '#';
      }
    }
    if (p == '$') {
      p = fgetc(finputfile);
      if (p == EOF) {
	fprintf(stderr, "\nUnexpected EOF (%s)...exiting:%d\n", macros, ite_line);
	exit (1);
      }
      if ((p >= '1') && (p <= '9')) {
	i = 0;
	while ((p >= '0') && (p <= '9')) {
	  integers[i] = p;
	  i++;
	  if(i >= max_integers) {
	    integers = (char *)sbsat_recalloc(integers, max_integers, max_integers+10, sizeof(char), 9, "integers");
	    max_integers+=10;
	  }
	  p = fgetc(finputfile);
	  if (p == EOF) {
	    fprintf(stderr, "\nUnexpected EOF (%s)...exiting:%d\n",	integers, ite_line);
	    exit (1);
	  }
	}
	integers[i] = 0;
	(*intnum) = atoi (integers) - 1;
	if ((*intnum) > (intmax_t)BM->nNumBDDs - 1) {
	  fprintf(stderr, "\nFunction $%jd has not yet been defined...exiting:%d\n", (*intnum) + 1, ite_line);
	  exit (1);
	}
	ungetc (p, finputfile);
	(*bdd) = BM->BDDList[(*intnum)];
	return 'b';
      } else {
	fprintf(stderr, "\nFormula parse error...exiting:%d\n",	ite_line);
	exit (1);
      }
    }
  }
}

uint8_t iteloop (BDDManager *BM) {
  uint8_t ret = NO_ERROR;
  
  fscanf (finputfile, "%ju %ju", &nNumVariables, &nNumBDDs);
  d9_printf3("%ju %ju", nNumVariables, nNumBDDs);
  
  Cudd_bddIthVar(BM->dd, nNumVariables); //Necessary to resize CUDD arrays to the maximum size (can vastly increase efficiency)
  
  //  Cudd_AutodynDisable(BM->dd);                                                                                            
  
  ite_line = 1;
  int intnum = 0, keepnum = 0;
  DdNode *bdd = NULL;
  int p = 0;
  
  int nForceBackjumpLevel = -1;
  
  max_initbranch = 1;
  max_varlevel = 1;
  initbranch = (initbranch_struct *)sbsat_recalloc(initbranch, 0, max_initbranch, sizeof(initbranch_struct), 9, "initbranch");
  initbranch[0].branch_level = 0;
  initbranch[0].max_initbranch_level = 10;
  initbranch[0].num_initbranch_level = 0;
  initbranch[0].vars = (initbranch_level *)sbsat_recalloc(initbranch[0].vars, 0, initbranch[0].max_initbranch_level, sizeof(initbranch_level), 9, "initbranch[0].vars");
  
  totaldefines = 0;
  max_defines = 100;
  ite_defines = (ite_defines_struct *)sbsat_recalloc(ite_defines, 0, max_defines, sizeof(ite_defines_struct), 9, "ite_defines");
  
  max_integers = 20;
  integers = (char *)sbsat_recalloc(integers, 0, max_integers, sizeof(char), 9, "integers");
  
  max_macros = 20;
  macros = (char *)sbsat_recalloc(macros, 0, max_macros, sizeof(char), 9, "macros");
  
  int *keep = (int *)sbsat_calloc(nNumBDDs + 2, sizeof(int), 9, "keep");
  
  p = fgetc(finputfile);
  
  d4_printf1("\n");
  
  while (1) {		//(p = fgetc(finputfile))!=EOF) 
    if (ite_line%10000 == 1) {
      d2e_printf2("\rReading %d ", ite_line);
      d4_printf2("Reading %d ", ite_line);
    }
    if (p == '\n') {
      p = fgetc(finputfile);
      if(p == EOF) {
	goto Exit;
      }
      ite_line++;
      d4_printf1("\r");
      continue;
    }
    if (p == ';') {
      while (p != '\n') {
	p = fgetc(finputfile);
	if (p == EOF) {
	  goto Exit;
	}
      }
      d4_printf1("\r");
      continue;
    }
    if (p == '	' || p == ' ' || p == ')' || p == '(') {
      while (p == '	' || p == ' ' || p == ')' || p == '(') {
	p = fgetc(finputfile);
	if (p == EOF) {
	  goto Exit;
	}
      }
      //			p = fgetc(finputfile);
      if(p == EOF) {
	goto Exit;
      }
      
      d4_printf1("\r");
      continue;
    }
    if (p == '*') {
      DdNode *temp = putite(BM, intnum, bdd);
      if(temp == NULL) goto Exit;
      if ((strcasecmp (macros, "pprint_tree"))
	  && (strncasecmp (macros, "print_", 6))
	  && (strcasecmp (macros, "define"))
	  && (strcasecmp (macros, "order"))
	  && (strcasecmp (macros, "countT"))
	  && (strcasecmp (macros, "initial_branch"))) {
	keep[add_BDD_to_manager(BM, temp)] = 1;
	Cudd_IterDerefBdd(BM->dd, temp);
	keepnum++;
	if (BM->nNumBDDs > nNumBDDs) {
	  fprintf(stderr, "Too many functions, increase to larger than %ju...exiting:%d",	nNumBDDs, ite_line);
	  exit (1);
	}
      } else Cudd_RecursiveDeref(BM->dd, temp);
      d4_printf1("*");
    } else {
      ungetc (p, finputfile);
      if (p == ';')
	continue;
      DdNode * temp = putite(BM, intnum, bdd);
      if(temp == NULL) goto Exit;
      if ((strcasecmp (macros, "pprint_tree"))
	  && (strncasecmp (macros, "print_", 6))
	  && (strcasecmp (macros, "define"))
	  && (strcasecmp (macros, "order"))
	  && (strcasecmp (macros, "countT"))
	  && (strcasecmp (macros, "initial_branch"))) {
	add_BDD_to_manager(BM, temp);
	Cudd_IterDerefBdd(BM->dd, temp);
	
	if (BM->nNumBDDs > nNumBDDs) {
	  fprintf(stderr, "Too many functions, increase to larger than %ju...exiting:%d",	nNumBDDs, ite_line);
	  exit (1);
	}
      } else Cudd_RecursiveDeref(BM->dd, temp);
    }
    D_5(
	if ((strcasecmp (macros, "pprint_tree"))
	    && (strncasecmp (macros, "print_", 6))
	    && (strcasecmp (macros, "define"))
	    && (strcasecmp (macros, "order"))
	    && (strcasecmp (macros, "countT"))
	    && (strcasecmp (macros, "initial_branch"))) {
	  fprintf(stddbg, "BDD $%ju: ", BM->nNumBDDs);
	  //SEAN!!! Add this back when we can support it
	  //printBDDfile (BM->BDDList[BM->nNumBDDs - 1], stddbg);
	}
	)
      
      p = fgetc (finputfile);
    if (p != '\n') {
      int continue_all = 0;
      if(p==';') continue_all = 1;
      else if (p!='	' && p!='\r' && p!=' ' && p!='(' && p!=')' && p!=',') {
	fprintf(stderr, "Error: Extra characters following line %d...exiting\n", ite_line);
	exit (1);
	continue_all = 1;
      }
      p = fgetc(finputfile);
      while (p != EOF) {
	if (p == '\n')
	  break;
	if (p == ';') continue_all = 1;
	if (continue_all == 0 && p!='	' && p!='\r' && p!=' ' && p!='(' && p!=')' && p!=',') {
	  fprintf(stderr, "Error: Extra characters following line %d...exiting\n", ite_line);
	  exit (1);
	  continue_all = 1;
	}
	p = fgetc(finputfile);
      }
      if (p != '\n')
	goto Exit;
    }
    d4_printf1("\n");
  }
 Exit:;
  
  int *initbranch_used = (int *)sbsat_calloc(nNumVariables + 2, sizeof(int), 9, "initbranch_used");
  
  //arrVarChoiceLevels = (int **)sbsat_calloc(max_varlevel, sizeof(int *), 9, "arrVarChoiceLevels");
  arrVarTrueInfluences = (double *)sbsat_calloc(nNumVariables+1, sizeof(double), 9, "arrVarTrueInfluences");
  
  for(uintmax_t i = 0; i < nNumVariables+1; i++)
    arrVarTrueInfluences[i] = 0.5;
  
  //Need to sort initbranch based on initbranch[x].branch_level
  //qsort(initbranch, max_initbranch, sizeof(initbranch_struct), initbranch_compfunc);
  
  for(int i = 0; i < max_initbranch; i++) {
    int max_CLevels = initbranch[i].num_initbranch_level+1;
    //arrVarChoiceLevels[initbranch[i].branch_level] = (int *)sbsat_recalloc(arrVarChoiceLevels[initbranch[i].branch_level], 0, max_CLevels, sizeof(int), 9, "arrVarChoiceLevels[i]");
    //arrVarChoiceLevels[i] = (int *)sbsat_calloc(sizeof(int), 9, "arrVarChoiceLevels[i]");
    int nVarChoiceIter = 0;
    d9_printf3("[%d [%s]]", i, initbranch[i].vars[0].string);
    for(int x = 0; x < initbranch[i].num_initbranch_level; x++) {
      t_myregex myrg;
      sym_regex_init(&myrg, initbranch[i].vars[x].string);
      int id = sym_regex(&myrg);
      int looper = 0;
      while (id) {
	looper++;
	if(initbranch_used[id] == 0) {
	  // found variable and the variable id is id
	  initbranch_used[id] = 1;
	  arrVarTrueInfluences[id] = initbranch[i].vars[x].true_inf_weight;
	  //arrVarChoiceLevels[initbranch[i].branch_level][nVarChoiceIter++] = id;
	  //d5_printf5("%d indep=%d %s %s\n", looper, id, getsym_i(id)->name, initbranch[i].vars[x].string);
	  d5_printf7("%d %d %s %s priority=%d true_inf=%4.6f\n", looper, id, getsym_i(id)->name, initbranch[i].vars[x].string, initbranch[i].branch_level, initbranch[i].vars[x].true_inf_weight);
	  if(nVarChoiceIter >= max_CLevels) {
	    //arrVarChoiceLevels[initbranch[i].branch_level] = (int *)sbsat_recalloc(arrVarChoiceLevels[initbranch[i].branch_level], max_CLevels, max_CLevels+10, sizeof(int), 9, "arrVarChoiceLevels[i]");
	    max_CLevels += 10;
	  }
	}
	id = sym_regex(&myrg);
      }
      sym_regex_free(&myrg);
    }
    //arrVarChoiceLevels[initbranch[i].branch_level][nVarChoiceIter] = 0;
  }
  
  if(nForceBackjumpLevel >= (int)max_varlevel) {
    fprintf(stderr, "\n'--force-backjump-level %d' does not correspond to a level specified by 'initial_branch'...exiting\n", nForceBackjumpLevel);
    exit(1);
  }
  
  //Remove blank levels
  /*
  int count=-1;
  for(unsigned int x = 0; x < max_varlevel; x++) {
    count++;
    int *tmp = arrVarChoiceLevels[count];
    arrVarChoiceLevels[count] = arrVarChoiceLevels[x];
    if(!arrVarChoiceLevels[x] || arrVarChoiceLevels[x][0] == 0) {
      if(nForceBackjumpLevel == (int)x) {
	fprintf(stderr, "\n'--force-backjump-level %d' does not correspond to a level specified by 'initial_branch'...exiting\n", nForceBackjumpLevel);
	exit(1);
      }
      count--;
    }
    if(nForceBackjumpLevel == (int)x) { nForceBackjumpLevel = count; d9_printf2("nFBL = %d\n", nForceBackjumpLevel);}
    arrVarChoiceLevels[x] = tmp;
  }
  
  nVarChoiceLevelsMax = max_initbranch;
  nVarChoiceLevelsNum = count+1;
  */  

  //Also we must take equivalences into account in the preprocessor!
  
  if (keepnum == 0)
    for (uintmax_t x = 0; x < BM->nNumBDDs; x++)
      keep[x] = 1;
  
  //(x <= nNumBDDs) means to throw away functions that come after
  //the cutoff part nNumBDDs.
  for (uintmax_t x = 0; x < BM->nNumBDDs; x++) {
    if(keep[x] == 0)
      remove_BDD_from_manager(BM, x);
    else if(BM->BDDList[x] == Cudd_Not(Cudd_ReadOne(BM->dd))) {
      ret = UNSAT;
      //fprintf(stderr, "\nProblem is Unsatisfiable...exiting:%d\n", x+1, ite_line);
      //exit(1);
    }
  }

  d2_printf1("\rReading ITE ... Done\n");
  
  for(int i = 0; i < max_initbranch; i++) {
    for(int x = 0; x < initbranch[i].num_initbranch_level; x++)
      sbsat_free((void **)&initbranch[i].vars[x].string);
    sbsat_free((void **)&initbranch[i].vars);
  }
  sbsat_free((void **)&initbranch);
  sbsat_free((void **)&initbranch_used);
  sbsat_free((void **)&keep);
  for(int x = 0; x < totaldefines; x++) {
    sbsat_free((void **)&ite_defines[x].string);
    sbsat_free((void **)&ite_defines[x].vlist);
  }
  sbsat_free((void **)&ite_defines);
  sbsat_free((void **)&integers);
  sbsat_free((void **)&macros);
  
  //  Cudd_AutodynEnable(BM->dd, CUDD_REORDER_SAME);
  
  return ret;
}
