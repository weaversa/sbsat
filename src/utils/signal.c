/* 
   Copyright (C) 2011 Sean Weaver

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
#include <signal.h>

void handle_sigalrm(int unused) {
  d6_printf1("\nChecking limits ... ");

  if(signal(SIGALRM, handle_sigalrm) == SIG_ERR) {
    fprintf(stderr, "Unable to catch SIGALRM\n");
    return;
  }

  if(check_limits()!=NO_ERROR) {

    //Turn off timer
    struct itimerval it_val; // for setting itimer
    it_val.it_interval.tv_sec = 0;
    it_val.it_interval.tv_usec = 0;
    it_val.it_value.tv_sec = 0;
    it_val.it_value.tv_usec = 0;
    if(setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
      fprintf(stderr, "Error disabling setitimer\n");
      sbsat_stats[STAT_ERROR_NUM] = ERR_SIG_INIT;
    }

    d2_printf1("Limit hit, exiting...\n");
    d0_printf1("c UNKNOWN\n");

    sbsat_stats_f[STAT_F_TOTAL_TIME] = get_runtime();
    sbsat_dump_stats();

    exit(0);
  }

  if(!feof(stdin)) {
    char c = '0';//fgetc(stdin);
    if(c == 'q') {
      d2_printf1("\n");
      sbsat_stats[STAT_ERROR_NUM] = ERR_SIG_USER_QUIT;
      sbsat_stats_f[STAT_F_TOTAL_TIME] = get_runtime();
      sbsat_dump_stats();
      
      d2_printf1("Exiting...\n");
      exit(0);
    }
  }
}

uint8_t set_signal_timer() {
  uint8_t ret = NO_ERROR;

  if(signal(SIGALRM, handle_sigalrm) == SIG_ERR) {
    fprintf(stderr, "Unable to catch SIGALRM\n");
    return ERR_SIG_INIT;
  }

  struct itimerval it_val; // for setting itimer
  it_val.it_interval.tv_sec = 1;
  it_val.it_interval.tv_usec = 0;
  it_val.it_value.tv_sec = 1;
  it_val.it_value.tv_usec = 0;
  if(setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
    fprintf(stderr, "Error calling setitimer\n");
    return ERR_SIG_INIT;
  }

  return ret;
}
