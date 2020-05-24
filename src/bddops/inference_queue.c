/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2006, University of Cincinnati.  All rights reserved.
 By using this software the USER indicates that he or she has read,
 understood and will comply with the following:

 --- University of Cincinnati hereby grants USER nonexclusive permission
 to use, copy and/or modify this software for internal, noncommercial,
 research purposes only. Any distribution, including commercial sale
 or license, of this software, copies of the software, its associated
 documentation and/or modifications of either is strictly prohibited
 without the prior consent of University of Cincinnati.  Title to copyright
 to this software and its associated documentation shall at all times
 remain with University of Cincinnati.  Appropriate copyright notice shall
 be placed on all software copies, and a complete copy of this notice
 shall be included in all copies of the associated documentation.
 No right is  granted to use in advertising, publicity or otherwise
 any trademark,  service mark, or the name of University of Cincinnati.


 --- This software and any associated documentation is provided "as is"

 UNIVERSITY OF CINCINNATI MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS
 OR IMPLIED, INCLUDING THOSE OF MERCHANTABILITY OR FITNESS FOR A
 PARTICULAR PURPOSE, OR THAT  USE OF THE SOFTWARE, MODIFICATIONS, OR
 ASSOCIATED DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS,
 TRADEMARKS OR OTHER INTELLECTUAL PROPERTY RIGHTS OF A THIRD PARTY.

 University of Cincinnati shall not be liable under any circumstances for
 any direct, indirect, special, incidental, or consequential damages
 with respect to any claim by USER or any third party on account of
 or arising from the use, or inability to use, this software or its
 associated documentation, even if University of Cincinnati has been advised
 of the possibility of those damages.
*********************************************************************/

#include "sbsat.h"

void add_inference_pool(inference_pool *infer_pool) {
  infer_pool->pools = (inference_item **)realloc(infer_pool->pools, (infer_pool->max_pools+1) * sizeof(inference_item*));
  infer_pool->pools[infer_pool->max_pools] = (inference_item *)malloc(INFER_POOL_SIZE * sizeof(inference_item));
  infer_pool->max_pools++;
}

inference_pool *init_inference_pool() {
  //d4_printf1("Init inference pool\n");
  inference_pool *infer_pool = (inference_pool *)malloc(1 * sizeof(inference_pool));
  infer_pool->pools = (inference_item **)malloc(1 * sizeof(inference_item*));
  infer_pool->pools[0] = (inference_item *)malloc(INFER_POOL_SIZE * sizeof(inference_item));
  infer_pool->max_pools = 1;
  infer_pool->curr_pool = 0;
  infer_pool->next_item = 0;
  return infer_pool;
}

void free_inference_pool(inference_pool *infer_pool) {
  assert(infer_pool != NULL);
  if(infer_pool->pools != NULL) {
    for(uintmax_t i = 0; i < infer_pool->max_pools; i++)
      free(infer_pool->pools[i]);
    free(infer_pool->pools);
  }
  free(infer_pool);
}

void reset_inference_pool(inference_pool *infer_pool) {
  infer_pool->curr_pool = 0;
  infer_pool->next_item = 0;
}

inference_item *get_inference(inference_pool *infer_pool, intmax_t lft, intmax_t rgt, inference_item *next) {
  if(infer_pool->next_item >= INFER_POOL_SIZE) {
    if(infer_pool->curr_pool+1 >= infer_pool->max_pools) {
      add_inference_pool(infer_pool);
    }
    infer_pool->curr_pool++;
    infer_pool->next_item = 0;
  }
  inference_item *inference = &(infer_pool->pools[infer_pool->curr_pool][infer_pool->next_item++]);
  inference->lft = lft;
  inference->rgt = rgt;
  inference->next = next;

  return inference;
}

void print_inference(FILE *fout, inference_item *inference) {
  while(inference != NULL) {
    d2_printf3("(%jd=%jd)", inference->lft, inference->rgt);
    d4_printf3("(%s=%s)", s_name(imaxabs(inference->lft)), s_name(imaxabs(inference->rgt)));
    inference = inference->next;
  }
}
