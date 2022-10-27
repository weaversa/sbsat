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
 * */

#include "sbsat.h"

void printBDDSupport(BDDManager *BM, DdNode *bdd) {
  uintmax_t length;
  BDD_Support(&length, &(BM->uTempList.nLength_max), &(BM->uTempList.pList), bdd);
  qsort(BM->uTempList.pList, BM->uTempList.nLength, sizeof(uintmax_t), compfunc);  
  
  for(uintmax_t i = 0; i < length; i++) {
    d2_printf2("%s", s_name(BM->uTempList.pList[i]));
    d4_printf2("(%ju)", BM->uTempList.pList[i]);
    d2_printf1(" ");
  }
}

void printBDD(DdManager *dd, DdNode *bdd) {
  if(bdd == DD_ONE(dd)) {
    d2_printf1("T");
    return;
  }
  if(bdd == Cudd_Not(DD_ONE(dd))) {
    d2_printf1("F");
    return;
  }

  DdNode *bddr = Cudd_Regular(bdd);
  d2_printf1("(");

  printBDD(dd, Cudd_NotCond(cuddT(bddr), bdd != bddr));
  //fprintf(stdout, "[%ju]", bddr->index);
  d2_printf2("[%s", s_name(bddr->index));
  d4_printf2("(%d)", bddr->index);
  d2_printf1("]");
  printBDD(dd, Cudd_NotCond(cuddE(bddr), bdd != bddr));
  d2_printf1(")");
}

void printBDDerr(DdManager *dd, DdNode *bdd) {
  if(bdd == DD_ONE(dd)) {
    fprintf(stderr, "T:%d", bdd->ref);
    return;
  }
  if(bdd == Cudd_Not(DD_ONE(dd))) {
    fprintf(stderr, "F:%d", bdd->ref);
    return;
  }

  DdNode *bddr = Cudd_Regular(bdd);

  fprintf(stderr, "(");
  printBDDerr(dd, Cudd_NotCond(cuddT(bddr), bdd != bddr));
  if(1) {
    fprintf(stderr, "[%d:r=%d]", bddr->index, bddr->ref);
  } else {
    fprintf(stderr, "[%s", s_name(bddr->index));
    fprintf(stderr, "(%d)]", bddr->index);
  }
  printBDDerr(dd, Cudd_NotCond(cuddE(bddr), bdd != bddr));
  fprintf(stderr, ")");
}

void _printBDDTruthTable(DdManager *dd, DdNode *f, uintmax_t_list *support, uintmax_t level) {
  /* SEAN!!! TODO
  if(f == DD_ONE(dd)) {
    //mpz_setbit(*ret, support->nLength - level);
    return ret;
  }
  if(f == Cudd_Not(DD_ONE(dd))) {
    //?
    return ret;
  }
  
  DdNode *F = Cudd_Regular(f);  
  DdNode *T, *E;

  uintmax_t misses = level;
  while(F->index != support->pList[level]) level++;
  assert(level < support->nLength);

  T = cuddT(F); E = cuddE(F);
  if(Cudd_IsComplement(f)) {
    T = Cudd_Not(T); E = Cudd_Not(E);
  }

  _printBDDTruthTable(dd, T, support, level+1);
  _printBDDTruthTable(dd, E, support, level+1);
  */
}

void printBDDTruthTable(BDDManager *BM, DdNode *f, uintmax_t_list *support) {
  //This not mean to work on large BDDs
  uintmax_t_list_copy(&BM->uTempList, support);
  qsort(BM->uTempList.pList, BM->uTempList.nLength, sizeof(uintmax_t), bddperm_compfunc);
  _printBDDTruthTable(BM->dd, f, &BM->uTempList, 0);
}

uint8_t isSingleton(DdNode *bdd) {
  bdd = Cudd_Regular(bdd);
  if(cuddIsConstant(bdd)) return 1;
  if(cuddIsConstant(cuddT(bdd)) && cuddIsConstant(Cudd_Regular(cuddE(bdd)))) return 1;
  return 0;
}

