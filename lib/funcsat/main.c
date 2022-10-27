/*
 * Copyright 2012 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000, there is a non-exclusive license for use of this work by
 * or on behalf of the U.S. Government. Export of this program may require a
 * license from the United States Government.
 */

#include "funcsat/config.h"

#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#ifndef __USE_POSIX199309
#  define __USE_POSIX199309
#endif
#ifndef __USE_POSIX
#  define __USE_POSIX
#endif
#include <signal.h>
#include <zlib.h>
#include <time.h>
#include <libgen.h>
#include <limits.h>
#include <math.h>

#include "funcsat.h"
#include "funcsat_internal.h"

enum {
  UNSEEN = -1,
  SEEN = 2,
  SEEN_PROCESSED = 3,
  YES = 1,
  NO = 0,
};

static int PhaseSaving = 0;
static int SelfSubsumingResolution = 0;
static int DisplayStats = 1;
static int GraphStats = 0;
static int MinimizeLearnedClauses = 1;
static int NumUipsToLearn = -1;
static int HasGc = 0;
static int PrintSolution = YES;
static uintmax_t AlarmInterval = 1, /* seconds */
  AlarmNewlineInterval = 10;        /* AlarmIntervals */
static int HasCountSol = UNSEEN;
static int HasAssume = UNSEEN;
static int HasQuiet = UNSEEN;
static int HasBumpAdds = 0;


/* longOptions and optDescription are paired.  The string optDescription[i] in
 * optDescription is the description of the option at longOptions[i].  So each
 * time a new command-line option is added, a new description must be added as
 * well. */
static struct option longOptions[] = {
  { .name="help", .has_arg=0, .flag=0, .val='h'},
  {"restart", required_argument, 0, 'r'},

  /* the following options (not including debug) are handled specially in 'case
   * 0' of the getopt_long switch */
  {"phase-saving", optional_argument, &PhaseSaving, 0},
  {"ssr", optional_argument, &SelfSubsumingResolution, 0},
  {"live-stats", optional_argument, &DisplayStats, 0},
  {"graph-stats", optional_argument, &GraphStats, 0},
  {"minimize", optional_argument, &MinimizeLearnedClauses, 0},
  {"num-uips", optional_argument, &NumUipsToLearn, 0},
  {"nosolprint", no_argument, &PrintSolution, NO},
  {"gc", required_argument, &HasGc, 1},
  {"max-jail-dl", required_argument, 0, 'j'},
  {"count-solutions", required_argument, &HasCountSol, SEEN},
  {"assume", required_argument, &HasAssume, SEEN},
  {"quiet", no_argument, &HasQuiet, SEEN_PROCESSED},
  {"bump-adds", optional_argument, &HasBumpAdds, 0},

  /* add new options above this one */
  {"debug", required_argument, 0, 'd'},
  {0, 0, 0, 0}
};
static char *optDescription[] = {
  "Get help.",
  "Specify restart policy.  One of: luby (default), inout, minisat, none.",
  "Controls phase saving (off by default).",
  "Controls dynamic, self-subsuming resolution during learning (on by default).",
  "Controls live statistics printing (on by default).",
  "Controls whether output is formatted for easy graphing (off by default).",
  "Controls learned clauses minimisation (on by default).",
  "Learn only ARG uips for each conflict (0 for all, 0 by default).",
  "Do not print the solution, just the result (SAT or UNSAT).",
  "Learned clause GC strategy: none, minisat, glucose (default).",
  "Maximum decision level allowed for true literals to jail clauses.",
  "Count solutions on variable subset (ARG=1,2-5,...).",
  "Assume the given literals (ARG=1,-2,17,...). Multiple occurrences append. Not compatible with --count-solutions.",
  "Produce minimal output.",
  "If true, bumps the priority of each variable occurrence as a clause is added.",

  /* debug */
#ifdef FUNCSAT_LOG
  "Turn on debugging (ARG=label,number).",
#else
  "(Debugging was not configured.)",
#endif
  NULL
};

static char *dups(const char *str)
{
  char *res;
  FS_CALLOC(res, strlen(str)+1, sizeof(*str));
  strcpy(res, str);
  return res;
}

