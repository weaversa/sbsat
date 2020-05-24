/* struct clause_head -- element type
   clause_head -- used to name all operations
*/

#ifndef vec_clause_head_h_included
#define vec_clause_head_h_included

#include <inttypes.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct vec_clause_head
{
  struct clause_head *data;

  uintptr_t size;
  uintptr_t capacity;
};

/**
 * Iterate over a struct vec_clause_head. `elt' will be declared as an
 * identifier of type `struct clause_head *' in the body of the iterator.
 *
 *   for_vec_bv(elt, vec) {
 *     ... do something with *elt ...
 *   }
 */
#define for_vec_clause_head(elt, vec)                                   \
  for (struct clause_head *elt = (vec)->data; elt != ((vec)->data + (vec)->size); elt++)

/**
 * Same as the for_vec_clause_head variant except declares the given type in the
 * clause head and casts the data pointer as that type. The 99 refers to
 * C99-style declaration. (shrug)
 */
#define for_vec_clause_head99(ty, elt, vec)                             \
  for (ty elt = (ty)(vec)->data; elt != (ty)((vec)->data + (vec)->size); elt++)

/**
 * Same arguments as for_vec_clause_head, but iterates over elements in the
 * reverse order.
 */
#define for_vec_clause_head_rev(elt, vec)                               \
  for (struct clause_head *elt = ((vec)->data + (vec)->size - 1); elt != ((vec)->data - 1); elt--)

/**
 * Same as the for_vec_clause_head_rev variant except declares the given type in
 * the clause head and casts the data pointer as that type. The 99 refers to
 * C99-style declaration. (shrug)
 */
#define for_vec_clause_head_rev99(ty, elt, vec)                         \
  for (ty elt = (ty)((vec)->data + (vec)->size - 1); elt != (ty)((vec)->data - 1); elt--)



static inline struct vec_clause_head *vec_clause_head_create(uintptr_t capacity);
#define vec_clause_head_init vec_clause_head_create
static inline void vec_clause_head_destroy(struct vec_clause_head *v);
static inline void vec_clause_head_mk(struct vec_clause_head *v, uintptr_t capacity);
static inline void vec_clause_head_unmk(struct vec_clause_head *v);
static inline void vec_clause_head_setall(struct vec_clause_head *v, struct clause_head val);
/**
 * Sets the size to 0.
 */
static inline void vec_clause_head_clear(struct vec_clause_head *v);
/**
 * Sets every location (0 to size-1) to 0.
 */
static inline void vec_clause_head_zero(struct vec_clause_head *v);
static inline void vec_clause_head_push(struct vec_clause_head *v, struct clause_head data);
static inline void vec_clause_head_push_at(struct vec_clause_head *v, struct clause_head data, uintptr_t i);
/* returns the old capacity */
static inline uintptr_t vec_clause_head_grow_to(struct vec_clause_head *v, uintptr_t newCapacity);
static inline struct clause_head vec_clause_head_pop(struct vec_clause_head *v);
static inline struct clause_head vec_clause_head_pop_at(struct vec_clause_head *v, uintptr_t i);
static inline struct clause_head vec_clause_head_peek(struct vec_clause_head *v);
#define vec_clause_head_get(v,i) ((v)->data[(i)])
static inline void vec_clause_head_set(struct vec_clause_head *v, uintptr_t i, struct clause_head p);
static inline void vec_clause_head_copy(struct vec_clause_head *dst, struct vec_clause_head *src);

static inline struct vec_clause_head *vec_clause_head_create(uintptr_t capacity)
{
  struct vec_clause_head *r = calloc(1, sizeof(*r));
  vec_clause_head_mk(r, capacity);
  return r;
}

static inline void vec_clause_head_destroy(struct vec_clause_head *v)
{
  vec_clause_head_unmk(v);
  free(v);
}

static inline void vec_clause_head_mk(struct vec_clause_head *v, uintptr_t capacity)
{
  uintptr_t c = capacity > 0 ? capacity : 4;
  v->data = calloc(c, sizeof(struct clause_head));
  v->size = 0;
  v->capacity = c;
}

static inline void vec_clause_head_unmk(struct vec_clause_head *v)
{
  free(v->data);
  v->data = NULL;
  v->size = 0;
  v->capacity = 0;
}

static inline void vec_clause_head_clear(struct vec_clause_head *v)
{
  v->size = 0;
}

static inline void vec_clause_head_zero(struct vec_clause_head *v)
{
  memset(v->data, 0, sizeof(struct clause_head)*v->size);
}

static inline void vec_clause_head_setall(struct vec_clause_head *v, struct clause_head val)
{
  for (uintptr_t i = 0; i < v->size; i++) {
    v->data[i] = val;
  }
}

static inline void vec_clause_head_push(struct vec_clause_head *v, struct clause_head data)
{
  if (v->capacity <= v->size) {
    if (v->capacity == 0) v->capacity = 1;
    while (v->capacity <= v->size) {
      v->capacity = v->capacity*2;
    }
    void *tmp = realloc(v->data, v->capacity*sizeof(struct clause_head));
    v->data = tmp;
  }
  v->data[v->size++] = data;
}

static inline void vec_clause_head_push_at(struct vec_clause_head *v, struct clause_head data, uintptr_t i)
{
  uintptr_t j;
  assert(i <= v->size);
  if (v->capacity <= v->size) {
    while (v->capacity <= v->size) {
      v->capacity = v->capacity*2;
    }
    void *tmp = realloc(v->data, v->capacity*sizeof(struct clause_head));
    v->data = tmp;
  }
  v->size++;
  for (j = v->size-1; j > i; j--) {
    v->data[j] = v->data[j-1];
  }
  v->data[i] = data;
}


static inline uintptr_t vec_clause_head_grow_to(struct vec_clause_head *v, uintptr_t newCapacity)
{
  uintptr_t ret = v->capacity;
  if (v->capacity < newCapacity) {
    v->capacity = newCapacity;
    v->data = realloc(v->data, v->capacity*sizeof(struct clause_head));
  }
  assert(v->capacity >= newCapacity);
  return ret;
}


static inline struct clause_head vec_clause_head_pop(struct vec_clause_head *v)
{
  assert(v->size != 0);
  return v->data[v->size-- - 1];
}

static inline struct clause_head vec_clause_head_pop_at(struct vec_clause_head *v, uintptr_t i)
{
  uintptr_t j;
  assert(v->size != 0);
  struct clause_head res = v->data[i];
  for (j = i; j < v->size-1; j++) {
    v->data[j] = v->data[j+1];
  }
  v->size--;
  return res;
}

static inline struct clause_head vec_clause_head_peek(struct vec_clause_head *v)
{
  assert(v->size != 0);
  return v->data[v->size-1];
}

static inline void vec_clause_head_set(struct vec_clause_head *v, uintptr_t i, struct clause_head p)
{
  v->data[i] = p;
}

static inline void vec_clause_head_copy(struct vec_clause_head *dst, struct vec_clause_head *src)
{
  for (uintptr_t i = 0; i < src->size; i++) {
    vec_clause_head_push(dst, src->data[i]);
  }
}

#endif
