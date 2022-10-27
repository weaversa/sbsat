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

//Clause *pClauses;
uintmax_t nNumClauses;
uintmax_t nNumCNFVariables;

uint8_t getNextSymbol_CNF (intmax_t *intnum) {
  int p = 0;
  while(1) {
    p = fgetc(finputfile);
    if(p == EOF) return ERR_IO_READ;
    else if(((p >= '0') && (p <= '9')) || (p == '-')) {			
      ungetc(p, finputfile);
      if(fscanf(finputfile, "%jd", &(*intnum)) != 1) {
	return ERR_IO_UNEXPECTED_CHAR;
      }
      d9_printf2("%jd", (*intnum));
      return NO_ERROR;
    } else if(p == 'c') {
      while(p != '\n') {
	d9_printf2("%c", p);
	p = fgetc(finputfile);
	if(p == EOF) {
	  return ERR_IO_READ;
	}
      }
      d9_printf2("%c", p);
      p = fgetc(finputfile);
      if (p == EOF) {
	return ERR_IO_READ;
      }
      ungetc(p, finputfile);
      continue;
    } else if(p == 'p') {
      d9_printf2("%c", p);
      return IO_CNF_HEADER;
    } else if(p >='A' && p<='z') {
      d9_printf2("%c", p);
      return ERR_IO_UNEXPECTED_CHAR;
    }
    d9_printf2("%c", p);
  }
}

