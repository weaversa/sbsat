/* struct reason_info -- element type
   reason_info -- used to name all operations
*/

#ifndef vec_reason_info_h_included
#define vec_reason_info_h_included

#include <inttypes.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct vec_reason_info
{
  struct reason_info *data;

  uintptr_t size;
  uintptr_t capacity;
};

/**
 * Iterate over a struct vec_reason_info. `elt' will be declared as an
 * identifier of type `struct reason_info *' in the body of the iterator.
 *
 *   for_vec_bv(elt, vec) {
 *     ... do something with *elt ...
 *   }
 */
#define for_vec_reason_info(elt, vec)                                   \
  for (struct reason_info *elt = (vec)->data; elt != ((vec)->data + (vec)->size); elt++)

/**
 * Same as the for_vec_reason_info variant except declares the given type in the
 * clause head and casts the data pointer as that type. The 99 refers to
 * C99-style declaration. (shrug)
 */
#define for_vec_reason_info99(ty, elt, vec)                             \
  for (ty elt = (ty)(vec)->data; elt != (ty)((vec)->data + (vec)->size); elt++)

/**
 * Same arguments as for_vec_reason_info, but iterates over elements in the
 * reverse order.
 */
#define for_vec_reason_info_rev(elt, vec)                               \
  for (struct reason_info *elt = ((vec)->data + (vec)->size - 1); elt != ((vec)->data - 1); elt--)

/**
 * Same as the for_vec_reason_info_rev variant except declares the given type in
 * the clause head and casts the data pointer as that type. The 99 refers to
 * C99-style declaration. (shrug)
 */
#define for_vec_reason_info_rev99(ty, elt, vec)                         \
  for (ty elt = (ty)((vec)->data + (vec)->size - 1); elt != (ty)((vec)->data - 1); elt--)



static inline struct vec_reason_info *vec_reason_info_create(uintptr_t capacity);
#define vec_reason_info_init vec_reason_info_create
static inline void vec_reason_info_destroy(struct vec_reason_info *v);
static inline void vec_reason_info_mk(struct vec_reason_info *v, uintptr_t capacity);
static inline void vec_reason_info_unmk(struct vec_reason_info *v);
static inline void vec_reason_info_setall(struct vec_reason_info *v, struct reason_info val);
/**
 * Sets the size to 0.
 */
static inline void vec_reason_info_clear(struct vec_reason_info *v);
/**
 * Sets every location (0 to size-1) to 0.
 */
static inline void vec_reason_info_zero(struct vec_reason_info *v);
static inline void vec_reason_info_push(struct vec_reason_info *v, struct reason_info data);
static inline void vec_reason_info_push_at(struct vec_reason_info *v, struct reason_info data, uintptr_t i);
/* returns the old capacity */
static inline uintptr_t vec_reason_info_grow_to(struct vec_reason_info *v, uintptr_t newCapacity);
static inline struct reason_info vec_reason_info_pop(struct vec_reason_info *v);
static inline struct reason_info vec_reason_info_pop_at(struct vec_reason_info *v, uintptr_t i);
static inline struct reason_info vec_reason_info_peek(struct vec_reason_info *v);
#define vec_reason_info_get(v,i) ((v)->data[(i)])
static inline void vec_reason_info_set(struct vec_reason_info *v, uintptr_t i, struct reason_info p);
static inline void vec_reason_info_copy(struct vec_reason_info *dst, struct vec_reason_info *src);

static inline struct vec_reason_info *vec_reason_info_create(uintptr_t capacity)
{
  struct vec_reason_info *r = calloc(1, sizeof(*r));
  vec_reason_info_mk(r, capacity);
  return r;
}

static inline void vec_reason_info_destroy(struct vec_reason_info *v)
{
  vec_reason_info_unmk(v);
  free(v);
}

static inline void vec_reason_info_mk(struct vec_reason_info *v, uintptr_t capacity)
{
  uintptr_t c = capacity > 0 ? capacity : 4;
  v->data = calloc(c, sizeof(struct reason_info));
  v->size = 0;
  v->capacity = c;
}

static inline void vec_reason_info_unmk(struct vec_reason_info *v)
{
  free(v->data);
  v->data = NULL;
  v->size = 0;
  v->capacity = 0;
}

static inline void vec_reason_info_clear(struct vec_reason_info *v)
{
  v->size = 0;
}

static inline void vec_reason_info_zero(struct vec_reason_info *v)
{
  memset(v->data, 0, sizeof(struct reason_info)*v->size);
}

static inline void vec_reason_info_setall(struct vec_reason_info *v, struct reason_info val)
{
  for (uintptr_t i = 0; i < v->size; i++) {
    v->data[i] = val;
  }
}

static inline void vec_reason_info_push(struct vec_reason_info *v, struct reason_info data)
{
  if (v->capacity <= v->size) {
    if (v->capacity == 0) v->capacity = 1;
    while (v->capacity <= v->size) {
      v->capacity = v->capacity*2;
    }
    void *tmp = realloc(v->data, v->capacity*sizeof(struct reason_info));
    v->data = tmp;
  }
  v->data[v->size++] = data;
}

static inline void vec_reason_info_push_at(struct vec_reason_info *v, struct reason_info data, uintptr_t i)
{
  uintptr_t j;
  assert(i <= v->size);
  if (v->capacity <= v->size) {
    while (v->capacity <= v->size) {
      v->capacity = v->capacity*2;
    }
    void *tmp = realloc(v->data, v->capacity*sizeof(struct reason_info));
    v->data = tmp;
  }
  v->size++;
  for (j = v->size-1; j > i; j--) {
    v->data[j] = v->data[j-1];
  }
  v->data[i] = data;
}


static inline uintptr_t vec_reason_info_grow_to(struct vec_reason_info *v, uintptr_t newCapacity)
{
  uintptr_t ret = v->capacity;
  if (v->capacity < newCapacity) {
    v->capacity = newCapacity;
    v->data = realloc(v->data, v->capacity*sizeof(struct reason_info));
  }
  assert(v->capacity >= newCapacity);
  return ret;
}


static inline struct reason_info vec_reason_info_pop(struct vec_reason_info *v)
{
  assert(v->size != 0);
  return v->data[v->size-- - 1];
}

static inline struct reason_info vec_reason_info_pop_at(struct vec_reason_info *v, uintptr_t i)
{
  uintptr_t j;
  assert(v->size != 0);
  struct reason_info res = v->data[i];
  for (j = i; j < v->size-1; j++) {
    v->data[j] = v->data[j+1];
  }
  v->size--;
  return res;
}

static inline struct reason_info vec_reason_info_peek(struct vec_reason_info *v)
{
  assert(v->size != 0);
  return v->data[v->size-1];
}

static inline void vec_reason_info_set(struct vec_reason_info *v, uintptr_t i, struct reason_info p)
{
  v->data[i] = p;
}

static inline void vec_reason_info_copy(struct vec_reason_info *dst, struct vec_reason_info *src)
{
  for (uintptr_t i = 0; i < src->size; i++) {
    vec_reason_info_push(dst, src->data[i]);
  }
}

#endif
