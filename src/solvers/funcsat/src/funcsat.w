% -*- mode: cweb -*-

% Copyright 2012 Sandia Corporation. Under the terms of Contract
% DE-AC04-94AL85000, there is a non-exclusive license for use of this work by or
% on behalf of the U.S. Government. Export of this program may require a license
% from the United States Government.

\def\todo{TODO COMMAND}
\let\ifpdf=\relax
\def\NULL{{\tt NULL}} %I think the default greek thing is confusing
\input eplain
\beginpackages
  \usepackage{url}
  \usepackage[dvipsnames]{color}
\endpackages
\enablehyperlinks
\hlopts{bwidth=0}
\hlopts[url]{colormodel=named,color=BlueViolet}

\def\numberedmarker{{\fam1\teni \number\itemnumber}}
\def\numberedprintmarker#1{\llap{#1 \listmarkerspace}}
\def\unorderedmarker{---}
\listleftindent=\parindent
\listleftindent=2\parindent
\abovelistskipamount=0pt
\belowlistskipamount=0pt
\interitemskipamount=0pt

\def\osf#1{{\fam1 \teni #1}}
\let\footnote=\numberedfootnote



\input font_palatino

\baselineskip=12pt %10/12
\let\cmntfont=\sl
\let\mainfont=\rm
\mainfont
\def\paragraphit#1{{\it #1\/}\hskip\parindent}


\def\funcsat{{\caps funcsat}}
\def\minisat{{\caps minisat}}
\def\picosat{{\caps picosat}}

\def\acro#1{{\twelvecaps #1}}
\def\textit#1{{\it #1\/}}

\def\SAT{\acro{sat}}
\def\UNSAT{\acro{unsat}}
\def\UIP{\acro{uip}}
\def\LBD{\acro{lbd}}
\def\BCP{\acro{bcp}}

\let\or\vee
\let\and\hat



% Important sections for this WEB:

% External types -- goes into <file>.h
% External declarations -- goes into <file>.h after External ty...
% Internal types -- goes into <file>_internal.h
% Internal declarations -- goes into <file>_internal.h after Internal ty...
% Global definitions -- goes into <file>.c after Internal decl...


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                  IT BEGINS
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%




@* Introduction.  \funcsat\ is a CDCL SAT solver, written by Denis Bueno
\url{denis.bueno@@sandia.gov} at Sandia National Labs. It was developed under
LDRD funding. It is written in ANSI \CEE/99.

Copyright 2012 Sandia Corporation. Under the terms of Contract
DE-AC04-94AL85000, there is a non-exclusive license for use of this work by or
on behalf of the U.S. Government. Export of this program may require a license
from the United States Government.


\funcsat's main goal is {\it flexibility}. It has a bunch of features, blah blah.

@s uintmax_t int
@s intmax_t int
@s clause int
@s funcsat int
@s literal int
@s variable int
@s uint64_t int
@s uint32_t int
@s uint16_t int
@s uint8_t char
@s int64_t int
@s int32_t int
@s int16_t int
@s int8_t char
@s fs_ifdbg if
@s forVector for
@s forVectorRev for
@s mbool bool
@s funcsat_config int
@s for_watchlist for
@s for_watchlist_continue for
@s for_clause for
@s for_head_tail for
@s funcsat_result int
@s hashtable int
@s fibheap int
@s binvec_t int
@s watchlist int
@s watchlist_elt int
@s all_watches int
@s head_tail int
@s new numVars
@s uintptr_t int
@s bh_node int
@s vec_ptr int
@s for_vec_ptr for
@s for_vec_ptr99 for
@s vec_intmax int
@s for_vec_intmax for
@s for_vec_intmax99 for
@s vec_uintmax int
@s for_vec_uintmax for
@s for_vec_uintmax99 for
@s vec_uintptr int
@s for_vec_uintptr for
@s for_vec_uintptr99 for



@* The Client. Information in the {\tt funcsat.h} header file should give you all you
need to begin using \funcsat. We provide methods for the following:

\numberedlist

\li Creating (|funcsatInit|) and destroying (|funcsatDestroy|) a SAT solver

\li Adding instance clauses (|funcsatAddClause|)

\li Solving (|funcsatSolve|) SAT instances.

\endnumberedlist

\funcsat\ is an extensible SAT solver. This means SAT instances can be solved
under unit assumptions.

\numberedlist

\li In order to add a unit assumption, use |funcsatPushAssumption|.

\li {\bf You must check the return value}. If it returns |FS_UNSAT|, the
  assumption is not actually pushed because your problem is trivially
  unsatisfiable.

\li At this point, if you call |funcsatSolve|, the SAT instance is restricted to
  solutions where every push assumption is true.

\li In order to relieve assumptions, call |funcsatPopAssumptions|. Assumptions
are kept on a LIFO stack and {\it push} and {\it pop} have LIFO semantics.

\endnumberedlist

@* The Developer.

The second category of user is going to want to modify funcsat. What follows is
an overview of all data structures in \funcsat.


@ Vocab and data types.

\numberedlist

\li The order of inferences (implied literals) is kept in the |funcsat->trail|,
a chronological list of the current (partial) assignment.

\li The |funcsat->model| tells whether a variable is assigned given the
variable's index in the trail. A variables assignment (either True or False) is
called its {\it phase}.

\li The |funcsat->level| of each variable is the decision level at which the
variable was set: if the variable was set at decision level 0, that means it's
True for all time.

\li The |funcsat->decisions| tells whether a particular variable is a
{\it decision variable} (a choice point, a branch) or not. A variable that is
not a decision is an {\it inference}, or currently unassigned. Each inference
has a {\it reason} (a clause sometimes called an antecedent) stored in
|funcsat->reason|.

\li Each clause can either be original or learned; a learned clause is implied
by the SAT instance.

\endnumberedlist

@ Generic data types. There are some datatypes used everywhere in funcsat:

\unorderedlist

\li |variable|, |literal|: just typedefs for |uintmax_t| and |intmax_t|, resp.

\li |vec_ptr|: a growable, sized array of pointers. It can act as an efficient
\acro{lifo} stack. In fact, there is a whole family of vectors with essentially
the same semantics, differing only in the underlying element type.

\li |clause|: a clause; or just a growable, sized array of |intmax_t|

\li |mbool|: a three-value Boolean, or ``multi-Bool''. Possible values are
|true|, |false|, and |unknown|.

\li |clause_head| and |clause_block|. You can read about these in the Clauses
section (page~\refn{pg-clauses}).


\endunorderedlist

Other available data types:
\unorderedlist

\li |struct hashtable|: just a direct chained hash table
\li |fibheap|: fibonacci heap (prio queue impl)

\endunorderedlist

@ If you want to add some new solver state...

The core solver state is in this file; the data type is |funcsat|. For example,
when I added phase saving I needed a new vector to store the last phase. So,

\unorderedlist
\li I added a |funcsat->phase| into |funcsat|.
\li Next, I allocated the state in |funcsatInit|
\li and free'd the state in |funcsatDestroy|
\li Finally, I added the incremental resizing of the state
in |funcsatResize|.

\endunorderedlist

{\bf Do not allocate during solving.} Allocate only during |funcsatInit| and
|funcsatResize|. Allocating during solving is bad. Of course we have to allocate
when learning a new learned clause. But we often do not have to call |malloc(1)|
when this happens -- because there is a pool of clauses that get recycled.

@ If you want to add a new user-settable parameter...

All such parameters are part of the |funcsat_config|, which is stored
per-solver in |funcsat->conf|.

@ If you want to change the clause learning...

See the learning on page \refn{pg-clause-learn}. It's a really nasty loop that
calculates all the UIPs of the current conflict graph, or at least all the UIPs
the user wants. It does resolution to figure out which clause to learn. It does
conflict clause minimisation using Van Gelder's DFS-based minimisation
algorithm. It does only-the-fly self-subsuming resolution with each resolvent
produced during learning.

@ If you want to add a new stat...

Put it in |funcsat|. Then you'll probably want to change |funcsatPrintStats|,
too (and possibly even |funcsatPrintColumnStats|).

@ If you want to add a new argument...

Probably you should be exposing some option in |funcsat->conf|. In any case, you
need to look in {\tt main.c} to see how getopt is used to handle the existing
options. Then hack yours in.


Create a new \funcsat\ instance.  \funcsat\ is designed to be thread-safe so one
process may create as many solvers as it likes. Get your default config from
|funcsatConfigInit|.

@* Implementation. This file was originally not literate. I'm converting it to
literate style bit by bit. Be patient. First, the public header file is pretty
simple.

@d swap(ty, x, y) ty _swap_tmp ## ty = (x); (x) = (y), (y) = _swap_tmp ## ty;


