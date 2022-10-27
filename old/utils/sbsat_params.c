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
#include "sbsat_params.h"

extern int params_current_src;

void ctrl_c_proc(int x);

uint8_t ctrl_c=0;
uint8_t nCtrlC=0; /* ctrl c pressed */

char debug_dev[256]="stderr";

void fn_parse_filename(char *filename);

t_opt options[] = { 
// p_target, l_opt, w_opt, p_type, p_value, p_defa, var_type, p_src=0, desc_opt

/* 
 * General SBSAT options 
 */
{ (void *)show_help, "h", "help", P_FN, V(i:0,"0"), V(i:0,"0"), VAR_CMD, 0,
     "Show all program options"},
{ (void *)show_version, "", "version", P_FN, V(i:0,"0"), V(i:0,"0"), VAR_CMD, 0,
     "Show program version"},
{ (void *)show_ini, "", "create-ini", P_FN, V(i:0,"0"), V(i:0,"0"), VAR_CMD, 0,
     "Create ini file"},
{ &ini_filename, "", "ini", P_STRING, V(i:255,"255"), {"~/sbsat.ini"}, VAR_CMD+VAR_DUMP, 0,
     "Set the ini file"},
{ &DEBUG_LVL, "", "debug", P_INT, V(i:0,"0"), V(i:2,"2"), VAR_NORMAL, 0,
     "debugging level (0-none, 9-max)"},
{ &debug_dev, "", "debug-dev", P_STRING, V(i:255,"255"), {"stderr"}, VAR_NORMAL, 0,
     "debugging device"},
{ &PARAMS_DUMP, "D", "params-dump", P_PRE_INT, V(i:1,"1"), V(i:0,"0"), VAR_CMD, 0,
     "dump all internal parameters before processing"},
{ &random_seed, "seed", "random-seed", P_INT, V(i:0,"0"), V(i:0,"0"), VAR_NORMAL, 0,
     "Random seed to use (0 generates a random seed)"},
{ sbsat_stats_file, "", "stats-file", P_STRING, V(i:255,"255"), {""}, VAR_NORMAL, 0,
     "File to appends stats in CSV format, use \"-\" for stdout"},
{ sbsat_display_stat, "", "display-stats", P_STRING, V(i:255,"255"), {""}, VAR_NORMAL, 0,
     "A debugging flag used to print only certain stats, ex) \"00000001\" prints only the total time"},
{ &sbsat_max_ram, "ram", "max-ram-usage", P_INT, V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0,
     "Abort if sbsat uses more than this much RAM (in megs) (0 for no limit)"},
{ &sbsat_max_runtime, "time", "max-runtime", P_FLOAT, V(i:0,"0"), V(f:0.0,"0.0"), VAR_NORMAL, 0,
     "Abort if sbsat runs longer than this limit (in seconds) (0 for no limit)"},
{ inputfile, "", "input-file", P_STRING, V(i:255,"255"), {"-"}, VAR_NORMAL, 0,
     "input filename"},
{ outputfile, "", "output-file", P_STRING, V(i:255,"255"), {"-"}, VAR_NORMAL, 0, 
     "output filename"},
{ temp_dir, "", "temp-dir", P_STRING, V(i:255,"255"), {"$TEMP"}, VAR_NORMAL, 0,
     "directory for temporary files"},
{ (void *)fn_parse_filename, "", "parse-filename", P_FN_STRING, V(i:0,"0"), V(i:0,"0"), VAR_CMDLINE+VAR_DUMP, 0,
     ""},
{ &break_after_input, "eapi", "exit-after-parsing-input", P_PRE_INT, V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0,
     "Exit after parsing input file - this is useful while developing new input formats and displaying BDDs and Smurfs from the ite format"},
{ &do_not_solve, "dns", "do-not-solve", P_PRE_INT, V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0,
     "Do not call any solver, but do read and preprocess the input and potentially perform translations to various output formats"},
{ &count_BDD_solutions_flag, "count", "count-solutions", P_PRE_INT, V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0,
     "Conjoin all BDDs left after VE into the monolithic BDD and count the number of its solutions"},
{ &solver_flag, "s", "solver", P_CHAR, V(c:'s',"s"), V(c:'s',"s"), VAR_NORMAL, 0,
    "Call a specific solver (SMURF = s, Funcsat = f)" },

/* 
 * Preprocessing options
 */
{ NULL, "", "", P_NONE, {"0"}, {"0"}, VAR_NORMAL, 0, 
     "\nPreprocessing options:"},
{ functionTypeLimits+FN_TYPE_AND_EQU, "", "limit-and-equ", P_INT, V(i:0,"0"), V(i:2,"2"), VAR_NORMAL, 0,
     "The minimum # of literals to flag special function and_equ"},
{ functionTypeLimits+FN_TYPE_OR_EQU, "", "limit-or-equ", P_INT, V(i:0,"0"), V(i:2,"2"), VAR_NORMAL, 0,
     "The minimum # of literals to flag special function or_equ"},
{ functionTypeLimits+FN_TYPE_OR, "", "limit-or", P_INT, V(i:0,"0"), V(i:2,"2"), VAR_NORMAL, 0,
     "The minimum # of literals to flag special function or"},
{ functionTypeLimits+FN_TYPE_AND, "", "limit-and", P_INT, V(i:0,"0"), V(i:2,"2"), VAR_NORMAL, 0,
     "The minimum # of literals to flag special function and"},
{ functionTypeLimits+FN_TYPE_XOR, "", "limit-xor", P_INT, V(i:0,"0"), V(i:2,"2"), VAR_NORMAL, 0,
     "The minimum # of literals to flag special function xor"},
{ functionTypeLimits+FN_TYPE_MINMAX, "", "limit-minmax", P_INT, V(i:0,"0"), V(i:3,"3"), VAR_NORMAL, 0,
     "The minimum # of literals to flag special function minmax"},
{ functionTypeLimits+FN_TYPE_NEGMINMAX, "", "limit-neg-minmax", P_INT, V(i:0,"0"), V(i:3,"3"), VAR_NORMAL, 0,
     "The minimum # of literals to flag special function negative minmax"},
{ &functionTypeLimits_all, "", "limit-all", P_INT, V(i:0,"0"), V(i:-1,"-1"), VAR_NORMAL, 0,
     "Set the limit of all special functions to this value (-1 to inactive this command)"},
{ &find_cnf_gates_flag, "f", "find-known-gates", P_PRE_INT, V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0,
     "Attempt to discover gates in the input CNF"},
{ &cluster_cnf_flag, "dcc", "dont-cluster-cnf", P_PRE_INT, V(i:0,"0"), V(i:1,"1"), VAR_NORMAL, 0,
     "Turn off functionality that attempts to partially cluster the input CNF (only active when 'find-known-gates' is used)"},
{ &clause_cluster_overlap, "cco", "clause-cluster-overlap", P_INT, V(i:0,"0"), V(i:0,"0"), VAR_NORMAL, 0,
     "Cluster two clauses if they differ in at most 'clause-cluster-overlap' literals (used in conjunction with 'cluster-cnf')"},
{ &clause_cluster_max_length, "ccl", "clause-cluster-length", P_INT, V(i:0,"0"), V(i:300,"300"), VAR_NORMAL, 0,
     "Cluster two clauses if they both have less than 'clause-cluster-length' literals (used in conjunction with 'cluster-cnf')"},
{ &ve_iterations, "ve", "ve-iterations", P_INT, V(i:0,"0"), V(i:0,"0"), VAR_NORMAL, 0,
     "Number of iterations to perform with VE (BDD clustering) (-1 for max iterations)"},
{ &ve_restarts, "vr", "ve-restarts", P_INT, V(i:0,"0"), V(i:0,"0"), VAR_NORMAL, 0,
     "Number of times to restart VE (-1 for max iterations)"},
{ &bdd_cluster_limit, "cvl", "cluster-var-limit", P_INT, V(i:0,"-1"), V(i:0,"-1"), VAR_NORMAL, 0,
     "Don't cluster two BDDs if their conjunction will contain more than this many variables (-1 for no limit)"},
{ &ve_heuristic, "vh", "ve-heuristic", P_INT, V(i:0,"5"), V(i:0,"5"), VAR_NORMAL, 0,
     "Heuristic used to compute quantification schedule for VE (Static=0, Random=1, CUDD=2, FORCE=3, Overlap=4, VarScore=5, Optimal=6)"},
{ &pairwise_bdd_reduction, "pwbr", "pairwise-bdd-reduction", P_INT, V(i:0,"0"), V(i:0,"0"), VAR_NORMAL, 0,
     "Pairwise BDD reduction algorithm used during clustering (None=0, Restrict=1, LI Compaction=2, NPAnd=3)"},
{ preset_variables_string, "", "preset-variables", P_STRING, V(i:4096,"4096"), {""}, VAR_NORMAL, 0,
  "Variables forced during preprocessing. Options are ([[+var|-var] ]*)"},


/* 
 * Output options
 */
{ NULL, "", "", P_NONE, {"0"}, {"0"}, VAR_NORMAL, 0, 
     "\nOutput options (can be used collectively):"},
{ &write_gates_flag, "g", "write-gates", P_PRE_INT, V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0,
     "Output the gates found during CNF processing"},
{ &write_cnf_flag, "", "write-cnf", P_PRE_INT, V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0,
     "Output the CNF after CNF specific processing"},


/* 
 * BDD options
 */
{ NULL, "", "", P_NONE, {"0"}, {"0"}, VAR_NORMAL, 0, 
     "\nBDD options:"},
{ &Cudd_var_ordering_algType, "", "bdd-ordering", P_INT, V(i:0,"0"), V(i:0,"0"), VAR_NORMAL, 0,
     "BDD variable ordering heuristic (0 for none, 20-max)"},
{ &apply_BDD_inferences, "inf", "bdd-inferences", P_PRE_INT, V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0,
     "Search for and apply inferences to BDDs"},
{ &apply_BDD_equivalences, "equ", "bdd-equivalences", P_PRE_INT, V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0,
     "Search for and apply equivalences to BDDs (only active when 'bdd-inferences' is used"},
{ &safe_BDD_assignments, "safe", "safe-assignmets", P_PRE_INT, V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0,
     "Search for and apply safe assignments to BDDs"},
{ &early_quantify_BDD_vars, "exq", "early-quantify", P_PRE_INT, V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0,
     "Existentially quantify a variable away from a BDD when the variable exists in only one BDD"},
{ &sbsat_turn_off_bdds, "bdds-off", "bdds-off", P_PRE_INT, V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0,
     "A debugging flag to turn off BDDs"},


/* 
 * SMURF options
 */
{ NULL, "", "", P_NONE, {"0"}, {"0"}, VAR_NORMAL, 0, 
     "\nSMURF options:"},
{ &use_dropped_var_smurfs, "dvs", "dropped-var-smurfs", P_INT, V(i:0,"0"), V(i:0,"0"), VAR_NORMAL, 0,
     "Use Dropped-Var SMURFs - can aid in SMURF compression but may slow down search"},
{ &turn_off_smurf_normalization, "nsn", "", P_PRE_INT, V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0,
     "Turn off SMURF normalization - a debugging flag (should not be used for solving)"},
{ &find_xor_factors, "xfs", "xor-factor-smurfs", P_PRE_INT, V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0,
     "Use XOR Factor SMURFs - can aid in SMURF compression - only active in conjunction with Gaussian Elimination"},
{ &gelim_smurfs, "ges", "gelim-smurfs", P_PRE_INT, V(i:1, "1"),  V(i:'0', "0"), VAR_NORMAL, 0,
     "Use Gaussian Elimination during search"},
{ &minimize_smurf_clauses, "nmins", "", P_PRE_INT, V(i:0, "0"),  V(i:'1', "1"), VAR_NORMAL, 0,
     "Turn off minimization of clauses produced by Smurfs"},
{ &precompute_smurfs, "pcs", "precompute-smurfs", P_INT, V(i:12,"12"),  V(i:12,"12"), VAR_NORMAL, 0,
    "General Smurfs with more than this many variables will not be precomputed before search"},
{ &sHeuristic, "H", "heuristic", P_CHAR, V(c:'j',"j"), V(c:'j', "j"), VAR_NORMAL, 0,
    "Choose heuristic {n=none, j=LSGB, r=random, s=static}"},
{ &JHEURISTIC_K, "K", "jheuristic-k", P_FLOAT, V(i:0,"0"), V(f:3.0, "3.0"), VAR_NORMAL, 0,
    "set the value of K"},
{ &JHEURISTIC_K_TRUE, "", "jheuristic-k-true", P_FLOAT, V(i:0,"0"), V(f:0.0, "0.0"), VAR_NORMAL, 0,
    "set the value of True state"},
{ &JHEURISTIC_K_INF, "", "jheuristic-k-inf", P_FLOAT, V(i:0,"0"), V(f:1.0, "1.0"), VAR_NORMAL, 0,
    "set the value of the inference multiplier"},
{ &JHEURISTIC_K_UNKNOWN, "", "jheuristic-k-unk", P_FLOAT, V(i:0,"0"), V(f:0.0, "0.0"), VAR_NORMAL, 0,
    "set the value of unknown states (used with --precompute-states 0)"},


/* 
 * Output options
 */
{ NULL, "", "", P_NONE, {"0"}, {"0"}, VAR_NORMAL, 0, 
     "\nOutput options:"},
{ &output_format, "b", "", P_PRE_CHAR, V(c:'b',"b"), V(c:'n',"n"), VAR_NORMAL, 0,
     "Convert input to BDD format"},
{ &output_format, "c", "", P_PRE_CHAR, V(c:'c',"c"), V(c:'n',"n"), VAR_NORMAL, 0,
     "Convert input to CNF format"},
   

/* 
 * the end
 */
{ NULL, "", "", P_NONE, {"0"}, {"0"}, 0, 0, ""}
};

