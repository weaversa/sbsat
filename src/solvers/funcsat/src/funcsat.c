#define swap(ty,x,y) ty _swap_tmp##ty= (x) ;(x) = (y) ,(y) = _swap_tmp##ty; \
 \

#define Unassigned (-1)  \

#define INITIAL_CLAUSE_POOL 1024 \

#define for_head_tail(f,ht,prev,curr,next)  \
for(prev= next= NO_CLS,curr= (ht) ->hd, \
((curr!=NO_CLS) ?next= clause_head_ptr(f,(curr) ) ->link:0) ; \
(curr!=NO_CLS) ; \
(clause_head_ptr(f,(curr) ) ->link!=NO_CLS?prev= (curr) :0) , \
curr= (next) ,((curr!=NO_CLS) ?next= clause_head_ptr(f,(curr) ) ->link:0) )  \

#define head_tail_iter_rm(f,ht,prev,curr,next)  \
if((ht) ->hd==(curr) ) { \
(ht) ->hd= (next) ; \
} \
if((ht) ->tl==(curr) ) { \
(ht) ->tl= (prev) ; \
} \
if(prev!=NO_CLS) { \
clause_head_ptr(f,(prev) ) ->link= (next) ; \
} \
clause_head_ptr(f,(curr) ) ->link= NO_CLS; \

#define watchlist_next_elt(elt,wl)  \
((elt) +1==(wl) ->elts+WATCHLIST_HEAD_SIZE_MAX?elt= (wl) ->rest:(elt) ++) 
#define for_watchlist(elt,dump,wl)  \
for(elt= dump= (wl) ->elts; \
(watchlist_is_elt_in_head(elt,(wl) )  \
?elt<watchlist_head_size_ptr(wl)  \
:elt<watchlist_rest_size_ptr(wl) ) ; \
watchlist_next_elt(elt,(wl) ) )  \

#define for_watchlist_continue(elt,dump,wl)  \
for(; \
(watchlist_is_elt_in_head(elt,wl)  \
?elt<watchlist_head_size_ptr(wl)  \
:elt<watchlist_rest_size_ptr(wl) ) ; \
watchlist_next_elt(elt,wl) )  \

#define NEWLINE(s) fprintf((!(s) ?stderr:(s) ) ,"\n")  \

#define reason_info_ptr(f,i) (&((f) ->reason_infos->data[i]) )  \

#define reason_info_idx(f,r) ((r) -(f) ->reason_infos->data)  \

#define NO_POS UINTPTR_MAX \

#define fs_dbgout(f) (f) ->conf->debugStream \

#define otherWatchIdx(watchIdx) ((watchIdx) ==0?1:0)  \

#define forEachWatchedClause(c,p,wi,nx,end,foundEnd)  \
if(c)  \
for(wi= (c->data[0]==p?0:1) ,end= c->prev[wi],nx= c->next[wi],foundEnd= false; \
!foundEnd; \
foundEnd= (c==end) ,c= nx,wi= (c->data[0]==p?0:1) ,nx= c->next[wi]) 
#define forEachClause(c,nx,end,foundEnd)  \
if(c)  \
for(end= c->prev[0],nx= c->next[0],foundEnd= false; \
!foundEnd; \
foundEnd= (c==end) ,c= nx,nx= c->next[0])  \
 \
 \
 \
 \
 \

/*14:*/
#line 335 "./funcsat.w"

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

/*16:*/
#line 415 "./funcsat.w"

funcsat_result funcsatSolve(funcsat*f)
{
if(FS_UNSAT==(f->lastResult= startSolving(f)))goto Done;

if(!bcpAndJail(f))goto Unsat;

while(!f->conf->isResourceLimitHit(f,f->conf->user)){
fs_ifdbg(f,"solve",3)fs_print_state(f,fs_dbgout(f));
if(!bcp(f)){
if(0==f->decisionLevel)goto Unsat;
if(!analyze_conflict(f))goto Unsat;
if(f->conf->gc)f->conf->sweepClauses(f,f->conf->user);
continue;
}

if(f->trail.size!=f->numVars&&
f->conf->isTimeToRestart(f,f->conf->user)){
fslog(f,"solve",1,"restarting\n");
++f->numRestarts;
backtrack(f,0,NULL,true);
continue;
}

if(!funcsatMakeDecision(f,f->conf->user)){
f->lastResult= FS_SAT;
goto Done;
}
}

Unsat:
f->lastResult= FS_UNSAT;

Done:
fslog(f,"solve",1,"instance is %s\n",funcsatResultAsString(f->lastResult));
assert(f->lastResult!=FS_SAT||f->trail.size==f->numVars);
finishSolving(f);
return f->lastResult;
}

/*:16*//*19:*/
#line 474 "./funcsat.w"

void funcsatCheck(funcsat*f,funcsat_result r)
{
assert(r==f->lastResult);
if(r==FS_SAT){
for_vec_uintptr(ix_cl,f->orig_clauses){
/*20:*/
#line 489 "./funcsat.w"

struct clause_iter it;
bool sat= false;
clause_iter_init(f,*ix_cl,&it);
for(literal*p= clause_iter_next(f,&it);p;p= clause_iter_next(f,&it)){
assert(funcsatValue(f,*p)!=unknown);
if(funcsatValue(f,*p)==true){
sat= true;
break;
}
}
assert(sat);


/*:20*/
#line 480 "./funcsat.w"

}
for_vec_uintptr(ix_cl,f->learned_clauses){
/*20:*/
#line 489 "./funcsat.w"

struct clause_iter it;
bool sat= false;
clause_iter_init(f,*ix_cl,&it);
for(literal*p= clause_iter_next(f,&it);p;p= clause_iter_next(f,&it)){
assert(funcsatValue(f,*p)!=unknown);
if(funcsatValue(f,*p)==true){
sat= true;
break;
}
}
assert(sat);


/*:20*/
#line 483 "./funcsat.w"

}
}
}

/*:19*//*22:*/
#line 688 "./funcsat.w"

funcsat_result funcsatAddClause(funcsat*f,clause*c)
{
variable maxVar= 0;
uintptr_t ix_cl;
struct clause_head*cl;

funcsatReset(f);

if(c->size> 1){
uintmax_t size= c->size;
literal*i,*j,*end;
sortClause(c);

for(i= j= (literal*)c->data,end= i+c->size;i!=end;i++){
literal p= *i,q= *j;
if(i!=j){
if(p==q){
size--;
continue;
}else if(p==-q){
clauseDestroy(c);
goto Done;
}else*(++j)= p;
}
}
c->size= size;
}

for(variable k= 0;k<c->size;k++){
variable v= fs_lit2var(c->data[k]);
maxVar= v> maxVar?v:maxVar;
}
funcsatResize(f,maxVar);

ix_cl= clause_head_alloc_from_clause(f,c);
cl= clause_head_ptr(f,ix_cl);

cl->is_learned= false;
cl->is_reason= false;
cl->lbd_score= LBD_SCORE_MAX;
cl->activity= 0.f;
f->conf->bumpOriginal(f,ix_cl);

fs_ifdbg(f,"solve",2){
fslog(f,"solve",2,"adding %ju ",funcsatNumClauses(f)+1);
fs_clause_print(f,fs_dbgout(f),c);
fprintf(fs_dbgout(f),"\n");
}
funcsat_result clauseResult= addClause(f,ix_cl);
if(f->lastResult!=FS_UNSAT){
f->lastResult= clauseResult;
}
vec_uintptr_push(f->orig_clauses,ix_cl);
Done:
return f->lastResult;
}

/*:22*//*23:*/
#line 749 "./funcsat.w"

inline uintptr_t clause_head_alloc_from_clause(funcsat*f,clause*c)
{
uintmax_t sz_orig= c->size;
uintptr_t ix_cl= clause_head_mk(f,c->size);
struct clause_head*cl= clause_head_ptr(f,ix_cl);
intmax_t sz_copied= 0;
for(int i= 0;i<CLAUSE_HEAD_SIZE&&sz_copied<c->size;i++){

cl->lits[i]= c->data[sz_copied++];
cl->sz++;
}
if(sz_copied<c->size){
uintptr_t ix_bl= clause_block_from_clause(f,c,cl,&sz_copied);
cl->nx= ix_bl;
while(sz_copied<c->size){
struct clause_block*bl;
uintptr_t ix_bl_nx= clause_block_from_clause(f,c,cl,&sz_copied);
bl= clause_block_ptr(f,ix_bl);
bl->nx= ix_bl= ix_bl_nx;
}
}
assert(cl->sz==sz_orig);
return ix_cl;
}

/*:23*//*24:*/
#line 777 "./funcsat.w"

static inline uintptr_t clause_block_from_clause(funcsat*f,clause*c,
struct clause_head*cl,
intmax_t*sz_copied)
{
uintptr_t ix_bl= clause_block_mk(f);
struct clause_block*bl= clause_block_ptr(f,ix_bl);
for(int i= 0;i<CLAUSE_BLOCK_SIZE&&*sz_copied<c->size;i++){
bl->lits[i]= c->data[(*sz_copied)++];
cl->sz++;
}
return ix_bl;
}

/*:24*//*27:*/
#line 807 "./funcsat.w"

static inline funcsat_result addClause(funcsat*f,uintptr_t ix_cl)
{
struct clause_head*h= clause_head_ptr(f,ix_cl);
funcsat_result result= FS_UNKNOWN;
assert(f->decisionLevel==0);
h->is_learned= false;
if(h->sz==0){
f->conflict_clause= ix_cl;
result= FS_UNSAT;
}else if(h->sz==1){
mbool val= funcsatValue(f,h->lits[0]);
if(val==false){
f->conflict_clause= ix_cl;
result= FS_UNSAT;
}else{
if(val==unknown){
trailPush(f,h->lits[0],reason_info_mk(f,ix_cl));
head_tail_add(f,&f->unit_facts[fs_lit2var(h->lits[0])],ix_cl);
}
}
}else addWatch(f,ix_cl);
return result;
}


/*:27*//*29:*/
#line 864 "./funcsat.w"

funcsat_result funcsatPushAssumption(funcsat*f,literal p)
{
if(p==0){

clausePush(&f->assumptions,0);
return FS_UNKNOWN;
}

f->conf->minimizeLearnedClauses= false;
backtrack(f,0UL,NULL,true);

funcsatResize(f,fs_lit2var(p));
if(funcsatValue(f,p)==false){
return FS_UNSAT;
}else if(funcsatValue(f,p)==unknown){

clausePush(&f->assumptions,p);
trailPush(f,p,NO_CLS);
}else{
clausePush(&f->assumptions,0);
}
return FS_UNKNOWN;
}

funcsat_result funcsatPushAssumptions(funcsat*f,clause*c){
for(uintmax_t i= 0;i<c->size;i++){
if(funcsatPushAssumption(f,c->data[i])==FS_UNSAT){
funcsatPopAssumptions(f,i);
return FS_UNSAT;
}
}
return FS_UNKNOWN;
}

/*:29*//*30:*/
#line 901 "./funcsat.w"

void funcsatPopAssumptions(funcsat*f,uintmax_t num){

head_tail facts;
head_tail_clear(&facts);

assert(num<=f->assumptions.size);

backtrack(f,0,NULL,true);

for(uintmax_t i= 0;i<num;i++){
literal p= clausePop(&f->assumptions);
if(p==0)return;

literal t= trailPop(f,&facts);

while(p!=t){
t= trailPop(f,&facts);
}
}

restore_facts(f,&facts);
}

/*:30*//*31:*/
#line 928 "./funcsat.w"


void funcsatReset(funcsat*f)
{
f->conflict_clause= NO_CLS;
backtrack(f,0UL,NULL,true);
f->propq= 0;
f->lastResult= FS_UNKNOWN;
}





/*:31*//*34:*/
#line 1002 "./funcsat.w"

void clauseInit(clause*v,uint32_t capacity)
{
uint32_t c= capacity> 0?capacity:4;
FS_CALLOC(v->data,c,sizeof(*v->data));
v->size= 0;
v->capacity= c;
v->isLearnt= false;
v->nx= NULL;
v->is_watched= false;
v->isReason= false;
v->activity= 0.f;
}

void clauseFree(clause*v)
{
clauseDestroy(v);
free(v);
}

void clauseDestroy(clause*v)
{
free(v->data);
v->data= NULL;
v->size= 0;
v->capacity= 0;
v->isLearnt= false;
}

void clauseClear(clause*v)
{
v->size= 0;
v->isLearnt= false;
}

void clausePush(clause*v,literal data)
{
if(v->capacity<=v->size){
while(v->capacity<=v->size){
v->capacity= v->capacity*2+1;
}
FS_REALLOC(v->data,v->capacity,sizeof(*v->data));
}
v->data[v->size++]= data;
}

void clausePushAt(clause*v,literal data,uint32_t i)
{
uint32_t j;
assert(i<=v->size);
if(v->capacity<=v->size){
while(v->capacity<=v->size){
v->capacity= v->capacity*2+1;
}
FS_REALLOC(v->data,v->capacity,sizeof(*v->data));
}
v->size++;
for(j= v->size-(uint32_t)1;j> i;j--){
v->data[j]= v->data[j-1];
}
v->data[i]= data;
}


void clauseGrowTo(clause*v,uint32_t newCapacity)
{
if(v->capacity<newCapacity){
v->capacity= newCapacity;
FS_REALLOC(v->data,v->capacity,sizeof(*v->data));
}
assert(v->capacity>=newCapacity);
}


literal clausePop(clause*v)
{
assert(v->size!=0);
return v->data[v->size---1];
}

literal clausePopAt(clause*v,uint32_t i)
{
uint32_t j;
assert(v->size!=0);
literal res= v->data[i];
for(j= i;j<v->size-(uint32_t)1;j++){
v->data[j]= v->data[j+1];
}
v->size--;
return res;
}

literal clausePeek(clause*v)
{
assert(v->size!=0);
if(v->size==0){
v->size= 1;
return 0;
}
return v->data[v->size-1];
}

void clauseSet(clause*v,uint32_t i,literal p)
{
v->data[i]= p;
}

void clauseCopy(clause*dst,clause*src)
{
literal i;
for(i= 0;i<src->size;i++){
clausePush(dst,src->data[i]);
}
dst->isLearnt= src->isLearnt;
}




/*:34*//*35:*/
#line 1122 "./funcsat.w"

void fs_clause_print(funcsat*f,FILE*out,clause*c)
{
if(!out)out= stderr;
literal*p;
for_clause(p,c){
fprintf(out,"%ji_%ji%s ",*p,levelOf(f,fs_lit2var(*p)),
(funcsatValue(f,*p)==true?"T":
(funcsatValue(f,*p)==false?"F":"U")));
}
}

/*:35*//*36:*/
#line 1135 "./funcsat.w"

void dimacsPrintClause(FILE*out,clause*c)
{
literal*p;
if(!out)out= stderr;
for_clause(p,c){
fprintf(out,"%ji ",*p);
}
fprintf(out,"0");
}

/*:36*//*44:*/
#line 1343 "./funcsat.w"

static inline uintptr_t clause_head_mk(funcsat*f,uint32_t sz_hint)
{
const uintptr_t cap= f->clheads.heads->capacity;
uintptr_t freelist= f->clheads.freelist;
if(freelist>=cap){

vec_clause_head_grow_to(f->clheads.heads,cap*2);
for(uintptr_t i= cap;i<f->clheads.heads->capacity;i++){
clause_head_release(f,i);
}
}
struct clause_head*h= clause_head_ptr(f,freelist= f->clheads.freelist);
f->clheads.freelist= h->nx;

memset(h,0,sizeof(*h));
h->nx= h->link= NO_CLS;
return freelist;
}

static inline uintptr_t clause_block_mk(funcsat*f)
{
const uintptr_t cap= f->clblocks.blocks->capacity;
uintptr_t freelist= f->clblocks.freelist;
if(freelist>=cap){

vec_clause_block_grow_to(f->clblocks.blocks,cap*2);
for(uintptr_t i= cap;i<f->clblocks.blocks->capacity;i++){
clause_block_release(f,i);
}
}
struct clause_block*h= clause_block_ptr(f,freelist= f->clblocks.freelist);
f->clblocks.freelist= h->nx;

memset(h,0,sizeof(*h));
h->nx= NO_CLS;
return freelist;
}


/*:44*//*45:*/
#line 1385 "./funcsat.w"

static inline void clause_head_release(funcsat*f,uintptr_t ix)
{
assert(ix<f->clheads.heads->capacity);
struct clause_head*h= clause_head_ptr(f,ix);
assert(memset(h,0,sizeof(struct clause_head)));
h->nx= f->clheads.freelist,f->clheads.freelist= ix;
}

static inline void clause_block_release(funcsat*f,uintptr_t ix)
{
assert(ix<f->clblocks.blocks->capacity);
struct clause_block*h= clause_block_ptr(f,ix);
assert(memset(h,0,sizeof(struct clause_block)));
h->nx= f->clblocks.freelist,f->clblocks.freelist= ix;
}

/*:45*//*46:*/
#line 1403 "./funcsat.w"