@(funcsat.h@>=
#ifndef funcsat_h_included
#define funcsat_h_included
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>

#include "funcsat/vec_bool.h"
#include "funcsat/vec_uint64.h"

@<External types@>@;

@<External declarations@>@;

#endif

@ The internal header file is where our internal types and declarations go

@(funcsat_internal.h@>=
#ifndef funcsat_internal_h_included
#define funcsat_internal_h_included
#include "funcsat/vec_ptr.h"
#include "funcsat/vec_intmax.h"
#include <funcsat/fibheap.h>

  @<Conditional macros@>@;
  @<Internal types@>@;
  @<Main \funcsat\ type@>@;
  @<Internal declarations@>@;

#endif

@  All the external types need the basic \funcsat\ types.

@<External ty...@>=
#include "funcsat/system.h"
#include "funcsat/vec_uintmax.h"
#include "funcsat/hashtable.h"


@ Next comes \funcsat! I've put all the includes here, for simplicity. Note that
this file has no {\tt main} function---we only define \funcsat's internals and
API here. See {\tt main.c} for the grungy details of exposing all of \funcsat's
options on the command-line.

It's pretty gross.

@c
#include "funcsat/config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <errno.h>
#include <funcsat/hashtable.h>
#include <funcsat/system.h>
#include <zlib.h>
#include <ctype.h>


#include "funcsat.h"
#include "funcsat_internal.h"
#include "funcsat/vec_uintptr.h"

@<Global definitions@>@;



@ I use the following macros for memory management, just because.

@<External decl...@>=

#define FS_CALLOC(ptr, n, size)                   \
  do {                                         \
    ptr = calloc((n), (size));                 \
    if (!ptr) perror("FS_CALLOC"), abort();       \
  } while (0);

#define FS_MALLOC(ptr, n, size)                   \
  do {                                         \
    ptr = malloc((n)*(size));                  \
    if (!ptr) perror("FS_MALLOC"), abort();       \
  } while (0);

#define FS_MALLOC_TY(ty, ptr, n, size)             \
  do {                                          \
    ptr = (ty) malloc((n)*(size));              \
    if (!ptr) perror("FS_MALLOC_TY"), abort();     \
  } while (0);

#define FS_REALLOC(ptr, n, size)                                           \
  do {                                                                  \
    void *tmp_funcsat_ptr__;                                            \
    tmp_funcsat_ptr__ = realloc(ptr, (n)*(size));                       \
    if (!tmp_funcsat_ptr__)  free(ptr), perror("FS_REALLOC"), abort();     \
    ptr = tmp_funcsat_ptr__;                                            \
  } while (0);

#define FS_REFS_CALLOC(p,o,n,sz)                                      \
  do {                                                          \
    assert((n) >= (o));                                         \
    void *tmp_funcsat_ptr__;                                    \
    tmp_funcsat_ptr__ = realloc(p, (n)*(sz));                   \
    if (!tmp_funcsat_ptr__) perror("FS_REFS_CALLOC"), abort();        \
    memset(tmp_funcsat_ptr__ + ((o)*(sz)), 0, ((n)-(o))*(sz));  \
    (p) = tmp_funcsat_ptr__;                                    \
  } while (0);


@*1 Top-level solver. This is the top-level solving loop. You should read
this. It's really short! it should stay that way. It should fit on a page. It
serves as a pretty easy-to-grasp example that can be referred to for writing a
custom search strategy using \funcsat\ components.

The main parameters of the solver allow configuring a wide range of search
behavior. Note how the loop runs until the solver ``runs out of resources.''
This allows one to use \funcsat\ to do cheap probing for properties, allowing
the solver to return ``unknown'' if it needs to. As another example,
|isTimeToRestart| allows one to insert a custom restart strategy.

@<Global def...@>=
funcsat_result funcsatSolve(funcsat *f)
{
  if (FS_UNSAT == (f->lastResult = startSolving(f))) goto Done;

  if (!bcpAndJail(f)) goto Unsat;

  while (!f->conf->isResourceLimitHit(f, f->conf->user)) {
    fs_ifdbg(f, "solve", 3) fs_print_state(f, fs_dbgout(f));
    if (!bcp(f)) {
      if (0 == f->decisionLevel) goto Unsat;
      if (!analyze_conflict(f)) goto Unsat;
      if (f->conf->gc) f->conf->sweepClauses(f, f->conf->user);
      continue;
    }

    if (f->trail.size != f->numVars &&
        f->conf->isTimeToRestart(f, f->conf->user)) {
      fslog(f, "solve", 1, "restarting\n");
      ++f->numRestarts;
      backtrack(f, 0, NULL, true);
      continue;
    }

    if (!funcsatMakeDecision(f, f->conf->user)) {
      f->lastResult = FS_SAT;
      goto Done;
    }
  }

Unsat:
  f->lastResult = FS_UNSAT;

Done:
  fslog(f, "solve", 1, "instance is %s\n", funcsatResultAsString(f->lastResult));
  assert(f->lastResult != FS_SAT || f->trail.size == f->numVars);
  finishSolving(f);
  return f->lastResult;
}

@ Some notes about the external API.

Funcsat returns results using the following datatype. The codes are the typical
codes used for \SAT\ solvers in the \SAT\ competition.

@<External ty...@>=
typedef enum
{
  FS_UNKNOWN = 0,
  FS_SAT     = 10,
  FS_UNSAT   = 20
} funcsat_result;

@

@<External decl...@>=
void funcsatCheck(funcsat *f, funcsat_result r);
@

@<Global def...@>=
void funcsatCheck(funcsat *f, funcsat_result r)
{
  assert(r == f->lastResult);
  if (r == FS_SAT) {
    for_vec_uintptr (ix_cl, f->orig_clauses) {
      @<Check that |ix_cl| is satisfied@>@;
    }
    for_vec_uintptr (ix_cl, f->learned_clauses) {
      @<Check that |ix_cl| is satisfied@>@;
    }
  }
}

@
@<Check that |ix_cl| is satisfied@>=
struct clause_iter it;
bool sat = false;
clause_iter_init(f, *ix_cl, &it);
for (literal *p = clause_iter_next(f, &it); p; p = clause_iter_next(f, &it)) {
  assert(funcsatValue(f, *p) != unknown);
  if (funcsatValue(f, *p) == true) {
    sat = true;
    break;
  }
}
assert(sat);


@*2 Solver type. \funcsat\ has a bunch of members.

@d Unassigned (-1)

@<Main \funcsat\ type@>=
struct funcsat_config;
struct clause_head;
struct clause_block;

struct funcsat
{
  struct funcsat_config *conf;

  /*Added by SEAN!!!*/
  uint8_t  (*Backtrack_hook)(uintptr_t *_SM, uintmax_t level);
  uint8_t  (*BCP_hook)(uintptr_t *_SM, uintmax_t nVariable, uint8_t bPolarity);
  void     (*MakeDecision_hook)(uintptr_t *_SM);
  intmax_t (*ExternalHeuristic_hook)(uintptr_t *_SM);

  clause assumptions; /* assumptions as given by the user */

  funcsat_result lastResult; /* result of the last incremental call  */

  uintmax_t decisionLevel; /* current decision level */

  uintmax_t propq; /* Unit propagation queue, which is an index into
                      |funcsat->trail|. The element pointed at by |propq| is the
                      first literal to propagate.  {\it Invariant:} If |propq|
                      is |funcsat->trail.size|, then the queue is empty. */

  clause trail; /* Current (partial) assignment stack. */

  struct vec_uintmax model; /* If a variable is assigned (see |funcsat->level|), contains
                        the index of the literal in the trail. \todo\ bitpack */

  clause phase; /* Stores the current, or last, phase of each variable (for
                   phase saving). */

  clause level; /* Records the decision level of each variable.  If a variable
                   is unset, its |level| is |Unassigned|. */

  struct vec_uintmax decisions; /* For each decision variable, this maps to its
                            decision level. A non-decision variable maps to
                            0. */

  struct vec_uintptr *reason; /* Maps a variable to the (index for the) |struct reason|
                         that became unit to cause a variable assignment (see
                         |funcsat->reason_infos|). The reason is |NO_CLS| if
                         the variable is a decision variable. */

  struct vec_ptr reason_hooks; /* Indexed by types, returns a function pointer that
                        calculates a clause given a |funcsat| and a
                        |literal|. */

  struct vec_reason_info *reason_infos; /* It is not safe to take pointers of
                                         these values because this vector can be
                                         realloc'd. */
  uintptr_t reason_infos_freelist; /* index of first free |reason_info| */

  head_tail *unit_facts;  /* Indexed by variable, each each |head_tail| list
                             points to a linked list of clauses. */
  uintmax_t unit_facts_size;
  uintmax_t unit_facts_capacity;

  head_tail *jail; /* Indexed by variable, each mapped to a |head_tail|
                      list. Each list contains only clauses currently satisfied
                      by the associated variable. */

  all_watches watches; /* Indexed from literals (using |fs_lit2idx|) into a
                          |watcherlist|. */

  struct vec_uintptr *orig_clauses; /* A list of all the original clauses. */
  struct vec_uintptr *learned_clauses; /* A list of all the learned clauses. */
  uintmax_t numVars;

  struct clause_head_pool  clheads;
  struct clause_block_pool clblocks;

  uintptr_t conflict_clause; /*  When unit propagation discovers a conflicting
                                 clause, its index is stashed here. */


  struct litpos litpos_uip; /* used during conflict analysis */
  clause uipClause; /* Working area for the clause we will eventually learn
                     during conflict analysis. */
  struct vec_ptr subsumed; /* list of subsumed clauses. used by clause learning. */


  
  uintmax_t LBD_count; /* \LBD\ heuristic stuff -- see |LBD_compute_score| */
  struct vec_uint64 LBD_levels; /* \LBD\ heuristic stuff -- see |LBD_compute_score| */
  struct vec_uint64 LBD_histogram;
  uint64_t LBD_last_num_conflicts;
  uint64_t LBD_base;
  uint64_t LBD_increment;

  
  double   claDecay; /* clause activities */
  double   claInc;
  double   learnedSizeFactor;   /* when doing clause-activity based gc, this is
                                   the initial limit for learned clauses, a
                                   fraction of the original clauses (as in
                                   minisat 2.2.0) */

  double   maxLearned;
  double   learnedSizeAdjustConfl;
  uint64_t learnedSizeAdjustCnt;
  uint64_t learnedSizeAdjustInc;
  double   learnedSizeInc;


  
  struct vec_uintmax seen;             /* conflict clause minimisation stuff */
  struct vec_intmax analyseToClear;
  struct vec_uintmax analyseStack;     /* dfs stack */
  struct vec_uintmax allLevels;        /* set of the levels-of-vars that occur in the
                                   clause */


  
  double   varInc; /* dynamic variable order stuff */
  double   varDecay;

  struct bh_node *binvar_heap;  /* binary heap of variables */
  uintmax_t       binvar_heap_size; /* number of elements in |binvar_heap| */
  uintmax_t      *binvar_pos; /* locations of each var in the heap */

  
  int64_t  lrestart; /* luby restart stuff -- cribbed from \picosat */
  uint64_t lubycnt;
  uint64_t lubymaxdelta;
  bool   waslubymaxdelta;

  
  struct drand48_data *rand;



  uint64_t numSolves;   /* Number of calls to ::funcsatSolve */


  uint64_t numLearnedClauses;   /* How many learned clauses */


  uint64_t numSweeps;   /* How many times we deleted learned clauses */


  uint64_t numLearnedDeleted;   /* How many learned clauses we deleted */


  uint64_t numLiteralsDeleted;  /*
  How many literals deleted from learned clauses as we simplify them */


  uint64_t numProps;  /* How many unit propagations */
  uint64_t numUnitFactProps;

  uint64_t numJails;


  uint64_t numConflicts;  /* How many conflicts */

  uint64_t numResolutions;


  uint64_t numRestarts;  /* How many restarts */


  uint64_t numDecisions;  /* How many decisions */


  uint64_t numSubsumptions;  /*
  How many times a clause was simplified due to self-subsuming resolution */


  uint64_t numSubsumedOrigClauses;  /*
  How many subsumption simplifications were done for original clauses */

  uint64_t numSubsumptionUips;
};


@*2 Adding clauses. The main solver interface is just a few functions.


@<Global def...@>=
funcsat_result funcsatAddClause(funcsat *f, clause *c)
{
  variable maxVar = 0;
  uintptr_t ix_cl;
  struct clause_head *cl;

  funcsatReset(f);

  if (c->size > 1) {            /* a few trivial clause simplifications */
    uintmax_t size = c->size;
    literal *i, *j, *end;
    sortClause(c);
    /* i is current, j is target */
    for (i = j = (literal *)c->data, end = i + c->size; i != end; i++) {
      literal p = *i, q = *j;
      if (i != j) {
        if (p == q) {           /* duplicate literal */
          size--;
          continue;
        } else if (p == -q) {   /* trivial clause */
          clauseDestroy(c);
          goto Done;
        } else *(++j) = p;
      }
    }
    c->size = size;
  }

  for (variable k = 0; k < c->size; k++) {
    variable v = fs_lit2var(c->data[k]);
    maxVar = v > maxVar ? v : maxVar;
  }
  funcsatResize(f, maxVar);

  ix_cl = clause_head_alloc_from_clause(f,c);
  cl = clause_head_ptr(f, ix_cl);

  cl->is_learned = false;
  cl->is_reason = false;
  cl->lbd_score = LBD_SCORE_MAX;
  cl->activity = 0.f;
  f->conf->bumpOriginal(f, ix_cl);

  fs_ifdbg (f, "solve", 2) {
    fslog(f, "solve", 2, "adding %ju ", funcsatNumClauses(f)+1);
    fs_clause_print(f, fs_dbgout(f), c);
    fprintf(fs_dbgout(f), "\n");
  }
  funcsat_result clauseResult = addClause(f, ix_cl);
  if (f->lastResult != FS_UNSAT) {
    f->lastResult = clauseResult;
  }
  vec_uintptr_push(f->orig_clauses, ix_cl);
Done:
  return f->lastResult;
}

@ When adding a clause we need to allocate an internal |clause_head| for
it. This function creates a new clause with exactly the same literals (in the
same order) as |c|. It returns the new |clause_head|'s index.
@<Global def...@>=
inline uintptr_t clause_head_alloc_from_clause(funcsat *f, clause *c)
{
  uintmax_t sz_orig = c->size;
  uintptr_t ix_cl = clause_head_mk(f, c->size);
  struct clause_head *cl = clause_head_ptr(f, ix_cl);
  intmax_t sz_copied = 0; /* use this index to keep the order of |c| */
  for (int i = 0; i < CLAUSE_HEAD_SIZE && sz_copied < c->size; i++) {
    /* copy head literals */
    cl->lits[i] = c->data[sz_copied++];
    cl->sz++;
  }
  if (sz_copied < c->size) { /* create first block */
    uintptr_t ix_bl = clause_block_from_clause(f, c, cl, &sz_copied);
    cl->nx = ix_bl;
    while (sz_copied < c->size) { /* create additional blocks */
      struct clause_block *bl;
      uintptr_t ix_bl_nx = clause_block_from_clause(f, c, cl, &sz_copied);
      bl = clause_block_ptr(f, ix_bl);
      bl->nx = ix_bl = ix_bl_nx;
    }
  }
  assert(cl->sz == sz_orig);
  return ix_cl;
}

@ Copies literals (up to |CLAUSE_BLOCK_SIZE| of them) from |c| beginning at
|sz_copied| into a fresh block, whose index is returned.
@<Global def...@>=
static inline uintptr_t clause_block_from_clause(funcsat *f, clause *c,
                                                 struct clause_head *cl,
                                                 intmax_t *sz_copied)
{
  uintptr_t ix_bl = clause_block_mk(f);
  struct clause_block *bl = clause_block_ptr(f, ix_bl);
  for (int i = 0; i < CLAUSE_BLOCK_SIZE && *sz_copied < c->size; i++) {
    bl->lits[i] = c->data[(*sz_copied)++];
    cl->sz++;
  }
  return ix_bl;
}

@
@<Internal decl...@>=
static inline uintptr_t clause_block_from_clause(funcsat *f, clause *c,
                                                 struct clause_head *cl,
                                                 intmax_t *ix_clause);

@ Prototype.
@<External decl...@>=
funcsat_result funcsatAddClause(funcsat *func, clause *clause);
extern uintptr_t clause_head_alloc_from_clause(funcsat *f, clause *c);


@ Add the initialized internal clause to the watcher or unit fact lists. This
may cause the solver to be in conflict.

Precondition: the decision level is 0.
@<Global def...@>=
static inline funcsat_result addClause(funcsat *f, uintptr_t ix_cl)
{
  struct clause_head *h = clause_head_ptr(f, ix_cl);
  funcsat_result result = FS_UNKNOWN;
  assert(f->decisionLevel == 0);
  h->is_learned = false;
  if (h->sz == 0) {
    f->conflict_clause = ix_cl;
    result = FS_UNSAT;
  } else if (h->sz == 1) {
    mbool val = funcsatValue(f, h->lits[0]);
    if (val == false) {
      f->conflict_clause = ix_cl;
      result = FS_UNSAT;
    } else {
      if (val == unknown) {
	trailPush(f, h->lits[0], reason_info_mk(f, ix_cl));
        head_tail_add(f, &f->unit_facts[fs_lit2var(h->lits[0])], ix_cl);
      }
    }
  } else addWatch(f, ix_cl);
  return result;
}


@ 
@<Internal decl...@>=
static inline funcsat_result addClause(funcsat *f, uintptr_t ix_cl);
@*2 Unit assumptions. \funcsat\ supports solving under any number of unit
assumptions, like similar solvers such as \minisat\ and \picosat. But our
interface behaves a bit differently.

\numberedlist

\li |funcsatPushAssumption(f,x)| adds a unit assumption |x|. This means that
every time |funcsatSolve| is called from now on (unless you manipulate the
assumptions again), all satisfying assignments must have |x| set to true. If the
solver returns |FS_UNSAT|, that means there exists no satisfying assignment
where |x| holds. It says {\it nothing} about satisfying assignments where |-x|
holds.

{\bf You must check the return value of |funcsatPushAssumption|!} If it ever
returns |FS_UNSAT|, it means the assumption was false and you should take
appropriate action before solving.

\li |funcsatPopAssumption| will remove the last unit assumption pushed (call it
|x|). All subsequent calls (unless you manipulate the assumptions again) will
not be sensitive to any particular assignment to |x|.

\li Therefore, assumptions are ordered in a LIFO stack.

\li Unlike \picosat, the solver \textit{never} implicitly removes assumptions
after a call to |funcsatSolve|.

\endnumberedlist

@<Global def...@>=
funcsat_result funcsatPushAssumption(funcsat *f, literal p)
{
  if(p == 0) {
    /*This is a hack to get the assumption stacks of bitfunc and funcsat to be the same size*/
    clausePush(&f->assumptions, 0);
    return FS_UNKNOWN;
  }
      
  f->conf->minimizeLearnedClauses = false;
  backtrack(f, 0UL, NULL, true);
  /* in a weird case, there are no clauses but some assumptions, so resize */
  funcsatResize(f, fs_lit2var(p));
  if (funcsatValue(f, p) == false) {
    return FS_UNSAT;
  } else if (funcsatValue(f, p) == unknown) {
    /* copy over assumptions that matter */
    clausePush(&f->assumptions, p);
    trailPush(f, p, NO_CLS);
  } else {
    clausePush(&f->assumptions, 0);
  }
  return FS_UNKNOWN;
}

funcsat_result funcsatPushAssumptions(funcsat *f, clause *c) {
  for(uintmax_t i = 0; i < c->size; i++) {
    if(funcsatPushAssumption(f, c->data[i]) == FS_UNSAT) {
      funcsatPopAssumptions(f, i);
      return FS_UNSAT;
    }
  }
  return FS_UNKNOWN;
}

@

@<Global def...@>=
void funcsatPopAssumptions(funcsat *f, uintmax_t num) {
 
  head_tail facts;
  head_tail_clear(&facts);

  assert(num <= f->assumptions.size);

  backtrack(f, 0, NULL, true);

  for (uintmax_t i = 0; i < num; i++) {
    literal p = clausePop(&f->assumptions);
    if (p == 0) return;

    literal t = trailPop(f, &facts);
  
    while (p != t) {
      t = trailPop(f, &facts);
    }
  }

  restore_facts(f, &facts);
}

@ At any point during solving, we can forcibly reset the solver state using this
function. It does {\it not} remove any assumptions.

@<Global def...@>=

void funcsatReset(funcsat *f)
{
  f->conflict_clause = NO_CLS;
  backtrack(f, 0UL, NULL, true);
  f->propq = 0;
  f->lastResult = FS_UNKNOWN;
}





@*1 Clauses. \definexref{pg-clauses}{\folio}{page}Clauses, as the client
sees them, are represented thusly.

@<External ty...@>=
typedef struct clause
{
  literal *data;

  uint32_t size;

  uint32_t capacity;

  uint32_t isLearnt : 1;
  uint32_t isReason : 1;
  uint32_t is_watched : 1; /* in the watched literals for the first 2 literals of the clause */

  double activity;

  struct clause *nx;
} clause;

@ Function prototypes for allocation, mutation, and deletion of clauses.

@<External decl...@>=
clause *clauseAlloc(uint32_t capacity); /* creates empty clause */

void clauseInit(clause *v, uint32_t capacity); /* initializes manually malloc'd clause */

void clauseDestroy(clause *); /*
  Frees the underlying literal buffer and resets clause metadata. Does not free
  the given pointer. */

void clauseFree(clause *);/* Calls |clauseDestroy| then frees the clause. */

void clauseClear(clause *v); /* 
 Clear the contents of the clause. Does not touch reference count or list
 links. */

void clausePush(clause *v, literal data); /*
 Allocates space (if necessary) for another literal; then appends the given
 literal. */

void clausePushAt(clause *v, literal data, uint32_t i);
void clauseGrowTo(clause *v, uint32_t newCapacity);
literal clausePop(clause *v);
literal clausePopAt(clause *v, uint32_t i);
literal clausePeek(clause *v);

void clauseCopy(clause *dst, clause *src); /*
 Copies all the literals and associated metadata, but the |dst| reference count
 is 1.
 */

#define forClause(elt, vec) for (elt = (vec)->data; elt != (vec)->data + (vec)->size; elt++)
#define for_clause(elt, cl) for (elt = (cl)->data; elt != (cl)->data + (cl)->size; elt++)
#define for_clause99(elt, cl) for (literal *elt = (cl)->data; \
			  elt != (cl)->data + (cl)->size; elt++)


@
@<Global def...@>=
void clauseInit(clause *v, uint32_t capacity)
{
  uint32_t c = capacity > 0 ? capacity : 4;
  FS_CALLOC(v->data, c, sizeof(*v->data));
  v->size = 0;
  v->capacity = c;
  v->isLearnt = false;
  v->nx = NULL;
  v->is_watched = false;
  v->isReason = false;
  v->activity = 0.f;
}

void clauseFree(clause *v)
{
  clauseDestroy(v);
  free(v);
}

void clauseDestroy(clause *v)
{
  free(v->data);
  v->data = NULL;
  v->size = 0;
  v->capacity = 0;
  v->isLearnt = false;
}

void clauseClear(clause *v)
{
  v->size = 0;
  v->isLearnt = false;
}

void clausePush(clause *v, literal data)
{
  if (v->capacity <= v->size) {
    while (v->capacity <= v->size) {
      v->capacity = v->capacity*2+1;
    }
    FS_REALLOC(v->data, v->capacity, sizeof(*v->data));
  }
  v->data[v->size++] = data;
}

void clausePushAt(clause *v, literal data, uint32_t i)
{
  uint32_t j;
  assert(i <= v->size);
  if (v->capacity <= v->size) {
    while (v->capacity <= v->size) {
      v->capacity = v->capacity*2+1;
    }
    FS_REALLOC(v->data, v->capacity, sizeof(*v->data));
  }
  v->size++;
  for (j = v->size-(uint32_t)1; j > i; j--) {
    v->data[j] = v->data[j-1];
  }
  v->data[i] = data;
}


void clauseGrowTo(clause *v, uint32_t newCapacity)
{
  if (v->capacity < newCapacity) {
    v->capacity = newCapacity;
    FS_REALLOC(v->data, v->capacity, sizeof(*v->data));
  }
  assert(v->capacity >= newCapacity);
}


literal clausePop(clause *v)
{
  assert(v->size != 0);
  return v->data[v->size-- - 1];
}

literal clausePopAt(clause *v, uint32_t i)
{
  uint32_t j;
  assert(v->size != 0);
  literal res = v->data[i];
  for (j = i; j < v->size-(uint32_t)1; j++) {
    v->data[j] = v->data[j+1];
  }
  v->size--;
  return res;
}

literal clausePeek(clause *v)
{
  assert(v->size != 0);
  if(v->size == 0) {
    v->size = 1;
    return 0;
  }
  return v->data[v->size-1];
}

void clauseSet(clause *v, uint32_t i, literal p)
{
  v->data[i] = p;
}

void clauseCopy(clause *dst, clause *src)
{
  literal i;
  for (i = 0; i < src->size; i++) {
    clausePush(dst, src->data[i]);
  }
  dst->isLearnt = src->isLearnt;
}




@
@<Global def...@>=
void fs_clause_print(funcsat *f, FILE *out, clause *c)
{
  if (!out) out = stderr;
  literal *p;
  for_clause (p,c) {
    fprintf(out, "%ji_%ji%s ", *p, levelOf(f, fs_lit2var(*p)),
            (funcsatValue(f, *p) == true ? "T" :
             (funcsatValue(f, *p) == false ? "F" : "U")));
  }
}

@ Print |clause|.
@<Global def...@>=
void dimacsPrintClause(FILE *out, clause *c)
{
  literal *p;
  if (!out) out = stderr;
  for_clause (p, c) {
    fprintf(out, "%ji ", *p);
  }
  fprintf(out, "0");
}
  
@

@<Internal decl...@>=
void fs_clause_print(funcsat *f, FILE *out, clause *c);
void dimacsPrintClause(FILE *out, clause *c);


@*2 Internal clauses.

\numberedlist

\li Clause heads will be separate from the ``rest'' of the clause. In fact, all
clauses will be chunked into head and body parts. There is one head and various
body parts. Each clause is referred to by a single index, a |uintptr_t|. See the
macros |clause_head_ptr| and |clause_block_ptr|.

\li The activity and the score need to be unified. First of all, the \LBD\ score
has way too many bits. After about \osf{5} bits of information all the \LBD s
blend together. The activity is slightly murkier because it's a float. Either I
need to (\osf{1}) create custom float operations packed into 28 bits or
(\osf{2}) using the int and make the activity recording work anyway. Not sure at
the moment how to do either but I know \picosat\ does (\osf{1}). (Can I get away
with fixed-point floats?)

\endnumberedlist

So here's the new |clause_head| data structure. The |lits| is |CLAUSE_HEAD_SIZE|
literals. |nx| is an index it's an index into |f->clblocks|.

The |clause_block| is similar. If the clause terminates in this block, then
|nx| is |NO_CLS|. If there is a spot for it, the literal after the last
literal in the clause is a 0. If there isn't a spot for it, then |lits| just has
all the literals.

Each clause block will be allocated from the huge array, |f->clblocks|. When due
to simplification or garbage collection a clause block is released, it is put
back onto the freelist of clause blocks. |f->block_freelist| is the index of the
first free |clause_block| in the freelist.

@s clause_head int
@s clause_block int
@s clause_state int

@<Internal ty...@>=
#ifndef CLAUSE_HEAD_SIZE
#  define CLAUSE_HEAD_SIZE 7    /* odd because of |clause_head->nx| */
#endif

#ifndef CLAUSE_BLOCK_SIZE
#  define CLAUSE_BLOCK_SIZE 7   /* odd because of |clause_block->nx| */
#endif

enum clause_state
{
  CLAUSE_WATCHED=1,             /* in watchlist */
  CLAUSE_JAILED,                /* in jail */
  CLAUSE_UNIT                   /* in unit facts, don't GC */
};

#define NO_CLS UINTPTR_MAX
#define LBD_SCORE_MAX 0x3f

struct clause_head
{
  literal lits[CLAUSE_HEAD_SIZE]; /* first few literals */
  uintptr_t nx;               /* index of first block, or |NO_CLS| if none */
  uintptr_t link;             /* used by |head_tail| lists. |NO_CLS| initially */

  uint32_t is_learned : 1;
  uint32_t is_reason : 1; /* pinned, don't GC */
  uint32_t where : 2; /* see |clause_state| */
  uint32_t lbd_score : 6;       /* also |LBD_SCORE_MAX| */
  uint32_t sz : 22;             /* max num lits is 2**22 */
  float activity;

};
#include "funcsat/vec_clause_head.h"

struct clause_block
{
  literal lits[CLAUSE_BLOCK_SIZE];
  uintptr_t nx;                 /* index of next block, or |NO_CLS| if none */
};
#include "funcsat/vec_clause_block.h"

@ In the |funcsat| type we need to store a vector of clause heads/blocks along
with a freelist pointer.

@<Internal ty...@>=
struct clause_head_pool
{
  uintptr_t freelist;
  struct vec_clause_head *heads;
};
struct clause_block_pool
{
  uintptr_t freelist;
  struct vec_clause_block *blocks;
};

@ What is the lifecycle of a clause? And why should you care? If the lifecycle
is in one place, it's easy to figure out whether later clause modifications will
efficiently support the needed operations.

\unorderedlist

\li While the client is constructing the CNF instance (or it's being read from
disk), the clause is being built and eventually is given to |funcsatAddClause|.

  \orderedlist

  \li If the clause is empty, the solver becomes immediately conflicted.

  \li If the clause is singleton, we put it on the trail and attach it as a {\it
  unit fact} to the newly-added trail literal.

  \li If it's binary, it's put in the binary watcher list and it will never be
  garbage collected.

  \li Otherwise, it's put in the normal watchlist for the clause.

  \endorderedlist

\li Once all the clauses are added the search begins. At this point a clause can
become the {\it reason} for an inference. This means the clause was unit at the
time it implied a literal.

\li During backtracking a clause (especially learned) may transition to a unit
fact list which is eventually associated with a particular literal on the
trail. It may stay on a unit fact list across other decisions, inferences, and
backtracks. (But not across restarts unless it's a singleton.)

\li During \BCP\ a clause can be {\it jailed}, meaning it's removed from its
watchlists and put into a shadow watchlist while it's satisfied. It is restored
to the watchlists once it becomes unsatisfied. (It does {\it not} go through any
unit fact list.)

\li A literal could be completely deleted from a clause if a literal in the
clause becomes true at decision level 0.


\endunorderedlist

When a garbage collection is triggered, the clause could be in any of these
states. Assuming the clause is not pinned, it is returned to the clause pool. A
clause is {\it pinned} when any of the following hold:

  \unorderedlist

  \li The clause is in a unit fact list.
  \li The clause is currently the {\it reason} for an inference.
  \li The clause is binary (we keep them all around).

  \endunorderedlist

@ Initialize the pools for heads and blocks.

@d INITIAL_CLAUSE_POOL 1024

@<Initialize func...@>=
f->clheads.freelist = NO_CLS;
f->clheads.heads = vec_clause_head_init(INITIAL_CLAUSE_POOL);
f->clblocks.freelist = NO_CLS;
f->clblocks.blocks = vec_clause_block_init(INITIAL_CLAUSE_POOL);
for (uintptr_t i = 0; i < INITIAL_CLAUSE_POOL; i++) {
  clause_head_release(f,i);
  clause_block_release(f,i);
}


@ Free the pools.

@<Destroy func...@>=
vec_clause_head_destroy(f->clheads.heads);
vec_clause_block_destroy(f->clblocks.blocks);
f->clheads.freelist = NO_CLS;
f->clheads.freelist = NO_CLS;

@ Retrieving new clauses. The |capacity| of the vec is how many blocks are
initially available; if we need to allocate more, we simply allocate more
memory. The |size| field is ignored.

@<Internal decl...@>=
static inline struct clause_head *clause_head_ptr(funcsat *f, uintptr_t i)
{
  return (assert((i) < (f)->clheads.heads->capacity),(f)->clheads.heads->data+(i));
}
static inline struct clause_block *clause_block_ptr(funcsat *f, uintptr_t i)
{
  return (assert((i) < (f)->clblocks.blocks->capacity),(f)->clblocks.blocks->data+(i));
}


@ To allocate a clause head we find the first head in the freelist, allocating
new space if necessary. In the fast case, when there is a free head available,
this operation takes constant time. In the slow case we allocate a bunch of new
heads and put them into the freelist.
@<Global def...@>=
static inline uintptr_t clause_head_mk(funcsat *f, uint32_t sz_hint)
{
  const uintptr_t cap = f->clheads.heads->capacity;
  uintptr_t freelist = f->clheads.freelist;
  if (freelist >= cap) {
    /* resize if necessary */
    vec_clause_head_grow_to(f->clheads.heads, cap*2);
    for (uintptr_t i = cap; i < f->clheads.heads->capacity; i++) {
      clause_head_release(f,i);
    }
  }
  struct clause_head *h = clause_head_ptr(f, freelist = f->clheads.freelist);
  f->clheads.freelist = h->nx; /* use |h->nx| before we clobber it */

  memset(h, 0, sizeof(*h));
  h->nx = h->link = NO_CLS;
  return freelist;
}

static inline uintptr_t clause_block_mk(funcsat *f)
{
  const uintptr_t cap = f->clblocks.blocks->capacity;
  uintptr_t freelist = f->clblocks.freelist;
  if (freelist >= cap) {
    /* resize if necessary */
    vec_clause_block_grow_to(f->clblocks.blocks, cap*2);
    for (uintptr_t i = cap; i < f->clblocks.blocks->capacity; i++) {
      clause_block_release(f,i);
    }
  }
  struct clause_block *h = clause_block_ptr(f, freelist = f->clblocks.freelist);
  f->clblocks.freelist = h->nx; /* use |h->nx| before we clobber it */

  memset(h, 0, sizeof(*h));
  h->nx = NO_CLS;
  return freelist;
}


@ Return the clause to the pool.

@<Global def...@>=
static inline void clause_head_release(funcsat *f, uintptr_t ix)
{
  assert(ix < f->clheads.heads->capacity);
  struct clause_head *h = clause_head_ptr(f,ix);
  assert(memset(h, 0, sizeof(struct clause_head)));
  h->nx = f->clheads.freelist, f->clheads.freelist = ix;
}

static inline void clause_block_release(funcsat *f, uintptr_t ix)
{
  assert(ix < f->clblocks.blocks->capacity);
  struct clause_block *h = clause_block_ptr(f,ix);
  assert(memset(h, 0, sizeof(struct clause_block)));
  h->nx = f->clblocks.freelist, f->clblocks.freelist = ix;
}

@ To return a complete clause, blocks and everything, to the pool.
@<Global def...@>=
static inline void clause_release(funcsat *f, uintptr_t ix)
{
  struct clause_head *h = clause_head_ptr(f, ix);
  struct clause_block *b;
  uintptr_t nx = h->nx;
  clause_head_release(f, ix);
  while (nx != NO_CLS) {
    uintptr_t nx_nx = clause_block_ptr(f, nx)->nx;
    clause_block_release(f, nx);
    nx = nx_nx;
  }
}

@ Iterating over clauses. The iterator type is somewhat complex, unfortunately.

@<Internal ty...@>=
struct clause_iter
{
  int16_t l_ix; /* can be tiny because it only indexes into a block at a time */
  bool is_head;
  uint32_t sz;
  union
  {
    struct clause_head  *h;
    struct clause_block *b;
  };
};

@ Steps for iterating:

\unorderedlist

\li |clause_iter_init(f,i,it)|: initialize an iterator |it|. You must call
|clause_iter_next| to get the next element.

\li |clause_iter_next|: if it returns a non-|NULL| pointer, then it points to a
|literal|.
\endunorderedlist

@<Global def...@>=
void clause_iter_init(funcsat *f, uintptr_t ix_clause_head, struct clause_iter *it)
{
  struct clause_head *h = clause_head_ptr(f, ix_clause_head);
  it->l_ix = -1;
  it->is_head = true;
  it->h = h;
  it->sz = h->sz;
}

#define clause_iter_lit_ptr(f, it) ((it)->is_head ?             \
                                    (it)->h->lits+(it)->l_ix :  \
                                    (it)->b->lits+(it)->l_ix)

literal *clause_iter_next(funcsat *f, struct clause_iter *it)
{
  literal *lit = NULL;
  if (it->sz == 0) return NULL;
  ++it->l_ix;

  if (it->is_head) {
    if (it->l_ix >= CLAUSE_HEAD_SIZE) { /* go to first block */
      assert(it->h->nx != NO_CLS);
      it->is_head = false;
      it->b = clause_block_ptr(f, it->h->nx);
      it->l_ix = 0;
    }
  } else {
    if (it->l_ix >= CLAUSE_BLOCK_SIZE) { /* go to next block */
      assert(it->b->nx != NO_CLS);
      it->b = clause_block_ptr(f, it->b->nx);
      it->l_ix = 0;
    }
  }

  lit = clause_iter_lit_ptr(f, it);
  it->sz--;
  return lit;
}

@ Make printing convenient.
@<Global def...@>=
void fs_clause_head_print(funcsat *f, FILE *out, uintptr_t ix)
{
  if (!out) out = stderr;
  struct clause_iter it;
  clause_iter_init(f, ix, &it);
  for (literal *p = clause_iter_next(f, &it); p; p = clause_iter_next(f, &it)) {
    fprintf(out, "%ji_%ji%s ", *p, levelOf(f, fs_lit2var(*p)),
            (funcsatValue(f, *p) == true ? "T" :
             (funcsatValue(f, *p) == false ? "F" : "U")));
  }
}

void clause_head_print_dimacs(funcsat *f, FILE *out, uintptr_t ix)
{
  struct clause_iter it;
  if (!out) out = stderr;
  if (ix == NO_CLS) {
    fprintf(out, "(NULL CLAUSE)");
    return;
  }
  clause_iter_init(f, ix, &it);
  for (literal *p = clause_iter_next(f, &it); p; p = clause_iter_next(f, &it)) {
    fprintf(out, "%ji ", *p);
  }
  fprintf(out, "0");
}

void vec_clause_head_print_dimacs(funcsat *f, FILE *stream, struct vec_uintptr *clauses)
{
  for_vec_uintptr (ix, clauses) {
    clause_head_print_dimacs(f, stream, *ix);
    fprintf(stream, "\n");
  }
}


@ Prototypes.

@<Internal decl...@>=
static inline uintptr_t clause_head_mk(funcsat *f, uint32_t sz_hint);
static inline uintptr_t clause_block_mk(funcsat *f);
static inline void clause_head_release(funcsat *f, uintptr_t ix);
static inline void clause_block_release(funcsat *f, uintptr_t ix);
static inline void clause_releas(funcsat *f, uintptr_t ix);
void clause_iter_init(funcsat *f, uintptr_t ix_clause_head, struct clause_iter *it);
literal *clause_iter_next(funcsat *f, struct clause_iter *it);
void fs_clause_head_print(funcsat *f, FILE *out, uintptr_t ix);
void clause_head_print_dimacs(funcsat *f, FILE *out, uintptr_t);
void vec_clause_head_print_dimacs(funcsat *f, FILE *out, struct vec_uintptr *);

@
@<Initialize func...@>=
f->orig_clauses = vec_uintptr_init(2);
f->learned_clauses = vec_uintptr_init(2);


@ Destroy
@<Destroy func...@>=
vec_uintptr_destroy(f->orig_clauses);
vec_uintptr_destroy(f->learned_clauses);

@*2 Head-tail lists. Sometimes we use a head-tail representation for lists of
clauses. The |head_tail| structure points at the initial clause and the last
clause. The clauses in between the head and the tail are found by walking each
|clause->nx| pointer. Using these scheme we can append two such lists in
constant time.

@<Internal ty...@>=
typedef struct head_tail
{
  uintptr_t hd;                 /* |NO_CLS| used if empty */
  uintptr_t tl;
} head_tail;

@ Clearing and testing for emptiness should be clear (ha!) enough.
@<Global def...@>=
static inline void head_tail_clear(head_tail *ht) {
  ht->hd = ht->tl = NO_CLS;
}

static inline bool head_tail_is_empty(head_tail *ht) {
  return ht->hd == NO_CLS && ht->tl == NO_CLS;
}

@ The result will be stored in |ht1|. If we think of |ht1| as a list
$x_1\rightarrow\cdots\rightarrow x_m$ and |ht2| as a list
$y_1\rightarrow\cdots\rightarrow y_n$ then we want to make |ht1| the list
$x_1\rightarrow\cdots\rightarrow x_m\rightarrow y_1\rightarrow\cdots\rightarrow
y_n$ in that order.

The complication is that either or both lists may be empty. But we need to:
\unorderedlist

\li If $m>0$, adjust $x_m$'s |link| to be $y_1$. (If $n=0$, this will simply
reinforce that $x_m$|->link| is |NO_CLS|.)

\li If $n>0$, adjust |ht1|'s |tl| to be $y_n$. (And if |ht1| is empty we make
sure to make it start at $y_1$.

\endunorderedlist
@<Global def...@>=
static inline void head_tail_append(funcsat *f, head_tail *ht1, head_tail *ht2) {
  if (ht1->tl != NO_CLS)
    clause_head_ptr(f, ht1->tl)->link = ht2->hd;
  if (ht2->tl != NO_CLS) {
    ht1->tl = ht2->tl;
    if (ht1->hd == NO_CLS)
      ht1->hd = ht2->hd;
  }
  head_tail_clear(ht2);
#if 0
  if (!head_tail_is_empty(ht2)) {
    if (head_tail_is_empty(ht1)) {
      ht1->hd = ht2->hd;
    } else {
      struct clause_head *cl_tl = clause_head_ptr(f, ht1->tl);
      assert(ht1->hd != NO_CLS);
      assert(cl_tl->link == NO_CLS);
      cl_tl->link = ht2->hd;
    }
    ht1->tl = ht2->tl;
  }
#endif
}

@ Adding a single clause to the list is a special case of |head_tail_append|.
@<Global def...@>=
static inline void head_tail_add(funcsat *f, head_tail *ht1, uintptr_t ix) {
  head_tail ht2;
  ht2.hd = ht2.tl = ix;
  head_tail_append(f, ht1, &ht2);
#if 0
  struct clause_head *c = clause_head_ptr(f, ix);
  c->link = ht1->hd;
  ht1->hd = ix;
  if (ht1->tl == NO_CLS) {
    ht1->tl = ix; }
#endif
}

@
@<Internal decl...@>=
static inline void head_tail_clear(head_tail *ht);
static inline bool head_tail_is_empty(head_tail *ht);
static inline void head_tail_append(funcsat *f, head_tail *ht1, head_tail *ht2);
static inline void head_tail_add(funcsat *f, head_tail *ht1, uintptr_t);


@ Iterating over a |head_tail| list is slightly tricky. The common case is
iterating and doing something to each element, without removing it. That's
pretty easy.

But often we want to preserve some elements in the list and delete others, while
we iterate. That's what |head_tail_iter_rm| is for. It deletes the |curr|
element from the list and sets |curr->nx| to |NULL|. The iterator can therefore
detect that |curr| has been deleted (by testing its link field) and not update
the |prev| pointer when that happens.

@d for_head_tail(f, ht, prev, curr, next) /* iterate over all the clauses in the list */
    for (prev = next = NO_CLS, curr = (ht)->hd,
           ((curr != NO_CLS) ? next = clause_head_ptr(f, (curr))->link : 0);@/
         (curr != NO_CLS);@/
         (clause_head_ptr(f, (curr))->link != NO_CLS ? prev = (curr) : 0),
           curr = (next), ((curr != NO_CLS) ? next = clause_head_ptr(f, (curr))->link : 0))

@d head_tail_iter_rm(f, ht, prev, curr, next) /* delete |curr| from iteration */
    if ((ht)->hd == (curr)) { /* fix up |hd| and |tl| fields */
      (ht)->hd = (next);
    }
    if ((ht)->tl == (curr)) {
      (ht)->tl = (prev);
    }
    if (prev != NO_CLS) {
      clause_head_ptr(f, (prev))->link = (next);
    }
    clause_head_ptr(f, (curr))->link = NO_CLS;

@c


@

@<Global def...@>=
inline void head_tail_print(funcsat *f, FILE *out, head_tail *l)
{
  if (!out) out = stderr;
  if (l->hd) {
    uintptr_t p, c, nx;
    for_head_tail (f, l, p, c, nx) {
      struct clause_head *h = clause_head_ptr(f,c);
      if (h->link == NO_CLS && l->tl != c) {
        fprintf(out, "warning: tl is not last clause\n");
      }
      fs_clause_head_print(f, out, c);
      fprintf(out, "\n");
    }
  } else if (l->tl != NO_CLS) {
    fprintf(out, "warning: hd unset but tl set!\n");
  }
}

@

@<Internal decl...@>=
extern void head_tail_print(funcsat *f, FILE *out, head_tail *l);


