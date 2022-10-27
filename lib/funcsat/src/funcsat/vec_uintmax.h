/* uintmax_t -- element type
   uintmax -- used to name all operations
*/

#ifndef vec_uintmax_h_included
#define vec_uintmax_h_included

#include <inttypes.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct vec_uintmax
{
  uintmax_t *data;

  uintptr_t size;
  uintptr_t capacity;
};

/**
 * Iterate over a struct vec_uintmax. `elt' will be declared as an
 * identifier of type `uintmax_t *' in the body of the iterator.
 *
 *   for_vec_bv(elt, vec) {
 *     ... do something with *elt ...
 *   }
 */
#define for_vec_uintmax(elt, vec)                                   \
  for (uintmax_t *elt = (vec)->data; elt != ((vec)->data + (vec)->size); elt++)

/**
 * Same as the for_vec_uintmax variant except declares the given type in the
 * clause head and casts the data pointer as that type. The 99 refers to
 * C99-style declaration. (shrug)
 */
#define for_vec_uintmax99(ty, elt, vec)                             \
  for (ty elt = (ty)(vec)->data; elt != (ty)((vec)->data + (vec)->size); elt++)

/**
 * Same arguments as for_vec_uintmax, but iterates over elements in the
 * reverse order.
 */
#define for_vec_uintmax_rev(elt, vec)                               \
  for (uintmax_t *elt = ((vec)->data + (vec)->size - 1); elt != ((vec)->data - 1); elt--)

/**
 * Same as the for_vec_uintmax_rev variant except declares the given type in
 * the clause head and casts the data pointer as that type. The 99 refers to
 * C99-style declaration. (shrug)
 */
#define for_vec_uintmax_rev99(ty, elt, vec)                         \
  for (ty elt = (ty)((vec)->data + (vec)->size - 1); elt != (ty)((vec)->data - 1); elt--)



static inline struct vec_uintmax *vec_uintmax_create(uintptr_t capacity);
#define vec_uintmax_init vec_uintmax_create
static inline void vec_uintmax_destroy(struct vec_uintmax *v);
static inline void vec_uintmax_mk(struct vec_uintmax *v, uintptr_t capacity);
static inline void vec_uintmax_unmk(struct vec_uintmax *v);
static inline void vec_uintmax_setall(struct vec_uintmax *v, uintmax_t val);
/**
 * Sets the size to 0.
 */
static inline void vec_uintmax_clear(struct vec_uintmax *v);
/**
 * Sets every location (0 to size-1) to 0.
 */
static inline void vec_uintmax_zero(struct vec_uintmax *v);
static inline void vec_uintmax_push(struct vec_uintmax *v, uintmax_t data);
static inline void vec_uintmax_push_at(struct vec_uintmax *v, uintmax_t data, uintptr_t i);
/* returns the old capacity */
static inline uintptr_t vec_uintmax_grow_to(struct vec_uintmax *v, uintptr_t newCapacity);
static inline uintmax_t vec_uintmax_pop(struct vec_uintmax *v);
static inline uintmax_t vec_uintmax_pop_at(struct vec_uintmax *v, uintptr_t i);
static inline uintmax_t vec_uintmax_peek(struct vec_uintmax *v);
#define vec_uintmax_get(v,i) ((v)->data[(i)])
static inline void vec_uintmax_set(struct vec_uintmax *v, uintptr_t i, uintmax_t p);
static inline void vec_uintmax_copy(struct vec_uintmax *dst, struct vec_uintmax *src);

static inline struct vec_uintmax *vec_uintmax_create(uintptr_t capacity)
{
  struct vec_uintmax *r = calloc(1, sizeof(*r));
  vec_uintmax_mk(r, capacity);
  return r;
}

static inline void vec_uintmax_destroy(struct vec_uintmax *v)
{
  vec_uintmax_unmk(v);
  free(v);
}

static inline void vec_uintmax_mk(struct vec_uintmax *v, uintptr_t capacity)
{
  uintptr_t c = capacity > 0 ? capacity : 4;
  v->data = calloc(c, sizeof(uintmax_t));
  v->size = 0;
  v->capacity = c;
}

static inline void vec_uintmax_unmk(struct vec_uintmax *v)
{
  free(v->data);
  v->data = NULL;
  v->size = 0;
  v->capacity = 0;
}

static inline void vec_uintmax_clear(struct vec_uintmax *v)
{
  v->size = 0;
}

static inline void vec_uintmax_zero(struct vec_uintmax *v)
{
  memset(v->data, 0, sizeof(uintmax_t)*v->size);
}

static inline void vec_uintmax_setall(struct vec_uintmax *v, uintmax_t val)
{
  for (uintptr_t i = 0; i < v->size; i++) {
    v->data[i] = val;
  }
}

static inline void vec_uintmax_push(struct vec_uintmax *v, uintmax_t data)
{
  if (v->capacity <= v->size) {
    if (v->capacity == 0) v->capacity = 1;
    while (v->capacity <= v->size) {
      v->capacity = v->capacity*2;
    }
    void *tmp = realloc(v->data, v->capacity*sizeof(uintmax_t));
    v->data = tmp;
  }
  v->data[v->size++] = data;
}

static inline void vec_uintmax_push_at(struct vec_uintmax *v, uintmax_t data, uintptr_t i)
{
  uintptr_t j;
  assert(i <= v->size);
  if (v->capacity <= v->size) {
    while (v->capacity <= v->size) {
      v->capacity = v->capacity*2;
    }
    void *tmp = realloc(v->data, v->capacity*sizeof(uintmax_t));
    v->data = tmp;
  }
  v->size++;
  for (j = v->size-1; j > i; j--) {
    v->data[j] = v->data[j-1];
  }
  v->data[i] = data;
}


static inline uintptr_t vec_uintmax_grow_to(struct vec_uintmax *v, uintptr_t newCapacity)
{
  uintptr_t ret = v->capacity;
  if (v->capacity < newCapacity) {
    v->capacity = newCapacity;
    v->data = realloc(v->data, v->capacity*sizeof(uintmax_t));
  }
  assert(v->capacity >= newCapacity);
  return ret;
}


static inline uintmax_t vec_uintmax_pop(struct vec_uintmax *v)
{
  assert(v->size != 0);
  return v->data[v->size-- - 1];
}

static inline uintmax_t vec_uintmax_pop_at(struct vec_uintmax *v, uintptr_t i)
{
  uintptr_t j;
  assert(v->size != 0);
  uintmax_t res = v->data[i];
  for (j = i; j < v->size-1; j++) {
    v->data[j] = v->data[j+1];
  }
  v->size--;
  return res;
}

static inline uintmax_t vec_uintmax_peek(struct vec_uintmax *v)
{
  assert(v->size != 0);
  return v->data[v->size-1];
}

static inline void vec_uintmax_set(struct vec_uintmax *v, uintptr_t i, uintmax_t p)
{
  v->data[i] = p;
}

static inline void vec_uintmax_copy(struct vec_uintmax *dst, struct vec_uintmax *src)
{
  for (uintptr_t i = 0; i < src->size; i++) {
    vec_uintmax_push(dst, src->data[i]);
  }
}

#endif
