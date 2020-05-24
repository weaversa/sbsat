/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2012, University of Cincinnati.  All rights reserved.
 By using this software the USER indicates that he or she has read,
 understood and will comply with the following:

 --- University of Cincinnati hereby grants USER nonexclusive permission
 to use, copy and/or modify this software for internal, noncommercial,
 research purposes only. Any distribution, including commercial sale
 or license, of this software, copies of the software, its associated
 documentation and/or modifications of either is strictly prohibited
 without the prior consent of University of Cincinnati.  Title to copyright
 to this software and its associated documentation shall at all times
 remain with University of Cincinnati.  Appropriate copyright notice shall
 be placed on all software copies, and a complete copy of this notice
 shall be included in all copies of the associated documentation.
 No right is  granted to use in advertising, publicity or otherwise
 any trademark, service mark, or the name of University of Cincinnati.


 --- This software and any associated documentation is provided "as is"

 UNIVERSITY OF CINCINNATI MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS
 OR IMPLIED, INCLUDING THOSE OF MERCHANTABILITY OR FITNESS FOR A
 PARTICULAR PURPOSE, OR THAT  USE OF THE SOFTWARE, MODIFICATIONS, OR
 ASSOCIATED DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS,
 TRADEMARKS OR OTHER INTELLECTUAL PROPERTY RIGHTS OF A THIRD PARTY.

 University of Cincinnati shall not be liable under any circumstances for
 any direct, indirect, special, incidental, or consequential damages
 with respect to any claim by USER or any third party on account of
 or arising from the use, or inability to use, this software or its
 associated documentation, even if University of Cincinnati has been advised
 of the possibility of those damages.
*********************************************************************/

#include <sbsat.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifndef SBSAT_QUEUE_H
#define SBSAT_QUEUE_H

//-------------Pointer-based Queue Manipulations-------------//

typedef struct cell_queue {
	void *x;
	struct cell_queue *next;
} cell_queue;

typedef struct void_queue {
	cell_queue *head;
	cell_queue *tail;
} void_queue;

void_queue *queue_init();
void queue_free(void_queue *queue);
void enqueue_x(void_queue *queue, void *x);
void *dequeue(void_queue *queue);
uint8_t queue_is_empty(void_queue *queue);

//-------------Pointer-based Stack Manipulations-------------//

typedef struct cell_stack {
	void *x;
	struct cell_stack *push;
	struct cell_stack *pop;
} cell_stack;

typedef struct void_stack {
	cell_stack *head;
} void_stack;

void_stack *stack_init();
void stack_free(void_stack *stack);
void stack_push(void_stack *stack, void *x);
void *stack_pop(void_stack *stack);

//-------------Array-based Stack Manipulations---------------//

typedef struct void_arr_stack {
	uintmax_t head;
	uintmax_t size;
	void **mem;
} void_arr_stack;

void_arr_stack *arr_stack_init();
void arr_stack_free(void_arr_stack *stack);
void arr_stack_push(void_arr_stack *stack, void *x);
void *arr_stack_pop(void_arr_stack *stack);
uint8_t arr_stack_is_empty(void_arr_stack *stack);

//-------------TYPE Parameterized Array-based Stack Memory Undoer------------------------//
#define create_undoer_headers(TYPE)                                                      \
typedef struct TYPE##_memcell {                                                          \
  TYPE *pAddress;                                                                        \
  TYPE tValue;                                                                           \
} TYPE##_memcell;                                                                        \
                                                                                         \
typedef struct TYPE##_undoer {                                                           \
  uintmax_t nHead;                                                                       \
  uintmax_t nLength;                                                                     \
  TYPE##_memcell *pMemcell;                                                              \
  uintmax_t nResizeAmt;                                                                  \
} TYPE##_undoer;                                                                         \
                                                                                         \
