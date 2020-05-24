/* uintptr_t -- element type
   uintptr -- used to name all operations
*/

#ifndef vec_uintptr_h_included
#define vec_uintptr_h_included

#include <inttypes.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct vec_uintptr
{
  uintptr_t *data;

  uintptr_t size;
  uintptr_t capacity;
};

/**
 * Iterate over a struct vec_uintptr. `elt' will be declared as an
 * identifier of type `uintptr_t *' in the body of the iterator.
 *
 *   for_vec_bv(elt, vec) {
 *     ... do something with *elt ...
 *   }
 */
#define for_vec_uintptr(elt, vec)                                   \
  for (uintptr_t *elt = (vec)->data; elt != ((vec)->data + (vec)->size); elt++)

/**
 * Same as the for_vec_uintptr variant except declares the given type in the
 * clause head and casts the data pointer as that type. The 99 refers to
 * C99-style declaration. (shrug)
 */
#define for_vec_uintptr99(ty, elt, vec)                             \
  for (ty elt = (ty)(vec)->data; elt != (ty)((vec)->data + (vec)->size); elt++)

/**
 * Same arguments as for_vec_uintptr, but iterates over elements in the
 * reverse order.
 */
#define for_vec_uintptr_rev(elt, vec)                               \
  for (uintptr_t *elt = ((vec)->data + (vec)->size - 1); elt != ((vec)->data - 1); elt--)

/**
 * Same as the for_vec_uintptr_rev variant except declares the given type in
 * the clause head and casts the data pointer as that type. The 99 refers to
 * C99-style declaration. (shrug)
 */
#define for_vec_uintptr_rev99(ty, elt, vec)                         \
  for (ty elt = (ty)((vec)->data + (vec)->size - 1); elt != (ty)((vec)->data - 1); elt--)



static inline struct vec_uintptr *vec_uintptr_create(uintptr_t capacity);
#define vec_uintptr_init vec_uintptr_create
static inline void vec_uintptr_destroy(struct vec_uintptr *v);
static inline void vec_uintptr_mk(struct vec_uintptr *v, uintptr_t capacity);
static inline void vec_uintptr_unmk(struct vec_uintptr *v);
static inline void vec_uintptr_setall(struct vec_uintptr *v, uintptr_t val);
/**
 * Sets the size to 0.
 */
static inline void vec_uintptr_clear(struct vec_uintptr *v);
/**
 * Sets every location (0 to size-1) to 0.
 */
static inline void vec_uintptr_zero(struct vec_uintptr *v);
static inline void vec_uintptr_push(struct vec_uintptr *v, uintptr_t data);
static inline void vec_uintptr_push_at(struct vec_uintptr *v, uintptr_t data, uintptr_t i);
/* returns the old capacity */
static inline uintptr_t vec_uintptr_grow_to(struct vec_uintptr *v, uintptr_t newCapacity);
static inline uintptr_t vec_uintptr_pop(struct vec_uintptr *v);
static inline uintptr_t vec_uintptr_pop_at(struct vec_uintptr *v, uintptr_t i);
static inline uintptr_t vec_uintptr_peek(struct vec_uintptr *v);
#define vec_uintptr_get(v,i) ((v)->data[(i)])
static inline void vec_uintptr_set(struct vec_uintptr *v, uintptr_t i, uintptr_t p);
static inline void vec_uintptr_copy(struct vec_uintptr *dst, struct vec_uintptr *src);

static inline struct vec_uintptr *vec_uintptr_create(uintptr_t capacity)
{
  struct vec_uintptr *r = calloc(1, sizeof(*r));
  vec_uintptr_mk(r, capacity);
  return r;
}

static inline void vec_uintptr_destroy(struct vec_uintptr *v)
{
  vec_uintptr_unmk(v);
  free(v);
}

static inline void vec_uintptr_mk(struct vec_uintptr *v, uintptr_t capacity)
{
  uintptr_t c = capacity > 0 ? capacity : 4;
  v->data = calloc(c, sizeof(uintptr_t));
  v->size = 0;
  v->capacity = c;
}

static inline void vec_uintptr_unmk(struct vec_uintptr *v)
{
  free(v->data);
  v->data = NULL;
  v->size = 0;
  v->capacity = 0;
}

static inline void vec_uintptr_clear(struct vec_uintptr *v)
{
  v->size = 0;
}

static inline void vec_uintptr_zero(struct vec_uintptr *v)
{
  memset(v->data, 0, sizeof(uintptr_t)*v->size);
}

static inline void vec_uintptr_setall(struct vec_uintptr *v, uintptr_t val)
{
  for (uintptr_t i = 0; i < v->size; i++) {
    v->data[i] = val;
  }
}

static inline void vec_uintptr_push(struct vec_uintptr *v, uintptr_t data)
{
  if (v->capacity <= v->size) {
    if (v->capacity == 0) v->capacity = 1;
    while (v->capacity <= v->size) {
      v->capacity = v->capacity*2;
    }
    void *tmp = realloc(v->data, v->capacity*sizeof(uintptr_t));
    v->data = tmp;
  }
  v->data[v->size++] = data;
}

static inline void vec_uintptr_push_at(struct vec_uintptr *v, uintptr_t data, uintptr_t i)
{
  uintptr_t j;
  assert(i <= v->size);
  if (v->capacity <= v->size) {
    while (v->capacity <= v->size) {
      v->capacity = v->capacity*2;
    }
    void *tmp = realloc(v->data, v->capacity*sizeof(uintptr_t));
    v->data = tmp;
  }
  v->size++;
  for (j = v->size-1; j > i; j--) {
    v->data[j] = v->data[j-1];
  }
  v->data[i] = data;
}


static inline uintptr_t vec_uintptr_grow_to(struct vec_uintptr *v, uintptr_t newCapacity)
{
  uintptr_t ret = v->capacity;
  if (v->capacity < newCapacity) {
    v->capacity = newCapacity;
    v->data = realloc(v->data, v->capacity*sizeof(uintptr_t));
  }
  assert(v->capacity >= newCapacity);
  return ret;
}


static inline uintptr_t vec_uintptr_pop(struct vec_uintptr *v)
{
  assert(v->size != 0);
  return v->data[v->size-- - 1];
}

static inline uintptr_t vec_uintptr_pop_at(struct vec_uintptr *v, uintptr_t i)
{
  uintptr_t j;
  assert(v->size != 0);
  uintptr_t res = v->data[i];
  for (j = i; j < v->size-1; j++) {
    v->data[j] = v->data[j+1];
  }
  v->size--;
  return res;
}

static inline uintptr_t vec_uintptr_peek(struct vec_uintptr *v)
{
  assert(v->size != 0);
  return v->data[v->size-1];
}

static inline void vec_uintptr_set(struct vec_uintptr *v, uintptr_t i, uintptr_t p)
{
  v->data[i] = p;
}

static inline void vec_uintptr_copy(struct vec_uintptr *dst, struct vec_uintptr *src)
{
  for (uintptr_t i = 0; i < src->size; i++) {
    vec_uintptr_push(dst, src->data[i]);
  }
}

#endif