uint8_t read_cnf(CNF_Struct *CNF) {
  
  uint8_t ret = NO_ERROR;

  sbsat_stats[STAT_RAM_USAGE_BEFORE_READING_CNF] = get_memusage()/1000000;
  
  if(feof(finputfile)) {
    return ERR_IO_READ;
  }
  
  uintmax_t nNumVariables;
  uintmax_t nNumClauses;
  intmax_t next_symbol;

  if (fscanf(finputfile, "%ju %ju\n", &nNumVariables, &nNumClauses) != 2) {
    fprintf(stderr, "Error while parsing CNF input: bad header %jd %jd\n", nNumVariables, nNumClauses);
    return ERR_IO_READ;
  }

  d10_printf3("p cnf %ju %ju\n", nNumVariables, nNumClauses);

  uintmax_t max_int = (~0)>>1;
  if(nNumVariables > max_int) {
    fprintf(stderr, "Error while parsing CNF input: Number of variables is larger than the maximum number allowed (%ju).\n", max_int);
    return ERR_IO_READ;
    //  } else if(nNumVariables == 0) {
    //    fprintf(stderr, "Error while parsing CNF input: Number of variables is 0.\n");
    //    return ERR_IO_READ;
  }
  
  if(nNumClauses > max_int) {
    fprintf(stderr, "Error while parsing CNF input: Number of clauses is larger than the maximum number allowed (%ju).\n", max_int);
    return ERR_IO_READ;
  } else if(nNumClauses == 0) {
    //fprintf(stderr, "Error while parsing CNF input: Number of clauses is 0.\n");
    //return ERR_IO_READ;
    return ret;
  }
  
  uintmax_t nOrigNumClauses = nNumClauses;
  uintmax_t nNumVariablesFound = 0;
  
  Clause *pClauses = (Clause *)sbsat_calloc(nNumClauses, sizeof(Clause), 9, "read_cnf()::pClauses");
  CNF->pClauses = pClauses;
  CNF->nNumClauses = nNumClauses;

  uintmax_t tempint_max = 100;
  intmax_t *tempint = (intmax_t *)sbsat_calloc(tempint_max, sizeof(intmax_t), 9, "read_cnf()::tempint");
  
  //Get and store the CNF clauses
  uint8_t print_variable_warning = 1;
  uintmax_t x = 0;
  while(1) {
    if (x%10000 == 0) {
      d2_printf3("\rReading CNF %ju/%ju ... ", x, nNumClauses);
    }
    
    ret = getNextSymbol_CNF(&next_symbol);
    if(ret == ERR_IO_UNEXPECTED_CHAR) {
      fprintf(stderr, "\nError while parsing CNF input: Clause %ju\n", x);
      return ret;
    }
    
    if(x == nNumClauses) { //Should be done reading CNF
      if(ret == ERR_IO_READ) { ret = NO_ERROR; break; } //CNF has been fully read in
      if(next_symbol == 0) break; //Extra 0 at end of file
      fprintf(stderr, "Warning while parsing CNF input: more than %ju functions found\n", nOrigNumClauses);
      break;
    }
    
    if (ret != NO_ERROR) {
      fprintf(stderr, "Warning while parsing CNF input: premature end of file, only %ju functions found\n", x);
      ret = NO_ERROR;
      nNumClauses = x;
      break;
    } else {
      uintmax_t y = 0;
      while(1) {
	if (next_symbol == 0) break; //Clause has been terminated
	if (y >= tempint_max) {
	  tempint = (intmax_t *)sbsat_recalloc((void*)tempint, tempint_max, tempint_max+100, sizeof(intmax_t), 9, "read_cnf()::tempint");
	  tempint_max += 100;
	}
	tempint[y] = next_symbol;
	if((uintmax_t) imaxabs(tempint[y]) > nNumVariables) {
	  if(print_variable_warning) {
	    d2e_printf1("Warning while parsing CNF input: There are more variables in the input file than specified\n");
	    print_variable_warning = 0;
	  }
	}
	if(((uintmax_t) imaxabs(tempint[y])) > nNumVariablesFound) {
	  nNumVariablesFound = (uintmax_t) imaxabs(tempint[y]);
	}
        
	ret = getNextSymbol_CNF(&next_symbol);
	if (ret != NO_ERROR) {
	  fprintf(stderr, "Error while parsing CNF input: Clause %ju\n", x);
	  return ret;
	}
	y++;
      }
      if(y==0) {
	ret = UNSAT;
	break; //A '0' line -- the empty clause - formula is unsat
      }
      if(x >= nNumClauses) {
	d2e_printf1("Warning while parsing CNF input: There are more clauses in the input file than specified - ignoring the rest\n");
	break;
      }
      pClauses[x].length = y;
      pClauses[x].variables = (intmax_t *)sbsat_calloc(y, sizeof(intmax_t), 9, "read_cnf()::pClauses[x].variables");
      pClauses[x].subsumed = 0;
      pClauses[x].flag = 0;
      memcpy_sbsat(pClauses[x].variables, tempint, y*sizeof(intmax_t));
      x++;
    }
    if(x >= nNumClauses) {
      break;
    }
  }
  
  d2_printf3("\rReading CNF %ju/%ju            \n", nNumClauses, nNumClauses);
  
  if(nNumVariablesFound < nNumVariables) {
    d2e_printf2("Warning while parsing CNF input: There are less variables in input file than specified, only found %ju\n", nNumVariablesFound);
  }
  
  nNumVariables = nNumVariablesFound;   
  
  sbsat_free((void **)&tempint); tempint_max = 0;
  
  d2_printf2("Number of Variables - %ju\n", nNumVariables);
  d2_printf2("Number of Clauses - %ju\n", nNumClauses);
  
  CNF->nNumVariables = nNumVariables;
  CNF->nNumVariables_nosym = nNumVariables;
  CNF->nNumClauses = nNumClauses;

  if(ret == NO_ERROR) {
    ret = compress_variables(CNF);
    if(ret != NO_ERROR) return ret;
  }
  
  sbsat_stats[STAT_CNF_NUM_CLAUSES] = CNF->nNumClauses;
  sbsat_stats[STAT_CNF_NUM_VARIABLES] = CNF->nNumVariables;
  sbsat_stats[STAT_RAM_USAGE_AFTER_READING_CNF] = get_memusage()/1000000;
  sbsat_stats_f[STAT_F_CNF_READ_TIME] = get_runtime();

  return ret;
}