static inline void clause_release(funcsat*f,uintptr_t ix)
{
struct clause_head*h= clause_head_ptr(f,ix);
struct clause_block*b;
uintptr_t nx= h->nx;
clause_head_release(f,ix);
while(nx!=NO_CLS){
uintptr_t nx_nx= clause_block_ptr(f,nx)->nx;
clause_block_release(f,nx);
nx= nx_nx;
}
}

/*:46*//*48:*/
#line 1443 "./funcsat.w"

void clause_iter_init(funcsat*f,uintptr_t ix_clause_head,struct clause_iter*it)
{
struct clause_head*h= clause_head_ptr(f,ix_clause_head);
it->l_ix= -1;
it->is_head= true;
it->h= h;
it->sz= h->sz;
}

#define clause_iter_lit_ptr(f, it) ((it)->is_head ?             \
                                    (it)->h->lits+(it)->l_ix :  \
                                    (it)->b->lits+(it)->l_ix)

literal*clause_iter_next(funcsat*f,struct clause_iter*it)
{
literal*lit= NULL;
if(it->sz==0)return NULL;
++it->l_ix;

if(it->is_head){
if(it->l_ix>=CLAUSE_HEAD_SIZE){
assert(it->h->nx!=NO_CLS);
it->is_head= false;
it->b= clause_block_ptr(f,it->h->nx);
it->l_ix= 0;
}
}else{
if(it->l_ix>=CLAUSE_BLOCK_SIZE){
assert(it->b->nx!=NO_CLS);
it->b= clause_block_ptr(f,it->b->nx);
it->l_ix= 0;
}
}

lit= clause_iter_lit_ptr(f,it);
it->sz--;
return lit;
}

/*:48*//*49:*/
#line 1484 "./funcsat.w"

void fs_clause_head_print(funcsat*f,FILE*out,uintptr_t ix)
{
if(!out)out= stderr;
struct clause_iter it;
clause_iter_init(f,ix,&it);
for(literal*p= clause_iter_next(f,&it);p;p= clause_iter_next(f,&it)){
fprintf(out,"%ji_%ji%s ",*p,levelOf(f,fs_lit2var(*p)),
(funcsatValue(f,*p)==true?"T":
(funcsatValue(f,*p)==false?"F":"U")));
}
}

void clause_head_print_dimacs(funcsat*f,FILE*out,uintptr_t ix)
{
struct clause_iter it;
if(!out)out= stderr;
if(ix==NO_CLS){
fprintf(out,"(NULL CLAUSE)");
return;
}
clause_iter_init(f,ix,&it);
for(literal*p= clause_iter_next(f,&it);p;p= clause_iter_next(f,&it)){
fprintf(out,"%ji ",*p);
}
fprintf(out,"0");
}

void vec_clause_head_print_dimacs(funcsat*f,FILE*stream,struct vec_uintptr*clauses)
{
for_vec_uintptr(ix,clauses){
clause_head_print_dimacs(f,stream,*ix);
fprintf(stream,"\n");
}
}


/*:49*//*54:*/
#line 1560 "./funcsat.w"

static inline void head_tail_clear(head_tail*ht){
ht->hd= ht->tl= NO_CLS;
}

static inline bool head_tail_is_empty(head_tail*ht){
return ht->hd==NO_CLS&&ht->tl==NO_CLS;
}

/*:54*//*55:*/
#line 1585 "./funcsat.w"

static inline void head_tail_append(funcsat*f,head_tail*ht1,head_tail*ht2){
if(ht1->tl!=NO_CLS)
clause_head_ptr(f,ht1->tl)->link= ht2->hd;
if(ht2->tl!=NO_CLS){
ht1->tl= ht2->tl;
if(ht1->hd==NO_CLS)
ht1->hd= ht2->hd;
}
head_tail_clear(ht2);
#if 0
if(!head_tail_is_empty(ht2)){
if(head_tail_is_empty(ht1)){
ht1->hd= ht2->hd;
}else{
struct clause_head*cl_tl= clause_head_ptr(f,ht1->tl);
assert(ht1->hd!=NO_CLS);
assert(cl_tl->link==NO_CLS);
cl_tl->link= ht2->hd;
}
ht1->tl= ht2->tl;
}
#endif
}

/*:55*//*56:*/
#line 1611 "./funcsat.w"

static inline void head_tail_add(funcsat*f,head_tail*ht1,uintptr_t ix){
head_tail ht2;
ht2.hd= ht2.tl= ix;
head_tail_append(f,ht1,&ht2);
#if 0
struct clause_head*c= clause_head_ptr(f,ix);
c->link= ht1->hd;
ht1->hd= ix;
if(ht1->tl==NO_CLS){
ht1->tl= ix;}
#endif
}

/*:56*//*59:*/
#line 1667 "./funcsat.w"

inline void head_tail_print(funcsat*f,FILE*out,head_tail*l)
{
if(!out)out= stderr;
if(l->hd){
uintptr_t p,c,nx;
for_head_tail(f,l,p,c,nx){
struct clause_head*h= clause_head_ptr(f,c);
if(h->link==NO_CLS&&l->tl!=c){
fprintf(out,"warning: tl is not last clause\n");
}
fs_clause_head_print(f,out,c);
fprintf(out,"\n");
}
}else if(l->tl!=NO_CLS){
fprintf(out,"warning: hd unset but tl set!\n");
}
}

/*:59*//*62:*/
#line 1707 "./funcsat.w"

bool bcp(funcsat*f)
{
bool isConsistent= true;

while(f->propq<f->trail.size){
/*80:*/
#line 2020 "./funcsat.w"

literal p= f->trail.data[f->propq];
#ifndef NDEBUG
watchlist_check(f,p);
#endif
fs_ifdbg(f,"bcp",3){fslog(f,"bcp",3,"bcp on %ji\n",p);}
const literal false_lit= -p;

struct watchlist*wl= &f->watches.wlist[fs_lit2idx(p)];
struct watchlist_elt*elt,*dump;
uint32_t new_size= 0;

++f->numProps;
dopen(f,"bcp");

for_watchlist(elt,dump,wl){
struct clause_head*c= clause_head_ptr(f,elt->cls);
literal otherlit;
mbool litval;
fs_ifdbg(f,"bcp",9){
fslog(f,"bcp",9,"visit ");
fs_clause_head_print(f,fs_dbgout(f),elt->cls);
fprintf(fs_dbgout(f)," %"PRIuPTR"\n",elt->cls);
}
assert((false_lit==c->lits[0]||false_lit==c->lits[1]));
assert((c->link==NO_CLS));
/*81:*/
#line 2071 "./funcsat.w"

if(funcsatValue(f,elt->lit)==true)goto watch_continue;



/*:81*/
#line 2046 "./funcsat.w"

/*82:*/
#line 2079 "./funcsat.w"

if(c->lits[0]==false_lit){
literal tmp= c->lits[0];
elt->lit= c->lits[0]= c->lits[1],c->lits[1]= tmp;
assert(c->lits[1]==false_lit);
fs_ifdbg(f,"bcp",10){
fslog(f,"bcp",9,"swapped ");
fs_clause_head_print(f,fs_dbgout(f),elt->cls),fprintf(fs_dbgout(f),"\n");}
}
assert(c->lits[1]==false_lit);


/*:82*/
#line 2047 "./funcsat.w"

otherlit= c->lits[0];
/*83:*/
#line 2096 "./funcsat.w"

struct clause_iter it;
clause_iter_init(f,elt->cls,&it);
clause_iter_next(f,&it),clause_iter_next(f,&it);
for(literal*p= clause_iter_next(f,&it);p;p= clause_iter_next(f,&it)){
mbool v= funcsatValue(f,*p);
if(v!=false){
c->lits[1]= *p,*p= false_lit;
watch_l1(f,elt->cls);
fs_ifdbg(f,"bcp",9){
fslog(f,"bcp",9,"moved ");
fs_clause_head_print(f,fs_dbgout(f),elt->cls),NEWLINE(fs_dbgout(f));
}
elt->cls= NO_CLS;elt->lit= 0;
goto skip_watchelt_copy;
}
}

/*:83*/
#line 2049 "./funcsat.w"


litval= funcsatValue(f,otherlit);
if(litval==true)goto watch_continue;
if(litval==false){
/*84:*/
#line 2130 "./funcsat.w"

isConsistent= false;
f->conflict_clause= elt->cls;
for_watchlist_continue(elt,dump,wl){
*dump= *elt,watchlist_next_elt(dump,wl);
new_size++;
}
wl->size= new_size;
dclose(f,"bcp");
goto bcp_conflict;

/*:84*/
#line 2054 "./funcsat.w"

}else{
/*85:*/
#line 2143 "./funcsat.w"

fslog(f,"bcp",2," => %ji (%s:%d)\n",otherlit,__FILE__,__LINE__);
trailPush(f,otherlit,reason_info_mk(f,elt->cls));
f->conf->bumpUnitClause(f,elt->cls);

/*:85*/
#line 2056 "./funcsat.w"

}
watch_continue:
*dump= *elt,watchlist_next_elt(dump,wl);
new_size++;
skip_watchelt_copy:;
}
f->propq++;
wl->size= new_size;
dclose(f,"bcp");

/*:80*/
#line 1713 "./funcsat.w"


if(f->BCP_hook){
if(f->BCP_hook((uintptr_t*)f->conf->user,imaxabs(p),p> 0)==FS_UNSAT){
isConsistent= false;
goto bcp_conflict;
}
}
}

bcp_conflict:
return isConsistent;
}

/*:62*//*68:*/
#line 1830 "./funcsat.w"

static inline void all_watches_init(funcsat*f)
{
FS_CALLOC(f->watches.wlist,1<<7,sizeof(*f->watches.wlist));
f->watches.size= 2;
f->watches.capacity= 1<<7;
}

/*:68*//*69:*/
#line 1840 "./funcsat.w"

static inline void all_watches_destroy(funcsat*f)
{
free(f->watches.wlist);
}

/*:69*//*75:*/
#line 1921 "./funcsat.w"

static int compare_pointer(const void*x,const void*y)
{
uintptr_t xp= (uintptr_t)*(clause**)x;
uintptr_t yp= (uintptr_t)*(clause**)y;
if(xp<yp)return-1;
else if(xp> yp)return 1;
else return 0;
}


/*:75*//*77:*/
#line 1938 "./funcsat.w"

static inline void watchlist_check(funcsat*f,literal l)
{
literal false_lit= -l;

struct watchlist*wl= &f->watches.wlist[fs_lit2idx(l)];
struct watchlist_elt*elt,*dump;

struct vec_ptr*clauses= vec_ptr_init(wl->size);
for_watchlist(elt,dump,wl){
struct clause_head*c= clause_head_ptr(f,elt->cls);
literal*chkl;bool chk_in_cls= false;
struct clause_iter it;
vec_ptr_push(clauses,c);
clause_iter_init(f,elt->cls,&it);
for(literal*p= clause_iter_next(f,&it);p;p= clause_iter_next(f,&it)){

if(*p==elt->lit){
chk_in_cls= true;
break;
}
}
assert(chk_in_cls&&"watched lit not in clause");

assert((c->lits[0]==false_lit||c->lits[1]==false_lit)&&
"watched lit not in first 2");

uint32_t num_not_false= 0;
clause_iter_init(f,elt->cls,&it);
for(literal*p= clause_iter_next(f,&it);p;p= clause_iter_next(f,&it)){
if(tentativeValue(f,*p)!=false){
num_not_false++;
if(funcsatValue(f,*p)==true){
num_not_false= 0;
break;
}
}
}
if(num_not_false>=1){
assert((tentativeValue(f,c->lits[0])!=false||
tentativeValue(f,c->lits[1])!=false)&&
"watching bad literals");
if(num_not_false>=2){
assert(tentativeValue(f,c->lits[0])!=false&&
tentativeValue(f,c->lits[1])!=false&&
"watching bad literals");
}
}
}

qsort(clauses->data,clauses->size,sizeof(struct clause_head*),compare_pointer);

for(uintmax_t i= 0,j= 1;j<clauses->size;i++,j++){

assert(clauses->data[i]!=clauses->data[j]&&"duplicate clause");
}
vec_ptr_destroy(clauses);
}



/*:77*//*78:*/
#line 2000 "./funcsat.w"

static inline void watches_check(funcsat*f)
{
for(variable v= 1;v<=f->numVars;v++){
literal pos= (literal)v;
literal neg= -(literal)v;
watchlist_check(f,pos);
watchlist_check(f,neg);
}
}

/*:78*//*86:*/
#line 2150 "./funcsat.w"

static inline struct watchlist_elt*watch_lit(funcsat*f,struct watchlist*wl,
uintptr_t ix)
{
struct watchlist_elt*ret;
if(watchlist_head_size(wl)<WATCHLIST_HEAD_SIZE_MAX){
assert(watchlist_rest_size(wl)==0);
ret= &wl->elts[watchlist_head_size(wl)];
wl->elts[watchlist_head_size(wl)].cls= ix;
}else{
assert(watchlist_head_size(wl)>=WATCHLIST_HEAD_SIZE_MAX);
/*87:*/
#line 2185 "./funcsat.w"

if(wl->capacity> 0){
if(watchlist_rest_size(wl)>=wl->capacity){
FS_REALLOC(wl->rest,wl->capacity*2,sizeof(*wl->rest));
wl->capacity*= 2;
}
}else{
FS_CALLOC(wl->rest,8,sizeof(*wl->rest));
wl->capacity= 8;
}

/*:87*/
#line 2161 "./funcsat.w"

ret= &wl->rest[watchlist_rest_size(wl)];
wl->rest[watchlist_rest_size(wl)].cls= ix;
}
wl->size++;
return ret;
}
static inline void watch_l0(funcsat*f,uintptr_t ix)
{
struct clause_head*c= clause_head_ptr(f,ix);
struct watchlist*wl= &f->watches.wlist[fs_lit2idx(-c->lits[0])];
struct watchlist_elt*elt= watch_lit(f,wl,ix);
elt->lit= (c->sz==2?c->lits[1]:c->lits[0]);
}
static inline void watch_l1(funcsat*f,uintptr_t ix)
{
struct clause_head*c= clause_head_ptr(f,ix);
struct watchlist*wl= &f->watches.wlist[fs_lit2idx(-c->lits[1])];
struct watchlist_elt*elt= watch_lit(f,wl,ix);
elt->lit= (c->sz==2?c->lits[1]:c->lits[0]);
}

/*:86*//*89:*/
#line 2208 "./funcsat.w"

void addWatch(funcsat*f,uintptr_t ix)
{
makeWatchable(f,ix);
addWatchUnchecked(f,ix);
}

static inline void addWatchUnchecked(funcsat*f,uintptr_t ix)
{
struct clause_head*c= clause_head_ptr(f,ix);
fslog(f,"bcp",1,"watching %li and %li in ",c->lits[0],c->lits[1]);
fs_ifdbg(f,"bcp",1){fs_clause_head_print(f,fs_dbgout(f),ix),NEWLINE(fs_dbgout(f));}
assert(c->sz> 1);
watch_l0(f,ix);
watch_l1(f,ix);
c->where= CLAUSE_WATCHED;
}

/*:89*//*90:*/
#line 2230 "./funcsat.w"

void makeWatchable(funcsat*f,uintptr_t ix)
{
struct clause_head*c= clause_head_ptr(f,ix);
struct clause_iter pit,qit;
clause_iter_init(f,ix,&pit),clause_iter_init(f,ix,&qit);
variable swap_cnt= 0;
for(literal*p= clause_iter_next(f,&pit),*q= clause_iter_next(f,&qit);
p&&q&&swap_cnt<2;
p= clause_iter_next(f,&pit)){
mbool v= funcsatValue(f,*p);
if(v!=false&&p!=q){
literal tmp= *q;
*q= *p,*p= tmp;
swap_cnt++;
q= clause_iter_next(f,&qit);
}
}
#if 0
assert(is_watchable(f,c));
#endif
}
/*:90*//*91:*/
#line 2262 "./funcsat.w"

static inline bool is_watchable(funcsat*f,struct clause_head*c)
{
if(c->sz<2)return false;
return funcsatValue(f,c->lits[0])!=false&&
funcsatValue(f,c->lits[1])!=false;
}
/*:91*//*93:*/
#line 2279 "./funcsat.w"

static void fs_watches_print(funcsat*f,FILE*out,literal p)
{
if(!out)out= stderr;
fprintf(out,"watcher list for %ji:\n",p);
struct watchlist*wl= &f->watches.wlist[fs_lit2idx(p)];
struct watchlist_elt*elt,*dump;
for_watchlist(elt,dump,wl){
struct clause_head*c= clause_head_ptr(f,elt->cls);
if(c){
fprintf(out,"[%ji",elt->lit);
bool in_clause= false;
struct clause_iter it;
clause_iter_init(f,elt->cls,&it);
for(literal*q= clause_iter_next(f,&it);q;q= clause_iter_next(f,&it)){
if(*q==elt->lit){
in_clause= true;break;
}
}
if(!in_clause){
fprintf(out," oh noes!");
}
fprintf(out,"] ");
fs_clause_head_print(f,out,elt->cls),NEWLINE(out);
}else{
fprintf(out,"[EMPTY SLOT]\n");
}
}
}


/*:93*//*96:*/
#line 2339 "./funcsat.w"