static char *strpuniq(const char *str, char *strs[])
{
  struct vec_ptr v;
  vec_ptr_mk(&v, 2);
  for (uintptr_t i = 0; strs[i]; i++) vec_ptr_push(&v, strs[i]);
  for (uintptr_t i = 0; str[i]; i++) {
    for (uintptr_t j = 0; j < v.size;) {
      char *strj = v.data[j];
      if (str[i] != strj[i]) vec_ptr_pop_at(&v, j);
      else j++;
    }
  }

  char *ret = NULL;
  if (v.size == 1) ret = v.data[0];
  vec_ptr_unmk(&v);
  return ret;
}

void usage(int argc, char **argv)
{
  struct option *opt = longOptions;
  char **str = optDescription;
  fprintf(stderr, "%s [options] file.cnf[.gz]\n", argv[0]);
  printf("This is " PACKAGE_STRING " by Denis Bueno <" PACKAGE_BUGREPORT ">\n");
  fprintf(stderr, "A wicked-pluggable SAT solver.\n\n");
  fprintf(stderr,
          "Copyright 2012 Sandia Corporation. Under the terms of Contract\n"
          "DE-AC04-94AL85000, there is a non-exclusive license for use of this work by\n"
          "or on behalf of the U.S. Government. Export of this program may require a\n"
          "license from the United States Government.\n\n");
  fprintf(stderr, "Help:\n");
  fprintf(stderr, "Boolean arguments can be numbers (1 or 0) "
          "or words (yes or no, on or off).\n");
  fprintf(stderr, "Options:\n");
  while (!(opt->name == 0    &&
           opt->has_arg == 0 &&
           opt->flag == 0    &&
           opt->val == 0)) {
    if (opt->has_arg == required_argument) {
      fprintf(stderr, "  --%s=ARG: %s\n", opt->name, *str);
    } else if (opt->has_arg == optional_argument) {
      fprintf(stderr, "  --%s[=ARG]: %s\n", opt->name, *str);
    } else {
      fprintf(stderr, "  --%s: %s\n", opt->name, *str);
    }
    str++, opt++;
  }
}


static funcsat *globalSolver = NULL;




static void installHandlers();
static void removeHandlers();
static void deltaStatsHandler(int);
static void printAssignment(funcsat *f);

