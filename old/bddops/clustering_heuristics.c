/*
 *    Copyright (C) 2011 Sean Weaver
 * 
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2, or (at your option)
 *    any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software Foundation,
 *    Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * */

#include "sbsat.h"

int ve_heuristic = 5; //VarScore

Clustering_Heuristic CHeuristics[7] = {Static_heuristic,
                                       Random_heuristic,
                                       CUDD_heuristic,
                                       FORCE_heuristic,
                                       Overlap_heuristic,
                                       VarScore_heuristic,
                                       Optimal_heuristic};

uintmax_t BDD_span(BDDManager *BM, uintmax_t bdd_num, uintmax_t *order) {
  //The span of a BDD is the difference between the smallest and
  //greatest variables in the bdd.
  uintmax_t bdd_len = BM->Support[bdd_num].nLength;
  uintmax_t *bdd_support = BM->Support[bdd_num].pList;
  uintmax_t min = (~0)-1;
  uintmax_t max = 0;

  if(bdd_len == 0) return 0; //BDD is a constant

  for(uintmax_t i = 0; i < bdd_len; i++) {
    uintmax_t var = bdd_support[i];
    uintmax_t var_order = order[var];
    if(var_order < min) min = var_order;
    if(var_order > max) max = var_order;
  }

  d9_printf3("Span for BDD %ju is %ju\n", bdd_num, max-min);
  return max-min;
}

uintmax_t total_BDD_span(BDDManager *BM, uintmax_t *order) {
  uintmax_t total_span = 0;
  for(uintmax_t i = 0; i < BM->nNumBDDs; i++) {
    total_span += BDD_span(BM, i, order);
  }
  return total_span;
}

uint8_t Static_heuristic(BDDManager *BM, uintmax_t *order) {
  uint8_t ret = NO_ERROR;

  for(uintmax_t v = 0; v < BM->nNumVariables; v++)
    order[v] = v;
  
  return ret;
}

uint8_t Random_heuristic(BDDManager *BM, uintmax_t *order) {
  uint8_t ret = NO_ERROR;

  Static_heuristic(BM, order);

  //Pick a random clustering order
  if(BM->nNumVariables != 0) {
    for(uintmax_t i = 0; i < BM->nNumVariables - 1; i++) {
      int c = rand() / (RAND_MAX/(BM->nNumVariables - i) + 1);
      int t = order[i]; order[i] = order[i+c]; order[i+c] = t;/* swap */
    }
  }
  return ret;
}

uint8_t CUDD_heuristic(BDDManager *BM, uintmax_t *order) {
  uint8_t ret = NO_ERROR;

  //Force a reordering
  Cudd_ReduceHeap(BM->dd, CUDD_REORDER_SAME, 0);

  for(uintmax_t v = 0; v < BM->nNumVariables; v++)
    order[v] = BM->dd->invperm[v];

  uintmax_t total_span = total_BDD_span(BM, order);
  d9_printf2("Average span: %4.3f\n", ((double) total_span) / ((double) BM->nNumBDDs));
  
  return ret;
}