@*1 BCP. How does \funcsat's \BCP\ work? Here's the signature.

@<Internal decl...@>=
bool bcp(funcsat *f);

@ First, there's this notion of a {\it propagation queue} (|funcsat->propq|). It
is an index into the trail. It is the index of the earliest literal on the trail
that should be examined by \BCP. \BCP\ examines each literal, beginning with the
first in the propagation queue, until there are no more inferences or it finds a
conflict.

If we discover a conflict, |bcp| returns |false| and sets
|funcsat->conflictClause| to the clause that is false. Otherwise it returns
|true|.

@<Global def...@>=
bool bcp(funcsat *f)
{
  bool isConsistent = true;

  while (f->propq < f->trail.size) {
    @<\BCP\ clauses@>@;

    if(f->BCP_hook) {
      if(f->BCP_hook((uintptr_t *)f->conf->user, imaxabs(p), p > 0) == FS_UNSAT) {
        isConsistent = false;
        goto bcp_conflict;
      }
    }
  }

bcp_conflict:
  return isConsistent;
}

@ We put learned clauses in our normal watchlists. Each clause knows whether
it's learned or not.

\numberedlist

\li Con: when garbage collection is done, the learned watchlists are entirely
recreated. This is easiest because the GC can sort all the clauses by score,
delete the worst half, and recreate the watchlists. How could this be resolved?

  \numberedlist

  \li Can I add some information to the clauses themselves and GC them during
  \BCP? (This might be bad because it might make \BCP\ slow. But let's have some
  faith in branch prediction on processors, shall we?)

  \li At first, I'll ignore cleaning learned clauses. Next, I'll do a special
  walk over the whole watcher list and clean out certain of the learned
  clauses. Next, I'll think of some queue-of-craptastry in order to drop clauses
  more dynamically and in a fine-grained way.

  \endnumberedlist

\li Pro: every single watched literal operation does not have to be duplicated
for the original and learned lists, in the other representation. This is
annoying.

\endnumberedlist

@ Here we are using a representation inspired by the ``Cache-conscious SAT''
paper.

The watched literal structure is indexed by literals. At each index is stored a
|watchlist|, which points to a number of |watchlist_elt|s. The meaning of a
|watchlist_elt| differs depending on the clause:

\numberedlist

\li For large clauses: a literal from the clause (|lit|), clause index
(|cls|). (This is different from the paper. I can't figure out how they
managed to keep the {\it first} literal in the |watchlist_elt| at all times!)

\li For binary clauses: the other literal (|lit|), clause index
(|cls|). (This situation is different from the paper; there the clause
index was unused. We need it because we always store a clause index to each
clauses that is a {\it reason} for an inference.)

\endnumberedlist

@<Internal ty...@>=
struct watchlist_elt
{
  literal   lit;
  uintptr_t cls;
};


@ Here's the |watchlist|, which represents the list of clauses associated with a
single watched literal. Each |watchlist| stores a small number of clauses in its
head, because often the watch lists are small. If the watch list is small, we
don't need to fetch anything else from memory in order to propagate this watch
list.

I chose a size of 12 for |WATCHLIST_HEAD_SIZE_MAX| because I felt like it. And
because it was in the paper.

@<Internal ty...@>=
#ifndef WATCHLIST_HEAD_SIZE_MAX
#  define WATCHLIST_HEAD_SIZE_MAX 4
#endif

struct watchlist
{
  uint32_t size;               /* of |watchlist->elts + watchlist->rest| */
  uint32_t capacity;           /* of |watchlist->rest| */

  struct watchlist_elt elts[WATCHLIST_HEAD_SIZE_MAX];
  struct watchlist_elt *rest;
};

@ Traversing the watchlists. Using these macros makes the |bcp| code a bit
cleaner.

@d watchlist_next_elt(elt, wl)
  ((elt) + 1 == (wl)->elts + WATCHLIST_HEAD_SIZE_MAX ? elt = (wl)->rest : (elt)++)
@d for_watchlist(elt, dump, wl) /* |elt| ptr has to transition from head list to rest */
  for (elt = dump = (wl)->elts;
       (watchlist_is_elt_in_head(elt, (wl))
          ? elt < watchlist_head_size_ptr(wl)
          : elt < watchlist_rest_size_ptr(wl));
       watchlist_next_elt(elt, (wl)))

@ The |all_watches| is the vector of all the watcher lists for every literal.

@<Internal ty...@>=
typedef struct all_watches
{
  struct watchlist *wlist; /* indexed by |fs_lit2idx(i)| */
  uintmax_t size;
  uintmax_t capacity;
} all_watches;

@ Initializing.

@<Global def...@>=
static inline void all_watches_init(funcsat *f)
{
  FS_CALLOC(f->watches.wlist, 1<<7, sizeof(*f->watches.wlist));
  f->watches.size = 2;
  f->watches.capacity = 1<<7;
}

@ 

@<Global def...@>=
static inline void all_watches_destroy(funcsat *f)
{
  free(f->watches.wlist);
}

@ These used to be macros, but then GDB on OS X wouldn't let me call them no
matter how many funny {\tt -g} options I passed to gcc. So I made them
functions.

@<Internal decl...@>=
static inline bool watchlist_is_elt_in_head(struct watchlist_elt *elt, struct watchlist *wl)
{
  return (elt >= wl->elts && elt < (wl)->elts + WATCHLIST_HEAD_SIZE_MAX);
}
static inline uint32_t watchlist_head_size(struct watchlist *wl)
{
  return ((wl)->size > WATCHLIST_HEAD_SIZE_MAX ? WATCHLIST_HEAD_SIZE_MAX : (wl)->size);
}
static inline struct watchlist_elt *watchlist_head_size_ptr(struct watchlist *wl)
{
  return ((wl)->elts + watchlist_head_size(wl));
}
static inline uint32_t watchlist_rest_size(struct watchlist *wl)
{
  return ((wl)->size > WATCHLIST_HEAD_SIZE_MAX ? (wl)->size - WATCHLIST_HEAD_SIZE_MAX : 0);
}
static inline struct watchlist_elt *watchlist_rest_size_ptr(struct watchlist *wl)
{
  return ((wl)->rest + watchlist_rest_size(wl));
}


@

@<Internal decl...@>=
static inline void all_watches_init(funcsat *f);
static inline void all_watches_destroy(funcsat *f);

@

@<Incrementally resize internal...@>=
  if (f->watches.capacity <= f->watches.size+2) {
    while (f->watches.capacity <= f->watches.size) {
      f->watches.capacity = f->watches.capacity*2+2;
    }
    FS_REALLOC(f->watches.wlist, f->watches.capacity, sizeof(*f->watches.wlist));
  }

  @<Initialize wlist bucket...@>@;
  f->watches.size++;
  @<Initialize wlist bucket...@>@;
  f->watches.size++;
  assert(f->watches.size <= f->watches.capacity);

@

@<Initialize wlist bucket at |f->watches.size|@>=
f->watches.wlist[f->watches.size].size = 0;
f->watches.wlist[f->watches.size].capacity = 0;
for (uint32_t i = 0; i < WATCHLIST_HEAD_SIZE_MAX; i++) {
  f->watches.wlist[f->watches.size].elts[i].lit = 0;
  f->watches.wlist[f->watches.size].elts[i].cls = NO_CLS;
}
f->watches.wlist[f->watches.size].rest = NULL;
assert(0 == watchlist_head_size(&f->watches.wlist[f->watches.size]));
assert(0 == watchlist_rest_size(&f->watches.wlist[f->watches.size]));


@ Destroy the watcher lists.

@<Destroy func...@>=

for (variable i = 1; i <= f->numVars; i++) {
  free(f->watches.wlist[fs_lit2idx((literal)i)].rest);
  free(f->watches.wlist[fs_lit2idx(-(literal)i)].rest);
}
free(f->watches.wlist);


@ 
@<Global def...@>=
static int compare_pointer(const void *x, const void *y)
{
  uintptr_t xp = (uintptr_t)*(clause **)x;
  uintptr_t yp = (uintptr_t)*(clause **)y;
  if (xp < yp) return -1;
  else if (xp > yp) return 1;
  else return 0;
}


@ 
@<Internal decl...@>=
static int compare_pointer(const void *x, const void *y);


@ The watchlist invariants can be checked with this function.
@<Global def...@>=
static inline void watchlist_check(funcsat *f, literal l)
{
  literal false_lit = -l;

  struct watchlist *wl = &f->watches.wlist[fs_lit2idx(l)];
  struct watchlist_elt *elt, *dump;

  struct vec_ptr *clauses = vec_ptr_init(wl->size);
  for_watchlist (elt, dump, wl) {
    struct clause_head *c = clause_head_ptr(f, elt->cls);
    literal *chkl; bool chk_in_cls = false;
    struct clause_iter it;
    vec_ptr_push(clauses, c);
    clause_iter_init(f, elt->cls, &it);
    for (literal *p = clause_iter_next(f, &it); p; p = clause_iter_next(f, &it)) {
      /* ensure |elt->lit| is in |c| */
      if (*p == elt->lit) {
        chk_in_cls = true;
        break;
      }
    }
    assert(chk_in_cls && "watched lit not in clause");

    assert((c->lits[0] == false_lit || c->lits[1] == false_lit) &&
           "watched lit not in first 2");

    uint32_t num_not_false = 0; /* not-false literals are ``safe'' to watch */
    clause_iter_init(f, elt->cls, &it);
    for (literal *p = clause_iter_next(f, &it); p; p = clause_iter_next(f, &it)) {
      if (tentativeValue(f, *p) != false) {
        num_not_false++;
        if (funcsatValue(f, *p) == true) {
          num_not_false = 0;
          break;
        }
      }
    }
    if (num_not_false >= 1) { /* ensure we're watching ``safe'' literals */
      assert((tentativeValue(f, c->lits[0]) != false ||
              tentativeValue(f, c->lits[1]) != false) &&
               "watching bad literals");
      if (num_not_false >= 2) {
        assert(tentativeValue(f, c->lits[0]) != false &&
               tentativeValue(f, c->lits[1]) != false &&
               "watching bad literals");
      }
    }
  }
  
  qsort(clauses->data, clauses->size, sizeof(struct clause_head *), compare_pointer);
  
  for (uintmax_t i = 0, j = 1; j < clauses->size; i++, j++) {
    /* find duplicate clauses */
    assert(clauses->data[i] != clauses->data[j] && "duplicate clause");
  }
  vec_ptr_destroy(clauses);
}



@ We can check all the watchlists at once.
@<Global def...@>=
static inline void watches_check(funcsat *f)
{
  for (variable v = 1; v <= f->numVars; v++) {
    literal pos = (literal)v;
    literal neg = -(literal)v;
    watchlist_check(f, pos);
    watchlist_check(f, neg);
  }
}

@ 
@<Internal decl...@>=
static inline void watchlist_check(funcsat *f, literal l);
static inline void watches_check(funcsat *f);


@ With all this machinery we can now handle discovering unit inferences and
propagating them.

@<\BCP\ clauses@>=
literal p = f->trail.data[f->propq];
#ifndef NDEBUG
watchlist_check(f,p);
#endif
fs_ifdbg (f, "bcp", 3) { fslog(f, "bcp", 3, "bcp on %ji\n", p); }
const literal false_lit = -p;

struct watchlist *wl = &f->watches.wlist[fs_lit2idx(p)];
struct watchlist_elt *elt, *dump;
uint32_t new_size = 0;

++f->numProps;
dopen(f, "bcp");

for_watchlist (elt, dump, wl) {
  struct clause_head *c = clause_head_ptr(f, elt->cls);
  literal otherlit; /* the other watched lit in |c| */
  mbool litval;
  fs_ifdbg (f, "bcp", 9) {
    fslog(f, "bcp", 9, "visit ");
    fs_clause_head_print(f, fs_dbgout(f), elt->cls);
    fprintf(fs_dbgout(f), " %" PRIuPTR "\n", elt->cls);
  }
  assert((false_lit == c->lits[0] || false_lit == c->lits[1]));
  assert((c->link == NO_CLS));
  @<Check |elt->lit| for quick SAT clause@>@;
  @<Ensure false lit...@>@;
  otherlit = c->lits[0];
  @<Look for new, unfalsified literal to watch, removing |elt| if found@>@;

  litval = funcsatValue(f, otherlit);
  if (litval == true) goto watch_continue;
  if (litval == false) {
    @<Record conflict and return@>@;
  } else {
    @<Record new unit inference@>@;
  }
watch_continue:
  *dump = *elt, watchlist_next_elt(dump, wl); /* keep |elt| in this |wl| */
  new_size++;
skip_watchelt_copy:;
}
f->propq++;
wl->size = new_size;
dclose(f, "bcp");

@ The |watchlist_elt| is in the cache line. We have stored a literal from the
clause in there so that we can quickly test whether it's true; if it is, we can
ignore the current clause and go on to the next one.

@<Check |elt->lit| for quick SAT...@>=
    if (funcsatValue(f, elt->lit) == true) goto watch_continue; /*
    If |elt->lit| is |true|, clause already satisfied */


@ Just for organization's sake, we put the watched literal (which is false) into
|c->lits[1]|.

@<Ensure false literal is in |c->lits[1]|@>=
if (c->lits[0] == false_lit) {
  literal tmp = c->lits[0];
  elt->lit = c->lits[0] = c->lits[1], c->lits[1] = tmp;
  assert(c->lits[1] == false_lit);
  fs_ifdbg (f, "bcp", 10) {
    fslog(f, "bcp", 9, "swapped ");
    fs_clause_head_print(f, fs_dbgout(f), elt->cls), fprintf(fs_dbgout(f), "\n"); }
}
assert(c->lits[1] == false_lit);


@ Our goal is to discover whether the clause |c| is \textit{unit} or not. To do
that we check the clause (excepting the two first literals) for any literal that
is not false. If we fail to find one, since we have ensured that the first
literal is in |c->lits[1]| (see |@<\BCP\ cl...@>|), the clause is unit or false.

@<Look for new, unfalsified...@>=
struct clause_iter it;
clause_iter_init(f, elt->cls, &it);
clause_iter_next(f, &it), clause_iter_next(f, &it);
for (literal *p = clause_iter_next(f, &it); p; p = clause_iter_next(f, &it)) {
  mbool v = funcsatValue(f, *p);
  if (v != false) {
    c->lits[1] = *p, *p = false_lit;
    watch_l1(f, elt->cls);
    fs_ifdbg (f, "bcp", 9) {
      fslog(f, "bcp", 9, "moved ");
      fs_clause_head_print(f, fs_dbgout(f), elt->cls), NEWLINE(fs_dbgout(f));
    }
    elt->cls = NO_CLS; elt->lit = 0;
    goto skip_watchelt_copy;
  }
}

@ Recording the conflict should be straightforward, but we have to be careful
about the clauses after the conflict clause in this |watchlist|.  The loop below
ensures they're copied back into the current |watchlist| before we stop \BCP.

|for_watchlist_continue| simply omits the initialization part of |for_watchlist|,
but is otherwise identical. At this point we have found a false clause (a
conflict) and need to make sure not to drop the other watched clauses down the
disposal.

@d for_watchlist_continue(elt, dump, wl)
  for (;
       (watchlist_is_elt_in_head(elt, wl)
          ? elt < watchlist_head_size_ptr(wl)
          : elt < watchlist_rest_size_ptr(wl));
       watchlist_next_elt(elt, wl))

@<Record conflict...@>=
isConsistent = false;
f->conflict_clause = elt->cls;
for_watchlist_continue (elt, dump, wl) { /* save rest of watched clauses */
  *dump = *elt, watchlist_next_elt(dump, wl);
  new_size++;
}
wl->size = new_size;
dclose(f, "bcp");
goto bcp_conflict;

@ It's simple, really.

@<Record new unit...@>=
fslog(f, "bcp", 2, " => %ji (%s:%d)\n", otherlit, __FILE__, __LINE__);
trailPush(f, otherlit, reason_info_mk(f, elt->cls));
f->conf->bumpUnitClause(f, elt->cls);

@ We have a few helpers for watching literals 0 and 1 of a clause.

@<Global def...@>=
static inline struct watchlist_elt *watch_lit(funcsat *f, struct watchlist *wl,
                                              uintptr_t ix)
{
  struct watchlist_elt *ret;
  if (watchlist_head_size(wl) < WATCHLIST_HEAD_SIZE_MAX) {
    assert(watchlist_rest_size(wl) == 0);
    ret = &wl->elts[watchlist_head_size(wl)];
    wl->elts[watchlist_head_size(wl)].cls = ix;
  } else {
    assert(watchlist_head_size(wl) >= WATCHLIST_HEAD_SIZE_MAX);
    @q/*@>@<Allocate and/or grow |wl|'s watchlist element capacity@>@;@q*/@>
    ret = &wl->rest[watchlist_rest_size(wl)];
    wl->rest[watchlist_rest_size(wl)].cls = ix;
  }
  wl->size++;
  return ret;
}
static inline void watch_l0(funcsat *f, uintptr_t ix)
{
  struct clause_head *c = clause_head_ptr(f, ix);
  struct watchlist *wl = &f->watches.wlist[fs_lit2idx(-c->lits[0])];
  struct watchlist_elt *elt = watch_lit(f, wl, ix);
  elt->lit = (c->sz == 2 ? c->lits[1] : c->lits[0]);
}
static inline void watch_l1(funcsat *f, uintptr_t ix)
{
  struct clause_head *c = clause_head_ptr(f, ix);
  struct watchlist *wl = &f->watches.wlist[fs_lit2idx(-c->lits[1])];
  struct watchlist_elt *elt = watch_lit(f, wl, ix);
  elt->lit = (c->sz == 2 ? c->lits[1] : c->lits[0]);
}

@

@<Allocate and/or grow |wl|'s watchlist element capacity@>=
if (wl->capacity > 0) {
    if (watchlist_rest_size(wl) >= wl->capacity) {
      FS_REALLOC(wl->rest, wl->capacity*2, sizeof(*wl->rest));
      wl->capacity *= 2;
    }
} else {
  FS_CALLOC(wl->rest, 8, sizeof(*wl->rest));
  wl->capacity = 8;
}

@

@<Internal decl...@>=
static inline void watch_l0(funcsat *f, uintptr_t ix);
static inline void watch_l1(funcsat *f, uintptr_t ix);

@ When a clause is initially added, we watch it with |addWatch|. This ensures an
invariant holds (see |makeWatchable|) and then adds the clause into the watcher
lists.

@d NEWLINE(s) fprintf((!(s) ? stderr : (s)), "\n")

@<Global def...@>=
void addWatch(funcsat *f, uintptr_t ix)
{
  makeWatchable(f, ix);
  addWatchUnchecked(f, ix);
}

static inline void addWatchUnchecked(funcsat *f, uintptr_t ix)
{
  struct clause_head *c = clause_head_ptr(f, ix);
  fslog(f, "bcp", 1, "watching %li and %li in ", c->lits[0], c->lits[1]);
  fs_ifdbg (f, "bcp", 1) {fs_clause_head_print(f, fs_dbgout(f), ix), NEWLINE(fs_dbgout(f));}
  assert(c->sz > 1);
  watch_l0(f, ix);
  watch_l1(f, ix);
  c->where = CLAUSE_WATCHED;
}

@ A clause which is about we be watched must obey an invariant: that the first
(up to) two literals in the clause must be not false, if there are any such
literals.

@<Global def...@>=
void makeWatchable(funcsat *f, uintptr_t ix)
{
  struct clause_head *c = clause_head_ptr(f, ix);
  struct clause_iter pit, qit;
  clause_iter_init(f, ix, &pit), clause_iter_init(f, ix, &qit);
  variable swap_cnt = 0;
  for (literal *p = clause_iter_next(f, &pit), *q = clause_iter_next(f, &qit);
       p && q && swap_cnt < 2;
       p = clause_iter_next(f, &pit)) {
    mbool v = funcsatValue(f, *p);
    if (v != false && p != q) {
      literal tmp = *q;
      *q = *p, *p = tmp;
      swap_cnt++;
      q = clause_iter_next(f, &qit);
    }
  }
#if 0
  assert(is_watchable(f,c));
#endif
}
@ A clause is watchable if it has at least two literals and the first two
literals are not |false|.

The \BCP\ code when propagating $-l$ (and the other literal is $p$) will break
if: \numberedlist

\li $p$ is false and there is another literal in the clause (aside from $-l$)
that is not false

\endnumberedlist
@<Global def...@>=
static inline bool is_watchable(funcsat *f, struct clause_head *c)
{
  if (c->sz < 2) return false;
  return funcsatValue(f, c->lits[0]) != false &&
    funcsatValue(f, c->lits[1]) != false;
}
@ 

@<Internal decl...@>=
void addWatch(funcsat *f, uintptr_t ix);
static inline void addWatchUnchecked(funcsat *f, uintptr_t ix);
void makeWatchable(funcsat *f, uintptr_t ix);
static inline bool is_watchable(funcsat *f, struct clause_head *c);

@

@<Global def...@>=
static void fs_watches_print(funcsat *f, FILE *out, literal p)
{
  if (!out) out = stderr;
  fprintf(out, "watcher list for %ji:\n", p);
  struct watchlist *wl = &f->watches.wlist[fs_lit2idx(p)];
  struct watchlist_elt *elt, *dump;
  for_watchlist (elt, dump, wl) {
    struct clause_head *c = clause_head_ptr(f, elt->cls);
    if (c) {
      fprintf(out, "[%ji", elt->lit);
      bool in_clause = false;
      struct clause_iter it;
      clause_iter_init(f, elt->cls, &it);
      for (literal *q = clause_iter_next(f, &it); q; q = clause_iter_next(f, &it)) {
        if (*q == elt->lit) {
          in_clause = true; break;
        }
      }
      if (!in_clause) {
        fprintf(out, " oh noes!");
      }
      fprintf(out, "] ");
      fs_clause_head_print(f, out, elt->cls), NEWLINE(out);
    } else {
      fprintf(out, "[EMPTY SLOT]\n");
    }
  }
}


@*2 Reasons. We allow dynamic calculation of reason clauses.

@<External ty...@>=
enum reason_ty
{
  REASON_CLS_TY
};

@ Reasons are represented internally with this |struct|. |reason_info|s are
store in a pool, like other data structures in \funcsat. When the |reason_info|
is free, the |ty| is the index of the next |reason_info| in the
freelist. Otherwise it indicates the type of the |reason_info| (see |enum
reason_ty|).

@<Internal ty...@>=
struct reason_info
{
  uintptr_t ty;                       /* |enum reason_ty|, but can be others too */
  uintptr_t cls;                      /* index of |clause_head| */
};
#include "funcsat/vec_reason_info.h"


@ Getting a reason can now invoke whatever frontend is driving \funcsat, if that
frontend will dynamically calculate a reason clause. Note that this is invoked
\textit{at learning time}, not at inference time. This way other inference
procedures (not based on clauses and \BCP) can infer literals and only if that
inference causes a conflict will a reason clause be required.

@<Global def...@>=
static inline uintptr_t getReason(funcsat *f, literal l)
{
  uintptr_t reason_idx = f->reason->data[fs_lit2var(l)];
  if (reason_idx != NO_CLS) {
    struct reason_info *r = reason_info_ptr(f, reason_idx);
    if (r->ty == REASON_CLS_TY) return r->cls;
    uintptr_t (*get_reason_clause)(funcsat *, literal) = f->reason_hooks.data[r->ty];
    r->ty = REASON_CLS_TY; /* cache reason clause */
    return r->cls = get_reason_clause(f,l);
  } else
    return NO_CLS;
}


@ 
@<Internal decl...@>=
static inline uintptr_t getReason(funcsat *f, literal l);
@ The |reason| and |reason_hooks| need to be initialized.

@<Initialize func...@>=
f->reason = vec_uintptr_init(2);
vec_uintptr_push(f->reason, NO_CLS);
vec_ptr_mk(&f->reason_hooks, 2);

@ Clients should use this to add a hook for generating reasons.
@<Global def...@>=
void funcsatAddReasonHook(funcsat *f, uintptr_t ty,
                          uintptr_t (*hook)(funcsat *f, literal l))
{
  vec_ptr_push(&f->reason_hooks, hook);
}

@ Declaration.
@<External decl...@>=
void funcsatAddReasonHook(funcsat *f, uintptr_t ty,
                          uintptr_t (*hook)(funcsat *f, literal l));

@ When an inference is discovered, we need to set its reason using a fresh
|reason_info|. But of course we shouldn't allocate it. That would slow down
solving. We have a freelist of |reason_info|s and we pick the next free one.

@d reason_info_ptr(f,i) (&((f)->reason_infos->data[i]))

@<Global def...@>=
static inline uintptr_t reason_info_mk(funcsat *f, uintptr_t ix)
{
  assert(f->reason_infos_freelist < f->reason_infos->size);
  uintptr_t ret = f->reason_infos_freelist;
  struct reason_info *r = reason_info_ptr(f, ret);
  f->reason_infos_freelist = r->ty;

  r->ty = REASON_CLS_TY;
  r->cls = ix;
  return ret;
}

@ To reclaim the |reason_info|, we assign it to the head of the freelist and its
next to the current freelist head.

@d reason_info_idx(f,r) ((r) - (f)->reason_infos->data)

@<Global def...@>=
static inline void reason_info_release(funcsat *f, uintptr_t ri)
{
  struct reason_info *r = reason_info_ptr(f,ri);
  r->ty = f->reason_infos_freelist; /* |ty| becomes the next index */
  f->reason_infos_freelist = ri;
}

@ Allocate and initialize the freelist.

@<Initialize func...@>=
f->reason_infos_freelist = UINTMAX_MAX; /* empty */
f->reason_infos = vec_reason_info_init(2);
f->reason_infos->size = 0;


@ Just grow and add the new elements to the freelist.
@<Resize internal...@>=
vec_reason_info_grow_to(f->reason_infos, numVars);
for (uintptr_t i = f->reason_infos->size; i < numVars; i++) {
  reason_info_release(f,i);
}
f->reason_infos->size = numVars;

@ For each new variable, push that it has no reason.
@<Incrementally resize...@>=
vec_uintptr_push(f->reason, NO_CLS);

@ Destroy.
@<Destroy func...@>=
vec_uintptr_destroy(f->reason);
vec_reason_info_destroy(f->reason_infos);
vec_ptr_unmk(&f->reason_hooks);

@*2 Trail. The trail is managed pretty simply with |trailPush| and |trailPop|.

|trailPush| is for adding a new literal to the trail (inferred or decided).

