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

uint8_t input_format = '0';
void *input_object = NULL;

uint8_t read_input_open() {
   //open the input file 

   if (!strcmp(inputfile, "-")) {
      d2_printf2("Reading standard input %s...\n", comment);
   } else {
      d1_printf3("Reading File %s %s...\n", inputfile, comment);
   }
   
   int zip_type = check_gzip(inputfile);
   if (zip_type) {
      d2_printf1("gzip file -- using zread\n");
      finputfile = zread(inputfile, zip_type);
   } else if (!strcmp(inputfile, "-")) {
      finputfile = stdin;
   } else {
      finputfile = fopen(inputfile, "r");
   }
   
   if (!finputfile) {
      dE_printf2("Can't open the input file: %s\n", inputfile);
      return ERR_IO_INIT;
   } else d9_printf2("Input file opened: %s\n", inputfile);
 
   return NO_ERROR;
}

uint8_t auto_detect_input() {

   uint8_t format, a;
   
   if (feof(finputfile)) return 0;
   
   a = fgetc (finputfile);
   d9_printf3("getformat(FILE): first character: %c(0x%x)\n", a, a);

   // check for DIMACS p '   ' # #
   if (feof(finputfile)) return 0;
   while (a == 'c' || a == '\n' || a == ';') { //skip past comments
      while (a != '\n') {
         a = fgetc (finputfile);
         if (feof(finputfile)) return 0;
      }
      a = fgetc (finputfile);
      if (feof(finputfile)) return 0;
   }
   if (a == 'p') { //possibly a p '   ' # # format
     a = fgetc (finputfile); // space
     if (feof(finputfile) || a != ' ') return 0;
     format = fgetc (finputfile); // format 
     if (feof(finputfile)) return 0;
     a = fgetc (finputfile); // format
     if (feof(finputfile)) return 0;
     a = fgetc (finputfile); // format
     if (feof(finputfile)) return 0;
     a = fgetc (finputfile); // space
     if (feof(finputfile) || a != ' ') return 0;
   } else if(a == 'a') {
     //possibly an AIG
     ungetc(a, finputfile);
     format = 'a';
   } else {
     format = 0;
   }
   return format; //c for CNF d for DNF s for SAT, b for BDD's, x for XOR, a for AIG
}

uint8_t read_input(BDDManager *BM) {
  uint8_t ret = NO_ERROR;

   d9_printf1("read_input\n");
   
   ret = read_input_open();
   if (ret != NO_ERROR) return ret;
  
   input_format = auto_detect_input();
   
   switch (input_format) {
     case 'n': { return ret; } break; //Do nothing (default)

     case 'b': { ret = iteloop(BM);
                 if(ret != NO_ERROR) return ret; } break;

     case 'c': { CNF_Struct CNF;
                 input_object = (void *)&CNF;
                 ret = read_cnf(&CNF);
                 if (ret != NO_ERROR) {
                   return ret;
                 }
                 ret = process_cnf(&CNF, BM);
                 if (ret != NO_ERROR) return ret;
                 free_clauses(&CNF);
                 input_object = NULL; } break;
     case 'a': { ret = process_aiger(BM);
                 if(ret != NO_ERROR) return ret; } break;
     default:
       fprintf(stderr, "Unknown input format '%c'\n", input_format);
       ret = ERR_IO_READ;
   };

   d9_printf1("read_input - done\n");
   
   return ret;
}
