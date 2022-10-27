/*12:*/
#line 306 "./funcsat.w"

#ifndef funcsat_internal_h_included
#define funcsat_internal_h_included
#include "funcsat/vec_ptr.h"
#include "funcsat/vec_intmax.h"
#include <funcsat/fibheap.h> 

/*235:*/
#line 5852 "./funcsat.w"


#ifndef NDEBUG
#define assertExit(code, cond) \
  ((cond) ? (void)0 : exit(code))
#else
#define assertExit(code, cond)
#endif

/*:235*//*236:*/
#line 5866 "./funcsat.w"

#ifdef FUNCSAT_LOG
#ifndef SWIG
DECLARE_HASHTABLE(fsLogMapInsert,fsLogMapSearch,fsLogMapRemove,char,int)
#endif



#define fs_ifdbg(f, label, level)                                        \
  if ((f)->conf->logSyms &&                                             \
      fsLogMapSearch(f->conf->logSyms, (void *) (label)) &&             \
      (level) <= *fsLogMapSearch((f)->conf->logSyms, (void *) (label)))


#else


#define fs_ifdbg(f, label, level) if (false)

#endif

static inline int fslog(const struct funcsat*,const char*label,
int level,const char*format,...);
static inline int dopen(const struct funcsat*,const char*label);
static inline int dclose(const struct funcsat*,const char*label);

/*:236*/
#line 313 "./funcsat.w"

/*38:*/
#line 1189 "./funcsat.w"

#ifndef CLAUSE_HEAD_SIZE
#  define CLAUSE_HEAD_SIZE 7    
#endif

#ifndef CLAUSE_BLOCK_SIZE
#  define CLAUSE_BLOCK_SIZE 7   
#endif

enum clause_state
{
CLAUSE_WATCHED= 1,
CLAUSE_JAILED,
CLAUSE_UNIT
};

#define NO_CLS UINTPTR_MAX
#define LBD_SCORE_MAX 0x3f

struct clause_head
{
literal lits[CLAUSE_HEAD_SIZE];
uintptr_t nx;
uintptr_t link;

uint32_t is_learned:1;
uint32_t is_reason:1;
uint32_t where:2;
uint32_t lbd_score:6;
uint32_t sz:22;
float activity;

};
#include "funcsat/vec_clause_head.h"

struct clause_block
{
literal lits[CLAUSE_BLOCK_SIZE];
uintptr_t nx;
};
#include "funcsat/vec_clause_block.h"

/*:38*//*39:*/
#line 1234 "./funcsat.w"

struct clause_head_pool
{
uintptr_t freelist;
struct vec_clause_head*heads;
};
struct clause_block_pool
{
uintptr_t freelist;
struct vec_clause_block*blocks;
};

/*:39*//*47:*/
#line 1419 "./funcsat.w"

struct clause_iter
{
int16_t l_ix;
bool is_head;
uint32_t sz;
union
{
struct clause_head*h;
struct clause_block*b;
};
};

/*:47*//*53:*/
#line 1552 "./funcsat.w"

typedef struct head_tail
{
uintptr_t hd;
uintptr_t tl;
}head_tail;

/*:53*//*64:*/
#line 1775 "./funcsat.w"

struct watchlist_elt
{
literal lit;
uintptr_t cls;
};


/*:64*//*65:*/
#line 1792 "./funcsat.w"

#ifndef WATCHLIST_HEAD_SIZE_MAX
#  define WATCHLIST_HEAD_SIZE_MAX 4
#endif

struct watchlist
{
uint32_t size;
uint32_t capacity;

struct watchlist_elt elts[WATCHLIST_HEAD_SIZE_MAX];
struct watchlist_elt*rest;
};

/*:65*//*67:*/
#line 1820 "./funcsat.w"

typedef struct all_watches
{
struct watchlist*wlist;
uintmax_t size;
uintmax_t capacity;
}all_watches;

/*:67*//*95:*/
#line 2324 "./funcsat.w"

