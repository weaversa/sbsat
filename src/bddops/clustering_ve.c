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

//Ideas taken from "Toward Good Elimination Orders for Symbolic SAT Solving" by Jinbo Huang and Adnan Darwiche, 2004.

#include "sbsat.h"

int ve_iterations = 1;
int ve_restarts = 0;

uint8_t VE_auto(BDDManager *BM) {
  uint8_t ret = NO_ERROR;

  if(ve_heuristic < 0) ve_heuristic = 5;
  if(ve_heuristic >= NUM_CLUSTERING_HEURISTICS) ve_heuristic = 5;

  ret = VE(BM, CHeuristics[ve_heuristic], ve_iterations);

  sbsat_stats[STAT_NUM_VARIABLES_AFTER_CLUSTERING] = count_num_BDD_variables(BM);
  sbsat_stats[STAT_NUM_BDDS_AFTER_CLUSTERING] = BM->nNumBDDs;
  sbsat_stats[STAT_NUM_BDD_NODES_AFTER_CLUSTERING] = BM->dd->keys;
  sbsat_stats_f[STAT_F_CLUSTERING_COMPRESSION] = (double)sbsat_stats[STAT_NUM_BDDS_AFTER_PROCESSING_CNF] / (double)(BM->nNumBDDs==0?1:BM->nNumBDDs);
  sbsat_stats_f[STAT_F_TOTAL_COMPRESSION] = (double)sbsat_stats[STAT_CNF_NUM_CLAUSES] / (double)(BM->nNumBDDs==0?1:BM->nNumBDDs);
  
  return ret;
}

uint8_t VE(BDDManager *BM, Clustering_Heuristic CHeuristic, int num_iterations) {
  uint8_t ret = NO_ERROR;
  
  int iterations = 0;
  unsigned int limit = 0;

  if(num_iterations == 0) {
    return ret;
  }

  if(num_iterations == -1) num_iterations = INT_MAX; //A sufficiently large number

  uintmax_t *order = (uintmax_t *)sbsat_calloc(BM->nNumVariables, sizeof(uintmax_t), 9, "order");

  uintmax_t BDDs_clustered = 0;

  while(BM->nNumBDDs > 1) {

    ret = CHeuristic(BM, order); //get an ordering
    if(ret != NO_ERROR) {
      sbsat_free((void **)&order);
      return ret;
    }

    uint8_t limit_hit = 0;
    for(uintmax_t i = 0; i < BM->nNumVariables; i++) {
      uintmax_t v = order[i];
      uintmax_t loc = 0;
      while(BM->VarBDDMap[v].nLength > 1) {
	if(loc+1 >= BM->VarBDDMap[v].nLength) loc = 0;
	uintmax_t bdd0 = BM->VarBDDMap[v].pList[0+loc];
	uintmax_t bdd1 = BM->VarBDDMap[v].pList[1+loc];
	assert(bdd0 != bdd1);
	loc++;

        ret = cluster_BDDs(BM, bdd0, bdd1, limit);

	if(ret == CLUSTER_LIMIT) {
	  if(limit_hit == 0) {
	    d6_printf3("breaking %jd, limit %d\n", i, limit);
	    limit_hit = 1;
	  } else {
	    d9_printf3("breaking %jd, limit %d\n", i, limit);
	  }
	  ret = NO_ERROR;
	  break;
	} else if(ret != NO_ERROR) {
	  sbsat_free((void **)&order);
	  return ret;
	} else {
	  BDDs_clustered++;
	}
      }
    }

    //Don't increase the limit if a _significant_ number of BDDs were
    //clustered. Is is better to try again at the current limit.
    if(BDDs_clustered <= 10) {
      if(limit != INT_MAX) {
	limit=1+(limit*2);
      } else {
	d6_printf1("Max limit hit...returning\n");
	break;
      }
    }
    BDDs_clustered = 0;
    if(++iterations >= num_iterations)
      break;
  }
  
  d2_printf1("Done Clustering\n");
  
  sbsat_free((void **)&order);

  if(BM->nNumBDDs == 0) return SAT;
  
  /*
  uint8_t changed = 1;
  while(changed) {
    changed = 0;
    for(uintmax_t i = 0; i < BM->nNumBDDs; i++) {
      for(uintmax_t j = i+1; j < BM->nNumBDDs; j++) {
	ret = cluster_BDDs(BM, i, j, 99999999);
	if(ret != CLUSTER_LIMIT) changed = 1;
      }
    }
  }
  ret = NO_ERROR;
  */

  return ret;
}