void TYPE##_undoer_alloc(TYPE##_undoer *undoer, uintmax_t length, uintmax_t resize_amt); \
void TYPE##_undoer_push(TYPE##_undoer *undoer, TYPE *address);                           \
void TYPE##_undoer_forget(TYPE##_undoer *undoer);                                        \
void TYPE##_undoer_push_marker(TYPE##_undoer *undoer);                                   \
void TYPE##_undoer_undo(TYPE##_undoer *undoer);                                          \
void TYPE##_undoer_undo_to_last_marker(TYPE##_undoer *undoer);                           \
void TYPE##_undoer_free(TYPE##_undoer *undoer);                                          \

create_undoer_headers(uint8_t);
create_undoer_headers(int8_t);
create_undoer_headers(uint16_t);
create_undoer_headers(uint32_t);
create_undoer_headers(uint64_t);
create_undoer_headers(uintmax_t);
create_undoer_headers(uintptr_t);
create_undoer_headers(double);
create_undoer_headers(float);


//-------------TYPE Parameterized Array-based Stack Memory Undoer---------------------------------------------------------------//
#define create_undoer_type(TYPE)                                                                                                 \
inline                                                                                                                           \
void TYPE##_undoer_alloc(TYPE##_undoer *undoer, uintmax_t length, uintmax_t resize_amt) {                                        \
  undoer->pMemcell = (TYPE##_memcell *)malloc(length * sizeof(TYPE##_memcell));                                                  \
  undoer->nHead = 0;                                                                                                             \
  undoer->nLength = length;                                                                                                      \
  undoer->nResizeAmt = resize_amt;                                                                                               \
  TYPE##_undoer_push_marker(undoer);                                                                                             \
}                                                                                                                                \
                                                                                                                                 \
static inline                                                                                                                    \
void increase_##TYPE##_undoer(TYPE##_undoer *undoer) {	                                                                         \
  undoer->pMemcell = (TYPE##_memcell *)realloc(undoer->pMemcell, (undoer->nLength+undoer->nResizeAmt) * sizeof(TYPE##_memcell)); \
  undoer->nLength += undoer->nResizeAmt;                                                                                         \
}                                                                                                                                \
                                                                                                                                 \
inline                                                                                                                           \
void TYPE##_undoer_push(TYPE##_undoer *undoer, TYPE *address) {                                                                  \
  assert(address != NULL);                                                                                                       \
  TYPE value = *address;                                                                                                         \
  if(undoer->nHead >= undoer->nLength-1)                                                                                         \
    increase_##TYPE##_undoer(undoer);                                                                                            \
  TYPE##_memcell *memcell = &undoer->pMemcell[++undoer->nHead];		                                                         \
  memcell->pAddress = address;                                                                                                   \
  memcell->tValue = value;                                                                                                       \
}                                                                                                                                \
                                                                                                                                 \
inline                                                                                                                           \
void TYPE##_undoer_forget(TYPE##_undoer *undoer) {                                                                               \
  assert(undoer->nHead > 0);                                                                                                     \
  --undoer->nHead;                                                                                                               \
}                                                                                                                                \
                                                                                                                                 \
inline                                                                                                                           \
void TYPE##_undoer_push_marker(TYPE##_undoer *undoer) {                                                                          \
  if(undoer->nHead >= undoer->nLength-1)                                                                                         \
    increase_##TYPE##_undoer(undoer);                                                                                            \
  undoer->pMemcell[++undoer->nHead].pAddress = NULL;                                                                             \
  undoer->pMemcell[undoer->nHead].tValue = 0;                                                                                    \
}                                                                                                                                \
                                                                                                                                 \
inline                                                                                                                           \
void TYPE##_undoer_undo(TYPE##_undoer *undoer) {                                                                                 \
  if(undoer->nHead < 1) {                                                                                                        \
    fprintf(stderr, "Warning: TYPE undoer is undoing but no objects are on the stack.\n");                                       \
    assert(0);                                                                                                                   \
    return;                                                                                                                      \
  }                                                                                                                              \
  (*undoer->pMemcell[undoer->nHead].pAddress) = undoer->pMemcell[undoer->nHead].tValue;                                          \
  undoer->nHead--;                                                                                                               \
  return;                                                                                                                        \
}                                                                                                                                \
                                                                                                                                 \