@<Global def...@>=
void trailPush(funcsat *f, literal p, uintptr_t reason_info_idx)
{
  variable v = fs_lit2var(p);
  assert(v <= f->numVars);
#ifndef NDEBUG
  if (f->model.data[v] < f->trail.size) {
    assert(f->trail.data[f->model.data[v]] != p);
  }
#endif
  clausePush(&f->trail, p);
  f->model.data[v]  = f->trail.size-(uintmax_t)1;
  f->phase.data[v] = p;
  f->level.data[v]  = (literal)f->decisionLevel;
  
  if (reason_info_idx != NO_CLS) {
    struct reason_info *r = reason_info_ptr(f, reason_info_idx);
    if (r->ty == REASON_CLS_TY) {
      struct clause_head *h = clause_head_ptr(f, r->cls);
      h->is_reason = true;
    }
    f->reason->data[v] = reason_info_idx;
  }
}

@ |trailPop| is for removing the last pushed literal from the trail. The
|literal| is returned.

In order to pop the trail, you usually want to make sure you don't lose any
facts we have around for the literal you're popping. If that is true, supply a
list to merge with. If facts is |NULL|, any facts are released.

@<Global def...@>=
literal trailPop(funcsat *f, head_tail *facts)
{
  literal p = clausePeek(&f->trail);
  variable v = fs_lit2var(p);
  uintptr_t reason_ix;
  if (facts && !head_tail_is_empty(&f->unit_facts[v]))
    head_tail_append(f, facts, &f->unit_facts[v]);

#if 0
  exonerateClauses(f, v);
#endif

  if (f->decisions.data[v] != 0) {
    /*Added by SEAN!!!*/
    if(f->Backtrack_hook) f->Backtrack_hook((uintptr_t *)f->conf->user, f->decisionLevel);
    f->decisionLevel--;
    f->decisions.data[v] = 0;
  }
  /* no need to clear model */
  /* |f->model.data[v] = 0;| */
  f->level.data[v] = Unassigned;
  reason_ix = f->reason->data[v];
  if (reason_ix != NO_CLS) {
    struct reason_info *r = reason_info_ptr(f, reason_ix);
    if (r->ty == REASON_CLS_TY) {
      struct clause_head *h = clause_head_ptr(f, r->cls);
      h->is_reason = false;
    }
    reason_info_release(f, reason_ix);
    f->reason->data[v] = NO_CLS;
  }

  if (!bh_is_in_heap(f,v)) {
#ifndef NDEBUG
    bh_check(f);
#endif
    bh_insert(f,v);
  }
  clausePop(&f->trail);
  if (f->propq >= f->trail.size)
    f->propq = f->trail.size;
  return p;
}

@

@<Global def...@>=
static inline literal trailPeek(funcsat *f)
{
  literal p = clausePeek(&f->trail);
  return p;
}

@ The prototypes.
@<Internal decl...@>=

void trailPush(funcsat *f, literal p, uintptr_t reason_info_idx);
literal trailPop(funcsat *f, head_tail *facts);
static inline literal trailPeek(funcsat *f);
static inline uintptr_t reason_info_mk(funcsat *f, uintptr_t ix);

@*2 Jailing and exoneration. Clause jailing is simply stashing clauses that we
can easily prove BCP won't ever need to consider (in a particular part of the
search tree).

Right now it's turned off.

Preconditions:
\unorderedlist
\li the clause is alone through the 1st literal
\li |trueLit| is true
\li the clause is (2-watch) linked through the 0th literal
\endunorderedlist

@<Global def...@>=
static void jailClause(funcsat *f, literal trueLit, clause *c)
{
#if 0
  ++f->numJails;
  dopen(f, "jail");
  assert(funcsatValue(f, trueLit) == true);
  const variable trueVar = fs_lit2var(trueLit);
  dmsg(f, "jail", 7, false, "jailed for %ju: ", trueVar);
  clauseUnSpliceWatch((clause **) &watches->data[fs_lit2idx(-c->data[0])], c, 0);
  clause *cell = &f->jail.data[trueVar];
  clauseSplice1(c, &cell);
  dclose(f, "jail");
  assert(!c->is_watched);
#endif
}

static void exonerateClauses(funcsat *f, variable v)
{
  uintptr_t p, c, nx;
  for_head_tail (f, &f->jail[v], p, c, nx) {
    struct clause_head *h = clause_head_ptr(f, c);
    h->link = NO_CLS;
    addWatchUnchecked(f, c);
  }
  head_tail tmp;
  head_tail_clear(&tmp);
  memcpy(&f->jail[v], &tmp, sizeof(tmp));
}

@*1 Decisions. We use a heap to organize our unassigned variables. Then a
decision is made based on their priority. The heap needs to have in it at least
all unassigned variables, though it may have assigned variables in it.

\paragraphit{\it Digression.}
%
I originally wrote a fibonacci heap because I believe it would be the fastest
for this application. (Asymptotically, it is very fast.) But there are a few
problems with it:

\numberedlist

\li It is big. Each node needs something like four pointers, and even though I
allocate all the nodes contiguously, that's big. It's going to affect the cache
behavior.

An array-backed binary heap one pointer for each heap element: the pointer
points to the node associated with its position in the heap. The ``pointers'' to
the child elements are just array index expressions.

\li I've since learned about pairing heaps, which are alleged to be just as fast
(or faster) than fibheaps, and much cheaper to implement. From what I've read,
it looks like you can get a pairing heap with only two pointers per node
(instead of four).

I wonder, though, if people use pairing heaps chiefly because they're afraid of
trying to implement a fibonacci heap.

\endnumberedlist \hfill{\it End Digression.}

Anyway, back to this code. This is how we decide on the next branching variable:

\numberedlist

\li We extract the highest priority variable. It might be assigned or not, so we
have to check for that. If it is assigned, we keep picking variables until
(1)~we can't anymore or (2)~we find an unassigned one.

\li We check which phase the variable should get. (This should be made more
general. Probably we should have a heap of literals, not variables.)

\li Once we have a literal to set to true, we put it on the trail and adjust a
few other data structures.

\endnumberedlist

@<Global def...@>=
literal funcsatMakeDecision(funcsat *f, void *p)
{
  literal l = 0;

  if(f->ExternalHeuristic_hook != NULL)	 {
    l = f->ExternalHeuristic_hook((uintptr_t *)f->conf->user);
    variable v = fs_lit2var(l);
    ++f->numDecisions;
    trailPush(f, l, NO_CLS);
    f->level.data[v] = (intmax_t)++f->decisionLevel;
    f->decisions.data[v] = f->decisionLevel;
    fslog(f, "solve", 2, "branched on %ji\n", l);
  } else {
    while (bh_size(f) > 0) {
      fslog(f, "decide", 5, "extracting\n");
      variable v = bh_pop(f);
#ifndef NDEBUG
      bh_check(f);
#endif
      fslog(f, "decide", 5, "extracted %ju\n", v);
      literal p = -fs_var2lit(v);
      if (funcsatValue(f,p) == unknown) {
        if (f->conf->usePhaseSaving) l = f->phase.data[v];
        else l = p;
        ++f->numDecisions;
        trailPush(f, l, NO_CLS);
        f->level.data[v] = (intmax_t)++f->decisionLevel;
        f->decisions.data[v] = f->decisionLevel;
        fslog(f, "solve", 2, "branched on %ji\n", l);
        break;
      }
    }
    assert(l != 0 || bh_size(f) == 0);
  }

  /*Added by SEAN!!!*/
  if (f->MakeDecision_hook) f->MakeDecision_hook((uintptr_t *)f->conf->user);

  return l;
}

@ 
@<Internal decl...@>=

literal funcsatMakeDecision(funcsat *, void *);


@ Because the variable ordering is dynamic, we can choose strategic points at
which to ``bump'' the priority of a variable. So if at some point during solving
we decide that a given variable is a but more important, we call
|varBumpScore|.
@<Global def...@>=
inline void varBumpScore(funcsat *f, variable v)
{
  double *activity_v = bh_var2act(f,v);
  double origActivity, activity;
  origActivity = activity = *activity_v;
  if ((activity += f->varInc) > 1e100) {     /* rescale */
    for (variable j = 1; j <= f->numVars; j++) {
      double *m = bh_var2act(f,j);
      fslog(f, "decide", 5, "old activity %f, rescaling\n", *m);
      *m *= 1e-100;
    }
    double oldVarInc = f->varInc;
    f->varInc *= 1e-100;
    fslog(f, "decide", 1, "setting varInc from %f to %f\n", oldVarInc, f->varInc);
    activity *= 1e-100;
  }
  if (bh_is_in_heap(f, v)) {
    bh_increase_activity(f, v, activity);
  } else {
    *activity_v = activity;
  }
  fslog(f, "decide", 5, "bumped %ju from %.30f to %.30f\n", v, origActivity, *activity_v);
}

@ Activities are stored as |double|s. Comparing them always returns 1 if the
first activity is {\it more active} than the second (and therefore should be
branched on first).

@<Global def...@>=
static inline int activity_compare(double x, double y)
{
  if (x > y) return 1;
  else if (x < y) return -1;
  else return 0;
}

@
@<Internal decl...@>=
static inline int activity_compare(double x, double y);


@*2 Binary decision variable heap. This basic data structure is what Minisat
uses. This implementation, however, is original. The idea is pretty simple.

The heap (|f->binvar_heap|) is represented as a single array of nodes. Each
possible heap node is stored at some index of an array, {\it whether it's in the
heap or not}. If the node is not in the heap, it is stored after the
|f->binvar_heap_size| index. If the node is in the heap, its children are stored
at known offsets of the parent. We can do this since we know the array must be
of size |f->numVars|. The heap invariant is that a heap node is no smaller than
each of its children.

Each heap node is 16 bytes:
\numberedlist
\li a |double| priority (can we make this a |float|?)
\li a |variable| variable (needed to find out which var to return on a |bh_pop|)
\endnumberedlist

The auxiliary array of {\it positions} (|f->binvar_pos|) maps a |variable| to
its current position in the heap. This way when we adjust a |variable|'s
priority we can easily find the variable in the heap.  The |f->binvar_pos|
invariant is that |f->binvar_pos[v]| is the index of the heap node corresponding
to variable |v|.

@<Internal ty...@>=
struct bh_node
{
  variable var;
  double   act;
};

@ Returns the index into |f->binvar_heap| that gives the current variable's
priority.
@<Global def...@>=
static inline uintmax_t bh_var2pos(funcsat *f, variable v)
{
  return f->binvar_pos[v];
}
static inline bool bh_is_in_heap(funcsat *f, variable v)
{
  assert(bh_var2pos(f,v) > 0);
  return bh_var2pos(f,v) <= f->binvar_heap_size;
}
static inline bool bh_node_is_in_heap(funcsat *f, struct bh_node *n)
{
  assert(n >= f->binvar_heap);
  return (uintmax_t)(n - f->binvar_heap) <= f->binvar_heap_size;
}

@ |bh_increase_activity| needs to adjust the priority---the following function
is used for this.

@<Global def...@>=

static inline double *bh_var2act(funcsat *f, variable v)
{
  return &f->binvar_heap[bh_var2pos(f,v)].act;
}

@ The following functions allow one to traverse heap: to obtain the children and
the parent of a given node. Note, as with other |variable| data structures, the
first node of the heap is at index 1, leaving index 0 unused.

@<Global def...@>=
static inline struct bh_node *bh_top(funcsat *f)
{
  return f->binvar_heap + 1;
}
static inline struct bh_node *bh_bottom(funcsat *f)
{
  return f->binvar_heap + f->binvar_heap_size;
}
static inline bool bh_is_top(funcsat *f, struct bh_node *v)
{
  return bh_top(f) == v;
}
static inline struct bh_node *bh_left(funcsat *f, struct bh_node *v)
{
  return f->binvar_heap + (2 * (v - f->binvar_heap));
}
static inline struct bh_node *bh_right(funcsat *f, struct bh_node *v)
{
  return f->binvar_heap + (2 * (v - f->binvar_heap) + 1);
}
static inline struct bh_node *bh_parent(funcsat *f, struct bh_node *v)
{
  return f->binvar_heap + ((v - f->binvar_heap) / 2);
}
static inline uintmax_t bh_size(funcsat *f)
{
  return f->binvar_heap_size;
}

@ Getting the node location in the heap for an arbitrary |variable| is
accomplished with the help of |f->binvar_pos|, which records exactly that
information.

@<Global def...@>=
static inline struct bh_node *bh_node_get(funcsat *f, variable v)
{
  return f->binvar_heap + f->binvar_pos[v];
}


@ decls
@<Internal decl...@>=

static inline uintmax_t bh_var2pos(funcsat *f, variable v);
static inline bool bh_is_in_heap(funcsat *f, variable v);
static inline bool bh_node_is_in_heap(funcsat *f, struct bh_node *);
static inline double *bh_var2act(funcsat *f, variable v);
static inline struct bh_node *bh_top(funcsat *f);
static inline struct bh_node *bh_bottom(funcsat *f);
static inline bool bh_is_top(funcsat *f, struct bh_node *v);
static inline struct bh_node *bh_left(funcsat *f, struct bh_node *v);
static inline struct bh_node *bh_right(funcsat *f, struct bh_node *v);
static inline struct bh_node *bh_parent(funcsat *f, struct bh_node *v);
static inline uintmax_t bh_size(funcsat *f);
static inline variable bh_pop(funcsat *f);
static inline void bh_insert(funcsat *f, variable v);


@ Insert. After incrementing the heap size, the to insert is placed at the last
free location of the heap. Then it gets bubbled up to the location where the
heap invariant holds again.

Insertion and deletion need often to swap two elements of the heap. |bh_swap|
helps with this.

@<Global def...@>=
static inline void bh_swap(funcsat *f, struct bh_node **x, struct bh_node **y)
{
  struct bh_node tmp = **x, *tmpp = *x;
  **x = **y, **y = tmp;
  *x = *y, *y = tmpp;           /* swap the pointers, too */
  f->binvar_pos[(*x)->var] = *x - f->binvar_heap;
  f->binvar_pos[(*y)->var] = *y - f->binvar_heap;
}

static inline void bh_bubble_up(funcsat *f, struct bh_node *e)
{
  while (!bh_is_top(f, e)) {
    struct bh_node *p = bh_parent(f, e);
    if (activity_compare(p->act, e->act) < 0) {
      bh_swap(f, &p, &e);
    } else
      break;
  }
}

static inline void bh_insert(funcsat *f, variable v)
{
  assert(bh_size(f)+1 <= f->numVars);
  assert(bh_var2pos(f,v) > f->binvar_heap_size);
  struct bh_node *node = &f->binvar_heap[bh_var2pos(f,v)];
  assert(node->var == v);
  f->binvar_heap_size++;
  struct bh_node *last = &f->binvar_heap[f->binvar_heap_size];
  bh_swap(f, &node, &last);
  bh_bubble_up(f, node);
  assert(f->binvar_heap[bh_var2pos(f,v)].var == v); /* invariant */
}


@ Extract Max. We take the bottom element of the heap and replace the top with
it. Then we bubble it down until the heap property holds again.

@<Global def...@>=
static inline void bh_bubble_down(funcsat *f, struct bh_node *e)
{
  struct bh_node *l, *r;
  goto bh_bd_begin;
  while (bh_node_is_in_heap(f,l)) {
    if (bh_node_is_in_heap(f,r)) {
      if (activity_compare(l->act, r->act) < 0)
        l = r; /* put max child in |l| */
    }
    if (activity_compare(e->act, l->act) < 0) {
      bh_swap(f, &e, &l);
    } else
      break;
bh_bd_begin:
    l = bh_left(f,e), r = bh_right(f,e);
  }
}

static inline variable bh_pop(funcsat *f)
{
  assert(f->binvar_heap_size > 0);
  struct bh_node *top = bh_top(f);
  struct bh_node *bot = bh_bottom(f);
  bh_swap(f, &top, &bot);
  f->binvar_heap_size--;
  bh_bubble_down(f, bh_top(f));
  return top->var;
}

@ decls
@<Global def...@>=
static inline void bh_increase_activity(funcsat *f, variable v, double act_new)
{
  double *act_curr = bh_var2act(f,v);
  struct bh_node *n = bh_node_get(f, v);
  assert(n->var == v);
  assert(*act_curr <= act_new);
  *act_curr = act_new;
  bh_bubble_up(f, n);
}

@ decl
@<Internal decl...@>=
static inline void bh_increase_activity(funcsat *f, uintmax_t node_pos, double new_act);

@ we should check the heap property!
@<Global def...@>=
static void bh_check_node(funcsat *f, struct bh_node *x)
{
  struct bh_node *l = bh_left(f,x), *r = bh_right(f,x);
  if (bh_node_is_in_heap(f,l)) {
    assert(activity_compare(l->act, x->act) <= 0);
    bh_check_node(f,l);
  }
  if (bh_node_is_in_heap(f,r)) {
    assert(activity_compare(r->act, x->act) <= 0);
    bh_check_node(f,r);
  }
}

static void bh_check(funcsat *f)
{
  struct bh_node *root = bh_top(f);
  if (bh_node_is_in_heap(f, root)) {
    bh_check_node(f, root);
  }
  for (uintmax_t i = 1; i < f->numVars; i++) {
    assert(bh_node_get(f, i)->var == i);
  }
}

@
@<Internal decl...@>=
static void bh_check(funcsat *f);

@ Initializing is easy.
@<Initialize funcsat...@>=
FS_CALLOC(f->binvar_heap, 2, sizeof(*f->binvar_heap));
FS_CALLOC(f->binvar_pos, 2, sizeof(*f->binvar_pos));
f->binvar_heap_size = 0;


@ Destroying.
@<Destroy func...@>=
free(f->binvar_heap);
free(f->binvar_pos);
f->binvar_heap_size = 0;


@ Allocating is easy.
@<Resize internal data...@>=
FS_REALLOC(f->binvar_heap, numVars+1, sizeof(*f->binvar_heap));
FS_REALLOC(f->binvar_pos, numVars+1, sizeof(*f->binvar_pos));


@ Initializing each activity is also easy.
@<Incrementally resize internal data...@>=
f->binvar_heap[v].var = v;
f->binvar_heap[v].act = f->conf->getInitialActivity(&v);
f->binvar_pos[v] = v;
bh_insert(f, v);
assert(f->binvar_pos[v] != 0);


@ Print the heap.
@<Global def...@>=
static void bh_padding(funcsat *f, const char *s, int x)
{
  while (x-- > 0) {
    fprintf(fs_dbgout(f), "%s", s);
  }
}

static bool bh_print_levels(funcsat *f, FILE *dotfile, struct bh_node *r, int level)
{
  assert(r);
  if (bh_node_is_in_heap(f, r)) {
    bool lf, ri;
    lf = bh_print_levels(f, dotfile, bh_left(f, r), level+1);
    ri = bh_print_levels(f, dotfile, bh_right(f, r), level+1);
    if (lf) fprintf(dotfile, "%ju -> %ju [label=\"L\"];\n", r->var, bh_left(f,r)->var);
    if (ri) fprintf(dotfile, "%ju -> %ju [label=\"R\"];\n", r->var, bh_right(f,r)->var);
    fprintf(dotfile, "%ju [label=\"%ju%s, %.1f\"];\n", r->var, r->var,
            (funcsatValue(f, r->var) == true ? "T" :
             (funcsatValue(f, r->var) == false ? "F" : "?")), r->act);
    return true;
  } else {
    return false;
  }
}

static void bh_print(funcsat *f, const char *path, struct bh_node *r)
{
  if (!r) r = bh_top(f);
  FILE *dotfile;
  if (NULL == (dotfile = fopen(path, "w"))) perror("fopen"), exit(1);
  fprintf(dotfile, "digraph G {\n");
  bh_print_levels(f, dotfile, r, 0);
  fprintf(dotfile, "}\n");
  if (0 != fclose(dotfile)) perror("fclose");
  fprintf(fs_dbgout(f), "\n");
}

@ 
@<Internal decl...@>=
static void bh_print(funcsat *f, const char *path, struct bh_node *r);

@*2 Backtracking.
@<Global def...@>=
void backtrack(funcsat *f, variable newLevel, head_tail *facts, bool isRestart)
{
  head_tail restart_facts;
  if (isRestart) {
    assert(newLevel == 0UL), assert(!facts);
    facts = &restart_facts;
    head_tail_clear(&restart_facts);
  }
  while (f->decisionLevel != newLevel)
    trailPop(f, facts);
  if (isRestart) {
    literal top;

    /*Added by SEAN!!!*/
    if(f->Backtrack_hook) f->Backtrack_hook((uintptr_t *)f->conf->user, 0);
    if(f->MakeDecision_hook) f->MakeDecision_hook((uintptr_t *)f->conf->user);
    f->propq = 0;
    restore_facts(f, facts);
    if (f->trail.size > 0) {
      top = trailPeek(f);
      head_tail_append(f, &f->unit_facts[fs_lit2var(top)], facts);
    }
  }
}

static void restore_facts(funcsat *f, head_tail *facts)
{
  uintptr_t prev, curr, next;
  for_head_tail (f, facts, prev, curr, next) {
    /* if the clause was subsumed, it could be here but not learned */
    struct clause_head *h = clause_head_ptr(f, curr);
    assert(h->sz >= 1);

    if (h->sz == 1) {
      literal p = h->lits[0];
      mbool val = funcsatValue(f,p);
      assert(val != false);
      if (val == unknown) {
        trailPush(f, p, reason_info_mk(f, curr));
        fslog(f, "bcp", 5, " => %ji\n", p);
      }
    } else {
      head_tail_iter_rm(f, facts, prev, curr, next);
      addWatchUnchecked(f, curr);
#ifndef NDEBUG
      watches_check(f);
#endif
    }
  }
}

@
@<Internal decl...@>=
static void restore_facts(funcsat *f, head_tail *facts);


@*1 Clause learning.\definexref{pg-clause-learn}{\folio}{page} \funcsat\ learns
unique implication point (UIP) clauses. It can learn any number of them,
starting with the first. The top-level procedure is |analyze_conflict|, which
assumes:

\numberedlist

\li the solver is in conflict (|f->conflict_clause| is set)

\li the literals in |f->conflict_clause| are all |false| (under |funcsatValue|)

\endnumberedlist

Initially the conflict clause is unset.
@<Initialize fun...@>=
f->conflict_clause = NO_CLS;

@ This function is just a wrapper around the real work. It makes sure that we
approximate how much of the space we've explored and returns whether we have
proven the problem unsat to the caller. The real work is done in |find_uips|.
@<Global def...@>=
bool analyze_conflict(funcsat *f)
{
  struct clause_iter it;
  variable i, c = 0; /* c -- count current-decision-level literals in the
                      learned clause */
  literal *p;
  head_tail facts;
  literal uipLit = 0;

  ++f->numConflicts;

  assert(f->uipClause.size == 0);
  clause_iter_init(f, f->conflict_clause, &it);
  for (literal *p = clause_iter_next(f, &it); p; p = clause_iter_next(f, &it)) {
    clausePush(&f->uipClause, *p);
  }
#if !defined(NDEBUG) && defined(PRINT_IMPL_GRAPH)
  print_dot_impl_graph(f, &f->uipClause);
#endif

  head_tail_clear(&facts);
  litpos_clear(&f->litpos_uip);
  litpos_init_from_uip_clause(f);
  for_clause (p, &f->uipClause)
    if ((uintmax_t)levelOf(f, fs_lit2var(*p)) == f->decisionLevel)
      c++;
  int64_t entrydl = (int64_t)f->decisionLevel;
#ifndef NDEBUG
  watches_check(f);
#endif
  bool isUnsat = find_uips(f, c, &facts, &uipLit);
  if (isUnsat) {
    fslog(f, "solve", 1, "find_uips returned isUnsat\n");
    f->conflict_clause = NO_CLS;
    return false;
  }

  f->conf->decayAfterConflict(f);
  f->conflict_clause = NO_CLS;
  f->uipClause.size = 0;
#ifndef NDEBUG
  watches_check(f);
#endif
  if (!propagateFacts(f, &facts, uipLit)) {
    return analyze_conflict(f);
  } else {
    return true;
  }
}

@ |find_uips| does the heavy lifting of forming the \UIP\ clauses. Returns
|true| only if the \SAT\ instance is \UNSAT.

If we don't learn all the \UIP s that we could then there may be literals after
the decision literal on the trail. So we need to pop them; finally we pop the
decision literal.

@<Global def...@>=
bool find_uips(funcsat *f, uintmax_t c, head_tail *facts, literal *lit_uip)
{
  uint32_t num_uips_learned = 0, num_uips_to_learn = f->conf->numUipsToLearn;
  uint32_t num_resolutions = 0;
  literal p;
  fs_ifdbg (f, "find_uips", 1) {
    fslog(f, "find_uips", 1, "conflict@@%ju (#%" PRIu64 ") with ",
          f->decisionLevel, f->numConflicts);
    fs_clause_print(f, fs_dbgout(f), &f->uipClause);
    fprintf(fs_dbgout(f), "\n");
  }
  dopen(f, "find_uips");
  do {
    @<Find the next \UIP\ clause @>@;
  } while (num_uips_learned < num_uips_to_learn);
  *lit_uip = p;
  while (!isDecision(f, fs_lit2var(trailPeek(f))))
    trailPop(f, facts);
  p = trailPop(f, facts); /* pop decision literal */
  fslog(f, "find_uips", 5, "learned %" PRIu32 " UIPs\n", num_uips_learned);
  dclose(f, "find_uips");
  fslog(f, "find_uips", 5, "done\n");
  return false;
}

@ Initially, there might be some junk on the trail that hasn't been used for
propagation yet. We need to undo trail assignments until the top of the trail is
in the clause. This allows us to pick the most recently inferred literal and
resolve it out of the conflict clause.

After a single resolution step, one a few things has happened:

\orderedlist

\li In the common case we will have added some new literals from the current
level, keeping |c>1|. We will want to keep resolving. However, it often happens
that there are literals on the trail that don't occur in |f->uipClause| (the
clause we're building). We want to ignore those so we need to pop literals from
the trail until we find one that's in the conflict clause.

\li In the \UIP\ case, we find a \UIP: |c==1| and we need to add the new \UIP\ clause
to the clause database. In this case it's also desirable to pop literals from
the trail until we find one in the conflict clause.

\li It can happen that we resolve all current-level literals out and, although
the clause isn't empty, |c==0|. This means a decision level really wasn't
responsible for the current conflict and we need to backtrack over it until
we find something in the conflict clause (I think?).

\endorderedlist

During learning, there are a few conditions which mean we proved the problem
\UNSAT: (\osf{1}) if we backtrack to decision level 0 or (\osf{2}) if the \UIP\
clause becomes empty. We check these in the loop conditions.