void printBDDAND(DdManager *dd, DdNode *bdd, FILE *fout) {
  uint8_t neg = Cudd_IsComplement(bdd);
  bdd = Cudd_Regular(bdd);
  while(bdd != Cudd_NotCond(DD_ONE(dd), neg==1)) {
    if(cuddT(bdd) == Cudd_NotCond(DD_ONE(dd), neg==0)) {
      fprintf(fout, "-%s", s_name(bdd->index));
      neg = neg ^ Cudd_IsComplement(cuddE(bdd));
      bdd = Cudd_Regular(cuddE(bdd));
    } else if(Cudd_Regular(cuddE(bdd)) == Cudd_NotCond(DD_ONE(dd), neg == Cudd_IsComplement(cuddE(bdd)))) {
      fprintf(fout, "%s", s_name(bdd->index));
      bdd = cuddT(bdd);
    } else assert(0);
    if(bdd != Cudd_NotCond(DD_ONE(dd), neg==1))
      fprintf(fout, ", ");
  }
}

void printBDDAND_latex(DdManager *dd, DdNode *bdd, FILE *fout) {
  uint8_t neg = Cudd_IsComplement(bdd);
  bdd = Cudd_Regular(bdd);
  while(bdd != Cudd_NotCond(DD_ONE(dd), neg==1)) {
    if(cuddT(bdd) == Cudd_NotCond(DD_ONE(dd), neg==0)) {
      fprintf(fout, "\\overline{%s}", s_name(bdd->index));
      neg = neg ^ Cudd_IsComplement(cuddE(bdd));
      bdd = Cudd_Regular(cuddE(bdd));
    } else if(Cudd_Regular(cuddE(bdd)) == Cudd_NotCond(DD_ONE(dd), neg == Cudd_IsComplement(cuddE(bdd)))) {
      fprintf(fout, "%s", s_name(bdd->index));
      bdd = cuddT(bdd);
    } else assert(0);
    if(bdd != Cudd_NotCond(DD_ONE(dd), neg==1))
      fprintf(fout, " \\wedge ");
  }
}

void printBDDOR(DdManager *dd, DdNode *bdd, FILE *fout) {
  uint8_t neg = Cudd_IsComplement(bdd);
  bdd = Cudd_Regular(bdd);
  while(bdd != Cudd_NotCond(DD_ONE(dd), neg==0)) {
    if(cuddT(bdd) == Cudd_NotCond(DD_ONE(dd), neg==1)) {
      fprintf(fout, "%s", s_name(bdd->index));
      neg = neg ^ Cudd_IsComplement(cuddE(bdd));
      bdd = Cudd_Regular(cuddE(bdd));
    } else if(Cudd_Regular(cuddE(bdd)) == Cudd_NotCond(DD_ONE(dd), neg ^ Cudd_IsComplement(cuddE(bdd)))) {
      fprintf(fout, "-%s", s_name(bdd->index));
      bdd = cuddT(bdd);
    } else assert(0);
    if(bdd != Cudd_NotCond(DD_ONE(dd), neg==0))
      fprintf(fout, ", ");
  }
}

void printBDDOR_latex(DdManager *dd, DdNode *bdd, FILE *fout) {
  uint8_t neg = Cudd_IsComplement(bdd);
  bdd = Cudd_Regular(bdd);
  while(bdd != Cudd_NotCond(DD_ONE(dd), neg==0)) {
    if(cuddT(bdd) == Cudd_NotCond(DD_ONE(dd), neg==1)) {
      fprintf(fout, "%s", s_name(bdd->index));
      neg = neg ^ Cudd_IsComplement(cuddE(bdd));
      bdd = Cudd_Regular(cuddE(bdd));
    } else if(Cudd_Regular(cuddE(bdd)) == Cudd_NotCond(DD_ONE(dd), neg ^ Cudd_IsComplement(cuddE(bdd)))) {
      fprintf(fout, "\\overline{%s}", s_name(bdd->index));
      bdd = cuddT(bdd);
    } else assert(0);
    if(bdd != Cudd_NotCond(DD_ONE(dd), neg==0))
      fprintf(fout, " \\vee ");
  }
}