inline                                                                                                                           \
void TYPE##_undoer_undo_to_last_marker(TYPE##_undoer *undoer) {                                                                  \
  if(undoer->nHead < 1) {                                                                                                        \
    fprintf(stderr, "Warning: TYPE undoer is undoing but no objects are on the stack.\n");                                       \
    assert(0);                                                                                                                   \
    return;                                                                                                                      \
  }                                                                                                                              \
  TYPE##_memcell *memcell = undoer->pMemcell + undoer->nHead;                                                                    \
  while (memcell->pAddress != NULL) {                                                                                            \
    assert(((uintmax_t) (memcell - undoer->pMemcell)) != 0);                                                                     \
    (*memcell->pAddress) = memcell->tValue;                                                                                      \
    memcell--;                                                                                                                   \
  }                                                                                                                              \
  memcell--;                                                                                                                     \
  undoer->nHead = (uintmax_t)(memcell - undoer->pMemcell);                                                                       \
  return;                                                                                                                        \
}                                                                                                                                \
                                                                                                                                 \
inline                                                                                                                           \
void TYPE##_undoer_free(TYPE##_undoer *undoer) {                                                                                 \
  assert(undoer != NULL);                                                                                                        \
  if(undoer->nHead>1)                                                                                                            \
    fprintf(stderr, "Warning: TYPE undoer being free'd while objects are still on the stack. %ju\n", undoer->nHead);             \
  free(undoer->pMemcell);                                                                                                        \
}                                                                                                                                \


//-------------TYPE Parameterized Pointer Array-based Stack Memory Undoer------------------//
#define create_pundoer_headers(TYPE)                                                       \
typedef struct TYPE##_pmemcell {                                                           \
  TYPE **pAddress;                                                                         \
  TYPE *pValue;                                                                            \
} TYPE##_pmemcell;                                                                         \
                                                                                           \
typedef struct TYPE##_pundoer {                                                            \
  uintmax_t nHead;                                                                         \
  uintmax_t nLength;                                                                       \
  TYPE##_pmemcell *pMemcell;                                                               \
  uintmax_t nResizeAmt;                                                                    \
} TYPE##_pundoer;                                                                          \
                                                                                           \
void TYPE##_pundoer_alloc(TYPE##_pundoer *undoer, uintmax_t length, uintmax_t resize_amt); \
void TYPE##_pundoer_push(TYPE##_pundoer *undoer, TYPE **address);                          \
void TYPE##_pundoer_forget(TYPE##_pundoer *undoer);                                        \
void TYPE##_pundoer_push_marker(TYPE##_pundoer *undoer);                                   \
void TYPE##_pundoer_undo(TYPE##_pundoer *undoer);                                          \
void TYPE##_pundoer_undo_to_last_marker(TYPE##_pundoer *undoer);                           \
void TYPE##_pundoer_free(TYPE##_pundoer *undoer);                                          \

create_pundoer_headers(uint8_t);
create_pundoer_headers(int8_t);
create_pundoer_headers(uint16_t);
create_pundoer_headers(uint32_t);
create_pundoer_headers(uint64_t);
create_pundoer_headers(uintmax_t);
create_pundoer_headers(uintptr_t);
create_pundoer_headers(double);
create_pundoer_headers(float);


//-------------TYPE Parameterized Array-based Stack Memory Undoer------------------------------------------------------------------//
#define create_pundoer_type(TYPE)                                                                                                  \
inline                                                                                                                             \
void TYPE##_pundoer_alloc(TYPE##_pundoer *undoer, uintmax_t length, uintmax_t resize_amt) {                                        \
  undoer->pMemcell = (TYPE##_pmemcell *)malloc(length * sizeof(TYPE##_pmemcell));                                                  \
  undoer->nHead = 0;                                                                                                               \
  undoer->nLength = length;                                                                                                        \
  undoer->nResizeAmt = resize_amt;                                                                                                 \
  TYPE##_pundoer_push_marker(undoer);                                                                                              \
}                                                                                                                                  \
                                                                                                                                   \
