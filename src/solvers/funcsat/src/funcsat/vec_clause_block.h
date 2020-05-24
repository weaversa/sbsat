/* struct clause_block -- element type
   clause_block -- used to name all operations
*/

#ifndef vec_clause_block_h_included
#define vec_clause_block_h_included

#include <inttypes.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct vec_clause_block
{
  struct clause_block *data;

  uintptr_t size;
  uintptr_t capacity;
};

/**
 * Iterate over a struct vec_clause_block. `elt' will be declared as an
 * identifier of type `struct clause_block *' in the body of the iterator.
 *
 *   for_vec_bv(elt, vec) {
 *     ... do something with *elt ...
 *   }
 */
#define for_vec_clause_block(elt, vec)                                   \
  for (struct clause_block *elt = (vec)->data; elt != ((vec)->data + (vec)->size); elt++)

/**
 * Same as the for_vec_clause_block variant except declares the given type in the
 * clause head and casts the data pointer as that type. The 99 refers to
 * C99-style declaration. (shrug)
 */
#define for_vec_clause_block99(ty, elt, vec)                             \
  for (ty elt = (ty)(vec)->data; elt != (ty)((vec)->data + (vec)->size); elt++)

/**
 * Same arguments as for_vec_clause_block, but iterates over elements in the
 * reverse order.
 */
#define for_vec_clause_block_rev(elt, vec)                               \
  for (struct clause_block *elt = ((vec)->data + (vec)->size - 1); elt != ((vec)->data - 1); elt--)

/**
 * Same as the for_vec_clause_block_rev variant except declares the given type in
 * the clause head and casts the data pointer as that type. The 99 refers to
 * C99-style declaration. (shrug)
 */
#define for_vec_clause_block_rev99(ty, elt, vec)                         \
  for (ty elt = (ty)((vec)->data + (vec)->size - 1); elt != (ty)((vec)->data - 1); elt--)



static inline struct vec_clause_block *vec_clause_block_create(uintptr_t capacity);
#define vec_clause_block_init vec_clause_block_create
static inline void vec_clause_block_destroy(struct vec_clause_block *v);
static inline void vec_clause_block_mk(struct vec_clause_block *v, uintptr_t capacity);
static inline void vec_clause_block_unmk(struct vec_clause_block *v);
static inline void vec_clause_block_setall(struct vec_clause_block *v, struct clause_block val);
/**
 * Sets the size to 0.
 */
static inline void vec_clause_block_clear(struct vec_clause_block *v);
/**
 * Sets every location (0 to size-1) to 0.
 */
static inline void vec_clause_block_zero(struct vec_clause_block *v);
static inline void vec_clause_block_push(struct vec_clause_block *v, struct clause_block data);
static inline void vec_clause_block_push_at(struct vec_clause_block *v, struct clause_block data, uintptr_t i);
/* returns the old capacity */
static inline uintptr_t vec_clause_block_grow_to(struct vec_clause_block *v, uintptr_t newCapacity);
static inline struct clause_block vec_clause_block_pop(struct vec_clause_block *v);
static inline struct clause_block vec_clause_block_pop_at(struct vec_clause_block *v, uintptr_t i);
static inline struct clause_block vec_clause_block_peek(struct vec_clause_block *v);
#define vec_clause_block_get(v,i) ((v)->data[(i)])
static inline void vec_clause_block_set(struct vec_clause_block *v, uintptr_t i, struct clause_block p);
static inline void vec_clause_block_copy(struct vec_clause_block *dst, struct vec_clause_block *src);

static inline struct vec_clause_block *vec_clause_block_create(uintptr_t capacity)
{
  struct vec_clause_block *r = calloc(1, sizeof(*r));
  vec_clause_block_mk(r, capacity);
  return r;
}

static inline void vec_clause_block_destroy(struct vec_clause_block *v)
{
  vec_clause_block_unmk(v);
  free(v);
}

static inline void vec_clause_block_mk(struct vec_clause_block *v, uintptr_t capacity)
{
  uintptr_t c = capacity > 0 ? capacity : 4;
  v->data = calloc(c, sizeof(struct clause_block));
  v->size = 0;
  v->capacity = c;
}

static inline void vec_clause_block_unmk(struct vec_clause_block *v)
{
  free(v->data);
  v->data = NULL;
  v->size = 0;
  v->capacity = 0;
}

static inline void vec_clause_block_clear(struct vec_clause_block *v)
{
  v->size = 0;
}

static inline void vec_clause_block_zero(struct vec_clause_block *v)
{
  memset(v->data, 0, sizeof(struct clause_block)*v->size);
}

static inline void vec_clause_block_setall(struct vec_clause_block *v, struct clause_block val)
{
  for (uintptr_t i = 0; i < v->size; i++) {
    v->data[i] = val;
  }
}

static inline void vec_clause_block_push(struct vec_clause_block *v, struct clause_block data)
{
  if (v->capacity <= v->size) {
    if (v->capacity == 0) v->capacity = 1;
    while (v->capacity <= v->size) {
      v->capacity = v->capacity*2;
    }
    void *tmp = realloc(v->data, v->capacity*sizeof(struct clause_block));
    v->data = tmp;
  }
  v->data[v->size++] = data;
}

static inline void vec_clause_block_push_at(struct vec_clause_block *v, struct clause_block data, uintptr_t i)
{
  uintptr_t j;
  assert(i <= v->size);
  if (v->capacity <= v->size) {
    while (v->capacity <= v->size) {
      v->capacity = v->capacity*2;
    }
    void *tmp = realloc(v->data, v->capacity*sizeof(struct clause_block));
    v->data = tmp;
  }
  v->size++;
  for (j = v->size-1; j > i; j--) {
    v->data[j] = v->data[j-1];
  }
  v->data[i] = data;
}


static inline uintptr_t vec_clause_block_grow_to(struct vec_clause_block *v, uintptr_t newCapacity)
{
  uintptr_t ret = v->capacity;
  if (v->capacity < newCapacity) {
    v->capacity = newCapacity;
    v->data = realloc(v->data, v->capacity*sizeof(struct clause_block));
  }
  assert(v->capacity >= newCapacity);
  return ret;
}


static inline struct clause_block vec_clause_block_pop(struct vec_clause_block *v)
{
  assert(v->size != 0);
  return v->data[v->size-- - 1];
}

static inline struct clause_block vec_clause_block_pop_at(struct vec_clause_block *v, uintptr_t i)
{
  uintptr_t j;
  assert(v->size != 0);
  struct clause_block res = v->data[i];
  for (j = i; j < v->size-1; j++) {
    v->data[j] = v->data[j+1];
  }
  v->size--;
  return res;
}

static inline struct clause_block vec_clause_block_peek(struct vec_clause_block *v)
{
  assert(v->size != 0);
  return v->data[v->size-1];
}

static inline void vec_clause_block_set(struct vec_clause_block *v, uintptr_t i, struct clause_block p)
{
  v->data[i] = p;
}

static inline void vec_clause_block_copy(struct vec_clause_block *dst, struct vec_clause_block *src)
{
  for (uintptr_t i = 0; i < src->size; i++) {
    vec_clause_block_push(dst, src->data[i]);
  }
}

#endif
