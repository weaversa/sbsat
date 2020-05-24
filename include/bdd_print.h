/*
  *     sbsat - SBSAT is a state-based Satisfiability solver.
  *  
  *     Copyright (C) 2010 Sean Weaver
  *  
  *     This program is free software; you can redistribute it and/or modify
  *     it under the terms of the GNU General Public License as published by
  *     the Free Software Foundation; either version 2, or (at your option)
  *     any later version.
  *  
  *     This program is distributed in the hope that it will be useful,
  *     but WITHOUT ANY WARRANTY; without even the implied warranty of
  *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *     GNU General Public License for more details.
  *  
  *     You should have received a copy of the GNU General Public License
  *     along with this program; if not, write to the Free Software Foundation,
  *     Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
  * */

#ifndef BDD_PRINT_H
#define BDD_PRINT_H

#include "sbsat.h"

void printBDDSupport(BDDManager *BM, DdNode *bdd);

void printBDD(DdManager *dd, DdNode *bdd);
void printBDDerr(DdManager *dd, DdNode *bdd);

void printBDDAND(DdManager *dd, DdNode *bdd, FILE *fout);
void printBDDOR(DdManager *dd, DdNode *bdd, FILE *fout);

void printBDD_ReduceSpecFunc_latex(BDDManager *BM, DdNode *bdd, FILE *fout);
void printBDD_ReduceSpecFunc(BDDManager *BM, DdNode *bdd, FILE *fout);
uint8_t printBDDFormat(BDDManager *BM);

#endif
