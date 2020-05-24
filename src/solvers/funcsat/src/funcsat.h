/*11:*/
#line 287 "./funcsat.w"

#ifndef funcsat_h_included
#define funcsat_h_included
#include <stdio.h> 
#include <stdlib.h> 
#include <inttypes.h> 
#include <stdbool.h> 

#include "funcsat/vec_bool.h"
#include "funcsat/vec_uint64.h"

/*13:*/
#line 322 "./funcsat.w"

#include "funcsat/system.h"
#include "funcsat/vec_uintmax.h"
#include "funcsat/hashtable.h"


/*:13*//*17:*/
#line 460 "./funcsat.w"

typedef enum
{
FS_UNKNOWN= 0,
FS_SAT= 10,
FS_UNSAT= 20
}funcsat_result;

/*:17*//*32:*/
#line 945 "./funcsat.w"

typedef struct clause
{
literal*data;

uint32_t size;

uint32_t capacity;

uint32_t isLearnt:1;
uint32_t isReason:1;
uint32_t is_watched:1;

double activity;

struct clause*nx;
}clause;

/*:32*//*94:*/
#line 2312 "./funcsat.w"

enum reason_ty
{
REASON_CLS_TY
};

/*:94*//*234:*/
#line 5750 "./funcsat.w"

typedef struct funcsat funcsat;
typedef struct funcsat_config
{
void*user;

char*name;


struct hashtable*logSyms;
struct vec_uintmax logStack;
bool printLogLabel;
FILE*debugStream;

bool usePhaseSaving;

bool useSelfSubsumingResolution;

bool minimizeLearnedClauses;

uint32_t numUipsToLearn;


bool gc;

uintmax_t maxJailDecisionLevel;



bool(*isTimeToRestart)(funcsat*,void*);






bool(*isResourceLimitHit)(funcsat*,void*);






funcsat_result(*preprocessNewClause)(funcsat*,void*,clause*);




funcsat_result(*preprocessBeforeSolve)(funcsat*,void*);




double(*getInitialActivity)(variable*);




void(*bumpOriginal)(funcsat*,uintptr_t ix_cl);





void(*bumpReason)(funcsat*,uintptr_t);




void(*bumpLearned)(funcsat*,uintptr_t);




void(*bumpUnitClause)(funcsat*,uintptr_t);




void(*decayAfterConflict)(funcsat*f);




void(*sweepClauses)(funcsat*,void*);




}funcsat_config;





/*:234*/
#line 298 "./funcsat.w"


/*15:*/
#line 365 "./funcsat.w"


#define FS_CALLOC(ptr, n, size)                   \
  do {                                         \
    ptr =  calloc((n), (size));                 \
    if (!ptr) perror("FS_CALLOC"), abort();       \
  } while (0);

#define FS_MALLOC(ptr, n, size)                   \
  do {                                         \
    ptr =  malloc((n)*(size));                  \
    if (!ptr) perror("FS_MALLOC"), abort();       \
  } while (0);

#define FS_MALLOC_TY(ty, ptr, n, size)             \
  do {                                          \
    ptr =  (ty) malloc((n)*(size));              \
    if (!ptr) perror("FS_MALLOC_TY"), abort();     \
  } while (0);

#define FS_REALLOC(ptr, n, size)                                           \
  do {                                                                  \
    void *tmp_funcsat_ptr__;                                            \
    tmp_funcsat_ptr__ =  realloc(ptr, (n)*(size));                       \
    if (!tmp_funcsat_ptr__)  free(ptr), perror("FS_REALLOC"), abort();     \
    ptr =  tmp_funcsat_ptr__;                                            \
  } while (0);

#define FS_REFS_CALLOC(p,o,n,sz)                                      \
  do {                                                          \
    assert((n) >= (o));                                         \
    void *tmp_funcsat_ptr__;                                    \
    tmp_funcsat_ptr__ =  realloc(p, (n)*(sz));                   \
    if (!tmp_funcsat_ptr__) perror("FS_REFS_CALLOC"), abort();        \
    memset(tmp_funcsat_ptr__ + ((o)*(sz)), 0, ((n)-(o))*(sz));  \
    (p) =  tmp_funcsat_ptr__;                                    \
  } while (0);