Finally, if our \UIP\ literal is a decision variable, we need to stop learning
\UIP s -- so we check for that at the very end of this section.
@<Find the next \UIP...@>=
while (!litpos_contains(&f->litpos_uip, p = trailPeek(f))) trailPop(f, facts);
do {
  uintptr_t ix_reason;
  uintmax_t num_new_lits; /* new current-level lits */
  @<Check \UIP\ learning invariants if |NDEBUG| is not present@>@;
  assert(litpos_contains(&f->litpos_uip, p));
  ix_reason = getReason(f,p);
  num_new_lits = pop_and_resolve(f, ix_reason, facts);
  num_resolutions++;
  c = c - 1 + num_new_lits;
  while (f->trail.size > 0 && !litpos_contains(&f->litpos_uip, p = trailPeek(f)))
    trailPop(f, facts);
  if (c == 0) { /* reset count cuz we backtracked */
    literal *q;
    for_clause (q, &f->uipClause)
      if (levelOf(f, fs_lit2var(*q)) == (literal) f->decisionLevel) c++;
    fslog(f, "find_uips", 6,
          "c=0, backtracked to level %" PRIu64 ", c = %" PRIu32 ", p = %ji\n",
          f->decisionLevel, c, p);
  }
} while (c != 1 && f->uipClause.size > 0 && f->decisionLevel > 0);
num_uips_learned++;
@<Allocate and add the new \UIP...@>@;
if (isDecision(f, fs_lit2var(p)))
  num_uips_to_learn = num_uips_learned;


@ Check loop invariants to at least \textit{try} to be careful.

\orderedlist
\li Top of the trail must be in the clause.
\li Count |c| must be the number of current-decision-level literals.
\endorderedlist

These invariants should guarantee that there is at least one more UIP literal to
learn.
@<Check \UIP\ learning invariants...@>=
#ifndef NDEBUG
{
  literal *q;
  uint32_t inner_cnt = 0;
  assert(litpos_contains(&f->litpos_uip, trailPeek(f)));
  for_clause (q, &f->uipClause) {
    if (levelOf(f, fs_lit2var(*q)) == (literal) f->decisionLevel) inner_cnt++;
  }
  assert(inner_cnt == c);
}
#endif

@ At each step during learning a clause we perform a clause resolution. We will
always resolve away the variable at the top of the trail. As a result, the
clause may shrink or expand in size.

@<Global def...@>=
static inline uintmax_t pop_and_resolve(funcsat *f, uintptr_t ix_reason, head_tail *facts) {
  literal p;
  fs_ifdbg (f, "find_uips", 6) {
    fslog(f, "find_uips", 6, "resolving ");
    fs_clause_print(f, fs_dbgout(f), &f->uipClause);
    fprintf(fs_dbgout(f), " with ");
    fs_clause_head_print(f, fs_dbgout(f), ix_reason);
    fprintf(fs_dbgout(f), "\n");
  }
  p = trailPop(f, facts);
  f->conf->bumpReason(f, ix_reason);
  @<Resolve |f->uipClause| with reason...@>@;
}


@ Here the actual resolution step is performed. This code performs a resolution
on the literal |p| using the position list, which is stored in
|funcsat->litPos|. The code copies over all literals not currently in
|f->uipClause|; then, it deletes |-p| from |f->uipClause|.

Preconditions:
\unorderedlist
\li |ix_reason| is the clause index for |p|'s reason
\li |p| is in |ix_reason|
\li |-p| is in |f->uipClause|
\endunorderedlist

Returns the number of current-decision-level literals found in the |ix_reason|
clause.

@<Resolve |f->uipClause| with reason for |p| and count literals@>=
variable i, c = 0, pvar = fs_lit2var(p);
uintptr_t pos_p;
struct clause_iter it;

++f->numResolutions;

clause_iter_init(f, ix_reason, &it);
for (literal *q = clause_iter_next(f, &it); q; q = clause_iter_next(f, &it)) {
  literal l = *q;
  if (!litpos_contains(&f->litpos_uip, l)) {
    literal *new_lit;
    clausePush(&f->uipClause, l);
    new_lit = f->uipClause.data + f->uipClause.size - 1;
    assert(*new_lit == l);
    litpos_set(&f->litpos_uip, new_lit);
    litpos_check(&f->litpos_uip);
    if (levelOf(f, fs_lit2var(l)) == (literal)f->decisionLevel) c++;
  }
}
@<Delete |-p| from |f->uipClause|...@>@;
return c;

@ Swap |-p| and the last element of |f->uipClause| and decrement the clause
size. Simple.
@<Delete |-p| from |f->uipClause| after resolution@>=
literal *data = f->uipClause.data;
pos_p = litpos_pos(&f->litpos_uip, p);
swap(literal, data[pos_p], data[f->uipClause.size-1]);
litpos_set(&f->litpos_uip, data + pos_p);
litpos_unset(&f->litpos_uip, data[f->uipClause.size-1]);
f->uipClause.size--;
litpos_check(&f->litpos_uip);

@
@<Internal decl...@>=
static inline uintmax_t pop_and_resolve(funcsat *f, uintptr_t ix_reason,
                                 head_tail *facts);

@ You might think that allocating a new internal clause from the new \UIP\
clause would be straightforward.

But you would be wrong.

The first bit of (slight) complexity is that since we might generate more than
one learned clause per conflict, we need a list to store them. We use the unit
fact list at the current UIP literal |p| for our storage.

The second bit of complexity is that the order if literals in the learned clause
matters. (See the section |@<Order unit fact...@>| for details.)

@<Allocate and add the new \UIP\ clause to the unit facts for |fs_lit2var(p)|@>=
fs_ifdbg (f, "find_uips", 6) {
  fslog(f, "find_uips", 6, "found raw UIP: ");
  fs_clause_print(f, fs_dbgout(f), &f->uipClause);
  fprintf(fs_dbgout(f), "\n");
}

@<Check for \UNSAT\ after...@>@;
@<Order unit fact literals...@>@;

uintptr_t ix_uip = clause_head_alloc_from_clause(f, &f->uipClause);
struct clause_head *h_uip = clause_head_ptr(f, ix_uip);
f->conf->bumpLearned(f, ix_uip);
fs_ifdbg (f, "find_uips", 5) {
  fslog(f, "find_uips", 5, "found min UIP: ");
  fs_clause_head_print(f, fs_dbgout(f), ix_uip);
  fprintf(fs_dbgout(f), "\n");
}
h_uip->is_learned = true;
h_uip->where = CLAUSE_UNIT;
head_tail_add(f, &f->unit_facts[fs_lit2var(p)], ix_uip);
vec_uintptr_push(f->learned_clauses, ix_uip);
++f->numLearnedClauses;


@ After we have learned a \UIP\ we may have discovered that the \SAT\ instance
is unsatisfiable. This can happen when we backtrack all the way to decision
level 0 or if we resolve the empty clause directly.

@<Check for \UNSAT\ after learning@>=
if (f->decisionLevel == 0 || f->uipClause.size == 0) return true;

@ While munging the unit facts, we need to ensure that the invariant on
|f->unitFacts| for |propagateFacts| holds for this clause: the \UIP\ literal
needs to be the first literal in the clause, and the second literal is the one
that was most recently inferred.

We minimize the learned clause here, too.

@<Order unit fact literals and minimize the learned clause@>=
literal watch2 = 0, watch2_level = -1;
uintmax_t pPos = litpos_pos(&f->litpos_uip, p);
swap(literal, f->uipClause.data[0], f->uipClause.data[pPos]);
litpos_set(&f->litpos_uip, &f->uipClause.data[0]);
litpos_set(&f->litpos_uip, &f->uipClause.data[pPos]);
if (f->conf->minimizeLearnedClauses) minimizeUip(f, (&f->uipClause));
for (variable i = 1; i < (&f->uipClause)->size; i++) { /* find max level literal */
  literal lev = levelOf(f, fs_lit2var((&f->uipClause)->data[i]));
  if (watch2_level < lev) {
    watch2_level = lev;
    watch2 = (literal) i;
  }
  if (lev == (literal) f->decisionLevel) break;
}
if (watch2_level != -1) {
  swap(literal, f->uipClause.data[1], f->uipClause.data[watch2]);
  litpos_set(&f->litpos_uip, &f->uipClause.data[1]);
  litpos_set(&f->litpos_uip, &f->uipClause.data[watch2]);
}


@ During learning we may discover learned (intermediate or \UIP) clauses that
subsume existing clauses. If so, we should take that information and strengthen
our clause database.

@<Deal with subsumed clauses@>=
#if 0
  if (f->conf->useSelfSubsumingResolution) {
    for_vec_ptr99 (clause **, it, &f->subsumed) {
      bool subsumedByUip = (bool)*it;
      ++it;

      fs_ifdbg (f, "subsumption", 1) {
        fprintf(dbgout(f), "removing clause due to subsumption: ");
        clause_head_print_dimacs(dbgout(f), *it);
        fprintf(dbgout(f), "\n");
      }

      if ((*it)->isLearnt) {
        clause *removedClause = funcsatRemoveClause(f, *it);
        assert(removedClause);
        if (subsumedByUip) clauseFree(removedClause);
        else {
          removedClause->data[0] = removedClause->data[--removedClause->size];
          vec_ptr_push(&f->learnedClauses, removedClause);
          fs_ifdbg (f, "subsumption", 1) {
            fprintf(dbgout(f), "new adjusted clause: ");
            clause_head_print_dimacs(dbgout(f), removedClause);
            fprintf(dbgout(f), "\n");
          }
          if (removedClause->is_watched) {
            addWatch(f, removedClause);
          }
        }
      }
    }
    f->subsumed.size = 0;
  }
#endif

@ Prototypes.
@<Internal decl...@>=
static inline bool isAssumption(funcsat *f, variable v);

@ Learned clauses often contain redundant literals. Use Van Gelder's algorithm
to remove them.

@
@<Initialize func...@>=
vec_uintmax_mk(&f->seen, 2);
vec_uintmax_push(&f->seen, false);
vec_intmax_mk(&f->analyseToClear, 2);
vec_intmax_push(&f->analyseToClear, 0);
vec_uintmax_mk(&f->analyseStack, 2);
vec_uintmax_push(&f->analyseStack, 0);
vec_uintmax_mk(&f->allLevels, 2);
vec_uintmax_push(&f->allLevels, false);

@
@<Incrementally resize internal...@>=
vec_uintmax_push(&f->seen, false);
vec_uintmax_push(&f->allLevels, false);

@
@<Resize internal...@>=
vec_intmax_grow_to(&f->analyseToClear, numVars);
vec_uintmax_grow_to(&f->analyseStack, numVars);


@
@<Destroy func...@>=
vec_uintmax_unmk(&f->allLevels);
vec_uintmax_unmk(&f->analyseStack);
vec_intmax_unmk(&f->analyseToClear);
vec_uintmax_unmk(&f->seen);


@
@<Global def...@>=
static void cleanSeen(funcsat *f, variable top)
{
  while (f->analyseToClear.size > top) {
    variable v = fs_lit2var(vec_intmax_pop(&f->analyseToClear));
    f->seen.data[v] = false;
  }
}

static inline bool isAssumption(funcsat *f, variable v)
{
  literal *it;
  for_clause (it, &f->assumptions) {
    if (fs_lit2var(*it) == v) {
      return true;
    }
  }
  return false;
}

/**
 * Performs a depth-first search of the conflict graph beginning at q0.
 */
static bool litRedundant(funcsat *f, literal q0)
{
  if (levelOf(f, fs_lit2var(q0)) == 0) return false;
  if (isDecision(f, fs_lit2var(q0))) return false;
  assert(!isAssumption(f, fs_lit2var(q0)));

  /*fprintf(f->conf->debugStream, "literal %jd is not trivally redundant\n", q0);*/

  vec_uintmax_clear(&f->analyseStack);
  vec_uintmax_push(&f->analyseStack, fs_lit2var(q0));
  variable top = f->analyseToClear.size;

  /*fprintf(f->conf->debugStream, "pushing literal %jd to stack\n", q0);*/

  while (f->analyseStack.size > 0) {
    variable p = vec_uintmax_pop(&f->analyseStack);
    /*fprintf(f->conf->debugStream, "popping variable %ju from stack to get reason clause\n", p);*/
    uintptr_t ix = getReason(f, (literal)p);
    /*fs_clause_head_print(f, fs_dbgout(f), ix);*/
    struct clause_iter it;
    literal *q;
    clause_iter_init(f, ix, &it);
/*    q = clause_iter_next(f, &it);*/
/*    if (q) {*/
      /* begins at 1 because |fs_lit2var(c->data[0])==p| */
      for (q = clause_iter_next(f, &it); q; q = clause_iter_next(f, &it)) {
        /*fprintf(f->conf->debugStream, "considering literal %jd from clause\n", *q);*/
/*assert(q0 != *q);*/
if(*q == q0) continue;
        variable v = fs_lit2var(*q);
        literal lev = levelOf(f, v);
        if (!f->seen.data[v] && lev > 0) {
          if (!isDecision(f, fs_lit2var(*q)) && f->allLevels.data[lev]) {
            vec_uintmax_push(&f->analyseStack, v);
            vec_intmax_push(&f->analyseToClear, *q);
            f->seen.data[v] = true;
          } else {
            cleanSeen(f, top);
            return false;
          }
        }
      }
    /*}*/
  }
  return true;
}

@ |minimizeUip| assumes that the \UIP\ literal is at the first position in the
clause.
@<Global def...@>=
void minimizeUip(funcsat *f, clause *learned)
{
  /** todo use integer to denote the levels in the set, then just increment */
  variable i, j;
  for (i = 0; i < learned->size; i++) {
    literal l = levelOf(f, fs_lit2var(learned->data[i]));
    assert(l != Unassigned);
    f->allLevels.data[l] = true;
  }

  vec_intmax_clear(&f->analyseToClear);
  literal *it;
  forClause (it, learned) {
    vec_intmax_push(&f->analyseToClear, *it);
  }

  /* |memset(f->seen.data, 0, f->seen.capacity * sizeof(*f->seen.data));| */
  for (i = 0; i < learned->size; i++) {
    f->seen.data[fs_lit2var(learned->data[i])] = true;
  }

  /* uip literal is assumed to be in |learned->data[0]|, so skip it by starting at
   * |i=1| */
  fslog(f, "minimizeUip", 6, "searching learned clause for redundant literals\n");
  /*fs_clause_print(f, fs_dbgout(f), learned);*/

  for (i = 1, j = 1; i < learned->size; i++) {
    literal p = learned->data[i];
    /*fprintf(f->conf->debugStream, "considering literal %jd\n", p);*/
    if (!litRedundant(f, p)) {
      learned->data[j] = p;
      litpos_set(&f->litpos_uip, learned->data+j);
      j++;
    } else {
      assert(!isAssumption(f, fs_lit2var(p)));
      litpos_unset(&f->litpos_uip, p);
      fslog(f, "minimizeUip", 5, "deleted %ji\n", p);
      ++f->numLiteralsDeleted;
    }
  }
  learned->size -= i - j;

  /* specialised |cleanSeen(f, 0)| to include clearing of |allLevels| */
  while (f->analyseToClear.size > 0) {
    literal l = vec_intmax_pop(&f->analyseToClear);
    variable v = fs_lit2var(l);
    f->seen.data[v] = false;
    f->allLevels.data[levelOf(f,v)] = false;
  }
}

@
@<Find the most recent...@>=
while (!litpos_contains(&f->litpos_uip, p = trailPeek(f))) {
  trailPop(f, facts);
}

@
@<Internal decl...@>=
static void print_dot_impl_graph(funcsat *f, clause *cc);

@ It is convenient to be able to dump the implication graph to a dot file so we can
render it in graphviz.

@<Global def...@>=
static char *dot_lit2label(literal p)
{
  static char buf[64];
  sprintf(buf, "lit%ji", fs_lit2var(p));
  return buf;
}

static void print_dot_impl_graph_rec(funcsat *f, FILE *dotfile, struct vec_bool *seen, literal p)
{
  if (seen->data[fs_lit2var(p)] == false) {
    fprintf(dotfile, "%s ", dot_lit2label(p));
    fprintf(dotfile, "[label=\"%ji @@ %ju%s\"%s];\n",
      (funcsatValue(f,p) == false ? -p : p),
      levelOf(f, fs_lit2var(p)),
      (funcsatValue(f,p)==unknown ? "*" : ""),
      (levelOf(f, fs_lit2var(p))==(literal)f->decisionLevel ? ",color=green" : ""));
    seen->data[fs_lit2var(p)] = true;
    bool isdec = isDecision(f, fs_lit2var(p));
    if (!isdec) {
      uintptr_t ix_r = getReason(f,p);
      struct clause_iter it;
      clause_iter_init(f, ix_r, &it);
      fprintf(dotfile, "/* reason for %ji: ", p);
      fs_clause_head_print(f, dotfile, ix_r);
      fprintf(dotfile, "*/\n");
      for (literal *q = clause_iter_next(f, &it); q; q = clause_iter_next(f, &it)) {
        if (*q != -p) {
          fprintf(dotfile, "%s", dot_lit2label(*q));
          fprintf(dotfile, " -> ");
          fprintf(dotfile, "%s;\n", dot_lit2label(p));
        }
      }
      clause_iter_init(f, ix_r, &it);
      for (literal *q = clause_iter_next(f, &it); q; q = clause_iter_next(f, &it)) {
        print_dot_impl_graph_rec(f, dotfile, seen, *q);
      }
    } else {
      fprintf(dotfile, "/* no reason for %ji */\n", p);
    }
  }
}

static void print_dot_impl_graph(funcsat *f, clause *cc)
{
  literal *p, *q;
  struct vec_bool *seen = vec_bool_init(f->numVars+1);
  seen->size = f->numVars+1;
  char buf[64];
  snprintf(buf, 64, "conflict%" PRIu64 ".dot", f->numConflicts), buf[63] = 0;
  FILE *dotfile = fopen(buf, "w");

  fprintf(dotfile, "digraph G {\n");
  fprintf(dotfile, "/* conflict clause: ");
  fs_clause_print(f, dotfile, cc);
  fprintf(dotfile, "*/\n");
  buf[strlen(buf)-strlen(".dot")]=0;
  fprintf(dotfile, "%s [shape=note];\n", buf);
  fprintf(dotfile, "cclause [shape=note,label=\""); /* conflict clause */
  for_clause (p, cc) {
    fprintf(dotfile, "%ji%s ",*p,(funcsatValue(f,*p)==unknown ? "*" : ""));
  }
  fprintf(dotfile, "\"];\n");
  for_clause (p, cc)
    print_dot_impl_graph_rec(f, dotfile, seen, *p);
  fprintf(dotfile, "lambda [label=L];\n");
  for_clause (p, cc) {
    fprintf(dotfile, "%s", dot_lit2label(*p));
    fprintf(dotfile, " -> ");
    fprintf(dotfile, "lambda [color=\"red\"];\n");
  }
  fprintf(dotfile, "\n}");
  vec_bool_destroy(seen);
  fclose(dotfile);
}

@
@<Internal decl...@>=
#include "funcsat/vec_bool.h"


@*2 Unit facts. At the end of learning we have a list of new clauses. We need to
propagate them, and more importantly shunt them into {\it unit fact lists}.

@<Internal decl...@>=
bool find_uips(funcsat *f, uintmax_t c, head_tail *facts, literal *uipLit);
bool propagateFacts(funcsat *f, head_tail *facts, literal uipLit);

static uintmax_t resetLevelCount(funcsat *f, uintmax_t c, head_tail *facts);

static void checkSubsumption(
  funcsat *f,
  literal p, clause *learn, clause *reason,
  bool learnIsUip);

@ The following function propagates the given unit facts.  Each clause is
either:
%
\numberedlist
\li unit
\li or has at least two unassigned literals.
\endnumberedlist In the first case, we immediately make an assignment, but this function will
ignore that assignment for the purposes of detecting a conflict.  In the latter
case, we add the clause to the watched literals lists.  The uips not added to
the watched literal lists are attached as unit facts to the variable dec.

It is possible that propagateFacts may discover a conflict.  If so, it returns
|false|.


@ The following code is wrapped around the crucial node, |@<Propagate single
unit...@>|.

@<Global def...@>=
bool propagateFacts(funcsat *f, head_tail *facts, literal uipLit)
{
  bool isConsistent = true;
  variable uipVar = fs_lit2var(uipLit);
  uintmax_t cnt = 0;
  dopen(f, "bcp");
  /* Assign each unit fact */
  uintptr_t ix_prev, ix_curr, ix_next;
  for_head_tail (f, facts, ix_prev, ix_curr, ix_next) {
    ++f->numUnitFactProps, ++cnt;
    @<Propagate single unit fact@>@;
    if (!isConsistent) break;
  }

  dclose(f, "bcp");
  fslog(f, "bcp", 1, "propagated %ju facts\n", cnt);
  if (f->conflict_clause != NO_CLS) {
    if (funcsatValue(f, uipLit) == unknown) {
      if (f->decisionLevel > 0) {
        uipVar = fs_lit2var(f->trail.data[f->trail.size-1]);
      } else goto ReallyDone;
    }
  }
#if 0
  assert(!head || funcsatValue(f, fs_var2lit(uipVar)) != unknown);
  assert(clauseIsAlone(&f->unitFacts.data[uipVar], 0));
#endif
  /* if var@@0, then all learns are SAT@@0 */
  if (levelOf(f, uipVar) != 0) {
    head_tail_append(f, &f->unit_facts[uipVar], facts);
  }

ReallyDone:
  return isConsistent;
}

@ Using the list of |facts|, |curr| and |prev|, we can propagate a single unit
fact. If we detect an inconsistency we set |isConsistent| to |false|.

@<Propagate single unit...@>=
fs_ifdbg (f, "bcp", 5) {
  fslog(f, "bcp", 5, ""); /* logging label */
  fs_clause_head_print(f, fs_dbgout(f), ix_curr);
  fprintf(fs_dbgout(f), "\n");
}

struct clause_head *h_curr = clause_head_ptr(f, ix_curr);
if (h_curr->sz == 0) goto Conflict;
else if (h_curr->sz == 1) {
  literal p = h_curr->lits[0];
  mbool val = funcsatValue(f,p);
  if (val == false) goto Conflict;
  else if (val == unknown) {
    trailPush(f, p, reason_info_mk(f, ix_curr));
    fslog(f, "bcp", 1, " => %ji\n", p);
  }
  continue;
}

literal p = h_curr->lits[0], q = h_curr->lits[1];
assert(p != 0 && q != 0 && "unset literals");
assert(p != q && "did not find distinct literals");
mbool vp = tentativeValue(f,p), vq = tentativeValue(f,q);

/* if |(vp==true && vq == true)| (if jailing, put in jailed list for data[1],
 * else put in watcher list) */
if (vp == true && vq == true) {
  head_tail_iter_rm(f, facts, ix_prev, ix_curr, ix_next);
  addWatchUnchecked(f, ix_curr);
  fslog(f, "bcp", 5, " => watched\n");
} else if (vp == true || vq == true) {        /* clause is SAT */
  ;                           /* leave in unit facts */
  fslog(f, "bcp", 5, " => unmolested\n");
} else if (vp == unknown && vq == unknown) {
  head_tail_iter_rm(f, facts, ix_prev, ix_curr, ix_next);
  addWatchUnchecked(f, ix_curr);
  fslog(f, "bcp", 5, " => watched\n");
} else if (vp == unknown) {
  if (funcsatValue(f,p) == false) goto Conflict;
  assert(h_curr->lits[0] == p);
  trailPush(f, p, reason_info_mk(f, ix_curr)); /* clause is unit, implying p */
  f->conf->bumpUnitClause(f, ix_curr);
  fslog(f, "bcp", 1, " => %ji\n", p);
} else if (vq == unknown) {
  if (funcsatValue(f,q) == false) goto Conflict;
  assert(h_curr->lits[1] == q);
  h_curr->lits[0] = q, h_curr->lits[1] = p;
  trailPush(f, q, reason_info_mk(f, ix_curr)); /* clause is unit, implying q */
  fslog(f, "bcp", 1, " => %ji\n", q);
  f->conf->bumpUnitClause(f, ix_curr);
} else {
Conflict: /* clause is UNSAT */
  isConsistent = false;
  f->conflict_clause = ix_curr;
  fslog(f, "bcp", 1, " => X\n");
}

@ A slightly thorny case, that I can't explain yet, is why we use
|tentativeValue| here instead of |funcsatValue|. |tentativeValue| works just
like its counterpart, only looks at values that have been {\it already
propagated} by BCP. This can be fewer, but never more, than those that are
assigned.


@<Global def...@>=
static inline mbool tentativeValue(funcsat *f, literal p)
{
  variable v = fs_lit2var(p);
  literal *valLoc = &f->trail.data[f->model.data[v]];
  bool isTentative = valLoc >= f->trail.data + f->propq;
  if (levelOf(f,v) != Unassigned && !isTentative) return p == *valLoc;
  else return unknown;
}

@

@<Internal decl...@>=
static inline mbool tentativeValue(funcsat *f, literal p);


@ We need to initialize the unit facts.

@<Initialize funcsat type@>=
  FS_CALLOC(f->unit_facts, 2, sizeof(*f->unit_facts));
  f->unit_facts_size = 1;
  f->unit_facts_capacity = 2;


@ And grow them.
@<Incrementally resize internal data structures...@>=
if (f->unit_facts_size >= f->unit_facts_capacity) {
  FS_REALLOC(f->unit_facts, f->unit_facts_capacity*2, sizeof(*f->unit_facts));
  f->unit_facts_capacity *= 2;
}
head_tail_clear(&f->unit_facts[f->unit_facts_size]);
f->unit_facts_size++;


@ And free them.
@<Destroy func...@>=
free(f->unit_facts);

@

@<Internal decl...@>=
void minimizeUip(funcsat *f, clause *learned);

@*2 |litpos|. The |litpos| structure maintains a mapping of literals to indices
-- so that one can quickly query, ``is this literal in this clause?'' The
|indices| field is literally just a table indexed by |literal|s and mapping to
associated clause indices.

@d NO_POS UINTPTR_MAX

@<Internal ty...@>=
struct litpos
{
  clause *c; /* associated clause */
  struct vec_uintptr *indices; /* size is normally |f->numVars| */
};

@ 
@<Global def...@>=
static inline void litpos_init(struct litpos *r)
{
  r->c = NULL;
  r->indices = vec_uintptr_init(2);
}

static inline void litpos_destroy(struct litpos *lp)
{
  vec_uintptr_destroy(lp->indices);
}

@ Maps the literal |*p| to its index in the current clause. We require a pointer
to ``prove'' that the literal is in the clause.

@<Global def...@>=
static inline void litpos_set(struct litpos *lp, literal *p)
{
  assert(lp->c->data <= p);
  assert(p < lp->c->data + lp->c->size);
  lp->indices->data[fs_lit2var(*p)] = (uintptr_t)(p - lp->c->data);
}

@ Various ways to test/get a literal.
@<Global def...@>=
static inline literal *litpos_lit(struct litpos *lp, literal l)
{
  return &lp->c->data[litpos_pos(lp, l)];
}

static inline uintptr_t litpos_pos(struct litpos *lp, literal l)
{
  return lp->indices->data[fs_lit2var(l)];
}

static inline bool litpos_contains(struct litpos *lp, literal l)
{
  return lp->indices->data[fs_lit2var(l)] != NO_POS;
}