static inline uintptr_t getReason(funcsat*f,literal l)
{
uintptr_t reason_idx= f->reason->data[fs_lit2var(l)];
if(reason_idx!=NO_CLS){
struct reason_info*r= reason_info_ptr(f,reason_idx);
if(r->ty==REASON_CLS_TY)return r->cls;
uintptr_t(*get_reason_clause)(funcsat*,literal)= f->reason_hooks.data[r->ty];
r->ty= REASON_CLS_TY;
return r->cls= get_reason_clause(f,l);
}else
return NO_CLS;
}


/*:96*//*99:*/
#line 2365 "./funcsat.w"

void funcsatAddReasonHook(funcsat*f,uintptr_t ty,
uintptr_t(*hook)(funcsat*f,literal l))
{
vec_ptr_push(&f->reason_hooks,hook);
}

/*:99*//*101:*/
#line 2383 "./funcsat.w"

static inline uintptr_t reason_info_mk(funcsat*f,uintptr_t ix)
{
assert(f->reason_infos_freelist<f->reason_infos->size);
uintptr_t ret= f->reason_infos_freelist;
struct reason_info*r= reason_info_ptr(f,ret);
f->reason_infos_freelist= r->ty;

r->ty= REASON_CLS_TY;
r->cls= ix;
return ret;
}

/*:101*//*102:*/
#line 2401 "./funcsat.w"

static inline void reason_info_release(funcsat*f,uintptr_t ri)
{
struct reason_info*r= reason_info_ptr(f,ri);
r->ty= f->reason_infos_freelist;
f->reason_infos_freelist= ri;
}

/*:102*//*107:*/
#line 2439 "./funcsat.w"

void trailPush(funcsat*f,literal p,uintptr_t reason_info_idx)
{
variable v= fs_lit2var(p);
assert(v<=f->numVars);
#ifndef NDEBUG
if(f->model.data[v]<f->trail.size){
assert(f->trail.data[f->model.data[v]]!=p);
}
#endif
clausePush(&f->trail,p);
f->model.data[v]= f->trail.size-(uintmax_t)1;
f->phase.data[v]= p;
f->level.data[v]= (literal)f->decisionLevel;

if(reason_info_idx!=NO_CLS){
struct reason_info*r= reason_info_ptr(f,reason_info_idx);
if(r->ty==REASON_CLS_TY){
struct clause_head*h= clause_head_ptr(f,r->cls);
h->is_reason= true;
}
f->reason->data[v]= reason_info_idx;
}
}

/*:107*//*108:*/
#line 2471 "./funcsat.w"

literal trailPop(funcsat*f,head_tail*facts)
{
literal p= clausePeek(&f->trail);
variable v= fs_lit2var(p);
uintptr_t reason_ix;
if(facts&&!head_tail_is_empty(&f->unit_facts[v]))
head_tail_append(f,facts,&f->unit_facts[v]);

#if 0
exonerateClauses(f,v);
#endif

if(f->decisions.data[v]!=0){

if(f->Backtrack_hook)f->Backtrack_hook((uintptr_t*)f->conf->user,f->decisionLevel);
f->decisionLevel--;
f->decisions.data[v]= 0;
}


f->level.data[v]= Unassigned;
reason_ix= f->reason->data[v];
if(reason_ix!=NO_CLS){
struct reason_info*r= reason_info_ptr(f,reason_ix);
if(r->ty==REASON_CLS_TY){
struct clause_head*h= clause_head_ptr(f,r->cls);
h->is_reason= false;
}
reason_info_release(f,reason_ix);
f->reason->data[v]= NO_CLS;
}

if(!bh_is_in_heap(f,v)){
#ifndef NDEBUG
bh_check(f);
#endif
bh_insert(f,v);
}
clausePop(&f->trail);
if(f->propq>=f->trail.size)
f->propq= f->trail.size;
return p;
}

/*:108*//*109:*/
#line 2518 "./funcsat.w"

static inline literal trailPeek(funcsat*f)
{
literal p= clausePeek(&f->trail);
return p;
}

/*:109*//*111:*/
#line 2546 "./funcsat.w"

static void jailClause(funcsat*f,literal trueLit,clause*c)
{
#if 0
++f->numJails;
dopen(f,"jail");
assert(funcsatValue(f,trueLit)==true);
const variable trueVar= fs_lit2var(trueLit);
dmsg(f,"jail",7,false,"jailed for %ju: ",trueVar);
clauseUnSpliceWatch((clause**)&watches->data[fs_lit2idx(-c->data[0])],c,0);
clause*cell= &f->jail.data[trueVar];
clauseSplice1(c,&cell);
dclose(f,"jail");
assert(!c->is_watched);
#endif
}

static void exonerateClauses(funcsat*f,variable v)
{
uintptr_t p,c,nx;
for_head_tail(f,&f->jail[v],p,c,nx){
struct clause_head*h= clause_head_ptr(f,c);
h->link= NO_CLS;
addWatchUnchecked(f,c);
}
head_tail tmp;
head_tail_clear(&tmp);
memcpy(&f->jail[v],&tmp,sizeof(tmp));
}

/*:111*//*112:*/
#line 2622 "./funcsat.w"

literal funcsatMakeDecision(funcsat*f,void*p)
{
literal l= 0;

if(f->ExternalHeuristic_hook!=NULL){
l= f->ExternalHeuristic_hook((uintptr_t*)f->conf->user);
variable v= fs_lit2var(l);
++f->numDecisions;
trailPush(f,l,NO_CLS);
f->level.data[v]= (intmax_t)++f->decisionLevel;
f->decisions.data[v]= f->decisionLevel;
fslog(f,"solve",2,"branched on %ji\n",l);
}else{
while(bh_size(f)> 0){
fslog(f,"decide",5,"extracting\n");
variable v= bh_pop(f);
#ifndef NDEBUG
bh_check(f);
#endif
fslog(f,"decide",5,"extracted %ju\n",v);
literal p= -fs_var2lit(v);
if(funcsatValue(f,p)==unknown){
if(f->conf->usePhaseSaving)l= f->phase.data[v];
else l= p;
++f->numDecisions;
trailPush(f,l,NO_CLS);
f->level.data[v]= (intmax_t)++f->decisionLevel;
f->decisions.data[v]= f->decisionLevel;
fslog(f,"solve",2,"branched on %ji\n",l);
break;
}
}
assert(l!=0||bh_size(f)==0);
}


if(f->MakeDecision_hook)f->MakeDecision_hook((uintptr_t*)f->conf->user);

return l;
}

/*:112*//*114:*/
#line 2674 "./funcsat.w"

inline void varBumpScore(funcsat*f,variable v)
{
double*activity_v= bh_var2act(f,v);
double origActivity,activity;
origActivity= activity= *activity_v;
if((activity+= f->varInc)> 1e100){
for(variable j= 1;j<=f->numVars;j++){
double*m= bh_var2act(f,j);
fslog(f,"decide",5,"old activity %f, rescaling\n",*m);
*m*= 1e-100;
}
double oldVarInc= f->varInc;
f->varInc*= 1e-100;
fslog(f,"decide",1,"setting varInc from %f to %f\n",oldVarInc,f->varInc);
activity*= 1e-100;
}
if(bh_is_in_heap(f,v)){
bh_increase_activity(f,v,activity);
}else{
*activity_v= activity;
}
fslog(f,"decide",5,"bumped %ju from %.30f to %.30f\n",v,origActivity,*activity_v);
}

/*:114*//*115:*/
#line 2703 "./funcsat.w"

static inline int activity_compare(double x,double y)
{
if(x> y)return 1;
else if(x<y)return-1;
else return 0;
}

/*:115*//*118:*/
#line 2748 "./funcsat.w"

static inline uintmax_t bh_var2pos(funcsat*f,variable v)
{
return f->binvar_pos[v];
}
static inline bool bh_is_in_heap(funcsat*f,variable v)
{
assert(bh_var2pos(f,v)> 0);
return bh_var2pos(f,v)<=f->binvar_heap_size;
}
static inline bool bh_node_is_in_heap(funcsat*f,struct bh_node*n)
{
assert(n>=f->binvar_heap);
return(uintmax_t)(n-f->binvar_heap)<=f->binvar_heap_size;
}

/*:118*//*119:*/
#line 2767 "./funcsat.w"


static inline double*bh_var2act(funcsat*f,variable v)
{
return&f->binvar_heap[bh_var2pos(f,v)].act;
}

/*:119*//*120:*/
#line 2778 "./funcsat.w"

static inline struct bh_node*bh_top(funcsat*f)
{
return f->binvar_heap+1;
}
static inline struct bh_node*bh_bottom(funcsat*f)
{
return f->binvar_heap+f->binvar_heap_size;
}
static inline bool bh_is_top(funcsat*f,struct bh_node*v)
{
return bh_top(f)==v;
}
static inline struct bh_node*bh_left(funcsat*f,struct bh_node*v)
{
return f->binvar_heap+(2*(v-f->binvar_heap));
}
static inline struct bh_node*bh_right(funcsat*f,struct bh_node*v)
{
return f->binvar_heap+(2*(v-f->binvar_heap)+1);
}
static inline struct bh_node*bh_parent(funcsat*f,struct bh_node*v)
{
return f->binvar_heap+((v-f->binvar_heap)/2);
}
static inline uintmax_t bh_size(funcsat*f)
{
return f->binvar_heap_size;
}

/*:120*//*121:*/
#line 2812 "./funcsat.w"

static inline struct bh_node*bh_node_get(funcsat*f,variable v)
{
return f->binvar_heap+f->binvar_pos[v];
}


/*:121*//*123:*/
#line 2844 "./funcsat.w"

static inline void bh_swap(funcsat*f,struct bh_node**x,struct bh_node**y)
{
struct bh_node tmp= **x,*tmpp= *x;
**x= **y,**y= tmp;
*x= *y,*y= tmpp;
f->binvar_pos[(*x)->var]= *x-f->binvar_heap;
f->binvar_pos[(*y)->var]= *y-f->binvar_heap;
}

static inline void bh_bubble_up(funcsat*f,struct bh_node*e)
{
while(!bh_is_top(f,e)){
struct bh_node*p= bh_parent(f,e);
if(activity_compare(p->act,e->act)<0){
bh_swap(f,&p,&e);
}else
break;
}
}

static inline void bh_insert(funcsat*f,variable v)
{
assert(bh_size(f)+1<=f->numVars);
assert(bh_var2pos(f,v)> f->binvar_heap_size);
struct bh_node*node= &f->binvar_heap[bh_var2pos(f,v)];
assert(node->var==v);
f->binvar_heap_size++;
struct bh_node*last= &f->binvar_heap[f->binvar_heap_size];
bh_swap(f,&node,&last);
bh_bubble_up(f,node);
assert(f->binvar_heap[bh_var2pos(f,v)].var==v);
}


/*:123*//*124:*/
#line 2882 "./funcsat.w"

static inline void bh_bubble_down(funcsat*f,struct bh_node*e)
{
struct bh_node*l,*r;
goto bh_bd_begin;
while(bh_node_is_in_heap(f,l)){
if(bh_node_is_in_heap(f,r)){
if(activity_compare(l->act,r->act)<0)
l= r;
}
if(activity_compare(e->act,l->act)<0){
bh_swap(f,&e,&l);
}else
break;
bh_bd_begin:
l= bh_left(f,e),r= bh_right(f,e);
}
}

static inline variable bh_pop(funcsat*f)
{
assert(f->binvar_heap_size> 0);
struct bh_node*top= bh_top(f);
struct bh_node*bot= bh_bottom(f);
bh_swap(f,&top,&bot);
f->binvar_heap_size--;
bh_bubble_down(f,bh_top(f));
return top->var;
}

/*:124*//*125:*/
#line 2913 "./funcsat.w"

static inline void bh_increase_activity(funcsat*f,variable v,double act_new)
{
double*act_curr= bh_var2act(f,v);
struct bh_node*n= bh_node_get(f,v);
assert(n->var==v);
assert(*act_curr<=act_new);
*act_curr= act_new;
bh_bubble_up(f,n);
}

/*:125*//*127:*/
#line 2929 "./funcsat.w"

static void bh_check_node(funcsat*f,struct bh_node*x)
{
struct bh_node*l= bh_left(f,x),*r= bh_right(f,x);
if(bh_node_is_in_heap(f,l)){
assert(activity_compare(l->act,x->act)<=0);
bh_check_node(f,l);
}
if(bh_node_is_in_heap(f,r)){
assert(activity_compare(r->act,x->act)<=0);
bh_check_node(f,r);
}
}

static void bh_check(funcsat*f)
{
struct bh_node*root= bh_top(f);
if(bh_node_is_in_heap(f,root)){
bh_check_node(f,root);
}
for(uintmax_t i= 1;i<f->numVars;i++){
assert(bh_node_get(f,i)->var==i);
}
}

/*:127*//*133:*/
#line 2988 "./funcsat.w"

static void bh_padding(funcsat*f,const char*s,int x)
{
while(x--> 0){
fprintf(fs_dbgout(f),"%s",s);
}
}

static bool bh_print_levels(funcsat*f,FILE*dotfile,struct bh_node*r,int level)
{
assert(r);
if(bh_node_is_in_heap(f,r)){
bool lf,ri;
lf= bh_print_levels(f,dotfile,bh_left(f,r),level+1);
ri= bh_print_levels(f,dotfile,bh_right(f,r),level+1);
if(lf)fprintf(dotfile,"%ju -> %ju [label=\"L\"];\n",r->var,bh_left(f,r)->var);
if(ri)fprintf(dotfile,"%ju -> %ju [label=\"R\"];\n",r->var,bh_right(f,r)->var);
fprintf(dotfile,"%ju [label=\"%ju%s, %.1f\"];\n",r->var,r->var,
(funcsatValue(f,r->var)==true?"T":
(funcsatValue(f,r->var)==false?"F":"?")),r->act);
return true;
}else{
return false;
}
}

static void bh_print(funcsat*f,const char*path,struct bh_node*r)
{
if(!r)r= bh_top(f);
FILE*dotfile;
if(NULL==(dotfile= fopen(path,"w")))perror("fopen"),exit(1);
fprintf(dotfile,"digraph G {\n");
bh_print_levels(f,dotfile,r,0);
fprintf(dotfile,"}\n");
if(0!=fclose(dotfile))perror("fclose");
fprintf(fs_dbgout(f),"\n");
}

/*:133*//*135:*/
#line 3031 "./funcsat.w"

void backtrack(funcsat*f,variable newLevel,head_tail*facts,bool isRestart)
{
head_tail restart_facts;
if(isRestart){
assert(newLevel==0UL),assert(!facts);
facts= &restart_facts;
head_tail_clear(&restart_facts);
}
while(f->decisionLevel!=newLevel)
trailPop(f,facts);
if(isRestart){
literal top;


if(f->Backtrack_hook)f->Backtrack_hook((uintptr_t*)f->conf->user,0);
if(f->MakeDecision_hook)f->MakeDecision_hook((uintptr_t*)f->conf->user);
f->propq= 0;
restore_facts(f,facts);
if(f->trail.size> 0){
top= trailPeek(f);
head_tail_append(f,&f->unit_facts[fs_lit2var(top)],facts);
}
}
}

static void restore_facts(funcsat*f,head_tail*facts)
{
uintptr_t prev,curr,next;
for_head_tail(f,facts,prev,curr,next){

struct clause_head*h= clause_head_ptr(f,curr);
assert(h->sz>=1);

if(h->sz==1){
literal p= h->lits[0];
mbool val= funcsatValue(f,p);
assert(val!=false);
if(val==unknown){
trailPush(f,p,reason_info_mk(f,curr));
fslog(f,"bcp",5," => %ji\n",p);
}
}else{
head_tail_iter_rm(f,facts,prev,curr,next);
addWatchUnchecked(f,curr);
#ifndef NDEBUG
watches_check(f);
#endif
}
}
}

/*:135*//*138:*/
#line 3108 "./funcsat.w"

bool analyze_conflict(funcsat*f)
{
struct clause_iter it;
variable i,c= 0;

literal*p;
head_tail facts;
literal uipLit= 0;

++f->numConflicts;

assert(f->uipClause.size==0);
clause_iter_init(f,f->conflict_clause,&it);
for(literal*p= clause_iter_next(f,&it);p;p= clause_iter_next(f,&it)){
clausePush(&f->uipClause,*p);
}
#if !defined(NDEBUG) && defined(PRINT_IMPL_GRAPH)
print_dot_impl_graph(f,&f->uipClause);
#endif

head_tail_clear(&facts);
litpos_clear(&f->litpos_uip);
litpos_init_from_uip_clause(f);
for_clause(p,&f->uipClause)
if((uintmax_t)levelOf(f,fs_lit2var(*p))==f->decisionLevel)
c++;
int64_t entrydl= (int64_t)f->decisionLevel;
#ifndef NDEBUG
watches_check(f);
#endif
bool isUnsat= find_uips(f,c,&facts,&uipLit);
if(isUnsat){
fslog(f,"solve",1,"find_uips returned isUnsat\n");
f->conflict_clause= NO_CLS;
return false;
}

f->conf->decayAfterConflict(f);
f->conflict_clause= NO_CLS;
f->uipClause.size= 0;
#ifndef NDEBUG
watches_check(f);
#endif
if(!propagateFacts(f,&facts,uipLit)){
return analyze_conflict(f);
}else{
return true;
}
}