void printBDD_ReduceSpecFunc_latex(BDDManager *BM, DdNode *bdd, FILE *fout);

void printITEBDD_file_latex(BDDManager *BM, DdNode *bdd, FILE *fout) {
  DdManager *dd = BM->dd;
  DdNode *bddr;

  if(bdd == DD_ONE(dd)) {
    fprintf(fout, "{\\tt T}");
    return;
  }
  if(bdd == Cudd_Not(DD_ONE(dd))) {
    fprintf(fout, "{\\tt F}");
    return;
  }
  bddr = Cudd_Regular(bdd);
  
  if(cuddT(bddr) == DD_ONE(dd) && cuddE(bddr) == Cudd_Not(DD_ONE(dd))) {
    if(bdd==bddr) {
      fprintf(fout, "%s", s_name(bddr->index));
    } else {
      fprintf(fout, "\\overline{%s}", s_name(bddr->index));
    }
  } else if(cuddE(bddr) == DD_ONE(dd) && cuddE(bddr) == Cudd_Not(DD_ONE(dd))) {
    if(bdd==bddr) {
      fprintf(fout, "\\overline{%s}", s_name(bddr->index));
    } else {
      fprintf(fout, "%s", s_name(bddr->index));
    }
  } else {
    if(bdd==bddr) {
      fprintf(fout, "(%s \\wedge ", s_name(bddr->index));
    } else {
      fprintf(fout, "\\overline{(%s \\wedge ", s_name(bddr->index));
    }
    if(!isSingleton(cuddT(bddr))) fprintf(fout, "(");
    printBDD_ReduceSpecFunc_latex(BM, cuddT(bddr), fout);
    fprintf(fout, ")");
    if(!isSingleton(cuddT(bddr))) fprintf(fout, ")");

    fprintf(fout, " \\vee (\\overline{%s} \\wedge ", s_name(bddr->index));

    if(!isSingleton(cuddE(bddr))) fprintf(fout, "(");
    printBDD_ReduceSpecFunc_latex(BM, cuddE(bddr), fout);
    fprintf(fout, ")");
    if(!isSingleton(cuddE(bddr))) fprintf(fout, ")");

    if(bdd!=bddr) fprintf(fout, "}");
  }
}