static inline                                                                                                                      \
void increase_##TYPE##_pundoer(TYPE##_pundoer *undoer) {	                                                                   \
  undoer->pMemcell = (TYPE##_pmemcell *)realloc(undoer->pMemcell, (undoer->nLength+undoer->nResizeAmt) * sizeof(TYPE##_pmemcell)); \
  undoer->nLength += undoer->nResizeAmt;                                                                                           \
}                                                                                                                                  \
                                                                                                                                   \
inline                                                                                                                             \
void TYPE##_pundoer_push(TYPE##_pundoer *undoer, TYPE **address) {                                                                 \
  assert(address != NULL);                                                                                                         \
  TYPE *value = *address;                                                                                                          \
  if(undoer->nHead >= undoer->nLength-1)                                                                                           \
    increase_##TYPE##_pundoer(undoer);                                                                                             \
  TYPE##_pmemcell *memcell = &undoer->pMemcell[++undoer->nHead];		                                                   \
  memcell->pAddress = address;                                                                                                     \
  memcell->pValue = value;                                                                                                         \
}                                                                                                                                  \
                                                                                                                                   \
inline                                                                                                                             \
void TYPE##_pundoer_forget(TYPE##_pundoer *undoer) {                                                                               \
  assert(undoer->nHead > 0);                                                                                                       \
  --undoer->nHead;                                                                                                                 \
}                                                                                                                                  \
                                                                                                                                   \
inline                                                                                                                             \
void TYPE##_pundoer_push_marker(TYPE##_pundoer *undoer) {                                                                          \
  if(undoer->nHead >= undoer->nLength-1)                                                                                           \
    increase_##TYPE##_pundoer(undoer);                                                                                             \
  undoer->pMemcell[++undoer->nHead].pAddress = NULL;                                                                               \
  undoer->pMemcell[undoer->nHead].pValue = NULL;                                                                                   \
}                                                                                                                                  \
                                                                                                                                   \
inline                                                                                                                             \
void TYPE##_pundoer_undo(TYPE##_pundoer *undoer) {                                                                                 \
  if(undoer->nHead < 1) {                                                                                                          \
    fprintf(stderr, "Warning: TYPE undoer is undoing but no objects are on the stack.\n");                                         \
    assert(0);                                                                                                                     \
    return;                                                                                                                        \
  }                                                                                                                                \
  (*undoer->pMemcell[undoer->nHead].pAddress) = undoer->pMemcell[undoer->nHead].pValue;                                            \
  undoer->nHead--;                                                                                                                 \
  return;                                                                                                                          \
}                                                                                                                                  \
                                                                                                                                   \
inline                                                                                                                             \
void TYPE##_pundoer_undo_to_last_marker(TYPE##_pundoer *undoer) {                                                                  \
  if(undoer->nHead < 1) {                                                                                                          \
    fprintf(stderr, "Warning: TYPE undoer is undoing but no objects are on the stack.\n");                                         \
    assert(0);                                                                                                                     \
    return;                                                                                                                        \
  }                                                                                                                                \
  TYPE##_pmemcell *memcell = undoer->pMemcell + undoer->nHead;                                                                     \
  while (memcell->pAddress != NULL) {                                                                                              \
    assert(((uintmax_t) (memcell - undoer->pMemcell)) != 0);                                                                       \
    (*memcell->pAddress) = memcell->pValue;                                                                                        \
    memcell--;                                                                                                                     \
  }                                                                                                                                \
  memcell--;                                                                                                                       \
  undoer->nHead = (uintmax_t)(memcell - undoer->pMemcell);                                                                         \
  return;                                                                                                                          \
}                                                                                                                                  \
                                                                                                                                   \
inline                                                                                                                             \
void TYPE##_pundoer_free(TYPE##_pundoer *undoer) {                                                                                 \
  assert(undoer != NULL);                                                                                                          \
  if(undoer->nHead>1)                                                                                                              \
    fprintf(stderr, "Warning: TYPE undoer being free'd while objects are still on the stack. %ju\n", undoer->nHead);               \
  free(undoer->pMemcell);                                                                                                          \
}                                                                                                                                  \

#endif
