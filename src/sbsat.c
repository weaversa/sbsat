/* 
   Copyright (C) 2014 Sean Weaver

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

int break_after_input;
int do_not_solve;
int count_BDD_solutions_flag;
int random_seed;            /* seed for random */

BDDManager *BM_main;
SmurfManager *SM_main;
funcsat *FS_main;

uint8_t sbsat_main_init(int, char**);
uint8_t sbsat_main_load();
uint8_t sbsat_main();
uint8_t sbsat_main();
uint8_t sbsat_final();
uint8_t sbsat_io_init();
void sbsat_io_free();

int main(int argc, char *argv[]) {
  uint8_t ret = NO_ERROR;
  ret = sbsat_main_init(argc, argv);
  if (ret == NO_ERROR) {
    ret = sbsat_main_load();
    if (ret == NO_ERROR) {
      ret = sbsat_main();
    }
  }
  
  if(ret == SAT_UNKNOWN || ret == SAT || ret == UNSAT) {
    if(ret == UNSAT) {
      d0_printf1("c UNSAT\n");
    } else if(ret == SAT) {
      d0_printf1("c SAT\n");
    } else {
      d0_printf1("c UNKNOWN\n");
    }

    sbsat_stats[STAT_SAT_RESULT] = ret;
    ret = NO_ERROR;
  } else {
    d0_printf2("c PROBLEM %u\n", ret);
  }

  if(ret == NO_ERROR)
    ret = sbsat_final();

  sbsat_stats_f[STAT_F_TOTAL_TIME] = get_runtime();
  sbsat_stats[STAT_ERROR_NUM] = ret;
  sbsat_dump_stats();
  
  return ret;
}

uint8_t sbsat_main_init(int argc, char *argv[]) {
  uint8_t ret = NO_ERROR;

  sbsat_stats[STAT_ERROR_NUM] = NO_ERROR;
  sbsat_stats[STAT_SAT_RESULT] = SAT_UNKNOWN;
  sbsat_stats[STAT_MAX_RAM_USAGE] = 0;

  ret = set_signal_timer();
  if(ret != NO_ERROR) {
    sbsat_stats[STAT_ERROR_NUM] = ret;
    return ret;
  }
  
  init_terminal_in();
  init_terminal_out();
  
  ret = params_parse_cmd_line(argc, argv);
  if (ret != NO_ERROR) return ret;
  
  struct timeval tv1;
  struct timezone1 { //yuck
    int tz_minuteswest; /* minutes west of Greenwich */
    int tz_dsttime;     /* type of dst correction */
  };
  struct timezone1 tzp1;
  gettimeofday(&tv1,&tzp1);
  
  if(random_seed <= 0)
    random_seed = ((tv1.tv_sec & 0177 ) * 1000000) + tv1.tv_usec;
  d2_printf2("Random seed = %d\n", random_seed);
  srandom(random_seed);
  srand(random_seed);
  
  sym_init();
  
  BM_main = (BDDManager *)sbsat_calloc(1, sizeof(BDDManager), 9, "BM_main");
  ret = bdd_init(BM_main, 0);
  if(ret != NO_ERROR) {
    sbsat_stats[STAT_ERROR_NUM] = ret;
    return ret;
  }

  return ret;
}

uint8_t sbsat_io_init() {
  d9_printf1("sbsat_io_init\n");
  
  /*
   * open the output file
   */
  if (!strcmp(outputfile, "-")) foutputfile = stdout;
  else {
    struct stat buf;
    if (stat(outputfile, &buf) == 0) {
      dE_printf2("Error: File %s exists\n", outputfile);
      sbsat_stats[STAT_ERROR_NUM] = ERR_IO_INIT;
      return ERR_IO_INIT;
    }
    foutputfile = fopen(outputfile, "w");
  }
  
  if (!foutputfile) {
    dE_printf2("Can't open the output file: %s\n", outputfile);
    sbsat_stats[STAT_ERROR_NUM] = ERR_IO_INIT;
    return ERR_IO_INIT;
  } else d9_printf2("Output file opened: %s\n", outputfile);
  
  return NO_ERROR;
}