void
init_params()
{


}

void
finish_params()
{

   /* special files */
   if (!strcmp(debug_dev, "stdout")) stddbg = stdout;
   else
   if (!strcmp(debug_dev, "stderr")) stddbg = stderr;
   else {
   }

   /* temp-dir needs to have the ~ expanded to $HOME */
   /* and $TEMP expanded to  $TEMP or $TMPDIR or /tmp whichever comes first */
   t_opt *p_opt;
   char a_opt[][255] = { "temp-dir", /*"-any-other-par-",*/ "" };


   for (int i=0; a_opt[i][0]!=0; i++)
   { 
      p_opt = lookup_keyword(a_opt[i]);
      if (!p_opt) continue;
      if (((char*)(p_opt->p_target))[0] == '~') {
         char *env = getenv("HOME");
         if (env) 
         {
            char temp_str[256];
            sprintf(temp_str, "%s%s", 
                  env, ((char*)(p_opt->p_target))+1);
            sbsat_strncpy((char*)(p_opt->p_target), temp_str,
                  p_opt->p_value.i-1);
            ((char*)(p_opt->p_target))[p_opt->p_value.i]=0;
         }
      }
      if (!strncmp((char*)(p_opt->p_target), "$TEMP", 5)) {
         char *env = getenv("TEMP");
         char tmp_dir[] = "/tmp";
         if (!env) env = getenv("TMPDIR");
         if (!env) {
            struct stat buf;
            if (stat(tmp_dir, &buf) == 0) env = tmp_dir;
         }
         if (env)
         {
            char temp_str[256];
            sprintf(temp_str, "%s%s", 
                  env, ((char*)(p_opt->p_target))+5);
            sbsat_strncpy((char*)(p_opt->p_target), temp_str,
                  p_opt->p_value.i-1);
            ((char*)(p_opt->p_target))[p_opt->p_value.i]=0;
         } else {
            d1_printf1 ("TEMP or TMPDIR variable not found.\n");
         }
      }
   }

   //sbsat specific option computations

   if(functionTypeLimits_all > 0) {
     //set all function type limits to this value
     for(uintmax_t i = 0; i < FN_TYPE_MAX; i++) {
       functionTypeLimits[i] = functionTypeLimits_all;
     }
   }

}