/*:138*//*139:*/
#line 3166 "./funcsat.w"

bool find_uips(funcsat*f,uintmax_t c,head_tail*facts,literal*lit_uip)
{
uint32_t num_uips_learned= 0,num_uips_to_learn= f->conf->numUipsToLearn;
uint32_t num_resolutions= 0;
literal p;
fs_ifdbg(f,"find_uips",1){
fslog(f,"find_uips",1,"conflict@%ju (#%"PRIu64") with ",
f->decisionLevel,f->numConflicts);
fs_clause_print(f,fs_dbgout(f),&f->uipClause);
fprintf(fs_dbgout(f),"\n");
}
dopen(f,"find_uips");
do{
/*140:*/
#line 3224 "./funcsat.w"

while(!litpos_contains(&f->litpos_uip,p= trailPeek(f)))trailPop(f,facts);
do{
uintptr_t ix_reason;
uintmax_t num_new_lits;
/*141:*/
#line 3261 "./funcsat.w"

#ifndef NDEBUG
{
literal*q;
uint32_t inner_cnt= 0;
assert(litpos_contains(&f->litpos_uip,trailPeek(f)));
for_clause(q,&f->uipClause){
if(levelOf(f,fs_lit2var(*q))==(literal)f->decisionLevel)inner_cnt++;
}
assert(inner_cnt==c);
}
#endif

/*:141*/
#line 3229 "./funcsat.w"

assert(litpos_contains(&f->litpos_uip,p));
ix_reason= getReason(f,p);
num_new_lits= pop_and_resolve(f,ix_reason,facts);
num_resolutions++;
c= c-1+num_new_lits;
while(f->trail.size> 0&&!litpos_contains(&f->litpos_uip,p= trailPeek(f)))
trailPop(f,facts);
if(c==0){
literal*q;
for_clause(q,&f->uipClause)
if(levelOf(f,fs_lit2var(*q))==(literal)f->decisionLevel)c++;
fslog(f,"find_uips",6,
"c=0, backtracked to level %"PRIu64", c = %"PRIu32", p = %ji\n",
f->decisionLevel,c,p);
}
}while(c!=1&&f->uipClause.size> 0&&f->decisionLevel> 0);
num_uips_learned++;
/*146:*/
#line 3360 "./funcsat.w"

fs_ifdbg(f,"find_uips",6){
fslog(f,"find_uips",6,"found raw UIP: ");
fs_clause_print(f,fs_dbgout(f),&f->uipClause);
fprintf(fs_dbgout(f),"\n");
}

/*147:*/
#line 3389 "./funcsat.w"

if(f->decisionLevel==0||f->uipClause.size==0)return true;

/*:147*/
#line 3367 "./funcsat.w"

/*148:*/
#line 3399 "./funcsat.w"

literal watch2= 0,watch2_level= -1;
uintmax_t pPos= litpos_pos(&f->litpos_uip,p);
swap(literal,f->uipClause.data[0],f->uipClause.data[pPos]);
litpos_set(&f->litpos_uip,&f->uipClause.data[0]);
litpos_set(&f->litpos_uip,&f->uipClause.data[pPos]);
if(f->conf->minimizeLearnedClauses)minimizeUip(f,(&f->uipClause));
for(variable i= 1;i<(&f->uipClause)->size;i++){
literal lev= levelOf(f,fs_lit2var((&f->uipClause)->data[i]));
if(watch2_level<lev){
watch2_level= lev;
watch2= (literal)i;
}
if(lev==(literal)f->decisionLevel)break;
}
if(watch2_level!=-1){
swap(literal,f->uipClause.data[1],f->uipClause.data[watch2]);
litpos_set(&f->litpos_uip,&f->uipClause.data[1]);
litpos_set(&f->litpos_uip,&f->uipClause.data[watch2]);
}


/*:148*/
#line 3368 "./funcsat.w"


uintptr_t ix_uip= clause_head_alloc_from_clause(f,&f->uipClause);
struct clause_head*h_uip= clause_head_ptr(f,ix_uip);
f->conf->bumpLearned(f,ix_uip);
fs_ifdbg(f,"find_uips",5){
fslog(f,"find_uips",5,"found min UIP: ");
fs_clause_head_print(f,fs_dbgout(f),ix_uip);
fprintf(fs_dbgout(f),"\n");
}
h_uip->is_learned= true;
h_uip->where= CLAUSE_UNIT;
head_tail_add(f,&f->unit_facts[fs_lit2var(p)],ix_uip);
vec_uintptr_push(f->learned_clauses,ix_uip);
++f->numLearnedClauses;


/*:146*/
#line 3247 "./funcsat.w"

if(isDecision(f,fs_lit2var(p)))
num_uips_to_learn= num_uips_learned;


/*:140*/
#line 3180 "./funcsat.w"

}while(num_uips_learned<num_uips_to_learn);
*lit_uip= p;
while(!isDecision(f,fs_lit2var(trailPeek(f))))
trailPop(f,facts);
p= trailPop(f,facts);
fslog(f,"find_uips",5,"learned %"PRIu32" UIPs\n",num_uips_learned);
dclose(f,"find_uips");
fslog(f,"find_uips",5,"done\n");
return false;
}

/*:139*//*142:*/
#line 3278 "./funcsat.w"

static inline uintmax_t pop_and_resolve(funcsat*f,uintptr_t ix_reason,head_tail*facts){
literal p;
fs_ifdbg(f,"find_uips",6){
fslog(f,"find_uips",6,"resolving ");
fs_clause_print(f,fs_dbgout(f),&f->uipClause);
fprintf(fs_dbgout(f)," with ");
fs_clause_head_print(f,fs_dbgout(f),ix_reason);
fprintf(fs_dbgout(f),"\n");
}
p= trailPop(f,facts);
f->conf->bumpReason(f,ix_reason);
/*143:*/
#line 3309 "./funcsat.w"

variable i,c= 0,pvar= fs_lit2var(p);
uintptr_t pos_p;
struct clause_iter it;

++f->numResolutions;

clause_iter_init(f,ix_reason,&it);
for(literal*q= clause_iter_next(f,&it);q;q= clause_iter_next(f,&it)){
literal l= *q;
if(!litpos_contains(&f->litpos_uip,l)){
literal*new_lit;
clausePush(&f->uipClause,l);
new_lit= f->uipClause.data+f->uipClause.size-1;
assert(*new_lit==l);
litpos_set(&f->litpos_uip,new_lit);
litpos_check(&f->litpos_uip);
if(levelOf(f,fs_lit2var(l))==(literal)f->decisionLevel)c++;
}
}
/*144:*/
#line 3334 "./funcsat.w"

literal*data= f->uipClause.data;
pos_p= litpos_pos(&f->litpos_uip,p);
swap(literal,data[pos_p],data[f->uipClause.size-1]);
litpos_set(&f->litpos_uip,data+pos_p);
litpos_unset(&f->litpos_uip,data[f->uipClause.size-1]);
f->uipClause.size--;
litpos_check(&f->litpos_uip);

/*:144*/
#line 3329 "./funcsat.w"

return c;

/*:143*/
#line 3290 "./funcsat.w"

}


/*:142*//*156:*/
#line 3498 "./funcsat.w"

static void cleanSeen(funcsat*f,variable top)
{
while(f->analyseToClear.size> top){
variable v= fs_lit2var(vec_intmax_pop(&f->analyseToClear));
f->seen.data[v]= false;
}
}

static inline bool isAssumption(funcsat*f,variable v)
{
literal*it;
for_clause(it,&f->assumptions){
if(fs_lit2var(*it)==v){
return true;
}
}
return false;
}




static bool litRedundant(funcsat*f,literal q0)
{
if(levelOf(f,fs_lit2var(q0))==0)return false;
if(isDecision(f,fs_lit2var(q0)))return false;
assert(!isAssumption(f,fs_lit2var(q0)));



vec_uintmax_clear(&f->analyseStack);
vec_uintmax_push(&f->analyseStack,fs_lit2var(q0));
variable top= f->analyseToClear.size;



while(f->analyseStack.size> 0){
variable p= vec_uintmax_pop(&f->analyseStack);

uintptr_t ix= getReason(f,(literal)p);

struct clause_iter it;
literal*q;
clause_iter_init(f,ix,&it);



for(q= clause_iter_next(f,&it);q;q= clause_iter_next(f,&it)){


if(*q==q0)continue;
variable v= fs_lit2var(*q);
literal lev= levelOf(f,v);
if(!f->seen.data[v]&&lev> 0){
if(!isDecision(f,fs_lit2var(*q))&&f->allLevels.data[lev]){
vec_uintmax_push(&f->analyseStack,v);
vec_intmax_push(&f->analyseToClear,*q);
f->seen.data[v]= true;
}else{
cleanSeen(f,top);
return false;
}
}
}

}
return true;
}

/*:156*//*157:*/
#line 3570 "./funcsat.w"

void minimizeUip(funcsat*f,clause*learned)
{

variable i,j;
for(i= 0;i<learned->size;i++){
literal l= levelOf(f,fs_lit2var(learned->data[i]));
assert(l!=Unassigned);
f->allLevels.data[l]= true;
}

vec_intmax_clear(&f->analyseToClear);
literal*it;
forClause(it,learned){
vec_intmax_push(&f->analyseToClear,*it);
}


for(i= 0;i<learned->size;i++){
f->seen.data[fs_lit2var(learned->data[i])]= true;
}



fslog(f,"minimizeUip",6,"searching learned clause for redundant literals\n");


for(i= 1,j= 1;i<learned->size;i++){
literal p= learned->data[i];

if(!litRedundant(f,p)){
learned->data[j]= p;
litpos_set(&f->litpos_uip,learned->data+j);
j++;
}else{
assert(!isAssumption(f,fs_lit2var(p)));
litpos_unset(&f->litpos_uip,p);
fslog(f,"minimizeUip",5,"deleted %ji\n",p);
++f->numLiteralsDeleted;
}
}
learned->size-= i-j;


while(f->analyseToClear.size> 0){
literal l= vec_intmax_pop(&f->analyseToClear);
variable v= fs_lit2var(l);
f->seen.data[v]= false;
f->allLevels.data[levelOf(f,v)]= false;
}
}

/*:157*//*160:*/
#line 3635 "./funcsat.w"

static char*dot_lit2label(literal p)
{
static char buf[64];
sprintf(buf,"lit%ji",fs_lit2var(p));
return buf;
}

static void print_dot_impl_graph_rec(funcsat*f,FILE*dotfile,struct vec_bool*seen,literal p)
{
if(seen->data[fs_lit2var(p)]==false){
fprintf(dotfile,"%s ",dot_lit2label(p));
fprintf(dotfile,"[label=\"%ji @ %ju%s\"%s];\n",
(funcsatValue(f,p)==false?-p:p),
levelOf(f,fs_lit2var(p)),
(funcsatValue(f,p)==unknown?"*":""),
(levelOf(f,fs_lit2var(p))==(literal)f->decisionLevel?",color=green":""));
seen->data[fs_lit2var(p)]= true;
bool isdec= isDecision(f,fs_lit2var(p));
if(!isdec){
uintptr_t ix_r= getReason(f,p);
struct clause_iter it;
clause_iter_init(f,ix_r,&it);
fprintf(dotfile,"/* reason for %ji: ",p);
fs_clause_head_print(f,dotfile,ix_r);
fprintf(dotfile,"*/\n");
for(literal*q= clause_iter_next(f,&it);q;q= clause_iter_next(f,&it)){
if(*q!=-p){
fprintf(dotfile,"%s",dot_lit2label(*q));
fprintf(dotfile," -> ");
fprintf(dotfile,"%s;\n",dot_lit2label(p));
}
}
clause_iter_init(f,ix_r,&it);
for(literal*q= clause_iter_next(f,&it);q;q= clause_iter_next(f,&it)){
print_dot_impl_graph_rec(f,dotfile,seen,*q);
}
}else{
fprintf(dotfile,"/* no reason for %ji */\n",p);
}
}
}

static void print_dot_impl_graph(funcsat*f,clause*cc)
{
literal*p,*q;
struct vec_bool*seen= vec_bool_init(f->numVars+1);
seen->size= f->numVars+1;
char buf[64];
snprintf(buf,64,"conflict%"PRIu64".dot",f->numConflicts),buf[63]= 0;
FILE*dotfile= fopen(buf,"w");

fprintf(dotfile,"digraph G {\n");
fprintf(dotfile,"/* conflict clause: ");
fs_clause_print(f,dotfile,cc);
fprintf(dotfile,"*/\n");
buf[strlen(buf)-strlen(".dot")]= 0;
fprintf(dotfile,"%s [shape=note];\n",buf);
fprintf(dotfile,"cclause [shape=note,label=\"");
for_clause(p,cc){
fprintf(dotfile,"%ji%s ",*p,(funcsatValue(f,*p)==unknown?"*":""));
}
fprintf(dotfile,"\"];\n");
for_clause(p,cc)
print_dot_impl_graph_rec(f,dotfile,seen,*p);
fprintf(dotfile,"lambda [label=L];\n");
for_clause(p,cc){
fprintf(dotfile,"%s",dot_lit2label(*p));
fprintf(dotfile," -> ");
fprintf(dotfile,"lambda [color=\"red\"];\n");
}
fprintf(dotfile,"\n}");
vec_bool_destroy(seen);
fclose(dotfile);
}

/*:160*//*164:*/
#line 3748 "./funcsat.w"

bool propagateFacts(funcsat*f,head_tail*facts,literal uipLit)
{
bool isConsistent= true;
variable uipVar= fs_lit2var(uipLit);
uintmax_t cnt= 0;
dopen(f,"bcp");

uintptr_t ix_prev,ix_curr,ix_next;
for_head_tail(f,facts,ix_prev,ix_curr,ix_next){
++f->numUnitFactProps,++cnt;
/*165:*/
#line 3788 "./funcsat.w"

fs_ifdbg(f,"bcp",5){
fslog(f,"bcp",5,"");
fs_clause_head_print(f,fs_dbgout(f),ix_curr);
fprintf(fs_dbgout(f),"\n");
}

struct clause_head*h_curr= clause_head_ptr(f,ix_curr);
if(h_curr->sz==0)goto Conflict;
else if(h_curr->sz==1){
literal p= h_curr->lits[0];
mbool val= funcsatValue(f,p);
if(val==false)goto Conflict;
else if(val==unknown){
trailPush(f,p,reason_info_mk(f,ix_curr));
fslog(f,"bcp",1," => %ji\n",p);
}
continue;
}

literal p= h_curr->lits[0],q= h_curr->lits[1];
assert(p!=0&&q!=0&&"unset literals");
assert(p!=q&&"did not find distinct literals");
mbool vp= tentativeValue(f,p),vq= tentativeValue(f,q);



if(vp==true&&vq==true){
head_tail_iter_rm(f,facts,ix_prev,ix_curr,ix_next);
addWatchUnchecked(f,ix_curr);
fslog(f,"bcp",5," => watched\n");
}else if(vp==true||vq==true){
;
fslog(f,"bcp",5," => unmolested\n");
}else if(vp==unknown&&vq==unknown){
head_tail_iter_rm(f,facts,ix_prev,ix_curr,ix_next);
addWatchUnchecked(f,ix_curr);
fslog(f,"bcp",5," => watched\n");
}else if(vp==unknown){
if(funcsatValue(f,p)==false)goto Conflict;
assert(h_curr->lits[0]==p);
trailPush(f,p,reason_info_mk(f,ix_curr));
f->conf->bumpUnitClause(f,ix_curr);
fslog(f,"bcp",1," => %ji\n",p);
}else if(vq==unknown){
if(funcsatValue(f,q)==false)goto Conflict;
assert(h_curr->lits[1]==q);
h_curr->lits[0]= q,h_curr->lits[1]= p;
trailPush(f,q,reason_info_mk(f,ix_curr));
fslog(f,"bcp",1," => %ji\n",q);
f->conf->bumpUnitClause(f,ix_curr);
}else{
Conflict:
isConsistent= false;
f->conflict_clause= ix_curr;
fslog(f,"bcp",1," => X\n");
}

/*:165*/
#line 3759 "./funcsat.w"

if(!isConsistent)break;
}

dclose(f,"bcp");
fslog(f,"bcp",1,"propagated %ju facts\n",cnt);
if(f->conflict_clause!=NO_CLS){
if(funcsatValue(f,uipLit)==unknown){
if(f->decisionLevel> 0){
uipVar= fs_lit2var(f->trail.data[f->trail.size-1]);
}else goto ReallyDone;
}
}
#if 0
assert(!head||funcsatValue(f,fs_var2lit(uipVar))!=unknown);
assert(clauseIsAlone(&f->unitFacts.data[uipVar],0));
#endif

if(levelOf(f,uipVar)!=0){
head_tail_append(f,&f->unit_facts[uipVar],facts);
}

ReallyDone:
return isConsistent;
}

/*:164*//*166:*/
#line 3853 "./funcsat.w"

