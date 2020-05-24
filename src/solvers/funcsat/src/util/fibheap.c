/*
 * Copyright 2012 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000, there is a non-exclusive license for use of this work by
 * or on behalf of the U.S. Government. Export of this program may require a
 * license from the United States Government.
 */
#include <funcsat/config.h>

#include <assert.h>
#include <math.h>

#include "funcsat/vec_ptr.h"
#include "funcsat.h"
#include "funcsat/fibheap.h"
#include "funcsat/fibheap_internal.h"

#define CALLOC(ptr, n, size)                   \
  do {                                         \
    ptr = calloc((n), (size));                 \
    if (!ptr) perror("CALLOC"), abort();       \
  } while (0);


static void rootlistAdd(fibheap *h, fibnode *n);
static void rootlistAddImmediateChildren(fibheap *h, fibnode *n);
static void consolidate(fibheap *h);

/**
 * Removes the child node n from its parent, fixing up the child pointer of the
 * parent to point to the proper value.
 *
 * @pre n must have a parent
 * @post n will be alone (see ::isAlone)
 * @post n's parent's child list will be intact, save without n
 * @post n's parent's degree was decremented
 */
static void unSpliceChild(fibnode *n);

static intmax_t maxDegree(fibheap *h);
static void link(fibnode *newChild, fibnode *parent);
static void fibheapCut(fibheap *h, fibnode *n, fibnode *m);
static void fibheapCascadingCut(fibheap *h, fibnode *m);




fibheap *fibheapInit(int (*compare)(fibkey *, fibkey *), fibkey minKey)
{
  fibheap *h;
  CALLOC(h, 1, sizeof(*h));
  h->compare = compare;
  h->minKey = minKey;
  return h;
}


fibnode *fibheapMkNode(fibkey key, fibelt data)
{
  fibnode *node;
  CALLOC(node, 1, sizeof(*node));
  node->key = key;
  node->data = data;
  node->parent = NULL;
  node->child = NULL;
  node->left = node;
  node->right = node;
  node->mark = false;
  node->degree = 0;
  node->isInHeap = false;
  return node;
}

intmax_t fibheapNum(fibheap *h)
{
  return h->num;
}

/************************************************************************/
                                  /* INSERT */
/************************************************************************/

void fibheapInsert(fibheap *h, fibnode *n)
{
  ++h->numInsert;
  assert(isAlone(n));
  assert(h->compare(&n->key, &h->minKey) >= 0);
  rootlistAdd(h,n);
  n->isInHeap = true;
  if (h->compare(&n->key, &h->min->key) < 0) {
    h->min = n;
  }
  h->num++;
}

/************************************************************************/
                               /* EXTRACT MIN */
/************************************************************************/

fibnode *fibheapExtractMin(fibheap *h)
{
  ++h->numExtract;
  assert(h->min);
  fibnode *min = h->min;
  rootlistAddImmediateChildren(h, min);

  if (isAlone(min)) {
    h->min = NULL;
    assert(h->num-1 == 0);
  } else {
    h->min = min->right;
    unSplice(min);
    consolidate(h);
  }
  h->num--;
  min->isInHeap = false;
  return min;
}

/************************************************************************/
                               /* DECREASE KEY */
/************************************************************************/

void fibheapDecreaseKey(fibheap *h, fibnode *n, fibkey newKey)
{
  ++h->numDecrease;
  if (!(h->compare(&newKey, &n->key) < 0)) {
    fprintf(stderr, "error in fibheapDecreaseKey: new key not 'smaller'\n");
    exit(1);
  }
  n->key = newKey;
  fibnode *y = n->parent;
  /* If n's parent has greater key than newKey, push n up. */
  if (y && h->compare(&n->key, &y->key) < 0) {
    fibheapCut(h, n, y);
    fibheapCascadingCut(h, y);
  }
  if (h->compare(&n->key, &h->min->key) < 0) {
    h->min = n;
  }
}

/************************************************************************/
                               /* DECREASE KEY */
/************************************************************************/

void fibheapDelete(fibheap *h, fibnode *n)
{
  ++h->numDelete;
  fibheapDecreaseKey(h, n, h->minKey);
  fibheapExtractMin(h);
}





/************************************************************************/
/* Low level heap surgery */
static void rootlistAdd(fibheap *h, fibnode *n)
{
  if (h->min) {
    fhsplice(n, h->min);
  } else {
    h->min = n;
  }
}