struct reason_info
{
uintptr_t ty;
uintptr_t cls;
};
#include "funcsat/vec_reason_info.h"


/*:95*//*117:*/
#line 2739 "./funcsat.w"

struct bh_node
{
variable var;
double act;
};

/*:117*//*172:*/
#line 3903 "./funcsat.w"

struct litpos
{
clause*c;
struct vec_uintptr*indices;
};

/*:172*/
#line 314 "./funcsat.w"

/*21:*/
#line 507 "./funcsat.w"

struct funcsat_config;
struct clause_head;
struct clause_block;

struct funcsat
{
struct funcsat_config*conf;


uint8_t(*Backtrack_hook)(uintptr_t*_SM,uintmax_t level);
uint8_t(*BCP_hook)(uintptr_t*_SM,uintmax_t nVariable,uint8_t bPolarity);
void(*MakeDecision_hook)(uintptr_t*_SM);
intmax_t(*ExternalHeuristic_hook)(uintptr_t*_SM);

clause assumptions;

funcsat_result lastResult;

uintmax_t decisionLevel;

uintmax_t propq;




clause trail;

struct vec_uintmax model;


clause phase;


clause level;


struct vec_uintmax decisions;



struct vec_uintptr*reason;




struct vec_ptr reason_hooks;



struct vec_reason_info*reason_infos;


uintptr_t reason_infos_freelist;

head_tail*unit_facts;

uintmax_t unit_facts_size;
uintmax_t unit_facts_capacity;

head_tail*jail;



all_watches watches;


struct vec_uintptr*orig_clauses;
struct vec_uintptr*learned_clauses;
uintmax_t numVars;

struct clause_head_pool clheads;
struct clause_block_pool clblocks;

uintptr_t conflict_clause;



struct litpos litpos_uip;
clause uipClause;

struct vec_ptr subsumed;



uintmax_t LBD_count;
struct vec_uint64 LBD_levels;
struct vec_uint64 LBD_histogram;
uint64_t LBD_last_num_conflicts;
uint64_t LBD_base;
uint64_t LBD_increment;


double claDecay;
double claInc;
double learnedSizeFactor;




double maxLearned;
double learnedSizeAdjustConfl;
uint64_t learnedSizeAdjustCnt;
uint64_t learnedSizeAdjustInc;
double learnedSizeInc;



struct vec_uintmax seen;
struct vec_intmax analyseToClear;
struct vec_uintmax analyseStack;
struct vec_uintmax allLevels;




double varInc;
double varDecay;

struct bh_node*binvar_heap;
uintmax_t binvar_heap_size;
uintmax_t*binvar_pos;


int64_t lrestart;
uint64_t lubycnt;
uint64_t lubymaxdelta;
bool waslubymaxdelta;


struct drand48_data*rand;



uint64_t numSolves;


uint64_t numLearnedClauses;


uint64_t numSweeps;


uint64_t numLearnedDeleted;


uint64_t numLiteralsDeleted;



uint64_t numProps;
uint64_t numUnitFactProps;

uint64_t numJails;


uint64_t numConflicts;

uint64_t numResolutions;


uint64_t numRestarts;


uint64_t numDecisions;


uint64_t numSubsumptions;



uint64_t numSubsumedOrigClauses;


uint64_t numSubsumptionUips;
};


/*:21*/
#line 315 "./funcsat.w"

/*25:*/
#line 792 "./funcsat.w"

static inline uintptr_t clause_block_from_clause(funcsat*f,clause*c,
struct clause_head*cl,
intmax_t*ix_clause);

/*:25*//*28:*/
#line 834 "./funcsat.w"

static inline funcsat_result addClause(funcsat*f,uintptr_t ix_cl);
/*:28*//*37:*/
#line 1148 "./funcsat.w"

void fs_clause_print(funcsat*f,FILE*out,clause*c);
void dimacsPrintClause(FILE*out,clause*c);


/*:37*//*43:*/
#line 1328 "./funcsat.w"