void printBDD_ReduceSpecFunc_latex(BDDManager *BM, DdNode *bdd, FILE *fout) {
  DdManager *dd = BM->dd;
  
  uintmax_t fn_type = findandret_fnType(BM, bdd);

  uintmax_t bdd_length = BM->uTempList.nLength;
  uintmax_t *bdd_vars = BM->uTempList.pList;

  if(bdd_length < 2) {
    printITEBDD_file_latex(BM, bdd, fout);
  } else if(fn_type == FN_TYPE_UNSURE) {
    DdNode *cube;
    if((cube = Cudd_FindEssential(dd, bdd))!=NULL && Cudd_Regular(cube) != DD_ONE(dd)) {
      //bdd has the form and(v1..vk, f) where v1..vk are literals in cube
      //and f is an arbitrary bdd
      
      cuddRef(cube);
      bdd = Cudd_Cofactor(dd, bdd, cube); cuddRef(bdd);
      
      printBDDAND_latex(dd, cube, fout);
      fprintf(fout, " \\wedge ");
      Cudd_IterDerefBdd(dd, cube);
      fprintf(fout, "(");
      printBDD_ReduceSpecFunc_latex(BM, bdd, fout);
      fprintf(fout, ")");
      Cudd_IterDerefBdd(dd, bdd);
    } else if((cube = Cudd_FindEssential(dd, Cudd_Not(bdd)))!=NULL && Cudd_Regular(cube) != DD_ONE(dd)) {
      //bdd has the form or(v1..vk, f) where v1..vk are literals in cube
      //and f is an arbitrary bdd
      
      cuddRef(cube);
      bdd = Cudd_Not(Cudd_Cofactor(dd, Cudd_Not(bdd), cube)); cuddRef(bdd);
      
      printBDDOR_latex(dd, Cudd_Not(cube), fout);
      fprintf(fout, " \\vee ");
      Cudd_IterDerefBdd(dd, cube);
      fprintf(fout, "(");
      printBDD_ReduceSpecFunc_latex(BM, bdd, fout);
      fprintf(fout, ")");
      Cudd_IterDerefBdd(dd, bdd);
    } else {
      DdNode *factor = find_equals_factor(BM->dd, bdd); cuddRef(factor);
      if(factor == bdd) {
	Cudd_IterDerefBdd(dd, factor);
	printITEBDD_file_latex(BM, bdd, fout);	
      } else if(Cudd_Regular(factor) != DD_ONE(BM->dd)) {
	cuddRef(bdd);
	do {
	  fprintf(fout, "(");
	  printBDD_ReduceSpecFunc_latex(BM, factor, fout);
	  fprintf(fout, ")");
 	  DdNode *tmp_bdd = bdd;
	  bdd = better_gcf(BM, bdd, factor); cuddRef(bdd);
	  //bdd = Cudd_bddConstrain(BM->dd, bdd, factor); cuddRef(bdd);
	  //bdd = Cudd_bddRestrict(BM->dd, bdd, factor); cuddRef(bdd);
	  //bdd = Cudd_bddOr(BM->dd, bdd, Cudd_Not(factor)); cuddRef(bdd);
	  //bdd = DD_ONE(BM->dd); cuddRef(bdd);
	  //bdd = Cudd_bddSqueeze(BM->dd, bdd, factor); cuddRef(bdd);
	  //bdd = Cudd_bddLICompaction(BM->dd, bdd, factor); cuddRef(bdd);
	  //bdd = Cudd_bddMinimize(BM->dd, bdd, factor); cuddRef(bdd);
	  Cudd_IterDerefBdd(BM->dd, tmp_bdd);
	  Cudd_IterDerefBdd(BM->dd, factor);
	  if(Cudd_Regular(bdd) != DD_ONE(BM->dd)) {
	    fprintf(fout, " \\wedge ");
	  }
	  factor = find_equals_factor(BM->dd, bdd); cuddRef(factor);
	} while(Cudd_Regular(factor) != DD_ONE(BM->dd));
	Cudd_IterDerefBdd(BM->dd, factor);
	if(Cudd_Regular(bdd) != DD_ONE(BM->dd)) {
	  fprintf(stdout, "(");
	  printBDD_ReduceSpecFunc_latex(BM, bdd, fout);
	  fprintf(stdout, ")");
	}
	Cudd_IterDerefBdd(BM->dd, bdd);
      } else {
	Cudd_IterDerefBdd(BM->dd, factor);
	printITEBDD_file_latex(BM, bdd, fout);
      }
    }
  } else if(fn_type == FN_TYPE_OR) {
    printBDDOR_latex(dd, bdd, fout);
  } else if(fn_type == FN_TYPE_AND) {
    printBDDAND_latex(dd, bdd, fout);
  } else if(fn_type == FN_TYPE_XOR) {
    uint8_t neg=Cudd_IsComplement(bdd);
    fprintf(fout, "%s", s_name(bdd_vars[0]));
    for(uintmax_t y = 1; y < bdd_length; y++)
      fprintf(fout, "\\oplus %s", s_name(bdd_vars[y]));
    if(neg == (bdd_length&1))
      fprintf(fout, "\\oplus {\\tt T}");
  } else if(fn_type == FN_TYPE_OR_EQU) {
    Cudd_AutodynDisable(BM->dd);
    fprintf(fout, "%s \\oplus ", s_name(BM->equ_var->index));
    DdNode *or_bdd = Cudd_Cofactor(dd, bdd, BM->equ_var); cuddRef(or_bdd);
    assert(isOR(dd, or_bdd));
    fprintf(fout, "(");
    printBDDAND_latex(dd, Cudd_Not(or_bdd), fout);
    fprintf(fout, "))");
    Cudd_IterDerefBdd(dd, or_bdd);
    Cudd_AutodynEnable(BM->dd, CUDD_REORDER_SAME);
  } else if(fn_type == FN_TYPE_AND_EQU) {
    Cudd_AutodynDisable(BM->dd);
    fprintf(fout, "%s \\oplus ", s_name(BM->equ_var->index));
    DdNode *and_bdd = Cudd_Cofactor(dd, bdd, BM->equ_var); cuddRef(and_bdd);
    assert(isAND(dd, and_bdd));
    fprintf(fout, "(");
    printBDDOR_latex(dd, Cudd_Not(and_bdd), fout);
    fprintf(fout, "))");
    Cudd_IterDerefBdd(dd, and_bdd);
    Cudd_AutodynEnable(BM->dd, CUDD_REORDER_SAME);
  } else if(fn_type == FN_TYPE_MINMAX) {
    DdNode *one, *zero, *tmp_bdd;
    uintmax_t max, min;
    
    one = DD_ONE(dd);
    zero = Cudd_Not(one);
    
    uint8_t neg = Cudd_IsComplement(bdd);
    tmp_bdd = Cudd_Regular(bdd);
    max = -1;
    do {
      tmp_bdd = cuddT(tmp_bdd);
      max++;
    } while (!cuddIsConstant(tmp_bdd));
    
    tmp_bdd = Cudd_Regular(bdd);
    min = -1;
    do {
      tmp_bdd = cuddE(tmp_bdd);
      neg = neg ^ Cudd_IsComplement(tmp_bdd);
      tmp_bdd = Cudd_Regular(tmp_bdd);
      min++;
    } while (!cuddIsConstant(tmp_bdd));
    if(tmp_bdd == Cudd_NotCond(one, neg)) min = 0;
    else min = bdd_length - min;
    
    fprintf(fout, "%ju \\leq %s", min, s_name(bdd_vars[0]));
    for(uintmax_t y = 1; y < bdd_length; y++)
      fprintf(fout, " + %s", s_name(bdd_vars[y]));
    fprintf(fout, "\\leq %ju", max);
  } else if(fn_type == FN_TYPE_NEGMINMAX) {
    DdNode *one, *zero, *tmp_bdd;
    uintmax_t max, min;
    
    one = DD_ONE(dd);
    zero = Cudd_Not(one);
    
    bdd = Cudd_Not(bdd);
    
    uint8_t neg = Cudd_IsComplement(bdd);
    tmp_bdd = Cudd_Regular(bdd);
    max = -1;
    do {
      tmp_bdd = cuddT(tmp_bdd);
      max++;
    } while (!cuddIsConstant(tmp_bdd));
    
    tmp_bdd = Cudd_Regular(bdd);
    min = -1;
    do {
      tmp_bdd = cuddE(tmp_bdd);
      neg = neg ^ Cudd_IsComplement(tmp_bdd);
      tmp_bdd = Cudd_Regular(tmp_bdd);
      min++;
    } while (!cuddIsConstant(tmp_bdd));
    if(tmp_bdd == Cudd_NotCond(one, neg)) min = 0;
    else min = bdd_length - min;      
    
    fprintf(fout, "\\overline{%ju \\leq %s", min, s_name(bdd_vars[0]));
    for(uintmax_t y = 1; y < bdd_length; y++)
      fprintf(fout, " + %s", s_name(bdd_vars[y]));
    fprintf(fout, " \\leq %ju}", max);
  } else if(fn_type == FN_TYPE_DEP_VAR) {
    uintmax_t var = BM->equ_var->index;
    fprintf(fout, "%s \\oplus (", s_name(var));
    DdNode *equ_bdd = Cudd_Cofactor(BM->dd, bdd, BM->equ_var); cuddRef(equ_bdd);
    printBDD_ReduceSpecFunc_latex(BM, Cudd_Not(equ_bdd), fout);
    Cudd_IterDerefBdd(BM->dd, equ_bdd);
    fprintf(fout, ")");
  } else {
    fprintf(stderr, "UNIMPLEMENTED FUNCTION TYPE\n");
    assert(0);
  }
  return;
}

