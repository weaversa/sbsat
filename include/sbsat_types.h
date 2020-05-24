/*
    sbsat - SBSAT is a state-based Satisfiability solver.
 
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

#ifndef SBSAT_TYPES_H
#define SBSAT_TYPES_H

typedef struct dbl_index {
  uintmax_t index;
  double value;
} dbl_index;

//----------------------TYPE Parameterized List------------------------------------//
#define create_list_headers(TYPE)                                                  \
typedef struct TYPE##_list {                                                       \
  uintmax_t nLength;                                                               \
  uintmax_t nLength_max;                                                           \
  TYPE *pList;                                                                     \
  uintmax_t nResizeAmt;                                                            \
} TYPE##_list;                                                                     \
                                                                                   \
uint8_t TYPE##_list_alloc(TYPE##_list *x, uintmax_t length, uintmax_t resize_amt); \
uint8_t TYPE##_list_resize(TYPE##_list *x, uintmax_t new_length);                  \
uint8_t TYPE##_list_push(TYPE##_list *x, TYPE value);                              \
uint8_t TYPE##_list_add(TYPE##_list *x, uintmax_t index, TYPE value);              \
void TYPE##_list_copy(TYPE##_list *dst, TYPE##_list *src);                         \
void TYPE##_list_free(TYPE##_list *x);                                             \

create_list_headers(uint8_t);
create_list_headers(int8_t);
create_list_headers(uint32_t);
create_list_headers(int32_t);
create_list_headers(uintmax_t);
create_list_headers(intmax_t);
create_list_headers(uintptr_t);


//---------------------------------------TYPE Parameterized List----------------------------------------------------------------------//
#define create_list_type(TYPE)                                                                                                        \
uint8_t TYPE##_list_alloc(TYPE##_list *x, uintmax_t length, uintmax_t resize_amt) {                                                   \
  x->nLength = 0;                                                                                                                     \
  x->nLength_max = length;                                                                                                            \
  x->pList = (TYPE *)sbsat_calloc(length, sizeof(TYPE), 9, "TYPE##_list_alloc");                                                      \
  if(x->pList == NULL) {                                                                                                              \
    return MEM_ERR;                                                                                                                   \
  }                                                                                                                                   \
  x->nResizeAmt = resize_amt;                                                                                                         \
  return NO_ERROR;                                                                                                                    \
}                                                                                                                                     \
                                                                                                                                      \
uint8_t TYPE##_list_resize(TYPE##_list *x, uintmax_t new_length) {                                                                    \
  if(new_length > x->nLength_max) {                                                                                                   \
    uintmax_t delta = new_length - x->nLength_max;                                                                                    \
    if(delta < x->nResizeAmt) new_length = x->nLength_max + delta;                                                                    \
    x->pList = (TYPE *)sbsat_realloc((void*)x->pList, x->nLength_max, new_length, sizeof(TYPE), 9, "TYPE##_list_resize");             \
    if(x->pList==NULL) {                                                                                                              \
      assert(0); return MEM_ERR;						\
    }                                                                                                                                 \
    x->nLength_max=new_length;                                                                                                        \
  }                                                                                                                                   \
  return NO_ERROR;                                                                                                                    \
}                                                                                                                                     \
                                                                                                                                      \
uint8_t TYPE##_list_push(TYPE##_list *x, TYPE value) {                                                                                \
  if(TYPE##_list_resize(x, x->nLength+1) == MEM_ERR) return MEM_ERR;                                                                  \
  x->pList[x->nLength++] = value;                                                                                                     \
  return NO_ERROR;                                                                                                                    \
}                                                                                                                                     \
                                                                                                                                      \
uint8_t TYPE##_list_add(TYPE##_list *x, uintmax_t index, TYPE value) {                                                                \
  if(TYPE##_list_resize(x, index+1) == MEM_ERR) return MEM_ERR;                                                                       \
  x->pList[index] = value;                                                                                                            \
  if(x->nLength <= index) x->nLength = index+1;                                                                                       \
  return NO_ERROR;                                                                                                                    \
}                                                                                                                                     \
                                                                                                                                      \
void TYPE##_list_copy(TYPE##_list *dst, TYPE##_list *src) {                                                                           \
  TYPE##_list_resize(dst, src->nLength);                                                                                              \
  dst->nLength = src->nLength;                                                                                                        \
  memcpy(dst->pList, src->pList, dst->nLength * sizeof(TYPE));                                                                        \
}                                                                                                                                     \
                                                                                                                                      \
void TYPE##_list_free(TYPE##_list *x) {                                                                                               \
  if(x->pList == NULL) return;                                                                                                        \
  sbsat_free((void**)&(x->pList));                                                                                                    \
  x->pList = NULL;                                                                                                                    \
  x->nLength = 0;                                                                                                                     \
  x->nLength_max = 0;                                                                                                                 \
}                                                                                                                                     \


//----------------------TYPE Parameterized Pointer List------------------------------//
#define create_plist_headers(TYPE)                                                   \
typedef struct TYPE##_plist {                                                        \
  uintmax_t nLength;                                                                 \
  uintmax_t nLength_max;                                                             \
  TYPE **pList;                                                                      \
  uintmax_t nResizeAmt;                                                              \
} TYPE##_plist;                                                                      \
                                                                                     \
uint8_t TYPE##_plist_alloc(TYPE##_plist *x, uintmax_t length, uintmax_t resize_amt); \
uint8_t TYPE##_plist_resize(TYPE##_plist *x, uintmax_t new_length);                  \
uint8_t TYPE##_plist_push(TYPE##_plist *x, TYPE *value);                             \
uint8_t TYPE##_plist_add(TYPE##_plist *x, uintmax_t index, TYPE *value);             \
uint8_t TYPE##_plist_copy(TYPE##_plist *dst, TYPE##_plist *src);                     \
uint8_t TYPE##_plist_free(TYPE##_plist *x);                                          \

create_plist_headers(uint8_t);
create_plist_headers(int8_t);
create_plist_headers(uint32_t);
create_plist_headers(int32_t);
create_plist_headers(uintmax_t);
create_plist_headers(intmax_t);
create_plist_headers(uintptr_t);

//---------------------------------------TYPE Parameterized Pointer List--------------------------------------------------------------//
#define create_plist_type(TYPE)                                                                                                       \
uint8_t TYPE##_plist_alloc(TYPE##_plist *x, uintmax_t length, uintmax_t resize_amt) {                                                 \
  x->nLength = 0;                                                                                                                     \
  x->nLength_max = length;                                                                                                            \
  x->pList = (TYPE **)sbsat_calloc(length, sizeof(TYPE *), 9, "TYPE##_plist_alloc");                                                  \
  if(x->pList == NULL) {                                                                                                              \
    return MEM_ERR;                                                                                                                   \
  }                                                                                                                                   \
  x->nResizeAmt = resize_amt;		                                                                                              \
  return NO_ERROR;                                                                                                                    \
}                                                                                                                                     \
                                                                                                                                      \
uint8_t TYPE##_plist_resize(TYPE##_plist *x, uintmax_t new_length) {                                                                  \
  if(new_length > x->nLength_max) {                                                                                                   \
    uintmax_t delta = new_length - x->nLength_max;                                                                                    \
    if(delta < x->nResizeAmt) new_length = x->nLength_max + delta;                                                                    \
    x->pList = (TYPE **)sbsat_realloc((void*)x->pList, x->nLength_max, new_length, sizeof(TYPE *), 9, "TYPE##_plist_resize");         \
    if(x->pList==NULL) {                                                                                                              \
      return MEM_ERR;                                                                                                                 \
    }                                                                                                                                 \
    x->nLength_max=new_length;                                                                                                        \
  }                                                                                                                                   \
  return NO_ERROR;                                                                                                                    \
}                                                                                                                                     \
                                                                                                                                      \
uint8_t TYPE##_plist_push(TYPE##_plist *x, TYPE *value) {                                                                             \
  if(TYPE##_plist_resize(x, x->nLength+1) == MEM_ERR) return MEM_ERR;                                                                 \
  x->pList[x->nLength++] = value;                                                                                                     \
  return NO_ERROR;                                                                                                                    \
}                                                                                                                                     \
                                                                                                                                      \
uint8_t TYPE##_plist_add(TYPE##_plist *x, uintmax_t index, TYPE *value) {                                                             \
  if(TYPE##_plist_resize(x, index+1) == MEM_ERR) return MEM_ERR;                                                                      \
  x->pList[index] = value;                                                                                                            \
  return NO_ERROR;                                                                                                                    \
}                                                                                                                                     \
                                                                                                                                      \
uint8_t TYPE##_plist_copy(TYPE##_plist *dst, TYPE##_plist *src) {                                                                     \
  TYPE##_plist_resize(dst, src->nLength);                                                                                             \
  dst->nLength = src->nLength;                                                                                                        \
  memcpy(dst->pList, src->pList, dst->nLength * sizeof(TYPE *));                                                                      \
  return NO_ERROR;                                                                                                                    \
}                                                                                                                                     \
                                                                                                                                      \
uint8_t TYPE##_plist_free(TYPE##_plist *x) {                                                                                          \
  if(x->pList == NULL) return NO_ERROR;                                                                                               \
  sbsat_free((void**)&(x->pList));                                                                                                    \
  x->pList = NULL;                                                                                                                    \
  x->nLength = 0;                                                                                                                     \
  x->nLength_max = 0;                                                                                                                 \
  return NO_ERROR;                                                                                                                    \
}                                                                                                                                     \

#endif