static inline struct clause_head*clause_head_ptr(funcsat*f,uintptr_t i)
{
return(assert((i)<(f)->clheads.heads->capacity),(f)->clheads.heads->data+(i));
}
static inline struct clause_block*clause_block_ptr(funcsat*f,uintptr_t i)
{
return(assert((i)<(f)->clblocks.blocks->capacity),(f)->clblocks.blocks->data+(i));
}


/*:43*//*50:*/
#line 1523 "./funcsat.w"

static inline uintptr_t clause_head_mk(funcsat*f,uint32_t sz_hint);
static inline uintptr_t clause_block_mk(funcsat*f);
static inline void clause_head_release(funcsat*f,uintptr_t ix);
static inline void clause_block_release(funcsat*f,uintptr_t ix);
static inline void clause_releas(funcsat*f,uintptr_t ix);
void clause_iter_init(funcsat*f,uintptr_t ix_clause_head,struct clause_iter*it);
literal*clause_iter_next(funcsat*f,struct clause_iter*it);
void fs_clause_head_print(funcsat*f,FILE*out,uintptr_t ix);
void clause_head_print_dimacs(funcsat*f,FILE*out,uintptr_t);
void vec_clause_head_print_dimacs(funcsat*f,FILE*out,struct vec_uintptr*);

/*:50*//*57:*/
#line 1626 "./funcsat.w"

static inline void head_tail_clear(head_tail*ht);
static inline bool head_tail_is_empty(head_tail*ht);
static inline void head_tail_append(funcsat*f,head_tail*ht1,head_tail*ht2);
static inline void head_tail_add(funcsat*f,head_tail*ht1,uintptr_t);


/*:57*//*60:*/
#line 1688 "./funcsat.w"

extern void head_tail_print(funcsat*f,FILE*out,head_tail*l);


/*:60*//*61:*/
#line 1694 "./funcsat.w"

bool bcp(funcsat*f);

/*:61*//*70:*/
#line 1850 "./funcsat.w"

static inline bool watchlist_is_elt_in_head(struct watchlist_elt*elt,struct watchlist*wl)
{
return(elt>=wl->elts&&elt<(wl)->elts+WATCHLIST_HEAD_SIZE_MAX);
}
static inline uint32_t watchlist_head_size(struct watchlist*wl)
{
return((wl)->size> WATCHLIST_HEAD_SIZE_MAX?WATCHLIST_HEAD_SIZE_MAX:(wl)->size);
}
static inline struct watchlist_elt*watchlist_head_size_ptr(struct watchlist*wl)
{
return((wl)->elts+watchlist_head_size(wl));
}
static inline uint32_t watchlist_rest_size(struct watchlist*wl)
{
return((wl)->size> WATCHLIST_HEAD_SIZE_MAX?(wl)->size-WATCHLIST_HEAD_SIZE_MAX:0);
}
static inline struct watchlist_elt*watchlist_rest_size_ptr(struct watchlist*wl)
{
return((wl)->rest+watchlist_rest_size(wl));
}


/*:70*//*71:*/
#line 1875 "./funcsat.w"

static inline void all_watches_init(funcsat*f);
static inline void all_watches_destroy(funcsat*f);

/*:71*//*76:*/
#line 1933 "./funcsat.w"

static int compare_pointer(const void*x,const void*y);


/*:76*//*79:*/
#line 2012 "./funcsat.w"

static inline void watchlist_check(funcsat*f,literal l);
static inline void watches_check(funcsat*f);


/*:79*//*88:*/
#line 2198 "./funcsat.w"

static inline void watch_l0(funcsat*f,uintptr_t ix);
static inline void watch_l1(funcsat*f,uintptr_t ix);

/*:88*//*92:*/
#line 2271 "./funcsat.w"

void addWatch(funcsat*f,uintptr_t ix);
static inline void addWatchUnchecked(funcsat*f,uintptr_t ix);
void makeWatchable(funcsat*f,uintptr_t ix);
static inline bool is_watchable(funcsat*f,struct clause_head*c);