uint8_t FORCE_heuristic(BDDManager *BM, uintmax_t *order) {
  
  uint8_t ret = NO_ERROR;
  
  uintmax_t force_const = 10;
  uintmax_t force_limit = force_const * log(BM->nNumVariables);
  
  //randomly generate an initial order of vertices;
  ret = Random_heuristic(BM, order);
  if(ret != NO_ERROR) return ret;

  uintmax_t_list_resize(&(BM->uTempList), BM->nNumVariables);
  memcpy(BM->uTempList.pList, order, sizeof(uintmax_t) * BM->nNumVariables);
  
  uintmax_t nNumHyperEdges = BM->nNumBDDs;
  double *cog = (double *)sbsat_malloc(nNumHyperEdges, sizeof(double), 9, "Center of Gravity");
  
  uintmax_t nNumVertices = BM->nNumVariables;
  dbl_index *order_lv = (dbl_index *)sbsat_malloc(nNumVertices, sizeof(dbl_index), 9, "order_lv");

  uintmax_t total_span = ~0;
  uintmax_t old_total_span = total_BDD_span(BM, order);

  //repeat limit times or until total span stops decreasing
  for(uintmax_t iter = 0; (iter < force_limit) /* && (total_span < old_total_span) */ ; iter++) {
    //for each hyperedge e of E
    for(uintmax_t e = 0; e < nNumHyperEdges; e++) {
      //compute center of gravity of e (COG(e)) - traverse all hyperedges
      uintmax_t bdd_len = BM->Support[e].nLength;
      uintmax_t *bdd_support = BM->Support[e].pList;
      //for all v of e, COG(e) = (sum lv) / |e|
      //- lv and e denote the location of vertex v under the given
      //- linear ordering and the number of vertices connected to
      //- hyperedge e, respectively.
      cog[e] = 0.0;
      for(uintmax_t v = 0; v < bdd_len; v++) {
	cog[e] += (double) order[bdd_support[v]];
      }
      cog[e] /= (double) bdd_len;
    }
    
    //for each virtex v of V
    for(uintmax_t v = 0; v < nNumVertices; v++) {
      uintmax_t var_len = BM->VarBDDMap[v].nLength;
      uintmax_t *var_support = BM->VarBDDMap[v].pList;
      //compute tentative new location of v based on centers of gravity of hyperedges
      //for all e of Ev, lv' = (sum COG(e)) / |Ev|
      order_lv[v].index = v;
      order_lv[v].value = 0.0;
      for(uintmax_t e = 0; e < var_len; e++) {
	order_lv[v].value += cog[var_support[e]];
      }
      order_lv[v].value /= (double) var_len;
    }
    
    //sort tentative vertex locations;
    qsort(order_lv, nNumVertices, sizeof(dbl_index), dbl_indexcompfunc);
    
    //assign integer indices to the vertices;
    for(uintmax_t v = 0; v < nNumVertices; v++) {
      order[v] = order_lv[v].index;
    }

    //Recalculate total span
    total_span = total_BDD_span(BM, order);
    //d4_printf2("Total span: %ju\n", total_span);

    if(old_total_span > total_span) {
      memcpy(BM->uTempList.pList, order, sizeof(uintmax_t) * BM->nNumVariables);
      d9_printf1("*");
      old_total_span = total_span;
    } else {
      //For the future!
    }
    d9_printf2("Average span: %4.3f\n", ((double) total_span) / ((double) BM->nNumBDDs));
  }

  memcpy(order, BM->uTempList.pList, sizeof(uintmax_t) * BM->nNumVariables);

  sbsat_free((void**)&order_lv);
  sbsat_free((void**)&cog);

  return ret;
}

uint8_t Overlap_heuristic(BDDManager *BM, uintmax_t *order) {
  uint8_t ret = NO_ERROR;

  rand_list *rlist = (rand_list*)sbsat_malloc(BM->nNumVariables, sizeof(rand_list), 9, "rlist");

  for(uintmax_t v = 0; v < BM->nNumVariables; v++) {
    rlist[v].num = v;
    rlist[v].size = BM->VarBDDMap[v].nLength;
    rlist[v].prob = (double) random();
  }

  qsort(rlist, BM->nNumVariables, sizeof(rand_list), randlistcompfunc);
  
  for(uintmax_t v = 0; v < BM->nNumVariables; v++) {
    order[v] = rlist[v].num;
  }

  sbsat_free((void **)&rlist);

  return ret;
}

uint8_t VarScore_heuristic(BDDManager *BM, uintmax_t *order) {
  uint8_t ret = NO_ERROR;

  rand_list *rlist = (rand_list*)sbsat_calloc(BM->nNumVariables, sizeof(rand_list), 9, "rlist");
  uintmax_t *bdd_sizes = (uintmax_t *)sbsat_malloc(BM->nNumBDDs, sizeof(uintmax_t), 9, "bdd_sizes");
  uintmax_t maxint = ~(uintmax_t)0;

  for(uintmax_t bdd_loc = 0; bdd_loc < BM->nNumBDDs; bdd_loc++) {
    int bdd_size = Cudd_DagSize(BM->BDDList[bdd_loc]);
    if(bdd_size < 0) { //overflow
      bdd_sizes[bdd_loc] = maxint;
      continue;
    }
    bdd_sizes[bdd_loc] = (uintmax_t)bdd_size;
  }

  for(uintmax_t v = 0; v < BM->nNumVariables; v++) {
    rlist[v].num = v;
    if(rlist[v].size == maxint) continue; //overflow

    for(uintmax_t i = 0; i < BM->VarBDDMap[v].nLength; i++) {
      uintmax_t bdd_loc = BM->VarBDDMap[v].pList[i];
      if(bdd_sizes[bdd_loc] == maxint) {
	if(rlist[v].size == maxint)
	break; //overflow
      }
      
      if((maxint - (uintmax_t)bdd_sizes[bdd_loc]) < rlist[v].size) { //overflow
	rlist[v].size = maxint;
	break; //overflow
      }
      rlist[v].size += bdd_sizes[bdd_loc];
    }

    rlist[v].prob = (double)random();
  }
  
  qsort(rlist, BM->nNumVariables, sizeof(rand_list), randlistcompfunc);
  
  for(uintmax_t v = 0; v < BM->nNumVariables; v++) {
    order[v] = rlist[v].num;
  }

  sbsat_free((void **)&rlist);
  sbsat_free((void **)&bdd_sizes);

  return ret;
}