extern char *optarg;
int main(int argc, char **argv)
{
  funcsat_config *conf = funcsatConfigInit(NULL);
  int c;
  clause assumptions;
  clauseInit(&assumptions, 0);
#ifdef FUNCSAT_LOG
  conf->debugStream = stderr;
#endif
  /* uint32_t i; */
  /* int digit_optind = 0; */

  while (1) {
    /* int this_option_optind = optind ? optind : 1; */
    int optidx = 0;

    c = getopt_long(argc, argv, "hd:", longOptions, &optidx);
    if (c == -1)
      break;

    switch (c) {
    case 0:
      if (NumUipsToLearn == 0) {
        NumUipsToLearn = -1;
        uint32_t n = UINT32_MAX;
        if (optarg) {
          n = strtol((char *) optarg, NULL, 10);
          n = n == 0 ? UINT32_MAX : n;
        }
        conf->numUipsToLearn = n;
        break;
      }
      if (HasCountSol == SEEN) {
        if (HasAssume == SEEN || HasAssume == SEEN_PROCESSED) {
          fprintf(stderr, "c warning: --assume not compatible with --count-solutions\n");
          fprintf(stderr, "c assumptions cleared\n");
        }
        clauseClear(&assumptions);
        char *start = optarg, *end, *dash, *null = start;
        while (*null) null++;
        bool done = false;
        do {
          dash = strchr(start, '-');
          end  = strchr(start, ',');
          if (!end) done = true, end = null;
          assert(end);
          literal p = strtol(start, NULL, 10);
          /* fprintf(stderr, "p = %ji", p); */
          assert(p > 0);
          if (dash && dash < end) {
            dash++;
            literal q = strtol(dash, NULL, 10);
            /* fprintf(stderr, ", q = %ji\n", q); */
            assert(q > 0);
            for (; p <= q; p++) {
              clausePush(&assumptions, p);
            }
          } else {
            clausePush(&assumptions, p);
          }
          start = end+1;
        } while (!done);
        HasCountSol = SEEN_PROCESSED;
        break;
      }
      if (HasAssume == SEEN) {
        char *start = optarg, *end, *null = start;
        while (*null++);
        bool done = false;
        do {
          end = strchr(start, ',');
          if (!end) done = true, end = null;
          assert(end);
          literal p = strtol(start, NULL, 10);
          clausePush(&assumptions, p);
          start = end+1;
        } while (!done);
        HasAssume = SEEN_PROCESSED;
        break;
      }
      if (HasGc) {
        HasGc = false;
        char *uniq;
        char *stuff[4] = { "none", "minisat", "glucose", 0 };
        if ((uniq = strpuniq(optarg, stuff))) {
          if (0 == strcasecmp("none", uniq)) {
            conf->gc = false;
          } else if (0 == strcasecmp("minisat", uniq)) {
            funcsatSetupActivityGc(conf);
          } else if (0 == strcasecmp("glucose", uniq)) {
            funcsatSetupLbdGc(conf);
          } else assert(0 && "probably bug in strpuniq");
        } else {
          fprintf(stderr, "error: bad --gc option: %s\n", optarg);
          usage(argc, argv);
          exit(1);
        }
        break;
      }
      if (longOptions[optidx].has_arg == required_argument ||
          longOptions[optidx].has_arg == optional_argument) {
        /* this assumes arguments reaching here are all boolean */
        bool valAsBool = true;
        if (optarg) {
          char *uniq;
          char *options[] = { "yes", "no", "on", "off", "0", "1", 0 };
          if ((uniq = strpuniq(optarg, options))) {
            if (0 == strcasecmp("no", uniq) ||
                0 == strcasecmp("off", uniq) ||
                0 == strcasecmp("0", uniq)) {
              valAsBool = false;
            } else if (0 == strcasecmp("yes", uniq) ||
                       0 == strcasecmp("on", uniq) ||
                       0 == strcasecmp("1", uniq)) {
              valAsBool = true;
            }
          } else assert(0 && "probably bug in strpuniq");
        } else {
          fprintf(stderr, "error: bad boolean option: %s\n", optarg);
          usage(argc, argv);
          exit(1);
        }
        *(longOptions[optidx].flag) = valAsBool;
      }
      break;

    case 'h':
      usage(argc, argv);
      exit(0);

    case 'r': {
      char *uniq;
      char *options[5] = { "none", "luby", "inout", "minisat", 0 };
      if ((uniq = strpuniq(optarg, options))) {
        if (0 == strcasecmp("none", uniq)) {
          conf->isTimeToRestart = funcsatNoRestart;
        } else if (0 == strcasecmp("luby", uniq)) {
          conf->isTimeToRestart = funcsatLubyRestart;
        } else if (0 == strcasecmp("inout", uniq)) {
          conf->isTimeToRestart = funcsatInnerOuter;
        } else if (0 == strcasecmp("minisat", uniq)) {
          conf->isTimeToRestart = funcsatMinisatRestart;
        } else assert(0 && "probably bug in strpuniq");
      } else {
          fprintf(stderr, "Unrecognised restart strategy: '%s'\n", optarg);
          usage(argc, argv);
          exit(1);
        }
      break;
    }

    case 'j':
      conf->maxJailDecisionLevel = (uintmax_t) strtol((char *) optarg, NULL, 10);
      fprintf(stderr, "c maxJailDecisionLevel = %ju\n", conf->maxJailDecisionLevel);
      break;

#ifdef FUNCSAT_LOG
    case 'd': {
      char *num = strchr(optarg, ',');
      if (!num) {
        fprintf(stderr, "ERROR: debug options have the form "
                "'label,number': comma not found\n");
        usage(argc, argv);
        exit(1);
      }
      *(num++) = '\0';          /* need to null-terminate to compare labels */
      int level = atoi(num);
      if (0 == strcmp("none", optarg)) {
        fprintf(stderr, "Disabling debugging\n");
        hashtable_clear(conf->logSyms, true, true);
      } else {
        int *levelp;
        FS_MALLOC(levelp, 1, sizeof(*levelp));
        *levelp = level;
        hashtable_insert(conf->logSyms, dups(optarg), levelp);
        fprintf(stderr, "c log: enabling label '%s', max level %i\n", optarg, level);
      }
    }
      break;
#endif

    default:
      usage(argc, argv);
      exit(1);
    }
  }
  if (optind >= argc) {
    fprintf(stderr, "error: no cnf file given.\n");
    usage(argc, argv);
    exit(1);
  }

  if (HasQuiet != UNSEEN) DisplayStats = 0;

  if (!GraphStats && !(HasQuiet != UNSEEN)) {
    printf("c This is " PACKAGE_STRING " by Denis Bueno <" PACKAGE_BUGREPORT ">\n");
  }

  conf->usePhaseSaving = int2bool(PhaseSaving);
  conf->useSelfSubsumingResolution = int2bool(SelfSubsumingResolution);
  conf->minimizeLearnedClauses = int2bool(MinimizeLearnedClauses);
  conf->bumpOriginal = HasBumpAdds ? bumpOriginal : conf->bumpOriginal;
  char filename[PATH_MAX];
  strcpy(filename, argv[optind]);
  conf->name = basename(filename);
  funcsat_result result = FS_UNKNOWN;
  funcsat *func = funcsatInit(conf);
  globalSolver = func;
  installHandlers();
  if (!(HasQuiet != UNSEEN)) fprintf(stderr, "c reading '%s' ...\n", argv[optind]);
  result = parseDimacsCnf(argv[optind], func);
  /* funcsatDebug(func, "solve", 1); */
  if (!(HasQuiet != UNSEEN)) {
    fprintf(
      stderr,
      "c %ju variables, %ju clauses\n",
      funcsatNumVars(func), funcsatNumClauses(func));
  }
  if (result == FS_UNSAT) {
    fprintf(stderr, "c instance trivially UNSAT during parsing\n");
    goto TearDown;
  }
  if (HasCountSol == SEEN_PROCESSED) {
    clause lastSolution;
    clauseInit(&lastSolution, func->numVars);
    fprintf(stderr, "c counting distinct solutions differing on: ");
    dimacsPrintClause(stderr, &assumptions);
    fprintf(stderr, "\n");
    intmax_t count = funcsatSolCount(func, assumptions, &lastSolution);
    assert(count != -1 && "input error to funcsatSolCount");
    printf("s %ji\n", count);
    if (count >= 1 && PrintSolution) {
      fprintf(stderr, "c last solution:\n");
      /* assume the last assignment and "solve" so we can re-use
       * printAssignment */
      clauseClear(&assumptions);
      assert(lastSolution.size == func->numVars);
      clauseCopy(&assumptions, &lastSolution);
      
      funcsatPushAssumptions(func, &assumptions);

      funcsatSolve(func);
      
      printAssignment(func);
      
      funcsatPopAssumptions(func, assumptions.size);
    }
    clauseDestroy(&lastSolution);
    goto TearDown;
  }
  if (!(HasQuiet != UNSEEN)) {
    fprintf(stderr, "c each row is stats from roughly %ju seconds\n",
            (AlarmInterval*AlarmNewlineInterval)-1);
    fprintf(stderr, "c solving ");
  }
  if (HasAssume == SEEN_PROCESSED) {
    fprintf(stderr, "c assuming: ");
    dimacsPrintClause(stderr, &assumptions);
  }
  if (!(HasQuiet != UNSEEN)) fprintf(stderr, "\n");
  if (DisplayStats) deltaStatsHandler(0);

  if(funcsatPushAssumptions(func, &assumptions) == FS_UNSAT) {
    result = FS_UNSAT;
    assumptions.size = 0;
  } else result = funcsatSolve(func);
  globalSolver = NULL;
  removeHandlers();
  if (!(HasQuiet != UNSEEN)) fprintf(stderr, "\n");
TearDown:
  if (!GraphStats) printf("s %s\n", funcsatResultAsString(result));
  if (result == FS_SAT && PrintSolution) {
    printAssignment(func);
  }
  if (GraphStats) funcsatPrintColumnStats(stdout, func);
  else if (!(HasQuiet != UNSEEN)) {
    funcsatPrintStats(stdout, func);
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    double uTime = ((double)usage.ru_utime.tv_sec) +
      ((double)usage.ru_utime.tv_usec)/1000000;
    double sTime = ((double)usage.ru_stime.tv_sec) +
      ((double)usage.ru_stime.tv_usec)/1000000;
    fprintf(stderr, "c %.2lfs (user) / %.2lfs (sys)\n", uTime, sTime);
  }

  clauseDestroy(&assumptions);
  funcsatDestroy(func);
  if (!(HasQuiet != UNSEEN)) fprintf(stderr, "c done\n");
#ifdef FUNCSAT_LOG
  conf->debugStream = stderr;
#endif

  funcsatConfigDestroy(conf);

  return result;
}