void printBDD_ReduceSpecFunc(BDDManager *BM, DdNode *bdd, FILE *fout);

void printITEBDD_file(BDDManager *BM, DdNode *bdd, FILE *fout) {
  DdManager *dd = BM->dd;
  DdNode *bddr;

  if(bdd == DD_ONE(dd)) {
    fprintf(fout, "T");
    return;
  }
  if(bdd == Cudd_Not(DD_ONE(dd))) {
    fprintf(fout, "F");
    return;
  }
  bddr = Cudd_Regular(bdd);
  
  if(cuddT(bddr) == DD_ONE(dd) && cuddE(bddr) == Cudd_Not(DD_ONE(dd))) {
    if(bdd==bddr) {
      fprintf(fout, "%s", s_name(bddr->index));
    } else {
      fprintf(fout, "-%s", s_name(bddr->index));
    }
  } else if(cuddE(bddr) == DD_ONE(dd) && cuddE(bddr) == Cudd_Not(DD_ONE(dd))) {
    if(bdd==bddr) {
      fprintf(fout, "-%s", s_name(bddr->index));
    } else {
      fprintf(fout, "%s", s_name(bddr->index));
    }
  } else {
    if(bdd==bddr) {
      fprintf(fout, "ite(%s, ", s_name(bddr->index));
    } else {
      fprintf(fout, "not(ite(%s, ", s_name(bddr->index));
    }
    printBDD_ReduceSpecFunc(BM, cuddT(bddr), fout);
    fprintf(fout, ", ");
    printBDD_ReduceSpecFunc(BM, cuddE(bddr), fout);
    fprintf(fout, ")");
    if(bdd!=bddr) fprintf(fout, ")");
  }
}