uint8_t sbsat_main_load() {
  uint8_t ret = NO_ERROR;
  
  ret = sbsat_io_init();
  if (ret != NO_ERROR) return ret;
  
  /* read input file, generating BDDs */
  
  ret = read_input(BM_main);
  if (ret != NO_ERROR) return ret;
  
  ret = bdd_read_input_finished(BM_main);
  if (ret != NO_ERROR) return ret;

  if(break_after_input == 1) return SAT_UNKNOWN;

  return ret;
}

uint8_t sbsat_main() {
  uint8_t ret = SAT_UNKNOWN;
  
  d2_printf2("Number of BDDS = %ju\n", BM_main->nNumBDDs);
  
  /*  
  d2_printf1("Searching for function types\n");
  for(uintmax_t x = 0; x < BM_main->nNumBDDs; x++) {
    uint8_t fn_type = findandret_fnType(BM_main, BM_main->BDDList[x]);
  }
  d2_printf1("Done searching for function types\n");
  */

  //Clustering test

  for(uint32_t i = 0; i <= (uint32_t)ve_restarts; i++) {
    //if((i+1)%10 == 0) bdd_cluster_limit++;

    BDDManager *BM_clone = (BDDManager *)sbsat_calloc(1, sizeof(BDDManager), 9, "BM_clone");
    if(BM_clone == NULL) return MEM_ERR;
    ret = bdd_manager_clone(BM_clone, BM_main);
    if(ret != NO_ERROR) return ret;
    ret = bdd_read_input_finished(BM_clone);
    if(ret != NO_ERROR) return ret;
    ret = VE_auto(BM_clone);
    if(ret != NO_ERROR) return ret;

    //d2_printf1("\nCopying inferences to old manager\n");
    ret = bdd_manager_clone_inferences(BM_main, BM_clone);
    if(ret != NO_ERROR) return ret;
    //d2_printf1("\nDone Copying inferences to old manager\n");

    ret = bdd_free(BM_clone);
    if(ret != NO_ERROR) return ret;
  }    

  if(count_BDD_solutions_flag) {
    mpz_t *num = count_BDD_solutions(BM_main);
    if(num == NULL) {
      fprintf(stdout, "Error counting the number of solutions...exiting\n");
      return SAT_UNKNOWN;
    }

    char *count = mpz_get_str(NULL, 10, *num);
    fprintf(stdout, "Number of paths to true is %s\n", count);
    free(count);

    //if(count > 0) then ret = SAT else ret = UNSAT //SEAN!!!

    mpz_clear(*num);
    free(num);
  }

  if(ret != NO_ERROR && ret != SAT_UNKNOWN) return ret;

  if(do_not_solve != 1) {

    if(solver_flag == 's') {
      //Smurf solve
      
      //fprintf(stderr, "Solving with Smurf Solver\n");

      //Init funcsat manager - testing an idea
      funcsat_config *FS_config = funcsatConfigInit(NULL);
      FS_config->minimizeLearnedClauses = false;
      FS_config->debugStream = stddbg;
      //FS_config->isTimeToRestart = funcsatNoRestart;
      //FS_config->numUipsToLearn = 1;
      FS_main = funcsatInit(FS_config);
      
      funcsatDebug(FS_main, "find_uips", 9);
      
      funcsat_result FS_result;
      SM_main = Init_SmurfSolver(BM_main, FS_main);
      if(SM_main == NULL) {
	FS_result = FS_main->lastResult;
      } else {
      
	sbsat_stats_f[STAT_F_SOLVER_START_TIME] = get_runtime();
	
	FS_result = SmurfSolve(SM_main);

	sbsat_stats_f[STAT_F_SOLVER_TOTAL_TIME] = get_runtime() - sbsat_stats_f[STAT_F_SOLVER_START_TIME];

	d7_printf2("funcsat says: %s\n", funcsatResultAsString(FS_result));
	
	//Release funcsat manager
	funcsatReset(FS_main);
	FS_main->Backtrack_hook((uintptr_t *)SM_main, -1);
	funcsatConfigDestroy(FS_main->conf);
	funcsatDestroy(FS_main);
	
	ret = Final_SimpleSmurfSolver(SM_main);
      }

      if(FS_result == FS_UNKNOWN) {
	ret = SAT_UNKNOWN;
      } else if(FS_result == FS_SAT) {
	ret = SAT;
      } else if(FS_result == FS_UNSAT) {
	ret = UNSAT;
      }

    } else if(solver_flag == 'f') {
      //funcsat solve

      //fprintf(stderr, "Solving with Funcsat\n");
      
      //Init funcsat manager - testing an idea
      funcsat_config *FS_config = funcsatConfigInit(NULL);
      FS_config->minimizeLearnedClauses = false;
      FS_config->debugStream = stddbg;
      //FS_config->isTimeToRestart = funcsatNoRestart;
      //FS_config->numUipsToLearn = 1;
      FS_main = funcsatInit(FS_config);
      
      funcsatDebug(FS_main, "find_uips", 0);

      FS_main->lastResult = FS_UNKNOWN;
      SM_main = Init_SmurfSolver_funcsat(BM_main, FS_main);
      if(SM_main != NULL) {
	sbsat_stats_f[STAT_F_SOLVER_START_TIME] = get_runtime();
      
	if(FS_main->lastResult == FS_UNKNOWN)
	  funcsatSolve(FS_main);
	d7_printf2("funcsat says: %s\n", funcsatResultAsString(FS_main->lastResult));

	sbsat_stats_f[STAT_F_SOLVER_TOTAL_TIME] = get_runtime() - sbsat_stats_f[STAT_F_SOLVER_START_TIME];
      }
      
      if(FS_main->lastResult == FS_UNKNOWN) {
	ret = SAT_UNKNOWN;
      } else if(FS_main->lastResult == FS_SAT) {
	ret = SAT;
      } else if(FS_main->lastResult == FS_UNSAT) {
	ret = UNSAT;
      }

      
      //Release funcsat manager
      funcsatReset(FS_main);
      if(SM_main != NULL) FS_main->Backtrack_hook((uintptr_t *)SM_main, -1);
      funcsatConfigDestroy(FS_main->conf);
      funcsatDestroy(FS_main);
      
      if(SM_main != NULL) Final_SimpleSmurfSolver(SM_main);

    } else if(solver_flag == 'p') {
      PicosatManager *PM_main = (PicosatManager *)sbsat_calloc(1, sizeof(PicosatManager), 9, "PicosatManager");
      SM_main = Init_SmurfSolver_picosat(BM_main, PM_main);

      sbsat_stats_f[STAT_F_SOLVER_START_TIME] = get_runtime();

      int PM_result = picosatSolve(PM_main);

      sbsat_stats_f[STAT_F_SOLVER_TOTAL_TIME] = get_runtime() - sbsat_stats_f[STAT_F_SOLVER_START_TIME];

      SM_main->ESSM = NULL;
      sbsat_free((void **)&PM_main);
      ret = Final_SimpleSmurfSolver(SM_main);
      ret = PM_result;
    } else {
      fprintf(stderr, "no such solver '%c'\n", solver_flag);
    }

  }
  
  return ret;
}

uint8_t sbsat_main_free() {
  
  uint8_t ret = NO_ERROR;
  
  //Free any memory allocated in sbsat_main()
  
  return ret;
}

uint8_t sbsat_final() {
  uint8_t ret = NO_ERROR;
  
  //Write output
  write_output(BM_main);

  sbsat_main_free();
  
  sbsat_io_free();
  
  bdd_free(BM_main);
  sbsat_free((void**)&BM_main);
  
  sym_free();
  
  d2_printf2("Total Time: %4.3fs\n", get_runtime());
  
  free_terminal_in();
  free_terminal_out();
  
  return ret;
}

void sbsat_io_free() {
  d9_printf1("sbsat_io_free\n");
  if (finputfile && finputfile != stdin) fclose(finputfile);
  if (foutputfile && foutputfile != stdout && foutputfile != stderr) fclose(foutputfile);
  //if (fresultfile != stdin && fresultfile) fclose(fresultfile);
}