static void printAssignment(funcsat *func)
{
  const int maxWidth = 80;
  variable v;
  int cnt = 0;
  cnt += printf("v ");
  for (v = 1; v <= funcsatNumVars(func); v++) {
    mbool assign = funcsatValue(func, fs_var2lit(v));
    assert(assign != unknown);
    literal p = assign == true ? fs_var2lit(v) : -fs_var2lit(v);
    int sign = p < 0 ? 1 : 0;
    int numDigits = v == 1 ? 0 : ceil(log10(v));
    if (cnt >= maxWidth - sign - numDigits) printf("\nv "), cnt = 0;
    cnt += printf("%ji ", p);
  }
  printf("0\n");

}


/* When the user hits ctrl-cp, print out the solver stats */
static void termHandler(int);
struct sigaction defaultCcAction, alrmAction, termAction;
static void installHandlers()
{
  memset(&termAction, 0, sizeof(termAction));
  memset(&alrmAction, 0, sizeof(alrmAction));
  alrmAction.sa_handler = deltaStatsHandler;
  termAction.sa_handler = termHandler;
  memset(&defaultCcAction, 0, sizeof(defaultCcAction));

  sigaction(SIGINT, &termAction, &defaultCcAction);
  sigaction(SIGTERM, &termAction, NULL);
  sigaction(SIGALRM, &alrmAction, NULL);
}

