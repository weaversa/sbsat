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

uint8_t output_format = 'n';

uint8_t write_output(BDDManager *BM) {
   
   uint8_t ret = NO_ERROR;

   switch (output_format) {
    case 'n': { return ret; } //Do nothing (default)

    case 'b': { ret = printBDDFormat(BM);
                if(ret != NO_ERROR) return ret; } break;
    
    case 'c': { //ret = printBDDFormat(BM);
                if(ret != NO_ERROR) return ret; } break;
    
    default:
      fprintf(stderr, "Unknown output format '%c'\n", output_format);
   };
   
   return ret;
}
