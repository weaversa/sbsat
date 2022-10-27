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

char inputfile[256] = "-";
char outputfile[256] = "-";

FILE *finputfile = NULL;
FILE *foutputfile = NULL;
char temp_dir[256] = "";
char ini_filename[256] = "";
uint8_t result_display_type = 0;
char input_result_filename[256] = "";

int PARAMS_DUMP = 0;

char comment[1024]="";
