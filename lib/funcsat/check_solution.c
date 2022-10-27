#include <funcsat/config.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <funcsat.h>
#include "funcsat_internal.h"
#include "funcsat/vec_uintptr.h"


int main(int argc, char **argv)
{
  /* argv[1] - solution */
  /* argv[2] - cnf */
  if (argc != 3) {
    printf("usage: check_solution [solution-file] [cnf]\n");
    exit(1);
  }
  assert(argc == 3);
  funcsat_config *conf = funcsatConfigInit(NULL);
  funcsat *f = funcsatInit(conf);
  printf("Parsing CNF from '%s' ...\n", argv[2]);
  parseDimacsCnf(argv[2], f);
  FILE *solfile;
  printf("Parsing solution from '%s' ...\n", argv[1]);
  if ((solfile = fopen(argv[1], "r")) == NULL) {perror("fopen"), exit(1);}
  funcsat_result result = fs_parse_dimacs_solution(f, solfile);
  fclose(solfile);

  int ret = 0;
  uintptr_t fail;
  if (result == FS_SAT) {
    for_vec_uintptr (ix, f->orig_clauses) {
      struct clause_head *h = clause_head_ptr(f, *ix);
      struct clause_iter it;
      bool sat = false;
      clause_iter_init(f, *ix, &it);
      for (literal *p = clause_iter_next(f, &it); p; p = clause_iter_next(f, &it)) {
        if (funcsatValue(f, *p) == true) {
          sat = true;
          break;
        }
      }
      if (!sat) {
        fail = *ix;
        ret = 1;
        break;
      }
    }
  }
  if (ret == 0) {
    fprintf(stderr, "passed!\n");
  } else {
    fprintf(stderr, "FAIL\n");
    clause_head_print_dimacs(f, stderr, fail);
    fprintf(stderr, "\n");
  }
  return ret;
}
