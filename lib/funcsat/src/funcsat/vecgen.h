#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>

#include "funcsat/memory.h"


#define VecGenTy(name, eltTy)                                           \
  typedef struct                                                        \
  {                                                                     \
    eltTy   *data;                                                      \
    size_t size;                /* # items stored */                    \
    size_t capacity;            /* # items malloc'd */                  \
  } name ## _t;                                                         \
  static void name ## Destroy(name ## _t *v);                           \
  static void name ## Init(name ## _t *v, size_t capacity);             \
  static void name ## Clear(name ## _t *v);                             \
  static void name ## Push(name ## _t *v, eltTy data);                  \
  /* static void name ## PushAt(name ## _t *v, eltTy data, size_t i);             \ */
  /* eltTy name ## Pop(name ## _t *v);                                     \ */
  /* eltTy name ## PopAt(name ## _t *v, size_t i);                         \ */
  /* eltTy name ## Peek(name ## _t *v);                                    \ */
  /* eltTy name ## Get(name ## _t *v, size_t i);                           \ */
  /* static void name ## Copy(name ## _t *dst, name ## _t *src); */



#define VecGen(name, eltTy)                                     \
  static void name ## Init(name ## _t *v, size_t capacity)      \
  {                                                             \
    size_t c = capacity > 0 ? capacity : 4;                     \
    CallocX(v->data, c, sizeof(*v->data));                      \
    v->size = 0;                                                \
    v->capacity = c;                                            \
  }                                                             \
                                                                \
  static void name ## Destroy(name ## _t *v)                    \
  {                                                             \
    free(v->data);                                              \
    v->data = NULL;                                             \
    v->size = 0;                                                \
    v->capacity = 0;                                            \
  }                                                             \
                                                                \
  static void name ## Clear(name ## _t *v)                      \
  {                                                             \
    v->size = 0;                                                \
  }                                                             \
                                                                \
  static void name ## Push(name ## _t *v, eltTy data)           \
  {                                                             \
    if (v->capacity == v->size) {                               \
      ReallocX(v->data, v->capacity*2+1, sizeof(eltTy));        \
      v->capacity = v->capacity*2+1;                            \
    }                                                           \
    v->data[v->size++] = data;                                  \
  }
/*                                                               \ */
/* static void name ## PushAt(name ## _t *v, eltTy data, size_t i)      \ */
/* {                                                             \ */
/*   size_t j;                                                   \ */
/*   if (v->capacity == v->size) {                               \ */
/*     ReallocX(v->data, v->capacity*2+1, sizeof(eltTy));        \ */
/*     v->capacity = v->capacity*2+1;                            \ */
/*   }                                                           \ */
/*   for (j = i; j < v->size; j++) {                             \ */
/*     v->data[j+1] = v->data[j];                                \ */
/*   }                                                           \ */
/*   v->data[i] = data;                                          \ */
/*   v->size++;                                                  \ */
/* }                                                             \ */
/*                                                               \ */
/* eltTy name ## Pop(name ## _t *v)                              \ */
/* {                                                             \ */
/*   assert(v->size != 0);                                       \ */
/*   return v->data[v->size-- - 1];                              \ */
/* }                                                             \ */
/*                                                               \ */
/* eltTy name ## PopAt(name ## _t *v, size_t i)                  \ */
/* {                                                             \ */
/*   size_t j;                                                   \ */
/*   assert(v->size != 0);                                       \ */
/*   eltTy res = v->data[i];                                     \ */
/*   for (j = i; j < v->size-1; j++) {                           \ */
/*     v->data[j] = v->data[j+1];                                \ */
/*   }                                                           \ */
/*   v->size--;                                                  \ */
/*   return res;                                                 \ */
/* }                                                             \ */
/*                                                               \ */
/* eltTy name ## Peek(name ## _t *v)                             \ */
/* {                                                             \ */
/*   assert(v->size != 0);                                       \ */
/*   return v->data[v->size-1];                                  \ */
/* }                                                             \ */
/*                                                               \ */
/* eltTy name ## Get(name ## _t *v, size_t i)                    \ */
/* {                                                             \ */
/*   return v->data[i];                                          \ */
/* }                                                             \ */
/*                                                               \ */
/* static void name ## Copy(name ## _t *dst, name ## _t *src)           \ */
/* {                                                             \ */
/*   uint64_t i;                                                 \ */
/*   for (i = 0; i < src->size; i++) {                           \ */
/*     name ## Push(dst, src->data[i]);                          \ */
/*   }                                                           \ */
/* } */
