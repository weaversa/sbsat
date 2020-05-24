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

uint8_t write_clause(CNF_Struct *CNF, uintmax_t clause_num) {
  uint8_t ret = NO_ERROR;
  
  if(CNF->pClauses[clause_num].subsumed) fprintf(foutputfile, "s ");
  for(uintmax_t y = 0; y < CNF->pClauses[clause_num].length; y++) {
    if(CNF->pClauses[clause_num].variables[y]<0) fprintf(foutputfile, "-");
    fprintf(foutputfile, "%s ", getsym_i(abs(CNF->pClauses[clause_num].variables[y]))->name);
  }
  fprintf(foutputfile, "0\n");
  
  return ret;
}

uint8_t write_cnf(CNF_Struct *CNF) {
  uint8_t ret = NO_ERROR;
  
  fprintf(foutputfile, "p cnf %ju %ju\n", CNF->nNumVariables_nosym, CNF->nNumClauses);
  
  for(uintmax_t x = 0; x < CNF->nNumClauses; x++) {
    if(CNF->pClauses[x].subsumed) continue;
    ret = write_clause(CNF, x);
    if(ret != NO_ERROR) return ret;
  }
  
  return ret;
}