static void rootlistAddImmediateChildren(fibheap *h, fibnode *n)
{
  if (n->child) {
    fibnode *child, *start;
    child = start = n->child;
    do {
      child->parent = NULL;
      child = child->right;
    } while (child != start);
    rootlistAdd(h, child);
  }
  n->child = NULL;
  n->degree = 0;
}


static void consolidate(fibheap *h)
{
  fibnode **a, *curr = h->min;
  struct vec_ptr rootNodes;
  vec_ptr_mk(&rootNodes, 5);
  toVector(h->min, &rootNodes);
  CALLOC(a, (size_t) maxDegree(h)+1, sizeof(*a));

  /* This loop builds up a.  Postcondition: a[i] is the unique heap tree with
  degree i, if there is one. */
  while (rootNodes.size > 0) {
    curr = vec_ptr_pop(&rootNodes);
    intmax_t d = curr->degree;
    fibnode *x = curr;
    /* While there is another tree in the heap of the same degree, link them:
     * make one a child of the other, depending on the ordering. The linking
     * increases the degree of the linked tree; that's why we loop on d. */
    while (a[d]) {
      fibnode *y = a[d];     /* y has same degree as x */
      if (h->compare(&y->key, &x->key) < 0) {
        fibnode *tmp = x;
        x = y;
        y = tmp;
      }
      link(y, x);
      a[d++] = NULL;
    }
    unSplice(x);
    a[d] = x;
  }
  vec_ptr_unmk(&rootNodes);

  h->min = NULL;
  
  /* This loop reorganises the heap according to a. */
  intmax_t i;
  for (i = 0; i < maxDegree(h)+1; i++) {
    if (a[i]) {
      rootlistAdd(h, a[i]);
      if (!h->min || h->compare(&a[i]->key, &h->min->key) < 0) {
        h->min = a[i];
      }
    }
  }
  free(a);
}


static void fibheapCut(fibheap *h, fibnode *x, fibnode *y)
{
  assert(x->parent == y);
  unSpliceChild(x);
  rootlistAdd(h, x);
  x->parent = NULL;
  x->mark = false;
}


static void fibheapCascadingCut(fibheap *h, fibnode *y)
{
  if (y->parent) {
    fibnode *z = y->parent;
    if (!y->mark) {
      y->mark = true;
    } else {
      fibheapCut(h, y, z);
      fibheapCascadingCut(h, z);
    }
  }
}



/************************************************************************/
/* Low level utilities */

void addChild(fibnode *parent, fibnode *newChild)
{
  if (parent->child) {
    fhsplice(newChild, parent->child);
  } else {
    parent->child = newChild;
  }
  newChild->parent = parent;
  parent->degree++;
}

static void link(fibnode *newChild, fibnode *parent)
{
  unSplice(newChild);
  addChild(parent, newChild);
  newChild->mark = false;
}


static intmax_t maxDegree(fibheap *h)
{
  assert(h->num != 0);
  double n = h->num * 1.f;
  intmax_t res = (intmax_t) (log(n) / log((1.f + sqrt(5.f)) / 2.f));
  return res;
}


bool isAlone(fibnode *n)
{
  return n->right == n && n->left == n;
}


/**
 * Connects n and m so that n->right==m and m->left==n.
 */
void fhsplice(fibnode *n, fibnode *m)
{
  fibnode *right = n->right, *left = m->left;
  n->right = m;
  m->left = n;
  right->left = left;
  left->right = right;
}


/**
 * Removes n from the doubly-linked list of which it is a part (left/right),
 * properly linking up the nodes surrounding n, if any.  n will only point to
 * itself on returning.
 */
void unSplice(fibnode *n)
{
  fibnode *right = n->right, *left = n->left;
  left->right = right;
  right->left = left;
  /* make n point only to itself. */
  n->right = n;
  n->left = n;
}

static void unSpliceChild(fibnode *n)
{
  /* Before calling unSplice we have to make sure the parent's child list
   * doesn't start with n.  Once we select a new head for that list, we can
   * safely unSplice n.  */
  fibnode *p = n->parent;
  if (isAlone(n)) {
    p->child = NULL;
  } else {
    assert(p->child);
    if (p->child == n) {
      p->child = n->right;
      p->child->parent = p;
    }
  }
  p->degree--;
  unSplice(n);
}


void toVector(fibnode *n, struct vec_ptr *v)
{
  fibnode *start = n;
  do {
    vec_ptr_push(v, n);
    n = n->right;
  } while (n != start);
}


void fibheapFreeNodes(fibnode *n)
{
  fibnode *last = n->left;
  while (n != last) {
    fibnode *next = n->right;
    free(n);
    n = next;
  }
  free(n);
}