static inline mbool tentativeValue(funcsat*f,literal p)
{
variable v= fs_lit2var(p);
literal*valLoc= &f->trail.data[f->model.data[v]];
bool isTentative= valLoc>=f->trail.data+f->propq;
if(levelOf(f,v)!=Unassigned&&!isTentative)return p==*valLoc;
else return unknown;
}

/*:166*//*173:*/
#line 3911 "./funcsat.w"

static inline void litpos_init(struct litpos*r)
{
r->c= NULL;
r->indices= vec_uintptr_init(2);
}

static inline void litpos_destroy(struct litpos*lp)
{
vec_uintptr_destroy(lp->indices);
}

/*:173*//*174:*/
#line 3926 "./funcsat.w"

static inline void litpos_set(struct litpos*lp,literal*p)
{
assert(lp->c->data<=p);
assert(p<lp->c->data+lp->c->size);
lp->indices->data[fs_lit2var(*p)]= (uintptr_t)(p-lp->c->data);
}

/*:174*//*175:*/
#line 3935 "./funcsat.w"

static inline literal*litpos_lit(struct litpos*lp,literal l)
{
return&lp->c->data[litpos_pos(lp,l)];
}

static inline uintptr_t litpos_pos(struct litpos*lp,literal l)
{
return lp->indices->data[fs_lit2var(l)];
}

static inline bool litpos_contains(struct litpos*lp,literal l)
{
return lp->indices->data[fs_lit2var(l)]!=NO_POS;
}

/*:175*//*176:*/
#line 3953 "./funcsat.w"

static inline void litpos_unset(struct litpos*lp,literal p)
{
lp->indices->data[fs_lit2var(p)]= NO_POS;
}

/*:176*//*177:*/
#line 3960 "./funcsat.w"

static inline void litpos_clear(struct litpos*lp)
{
for_vec_uintptr(up,lp->indices)
*up= NO_POS;
}

/*:177*//*178:*/
#line 3968 "./funcsat.w"

inline void litpos_check(struct litpos*lp)
{
#ifndef NDEBUG
literal*p;
for_clause(p,lp->c)
assert(lp->c->data[litpos_pos(lp,*p)]==*p);
#endif
}

/*:178*//*182:*/
#line 4000 "./funcsat.w"

static inline void litpos_init_from_uip_clause(funcsat*f)
{
literal*p;
struct litpos*lp= &f->litpos_uip;
assert(f->numVars+1==lp->indices->size);
assert(lp->c==&f->uipClause);
for_clause(p,lp->c)litpos_set(lp,p);
}

/*:182*//*185:*/
#line 4056 "./funcsat.w"

static inline uint8_t LBD_compute_score(funcsat*f,uintptr_t ix)
{
uint8_t score= 0;
struct clause_iter it;
#ifndef NDEBUG
struct clause_head*h= clause_head_ptr(f,ix);
#endif
f->LBD_count++;

clause_iter_init(f,ix,&it);
for(literal*p= clause_iter_next(f,&it);p;p= clause_iter_next(f,&it)){
literal level= levelOf(f,fs_lit2var(*p));
assert(level!=Unassigned);
if(f->LBD_levels.data[level]!=f->LBD_count){
f->LBD_levels.data[level]= f->LBD_count;
score++;
}
if(score> LBD_SCORE_MAX)break;
}
assert(h->sz==0||score!=0);
return score;
}

/*:185*//*189:*/
#line 4096 "./funcsat.w"

void LBD_decay_after_conflict(funcsat*f)
{
#if 0
varDecayActivity(f);
#endif
}
/*:189*//*190:*/
#line 4106 "./funcsat.w"

void LBD_bump_learned(funcsat*f,uintptr_t ix)
{
struct clause_iter it;
struct clause_head*h= clause_head_ptr(f,ix);
h->lbd_score= LBD_compute_score(f,ix);

clause_iter_init(f,ix,&it);
for(literal*p= clause_iter_next(f,&it);p;p= clause_iter_next(f,&it)){
varBumpScore(f,fs_lit2var(*p));
}
varDecayActivity(f);
}

/*:190*//*191:*/
#line 4123 "./funcsat.w"

void LBD_bump_reason(funcsat*f,uintptr_t ix)
{
struct clause_iter it;
clause_iter_init(f,ix,&it);
for(literal*p= clause_iter_next(f,&it);p;p= clause_iter_next(f,&it)){
variable v= fs_lit2var(*p);
#if 0
uintptr_t r= getReason(f,(literal)v);
if(r!=NO_CLS){
struct clause_head*h= clause_head_ptr(f,r);
if(h->lbd_score==2){
varBumpScore(f,fs_lit2var(h->lits[0]));
}
}
#endif
varBumpScore(f,fs_lit2var(*p));
}
}


/*:191*//*192:*/
#line 4148 "./funcsat.w"

void LBD_sweep(funcsat*f,void*user)
{
uint64_t diffConfl= f->numConflicts-f->LBD_last_num_conflicts;
#if 0
uint64_t avg= diffConfl==0?1:Max(1,diffLearn/diffConfl);
#endif
uint64_t next= f->LBD_base+f->LBD_increment*f->numSweeps;

assert(next> 0);
if(diffConfl> next){
++f->numSweeps;
/*194:*/
#line 4189 "./funcsat.w"

uint64_t bad_clauses;
uint8_t del_score;
uint64_t max_del= f->learned_clauses->size/2,num_del= 0;

fslog(f,"sweep",1,"%ju learned clauses\n",f->learned_clauses->size);
fslog(f,"sweep",1,"max_del is %"PRIu64"\n",max_del);

/*195:*/
#line 4210 "./funcsat.w"

del_score= 0,bad_clauses= 0;
vec_uint64_zero(&f->LBD_histogram);
for_vec_uintptr(ix_cls,f->learned_clauses){
assert(clause_head_ptr(f,*ix_cls)->lbd_score> 0);
f->LBD_histogram.data[clause_head_ptr(f,*ix_cls)->lbd_score]++;
}

for(uint8_t s= LBD_SCORE_MAX;s> 0;s--){
if(bad_clauses+f->LBD_histogram.data[s]> max_del){
del_score= s;
break;
}
bad_clauses+= f->LBD_histogram.data[s];
}
assert(del_score);

/*:195*/
#line 4197 "./funcsat.w"

fslog(f,"sweep",2,"bad_clauses is %"PRIu64"\n",bad_clauses);
fslog(f,"sweep",2,"del_score is %"PRIu8"\n",del_score);

if(del_score> 2){
/*198:*/
#line 4241 "./funcsat.w"

if(bad_clauses){
uintptr_t i,new_size;
for(variable v= 1;v<=f->numVars;v++){
struct watchlist_elt*elt,*dump;
struct watchlist*wl;

wl= &f->watches.wlist[fs_lit2idx((literal)v)],new_size= wl->size;
for_watchlist(elt,dump,wl){
/*199:*/
#line 4274 "./funcsat.w"

struct clause_head*h= clause_head_ptr(f,elt->cls);
if(/*196:*/
#line 4228 "./funcsat.w"

(h->is_learned&&!h->is_reason&&h->where==CLAUSE_WATCHED&&
h->lbd_score> del_score)

/*:196*/
#line 4276 "./funcsat.w"
){
fs_ifdbg(f,"sweep",5){
fslog(f,"sweep",5,"marked clause ");
fs_clause_head_print(f,fs_dbgout(f),elt->cls);
fprintf(fs_dbgout(f)," score is %x",(unsigned)clause_head_ptr(f,elt->cls)->lbd_score);
NEWLINE(fs_dbgout(f));
}
new_size--;
continue;
}
*dump= *elt,watchlist_next_elt(dump,wl);



/*:199*/
#line 4250 "./funcsat.w"

}
wl->size= new_size;
wl= &f->watches.wlist[fs_lit2idx(-(literal)v)],new_size= wl->size;
for_watchlist(elt,dump,wl){
/*199:*/
#line 4274 "./funcsat.w"

struct clause_head*h= clause_head_ptr(f,elt->cls);
if(/*196:*/
#line 4228 "./funcsat.w"

(h->is_learned&&!h->is_reason&&h->where==CLAUSE_WATCHED&&
h->lbd_score> del_score)

/*:196*/
#line 4276 "./funcsat.w"
){
fs_ifdbg(f,"sweep",5){
fslog(f,"sweep",5,"marked clause ");
fs_clause_head_print(f,fs_dbgout(f),elt->cls);
fprintf(fs_dbgout(f)," score is %x",(unsigned)clause_head_ptr(f,elt->cls)->lbd_score);
NEWLINE(fs_dbgout(f));
}
new_size--;
continue;
}
*dump= *elt,watchlist_next_elt(dump,wl);



/*:199*/
#line 4255 "./funcsat.w"

}
wl->size= new_size;
}
for(i= 0,new_size= 0;i<f->learned_clauses->size;i++){
uintptr_t elt= f->learned_clauses->data[i];
struct clause_head*h= clause_head_ptr(f,elt);
if(/*197:*/
#line 4233 "./funcsat.w"

(!h->is_reason&&h->where==CLAUSE_WATCHED&&
h->lbd_score> del_score)

/*:197*/
#line 4262 "./funcsat.w"
){
num_del++,bad_clauses--;
f->numLearnedDeleted++;
fslog(f,"sweep",6,"deleted %"PRIuPTR"\n",elt);
clause_release(f,elt);
}else
f->learned_clauses->data[new_size++]= elt;
}
f->learned_clauses->size= new_size;
}

/*:198*/
#line 4202 "./funcsat.w"

/*200:*/
#line 4292 "./funcsat.w"



/*:200*/
#line 4203 "./funcsat.w"

}
fslog(f,"sweep",1,"deleted %"PRIu64" clauses\n",num_del);

f->LBD_last_num_conflicts= f->numConflicts;

/*:194*/
#line 4160 "./funcsat.w"

}
}

/*:192*//*203:*/
#line 4305 "./funcsat.w"

static int LBD_compare_rev(void*fp,const void*cl1,const void*cl2)
{
funcsat*f= (funcsat*)fp;
struct clause_head*c1= clause_head_ptr(f,*(uintptr_t*)cl1);
struct clause_head*c2= clause_head_ptr(f,*(uintptr_t*)cl2);
uint64_t s1= c1->lbd_score,s2= c2->lbd_score;
if(s1==s2)return 0;
else if(s1> s2)return-1;
else return 1;
}

/*:203*//*205:*/
#line 4329 "./funcsat.w"

void funcsatSetupLbdGc(funcsat_config*conf)
{
conf->gc= true;
conf->sweepClauses= LBD_sweep;
conf->bumpReason= LBD_bump_reason;
conf->bumpLearned= LBD_bump_learned;
conf->decayAfterConflict= LBD_decay_after_conflict;
}

/*:205*//*207:*/
#line 4345 "./funcsat.w"

void claActivitySweep(funcsat*f,void*user)
{
static uint64_t num= 0,lastNumConflicts= 0,lastNumLearnedClauses= 0;
#if 0
const uint64_t diffConfl= f->numConflicts-lastNumConflicts;
const uint64_t diffLearn= f->numLearnedClauses-lastNumLearnedClauses;
uint64_t size= f->learnedClauses.size;
double extraLim= f->claInc/(size*1.f);

if(f->numLearnedClauses*1.f>=f->maxLearned){
++f->numSweeps;
uint64_t numDeleted= 0;


qsort(f->learnedClauses.data,f->learnedClauses.size,sizeof(clause*),compareByActivityRev);

fs_ifdbg(f,"sweep",5){
uint64_t dupCount= 0;
int32_t lastLbd= LBD_SCORE_MAX;
FILE*out= f->conf->debugStream;
fprintf(out,"sorted:\n");
clause**it;
forVectorRev(clause**,it,&f->learnedClauses){
if((*it)->lbdScore==lastLbd){
dupCount++;
}else{
if(dupCount> 0){
fprintf(out,"(repeats %"PRIu64" times) %d ",dupCount,(*it)->lbdScore);
}else{
fprintf(out,"%d ",(*it)->lbdScore);
}
dupCount= 0;
lastLbd= (*it)->lbdScore;
}
}
if(dupCount> 0){
fprintf(out,"%d (repeats %"PRIu64" times) ",lastLbd,dupCount);
}

fprintf(out,"done\n");
}


for(uintmax_t i= 0;i<f->learnedClauses.size;i++){
clause*c= f->learnedClauses.data[i];
}


#if 0
for(variable i= 0;i<f->learnedClauses.size;i++){
clause*c= f->learnedClauses.data[i];
if(c->activity==0.f){
clause*rmClause= funcsatRemoveClause(f,c);
if(rmClause){
clauseFree(c);
--f->numLearnedClauses,++f->numLearnedDeleted,++numDeleted;
}
}else break;
}
#endif

const uint64_t max= f->learnedClauses.size/2;
fslog(f,"sweep",1,"deleting at most %"PRIu64
" clauses (of %ju)\n",max,f->learnedClauses.size);
for(variable i= 0;i<f->learnedClauses.size&&f->learnedClauses.size> max;i++){
clause*c= f->learnedClauses.data[i];
if(c->size> 2){
clause*rmClause= funcsatRemoveClause(f,c);
if(rmClause){
clauseFree(c);
--f->numLearnedClauses,++f->numLearnedDeleted,++numDeleted;
}
}
}
fslog(f,"sweep",1,">>> deleted %"PRIu64" clauses\n",numDeleted);

num++;
lastNumLearnedClauses= f->numLearnedClauses;
lastNumConflicts= f->numConflicts;
}
#endif
}


/*:207*//*208:*/
#line 4431 "./funcsat.w"

void bumpReasonByActivity(funcsat*f,uintptr_t ix)
{
struct clause_iter it;
bumpClauseByActivity(f,ix);

clause_iter_init(f,ix,&it);
for(literal*p= clause_iter_next(f,&it);p;p= clause_iter_next(f,&it)){
varBumpScore(f,fs_lit2var(*p));
}
}

void bumpLearnedByActivity(funcsat*f,uintptr_t ix)
{
struct clause_iter it;
bumpClauseByActivity(f,ix);

clause_iter_init(f,ix,&it);
for(literal*p= clause_iter_next(f,&it);p;p= clause_iter_next(f,&it)){
varBumpScore(f,fs_lit2var(*p));
varBumpScore(f,fs_lit2var(*p));
}

if(--f->learnedSizeAdjustCnt==0){
f->learnedSizeAdjustConfl*= f->learnedSizeAdjustInc;
f->learnedSizeAdjustCnt= (uint64_t)f->learnedSizeAdjustConfl;
f->maxLearned*= f->learnedSizeInc;
fslog(f,"sweep",1,"update: ml %f\n",f->maxLearned);

}

varDecayActivity(f);
claDecayActivity(f);
}

void lbdBumpActivity(funcsat*f,uintptr_t ix)
{
struct clause_iter it;
clause_iter_init(f,ix,&it);
for(literal*p= clause_iter_next(f,&it);p;p= clause_iter_next(f,&it)){
variable v= fs_lit2var(*p);
uintptr_t reason= getReason(f,(literal)v);
struct clause_head*h= clause_head_ptr(f,reason);
if(reason!=NO_CLS&&h->lbd_score==2){
varBumpScore(f,v);
}
}
}

/*:208*//*209:*/
#line 4481 "./funcsat.w"

static void bumpClauseByActivity(funcsat*f,uintptr_t ix)
{
struct clause_iter it;
struct clause_head*h= clause_head_ptr(f,ix);

if((h->activity+= f->claInc)> 1e20){
fslog(f,"sweep",5,"rescale for activity %f\n",h->activity);
for_vec_uintptr(ix_cl,f->learned_clauses){
struct clause_head*h_cl= clause_head_ptr(f,*ix_cl);
double oldActivity= h_cl->activity;
h_cl->activity*= 1e-20;
fslog(f,"sweep",5,"setting activity from %f to %f\n",oldActivity,h_cl->activity);
}
double oldClaInc= f->claInc;
f->claInc*= 1e-20;
fslog(f,"sweep",5,"setting claInc from %f to %f\n",oldClaInc,f->claInc);
}
}

static inline void varDecayActivity(funcsat*f)
{
f->varInc*= (1/f->varDecay);
}
static void claDecayActivity(funcsat*f)
{
double oldClaInc= f->claInc;
f->claInc*= (1/f->claDecay);
fslog(f,"sweep",9,"decaying claInc from %f to %f\n",oldClaInc,f->claInc);
}

/*:209*//*210:*/
#line 4514 "./funcsat.w"

void incLubyRestart(funcsat*f,bool skip);

bool funcsatNoRestart(funcsat*f,void*p){
return false;
}

bool funcsatLubyRestart(funcsat*f,void*p)
{
if((intmax_t)f->numConflicts>=f->lrestart&&f->decisionLevel> 2){
incLubyRestart(f,false);
return true;
}
return false;
}

bool funcsatInnerOuter(funcsat*f,void*p)
{
static uint64_t inner= 100UL,outer= 100UL,conflicts= 1000UL;
if(f->numConflicts>=conflicts){
conflicts+= inner;
if(inner>=outer){
outer*= 1.1;
inner= 100UL;
}else{
inner*= 1.1;
}
return true;
}
return false;
}