@ Removes the mapping for |literal| |p|. We don't take a pointer because |p|
may not be in the clause.
@<Global def...@>=
static inline void litpos_unset(struct litpos *lp, literal p)
{
  lp->indices->data[fs_lit2var(p)] = NO_POS;
}

@ Clears all positions and removes clause association.
@<Global def...@>=
static inline void litpos_clear(struct litpos *lp)
{
  for_vec_uintptr (up, lp->indices)
    *up = NO_POS;
}

@ Check invariant.
@<Global def...@>=
inline void litpos_check(struct litpos *lp)
{
#ifndef NDEBUG
  literal *p;
  for_clause (p, lp->c)
    assert(lp->c->data[litpos_pos(lp, *p)] == *p);
#endif
}

@ The next few sections deal with how |litpos| is used during solving.

When a new |funcsat| gets instantiated, we need to make sure it has a |litpos|
in it.

@<Initialize func...@>=
litpos_init(&f->litpos_uip);
f->litpos_uip.c = &f->uipClause;
vec_uintptr_grow_to(f->litpos_uip.indices, f->numVars+1);
vec_uintptr_push(f->litpos_uip.indices, NO_POS);

@ When resizing, resize with no new position info.

@<Incrementally resize internal...@>=
vec_uintptr_push(f->litpos_uip.indices, NO_POS);

@ When a new |funcsat| is destroy, we free.
@<Destroy func...@>=
litpos_destroy(&f->litpos_uip);

@ When conflict analysis begins, we will initialize our (already-allocated)
|litpos| with the positions of all literals in the conflicting clause.
@<Global def...@>=
static inline void litpos_init_from_uip_clause(funcsat *f)
{
  literal *p;
  struct litpos *lp = &f->litpos_uip;
  assert(f->numVars+1 == lp->indices->size);
  assert(lp->c == &f->uipClause);
  for_clause (p, lp->c) litpos_set(lp, p);
}

@
@<Internal decl...@>=
static inline void litpos_init(struct litpos *);
static inline void litpos_destroy(struct litpos *lp);
static inline void litpos_init_from_uip_clause(funcsat *f);
static inline void litpos_set(struct litpos *lp, literal *p);
static inline void litpos_unset(struct litpos *lp, literal p);
static inline literal *litpos_lit(struct litpos *pos, literal l);
static inline uintptr_t litpos_pos(struct litpos *lp, literal l);
static inline bool litpos_contains(struct litpos *pos, literal l);
static inline void litpos_clear(struct litpos *lp);
extern void litpos_check(struct litpos *lp);

@*1 Clause sweeping.

It turns you need to sweep the clause database from time to time. \funcsat\
exposes several strategios for sweeping.

@*2 glucose strategy. The first strategy we discuss is the \textit{literal
blocks distance} (\LBD) heuristic first introduced by glucose. (In this paper:
``Predicting Learnt Clauses Quality in Modern SAT Solvers'' by Audemard and
Simon.) The \LBD\ score of a clause is the number of different decision levels
of the literals that participate in a clause. For example, for the clause $(-5
\or 6 \or 7)$, if $-5$ and $6$ were set at level 1 and $7$ at level 2, then the
\LBD\ of the clause is 2. Incidentally, clauses with \LBD\ 2 are special -- they
are called \textit{glue clauses} because they glue decision levels together.

We compute the \LBD\ score by walking over the clause and counting levels. But
we use a trick borrowed from lookahead solvers: if a literal of level $d$ occurs
in the clause, we map that level $d$ to a counter value $c$. If there is a level
in the map that is mapped to something other than $c$, we know it didn't occur
in this clause. This makes ``clearing'' the map trivial -- we simply increment
the counter (which is done on entry to this function).

Our auxiliary map is |f->LBD_levels|. |f->LBD_levels.data[d] == f->LBD_count|
iff we've seen a literal l in |c| whose level is |d|. |f->LBD_count| is our
counter.

Things to do:
\orderedlist

\li I may eventually add a callback to adjust the \LBD\ of clauses as they are
unit propagated.

\endorderedlist

@<Global def...@>=
static inline uint8_t LBD_compute_score(funcsat *f, uintptr_t ix)
{
  uint8_t score = 0;
  struct clause_iter it;
#ifndef NDEBUG
  struct clause_head *h = clause_head_ptr(f, ix); /* only used by |assert| */
#endif
  f->LBD_count++; /* effectively zeros |f->LBD_levels| */

  clause_iter_init(f, ix, &it);
  for (literal *p = clause_iter_next(f, &it); p; p = clause_iter_next(f, &it)) {
    literal level = levelOf(f, fs_lit2var(*p));
    assert(level != Unassigned);
    if (f->LBD_levels.data[level] != f->LBD_count) {
      f->LBD_levels.data[level] = f->LBD_count;
      score++;
    }
     if (score > LBD_SCORE_MAX) break; /* who cares if the score is big. */
  }
  assert(h->sz == 0 || score != 0); /* |sz!=0| implies |score!=0| */
  return score;
}

@ Like many data structures, auxiliary \LBD\ structures need to be initialized,
resized, and destroyed. The next few sections take care of this.
@<Initialize func...@>=
vec_uint64_mk(&f->LBD_levels, 2);
vec_uint64_push(&f->LBD_levels, 0);

@
@<Incrementally resize internal...@>=
vec_uint64_push(&f->LBD_levels, 0);

@
@<Destroy func...@>=
vec_uint64_unmk(&f->LBD_levels);


@
@<Global def...@>=
void LBD_decay_after_conflict(funcsat *f)
{
#if 0
  varDecayActivity(f);
#endif
}
@ Each time a new, minimized clause is learned, this function is used to adjust
its \LBD\ heuristic value.

@<Global def...@>=
void LBD_bump_learned(funcsat *f, uintptr_t ix)
{
  struct clause_iter it;
  struct clause_head *h = clause_head_ptr(f, ix);
  h->lbd_score = LBD_compute_score(f, ix);

  clause_iter_init(f, ix, &it);
  for (literal *p = clause_iter_next(f, &it); p; p = clause_iter_next(f, &it)) {
    varBumpScore(f, fs_lit2var(*p));
  }
  varDecayActivity(f);
}

@ As clauses are being learned, we encounter various ``reason'' clauses that
participate in the conflict. This function will be called on each of them to
adjust their priority.
@<Global def...@>=
void LBD_bump_reason(funcsat *f, uintptr_t ix)
{
  struct clause_iter it;
  clause_iter_init(f, ix, &it);
  for (literal *p = clause_iter_next(f, &it); p; p = clause_iter_next(f, &it)) {
    variable v = fs_lit2var(*p);
#if 0
    uintptr_t r = getReason(f, (literal)v);
    if (r != NO_CLS) {
      struct clause_head *h = clause_head_ptr(f,r);
      if (h->lbd_score == 2) { /* bump twice for glue */
        varBumpScore(f, fs_lit2var(h->lits[0]));
      }
    }
#endif
    varBumpScore(f, fs_lit2var(*p));
  }
}


@ The first task of |LBD_sweep| is to figure out if it's time to perform a
sweep. We use essentially the same test as in glucose.


@<Global def...@>=
void LBD_sweep(funcsat *f, void *user)
{
  uint64_t diffConfl = f->numConflicts - f->LBD_last_num_conflicts;
#if 0
  uint64_t avg = diffConfl==0 ? 1 : Max(1, diffLearn/diffConfl);
#endif
  uint64_t next = f->LBD_base + f->LBD_increment*f->numSweeps;
  /* in the glucose paper they used 20000+500*num */
  assert(next > 0);
  if (diffConfl > next) {
    ++f->numSweeps;
    @<Sweep clauses with \LBD...@>@;
  }
}

@ Settings from the glucose paper.
@<Initialize func...@>=
f->LBD_base = 20000;
f->LBD_increment = 500;

@ Both original and learned clauses are in the same watcher list data
structure. It would be nice not to have to entirely recreate that structure
every time we want to delete clauses.

First, compute the max number of clauses to delete, $m$. Let $M$ be
|LBD_SCORE_MAX|. Second, compute a histogram of \LBD\ scores. For each score (in
ascending order) from $n \in [1,M]$, count how many clauses have that
score. Beginning from $M$, compute a running sum $x$ until $x+n > m$. This way
all clauses with score greater than the threshold $n$ will surely be deleted
(unless they are pinned -- more later); scores $n$ and lower contain some
clauses that may be deleted and others that won't. Basically, we've computed a
loose percentile of the clauses to delete.

Next, walk over the learned clauses again and mark to delete those beyond the
threshold. And walk over the watchlists to delete the references to them.

Guess what -- we don't have to sort the learned clauses by priority!

Now if only I can get away with not walking over the watchlists....

@<Sweep clauses with \LBD\ heuristic@>=
uint64_t bad_clauses; /* num. clauses worse than threshold */
uint8_t del_score; /* deletion threshold */
uint64_t max_del = f->learned_clauses->size/2, num_del = 0;

fslog(f, "sweep", 1, "%ju learned clauses\n", f->learned_clauses->size);
fslog(f, "sweep", 1, "max_del is %" PRIu64 "\n", max_del);

@<Create histogram...@>@;
fslog(f, "sweep", 2, "bad_clauses is %" PRIu64 "\n", bad_clauses);
fslog(f, "sweep", 2, "del_score is %" PRIu8 "\n", del_score);

if (del_score > 2) {
  @<Delete clauses worse than \LBD...@>@;
  @<Delete middling clauses by \LBD@>@;
}
fslog(f, "sweep", 1, "deleted %" PRIu64 " clauses\n", num_del);

f->LBD_last_num_conflicts = f->numConflicts;

@
@<Create histogram of \LBD\ scores and compute |del_score| and |bad_clauses|@>=
del_score = 0, bad_clauses = 0;
vec_uint64_zero(&f->LBD_histogram);
for_vec_uintptr (ix_cls, f->learned_clauses) {
  assert(clause_head_ptr(f, *ix_cls)->lbd_score > 0);
  f->LBD_histogram.data[clause_head_ptr(f, *ix_cls)->lbd_score]++;
}
    
for (uint8_t s = LBD_SCORE_MAX; s > 0; s--) {
  if (bad_clauses + f->LBD_histogram.data[s] > max_del) {
    del_score = s;
    break;
  }
  bad_clauses += f->LBD_histogram.data[s];
}
assert(del_score);

@
@<We can mark head |h| for deletion@>=
(h->is_learned && !h->is_reason && h->where==CLAUSE_WATCHED &&
 h->lbd_score > del_score)

@ Same as previous section except we assume the clause is learned.
@<We can mark learned head |h| for deletion@>=
(!h->is_reason && h->where==CLAUSE_WATCHED &&
 h->lbd_score > del_score)

@ Deleting the clauses has two steps. First we walk the watchlist to delete the
corresponding watches. Second we walk the learned clauses free them globally --
this way we don't have dangling pointers.

@<Delete clauses worse than \LBD\ score of |del_score|@>=
if (bad_clauses) {
  uintptr_t i, new_size; 
  for (variable v = 1; v <= f->numVars; v++) {
    struct watchlist_elt *elt, *dump;
    struct watchlist *wl;

    wl = &f->watches.wlist[fs_lit2idx((literal)v)], new_size = wl->size;
    for_watchlist (elt, dump, wl) {
      @<Delete watchlist |elt| if...@>@;
    }
    wl->size = new_size;
    wl = &f->watches.wlist[fs_lit2idx(-(literal)v)], new_size = wl->size;
    for_watchlist (elt, dump, wl) {
      @<Delete watchlist |elt| if...@>@;
    }
    wl->size = new_size;
  }
  for (i = 0, new_size = 0; i < f->learned_clauses->size; i++) {
    uintptr_t elt = f->learned_clauses->data[i];
    struct clause_head *h = clause_head_ptr(f, elt);
    if (@<We can mark learned head |h|...@>) {
      num_del++, bad_clauses--;
      f->numLearnedDeleted++;
      fslog(f, "sweep", 6, "deleted %" PRIuPTR "\n", elt);
      clause_release(f, elt);
    } else
      f->learned_clauses->data[new_size++] = elt;
  }
  f->learned_clauses->size = new_size;
}

@
@<Delete watchlist |elt| if score no less than |del_score|@>=
struct clause_head *h = clause_head_ptr(f, elt->cls);
if (@<We can mark head |h|...@>) {
  fs_ifdbg(f, "sweep", 5) {
    fslog(f, "sweep", 5, "marked clause ");
    fs_clause_head_print(f, fs_dbgout(f), elt->cls);
    fprintf(fs_dbgout(f), " score is %x", (unsigned)clause_head_ptr(f, elt->cls)->lbd_score);
    NEWLINE(fs_dbgout(f));
  }
  new_size--;
  continue; /* means to delete this |watchlist_elt| */
}
*dump = *elt, watchlist_next_elt(dump, wl); /* keep |elt| in this |wl| */



@ This isn't implemented yet.

@<Delete middling clauses by \LBD@>=


@ Obviously we should initialize the histogram when \funcsat\ starts up.
@<Initialize func...@>=
vec_uint64_mk(&f->LBD_histogram, LBD_SCORE_MAX+1);
f->LBD_histogram.size = LBD_SCORE_MAX+1;

@
@<Destroy func...@>=
vec_uint64_unmk(&f->LBD_histogram);

@
@<Global def...@>=
static int LBD_compare_rev(void *fp, const void *cl1, const void *cl2)
{
  funcsat *f = (funcsat *)fp;
  struct clause_head *c1 = clause_head_ptr(f, *(uintptr_t *)cl1);
  struct clause_head *c2 = clause_head_ptr(f, *(uintptr_t *)cl2);
  uint64_t s1 = c1->lbd_score, s2 = c2->lbd_score;
  if (s1 == s2) return 0;
  else if (s1 > s2) return -1;
  else return 1;
}

@
@<Internal decl...@>=
static int compareByActivityRev(const void *cl1, const void *cl2);
static int LBD_compare_rev(void *f, const void *cl1, const void *cl2);
void LBD_bump_reason(funcsat *f, uintptr_t c);
void LBD_bump_learned(funcsat *f, uintptr_t C);
static inline uint8_t LBD_compute_score(funcsat *f, uintptr_t c);
void LBD_decay_after_conflict(funcsat *f);
void LBD_sweep(funcsat *f, void *user);


@ In order to use the \LBD\ strategy, we expose a client function.
@<Global def...@>=
void funcsatSetupLbdGc(funcsat_config *conf)
{
  conf->gc = true;
  conf->sweepClauses = LBD_sweep;
  conf->bumpReason = LBD_bump_reason;
  conf->bumpLearned = LBD_bump_learned;
  conf->decayAfterConflict = LBD_decay_after_conflict;
}

@
@<External decl...@>=
void funcsatSetupLbdGc(funcsat_config *);

@*2 activity strategy.

@<Global def...@>=
void claActivitySweep(funcsat *f, void *user)
{
  static uint64_t num = 0, lastNumConflicts = 0, lastNumLearnedClauses = 0;
#if 0
  const uint64_t diffConfl = f->numConflicts - lastNumConflicts;
  const uint64_t diffLearn = f->numLearnedClauses - lastNumLearnedClauses;
  uint64_t size = f->learnedClauses.size;
  double extraLim = f->claInc / (size*1.f);

  if (f->numLearnedClauses*1.f >= f->maxLearned) {
    ++f->numSweeps;
    uint64_t numDeleted = 0;
    
    /* sort by activity and delete half */
    qsort(f->learnedClauses.data, f->learnedClauses.size, sizeof(clause *), compareByActivityRev);

    fs_ifdbg (f, "sweep", 5) {
      uint64_t dupCount = 0;
      int32_t lastLbd = LBD_SCORE_MAX;
      FILE *out = f->conf->debugStream;
      fprintf(out, "sorted:\n");
      clause **it;
      forVectorRev (clause **, it, &f->learnedClauses) {
        if ((*it)->lbdScore == lastLbd) {
          dupCount++;
        } else {
          if (dupCount > 0) {
            fprintf(out, "(repeats %" PRIu64 " times) %d ", dupCount, (*it)->lbdScore);
          } else {
            fprintf(out, "%d ", (*it)->lbdScore);
          }
          dupCount = 0;
          lastLbd = (*it)->lbdScore;
        }
      }
      if (dupCount > 0) {
        fprintf(out, "%d (repeats %" PRIu64 " times) ", lastLbd, dupCount);
      }

      fprintf(out, "done\n");
    }

    /* restore proper clause ids after sort. |funcsatRemoveClause| depends on this. */
    for (uintmax_t i = 0; i < f->learnedClauses.size; i++) {
      clause *c = f->learnedClauses.data[i];
    }

    /* remove clauses with 0 activity */
#if 0
    for (variable i = 0; i < f->learnedClauses.size; i++) {
      clause *c = f->learnedClauses.data[i];
      if (c->activity == 0.f) {
        clause *rmClause = funcsatRemoveClause(f, c);
        if (rmClause) {
          clauseFree(c);
          --f->numLearnedClauses, ++f->numLearnedDeleted, ++numDeleted;
        }
      } else break;
    }
#endif
  
    const uint64_t max = f->learnedClauses.size/2;
    fslog(f, "sweep", 1, "deleting at most %" PRIu64
          " clauses (of %ju)\n", max, f->learnedClauses.size);
    for (variable i = 0; i < f->learnedClauses.size && f->learnedClauses.size > max; i++) {
      clause *c = f->learnedClauses.data[i];
      if (c->size > 2) {
        clause *rmClause = funcsatRemoveClause(f, c);
        if (rmClause) {
          clauseFree(c);
          --f->numLearnedClauses, ++f->numLearnedDeleted, ++numDeleted;
        }
      }
    }
    fslog(f, "sweep", 1, ">>> deleted %" PRIu64 " clauses\n", numDeleted);

    num++;
    lastNumLearnedClauses = f->numLearnedClauses;
    lastNumConflicts = f->numConflicts;
  }
#endif
}


@ More bupming.
@<Global def...@>=
void bumpReasonByActivity(funcsat *f, uintptr_t ix)
{
  struct clause_iter it;
  bumpClauseByActivity(f, ix);

  clause_iter_init(f, ix, &it);
  for (literal *p = clause_iter_next(f, &it); p; p = clause_iter_next(f, &it)) {
    varBumpScore(f, fs_lit2var(*p));
  }
}

void bumpLearnedByActivity(funcsat *f, uintptr_t ix)
{
  struct clause_iter it;
  bumpClauseByActivity(f, ix);

  clause_iter_init(f, ix, &it);
  for (literal *p = clause_iter_next(f, &it); p; p = clause_iter_next(f, &it)) {
    varBumpScore(f, fs_lit2var(*p));
    varBumpScore(f, fs_lit2var(*p));
  }

  if (--f->learnedSizeAdjustCnt == 0) {
    f->learnedSizeAdjustConfl *= f->learnedSizeAdjustInc;
    f->learnedSizeAdjustCnt    = (uint64_t)f->learnedSizeAdjustConfl;
    f->maxLearned             *= f->learnedSizeInc;
    fslog(f, "sweep", 1, "update: ml %f\n", f->maxLearned);
    
  }

  varDecayActivity(f);
  claDecayActivity(f);
}

void lbdBumpActivity(funcsat *f, uintptr_t ix)
{
  struct clause_iter it;
  clause_iter_init(f, ix, &it);
  for (literal *p = clause_iter_next(f, &it); p; p = clause_iter_next(f, &it)) {
    variable v = fs_lit2var(*p);
    uintptr_t reason = getReason(f, (literal)v);
    struct clause_head *h = clause_head_ptr(f, reason);
    if (reason != NO_CLS && h->lbd_score == 2) {
      varBumpScore(f,v);
    }
  }
}

@
@<Global def...@>=
static void bumpClauseByActivity(funcsat *f, uintptr_t ix)
{
  struct clause_iter it;
  struct clause_head *h = clause_head_ptr(f, ix);
  /* if the clause's activity is too large, rescale */
  if ((h->activity += f->claInc) > 1e20) {
    fslog(f, "sweep", 5, "rescale for activity %f\n", h->activity);
    for_vec_uintptr (ix_cl, f->learned_clauses) {
      struct clause_head *h_cl = clause_head_ptr(f, *ix_cl);
      double oldActivity = h_cl->activity;
      h_cl->activity *= 1e-20;
      fslog(f, "sweep", 5, "setting activity from %f to %f\n", oldActivity, h_cl->activity);
    }
    double oldClaInc = f->claInc;
    f->claInc *= 1e-20;
    fslog(f, "sweep", 5, "setting claInc from %f to %f\n", oldClaInc, f->claInc);
  }
}

static inline void varDecayActivity(funcsat *f)
{
  f->varInc *= (1 / f->varDecay);
}
static void claDecayActivity(funcsat *f)
{
  double oldClaInc = f->claInc;
  f->claInc *= (1 / f->claDecay);
  fslog(f, "sweep", 9, "decaying claInc from %f to %f\n", oldClaInc, f->claInc);
}

@*1 Restart functions.

@<Global def...@>=
void incLubyRestart(funcsat *f, bool skip);

bool funcsatNoRestart(funcsat *f, void *p) {
  return false;
}

bool funcsatLubyRestart(funcsat *f, void *p)
{
  if ((intmax_t)f->numConflicts >= f->lrestart && f->decisionLevel > 2) {
    incLubyRestart(f, false);
    return true;
  }
  return false;
}

bool funcsatInnerOuter(funcsat *f, void *p)
{
  static uint64_t inner = 100UL, outer = 100UL, conflicts = 1000UL;
  if (f->numConflicts >= conflicts) {
    conflicts += inner;
    if (inner >= outer) {
      outer *= 1.1;
      inner = 100UL;
    } else {
      inner *= 1.1;
    }
    return true;
  }
  return false;
}

bool funcsatMinisatRestart(funcsat *f, void *p)
{
  static uint64_t cutoff = 100UL;
  if (f->numConflicts >= cutoff) {
    cutoff *= 1.5;
    return true;
  }
  return false;
}



/* This stuff was cribbed from \picosat\ and changed a smidge just to get bigger
 * integers. */


int64_t luby(int64_t i)
{
  int64_t k;
  for (k = 1; k < (int64_t)sizeof(k); k++)
    if (i == (1 << k) - 1)
      return 1 << (k - 1);

  for (k = 1;; k++)
    if ((1 << (k - 1)) <= i && i < (1 << k) - 1)
      return luby (i - (1 << (k-1)) + 1);
}

void incLubyRestart(funcsat *f, bool skip)
{
  uint64_t delta;

  /* Luby calculation takes a really long time around 255? */
  if (f->lubycnt > 250) {
    f->lubycnt = 0;
    f->lubymaxdelta = 0;
    f->waslubymaxdelta = false;
  }
  delta = 100 * (uint64_t)luby((int64_t)++f->lubycnt);
  f->lrestart = (int64_t)(f->numConflicts + delta);

  /* if (waslubymaxdelta) */
  /*   report (1, skip ? 'N' : 'R'); */
  /* else */
  /*   report (2, skip ? 'n' : 'r'); */

  if (delta > f->lubymaxdelta) {
    f->lubymaxdelta = delta;
    f->waslubymaxdelta = 1;
  } else {
    f->waslubymaxdelta = 0;
  }
}


@*1 Configuration. \funcsat\ is configurable in many ways. The |funcsat_config|
type is responsible for \funcsat's configuration.

@<Internal decl...@>=
void myDecayAfterConflict(funcsat *f);

@
@<Global def...@>=

funcsat_config funcsatDefaultConfig = {
  .user = NULL,
  .name = NULL,
  .usePhaseSaving = true,
  .useSelfSubsumingResolution = false,
  .minimizeLearnedClauses = true,
  .numUipsToLearn = UINT32_MAX,
  .gc = true,
  .maxJailDecisionLevel = 0,
  .logSyms = NULL,
  .printLogLabel = true,
  .debugStream = NULL,
  .isTimeToRestart = funcsatLubyRestart,
  .isResourceLimitHit = funcsatIsResourceLimitHit,
  .preprocessNewClause = funcsatPreprocessNewClause,
  .preprocessBeforeSolve = funcsatPreprocessBeforeSolve,
  .getInitialActivity = funcsatDefaultStaticActivity,
  .sweepClauses = LBD_sweep,
  .bumpOriginal = bumpOriginal,
  .bumpReason = LBD_bump_reason,
  .bumpLearned = LBD_bump_learned,
  .bumpUnitClause = bumpUnitClauseByActivity,
  .decayAfterConflict = LBD_decay_after_conflict,
};

@ We have the option of bumping the priority of variables as clauses are added
to \funcsat.

@<Global def...@>=
void bumpOriginal(funcsat *f, uintptr_t ix_clause)
{
  struct clause_head *h = clause_head_ptr(f, ix_clause);
  double orig_varInc = f->varInc;
  struct clause_iter it;

  f->varInc +=  2.*(1./(double)h->sz);
  clause_iter_init(f, ix_clause, &it);
  for (literal *p = clause_iter_next(f, &it); p; p = clause_iter_next(f, &it)) {
    varBumpScore(f, fs_lit2var(*p));
  }

  f->varInc = orig_varInc;

#if 0
  forClause (it, c) {
    varBumpScore(f, fs_lit2var(*it));
  }
#endif
}

double funcsatDefaultStaticActivity(variable *v)
{
  return 1.f;
}

void myDecayAfterConflict(funcsat *f)
{
  varDecayActivity(f);
  claDecayActivity(f);
}

void bumpUnitClauseByActivity(funcsat *f, uintptr_t ix)
{
  bumpClauseByActivity(f, ix);
}


@ Prototypes.
@<Internal decl...@>=
void bumpOriginal(funcsat *f, uintptr_t c);
void bumpReasonByActivity(funcsat *f, uintptr_t c);
void bumpLearnedByActivity(funcsat *f, uintptr_t c);
void bumpUnitClauseByActivity(funcsat *f, uintptr_t c);
void lbdBumpActivity(funcsat *f, uintptr_t c);
static inline void varDecayActivity(funcsat *f);
static void claDecayActivity(funcsat *f);
static void bumpClauseByActivity(funcsat *f, uintptr_t c);
extern void varBumpScore(funcsat *f, variable v);


@
@<Internal decl...@>=
void claActivitySweep(funcsat *f, void *user);

@*1 Initialization. Requires an initialized configuration.

@<Initialize parameters@>=
  f->varInc = 1.f;
  f->varDecay = 0.95f; /* or 0.999f; */
  f->claInc = 1.f;
  f->claDecay = 0.9999f; /* or 0.95f; */
  f->learnedSizeFactor = 1.f / 3.f;
  f->learnedSizeAdjustConfl = 25000;  /* see |startSolving| */ 
  f->learnedSizeAdjustCnt = 25000;
  f->maxLearned = 20000.f;
  f->learnedSizeAdjustInc = 1.5f;
  f->learnedSizeInc = 1.1f;


@ Initially allocates a |funcsat| object.
@c

