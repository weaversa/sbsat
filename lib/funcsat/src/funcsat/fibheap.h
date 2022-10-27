#ifndef fibheap_h_included
#define fibheap_h_included

#include <inttypes.h>

#include "funcsat/system.h"

typedef double fibkey;
typedef uintmax_t fibelt;

struct fibheap;

struct fibnode
{
  fibkey key;
  
  struct fibnode *parent;
  struct fibnode *child;
  struct fibnode *left;
  struct fibnode *right;

  /**
   * Has this node lost a child sinc the last time it was made the child of
   * another node?
   */
  bool mark;
  bool isInHeap;

  intmax_t degree;

  fibelt data;
};

typedef struct fibheap fibheap;
typedef struct fibnode fibnode;


fibheap *fibheapInit(int (*compare)(fibkey *, fibkey *), fibkey minKey);
fibnode *fibheapMkNode(fibkey key, fibelt data);

/**
 * @return the number of elements in the heap
 */
intmax_t fibheapNum(fibheap *h);

/**
 * Insert a node into the heap.
 */
void fibheapInsert(fibheap *h, fibnode *n);

/**
 * Extract the minimum key'd element from the heap.
 */
fibnode *fibheapExtractMin(fibheap *h);

/**
 * "Decreases" the key for the given node.
 *
 * @param h, n
 * @param newKey Must be less than n->key according to the heap comparison
 * function passed to ::fibheapInit.
 */
void fibheapDecreaseKey(fibheap *h, fibnode *n, fibkey newKey);

/**
 * Deletes the node from the heap.
 *
 * Works by decreasing the key of the node to the minKey (see ::fibheapInit) and
 * then extracting the min.
 *
 * @pre n is in the heap
 * @pre n is, um, not in the heap
 */
void fibheapDelete(fibheap *h, fibnode *n);

void fibheapFreeNodes(fibnode *n);

#endif