bool funcsatMinisatRestart(funcsat*f,void*p)
{
static uint64_t cutoff= 100UL;
if(f->numConflicts>=cutoff){
cutoff*= 1.5;
return true;
}
return false;
}







int64_t luby(int64_t i)
{
int64_t k;
for(k= 1;k<(int64_t)sizeof(k);k++)
if(i==(1<<k)-1)
return 1<<(k-1);

for(k= 1;;k++)
if((1<<(k-1))<=i&&i<(1<<k)-1)
return luby(i-(1<<(k-1))+1);
}

void incLubyRestart(funcsat*f,bool skip)
{
uint64_t delta;


if(f->lubycnt> 250){
f->lubycnt= 0;
f->lubymaxdelta= 0;
f->waslubymaxdelta= false;
}
delta= 100*(uint64_t)luby((int64_t)++f->lubycnt);
f->lrestart= (int64_t)(f->numConflicts+delta);






if(delta> f->lubymaxdelta){
f->lubymaxdelta= delta;
f->waslubymaxdelta= 1;
}else{
f->waslubymaxdelta= 0;
}
}


/*:210*//*212:*/
#line 4608 "./funcsat.w"


funcsat_config funcsatDefaultConfig= {
.user= NULL,
.name= NULL,
.usePhaseSaving= true,
.useSelfSubsumingResolution= false,
.minimizeLearnedClauses= true,
.numUipsToLearn= UINT32_MAX,
.gc= true,
.maxJailDecisionLevel= 0,
.logSyms= NULL,
.printLogLabel= true,
.debugStream= NULL,
.isTimeToRestart= funcsatLubyRestart,
.isResourceLimitHit= funcsatIsResourceLimitHit,
.preprocessNewClause= funcsatPreprocessNewClause,
.preprocessBeforeSolve= funcsatPreprocessBeforeSolve,
.getInitialActivity= funcsatDefaultStaticActivity,
.sweepClauses= LBD_sweep,
.bumpOriginal= bumpOriginal,
.bumpReason= LBD_bump_reason,
.bumpLearned= LBD_bump_learned,
.bumpUnitClause= bumpUnitClauseByActivity,
.decayAfterConflict= LBD_decay_after_conflict,
};

/*:212*//*213:*/
#line 4638 "./funcsat.w"

void bumpOriginal(funcsat*f,uintptr_t ix_clause)
{
struct clause_head*h= clause_head_ptr(f,ix_clause);
double orig_varInc= f->varInc;
struct clause_iter it;

f->varInc+= 2.*(1./(double)h->sz);
clause_iter_init(f,ix_clause,&it);
for(literal*p= clause_iter_next(f,&it);p;p= clause_iter_next(f,&it)){
varBumpScore(f,fs_lit2var(*p));
}

f->varInc= orig_varInc;

#if 0
forClause(it,c){
varBumpScore(f,fs_lit2var(*it));
}
#endif
}

double funcsatDefaultStaticActivity(variable*v)
{
return 1.f;
}

void myDecayAfterConflict(funcsat*f)
{
varDecayActivity(f);
claDecayActivity(f);
}

void bumpUnitClauseByActivity(funcsat*f,uintptr_t ix)
{
bumpClauseByActivity(f,ix);
}


/*:213*//*229:*/
#line 5525 "./funcsat.w"

#include "funcsat/vec_char.h"
int64_t readHeader(int(*getChar)(void*),void*,funcsat*func);
funcsat_result readClauses(
int(*getChar)(void*),void*,funcsat*func,uint64_t numClauses);

int fgetcWrapper(void*stream)
{
return fgetc((FILE*)stream);
}

funcsat_result parseDimacsCnf(const char*path,funcsat*f)
{
struct stat buf;
if(-1==stat(path,&buf))perror("stat"),exit(1);
if(!S_ISREG(buf.st_mode)){
fprintf(stderr,"Error: '%s' not a regular file\n",path);
exit(1);
}
int(*getChar)(void*);
void*stream;
const char*opener;
funcsat_result result;
bool isGz= 0==strcmp(".gz",path+(strlen(path)-strlen(".gz")));
if(isGz){
#ifdef HAVE_LIBZ
#  if 0
fprintf(stderr,"c found .gz file\n");
#  endif
getChar= (int(*)(void*))gzgetc;
stream= gzopen(path,"r");
opener= "gzopen";
#else
fprintf(stderr,"cannot read gzip'd file\n");
exit(1);
#endif
}else{
getChar= fgetcWrapper;
stream= fopen(path,"r");
opener= "fopen";
}
if(!stream){
perror(opener);
exit(1);
}

uint64_t num_clauses= (uint64_t)readHeader(getChar,stream,f);
fslog(f,"solve",2,"read %"PRIu64" clauses from header\n",num_clauses);
result= readClauses(getChar,stream,f,num_clauses);

if(isGz){
#ifdef HAVE_LIBZ
if(Z_OK!=gzclose(stream))perror("gzclose"),exit(1);
#else
assert(0&&"no libz and this shouldn't happen");
#endif
}else{
if(0!=fclose(stream))perror("fclose"),exit(1);
}
return result;
}


static literal readLiteral(
int(*getChar)(void*stream),
void*stream,
struct vec_char*tmp,
uint64_t numClauses);
int64_t readHeader(int(*getChar)(void*stream),void*stream,funcsat*func)
{
char c;
Comments:
while(isspace(c= getChar(stream)));
if('c'==c){
while('\n'!=(c= getChar(stream)));
}

if('p'!=c){
goto Comments;
}
while('c'!=(c= getChar(stream)));
assert(c=='c');
c= getChar(stream);
assert(c=='n');
c= getChar(stream);
assert(c=='f');

struct vec_char*tmp= vec_char_init(4);
readLiteral(getChar,stream,tmp,0);
uintmax_t numClauses= (uintmax_t)readLiteral(getChar,stream,tmp,0);
vec_char_destroy(tmp);
#if 0
fprintf(stderr,"c read header 'p cnf %ju %ju'\n",numVariables,numClauses);
#endif
return(intmax_t)numClauses;
}


funcsat_result readClauses(
int(*getChar)(void*stream),
void*stream,
funcsat*func,
uint64_t numClauses)
{
clause*clause;
funcsat_result result= FS_UNKNOWN;
struct vec_char*tmp= vec_char_init(4);
if(numClauses> 0){
do{
clause= clauseAlloc(5);

literal literal= readLiteral(getChar,stream,tmp,numClauses);

while(literal!=0){
clausePush(clause,literal);
literal= readLiteral(getChar,stream,tmp,numClauses);
}
#if 0
clause_head_print_dimacs(stderr,clause);
fprintf(stderr,"\n");
#endif
if(FS_UNSAT==funcsatAddClause(func,clause)){
result= FS_UNSAT;
clauseFree(clause);
break;
}else
clauseFree(clause);
}while(--numClauses> 0);
}
vec_char_destroy(tmp);
return result;
}

static literal readLiteral(
int(*getChar)(void*stream),
void*stream,
struct vec_char*tmp,
uint64_t numClauses)
{
char c;
bool begun;
vec_char_clear(tmp);
literal literal;
begun= false;
while(1){
c= getChar(stream);
if(isspace(c)||EOF==c){
if(begun){
break;
}else if(EOF==c){
fprintf(stderr,"readLiteral error: too few clauses (after %"
PRIu64" clauses)\n",numClauses);
exit(1);
}else{
continue;
}
}
begun= true;
vec_char_push(tmp,c);
}
vec_char_push(tmp,'\0');
literal= strtol((char*)tmp->data,NULL,10);
return literal;
}

/*:229*//*233:*/
#line 5709 "./funcsat.w"

void fs_vig_print(funcsat*f,const char*path)
{
FILE*dot;
char buf[256];
if(NULL==(dot= fopen(path,"w")))perror("fopen"),exit(1);
fprintf(dot,"graph G {\n");

fprintf(dot,"// variables\n");
for(variable i= 1;i<=f->numVars;i++){
fprintf(dot,"%ju;\n",i);
}

fprintf(dot,"// clauses\n");
uintmax_t cnt= 0;
for_vec_uintptr(ix_cl,f->orig_clauses){
struct clause_iter it;
char*bufptr= buf;
clause_iter_init(f,*ix_cl,&it);
for(literal*p= clause_iter_next(f,&it);p;p= clause_iter_next(f,&it)){
bufptr+= sprintf(bufptr,"%ji ",*p);
}
fprintf(dot,"clause%ju [shape=box,label=\"%s\",fillcolor=black,shape=box];\n",
cnt,buf);
clause_iter_init(f,*ix_cl,&it);
for(literal*p= clause_iter_next(f,&it);p;p= clause_iter_next(f,&it)){
fprintf(dot,"clause%ju -- %ju [%s];\n",
cnt,
fs_lit2var(*p),
(*p<0?"color=red":"color=green"));
}
cnt++;
}

fprintf(dot,"}\n");
if(0!=fclose(dot))perror("fclose");

}


/*:233*//*237:*/
#line 5894 "./funcsat.w"

DEFINE_HASHTABLE(fsLogMapInsert,fsLogMapSearch,fsLogMapRemove,char,int)

int fslog(const funcsat*f,const char*label,int msgLevel,const char*format,...)
{
int pr= 0;
int*logLevel;
va_list ap;

if(f->conf->logSyms&&(logLevel= hashtable_search(f->conf->logSyms,(void*)label))){
if(msgLevel<=*logLevel){
uintmax_t indent= vec_uintmax_peek(&f->conf->logStack),i;
for(i= 0;i<indent;i++)fprintf(f->conf->debugStream," ");
if(f->conf->printLogLabel)pr+= fprintf(f->conf->debugStream,"%s %d: ",label,msgLevel);
va_start(ap,format);
pr+= vfprintf(f->conf->debugStream,format,ap);
va_end(ap);
}
}
return pr;
}

int dopen(const funcsat*f,const char*label)
{
if(f->conf->logSyms&&hashtable_search(f->conf->logSyms,(void*)label)){
uintmax_t indent= vec_uintmax_peek(&f->conf->logStack)+2;
vec_uintmax_push(&f->conf->logStack,indent);
}
return 0;
}
int dclose(const funcsat*f,const char*label)
{
if(f->conf->logSyms&&hashtable_search(f->conf->logSyms,(void*)label)){
vec_uintmax_pop(&f->conf->logStack);
}
return 0;
}







/*:237*//*238:*/
#line 5939 "./funcsat.w"









static void swapInMaxLevelLit(funcsat*f,clause*reason,uintmax_t swapIdx,uintmax_t startIdx)
{
literal secondWatch= 0,swLevel= -1;

for(variable i= startIdx;i<reason->size;i++){
literal lev= levelOf(f,fs_lit2var(reason->data[i]));
fslog(f,"subsumption",9,"level of %ji = %ju\n",
reason->data[i],levelOf(f,fs_lit2var(reason->data[i])));
if(swLevel<lev)swLevel= lev,secondWatch= (literal)i;

if(lev==fs_var2lit(f->decisionLevel))break;
}
if(swLevel!=-1){
literal tmp= reason->data[swapIdx];
reason->data[swapIdx]= reason->data[secondWatch],reason->data[secondWatch]= tmp;
}
}


static void checkSubsumption(
funcsat*f,literal p,clause*learn,clause*reason,bool learnIsUip)
{
#if 0


if(learn->size==reason->size-1&&
reason->size> 1&&
learn->size> 2){

++f->numSubsumptions;

fs_ifdbg(f,"subsumption",1){
FILE*out= f->conf->debugStream;
clause_head_print_dimacs(f,out,learn);
fprintf(out," subsumes ");
clause_head_print_dimacs(f,out,reason);
fprintf(out,"\n");
}

assert(!reason->isReason);

if(learnIsUip)vectorPush(&f->subsumed,(void*)1);
else vectorPush(&f->subsumed,(void*)0);
vectorPush(&f->subsumed,reason);
}
#endif
}

static void minimizeClauseMinisat1(funcsat*f,clause*learned)
{
uintmax_t i,j;
for(i= j= 1;i<learned->size;i++){
variable x= fs_lit2var(learned->data[i]);

if(getReason(f,(literal)x)==NO_CLS)learned->data[j++]= learned->data[i];
else{
uintptr_t r= getReason(f,learned->data[i]);
struct clause_iter it;
clause_iter_init(f,r,&it);
clause_iter_next(f,&it);
for(literal*p= clause_iter_next(f,&it);p;p= clause_iter_next(f,&it)){
#if 0
if(!seen[var(c[k])]&&level(var(c[k]))> 0){
learned[j++]= learned[i];
break;
}
#endif
}
}
}

}






static int compareByActivityRev(const void*cl1,const void*cl2)
{
clause*c1= *(clause**)cl1;
clause*c2= *(clause**)cl2;
double s1= c1->activity,s2= c2->activity;
if(s1==s2)return 0;
else if(s1> s2)return-1;
else return 1;
}



/*:238*//*239:*/
#line 6059 "./funcsat.w"

unsigned int fsLitHash(void*lIn)
{

literal l= *(literal*)lIn;
return(unsigned int)l;
}
int fsLitEq(void*a,void*b){
literal p= *(literal*)a,q= *(literal*)b;
return p==q;
}
unsigned int fsVarHash(void*lIn)
{

literal l= *(literal*)lIn;
return(unsigned int)fs_lit2var(l);
}
int fsVarEq(void*a,void*b){
literal p= *(literal*)a,q= *(literal*)b;
return fs_lit2var(p)==fs_lit2var(q);
}



int litCompare(const void*l1,const void*l2)
{
literal x= *(literal*)l1,y= *(literal*)l2;
if(fs_lit2var(x)!=fs_lit2var(y)){
return fs_lit2var(x)<fs_lit2var(y)?-1:1;
}else{
if(x==y){
return 0;
}else{
return x<y?-1:1;
}
}
}


#if 0
int clauseCompare(const void*cp1,const void*cp2)
{
const clause*c= *(clause**)cp1,*d= *(clause**)cp2;
if(c->size!=d->size){
return c->size<d->size?-1:1;
}else{

uint32_t i;
for(i= 0;i<c->size;i++){
int ret;
if(0!=(ret= litCompare(&c->data[i],&d->data[i]))){
return ret;
}
}
return 0;
}
}
#endif


void sortClause(clause*clause)
{
qsort(clause->data,clause->size,sizeof(*clause->data),litCompare);
}


literal findLiteral(literal p,clause*clause)
{
literal min= 0,max= clause->size-1,mid= -1;
int res= -1;
while(!(res==0||min> max)){
mid= min+((max-min)/2);
res= litCompare(&p,&clause->data[mid]);
if(res> 0){
min= mid+1;
}else{
max= mid-1;
}
}
return res==0?mid:-1;
}

literal findVariable(variable v,clause*clause)
{
literal min= 0,max= clause->size-1,mid= -1;
int res= -1;
while(!(res==0||min> max)){
mid= min+((max-min)/2);
if(v==fs_lit2var(clause->data[mid])){
res= 0;
}else{
literal p= fs_var2lit(v);
res= litCompare(&p,&clause->data[mid]);
}
if(res> 0){
min= mid+1;
}else{
max= mid-1;
}
}
return res==0?mid:-1;
}




clause*clauseAlloc(uint32_t capacity)
{
clause*c;
FS_MALLOC(c,1,sizeof(*c));
clauseInit(c,capacity);
return c;
}



/*:239*//*240:*/
#line 6177 "./funcsat.w"






clause*funcsatSolToClause(funcsat*f){
clause*c= clauseAlloc(f->trail.size);
for(uintmax_t i= 0;i<f->trail.size;i++)
clausePush(c,-f->trail.data[i]);
return c;
}

funcsat_result funcsatFindAnotherSolution(funcsat*f){
clause*cur_sol= funcsatSolToClause(f);
funcsat_result res= funcsatAddClause(f,cur_sol);
if(res==FS_UNSAT)return FS_UNSAT;
res= funcsatSolve(f);
return res;
}

intmax_t funcsatSolCount(funcsat*f,clause subset,clause*lastSolution)
{
assert(f->assumptions.size==0);
intmax_t count= 0;
for(uintmax_t i= 0;i<subset.size;i++){
funcsatResize(f,fs_lit2var(subset.data[i]));
}

clause assumptions;
clauseInit(&assumptions,subset.size);

uintmax_t twopn= (uintmax_t)round(pow(2.,(double)subset.size));
fslog(f,"countsol",1,"%ju incremental problems to solve\n",twopn);
for(uintmax_t i= 0;i<twopn;i++){
fslog(f,"countsol",2,"%ju: ",i);
clauseClear(&assumptions);
clauseCopy(&assumptions,&subset);


uintmax_t n= i;
for(uintmax_t j= 0;j<subset.size;j++){

if((n%2)==0)assumptions.data[j]*= -1;
n>>= 1;
}

if(funcsatPushAssumptions(f,&assumptions)==FS_UNSAT){
continue;
}

if(FS_SAT==funcsatSolve(f)){
count++;
if(lastSolution){
clauseClear(lastSolution);
clauseCopy(lastSolution,&f->trail);
}
}

funcsatPopAssumptions(f,f->assumptions.size);
}

clauseDestroy(&assumptions);

return count;
}


