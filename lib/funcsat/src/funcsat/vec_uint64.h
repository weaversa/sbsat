/* uint64_t -- element type
   uint64 -- used to name all operations
*/

#ifndef vec_uint64_h_included
#define vec_uint64_h_included

#include <inttypes.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct vec_uint64
{
  uint64_t *data;

  uintptr_t size;
  uintptr_t capacity;
};

/**
 * Iterate over a struct vec_uint64. `elt' will be declared as an
 * identifier of type `uint64_t *' in the body of the iterator.
 *
 *   for_vec_bv(elt, vec) {
 *     ... do something with *elt ...
 *   }
 */
#define for_vec_uint64(elt, vec)                                   \
  for (uint64_t *elt = (vec)->data; elt != ((vec)->data + (vec)->size); elt++)

/**
 * Same as the for_vec_uint64 variant except declares the given type in the
 * clause head and casts the data pointer as that type. The 99 refers to
 * C99-style declaration. (shrug)
 */
#define for_vec_uint6499(ty, elt, vec)                             \
  for (ty elt = (ty)(vec)->data; elt != (ty)((vec)->data + (vec)->size); elt++)

/**
 * Same arguments as for_vec_uint64, but iterates over elements in the
 * reverse order.
 */
#define for_vec_uint64_rev(elt, vec)                               \
  for (uint64_t *elt = ((vec)->data + (vec)->size - 1); elt != ((vec)->data - 1); elt--)

/**
 * Same as the for_vec_uint64_rev variant except declares the given type in
 * the clause head and casts the data pointer as that type. The 99 refers to
 * C99-style declaration. (shrug)
 */
#define for_vec_uint64_rev99(ty, elt, vec)                         \
  for (ty elt = (ty)((vec)->data + (vec)->size - 1); elt != (ty)((vec)->data - 1); elt--)



static inline struct vec_uint64 *vec_uint64_create(uintptr_t capacity);
#define vec_uint64_init vec_uint64_create
static inline void vec_uint64_destroy(struct vec_uint64 *v);
static inline void vec_uint64_mk(struct vec_uint64 *v, uintptr_t capacity);
static inline void vec_uint64_unmk(struct vec_uint64 *v);
static inline void vec_uint64_setall(struct vec_uint64 *v, uint64_t val);
/**
 * Sets the size to 0.
 */
static inline void vec_uint64_clear(struct vec_uint64 *v);
/**
 * Sets every location (0 to size-1) to 0.
 */
static inline void vec_uint64_zero(struct vec_uint64 *v);
static inline void vec_uint64_push(struct vec_uint64 *v, uint64_t data);
static inline void vec_uint64_push_at(struct vec_uint64 *v, uint64_t data, uintptr_t i);
/* returns the old capacity */
static inline uintptr_t vec_uint64_grow_to(struct vec_uint64 *v, uintptr_t newCapacity);
static inline uint64_t vec_uint64_pop(struct vec_uint64 *v);
static inline uint64_t vec_uint64_pop_at(struct vec_uint64 *v, uintptr_t i);
static inline uint64_t vec_uint64_peek(struct vec_uint64 *v);
#define vec_uint64_get(v,i) ((v)->data[(i)])
static inline void vec_uint64_set(struct vec_uint64 *v, uintptr_t i, uint64_t p);
static inline void vec_uint64_copy(struct vec_uint64 *dst, struct vec_uint64 *src);

static inline struct vec_uint64 *vec_uint64_create(uintptr_t capacity)
{
  struct vec_uint64 *r = calloc(1, sizeof(*r));
  vec_uint64_mk(r, capacity);
  return r;
}

static inline void vec_uint64_destroy(struct vec_uint64 *v)
{
  vec_uint64_unmk(v);
  free(v);
}

static inline void vec_uint64_mk(struct vec_uint64 *v, uintptr_t capacity)
{
  uintptr_t c = capacity > 0 ? capacity : 4;
  v->data = calloc(c, sizeof(uint64_t));
  v->size = 0;
  v->capacity = c;
}

static inline void vec_uint64_unmk(struct vec_uint64 *v)
{
  free(v->data);
  v->data = NULL;
  v->size = 0;
  v->capacity = 0;
}

static inline void vec_uint64_clear(struct vec_uint64 *v)
{
  v->size = 0;
}

static inline void vec_uint64_zero(struct vec_uint64 *v)
{
  memset(v->data, 0, sizeof(uint64_t)*v->size);
}

static inline void vec_uint64_setall(struct vec_uint64 *v, uint64_t val)
{
  for (uintptr_t i = 0; i < v->size; i++) {
    v->data[i] = val;
  }
}

static inline void vec_uint64_push(struct vec_uint64 *v, uint64_t data)
{
  if (v->capacity <= v->size) {
    if (v->capacity == 0) v->capacity = 1;
    while (v->capacity <= v->size) {
      v->capacity = v->capacity*2;
    }
    void *tmp = realloc(v->data, v->capacity*sizeof(uint64_t));
    v->data = tmp;
  }
  v->data[v->size++] = data;
}

static inline void vec_uint64_push_at(struct vec_uint64 *v, uint64_t data, uintptr_t i)
{
  uintptr_t j;
  assert(i <= v->size);
  if (v->capacity <= v->size) {
    while (v->capacity <= v->size) {
      v->capacity = v->capacity*2;
    }
    void *tmp = realloc(v->data, v->capacity*sizeof(uint64_t));
    v->data = tmp;
  }
  v->size++;
  for (j = v->size-1; j > i; j--) {
    v->data[j] = v->data[j-1];
  }
  v->data[i] = data;
}


static inline uintptr_t vec_uint64_grow_to(struct vec_uint64 *v, uintptr_t newCapacity)
{
  uintptr_t ret = v->capacity;
  if (v->capacity < newCapacity) {
    v->capacity = newCapacity;
    v->data = realloc(v->data, v->capacity*sizeof(uint64_t));
  }
  assert(v->capacity >= newCapacity);
  return ret;
}


static inline uint64_t vec_uint64_pop(struct vec_uint64 *v)
{
  assert(v->size != 0);
  return v->data[v->size-- - 1];
}

static inline uint64_t vec_uint64_pop_at(struct vec_uint64 *v, uintptr_t i)
{
  uintptr_t j;
  assert(v->size != 0);
  uint64_t res = v->data[i];
  for (j = i; j < v->size-1; j++) {
    v->data[j] = v->data[j+1];
  }
  v->size--;
  return res;
}

static inline uint64_t vec_uint64_peek(struct vec_uint64 *v)
{
  assert(v->size != 0);
  return v->data[v->size-1];
}

static inline void vec_uint64_set(struct vec_uint64 *v, uintptr_t i, uint64_t p)
{
  v->data[i] = p;
}

static inline void vec_uint64_copy(struct vec_uint64 *dst, struct vec_uint64 *src)
{
  for (uintptr_t i = 0; i < src->size; i++) {
    vec_uint64_push(dst, src->data[i]);
  }
}

#endif