static void termHandler(int sig)
{
  /* install old handler */
  /* sigaction(SIGINT, &defaultCcAction, NULL); */
  if (!GraphStats) {
    printf("\n");
    printf("s UNKNOWN\n");
    printf("c forcibly terminated\n");
  }
  if (GraphStats) funcsatPrintColumnStats(stdout, globalSolver);
  else {
    funcsatPrintStats(stdout, globalSolver);
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    double uTime = ((double)usage.ru_utime.tv_sec) +
      ((double)usage.ru_utime.tv_usec)/1000000;
    double sTime = ((double)usage.ru_stime.tv_sec) +
      ((double)usage.ru_stime.tv_usec)/1000000;
    fprintf(stdout, "c %.2lfs (user) / %.2lfs (sys)\n", uTime, sTime);
  }
  exit(1);
}

static void removeHandlers()
{
  sigaction(SIGINT, &defaultCcAction, NULL);
  sigaction(SIGALRM, NULL, NULL);
}

static void deltaStatsHandler(int sig)
{
  static int n = 0;
  static intmax_t cnt = 0;
  funcsat *f = globalSolver;
  int i;
  if (!f) return;
  if ((cnt % 100) == 0) {
    fprintf(stderr,
            "\nc      dec/s     props/s    confl/s    learned"
            "   rest facts   pct (row=%jus)\n",
            (AlarmInterval*AlarmNewlineInterval)-1);
    n = 0;
  } else if (((uintmax_t) cnt % AlarmNewlineInterval) == 0) {
    fprintf(stderr, "\n");
    n = 0;
  }
  static uint64_t numInf = 0;
  while (numInf < f->trail.size && levelOf(f, fs_lit2var(f->trail.data[numInf])) == 0) {
    numInf++;
  }

  static uint64_t lastNumProps = 0;
  static uint64_t lastNumConflicts = 0;
  static uint64_t lastNumDecisions = 0;
  for (i = 0; i < n; i++) {
    fprintf(stderr, "\b");
  }
  n = fprintf(
    stderr,
    "c %10" PRIu64 " %11" PRIu64 " %10" PRIu64 " %10" PRIu64 " %6" PRIu64 " %5" PRIu64 "",
    (f->numDecisions - lastNumDecisions)/AlarmInterval,
    ((f->numProps + f->numUnitFactProps) - lastNumProps)/AlarmInterval,
    (f->numConflicts - lastNumConflicts)/AlarmInterval,
    f->numLearnedClauses,
    f->numRestarts,
    numInf);
  lastNumProps = f->numProps + f->numUnitFactProps;
  lastNumConflicts = f->numConflicts;
  lastNumDecisions = f->numDecisions;

  cnt++;
  if (f) alarm(AlarmInterval);
}