void printBDD_ReduceSpecFunc(BDDManager *BM, DdNode *bdd, FILE *fout) {
  DdManager *dd = BM->dd;
  
  uintmax_t fn_type = findandret_fnType(BM, bdd);

  uintmax_t bdd_length = BM->uTempList.nLength;
  uintmax_t *bdd_vars = BM->uTempList.pList;

  if(bdd_length < 2) {
    printITEBDD_file(BM, bdd, fout);
  } else if(fn_type == FN_TYPE_UNSURE) {
    DdNode *cube;
    if((cube = Cudd_FindEssential(dd, bdd))!=NULL && Cudd_Regular(cube) != DD_ONE(dd)) {
      //bdd has the form and(v1..vk, f) where v1..vk are literals in cube
      //and f is an arbitrary bdd
      
      cuddRef(cube);
      bdd = Cudd_Cofactor(dd, bdd, cube); cuddRef(bdd);
      
      fprintf(fout, "and(");
      printBDDAND(dd, cube, fout);
      fprintf(fout, ", ");
      Cudd_IterDerefBdd(dd, cube);
      printBDD_ReduceSpecFunc(BM, bdd, fout);
      Cudd_IterDerefBdd(dd, bdd);
      fprintf(fout, ")");
    } else if((cube = Cudd_FindEssential(dd, Cudd_Not(bdd)))!=NULL && Cudd_Regular(cube) != DD_ONE(dd)) {
      //bdd has the form or(v1..vk, f) where v1..vk are literals in cube
      //and f is an arbitrary bdd
      
      cuddRef(cube);
      bdd = Cudd_Not(Cudd_Cofactor(dd, Cudd_Not(bdd), cube)); cuddRef(bdd);
      
      fprintf(fout, "or(");
      printBDDOR(dd, Cudd_Not(cube), fout);
      fprintf(fout, ", ");
      Cudd_IterDerefBdd(dd, cube);
      printBDD_ReduceSpecFunc(BM, bdd, fout);
      Cudd_IterDerefBdd(dd, bdd);
      fprintf(fout, ")");
    } else {
      DdNode *factor = find_equals_factor(BM->dd, bdd); cuddRef(factor);
      if(factor == bdd) {
	Cudd_IterDerefBdd(dd, factor);
	printITEBDD_file(BM, bdd, fout);	
      } else if(Cudd_Regular(factor) != DD_ONE(BM->dd)) {
	cuddRef(bdd);
	fprintf(fout, "and(");
	do {
	  printBDD_ReduceSpecFunc(BM, factor, fout);
	  Cudd_IterDerefBdd(BM->dd, factor);
	  DdNode *tmp_bdd = bdd;
	  bdd = better_gcf(BM, bdd, factor); cuddRef(bdd);
	  //bdd = Cudd_bddConstrain(BM->dd, bdd, factor); cuddRef(bdd);
	  //bdd = Cudd_bddRestrict(BM->dd, bdd, factor); cuddRef(bdd);
	  //bdd = Cudd_bddOr(BM->dd, bdd, Cudd_Not(factor)); cuddRef(bdd);
	  //bdd = DD_ONE(BM->dd); cuddRef(bdd);
	  //bdd = Cudd_bddSqueeze(BM->dd, bdd, factor); cuddRef(bdd);
	  //bdd = Cudd_bddLICompaction(BM->dd, bdd, factor); cuddRef(bdd);
	  //bdd = Cudd_bddMinimize(BM->dd, bdd, factor); cuddRef(bdd);
	  Cudd_IterDerefBdd(BM->dd, tmp_bdd);
	  if(Cudd_Regular(bdd) != DD_ONE(BM->dd)) {
	    fprintf(fout, ", ");
	  }
	  factor = find_equals_factor(BM->dd, bdd); cuddRef(factor);
	} while(Cudd_Regular(factor) != DD_ONE(BM->dd));
	Cudd_IterDerefBdd(BM->dd, factor);
	if(Cudd_Regular(bdd) != DD_ONE(BM->dd))
	  printBDD_ReduceSpecFunc(BM, bdd, fout);
	Cudd_IterDerefBdd(BM->dd, bdd);
	fprintf(fout, ")");
      } else {
	Cudd_IterDerefBdd(BM->dd, factor);
	printITEBDD_file(BM, bdd, fout);
      }
    }
  } else if(fn_type == FN_TYPE_OR) {
    fprintf(fout, "or(");
    printBDDOR(dd, bdd, fout);
    fprintf(fout, ")");
  } else if(fn_type == FN_TYPE_AND) {
    fprintf(fout, "and(");
    printBDDAND(dd, bdd, fout);
    fprintf(fout, ")");
  } else if(fn_type == FN_TYPE_XOR) {
    uint8_t neg=Cudd_IsComplement(bdd);
    fprintf(fout, "xor(%s", s_name(bdd_vars[0]));
    for(uintmax_t y = 1; y < bdd_length; y++)
      fprintf(fout, ", %s", s_name(bdd_vars[y]));
    if(neg == (bdd_length&1))
      fprintf(fout, ", T");
    fprintf(fout, ")");
  } else if(fn_type == FN_TYPE_OR_EQU) {
    Cudd_AutodynDisable(BM->dd);
    fprintf(fout, "equ(");
    fprintf(fout, "%s, ", s_name(BM->equ_var->index));
    DdNode *or_bdd = Cudd_Cofactor(dd, bdd, BM->equ_var); cuddRef(or_bdd);
    assert(isOR(dd, or_bdd));
    fprintf(fout, "or(");
    printBDDOR(dd, or_bdd, fout);
    fprintf(fout, "))");
    Cudd_IterDerefBdd(dd, or_bdd);
    Cudd_AutodynEnable(BM->dd, CUDD_REORDER_SAME);
  } else if(fn_type == FN_TYPE_AND_EQU) {
    Cudd_AutodynDisable(BM->dd);
    fprintf(fout, "equ(");
    fprintf(fout, "%s, ", s_name(BM->equ_var->index));

    DdNode *and_bdd = Cudd_Cofactor(dd, bdd, BM->equ_var); cuddRef(and_bdd);
    assert(isAND(dd, and_bdd));
    fprintf(fout, "and(");
    printBDDAND(dd, and_bdd, fout);
    fprintf(fout, "))");
    Cudd_IterDerefBdd(dd, and_bdd);
    Cudd_AutodynEnable(BM->dd, CUDD_REORDER_SAME);
  } else if(fn_type == FN_TYPE_MINMAX) {
    DdNode *one, *zero, *tmp_bdd;
    uintmax_t max, min;
    
    one = DD_ONE(dd);
    zero = Cudd_Not(one);
    
    uint8_t neg = Cudd_IsComplement(bdd);
    tmp_bdd = Cudd_Regular(bdd);
    max = -1;
    do {
      tmp_bdd = cuddT(tmp_bdd);
      max++;
    } while (!cuddIsConstant(tmp_bdd));
    
    tmp_bdd = Cudd_Regular(bdd);
    min = -1;
    do {
      tmp_bdd = cuddE(tmp_bdd);
      neg = neg ^ Cudd_IsComplement(tmp_bdd);
      tmp_bdd = Cudd_Regular(tmp_bdd);
      min++;
    } while (!cuddIsConstant(tmp_bdd));
    if(tmp_bdd == Cudd_NotCond(one, neg)) min = 0;
    else min = bdd_length - min;
    
    fprintf(fout, "minmax(%ju, %ju, %s", min, max, s_name(bdd_vars[0]));
    for(uintmax_t y = 1; y < bdd_length; y++)
      fprintf(fout, ", %s", s_name(bdd_vars[y]));
    fprintf(fout, ")");
  } else if(fn_type == FN_TYPE_NEGMINMAX) {
    DdNode *one, *zero, *tmp_bdd;
    uintmax_t max, min;
    
    one = DD_ONE(dd);
    zero = Cudd_Not(one);
    
    bdd = Cudd_Not(bdd);
    
    uint8_t neg = Cudd_IsComplement(bdd);
    tmp_bdd = Cudd_Regular(bdd);
    max = -1;
    do {
      tmp_bdd = cuddT(tmp_bdd);
      max++;
    } while (!cuddIsConstant(tmp_bdd));
    
    tmp_bdd = Cudd_Regular(bdd);
    min = -1;
    do {
      tmp_bdd = cuddE(tmp_bdd);
      neg = neg ^ Cudd_IsComplement(tmp_bdd);
      tmp_bdd = Cudd_Regular(tmp_bdd);
      min++;
    } while (!cuddIsConstant(tmp_bdd));
    if(tmp_bdd == Cudd_NotCond(one, neg)) min = 0;
    else min = bdd_length - min;      
    
    fprintf(fout, "not(minmax(%ju, %ju, %s", min, max, s_name(bdd_vars[0]));
    for(uintmax_t y = 1; y < bdd_length; y++)
      fprintf(fout, ", %s", s_name(bdd_vars[y]));
    fprintf(fout, "))");
  } else if(fn_type == FN_TYPE_DEP_VAR) {
    uintmax_t var = BM->equ_var->index;
    fprintf(fout, "equ(%s, ", s_name(var));
    DdNode *equ_bdd = Cudd_Cofactor(BM->dd, bdd, BM->equ_var); cuddRef(equ_bdd);
    printBDD_ReduceSpecFunc(BM, equ_bdd, fout);
    Cudd_IterDerefBdd(BM->dd, equ_bdd);
    fprintf(fout, ")");
  } else {
    fprintf(stderr, "UNIMPLEMENTED FUNCTION TYPE\n");
    assert(0);
  }
  return;
}

uint8_t printBDDFormat(BDDManager *BM) {
  uint8_t ret = NO_ERROR;
  
  fprintf(foutputfile, "p bdd %ju %ju\n", BM->nNumVariables-1, BM->nNumBDDs);
  fprintf(foutputfile, "order(");
  for(uintmax_t x = 1; x < BM->nNumVariables; x++) {
    if(s_name(x) == NULL) continue;
    fprintf(foutputfile, "%s", s_name(x));
    if(x != BM->nNumVariables-1) fprintf(foutputfile, " ");
  }
  fprintf(foutputfile, ")\n");
  for(uintmax_t x = 0; x < BM->nNumBDDs; x++) {
    printBDD_ReduceSpecFunc(BM, BM->BDDList[x], foutputfile);
    fprintf(foutputfile, "\n");
  }
  
  return ret;
}