funcsat *funcsatInit(funcsat_config *conf)
{
  funcsat *f;
  FS_CALLOC(f, 1, sizeof(*f));
  f->conf = conf;
  f->propq = 0;
  @<Initialize parameters@>@;
  fslog(f, "sweep", 1, "set maxLearned to %f\n", f->maxLearned);
  fslog(f, "sweep", 1, "set 1/f->claDecoy to %f\n", 1.f/f->claDecay);
  f->lrestart = 1;
  f->lubycnt = 0;
  f->lubymaxdelta = 0;
  f->waslubymaxdelta = false;

  f->numVars = 0;
  @<Initialize funcsat type@>@;
  clauseInit(&f->assumptions, 0);
  vec_uintmax_mk(&f->model, 2);
  vec_uintmax_push(&f->model, 0);
  clauseInit(&f->phase, 2);
  clausePush(&f->phase, 0);
  clauseInit(&f->level, 2);
  clausePush(&f->level, Unassigned);
  vec_uintmax_mk(&f->decisions, 2);
  vec_uintmax_push(&f->decisions, 0);
  clauseInit(&f->trail, 2);
  all_watches_init(f);
  clauseInit(&f->uipClause, 100);
  vec_ptr_mk(&f->subsumed, 10);

  return f;
}



@ Initializes a |funcsat| configuration.
@c
funcsat_config *funcsatConfigInit(void *userData)
{
  funcsat_config *conf = malloc(sizeof(*conf));
  memcpy(conf, &funcsatDefaultConfig, sizeof(*conf));
#ifdef FUNCSAT_LOG
  conf->logSyms = create_hashtable(16, hashString, stringEqual);
  vec_uintmax_mk(&conf->logStack, 2);
  vec_uintmax_push(&conf->logStack, 0);
  conf->debugStream = stderr;
#endif
  return conf;
}

void funcsatConfigDestroy(funcsat_config *conf)
{
#ifdef FUNCSAT_LOG
  hashtable_destroy(conf->logSyms, true, true);
  vec_uintmax_unmk(&conf->logStack);
#endif
  free(conf);
}

@ For incrementality, \funcsat\ resizes for a given predicted or known number of
variables. It just goes through and grows every data structure to fit |numVars|.

@c

void funcsatResize(funcsat *f, variable numVars)
{
  assert(f->decisionLevel == 0); /* so we can fix up unit facts */
  if (numVars > f->numVars) {
    const variable old = f->numVars, new = numVars;
    f->numVars = new;
    variable i;

    clauseGrowTo(&f->uipClause, numVars);
    @<Resize internal data structures up to new |numVars|@>@;
    for (i = old; i < new; i++) {
      variable v = i+1;
      @<Incrementally resize internal data structures up to new |numVars|@>@;
#if 0
      literal l = fs_var2lit(v);
#endif
      vec_uintmax_push(&f->model, UINTMAX_MAX);
      clausePush(&f->phase, -fs_var2lit(v));
      clausePush(&f->level, Unassigned);
      vec_uintmax_push(&f->decisions, 0);
    }
    uintmax_t highestIdx = fs_lit2idx(-(literal)numVars)+1;
    assert(f->model.size     == numVars+1);
    assert(!f->conf->usePhaseSaving || f->phase.size == numVars+1);
    assert(f->level.size     == numVars+1);
    assert(f->decisions.size == numVars+1);
    assert(f->reason->size    == numVars+1);
    assert(f->unit_facts_size == numVars+1);
    assert(f->uipClause.capacity >= numVars);
    assert(f->allLevels.size == numVars+1);
    assert(f->watches.size   == highestIdx);

    if (numVars > f->trail.capacity) {
      FS_REALLOC(f->trail.data, numVars, sizeof(*f->trail.data));
      f->trail.capacity = numVars;
    }
  }
}

@ Destroys a |funcsat|.

@c
void funcsatDestroy(funcsat *f)
{
  literal i;
  while (f->trail.size > 0) trailPop(f, NULL);
  @<Destroy funcsat type@>@;
  clauseDestroy(&f->assumptions);
  vec_uintmax_unmk(&f->model);
  clauseDestroy(&f->phase);
  clauseDestroy(&f->level);
  vec_uintmax_unmk(&f->decisions);
  clauseDestroy(&f->trail);
  clauseDestroy(&f->uipClause);
  vec_ptr_unmk(&f->subsumed);
  free(f);
}


@

@c
void funcsatSetupActivityGc(funcsat_config *conf)
{
  conf->gc = true;
  conf->sweepClauses = claActivitySweep;
  conf->bumpReason = bumpReasonByActivity;
  conf->bumpLearned = bumpLearnedByActivity;
  conf->decayAfterConflict = myDecayAfterConflict;
}

funcsat_result funcsatResult(funcsat *f)
{
  return f->lastResult;
}





@ These two functions are for sanity's sake.

@c

funcsat_result startSolving(funcsat *f)
{
  f->numSolves++;
  if (f->conflict_clause != NO_CLS) {
    struct clause_head *h = clause_head_ptr(f, f->conflict_clause);
    if(h->sz == 0) {
      return FS_UNSAT;
    } else {
      f->conflict_clause = NO_CLS;
    }
  }

  backtrack(f, 0UL, NULL, true);
  f->lastResult = FS_UNKNOWN;

  assert(f->decisionLevel == 0);
  return FS_UNKNOWN;
}

/**
 * Call me whenever (incremental) solving is done, whether because of a timeout
 * or whatever.
 */
static void finishSolving(funcsat *f)
{

}




@ Another break.

@c


static bool bcpAndJail(funcsat *f)
{
  if (!bcp(f)) {
    fslog(f, "solve", 2, "returning false at toplevel\n");
    return false;
  }

  fslog(f, "solve", 1, "bcpAndJail trailsize is %ju\n", f->trail.size);

  clause **cIt;
  struct vec_ptr *watches;
  uint64_t numJails = 0;

#if 0
  forVector (clause **, cIt, &f->origClauses) {
    if ((*cIt)->is_watched) {
      literal *lIt;
      bool allFalse = true;
      forClause (lIt, *cIt) {
        mbool value = funcsatValue(f, *lIt);
        if (value == false) continue;
        allFalse = false;
        if (value == true) {
          /* jail the clause, it is satisfied */
          clause **w0 = (clause **) &watches->data[fs_lit2idx(-(*cIt)->data[0])];
          /* assert(watcherFind(*cIt, w0, 0)); */
          clause **w1 = (clause **) &watches->data[fs_lit2idx(-(*cIt)->data[1])];
          /* assert(watcherFind(*cIt, w1, 0)); */
          /* clauseUnSpliceWatch(w0, *cIt, 0); */
          clauseUnSpliceWatch(w1, *cIt, 1);
          jailClause(f, *lIt, *cIt);
          numJails++;
          break;
        }
      }
      if (allFalse) {
        fslog(f, "solve", 2, "returning false at toplevel\n");
        return false;
      }
    }
  }
#endif

  fslog(f, "solve", 2, "jailed %" PRIu64 " clauses at toplevel\n", numJails);
  return true;
}


bool funcsatIsResourceLimitHit(funcsat *f, void *p)
{
  return false;
}
funcsat_result funcsatPreprocessNewClause(funcsat *f, void *p, clause *c)
{
  return (f->lastResult = FS_UNKNOWN);
}
funcsat_result funcsatPreprocessBeforeSolve(funcsat *f, void *p)
{
  return (f->lastResult = FS_UNKNOWN);
}


@ Rest of stuff.

@c
void funcsatPrintStats(FILE *stream, funcsat *f)
{
  fprintf(stream, "c %" PRIu64 " decisions\n", f->numDecisions);
  fprintf(stream, "c %" PRIu64 " propagations (%" PRIu64 " unit)\n",
          f->numProps + f->numUnitFactProps,
          f->numUnitFactProps);
  fprintf(stream, "c %" PRIu64 " jailed clauses\n", f->numJails);
  fprintf(stream, "c %" PRIu64 " conflicts\n", f->numConflicts);
  fprintf(stream, "c %" PRIu64 " learned clauses\n", f->numLearnedClauses);
  fprintf(stream, "c %" PRIu64 " learned clauses removed\n", f->numLearnedDeleted);
  fprintf(stream, "c %" PRIu64 " learned clause deletion sweeps\n", f->numSweeps);
  if (!f->conf->minimizeLearnedClauses) {
    fprintf(stream, "c (learned clause minimisation off)\n");
  } else {
    fprintf(stream, "c %" PRIu64 " redundant learned clause literals\n", f->numLiteralsDeleted);
  }
  if (!f->conf->useSelfSubsumingResolution) {
    fprintf(stream, "c (on-the-fly self-subsuming resolution off)\n");
  } else {
    fprintf(stream, "c %" PRIu64 " subsumptions\n", f->numSubsumptions);
    fprintf(stream, "c - %" PRIu64 " original clauses\n", f->numSubsumedOrigClauses);
    fprintf(stream, "c - %" PRIu64 " UIPs (%2.2lf%%)\n", f->numSubsumptionUips,
            (double)f->numSubsumptionUips*100./(double)f->numSubsumptions);
  }
  fprintf(stream, "c %" PRIu64 " restarts\n", f->numRestarts);
  fprintf(stream, "c %d assumptions\n", f->assumptions.size);
  fprintf(stream, "c %ju original clauses\n", f->orig_clauses->size);
}

void funcsatPrintColumnStats(FILE *stream, funcsat *f)
{
  struct rusage usage;
  getrusage(RUSAGE_SELF, &usage);
  double uTime = ((double)usage.ru_utime.tv_sec) +
    ((double)usage.ru_utime.tv_usec)/1000000;
  double sTime = ((double)usage.ru_stime.tv_sec) +
    ((double)usage.ru_stime.tv_usec)/1000000;
  fprintf(stream, "Name,NumDecisions,NumPropagations,NumUfPropagations,"
                  "NumLearnedClauses,NumLearnedClausesRemoved,"
                  "NumLearnedClauseSweeps,NumConflicts,NumSubsumptions,"
                  "NumSubsumedOrigClauses,NumSubsumedUips,NumRestarts,"
                  "UserTimeSeconds,SysTimeSeconds\n");
  fprintf(stream, "%s,", f->conf->name);
  fprintf(stream, "%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64
          ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64
          ",%" PRIu64 ",%.2lf,%.2lf\n",
          f->numDecisions,
          f->numProps + f->numUnitFactProps,
          f->numUnitFactProps,
          f->numLearnedClauses,
          f->numLearnedDeleted,
          f->numSweeps,
          f->numConflicts,
          f->numSubsumptions,
          f->numSubsumedOrigClauses,
          f->numSubsumptionUips,
          f->numRestarts,
          uTime,
          sTime);
}

void funcsatBumpLitPriority(funcsat *f, literal p)
{
  varBumpScore(f, fs_lit2var(p));
}







/* FORMERLY INTERNAL.H */


/* defaults */




literal levelOf(funcsat *f, variable v)
{
  return f->level.data[v];
}


literal fs_var2lit(variable v)
{
  literal result = (literal) v;
  assert((variable) result == v);
  return result;
}

inline variable fs_lit2var(literal l)
{
  if (l < 0) {
    l = -l;
  }
  return (variable) l;
}

uintmax_t fs_lit2idx(literal l)
{
  /* just move sign bit into the lowest bit instead of the highest */
  variable v = fs_lit2var(l);
  v <<= 1;
  v |= (l < 0);
  return v;
}

inline bool isDecision(funcsat *f, variable v)
{
  return 0 != f->decisions.data[v];
}




#if 0
void singlesPrint(FILE *stream, clause *begin)
{
  clause *c = begin;
  if (c) {
    do {
      clause *next = c->next[0];
      clause *prev = c->prev[0];
      if (!next) {
        fprintf(stream, "next is NULL");
        return;
      }
      if (!prev) {
        fprintf(stream, "prev is NULL");
        return;
      }
      if (next->prev[0] != c) fprintf(stream, "n*");
      if (prev->next[0] != c) fprintf(stream, "p*");
      clause_head_print_dimacs(stream, c);
      c = next;
      if (c != begin) fprintf(stream, ", ");
    } while (c != begin);
  } else {
    fprintf(stream, "NULL");
  }
}
#endif

#if 0
void watcherPrint(FILE *stream, clause *c, uint8_t w)
{
  if (!c) {
    fprintf(stream, "EMPTY\n");
    return;
  }

  clause *begin = c;
  literal data = c->data[w];
  fprintf(stream, "watcher list containing lit %ji\n", c->data[w]);
  do {
    uint8_t i = c->data[0]==data ? 0 : 1;
    clause *next = c->next[i];
    if (!next) {
      fprintf(stream, "next is NULL\n");
      return;
    }
    if (!(next->prev[ next->data[0]==c->data[i] ? 0 : 1 ] == c)) {
      fprintf(stream, "*");
    }
    clause_head_print_dimacs(stream, c);
    fprintf(stream, "\n");
    c = next;
  } while (c != begin);
}



bool watcherFind(clause *c, clause **watches, uint8_t w)
{
  clause *curr = *watches, *nx, *end;
  uint8_t wi;
  bool foundEnd;
  forEachWatchedClause(curr, c->data[w], wi, nx, end, foundEnd) {
    if (curr == c) return true;
  }
  return false;
}

void binWatcherPrint(FILE *stream, funcsat *f)
{
  variable v;
  uintmax_t i;
  for (v = 1; v <= f->numVars; v++) {
    binvec_t *bv = f->watchesBin.data[fs_lit2idx(fs_var2lit(v))];
    if (bv->size > 0) {
      fprintf(stream, "%5ji -> ", fs_var2lit(v));
      for (i = 0; i < bv->size; i++) {
        literal imp = bv->data[i].implied;
        fprintf(stream, "%ji", imp);
        if (i+1 != bv->size) fprintf(stream, ", ");
      }
      fprintf(stream, "\n");
    }

    bv = f->watchesBin.data[fs_lit2idx(-fs_var2lit(v))];
    if (bv->size > 0) {
      fprintf(stream, "%5ji -> ", -fs_var2lit(v));
      for (i = 0; i < bv->size; i++) {
        literal imp = bv->data[i].implied;
        fprintf(stream, "%ji", imp);
        if (i+1 != bv->size) fprintf(stream, ", ");
      }
      fprintf(stream, "\n");
    }
  }
}

#endif

uintmax_t funcsatNumClauses(funcsat *f)
{
  return f->orig_clauses->size;
}

uintmax_t funcsatNumVars(funcsat *f)
{
  return f->numVars;
}

@

@c
void funcsatPrintHeuristicValues(FILE *p, funcsat *f)
{
  for (variable i = 1; i <= f->numVars; i++) {
    double *value = bh_var2act(f,i);
    fprintf(p, "%ju = %4.2lf\n", i, *value);
  }
  fprintf(p, "\n");
}

void fs_print_state(funcsat *f, FILE *p)
{
  variable i;
  literal *it;
  if (!p) p = stderr;

  fprintf(p, "assumptions: ");
  forClause (it, &f->assumptions) {
    fprintf(p, "%ji ", *it);
  }
  fprintf(p, "\n");

  fprintf(p, "dl %ju (%" PRIu64 " X, %" PRIu64 " d, %" PRIu64 " r)\n",
          f->decisionLevel, f->numConflicts, f->numDecisions, f->numRestarts);

  if (f->conflict_clause != NO_CLS)
    fprintf(p, "cc %" PRIuPTR "\n", f->conflict_clause);
  if (f->uipClause.size > 0)
    fprintf(p, "uipc "), fs_clause_print(f, p, &f->uipClause), fprintf(p, "\n");

  fprintf(p, "trail (%" PRIu32 "): ", f->trail.size);
  for (i = 0; i < f->trail.size; i++) {
    fprintf(p, "%2ji", f->trail.data[i]);
    if (f->decisions.data[fs_lit2var(f->trail.data[i])] != 0) {
      fprintf(p, "@@%ju", f->decisions.data[fs_lit2var(f->trail.data[i])]);
    }
    if (!head_tail_is_empty(&f->unit_facts[fs_lit2var(f->trail.data[i])])) {
      fprintf(p, "*");
    }
#if 0
    if (!clauseIsAlone(&f->jail.data[fs_lit2var(f->trail.data[i])], 0)) {
      fprintf(p, "!");
    }
#endif
    fprintf(p, " ");
  }
  fprintf(p, "\n");

  fprintf(p, "model: ");
  for (i = 1; i <= f->numVars; i++) {
    if (levelOf(f, i) != Unassigned) {
      fprintf(p, "%3ji@@%ji ", f->trail.data[f->model.data[i]], levelOf(f, i));
    }
  }
  fprintf(p, "\n");
  fprintf(p, "pq: %ju (-> %ji)\n", f->propq, f->trail.data[f->propq]);
}

@


@c
void funcsatPrintConfig(FILE *stream, funcsat *f)
{
  funcsat_config *conf = f->conf;
  if (NULL != conf->user)
    fprintf(stream, "Has user data\n");
  if (NULL != conf->name)
    fprintf(stream, "Name: %s\n", conf->name);

  conf->usePhaseSaving ? 
    fprintf(stream, "phsv\t") : 
    fprintf(stream, "NO phsv\t");

  conf->useSelfSubsumingResolution ? 
    fprintf(stream, "ssr\t") : 
    fprintf(stream, "NO ssr\t");

  conf->minimizeLearnedClauses ?
    fprintf(stream, "min\t") : 
    fprintf(stream, "NO min\t");

  // TODO MAL check
  if (true == conf->gc) {
    if (LBD_sweep == conf->sweepClauses) {
      fprintf(stream, "gc glucose\t");
    } else if (claActivitySweep == conf->sweepClauses) {
      fprintf(stream, "gc minisat\t");
    } else {
      abort(); /* impossible */
    }
  } else {
    fprintf(stream, "NO gc\t\t");
    assert(NULL == conf->sweepClauses);
  }

  if (funcsatLubyRestart == conf->isTimeToRestart) {
    fprintf(stream, "restart luby\t");
  } else if (funcsatNoRestart == conf->isTimeToRestart) {
    fprintf(stream, "restart none\t");
  } else if (funcsatInnerOuter == conf->isTimeToRestart) {
    fprintf(stream, "restart inout\t");
  } else if (funcsatMinisatRestart == conf->isTimeToRestart) {
    fprintf(stream, "restart minisat\t");
  } else {
    abort(); /* impossible */
  }

  fprintf(stream, "learn %" PRIu32 " uips\t\t", conf->numUipsToLearn);

  // TODO MAL this is how to print?
  fprintf(stream, "Jail up to %" PRIuMAX " uips\n", 
          conf->maxJailDecisionLevel);
#if 0
  if (funcsatIsResourceLimitHit == conf->isResourceLimitHit) {
    fprintf(stream, "  resource hit default\n");
  } else {
    abort();
  }

  if (funcsatPreprocessNewClause == conf->preprocessNewClause) {
    fprintf(stream, "  UNUSED preprocess new clause default\n");
  } else {
    abort();
  }

  if (funcsatPreprocessBeforeSolve == conf->preprocessBeforeSolve) {
    fprintf(stream, "  UNUSED preprocess before solve default\n");
  } else {
    abort();
  }

  if (funcsatDefaultStaticActivity == conf->getInitialActivity) {
    fprintf(stream, "  initial activity static (default)\n");
  } else {
    abort();
  }
#endif
}

void funcsatPrintCnf(FILE *stream, funcsat *f, bool learned)
{
  fprintf(stream, "c clauses: %ju original", funcsatNumClauses(f));
  if (learned) {
    fprintf(stream, ", %ju learned", f->learned_clauses->size);
  }
  fprintf(stream, "\n");

  uintmax_t num_assumptions = 0;
  for (uintmax_t i = 0; i < f->assumptions.size; i++) {
    if(f->assumptions.data[i] != 0)
      num_assumptions++;
  }

  fprintf(stream, "c %ju assumptions\n", num_assumptions);
  uintmax_t numClauses = funcsatNumClauses(f) +
    (learned ? f->learned_clauses->size : 0) + num_assumptions;
  fprintf(stream, "p cnf %ju %ju\n", funcsatNumVars(f), numClauses);
  for (uintmax_t i = 0; i < f->assumptions.size; i++) {
    if(f->assumptions.data[i] != 0)
      fprintf(stream, "%ji 0\n", f->assumptions.data[i]);
  }
  vec_clause_head_print_dimacs(f, stream, f->orig_clauses);
  if (learned) {
    fprintf(stream, "c learned\n");
    vec_clause_head_print_dimacs(f, stream, f->learned_clauses);
  }
}

void funcsatClearStats(funcsat *f)
{
  f->numSweeps              = 0;
  f->numLearnedDeleted      = 0;
  f->numLiteralsDeleted     = 0;
  f->numProps               = 0;
  f->numUnitFactProps       = 0;
  f->numConflicts           = 0;
  f->numRestarts            = 0;
  f->numDecisions           = 0;
  f->numSubsumptions        = 0;
  f->numSubsumedOrigClauses = 0;
  f->numSubsumptionUips     = 0;
}

char *funcsatResultAsString(funcsat_result result)
{
  switch (result) {
  case FS_UNKNOWN: return "UNKNOWN";
  case FS_SAT: return "SATISFIABLE";
  case FS_UNSAT: return "UNSATISFIABLE";
  default: abort();             /* impossible */
  }
}


bool isUnitClause(funcsat *f, clause *c)
{
  variable i, numUnknowns = 0, numFalse = 0;
  for (i = 0; i < c->size; i++) {
    if (funcsatValue(f, c->data[i]) == unknown) {
      numUnknowns++;
    } else if (funcsatValue(f, c->data[i]) == false) {
      numFalse++;
    }
  }
  return numUnknowns==1 && numFalse==c->size-(uintmax_t)1;
}


int varOrderCompare(fibkey *a, fibkey *b)
{
  fibkey k = *a, l = *b;
  if (k > l) {
    return -1;
  } else if (k < l) {
    return 1;
  } else {
    return 0;
  }
}



clause *funcsatRemoveClause(funcsat *f, clause *c) { return NULL; }
#if 0
{
  assert(c->isLearnt);
  if (c->isReason) return NULL;

  if (c->is_watched) {
    /* remove \& release from the two watchers we had */
    clause **w0 = (clause **) &f->watches.data[fs_lit2idx(-c->data[0])];
    clause **w1 = (clause **) &f->watches.data[fs_lit2idx(-c->data[1])];
    clauseUnSpliceWatch(w0, c, 0);
    clauseUnSpliceWatch(w1, c, 1);
  } else {
    /* clause is in unit facts or jailed */
    clause *copy = c;
    clauseUnSplice(&copy, 0);
  }

  vector *clauses;
  if (c->isLearnt) {
    clauses = &f->learnedClauses;
  } else {
    clauses = &f->origClauses;
  }
  clauses->size--;
  return c;
}
#endif


extern int DebugSolverLoop;

static char *dups(const char *str)
{
  char *res;
  FS_CALLOC(res, strlen(str)+1, sizeof(*str));
  strcpy(res, str);
  return res;
}

bool funcsatDebug(funcsat *f, char *label, int level)
{
#ifdef FUNCSAT_LOG
  int *levelp;
  FS_MALLOC(levelp, 1, sizeof(*levelp));
  *levelp = level;
  hashtable_insert(f->conf->logSyms, dups(label), levelp);
  return true;
#else
  return false;
#endif
}



@
@<Internal decl...@>=
extern mbool funcsatValue(funcsat *f, literal p);


@

@<External declarations@>=
struct funcsat_config;
funcsat *funcsatInit(funcsat_config *conf);

funcsat_config *funcsatConfigInit(void *userData);

void funcsatConfigDestroy(funcsat_config *);

void funcsatResize(funcsat *f, variable numVars);

void funcsatDestroy(funcsat *);

funcsat_result funcsatPushAssumption(funcsat *f, literal p);

funcsat_result funcsatPushAssumptions(funcsat *f, clause *c);

void funcsatPopAssumptions(funcsat *f, uintmax_t num);

funcsat_result funcsatSolve(funcsat *func);

uintmax_t funcsatNumClauses(funcsat *func);

uintmax_t funcsatNumVars(funcsat *func);

void funcsatPrintStats(FILE *stream, funcsat *f);

void funcsatPrintColumnStats(FILE *stream, funcsat *f);

void funcsatClearStats(funcsat *f);

void funcsatBumpLitPriority(funcsat *f, literal p);

void funcsatPrintCnf(FILE *stream, funcsat *f, bool learned);

funcsat_result funcsatResult(funcsat *f);

clause *funcsatSolToClause(funcsat *f);

intmax_t funcsatSolCount(funcsat *f, clause subset, clause *lastSolution);

void funcsatReset(funcsat *f);

void funcsatSetupActivityGc(funcsat_config *);

bool funcsatDebug(funcsat *f, char *label, int level);

clause *funcsatRemoveClause(funcsat *f, clause *c);

@* Parsing CNF.

@<Global def...@>=
#include "funcsat/vec_char.h"
int64_t readHeader(int (*getChar)(void *), void *, funcsat *func);
funcsat_result readClauses(
  int (*getChar)(void *), void *, funcsat *func, uint64_t numClauses);

int fgetcWrapper(void *stream)
{
  return fgetc((FILE *) stream);
}

funcsat_result parseDimacsCnf(const char *path, funcsat *f)
{
  struct stat buf;
  if (-1 == stat(path, &buf)) perror("stat"), exit(1);
  if (!S_ISREG(buf.st_mode)) {
    fprintf(stderr, "Error: '%s' not a regular file\n", path);
    exit(1);
  }
  int (*getChar)(void *);
  void *stream;
  const char *opener;
  funcsat_result result;
  bool isGz = 0 == strcmp(".gz", path + (strlen(path)-strlen(".gz")));
  if (isGz) {
#ifdef HAVE_LIBZ
#  if 0
  fprintf(stderr, "c found .gz file\n");
#  endif
    getChar = (int(*)(void *))gzgetc;
    stream = gzopen(path, "r");
    opener = "gzopen";
#else
    fprintf(stderr, "cannot read gzip'd file\n");
    exit(1);
#endif
  } else {
    getChar = fgetcWrapper;
    stream = fopen(path, "r");
    opener = "fopen";
  }
  if (!stream) {
    perror(opener);
    exit(1);
  }

  uint64_t num_clauses = (uint64_t)readHeader(getChar, stream, f);
  fslog(f, "solve", 2, "read %" PRIu64 " clauses from header\n", num_clauses);
  result = readClauses(getChar, stream, f, num_clauses);

  if (isGz) {
#ifdef HAVE_LIBZ
    if (Z_OK != gzclose(stream)) perror("gzclose"), exit(1);
#else
    assert(0 && "no libz and this shouldn't happen");
#endif
  } else {
    if (0 != fclose(stream)) perror("fclose"), exit(1);
  }
  return result;
}


static literal readLiteral(
  int (*getChar)(void *stream),
  void *stream,
  struct vec_char *tmp,
  uint64_t numClauses);
