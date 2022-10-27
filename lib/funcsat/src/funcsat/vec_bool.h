/* bool -- element type
   bool -- used to name all operations
*/

#ifndef vec_bool_h_included
#define vec_bool_h_included

#include <inttypes.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct vec_bool
{
  bool *data;

  uintptr_t size;
  uintptr_t capacity;
};

/**
 * Iterate over a struct vec_bool. `elt' will be declared as an
 * identifier of type `bool *' in the body of the iterator.
 *
 *   for_vec_bv(elt, vec) {
 *     ... do something with *elt ...
 *   }
 */
#define for_vec_bool(elt, vec)                                   \
  for (bool *elt = (vec)->data; elt != ((vec)->data + (vec)->size); elt++)

/**
 * Same as the for_vec_bool variant except declares the given type in the
 * clause head and casts the data pointer as that type. The 99 refers to
 * C99-style declaration. (shrug)
 */
#define for_vec_bool99(ty, elt, vec)                             \
  for (ty elt = (ty)(vec)->data; elt != (ty)((vec)->data + (vec)->size); elt++)

/**
 * Same arguments as for_vec_bool, but iterates over elements in the
 * reverse order.
 */
#define for_vec_bool_rev(elt, vec)                               \
  for (bool *elt = ((vec)->data + (vec)->size - 1); elt != ((vec)->data - 1); elt--)

/**
 * Same as the for_vec_bool_rev variant except declares the given type in
 * the clause head and casts the data pointer as that type. The 99 refers to
 * C99-style declaration. (shrug)
 */
#define for_vec_bool_rev99(ty, elt, vec)                         \
  for (ty elt = (ty)((vec)->data + (vec)->size - 1); elt != (ty)((vec)->data - 1); elt--)



static inline struct vec_bool *vec_bool_create(uintptr_t capacity);
#define vec_bool_init vec_bool_create
static inline void vec_bool_destroy(struct vec_bool *v);
static inline void vec_bool_mk(struct vec_bool *v, uintptr_t capacity);
static inline void vec_bool_unmk(struct vec_bool *v);
static inline void vec_bool_setall(struct vec_bool *v, bool val);
/**
 * Sets the size to 0.
 */
static inline void vec_bool_clear(struct vec_bool *v);
/**
 * Sets every location (0 to size-1) to 0.
 */
static inline void vec_bool_zero(struct vec_bool *v);
static inline void vec_bool_push(struct vec_bool *v, bool data);
static inline void vec_bool_push_at(struct vec_bool *v, bool data, uintptr_t i);
/* returns the old capacity */
static inline uintptr_t vec_bool_grow_to(struct vec_bool *v, uintptr_t newCapacity);
static inline bool vec_bool_pop(struct vec_bool *v);
static inline bool vec_bool_pop_at(struct vec_bool *v, uintptr_t i);
static inline bool vec_bool_peek(struct vec_bool *v);
#define vec_bool_get(v,i) ((v)->data[(i)])
static inline void vec_bool_set(struct vec_bool *v, uintptr_t i, bool p);
static inline void vec_bool_copy(struct vec_bool *dst, struct vec_bool *src);

static inline struct vec_bool *vec_bool_create(uintptr_t capacity)
{
  struct vec_bool *r = calloc(1, sizeof(*r));
  vec_bool_mk(r, capacity);
  return r;
}

static inline void vec_bool_destroy(struct vec_bool *v)
{
  vec_bool_unmk(v);
  free(v);
}

static inline void vec_bool_mk(struct vec_bool *v, uintptr_t capacity)
{
  uintptr_t c = capacity > 0 ? capacity : 4;
  v->data = calloc(c, sizeof(bool));
  v->size = 0;
  v->capacity = c;
}

static inline void vec_bool_unmk(struct vec_bool *v)
{
  free(v->data);
  v->data = NULL;
  v->size = 0;
  v->capacity = 0;
}

static inline void vec_bool_clear(struct vec_bool *v)
{
  v->size = 0;
}

static inline void vec_bool_zero(struct vec_bool *v)
{
  memset(v->data, 0, sizeof(bool)*v->size);
}

static inline void vec_bool_setall(struct vec_bool *v, bool val)
{
  for (uintptr_t i = 0; i < v->size; i++) {
    v->data[i] = val;
  }
}

static inline void vec_bool_push(struct vec_bool *v, bool data)
{
  if (v->capacity <= v->size) {
    if (v->capacity == 0) v->capacity = 1;
    while (v->capacity <= v->size) {
      v->capacity = v->capacity*2;
    }
    void *tmp = realloc(v->data, v->capacity*sizeof(bool));
    v->data = tmp;
  }
  v->data[v->size++] = data;
}

static inline void vec_bool_push_at(struct vec_bool *v, bool data, uintptr_t i)
{
  uintptr_t j;
  assert(i <= v->size);
  if (v->capacity <= v->size) {
    while (v->capacity <= v->size) {
      v->capacity = v->capacity*2;
    }
    void *tmp = realloc(v->data, v->capacity*sizeof(bool));
    v->data = tmp;
  }
  v->size++;
  for (j = v->size-1; j > i; j--) {
    v->data[j] = v->data[j-1];
  }
  v->data[i] = data;
}


static inline uintptr_t vec_bool_grow_to(struct vec_bool *v, uintptr_t newCapacity)
{
  uintptr_t ret = v->capacity;
  if (v->capacity < newCapacity) {
    v->capacity = newCapacity;
    v->data = realloc(v->data, v->capacity*sizeof(bool));
  }
  assert(v->capacity >= newCapacity);
  return ret;
}


static inline bool vec_bool_pop(struct vec_bool *v)
{
  assert(v->size != 0);
  return v->data[v->size-- - 1];
}

static inline bool vec_bool_pop_at(struct vec_bool *v, uintptr_t i)
{
  uintptr_t j;
  assert(v->size != 0);
  bool res = v->data[i];
  for (j = i; j < v->size-1; j++) {
    v->data[j] = v->data[j+1];
  }
  v->size--;
  return res;
}

static inline bool vec_bool_peek(struct vec_bool *v)
{
  assert(v->size != 0);
  return v->data[v->size-1];
}

static inline void vec_bool_set(struct vec_bool *v, uintptr_t i, bool p)
{
  v->data[i] = p;
}

static inline void vec_bool_copy(struct vec_bool *dst, struct vec_bool *src)
{
  for (uintptr_t i = 0; i < src->size; i++) {
    vec_bool_push(dst, src->data[i]);
  }
}

#endif