/*:92*//*97:*/
#line 2355 "./funcsat.w"

static inline uintptr_t getReason(funcsat*f,literal l);
/*:97*//*110:*/
#line 2526 "./funcsat.w"


void trailPush(funcsat*f,literal p,uintptr_t reason_info_idx);
literal trailPop(funcsat*f,head_tail*facts);
static inline literal trailPeek(funcsat*f);
static inline uintptr_t reason_info_mk(funcsat*f,uintptr_t ix);

/*:110*//*113:*/
#line 2665 "./funcsat.w"


literal funcsatMakeDecision(funcsat*,void*);


/*:113*//*116:*/
#line 2712 "./funcsat.w"

static inline int activity_compare(double x,double y);


/*:116*//*122:*/
#line 2820 "./funcsat.w"


static inline uintmax_t bh_var2pos(funcsat*f,variable v);
static inline bool bh_is_in_heap(funcsat*f,variable v);
static inline bool bh_node_is_in_heap(funcsat*f,struct bh_node*);
static inline double*bh_var2act(funcsat*f,variable v);
static inline struct bh_node*bh_top(funcsat*f);
static inline struct bh_node*bh_bottom(funcsat*f);
static inline bool bh_is_top(funcsat*f,struct bh_node*v);
static inline struct bh_node*bh_left(funcsat*f,struct bh_node*v);
static inline struct bh_node*bh_right(funcsat*f,struct bh_node*v);
static inline struct bh_node*bh_parent(funcsat*f,struct bh_node*v);
static inline uintmax_t bh_size(funcsat*f);
static inline variable bh_pop(funcsat*f);
static inline void bh_insert(funcsat*f,variable v);


/*:122*//*126:*/
#line 2925 "./funcsat.w"

static inline void bh_increase_activity(funcsat*f,uintmax_t node_pos,double new_act);

/*:126*//*128:*/
#line 2955 "./funcsat.w"

static void bh_check(funcsat*f);

/*:128*//*134:*/
#line 3027 "./funcsat.w"

static void bh_print(funcsat*f,const char*path,struct bh_node*r);

/*:134*//*136:*/
#line 3084 "./funcsat.w"

static void restore_facts(funcsat*f,head_tail*facts);


/*:136*//*145:*/
#line 3344 "./funcsat.w"

static inline uintmax_t pop_and_resolve(funcsat*f,uintptr_t ix_reason,
head_tail*facts);

/*:145*//*150:*/
#line 3461 "./funcsat.w"

static inline bool isAssumption(funcsat*f,variable v);

/*:150*//*159:*/
#line 3629 "./funcsat.w"

static void print_dot_impl_graph(funcsat*f,clause*cc);

/*:159*//*161:*/
#line 3712 "./funcsat.w"

#include "funcsat/vec_bool.h"


/*:161*//*162:*/
#line 3719 "./funcsat.w"

bool find_uips(funcsat*f,uintmax_t c,head_tail*facts,literal*uipLit);
bool propagateFacts(funcsat*f,head_tail*facts,literal uipLit);

static uintmax_t resetLevelCount(funcsat*f,uintmax_t c,head_tail*facts);

static void checkSubsumption(
funcsat*f,
literal p,clause*learn,clause*reason,
bool learnIsUip);

/*:162*//*167:*/
#line 3865 "./funcsat.w"

static inline mbool tentativeValue(funcsat*f,literal p);


/*:167*//*171:*/
#line 3893 "./funcsat.w"

void minimizeUip(funcsat*f,clause*learned);

/*:171*//*183:*/
#line 4011 "./funcsat.w"

static inline void litpos_init(struct litpos*);
static inline void litpos_destroy(struct litpos*lp);
static inline void litpos_init_from_uip_clause(funcsat*f);
static inline void litpos_set(struct litpos*lp,literal*p);
static inline void litpos_unset(struct litpos*lp,literal p);
static inline literal*litpos_lit(struct litpos*pos,literal l);
static inline uintptr_t litpos_pos(struct litpos*lp,literal l);
static inline bool litpos_contains(struct litpos*pos,literal l);
static inline void litpos_clear(struct litpos*lp);
extern void litpos_check(struct litpos*lp);