/*:15*//*18:*/
#line 470 "./funcsat.w"

void funcsatCheck(funcsat*f,funcsat_result r);
/*:18*//*26:*/
#line 798 "./funcsat.w"

funcsat_result funcsatAddClause(funcsat*func,clause*clause);
extern uintptr_t clause_head_alloc_from_clause(funcsat*f,clause*c);


/*:26*//*33:*/
#line 965 "./funcsat.w"

clause*clauseAlloc(uint32_t capacity);

void clauseInit(clause*v,uint32_t capacity);

void clauseDestroy(clause*);



void clauseFree(clause*);

void clauseClear(clause*v);



void clausePush(clause*v,literal data);



void clausePushAt(clause*v,literal data,uint32_t i);
void clauseGrowTo(clause*v,uint32_t newCapacity);
literal clausePop(clause*v);
literal clausePopAt(clause*v,uint32_t i);
literal clausePeek(clause*v);

void clauseCopy(clause*dst,clause*src);




#define forClause(elt, vec) for (elt =  (vec)->data; elt != (vec)->data + (vec)->size; elt++)
#define for_clause(elt, cl) for (elt =  (cl)->data; elt != (cl)->data + (cl)->size; elt++)
#define for_clause99(elt, cl) for (literal *elt =  (cl)->data; \
     elt != (cl)->data + (cl)->size; elt++)


/*:33*//*100:*/
#line 2373 "./funcsat.w"

void funcsatAddReasonHook(funcsat*f,uintptr_t ty,
uintptr_t(*hook)(funcsat*f,literal l));

/*:100*//*206:*/
#line 4340 "./funcsat.w"

void funcsatSetupLbdGc(funcsat_config*);

/*:206*//*228:*/
#line 5475 "./funcsat.w"

struct funcsat_config;
funcsat*funcsatInit(funcsat_config*conf);

funcsat_config*funcsatConfigInit(void*userData);

void funcsatConfigDestroy(funcsat_config*);

void funcsatResize(funcsat*f,variable numVars);

void funcsatDestroy(funcsat*);

funcsat_result funcsatPushAssumption(funcsat*f,literal p);

funcsat_result funcsatPushAssumptions(funcsat*f,clause*c);

void funcsatPopAssumptions(funcsat*f,uintmax_t num);

funcsat_result funcsatSolve(funcsat*func);

uintmax_t funcsatNumClauses(funcsat*func);

uintmax_t funcsatNumVars(funcsat*func);

void funcsatPrintStats(FILE*stream,funcsat*f);

void funcsatPrintColumnStats(FILE*stream,funcsat*f);

void funcsatClearStats(funcsat*f);

void funcsatBumpLitPriority(funcsat*f,literal p);

void funcsatPrintCnf(FILE*stream,funcsat*f,bool learned);

funcsat_result funcsatResult(funcsat*f);

clause*funcsatSolToClause(funcsat*f);

intmax_t funcsatSolCount(funcsat*f,clause subset,clause*lastSolution);

void funcsatReset(funcsat*f);

void funcsatSetupActivityGc(funcsat_config*);

bool funcsatDebug(funcsat*f,char*label,int level);

clause*funcsatRemoveClause(funcsat*f,clause*c);

/*:228*//*230:*/
#line 5691 "./funcsat.w"

funcsat_result parseDimacsCnf(const char*path,funcsat*f);


/*:230*//*231:*/
#line 5697 "./funcsat.w"

char*funcsatResultAsString(funcsat_result result);


/*:231*//*232:*/
#line 5704 "./funcsat.w"

void fs_vig_print(funcsat*f,const char*path);

/*:232*//*243:*/
#line 6373 "./funcsat.w"

funcsat_result fs_parse_dimacs_solution(funcsat*f,FILE*solutionFile);


/*:243*/
#line 300 "./funcsat.w"


#endif

/*:11*/