int64_t readHeader(int (*getChar)(void *stream), void *stream, funcsat *func)
{
  char c;
Comments:
  while (isspace(c = getChar(stream))); /* skip leading spaces */
  if ('c' == c) {
    while ('\n' != (c = getChar(stream)));
  }

  if ('p' != c) {
    goto Comments;
  }
  while ('c' != (c = getChar(stream)));
  assert(c == 'c');
  c = getChar(stream);
  assert(c == 'n');
  c = getChar(stream);
  assert(c == 'f');

  struct vec_char *tmp = vec_char_init(4);
  readLiteral(getChar, stream, tmp, 0);
  uintmax_t numClauses = (uintmax_t) readLiteral(getChar, stream, tmp, 0);
  vec_char_destroy(tmp);
#if 0
  fprintf(stderr, "c read header 'p cnf %ju %ju'\n", numVariables, numClauses);
#endif
  return (intmax_t) numClauses;
}


funcsat_result readClauses(
  int (*getChar)(void *stream),
  void *stream,
  funcsat *func,
  uint64_t numClauses)
{
  clause *clause;
  funcsat_result result = FS_UNKNOWN;
  struct vec_char *tmp = vec_char_init(4);
  if (numClauses > 0) {
    do {
      clause = clauseAlloc(5);

      literal literal = readLiteral(getChar, stream, tmp, numClauses);

      while (literal != 0) {
        clausePush(clause, literal);
        literal = readLiteral(getChar, stream, tmp, numClauses);
      }
#if 0
      clause_head_print_dimacs(stderr, clause);
      fprintf(stderr, "\n");
#endif
      if (FS_UNSAT == funcsatAddClause(func, clause)) {
        result = FS_UNSAT;
        clauseFree(clause);
        break;
      } else
        clauseFree(clause);
    } while (--numClauses > 0);
  }
  vec_char_destroy(tmp);
  return result;
}

static literal readLiteral(
  int (*getChar)(void *stream),
  void *stream,
  struct vec_char *tmp,
  uint64_t numClauses)
{
  char c;
  bool begun;
  vec_char_clear(tmp);
  literal literal;
  begun = false;
  while (1) {
    c = getChar(stream);
    if (isspace(c) || EOF == c) {
      if (begun) {
        break;
      } else if (EOF == c) {
        fprintf(stderr, "readLiteral error: too few clauses (after %"
                PRIu64 " clauses)\n", numClauses);
        exit(1);
      } else {
        continue;
      }
    }
    begun = true;
    vec_char_push(tmp, c);
  }
  vec_char_push(tmp, '\0');
  literal = strtol((char *) tmp->data, NULL, 10);
  return literal;
}

@
@<External decl...@>=
funcsat_result parseDimacsCnf(const char *path, funcsat *f);


@* Debugging support.

@<External decl...@>=
char *funcsatResultAsString(funcsat_result result);


@ We want to print the variable interaction graph. I think that's what it's
called.

@<External decl...@>=
void fs_vig_print(funcsat *f, const char *path);

@

@<Global def...@>=
void fs_vig_print(funcsat *f, const char *path)
{
  FILE *dot;
  char buf[256];
  if (NULL == (dot = fopen(path, "w"))) perror("fopen"), exit(1);
  fprintf(dot, "graph G {\n");

  fprintf(dot, "// variables\n");
  for (variable i = 1; i <= f->numVars; i++) {
    fprintf(dot, "%ju;\n", i);
  }

  fprintf(dot, "// clauses\n");
  uintmax_t cnt = 0;
  for_vec_uintptr (ix_cl, f->orig_clauses) {
    struct clause_iter it;
    char *bufptr = buf;
    clause_iter_init(f, *ix_cl, &it);
    for (literal *p = clause_iter_next(f, &it); p; p = clause_iter_next(f, &it)) {
      bufptr += sprintf(bufptr, "%ji ", *p);
    }
    fprintf(dot, "clause%ju [shape=box,label=\"%s\",fillcolor=black,shape=box];\n",
            cnt, buf);
    clause_iter_init(f, *ix_cl, &it);
    for (literal *p = clause_iter_next(f, &it); p; p = clause_iter_next(f, &it)) {
      fprintf(dot, "clause%ju -- %ju [%s];\n",
              cnt,
              fs_lit2var(*p),
              (*p < 0 ? "color=red" : "color=green"));
    }
    cnt++;
  }
  
  fprintf(dot, "}\n");
  if (0 != fclose(dot)) perror("fclose");

}


@ The \funcsat\ configuration.
@<External ty...@>=
typedef struct funcsat funcsat;
typedef struct funcsat_config
{
  void *user; /* clients can use this */

  char *name; /* Name of the currently running sat problem.  (May be null) */


  struct hashtable *logSyms;   /*  for logging */
  struct vec_uintmax logStack;
  bool printLogLabel;
  FILE *debugStream;

  bool usePhaseSaving; /* whether to save variable phases */

  bool useSelfSubsumingResolution;

  bool minimizeLearnedClauses;

  uint32_t numUipsToLearn;

  
  bool gc; /* If false, all learned clauses are kept for all time. */

  uintmax_t maxJailDecisionLevel;  /*
   Only jail clauses that occur at or below this decision level.
   */

  bool (*isTimeToRestart)(funcsat *, void *);  /*
   This function is called after a conflict is discovered and analyzed.  If it
   function returns |true|, \funcsat\ backtracks to decision level 0 and
   recommences.
   */


  bool (*isResourceLimitHit)(funcsat *, void *);   /*
   This function is called after any learned clause is added but before
   beginning the next step of unit propagation.  If it returns |true|, \funcsat\
   solver terminates whether the problem has been solved or not.
   */


  funcsat_result (*preprocessNewClause)(funcsat *, void *, clause *);  /*
   UNUSED
   */


  funcsat_result (*preprocessBeforeSolve)(funcsat *, void *);  /*
   UNUSED
   */


  double (*getInitialActivity)(variable *);  /*
   Gets the initial activity for the given variable.
   */


  void (*bumpOriginal)(funcsat *, uintptr_t ix_cl);  /*
   Called on original clauses.
   */



  void (*bumpReason)(funcsat *, uintptr_t);  /*
   Called on clauses that are resolved on during learning.
   */


  void (*bumpLearned)(funcsat *, uintptr_t);  /*
   Called on the new UIP clause after the conflict is analyzed.
   */


  void (*bumpUnitClause)(funcsat *, uintptr_t);  /*
   Called when a clause implies a unit literal.
   */


  void (*decayAfterConflict)(funcsat *f);  /*
   Called after a conflict is analyzed.
   */


  void (*sweepClauses)(funcsat *, void *);  /*
   Implements a policy for deleting learned clauses immediately after a
   restart.
   */

} funcsat_config;

/* |uint64_t funcsatDebugSet;| in main.c*/

/* Descriptions for these options can be found in |debugDescriptions[Option]| */

@ When debugging \funcsat, it is convenient to be able to minimize a test
case. Armin Biere has been kind enough to write {\sc cnfdd}, which is a
delta-debugging algorithm for CNF files. The macro |assertExit| lets one quickly
change a failing |assert| into one with exits with a particular code. So when
\funcsat\ fails a particular |assert(x)|, change it to |assertExit(2, x)| and
run {\sc cnfdd} to produce a minimal test case.

@<Conditional mac...@>=

#ifndef NDEBUG
#define assertExit(code, cond) \
  ((cond) ? (void)0 : exit(code))
#else
#define assertExit(code, cond)
#endif

@ The following are functions used for logging messages to the console (if
|FUNCSAT_LOG| is enabled).

@d fs_dbgout(f) (f)->conf->debugStream

@<Conditional mac...@>=
#ifdef FUNCSAT_LOG
#ifndef SWIG
DECLARE_HASHTABLE(fsLogMapInsert, fsLogMapSearch, fsLogMapRemove, char, int)
#endif

/*#define fs_ifdbg(f, label, level) if (true)*/

#define fs_ifdbg(f, label, level)                                        \
  if ((f)->conf->logSyms &&                                             \
      fsLogMapSearch(f->conf->logSyms, (void *) (label)) &&             \
      (level) <= *fsLogMapSearch((f)->conf->logSyms, (void *) (label)))


#else

/*#define fs_ifdbg(f, label, level) if (true)*/
#define fs_ifdbg(f, label, level) if (false)

#endif

static inline int fslog(const struct funcsat *, const char *label,
                        int level, const char *format, ...);
static inline int dopen(const struct funcsat *, const char *label);
static inline int dclose(const struct funcsat *, const char *label);

@

@<Global def...@>=
DEFINE_HASHTABLE(fsLogMapInsert, fsLogMapSearch, fsLogMapRemove, char, int)

int fslog(const funcsat *f, const char *label, int msgLevel, const char *format, ...)
{
  int pr = 0;
  int *logLevel;
  va_list ap;

  if (f->conf->logSyms && (logLevel = hashtable_search(f->conf->logSyms, (void *) label))) {
    if (msgLevel <= *logLevel) {
      uintmax_t indent = vec_uintmax_peek(&f->conf->logStack), i;
      for (i = 0; i < indent; i++) fprintf(f->conf->debugStream, " ");
      if (f->conf->printLogLabel) pr += fprintf(f->conf->debugStream, "%s %d: ", label, msgLevel);
      va_start(ap, format);
      pr += vfprintf(f->conf->debugStream, format, ap);
      va_end(ap);
    }
  }
  return pr;
}

int dopen(const funcsat *f, const char *label)
{
  if (f->conf->logSyms && hashtable_search(f->conf->logSyms, (void *) label)) {
    uintmax_t indent = vec_uintmax_peek(&f->conf->logStack)+2;
    vec_uintmax_push(&f->conf->logStack, indent);
  }
  return 0;
}
int dclose(const funcsat *f, const char *label)
{
  if (f->conf->logSyms && hashtable_search(f->conf->logSyms, (void *) label)) {
    vec_uintmax_pop(&f->conf->logStack);
  }
  return 0;
}







@
@<Global def...@>=


/**
 * Searches in the clause for the literal with the maximum decision level,
 * beginning at startIdx. If found, that literal is swapped with
 * reason[swapIdx]. This preserves the necessary invariant for the unit facts
 * list.
 */
static void swapInMaxLevelLit(funcsat *f, clause *reason, uintmax_t swapIdx, uintmax_t startIdx)
{
  literal secondWatch = 0, swLevel = -1;
  /* find max level lit, swap with lit[1] */
  for (variable i = startIdx; i < reason->size; i++) {
    literal lev = levelOf(f, fs_lit2var(reason->data[i]));
    fslog(f, "subsumption", 9, "level of %ji = %ju\n",
          reason->data[i], levelOf(f, fs_lit2var(reason->data[i])));
    if (swLevel < lev) swLevel = lev, secondWatch = (literal) i;
    /* TODO Does this speed stuff up? */
    if (lev == fs_var2lit(f->decisionLevel)) break;
  }
  if (swLevel != -1) {
    literal tmp = reason->data[swapIdx];
    reason->data[swapIdx] = reason->data[secondWatch], reason->data[secondWatch] = tmp;
  }
}

/* this function must be called after the resolution operation */
static void checkSubsumption(
  funcsat *f, literal p, clause *learn, clause *reason, bool learnIsUip)
{
#if 0
  /* This is the test suggested in the paper "On-the-fly clause improvement" in
   * SAT09. */
  if (learn->size == reason->size-1 &&
      reason->size > 1 &&       /* learn subsumes reason */
      learn->size > 2) {

    ++f->numSubsumptions;

    fs_ifdbg (f, "subsumption", 1) {
      FILE *out = f->conf->debugStream;
      clause_head_print_dimacs(f, out, learn);
      fprintf(out, " subsumes ");
      clause_head_print_dimacs(f, out, reason);
      fprintf(out, "\n");
    }

    assert(!reason->isReason);

    if (learnIsUip) vectorPush(&f->subsumed, (void *)1);
    else vectorPush(&f->subsumed, (void *)0);
    vectorPush(&f->subsumed, reason);
  }
#endif
}

static void minimizeClauseMinisat1(funcsat *f, clause *learned)
{
  uintmax_t i, j;
  for (i = j = 1; i < learned->size; i++) {
    variable x = fs_lit2var(learned->data[i]);

    if (getReason(f, (literal)x) == NO_CLS) learned->data[j++] = learned->data[i];
    else {
      uintptr_t r = getReason(f, learned->data[i]);
      struct clause_iter it;
      clause_iter_init(f, r, &it);
      clause_iter_next(f, &it);
      for (literal *p = clause_iter_next(f, &it); p; p = clause_iter_next(f, &it)) {
#if 0
        if (!seen[var(c[k])] && level(var(c[k])) > 0) {
          learned[j++] = learned[i];
          break;
        }
#endif
      }
    }
  }
  
}



/* utilities */
/************************************************************************/

static int compareByActivityRev(const void *cl1, const void *cl2)
{
  clause *c1 = *(clause **) cl1;
  clause *c2 = *(clause **) cl2;
  double s1 = c1->activity, s2 = c2->activity;
  if (s1 == s2) return 0;
  else if (s1 > s2) return -1;
  else return 1;
}



@


@d otherWatchIdx(watchIdx) ((watchIdx) == 0 ? 1 : 0)
   /*Gets the index of the other watch given the index of the current watch.*/
@d forEachWatchedClause(c, p, wi, nx, end, foundEnd)               
    if (c)                                                              
      for (wi = (c->data[0] == p ? 0 : 1), end = c->prev[wi], nx = c->next[wi], foundEnd = false; 
           !foundEnd;                                                   
           foundEnd = (c == end), c = nx, wi = (c->data[0] == p ? 0 : 1), nx = c->next[wi])
@d forEachClause(c, nx, end, foundEnd)                             
  if (c)                                                                
    for (end = c->prev[0], nx = c->next[0], foundEnd = false;           
         !foundEnd;                                                     
         foundEnd = (c == end), c = nx, nx = c->next[0])






@<Global def...@>=
unsigned int fsLitHash(void *lIn)
{
  /* TODO get better hash function */
  literal l = *(literal *) lIn;
  return (unsigned int) l;
}
int fsLitEq(void *a, void *b) {
  literal p = *(literal *) a, q = *(literal *) b;
  return p == q;
}
unsigned int fsVarHash(void *lIn)
{
  /* TODO get better hash function */
  literal l = *(literal *) lIn;
  return (unsigned int) fs_lit2var(l);
}
int fsVarEq(void *a, void *b) {
  literal p = *(literal *) a, q = *(literal *) b;
  return fs_lit2var(p) == fs_lit2var(q);
}



int litCompare(const void *l1, const void *l2)
{
  literal x = *(literal *) l1, y = *(literal *) l2;
  if (fs_lit2var(x) != fs_lit2var(y)) {
    return fs_lit2var(x) < fs_lit2var(y) ? -1 : 1;
  } else {
    if (x == y) {
      return 0;
    } else {
      return x < y ? -1 : 1;
    }
  }
}


#if 0
int clauseCompare(const void *cp1, const void *cp2)
{
  const clause *c = *(clause **) cp1, *d = *(clause **) cp2;
  if (c->size != d->size) {
    return c->size < d->size ? -1 : 1;
  } else {
    /* lexicographically compare */
    uint32_t i;
    for (i = 0; i < c->size; i++) {
      int ret;
      if (0 != (ret = litCompare(&c->data[i], &d->data[i]))) {
        return ret;
      }
    }
    return 0;
  }
}
#endif


void sortClause(clause *clause)
{
  qsort(clause->data, clause->size, sizeof(*clause->data), litCompare);
}


literal findLiteral(literal p, clause *clause)
{
  literal min = 0, max = clause->size-1, mid = -1;
  int res = -1;                 /* comparison */
  while (!(res == 0 || min > max)) {
    mid = min + ((max-min) / 2);
    res = litCompare(&p, &clause->data[mid]);
    if (res > 0) {
      min = mid + 1;
    } else {
      max = mid - 1;
    }
  } 
  return res == 0 ? mid : -1;
}

literal findVariable(variable v, clause *clause)
{
  literal min = 0, max = clause->size-1, mid = -1;
  int res = -1;                 /* comparison */
  while (!(res == 0 || min > max)) {
    mid = min + ((max-min) / 2);
    if (v == fs_lit2var(clause->data[mid])) {
      res = 0;
    } else {
      literal p = fs_var2lit(v);
      res = litCompare(&p, &clause->data[mid]);
    }
    if (res > 0) {
      min = mid + 1;
    } else {
      max = mid - 1;
    }
  } 
  return res == 0 ? mid : -1;
}



/* Low level clause manipulations. */
clause *clauseAlloc(uint32_t capacity)
{
  clause *c;
  FS_MALLOC(c, 1, sizeof(*c));
  clauseInit(c, capacity);
  return c;
}



@

@<Global def...@>=
/**
 * Generates a clause that is 
 *   1) satisfied if a given literal violates the current assignment and
 *   2) falsified only under the current assignment.
 */
clause *funcsatSolToClause(funcsat *f) {
  clause *c = clauseAlloc(f->trail.size);
  for(uintmax_t i = 0; i < f->trail.size; i++)
    clausePush(c, -f->trail.data[i]);
  return c;
}

funcsat_result funcsatFindAnotherSolution(funcsat *f) {
  clause *cur_sol = funcsatSolToClause(f);
  funcsat_result res = funcsatAddClause(f, cur_sol);
  if(res == FS_UNSAT) return FS_UNSAT;
  res = funcsatSolve(f);
  return res;  
}

intmax_t funcsatSolCount(funcsat *f, clause subset, clause *lastSolution)
{
  assert(f->assumptions.size == 0);
  intmax_t count = 0;
  for (uintmax_t i = 0; i < subset.size; i++) {
    funcsatResize(f, fs_lit2var(subset.data[i]));
  }
 
  clause assumptions;
  clauseInit(&assumptions, subset.size);

  uintmax_t twopn = (uintmax_t) round(pow(2., (double)subset.size));
  fslog(f, "countsol", 1, "%ju incremental problems to solve\n", twopn);
  for (uintmax_t i = 0; i < twopn; i++) {
    fslog(f, "countsol", 2, "%ju: ", i);
    clauseClear(&assumptions);
    clauseCopy(&assumptions, &subset);

    /* negate literals that are 0 in n. */
    uintmax_t n = i;
    for (uintmax_t j = 0; j < subset.size; j++) {
        /* 0 bit */
      if ((n % 2) == 0) assumptions.data[j] *= -1;
      n >>= 1;
    }

    if (funcsatPushAssumptions(f, &assumptions) == FS_UNSAT) {
      continue;
    }

    if (FS_SAT == funcsatSolve(f)) {
      count++;
      if (lastSolution) {
        clauseClear(lastSolution);
        clauseCopy(lastSolution, &f->trail);
      }
    }
    
    funcsatPopAssumptions(f, f->assumptions.size);
  }

  clauseDestroy(&assumptions);

  return count;
}


@*1 Rest.

@<Internal decl...@>=

static void finishSolving(funcsat *func);
static bool bcpAndJail(funcsat *f);


bool funcsatLubyRestart(funcsat *f, void *p);
bool funcsatNoRestart(funcsat *, void *);
bool funcsatInnerOuter(funcsat *f, void *p);
bool funcsatMinisatRestart(funcsat *f, void *p);

/**
 * Undoes the given set of unit assumptions.  Assumes the decision level is 0.
 */
void undoAssumptions(funcsat *func, clause *assumptions);

funcsat_result startSolving(funcsat *f);

/**
 * Analyses conflict, produces learned clauses, backtracks, and asserts the
 * learned clauses.  If returns false, this means the SAT problem is unsat; if
 * it returns true, it means the SAT problem is not known to be unsat.
 */
bool analyze_conflict(funcsat *func);

/**
 * Undoes the trail and assignments so that the new decision level is
 * ::newDecisionLevel.  The lowest decision level is 0.
 *
 * @@param func
 * @@param newDecisionLevel
 * @@param isRestart
 * @@param facts same as in ::trailPop
 */
void backtrack(funcsat *func, variable newDecisionLevel, head_tail *facts, bool isRestart);




/* Mutators */

void fs_print_state(funcsat *, FILE *);
void funcsatPrintConfig(FILE *f, funcsat *);


bool funcsatIsResourceLimitHit(funcsat *, void *);
funcsat_result funcsatPreprocessNewClause(funcsat *, void *, clause *);
funcsat_result funcsatPreprocessBeforeSolve(funcsat *, void *);
variable funcsatLearnClauses(funcsat *, void *);

int varOrderCompare(fibkey *, fibkey *);
double funcsatDefaultStaticActivity(variable *v);

void singlesPrint(FILE *stream, clause *begin);


bool watcherFind(clause *c, clause **watches, uint8_t w);
void watcherPrint(FILE *stream, clause *c, uint8_t w);
void singlesPrint(FILE *stream, clause *begin);
void binWatcherPrint(FILE *stream, funcsat *f);


/**
 * FOR DEBUGGING
 */
bool isUnitClause(funcsat *f, clause *c);



/**
 * Returns the decision level of the given variable.
 */
literal levelOf(funcsat *f, variable v);

extern variable fs_lit2var(literal l);
/**
 * Returns the positive literal of the given variable.
 */
literal fs_var2lit(variable v);
/**
 * Converts lit to int suitable for array indexing */
uintmax_t fs_lit2idx(literal l);

extern bool isDecision(funcsat *, variable);





/* sorted clause manipulation */

/**
 * Sorts the literals in a clause.  Useful for performing clause operations
 * (::findLiteral, ::findVariable, ::clauseRemove, and
 * resolution) in logarithmic time in the size of the clause.
 */
void sortClause(clause *c);
/**
 * Finds a literal using binary search on the given clause.  Returns the
 * literal's index (0 .. clause->size-1) if found, -1 otherwise.
 */
literal findLiteral(literal l, clause *);
/**
 * Same as ::findLiteral but works on variables.
 */
literal findVariable(variable l, clause *);

unsigned int fsLitHash(void *);
int fsLitEq(void *, void *);
int litCompare(const void *l1, const void *l2);



@

@<Global def...@>=

extern mbool funcsatValue(funcsat *f, literal p)
{
  variable v = fs_lit2var(p);                         
  if (f->level.data[v] == Unassigned) return unknown; 
  literal value = f->trail.data[f->model.data[v]];    
  return p == value;
}

@ Prototype.
@<External decl...@>=
funcsat_result fs_parse_dimacs_solution(funcsat *f, FILE *solutionFile);


@ Parse solution.
@<Global def...@>=

static char parse_read_char(funcsat *f, FILE *solutionFile) {
  char c;
  c = fgetc(solutionFile);
#if 0
  bf_log(b, "bf", 8, "Read character '%c'\n", c);
#endif
  return c;
}

static const char *s_SATISFIABLE = "SATISFIABLE\n";
static const char *s_UNSATISFIABLE = "UNSATISFIABLE\n";

funcsat_result fs_parse_dimacs_solution(funcsat *f, FILE *solutionFile)
{
  char c;
  literal var;
  bool truth;
  funcsat_result result = FS_UNKNOWN;
  bool have_var = false;
  const char *cur;
state_new_line:
  while (true) {
    c = parse_read_char(f, solutionFile);
    switch (c) {
      case EOF:
        goto state_eof;
      case 'c':
        goto state_comment;
      case 's':
        goto state_satisfiablility;
      case 'v':
        goto state_variables;
      default:
        fslog(f, "fs", 1, "unknown line type '%c' in solution file\n", c);
        goto state_error;
    }
  }
state_comment:
  while (true) {
    c = parse_read_char(f, solutionFile);
    switch (c) {
      case EOF:
        goto state_eof;
      case '\n':
        goto state_new_line;
      default:
        break;
    }
  }
state_satisfiablility:
  cur = NULL;
  funcsat_result pending = FS_UNKNOWN;
  while (true) {
    c = parse_read_char(f, solutionFile);
    switch (c) {
      case ' ':
      case '\t':
        continue;
      case EOF:
        goto state_eof;
      default:
        ungetc(c, solutionFile);
        break;
    }
    break;
  }
  while (true) {
    c = parse_read_char(f, solutionFile);
    if (cur == NULL) {
      switch (c) {
        case 'S':
          cur = &s_SATISFIABLE[1];
          pending = FS_SAT;
          break;
        case 'U':
          cur = &s_UNSATISFIABLE[1];
          pending = FS_UNSAT;
          break;
        case EOF:
          goto state_eof;
        default:
          fslog(f, "fs", 1, "unknown satisfiability\n");
          goto state_error;
      }
    } else {
      if (c == EOF) {
        goto state_eof;
      } else if (c != *cur) {
        fslog(f, "fs", 1, "reading satisfiability, got '%c', expected '%c'\n", c, *cur);
        goto state_error;
      }
      if (c == '\n') {
        result = pending;
        goto state_new_line;
      }
      ++cur;
    }
  }
state_variables:
  while (true) {
    c = parse_read_char(f, solutionFile);
    switch (c) {
      case '\n':
        goto state_new_line;
      case ' ':
      case '\t':
        break;
      case EOF:
        goto state_eof;
      default:
        ungetc(c, solutionFile);
        goto state_sign;
    }
  }
state_sign:
  have_var = false;
  truth = true;
  c = parse_read_char(f, solutionFile);
  if (c == '-') {
    truth = false;
  } else {
    ungetc(c, solutionFile);
  }
  var = 0;
  goto state_variable;
state_variable:
  while (true) {
    c = parse_read_char(f, solutionFile);
    switch (c) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        have_var = true;
        var = var * 10 + (c - '0');
        break;
      default:
        ungetc(c, solutionFile);
        goto state_record_variable;
    }
  }
state_record_variable:
  if (have_var) {
    if (var == 0) {
      /* sometimes the SAT solver will be told ``there are 47000 variables but
       * I'm only going to use 100 of them in the instance.'' {\bf some solvers}
       * (lingeling) will say ``ah well i can get rid of 46000 variables! i have
       * been so helpful today!'' which is all well and good except that then it
       * outputs the solution on only the 100 of them that are used. in this
       * case, we log what happened and hope that giving an arbitrary assignment
       * (false) to the rest of the inputs is the right thing. */

      /* we assume this doesn't happen */

      goto state_exit;
    } else {
      variable v = fs_lit2var(var);
      funcsatPushAssumption(f, (truth ? (literal)v : -(literal)v));
    }
    goto state_variables;
  } else {
    abort();
    fslog(f, "fs", 1, "expected variable, but didn't get one\n");
    goto state_error;
  }
state_eof:
  if (ferror(solutionFile)) {
    abort();
    fslog(f, "fs", 1, "IO error reading solution file: %s", strerror(errno));
    goto state_error;
  } else {
    goto state_exit;
  }
state_error:
  result = FS_UNKNOWN;
  goto state_exit;
state_exit:
  return result;
}
@
@(funcsat_test.c@>=
#include "funcsat.h"