/*:183*//*204:*/
#line 4318 "./funcsat.w"

static int compareByActivityRev(const void*cl1,const void*cl2);
static int LBD_compare_rev(void*f,const void*cl1,const void*cl2);
void LBD_bump_reason(funcsat*f,uintptr_t c);
void LBD_bump_learned(funcsat*f,uintptr_t C);
static inline uint8_t LBD_compute_score(funcsat*f,uintptr_t c);
void LBD_decay_after_conflict(funcsat*f);
void LBD_sweep(funcsat*f,void*user);


/*:204*//*211:*/
#line 4604 "./funcsat.w"

void myDecayAfterConflict(funcsat*f);

/*:211*//*214:*/
#line 4678 "./funcsat.w"

void bumpOriginal(funcsat*f,uintptr_t c);
void bumpReasonByActivity(funcsat*f,uintptr_t c);
void bumpLearnedByActivity(funcsat*f,uintptr_t c);
void bumpUnitClauseByActivity(funcsat*f,uintptr_t c);
void lbdBumpActivity(funcsat*f,uintptr_t c);
static inline void varDecayActivity(funcsat*f);
static void claDecayActivity(funcsat*f);
static void bumpClauseByActivity(funcsat*f,uintptr_t c);
extern void varBumpScore(funcsat*f,variable v);


/*:214*//*215:*/
#line 4691 "./funcsat.w"

void claActivitySweep(funcsat*f,void*user);

/*:215*//*227:*/
#line 5469 "./funcsat.w"

extern mbool funcsatValue(funcsat*f,literal p);


/*:227*//*241:*/
#line 6247 "./funcsat.w"


static void finishSolving(funcsat*func);
static bool bcpAndJail(funcsat*f);


bool funcsatLubyRestart(funcsat*f,void*p);
bool funcsatNoRestart(funcsat*,void*);
bool funcsatInnerOuter(funcsat*f,void*p);
bool funcsatMinisatRestart(funcsat*f,void*p);




void undoAssumptions(funcsat*func,clause*assumptions);

funcsat_result startSolving(funcsat*f);






bool analyze_conflict(funcsat*func);










void backtrack(funcsat*func,variable newDecisionLevel,head_tail*facts,bool isRestart);






void fs_print_state(funcsat*,FILE*);
void funcsatPrintConfig(FILE*f,funcsat*);


bool funcsatIsResourceLimitHit(funcsat*,void*);
funcsat_result funcsatPreprocessNewClause(funcsat*,void*,clause*);
funcsat_result funcsatPreprocessBeforeSolve(funcsat*,void*);
variable funcsatLearnClauses(funcsat*,void*);

int varOrderCompare(fibkey*,fibkey*);
double funcsatDefaultStaticActivity(variable*v);

void singlesPrint(FILE*stream,clause*begin);


bool watcherFind(clause*c,clause**watches,uint8_t w);
void watcherPrint(FILE*stream,clause*c,uint8_t w);
void singlesPrint(FILE*stream,clause*begin);
void binWatcherPrint(FILE*stream,funcsat*f);





bool isUnitClause(funcsat*f,clause*c);






literal levelOf(funcsat*f,variable v);

extern variable fs_lit2var(literal l);



literal fs_var2lit(variable v);


uintmax_t fs_lit2idx(literal l);

extern bool isDecision(funcsat*,variable);












void sortClause(clause*c);




literal findLiteral(literal l,clause*);



literal findVariable(variable l,clause*);

unsigned int fsLitHash(void*);
int fsLitEq(void*,void*);
int litCompare(const void*l1,const void*l2);



/*:241*/
#line 316 "./funcsat.w"


#endif

/*:12*/
