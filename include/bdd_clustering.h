/*
  *     sbsat - SBSAT is a state-based Satisfiability solver.
  *  
  *     Copyright (C) 2010 Sean Weaver
  *  
  *     This program is free software; you can redistribute it and/or modify
  *     it under the terms of the GNU General Public License as published by
  *     the Free Software Foundation; either version 2, or (at your option)
  *     any later version.
  *  
  *     This program is distributed in the hope that it will be useful,
  *     but WITHOUT ANY WARRANTY; without even the implied warranty of
  *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *     GNU General Public License for more details.
  *  
  *     You should have received a copy of the GNU General Public License
  *     along with this program; if not, write to the Free Software Foundation,
  *     Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
  * */

#ifndef BDD_CLUSTERING_H
#define BDD_CLUSTERING_H

#include "sbsat.h"

#define PAIRWISE_BDD_REDUCTION_NONE         0
#define PAIRWISE_BDD_REDUCTION_RESTRICT     1
#define PAIRWISE_BDD_REDUCTION_LICOMPACTION 2
#define PAIRWISE_BDD_REDUCTION_NPAND        3

extern int pairwise_bdd_reduction;

typedef uint8_t (*Clustering_Heuristic)(BDDManager *BM, uintmax_t *order);

extern int ve_iterations;
extern int ve_restarts;
extern int ve_heuristic;

extern int bdd_cluster_limit;

uint8_t Static_heuristic(BDDManager *BM, uintmax_t *order);   //0
uint8_t Random_heuristic(BDDManager *BM, uintmax_t *order);   //1
uint8_t CUDD_heuristic(BDDManager *BM, uintmax_t *order);     //2
uint8_t FORCE_heuristic(BDDManager *BM, uintmax_t *order);    //3
uint8_t Overlap_heuristic(BDDManager *BM, uintmax_t *order);  //4
uint8_t VarScore_heuristic(BDDManager *BM, uintmax_t *order); //5
uint8_t Optimal_heuristic(BDDManager *BM, uintmax_t *order);  //6

#define NUM_CLUSTERING_HEURISTICS 7
extern Clustering_Heuristic CHeuristics[NUM_CLUSTERING_HEURISTICS];

uint8_t VE_auto(BDDManager *BM);
uint8_t VE(BDDManager *BM, Clustering_Heuristic CH, int num_iterations);

#endif