int
params_parse_cmd_line(int argc, char *argv[])
{
   init_params();
   init_options();
   read_cmd(argc, argv);
   fix_ini_filename();
   read_ini(ini_filename);
   finish_params();

   if (PARAMS_DUMP) dump_params();
   return 0;
}

void
fn_parse_filename(char *filename)
{
   // filename in the format
   // _ are separators (replaced with 0)
   // name _ (un)sat _ arguments . ext
   //
   int argc=4;
   char *filename_copy = strdup(filename);
   char *ptr=filename_copy;
   if (ptr == NULL) { perror("strdup"); exit(1); }
   while(*ptr != 0) {
      if (*ptr == '_') argc++;
      ptr++;
   }
   char **argv = (char**)sbsat_calloc(argc+1, sizeof(char*), 9, "argv");
   argc=0;
   argv[argc++] = NULL; // skipped
   argv[argc++] = filename;
   // strip the extension
   if ((ptr = strrchr(filename_copy, '.'))!=NULL) *ptr = 0;
   ptr = filename_copy;
   while(*ptr != 0 && *ptr!='_') ptr++;
   if (*ptr != 0) {
// set the expected result
//      ptr++;
//      char expected_result[] = "--expected-result";
//      argv[argc++] = expected_result;
//      argv[argc++] = ptr;
      while(1) {
         while(*ptr != 0 && *ptr!='_') ptr++;
         if (*ptr == 0) break;
         *ptr = 0;
         ptr++;
         if (*ptr == 0) break;
         argv[argc++] = ptr;
         ptr++;
      }
   }
   read_cmd(argc, argv);
   free(filename_copy);
   free(argv);
}