/*:240*//*242:*/
#line 6362 "./funcsat.w"


extern mbool funcsatValue(funcsat*f,literal p)
{
variable v= fs_lit2var(p);
if(f->level.data[v]==Unassigned)return unknown;
literal value= f->trail.data[f->model.data[v]];
return p==value;
}

/*:242*//*244:*/
#line 6378 "./funcsat.w"


static char parse_read_char(funcsat*f,FILE*solutionFile){
char c;
c= fgetc(solutionFile);
#if 0
bf_log(b,"bf",8,"Read character '%c'\n",c);
#endif
return c;
}

static const char*s_SATISFIABLE= "SATISFIABLE\n";
static const char*s_UNSATISFIABLE= "UNSATISFIABLE\n";

funcsat_result fs_parse_dimacs_solution(funcsat*f,FILE*solutionFile)
{
char c;
literal var;
bool truth;
funcsat_result result= FS_UNKNOWN;
bool have_var= false;
const char*cur;
state_new_line:
while(true){
c= parse_read_char(f,solutionFile);
switch(c){
case EOF:
goto state_eof;
case'c':
goto state_comment;
case's':
goto state_satisfiablility;
case'v':
goto state_variables;
default:
fslog(f,"fs",1,"unknown line type '%c' in solution file\n",c);
goto state_error;
}
}
state_comment:
while(true){
c= parse_read_char(f,solutionFile);
switch(c){
case EOF:
goto state_eof;
case'\n':
goto state_new_line;
default:
break;
}
}
state_satisfiablility:
cur= NULL;
funcsat_result pending= FS_UNKNOWN;
while(true){
c= parse_read_char(f,solutionFile);
switch(c){
case' ':
case'\t':
continue;
case EOF:
goto state_eof;
default:
ungetc(c,solutionFile);
break;
}
break;
}
while(true){
c= parse_read_char(f,solutionFile);
if(cur==NULL){
switch(c){
case'S':
cur= &s_SATISFIABLE[1];
pending= FS_SAT;
break;
case'U':
cur= &s_UNSATISFIABLE[1];
pending= FS_UNSAT;
break;
case EOF:
goto state_eof;
default:
fslog(f,"fs",1,"unknown satisfiability\n");
goto state_error;
}
}else{
if(c==EOF){
goto state_eof;
}else if(c!=*cur){
fslog(f,"fs",1,"reading satisfiability, got '%c', expected '%c'\n",c,*cur);
goto state_error;
}
if(c=='\n'){
result= pending;
goto state_new_line;
}
++cur;
}
}
state_variables:
while(true){
c= parse_read_char(f,solutionFile);
switch(c){
case'\n':
goto state_new_line;
case' ':
case'\t':
break;
case EOF:
goto state_eof;
default:
ungetc(c,solutionFile);
goto state_sign;
}
}
state_sign:
have_var= false;
truth= true;
c= parse_read_char(f,solutionFile);
if(c=='-'){
truth= false;
}else{
ungetc(c,solutionFile);
}
var= 0;
goto state_variable;
state_variable:
while(true){
c= parse_read_char(f,solutionFile);
switch(c){
case'0':
case'1':
case'2':
case'3':
case'4':
case'5':
case'6':
case'7':
case'8':
case'9':
have_var= true;
var= var*10+(c-'0');
break;
default:
ungetc(c,solutionFile);
goto state_record_variable;
}
}
state_record_variable:
if(have_var){
if(var==0){










goto state_exit;
}else{
variable v= fs_lit2var(var);
funcsatPushAssumption(f,(truth?(literal)v:-(literal)v));
}
goto state_variables;
}else{
abort();
fslog(f,"fs",1,"expected variable, but didn't get one\n");
goto state_error;
}
state_eof:
if(ferror(solutionFile)){
abort();
fslog(f,"fs",1,"IO error reading solution file: %s",strerror(errno));
goto state_error;
}else{
goto state_exit;
}
state_error:
result= FS_UNKNOWN;
goto state_exit;
state_exit:
return result;
}
/*:244*/
#line 359 "./funcsat.w"




/*:14*//*58:*/
#line 1662 "./funcsat.w"



/*:58*//*217:*/
#line 4710 "./funcsat.w"


funcsat*funcsatInit(funcsat_config*conf)
{
funcsat*f;
FS_CALLOC(f,1,sizeof(*f));
f->conf= conf;
f->propq= 0;
/*216:*/
#line 4696 "./funcsat.w"

f->varInc= 1.f;
f->varDecay= 0.95f;
f->claInc= 1.f;
f->claDecay= 0.9999f;
f->learnedSizeFactor= 1.f/3.f;
f->learnedSizeAdjustConfl= 25000;
f->learnedSizeAdjustCnt= 25000;
f->maxLearned= 20000.f;
f->learnedSizeAdjustInc= 1.5f;
f->learnedSizeInc= 1.1f;


/*:216*/
#line 4718 "./funcsat.w"

fslog(f,"sweep",1,"set maxLearned to %f\n",f->maxLearned);
fslog(f,"sweep",1,"set 1/f->claDecoy to %f\n",1.f/f->claDecay);
f->lrestart= 1;
f->lubycnt= 0;
f->lubymaxdelta= 0;
f->waslubymaxdelta= false;

f->numVars= 0;
/*41:*/
#line 1305 "./funcsat.w"

f->clheads.freelist= NO_CLS;
f->clheads.heads= vec_clause_head_init(INITIAL_CLAUSE_POOL);
f->clblocks.freelist= NO_CLS;
f->clblocks.blocks= vec_clause_block_init(INITIAL_CLAUSE_POOL);
for(uintptr_t i= 0;i<INITIAL_CLAUSE_POOL;i++){
clause_head_release(f,i);
clause_block_release(f,i);
}


/*:41*//*51:*/
#line 1536 "./funcsat.w"

f->orig_clauses= vec_uintptr_init(2);
f->learned_clauses= vec_uintptr_init(2);


/*:51*//*98:*/
#line 2359 "./funcsat.w"

f->reason= vec_uintptr_init(2);
vec_uintptr_push(f->reason,NO_CLS);
vec_ptr_mk(&f->reason_hooks,2);

/*:98*//*103:*/
#line 2411 "./funcsat.w"

f->reason_infos_freelist= UINTMAX_MAX;
f->reason_infos= vec_reason_info_init(2);
f->reason_infos->size= 0;


/*:103*//*129:*/
#line 2959 "./funcsat.w"

FS_CALLOC(f->binvar_heap,2,sizeof(*f->binvar_heap));
FS_CALLOC(f->binvar_pos,2,sizeof(*f->binvar_pos));
f->binvar_heap_size= 0;


/*:129*//*137:*/
#line 3102 "./funcsat.w"

f->conflict_clause= NO_CLS;

/*:137*//*152:*/
#line 3468 "./funcsat.w"

vec_uintmax_mk(&f->seen,2);
vec_uintmax_push(&f->seen,false);
vec_intmax_mk(&f->analyseToClear,2);
vec_intmax_push(&f->analyseToClear,0);
vec_uintmax_mk(&f->analyseStack,2);
vec_uintmax_push(&f->analyseStack,0);
vec_uintmax_mk(&f->allLevels,2);
vec_uintmax_push(&f->allLevels,false);

/*:152*//*168:*/
#line 3871 "./funcsat.w"

FS_CALLOC(f->unit_facts,2,sizeof(*f->unit_facts));
f->unit_facts_size= 1;
f->unit_facts_capacity= 2;


/*:168*//*179:*/
#line 3983 "./funcsat.w"

litpos_init(&f->litpos_uip);
f->litpos_uip.c= &f->uipClause;
vec_uintptr_grow_to(f->litpos_uip.indices,f->numVars+1);
vec_uintptr_push(f->litpos_uip.indices,NO_POS);

/*:179*//*186:*/
#line 4082 "./funcsat.w"

vec_uint64_mk(&f->LBD_levels,2);
vec_uint64_push(&f->LBD_levels,0);

/*:186*//*193:*/
#line 4165 "./funcsat.w"

f->LBD_base= 20000;
f->LBD_increment= 500;

/*:193*//*201:*/
#line 4296 "./funcsat.w"

vec_uint64_mk(&f->LBD_histogram,LBD_SCORE_MAX+1);
f->LBD_histogram.size= LBD_SCORE_MAX+1;

/*:201*/
#line 4727 "./funcsat.w"

clauseInit(&f->assumptions,0);
vec_uintmax_mk(&f->model,2);
vec_uintmax_push(&f->model,0);
clauseInit(&f->phase,2);
clausePush(&f->phase,0);
clauseInit(&f->level,2);
clausePush(&f->level,Unassigned);
vec_uintmax_mk(&f->decisions,2);
vec_uintmax_push(&f->decisions,0);
clauseInit(&f->trail,2);
all_watches_init(f);
clauseInit(&f->uipClause,100);
vec_ptr_mk(&f->subsumed,10);

return f;
}



/*:217*//*218:*/
#line 4748 "./funcsat.w"

funcsat_config*funcsatConfigInit(void*userData)
{
funcsat_config*conf= malloc(sizeof(*conf));
memcpy(conf,&funcsatDefaultConfig,sizeof(*conf));
#ifdef FUNCSAT_LOG
conf->logSyms= create_hashtable(16,hashString,stringEqual);
vec_uintmax_mk(&conf->logStack,2);
vec_uintmax_push(&conf->logStack,0);
conf->debugStream= stderr;
#endif
return conf;
}

void funcsatConfigDestroy(funcsat_config*conf)
{
#ifdef FUNCSAT_LOG
hashtable_destroy(conf->logSyms,true,true);
vec_uintmax_unmk(&conf->logStack);
#endif
free(conf);
}

/*:218*//*219:*/
#line 4774 "./funcsat.w"


void funcsatResize(funcsat*f,variable numVars)
{
assert(f->decisionLevel==0);
if(numVars> f->numVars){
const variable old= f->numVars,new= numVars;
f->numVars= new;
variable i;

clauseGrowTo(&f->uipClause,numVars);
/*104:*/
#line 2418 "./funcsat.w"

vec_reason_info_grow_to(f->reason_infos,numVars);
for(uintptr_t i= f->reason_infos->size;i<numVars;i++){
reason_info_release(f,i);
}
f->reason_infos->size= numVars;

/*:104*//*131:*/
#line 2973 "./funcsat.w"

FS_REALLOC(f->binvar_heap,numVars+1,sizeof(*f->binvar_heap));
FS_REALLOC(f->binvar_pos,numVars+1,sizeof(*f->binvar_pos));


/*:131*//*154:*/
#line 3484 "./funcsat.w"

vec_intmax_grow_to(&f->analyseToClear,numVars);
vec_uintmax_grow_to(&f->analyseStack,numVars);


/*:154*/
#line 4785 "./funcsat.w"

for(i= old;i<new;i++){
variable v= i+1;
/*72:*/
#line 1881 "./funcsat.w"

if(f->watches.capacity<=f->watches.size+2){
while(f->watches.capacity<=f->watches.size){
f->watches.capacity= f->watches.capacity*2+2;
}
FS_REALLOC(f->watches.wlist,f->watches.capacity,sizeof(*f->watches.wlist));
}

/*73:*/
#line 1897 "./funcsat.w"

f->watches.wlist[f->watches.size].size= 0;
f->watches.wlist[f->watches.size].capacity= 0;
for(uint32_t i= 0;i<WATCHLIST_HEAD_SIZE_MAX;i++){
f->watches.wlist[f->watches.size].elts[i].lit= 0;
f->watches.wlist[f->watches.size].elts[i].cls= NO_CLS;
}
f->watches.wlist[f->watches.size].rest= NULL;
assert(0==watchlist_head_size(&f->watches.wlist[f->watches.size]));
assert(0==watchlist_rest_size(&f->watches.wlist[f->watches.size]));


/*:73*/
#line 1889 "./funcsat.w"

f->watches.size++;
/*73:*/
#line 1897 "./funcsat.w"

f->watches.wlist[f->watches.size].size= 0;
f->watches.wlist[f->watches.size].capacity= 0;
for(uint32_t i= 0;i<WATCHLIST_HEAD_SIZE_MAX;i++){
f->watches.wlist[f->watches.size].elts[i].lit= 0;
f->watches.wlist[f->watches.size].elts[i].cls= NO_CLS;
}
f->watches.wlist[f->watches.size].rest= NULL;
assert(0==watchlist_head_size(&f->watches.wlist[f->watches.size]));
assert(0==watchlist_rest_size(&f->watches.wlist[f->watches.size]));


/*:73*/
#line 1891 "./funcsat.w"

f->watches.size++;
assert(f->watches.size<=f->watches.capacity);

/*:72*//*105:*/
#line 2426 "./funcsat.w"

vec_uintptr_push(f->reason,NO_CLS);

/*:105*//*132:*/
#line 2979 "./funcsat.w"

f->binvar_heap[v].var= v;
f->binvar_heap[v].act= f->conf->getInitialActivity(&v);
f->binvar_pos[v]= v;
bh_insert(f,v);
assert(f->binvar_pos[v]!=0);


/*:132*//*153:*/
#line 3479 "./funcsat.w"

vec_uintmax_push(&f->seen,false);
vec_uintmax_push(&f->allLevels,false);

/*:153*//*169:*/
#line 3878 "./funcsat.w"

if(f->unit_facts_size>=f->unit_facts_capacity){
FS_REALLOC(f->unit_facts,f->unit_facts_capacity*2,sizeof(*f->unit_facts));
f->unit_facts_capacity*= 2;
}
head_tail_clear(&f->unit_facts[f->unit_facts_size]);
f->unit_facts_size++;


/*:169*//*180:*/
#line 3991 "./funcsat.w"

vec_uintptr_push(f->litpos_uip.indices,NO_POS);

/*:180*//*187:*/
#line 4087 "./funcsat.w"

vec_uint64_push(&f->LBD_levels,0);

/*:187*/
#line 4788 "./funcsat.w"

#if 0
literal l= fs_var2lit(v);
#endif
vec_uintmax_push(&f->model,UINTMAX_MAX);
clausePush(&f->phase,-fs_var2lit(v));
clausePush(&f->level,Unassigned);
vec_uintmax_push(&f->decisions,0);
}
uintmax_t highestIdx= fs_lit2idx(-(literal)numVars)+1;
assert(f->model.size==numVars+1);
assert(!f->conf->usePhaseSaving||f->phase.size==numVars+1);
assert(f->level.size==numVars+1);
assert(f->decisions.size==numVars+1);
assert(f->reason->size==numVars+1);
assert(f->unit_facts_size==numVars+1);
assert(f->uipClause.capacity>=numVars);
assert(f->allLevels.size==numVars+1);
assert(f->watches.size==highestIdx);

if(numVars> f->trail.capacity){
FS_REALLOC(f->trail.data,numVars,sizeof(*f->trail.data));
f->trail.capacity= numVars;
}
}
}

/*:219*//*220:*/
#line 4817 "./funcsat.w"

