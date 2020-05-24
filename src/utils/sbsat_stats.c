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

long double sbsat_stats_f[STAT_F_MAX];
uintmax_t sbsat_stats[STAT_MAX];
char sbsat_stats_file[256] = "";
char sbsat_display_stat[256] = "";

void sbsat_dump_stats() {
  if(strcmp(sbsat_stats_file, "")==0) return;

  FILE *fstatsfile;

  //Correct sbsat_display_stat
  if(sbsat_display_stat[0] == 0) {
    for(uintmax_t i = 0; i < 256; i++) {
      sbsat_display_stat[i] = '1';
    }
  } else {
    uint8_t display_stat = 1;
    for(uintmax_t i = 0; i < 256; i++) {
      if(display_stat!=1) sbsat_display_stat[i] = '0';
      else if(sbsat_display_stat[i] != '0') {
	if(sbsat_display_stat[i] != '1') {
	  if(i > STAT_F_MAX+STAT_MAX) {
	    fprintf(stderr, "'display-stat' argument string is too long\n");
	    assert(0);
	  }
	  display_stat = 0;
	  sbsat_display_stat[i] = '0';
	}
      }
    }
  }

  if(strcmp(sbsat_stats_file, "-")==0) {
    //write to stdout
    fstatsfile = stdout;
  } else {
    fstatsfile = fopen(sbsat_stats_file, "a");
    d4_printf2("Appending stats to %s\n", sbsat_stats_file);
  }

  if(fstatsfile == NULL) {
    fprintf(stderr, "Invalid 'stats-filename'\n");
    return;
  }

  if(finputfile != stdin) fprintf(fstatsfile, "%s", inputfile);
  else fprintf(fstatsfile, "stdin");

  for(uintmax_t i = 0; i < STAT_F_MAX; i++) {
    if(sbsat_display_stat[i]=='1') fprintf(fstatsfile, ", %Lf", sbsat_stats_f[i]);
  }

  for(uintmax_t i = 0; i < STAT_MAX; i++)
    if(sbsat_display_stat[STAT_F_MAX+i]=='1') fprintf(fstatsfile, ", %ju", sbsat_stats[i]);

  fprintf(fstatsfile, "\n");
  
  fclose(fstatsfile);
}
