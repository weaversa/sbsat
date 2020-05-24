/*
 *     Copyright (C) 2011 Sean Weaver
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

#ifndef SBSAT_STATS_H
#define SBSAT_STATS_H

//Floating point statistics
#define STAT_F_CNF_READ_TIME          0
#define STAT_F_CNF_PROCESS_TIME       1
#define STAT_F_CNF_COMPRESSION        2
#define STAT_F_CNF_TOTAL_TIME         3
#define STAT_F_CLUSTERING_COMPRESSION 4
#define STAT_F_TOTAL_COMPRESSION      5
#define STAT_F_SMURF_BUILDING_TIME    6
#define STAT_F_SOLVER_START_TIME      7
#define STAT_F_SOLVER_TOTAL_TIME      8
#define STAT_F_TOTAL_TIME             9

#define STAT_F_MAX                    10
extern long double sbsat_stats_f[STAT_F_MAX];

//Integer statistics
#define STAT_CNF_NUM_CLAUSES                      0
#define STAT_CNF_NUM_VARIABLES                    1
#define STAT_RAM_USAGE_BEFORE_READING_CNF         2
#define STAT_RAM_USAGE_AFTER_READING_CNF          3
#define STAT_RAM_USAGE_AFTER_PROCESSING_CNF       4
#define STAT_BDD_NODES_AFTER_PROCESSING_CNF       5
#define STAT_NUM_BDDS_AFTER_PROCESSING_CNF        6
#define STAT_ERROR_NUM                            7
#define STAT_NUM_XORS                             8
#define STAT_NUM_AND_EQUALS                       9
#define STAT_NUM_ITE_EQUALS                      10
#define STAT_NUM_MAJV_EQUALS                     11
#define STAT_NUM_CARDINALITIES                   12
#define STAT_NUM_UNKNOWN_CLUSTERS                13
#define STAT_NUM_UNITS                           14
#define STAT_NUM_CLAUSES                         15
#define STAT_NUM_VARIABLES_AFTER_CLUSTERING      16
#define STAT_NUM_BDDS_AFTER_CLUSTERING           17
#define STAT_NUM_BDD_NODES_AFTER_CLUSTERING      18
#define STAT_MAX_RAM_USAGE                       19
#define STAT_SAT_RESULT                          20

//Smurfstate counters
#define STAT_DROPPEDVAR_STATES_BUILT             21
#define STAT_OR_STATES_BUILT                     22
#define STAT_XOR_STATES_BUILT                    23
#define STAT_ANDEQUALS_STATES_BUILT              24
#define STAT_MINMAX_STATES_BUILT                 25
#define STAT_NEGMINMAX_STATES_BUILT              26
#define STAT_INFERENCE_STATES_BUILT              27
#define STAT_XORFACTOR_STATES_BUILT              28
#define STAT_SMURF_STATES_BUILT                  29
#define STAT_STATES_BUILT                        30
#define STAT_SMURF_STATES_EXAMINED               31

//Search statistics
#define STAT_NUM_INFERENCES                      32
#define STAT_NUM_PROPAGATIONS                    33
#define STAT_NUM_BACKTRACKS                      34

#define STAT_MAX                                 35
extern uintmax_t sbsat_stats[STAT_MAX];

extern char sbsat_stats_file[256];
extern char sbsat_display_stat[256];

void sbsat_dump_stats();

#endif