uint8_t Optimal_heuristic(BDDManager *BM, uintmax_t *order) {
  uint8_t ret = NO_ERROR;

  uintmax_t opt_span = ~0, tmp_span;
  uintmax_t *opt_order = (uintmax_t *)sbsat_malloc(BM->nNumVariables, sizeof(uintmax_t), 9, "opt_order");
  double avg_span = 0;

  ret = Static_heuristic(BM, order);
  if(ret != NO_ERROR) return ret;
  tmp_span = total_BDD_span(BM, order);
  if(tmp_span < opt_span) {
    memcpy(opt_order, order, sizeof(uintmax_t) * BM->nNumVariables);
    opt_span = tmp_span;
    avg_span = BM->nNumBDDs==0?0.0:((double) opt_span) / ((double) BM->nNumBDDs);
    d4_printf2("Choosing Static_heuristic because of average span: %4.3f\n", avg_span);
  }

  ret = Overlap_heuristic(BM, order);
  if(ret != NO_ERROR) return ret;
  tmp_span = total_BDD_span(BM, order);
  if(tmp_span < opt_span) {
    memcpy(opt_order, order, sizeof(uintmax_t) * BM->nNumVariables);
    opt_span = tmp_span;
    avg_span = BM->nNumBDDs==0?0.0:((double) opt_span) / ((double) BM->nNumBDDs);
    d4_printf2("Choosing Overlap_heuristic because of average span: %4.3f\n", avg_span);
  }

  ret = VarScore_heuristic(BM, order);
  if(ret != NO_ERROR) return ret;
  tmp_span = total_BDD_span(BM, order);
  if(tmp_span < opt_span) {
    memcpy(opt_order, order, sizeof(uintmax_t) * BM->nNumVariables);
    opt_span = tmp_span;
    avg_span = BM->nNumBDDs==0?0.0:((double) opt_span) / ((double) BM->nNumBDDs);
    d4_printf2("Choosing VarScore_heuristic because of average span: %4.3f\n", avg_span);
  }

  ret = Random_heuristic(BM, order);
  if(ret != NO_ERROR) return ret;
  tmp_span = total_BDD_span(BM, order);
  if(tmp_span < opt_span) {
    memcpy(opt_order, order, sizeof(uintmax_t) * BM->nNumVariables);
    opt_span = tmp_span;
    avg_span = BM->nNumBDDs==0?0.0:((double) opt_span) / ((double) BM->nNumBDDs);
    d4_printf2("Choosing Random_heuristic because of average span %4.3f\n", avg_span);
  }

  ret = CUDD_heuristic(BM, order);
  if(ret != NO_ERROR) return ret;
  tmp_span = total_BDD_span(BM, order);
  if(tmp_span < opt_span) {
    memcpy(opt_order, order, sizeof(uintmax_t) * BM->nNumVariables);
    opt_span = tmp_span;
    avg_span = BM->nNumBDDs==0?0.0:((double) opt_span) / ((double) BM->nNumBDDs);
    d4_printf2("Choosing CUDD_heuristic (same) because of average span %4.3f\n", avg_span);
  }

  ret = FORCE_heuristic(BM, order);
  if(ret != NO_ERROR) return ret;
  tmp_span = total_BDD_span(BM, order);
  if(tmp_span < opt_span) {
    memcpy(opt_order, order, sizeof(uintmax_t) * BM->nNumVariables);
    opt_span = tmp_span;
    avg_span = BM->nNumBDDs==0?0.0:((double) opt_span) / ((double) BM->nNumBDDs);
    d4_printf2("Choosing FORCE_heuristic because of average span %4.3f\n", avg_span);
  }

  memcpy(order, opt_order, sizeof(uintmax_t) * BM->nNumVariables);

  sbsat_free((void**)&opt_order);
  
  return ret;
}