void funcsatDestroy(funcsat*f)
{
literal i;
while(f->trail.size> 0)trailPop(f,NULL);
/*42:*/
#line 1318 "./funcsat.w"

vec_clause_head_destroy(f->clheads.heads);
vec_clause_block_destroy(f->clblocks.blocks);
f->clheads.freelist= NO_CLS;
f->clheads.freelist= NO_CLS;

/*:42*//*52:*/
#line 1542 "./funcsat.w"

vec_uintptr_destroy(f->orig_clauses);
vec_uintptr_destroy(f->learned_clauses);

/*:52*//*74:*/
#line 1911 "./funcsat.w"


for(variable i= 1;i<=f->numVars;i++){
free(f->watches.wlist[fs_lit2idx((literal)i)].rest);
free(f->watches.wlist[fs_lit2idx(-(literal)i)].rest);
}
free(f->watches.wlist);


/*:74*//*106:*/
#line 2430 "./funcsat.w"

vec_uintptr_destroy(f->reason);
vec_reason_info_destroy(f->reason_infos);
vec_ptr_unmk(&f->reason_hooks);

/*:106*//*130:*/
#line 2966 "./funcsat.w"

free(f->binvar_heap);
free(f->binvar_pos);
f->binvar_heap_size= 0;


/*:130*//*155:*/
#line 3490 "./funcsat.w"

vec_uintmax_unmk(&f->allLevels);
vec_uintmax_unmk(&f->analyseStack);
vec_intmax_unmk(&f->analyseToClear);
vec_uintmax_unmk(&f->seen);


/*:155*//*170:*/
#line 3888 "./funcsat.w"

free(f->unit_facts);

/*:170*//*181:*/
#line 3995 "./funcsat.w"

litpos_destroy(&f->litpos_uip);

/*:181*//*188:*/
#line 4091 "./funcsat.w"

vec_uint64_unmk(&f->LBD_levels);


/*:188*//*202:*/
#line 4301 "./funcsat.w"

vec_uint64_unmk(&f->LBD_histogram);

/*:202*/
#line 4822 "./funcsat.w"

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


/*:220*//*221:*/
#line 4837 "./funcsat.w"

void funcsatSetupActivityGc(funcsat_config*conf)
{
conf->gc= true;
conf->sweepClauses= claActivitySweep;
conf->bumpReason= bumpReasonByActivity;
conf->bumpLearned= bumpLearnedByActivity;
conf->decayAfterConflict= myDecayAfterConflict;
}

funcsat_result funcsatResult(funcsat*f)
{
return f->lastResult;
}





/*:221*//*222:*/
#line 4858 "./funcsat.w"


funcsat_result startSolving(funcsat*f)
{
f->numSolves++;
if(f->conflict_clause!=NO_CLS){
struct clause_head*h= clause_head_ptr(f,f->conflict_clause);
if(h->sz==0){
return FS_UNSAT;
}else{
f->conflict_clause= NO_CLS;
}
}

backtrack(f,0UL,NULL,true);
f->lastResult= FS_UNKNOWN;

assert(f->decisionLevel==0);
return FS_UNKNOWN;
}





static void finishSolving(funcsat*f)
{

}




/*:222*//*223:*/
#line 4893 "./funcsat.w"



static bool bcpAndJail(funcsat*f)
{
if(!bcp(f)){
fslog(f,"solve",2,"returning false at toplevel\n");
return false;
}

fslog(f,"solve",1,"bcpAndJail trailsize is %ju\n",f->trail.size);

clause**cIt;
struct vec_ptr*watches;
uint64_t numJails= 0;

#if 0
forVector(clause**,cIt,&f->origClauses){
if((*cIt)->is_watched){
literal*lIt;
bool allFalse= true;
forClause(lIt,*cIt){
mbool value= funcsatValue(f,*lIt);
if(value==false)continue;
allFalse= false;
if(value==true){

clause**w0= (clause**)&watches->data[fs_lit2idx(-(*cIt)->data[0])];

clause**w1= (clause**)&watches->data[fs_lit2idx(-(*cIt)->data[1])];


clauseUnSpliceWatch(w1,*cIt,1);
jailClause(f,*lIt,*cIt);
numJails++;
break;
}
}
if(allFalse){
fslog(f,"solve",2,"returning false at toplevel\n");
return false;
}
}
}
#endif

fslog(f,"solve",2,"jailed %"PRIu64" clauses at toplevel\n",numJails);
return true;
}


bool funcsatIsResourceLimitHit(funcsat*f,void*p)
{
return false;
}
funcsat_result funcsatPreprocessNewClause(funcsat*f,void*p,clause*c)
{
return(f->lastResult= FS_UNKNOWN);
}
funcsat_result funcsatPreprocessBeforeSolve(funcsat*f,void*p)
{
return(f->lastResult= FS_UNKNOWN);
}


/*:223*//*224:*/
#line 4960 "./funcsat.w"

void funcsatPrintStats(FILE*stream,funcsat*f)
{
fprintf(stream,"c %"PRIu64" decisions\n",f->numDecisions);
fprintf(stream,"c %"PRIu64" propagations (%"PRIu64" unit)\n",
f->numProps+f->numUnitFactProps,
f->numUnitFactProps);
fprintf(stream,"c %"PRIu64" jailed clauses\n",f->numJails);
fprintf(stream,"c %"PRIu64" conflicts\n",f->numConflicts);
fprintf(stream,"c %"PRIu64" learned clauses\n",f->numLearnedClauses);
fprintf(stream,"c %"PRIu64" learned clauses removed\n",f->numLearnedDeleted);
fprintf(stream,"c %"PRIu64" learned clause deletion sweeps\n",f->numSweeps);
if(!f->conf->minimizeLearnedClauses){
fprintf(stream,"c (learned clause minimisation off)\n");
}else{
fprintf(stream,"c %"PRIu64" redundant learned clause literals\n",f->numLiteralsDeleted);
}
if(!f->conf->useSelfSubsumingResolution){
fprintf(stream,"c (on-the-fly self-subsuming resolution off)\n");
}else{
fprintf(stream,"c %"PRIu64" subsumptions\n",f->numSubsumptions);
fprintf(stream,"c - %"PRIu64" original clauses\n",f->numSubsumedOrigClauses);
fprintf(stream,"c - %"PRIu64" UIPs (%2.2lf%%)\n",f->numSubsumptionUips,
(double)f->numSubsumptionUips*100./(double)f->numSubsumptions);
}
fprintf(stream,"c %"PRIu64" restarts\n",f->numRestarts);
fprintf(stream,"c %d assumptions\n",f->assumptions.size);
fprintf(stream,"c %ju original clauses\n",f->orig_clauses->size);
}

void funcsatPrintColumnStats(FILE*stream,funcsat*f)
{
struct rusage usage;
getrusage(RUSAGE_SELF,&usage);
double uTime= ((double)usage.ru_utime.tv_sec)+
((double)usage.ru_utime.tv_usec)/1000000;
double sTime= ((double)usage.ru_stime.tv_sec)+
((double)usage.ru_stime.tv_usec)/1000000;
fprintf(stream,"Name,NumDecisions,NumPropagations,NumUfPropagations,"
"NumLearnedClauses,NumLearnedClausesRemoved,"
"NumLearnedClauseSweeps,NumConflicts,NumSubsumptions,"
"NumSubsumedOrigClauses,NumSubsumedUips,NumRestarts,"
"UserTimeSeconds,SysTimeSeconds\n");
fprintf(stream,"%s,",f->conf->name);
fprintf(stream,"%"PRIu64",%"PRIu64",%"PRIu64",%"PRIu64",%"PRIu64
",%"PRIu64",%"PRIu64",%"PRIu64",%"PRIu64",%"PRIu64
",%"PRIu64",%.2lf,%.2lf\n",
f->numDecisions,
f->numProps+f->numUnitFactProps,
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

void funcsatBumpLitPriority(funcsat*f,literal p)
{
varBumpScore(f,fs_lit2var(p));
}















literal levelOf(funcsat*f,variable v)
{
return f->level.data[v];
}


literal fs_var2lit(variable v)
{
literal result= (literal)v;
assert((variable)result==v);
return result;
}

inline variable fs_lit2var(literal l)
{
if(l<0){
l= -l;
}
return(variable)l;
}

uintmax_t fs_lit2idx(literal l)
{

variable v= fs_lit2var(l);
v<<= 1;
v|= (l<0);
return v;
}

inline bool isDecision(funcsat*f,variable v)
{
return 0!=f->decisions.data[v];
}




#if 0
void singlesPrint(FILE*stream,clause*begin)
{
clause*c= begin;
if(c){
do{
clause*next= c->next[0];
clause*prev= c->prev[0];
if(!next){
fprintf(stream,"next is NULL");
return;
}
if(!prev){
fprintf(stream,"prev is NULL");
return;
}
if(next->prev[0]!=c)fprintf(stream,"n*");
if(prev->next[0]!=c)fprintf(stream,"p*");
clause_head_print_dimacs(stream,c);
c= next;
if(c!=begin)fprintf(stream,", ");
}while(c!=begin);
}else{
fprintf(stream,"NULL");
}
}
#endif

#if 0
void watcherPrint(FILE*stream,clause*c,uint8_t w)
{
if(!c){
fprintf(stream,"EMPTY\n");
return;
}

clause*begin= c;
literal data= c->data[w];
fprintf(stream,"watcher list containing lit %ji\n",c->data[w]);
do{
uint8_t i= c->data[0]==data?0:1;
clause*next= c->next[i];
if(!next){
fprintf(stream,"next is NULL\n");
return;
}
if(!(next->prev[next->data[0]==c->data[i]?0:1]==c)){
fprintf(stream,"*");
}
clause_head_print_dimacs(stream,c);
fprintf(stream,"\n");
c= next;
}while(c!=begin);
}



bool watcherFind(clause*c,clause**watches,uint8_t w)
{
clause*curr= *watches,*nx,*end;
uint8_t wi;
bool foundEnd;
forEachWatchedClause(curr,c->data[w],wi,nx,end,foundEnd){
if(curr==c)return true;
}
return false;
}

void binWatcherPrint(FILE*stream,funcsat*f)
{
variable v;
uintmax_t i;
for(v= 1;v<=f->numVars;v++){
binvec_t*bv= f->watchesBin.data[fs_lit2idx(fs_var2lit(v))];
if(bv->size> 0){
fprintf(stream,"%5ji -> ",fs_var2lit(v));
for(i= 0;i<bv->size;i++){
literal imp= bv->data[i].implied;
fprintf(stream,"%ji",imp);
if(i+1!=bv->size)fprintf(stream,", ");
}
fprintf(stream,"\n");
}

bv= f->watchesBin.data[fs_lit2idx(-fs_var2lit(v))];
if(bv->size> 0){
fprintf(stream,"%5ji -> ",-fs_var2lit(v));
for(i= 0;i<bv->size;i++){
literal imp= bv->data[i].implied;
fprintf(stream,"%ji",imp);
if(i+1!=bv->size)fprintf(stream,", ");
}
fprintf(stream,"\n");
}
}
}

#endif

uintmax_t funcsatNumClauses(funcsat*f)
{
return f->orig_clauses->size;
}

uintmax_t funcsatNumVars(funcsat*f)
{
return f->numVars;
}

/*:224*//*225:*/
#line 5190 "./funcsat.w"

void funcsatPrintHeuristicValues(FILE*p,funcsat*f)
{
for(variable i= 1;i<=f->numVars;i++){
double*value= bh_var2act(f,i);
fprintf(p,"%ju = %4.2lf\n",i,*value);
}
fprintf(p,"\n");
}

void fs_print_state(funcsat*f,FILE*p)
{
variable i;
literal*it;
if(!p)p= stderr;

fprintf(p,"assumptions: ");
forClause(it,&f->assumptions){
fprintf(p,"%ji ",*it);
}
fprintf(p,"\n");

fprintf(p,"dl %ju (%"PRIu64" X, %"PRIu64" d, %"PRIu64" r)\n",
f->decisionLevel,f->numConflicts,f->numDecisions,f->numRestarts);

if(f->conflict_clause!=NO_CLS)
fprintf(p,"cc %"PRIuPTR"\n",f->conflict_clause);
if(f->uipClause.size> 0)
fprintf(p,"uipc "),fs_clause_print(f,p,&f->uipClause),fprintf(p,"\n");

fprintf(p,"trail (%"PRIu32"): ",f->trail.size);
for(i= 0;i<f->trail.size;i++){
fprintf(p,"%2ji",f->trail.data[i]);
if(f->decisions.data[fs_lit2var(f->trail.data[i])]!=0){
fprintf(p,"@%ju",f->decisions.data[fs_lit2var(f->trail.data[i])]);
}
if(!head_tail_is_empty(&f->unit_facts[fs_lit2var(f->trail.data[i])])){
fprintf(p,"*");
}
#if 0
if(!clauseIsAlone(&f->jail.data[fs_lit2var(f->trail.data[i])],0)){
fprintf(p,"!");
}
#endif
fprintf(p," ");
}
fprintf(p,"\n");

fprintf(p,"model: ");
for(i= 1;i<=f->numVars;i++){
if(levelOf(f,i)!=Unassigned){
fprintf(p,"%3ji@%ji ",f->trail.data[f->model.data[i]],levelOf(f,i));
}
}
fprintf(p,"\n");
fprintf(p,"pq: %ju (-> %ji)\n",f->propq,f->trail.data[f->propq]);
}

/*:225*//*226:*/
#line 5251 "./funcsat.w"

void funcsatPrintConfig(FILE*stream,funcsat*f)
{
funcsat_config*conf= f->conf;
if(NULL!=conf->user)
fprintf(stream,"Has user data\n");
if(NULL!=conf->name)
fprintf(stream,"Name: %s\n",conf->name);

conf->usePhaseSaving?
fprintf(stream,"phsv\t"):
fprintf(stream,"NO phsv\t");

conf->useSelfSubsumingResolution?
fprintf(stream,"ssr\t"):
fprintf(stream,"NO ssr\t");

conf->minimizeLearnedClauses?
fprintf(stream,"min\t"):
fprintf(stream,"NO min\t");


if(true==conf->gc){
if(LBD_sweep==conf->sweepClauses){
fprintf(stream,"gc glucose\t");
}else if(claActivitySweep==conf->sweepClauses){
fprintf(stream,"gc minisat\t");
}else{
abort();
}
}else{
fprintf(stream,"NO gc\t\t");
assert(NULL==conf->sweepClauses);
}

if(funcsatLubyRestart==conf->isTimeToRestart){
fprintf(stream,"restart luby\t");
}else if(funcsatNoRestart==conf->isTimeToRestart){
fprintf(stream,"restart none\t");
}else if(funcsatInnerOuter==conf->isTimeToRestart){
fprintf(stream,"restart inout\t");
}else if(funcsatMinisatRestart==conf->isTimeToRestart){
fprintf(stream,"restart minisat\t");
}else{
abort();
}

fprintf(stream,"learn %"PRIu32" uips\t\t",conf->numUipsToLearn);


fprintf(stream,"Jail up to %"PRIuMAX" uips\n",
conf->maxJailDecisionLevel);
#if 0
if(funcsatIsResourceLimitHit==conf->isResourceLimitHit){
fprintf(stream,"  resource hit default\n");
}else{
abort();
}

if(funcsatPreprocessNewClause==conf->preprocessNewClause){
fprintf(stream,"  UNUSED preprocess new clause default\n");
}else{
abort();
}

if(funcsatPreprocessBeforeSolve==conf->preprocessBeforeSolve){
fprintf(stream,"  UNUSED preprocess before solve default\n");
}else{
abort();
}

if(funcsatDefaultStaticActivity==conf->getInitialActivity){
fprintf(stream,"  initial activity static (default)\n");
}else{
abort();
}
#endif
}

void funcsatPrintCnf(FILE*stream,funcsat*f,bool learned)
{
fprintf(stream,"c clauses: %ju original",funcsatNumClauses(f));
if(learned){
fprintf(stream,", %ju learned",f->learned_clauses->size);
}
fprintf(stream,"\n");

uintmax_t num_assumptions= 0;
for(uintmax_t i= 0;i<f->assumptions.size;i++){
if(f->assumptions.data[i]!=0)
num_assumptions++;
}

fprintf(stream,"c %ju assumptions\n",num_assumptions);
uintmax_t numClauses= funcsatNumClauses(f)+
(learned?f->learned_clauses->size:0)+num_assumptions;
fprintf(stream,"p cnf %ju %ju\n",funcsatNumVars(f),numClauses);
for(uintmax_t i= 0;i<f->assumptions.size;i++){
if(f->assumptions.data[i]!=0)
fprintf(stream,"%ji 0\n",f->assumptions.data[i]);
}
vec_clause_head_print_dimacs(f,stream,f->orig_clauses);
if(learned){
fprintf(stream,"c learned\n");
vec_clause_head_print_dimacs(f,stream,f->learned_clauses);
}
}

void funcsatClearStats(funcsat*f)
{
f->numSweeps= 0;
f->numLearnedDeleted= 0;
f->numLiteralsDeleted= 0;
f->numProps= 0;
f->numUnitFactProps= 0;
f->numConflicts= 0;
f->numRestarts= 0;
f->numDecisions= 0;
f->numSubsumptions= 0;
f->numSubsumedOrigClauses= 0;
f->numSubsumptionUips= 0;
}

char*funcsatResultAsString(funcsat_result result)
{
switch(result){
case FS_UNKNOWN:return"UNKNOWN";
case FS_SAT:return"SATISFIABLE";
case FS_UNSAT:return"UNSATISFIABLE";
default:abort();
}
}


bool isUnitClause(funcsat*f,clause*c)
{
variable i,numUnknowns= 0,numFalse= 0;
for(i= 0;i<c->size;i++){
if(funcsatValue(f,c->data[i])==unknown){
numUnknowns++;
}else if(funcsatValue(f,c->data[i])==false){
numFalse++;
}
}
return numUnknowns==1&&numFalse==c->size-(uintmax_t)1;
}


int varOrderCompare(fibkey*a,fibkey*b)
{
fibkey k= *a,l= *b;
if(k> l){
return-1;
}else if(k<l){
return 1;
}else{
return 0;
}
}



clause*funcsatRemoveClause(funcsat*f,clause*c){return NULL;}
#if 0
{
assert(c->isLearnt);
if(c->isReason)return NULL;

if(c->is_watched){

clause**w0= (clause**)&f->watches.data[fs_lit2idx(-c->data[0])];
clause**w1= (clause**)&f->watches.data[fs_lit2idx(-c->data[1])];
clauseUnSpliceWatch(w0,c,0);
clauseUnSpliceWatch(w1,c,1);
}else{

clause*copy= c;
clauseUnSplice(&copy,0);
}

vector*clauses;
if(c->isLearnt){
clauses= &f->learnedClauses;
}else{
clauses= &f->origClauses;
}
clauses->size--;
return c;
}
#endif


extern int DebugSolverLoop;

static char*dups(const char*str)
{
char*res;
FS_CALLOC(res,strlen(str)+1,sizeof(*str));
strcpy(res,str);
return res;
}

bool funcsatDebug(funcsat*f,char*label,int level)
{
#ifdef FUNCSAT_LOG
int*levelp;
FS_MALLOC(levelp,1,sizeof(*levelp));
*levelp= level;
hashtable_insert(f->conf->logSyms,dups(label),levelp);
return true;
#else
return false;
#endif
}



/*:226*/
