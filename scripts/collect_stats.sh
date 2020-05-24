#!/bin/sh

#BENCHES="sat-ex sat-2002-beta sat-2003 sat-2004 sat-2005 sat-2006-race sat-2007 sat-2008 sat-2009 WildBenches2009 sat-2010-race"
#BENCHES="cnf_clustering_benches ebddres-examples"
BENCHES="cnf_clustering_benches"
#BENCHES="ebddres-examples"


SBSAT_DESCS=(
               "Find gates, Cluster CNF with overlap 2, Limit Clustering to 12 variables, Inferences, Equivalences, VE(ALL), VE Heuristic VarScore, BDD Ordering 0, Compress Smurfs, Ram limit: 2000M, Time limit: 600s"
               "Find gates, Cluster CNF with overlap 2, Limit Clustering to 12 variables, Inferences, Equivalences, VE(ALL), VE Heuristic VarScore, BDD Ordering 11, Compress Smurfs, Ram limit: 2000M, Time limit: 600s"
               "Find gates, Cluster CNF with overlap 2, Limit Clustering to 12 variables, Inferences, Equivalences, VE(ALL), VE Heuristic VarScore, BDD Ordering 0, Ram limit: 2000M, Time limit: 600s"
               "Cluster CNF with overlap 2, Limit Clustering to 12 variables, Inferences, Equivalences, Early Quantification, VE(ALL), VE Heuristic VarScore, BDD Ordering 0, Compress Smurfs, No Spec. Smurfs, Ram limit: 2000M, Time limit: 600s"
               "Cluster CNF with overlap 2, Limit Clustering to 12 variables, Inferences, Equivalences, Early Quantification, VE(ALL), VE Heuristic VarScore, BDD Ordering 0, No Spec. Smurfs, Ram limit: 2000M, Time limit: 600s"
               "Find gates, Cluster CNF with overlap 2, Limit Clustering to 12 variables, Inferences, Equivalences, Early Quantification, VE(0), BDD Ordering 0, Compress Smurfs, Ram limit: 2000M, Time limit: 600s"

#               "Find gates, Cluster CNF with overlap 2, Limit Clustering to 12 variables, Inferences, Equivalences, Early Quantification, VE(ALL), VE Heuristic VarScore, BDD Ordering 0, Compress Smurfs, Dropped Var States, Ram limit: 2000M, Time limit: 600s"
#               "Find gates, Cluster CNF with overlap 0, Inferences, Equivalences, Early Quantification, VE(1), VE Heuristic VarScore, BDD Ordering 0, Ram limit: 2000M, Time limit: 120s"

#                "Find gates, Cluster CNF with overlap 0, Early Quantification, VE(1), VE Heuristic FORCE, BDD Ordering 0, Ram limit: 2000M, Time limit: 120s"
#                "Find gates, Cluster CNF with overlap 0, Early Quantification, VE(2), VE Heuristic FORCE, BDD Ordering 0, Ram limit: 2000M, Time limit: 120s"
#                "Find gates, Cluster CNF with overlap 0, Early Quantification, VE(3), VE Heuristic FORCE, BDD Ordering 0, Ram limit: 2000M, Time limit: 120s"
#                "Find gates, Cluster CNF with overlap 0, Early Quantification, VE(1), VE Heuristic FORCE, BDD Ordering 11, Ram limit: 2000M, Time limit: 120s"
#                "Find gates, Cluster CNF with overlap 0, Early Quantification, VE(2), VE Heuristic FORCE, BDD Ordering 11, Ram limit: 2000M, Time limit: 120s"
#                "Find gates, Cluster CNF with overlap 0, Early Quantification, VE(3), VE Heuristic FORCE, BDD Ordering 11, Ram limit: 2000M, Time limit: 120s"
#                "Find gates, Cluster CNF with overlap 0, Inferences, Equivalences, Early Quantification, VE(1), VE Heuristic FORCE, BDD Ordering 11, Ram limit: 2000M, Time limit: 120s"
#                "Find gates, Cluster CNF with overlap 0, Inferences, Equivalences, Early Quantification, VE(2), VE Heuristic FORCE, BDD Ordering 11, Ram limit: 2000M, Time limit: 120s"
#                "Find gates, Cluster CNF with overlap 0, Inferences, Equivalences, Early Quantification, VE(3), VE Heuristic FORCE, BDD Ordering 11, Ram limit: 2000M, Time limit: 120s"

#                "Find gates, Cluster CNF with overlap 0, Early Quantification, VE(1), VE Heuristic VarScore, BDD Ordering 0, Ram limit: 2000M, Time limit: 120s"
#                "Find gates, Cluster CNF with overlap 0, Early Quantification, VE(2), VE Heuristic VarScore, BDD Ordering 0, Ram limit: 2000M, Time limit: 120s"
#                "Find gates, Cluster CNF with overlap 0, Early Quantification, VE(3), VE Heuristic VarScore, BDD Ordering 0, Ram limit: 2000M, Time limit: 120s"
#                "Find gates, Cluster CNF with overlap 0, Early Quantification, VE(1), VE Heuristic VarScore, BDD Ordering 11, Ram limit: 2000M, Time limit: 120s"
#                "Find gates, Cluster CNF with overlap 0, Early Quantification, VE(2), VE Heuristic VarScore, BDD Ordering 11, Ram limit: 2000M, Time limit: 120s"
#                "Find gates, Cluster CNF with overlap 0, Early Quantification, VE(3), VE Heuristic VarScore, BDD Ordering 11, Ram limit: 2000M, Time limit: 120s"
#                "Find gates, Cluster CNF with overlap 0, Inferences, Equivalences, Early Quantification, VE(1), VE Heuristic VarScore, BDD Ordering 11, Ram limit: 2000M, Time limit: 120s"
#                "Find gates, Cluster CNF with overlap 0, Inferences, Equivalences, Early Quantification, VE(2), VE Heuristic VarScore, BDD Ordering 11, Ram limit: 2000M, Time limit: 120s"
#                "Find gates, Cluster CNF with overlap 0, Inferences, Equivalences, Early Quantification, VE(3), VE Heuristic VarScore, BDD Ordering 11, Ram limit: 2000M, Time limit: 120s"

#                "Find gates, Cluster CNF with overlap 0, Early Quantification, VE(1), VE Heuristic Optimal, BDD Ordering 0, Ram limit: 2000M, Time limit: 120s"
#                "Find gates, Cluster CNF with overlap 0, Early Quantification, VE(2), VE Heuristic Optimal, BDD Ordering 0, Ram limit: 2000M, Time limit: 120s"
#                "Find gates, Cluster CNF with overlap 0, Early Quantification, VE(3), VE Heuristic Optimal, BDD Ordering 0, Ram limit: 2000M, Time limit: 120s"
#                "Find gates, Cluster CNF with overlap 0, Early Quantification, VE(1), VE Heuristic Optimal, BDD Ordering 11, Ram limit: 2000M, Time limit: 120s"
#                "Find gates, Cluster CNF with overlap 0, Early Quantification, VE(2), VE Heuristic Optimal, BDD Ordering 11, Ram limit: 2000M, Time limit: 120s"
#                "Find gates, Cluster CNF with overlap 0, Early Quantification, VE(3), VE Heuristic Optimal, BDD Ordering 11, Ram limit: 2000M, Time limit: 120s"
#                "Find gates, Cluster CNF with overlap 0, Inferences, Equivalences, Early Quantification, VE(1), VE Heuristic Optimal, BDD Ordering 11, Ram limit: 2000M, Time limit: 120s"
#                "Find gates, Cluster CNF with overlap 0, Inferences, Equivalences, Early Quantification, VE(2), VE Heuristic Optimal, BDD Ordering 11, Ram limit: 2000M, Time limit: 120s"
#                "Find gates, Cluster CNF with overlap 0, Inferences, Equivalences, Early Quantification, VE(3), VE Heuristic Optimal, BDD Ordering 11, Ram limit: 2000M, Time limit: 120s"

#              "Find gates, Cluster CNF with overlap 1, Inferences, Equivalences, Early Quantification, VE(ALL), VE Heuristic Static, BDD Ordering 5, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Inferences, Equivalences, Early Quantification, VE(ALL), VE Heuristic Random, BDD Ordering 5, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Inferences, Equivalences, Early Quantification, VE(ALL), VE Heuristic CUDD, BDD Ordering 5, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Inferences, Equivalences, Early Quantification, VE(ALL), VE Heuristic FORCE, BDD Ordering 5, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Inferences, Equivalences, Early Quantification, VE(ALL), VE Heuristic Overlap, BDD Ordering 5, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Inferences, Equivalences, Early Quantification, VE(ALL), VE Heuristic VarScore, BDD Ordering 5, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Inferences, Equivalences, Early Quantification, VE(ALL), VE Heuristic Optimal, BDD Ordering 5, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Early Quantification, VE(ALL), VE Heuristic FORCE, BDD Ordering 5, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Early Quantification, VE(ALL), VE Heuristic VarScore, BDD Ordering 5, Ram limit: 2000M, Time limit: 600s"
#              "Early Quantification, VE(ALL), VE Heuristic FORCE, BDD Ordering 5, Ram limit: 2000M, Time limit: 600s"
#              "Early Quantification, VE(ALL), VE Heuristic VarScore, BDD Ordering 5, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Inferences, Equivalences, Early Quantification, VE(ALL), VE Heuristic Static, BDD Ordering 11, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Inferences, Equivalences, Early Quantification, VE(ALL), VE Heuristic Random, BDD Ordering 11, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Inferences, Equivalences, Early Quantification, VE(ALL), VE Heuristic CUDD, BDD Ordering 11, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Inferences, Equivalences, Early Quantification, VE(ALL), VE Heuristic FORCE, BDD Ordering 11, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Inferences, Equivalences, Early Quantification, VE(ALL), VE Heuristic Overlap, BDD Ordering 11, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Inferences, Equivalences, Early Quantification, VE(ALL), VE Heuristic VarScore, BDD Ordering 11, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Inferences, Equivalences, Early Quantification, VE(ALL), VE Heuristic Optimal, BDD Ordering 11, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Early Quantification, VE(ALL), VE Heuristic FORCE, BDD Ordering 11, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Early Quantification, VE(ALL), VE Heuristic VarScore, BDD Ordering 11, Ram limit: 2000M, Time limit: 600s"
#              "Early Quantification, VE(ALL), VE Heuristic FORCE, BDD Ordering 11, Ram limit: 2000M, Time limit: 600s"
#              "Early Quantification, VE(ALL), VE Heuristic VarScore, BDD Ordering 11, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Inferences, Equivalences, Early Quantification, VE(ALL), VE Heuristic Static, BDD Ordering 17, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Inferences, Equivalences, Early Quantification, VE(ALL), VE Heuristic Random, BDD Ordering 17, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Inferences, Equivalences, Early Quantification, VE(ALL), VE Heuristic CUDD, BDD Ordering 17, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Inferences, Equivalences, Early Quantification, VE(ALL), VE Heuristic FORCE, BDD Ordering 17, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Inferences, Equivalences, Early Quantification, VE(ALL), VE Heuristic Overlap, BDD Ordering 17, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Inferences, Equivalences, Early Quantification, VE(ALL), VE Heuristic VarScore, BDD Ordering 17, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Inferences, Equivalences, Early Quantification, VE(ALL), VE Heuristic Optimal, BDD Ordering 17, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Early Quantification, VE(ALL), VE Heuristic FORCE, BDD Ordering 17, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Early Quantification, VE(ALL), VE Heuristic VarScore, BDD Ordering 17, Ram limit: 2000M, Time limit: 600s"
#              "Early Quantification, VE(ALL), VE Heuristic FORCE, BDD Ordering 17, Ram limit: 2000M, Time limit: 600s"
#              "Early Quantification, VE(ALL), VE Heuristic VarScore, BDD Ordering 17, Ram limit: 2000M, Time limit: 600s"
#              "Early Quantification, VE(ALL), VE Heuristic Static, BDD Ordering 0, Ram limit: 2000M, Time limit: 600s"
#              "Find gates, Cluster CNF with overlap 1, Early Quantification, VE(ALL), VE Heuristic Static, BDD Ordering 0, Ram limit: 2000M, Time limit: 600s"

            )

SBSAT_ARGS=(
               "-seed 97967 --debug 1 -f -cco 2 -ccl 10 -cvl 12 -inf -equ -ve -1 --ve-heuristic 5 --bdd-ordering 0 -cs 1 -dvs 0 -ram 2000 -time 600"
               "-seed 97967 --debug 1 -f -cco 2 -ccl 10 -cvl 12 -inf -equ -ve -1 --ve-heuristic 5 --bdd-ordering 11 -cs 1 -dvs 0 -ram 2000 -time 600"
               "-seed 97967 --debug 1 -f -cco 2 -ccl 10 -cvl 12 -inf -equ -ve -1 --ve-heuristic 5 --bdd-ordering 0 -cs 0 -dvs 0 -ram 2000 -time 600"
               "-seed 97967 --debug 1 -cco 2 -ccl 10 -cvl 12 -inf -equ -exq -ve -1 --ve-heuristic 5 --bdd-ordering 0 -cs 1 -dvs 0 --limit-all 1000 -ram 2000 -time 600"
               "-seed 97967 --debug 1 -cco 2 -ccl 10 -cvl 12 -inf -equ -exq -ve -1 --ve-heuristic 5 --bdd-ordering 0 -cs 0 -dvs 0 --limit-all 1000 -ram 2000 -time 600"
               "-seed 97967 --debug 1 -f -cco 2 -ccl 10 -cvl 12 -inf -equ -exq -ve 0 --bdd-ordering 0 -cs 1 -dvs 0 -ram 2000 -time 600"

#               "--debug 1 -f -cco 2 -ccl 10 -cvl 12 -inf -equ -exq -ve -1 --ve-heuristic 5 --bdd-ordering 0 -cs 1 -dvs 1 -ram 2000 -time 600"
#               "--debug 1 -f -cco 0 -inf -equ -exq -ve 1 --ve-heuristic 5 --bdd-ordering 0 -ram 2000 -time 120"

#                "--debug 1 -f -cco 0 -exq -ve 1 --ve-heuristic 3 --bdd-ordering 0 -ram 2000 -time 120"
#                "--debug 1 -f -cco 0 -exq -ve 2 --ve-heuristic 3 --bdd-ordering 0 -ram 2000 -time 120"
#                "--debug 1 -f -cco 0 -exq -ve 3 --ve-heuristic 3 --bdd-ordering 0 -ram 2000 -time 120"
#                "--debug 1 -f -cco 0 -exq -ve 1 --ve-heuristic 3 --bdd-ordering 11 -ram 2000 -time 120"
#                "--debug 1 -f -cco 0 -exq -ve 2 --ve-heuristic 3 --bdd-ordering 11 -ram 2000 -time 120"
#                "--debug 1 -f -cco 0 -exq -ve 3 --ve-heuristic 3 --bdd-ordering 11 -ram 2000 -time 120"
#                "--debug 1 -f -cco 0 -inf -equ -exq -ve 1 --ve-heuristic 3 --bdd-ordering 11 -ram 2000 -time 120"
#                "--debug 1 -f -cco 0 -inf -equ -exq -ve 2 --ve-heuristic 3 --bdd-ordering 11 -ram 2000 -time 120"
#                "--debug 1 -f -cco 0 -inf -equ -exq -ve 3 --ve-heuristic 3 --bdd-ordering 11 -ram 2000 -time 120"

#                "--debug 1 -f -cco 0 -exq -ve 1 --ve-heuristic 5 --bdd-ordering 0 -ram 2000 -time 120"
#                "--debug 1 -f -cco 0 -exq -ve 2 --ve-heuristic 5 --bdd-ordering 0 -ram 2000 -time 120"
#                "--debug 1 -f -cco 0 -exq -ve 3 --ve-heuristic 5 --bdd-ordering 0 -ram 2000 -time 120"
#                "--debug 1 -f -cco 0 -exq -ve 1 --ve-heuristic 5 --bdd-ordering 11 -ram 2000 -time 120"
#                "--debug 1 -f -cco 0 -exq -ve 2 --ve-heuristic 5 --bdd-ordering 11 -ram 2000 -time 120"
#                "--debug 1 -f -cco 0 -exq -ve 3 --ve-heuristic 5 --bdd-ordering 11 -ram 2000 -time 120"
#                "--debug 1 -f -cco 0 -inf -equ -exq -ve 1 --ve-heuristic 5 --bdd-ordering 11 -ram 2000 -time 120"
#                "--debug 1 -f -cco 0 -inf -equ -exq -ve 2 --ve-heuristic 5 --bdd-ordering 11 -ram 2000 -time 120"
#                "--debug 1 -f -cco 0 -inf -equ -exq -ve 3 --ve-heuristic 5 --bdd-ordering 11 -ram 2000 -time 120"

#                "--debug 1 -f -cco 0 -exq -ve 1 --ve-heuristic 6 --bdd-ordering 0 -ram 2000 -time 120"
#                "--debug 1 -f -cco 0 -exq -ve 2 --ve-heuristic 6 --bdd-ordering 0 -ram 2000 -time 120"
#                "--debug 1 -f -cco 0 -exq -ve 3 --ve-heuristic 6 --bdd-ordering 0 -ram 2000 -time 120"
#                "--debug 1 -f -cco 0 -exq -ve 1 --ve-heuristic 6 --bdd-ordering 11 -ram 2000 -time 120"
#                "--debug 1 -f -cco 0 -exq -ve 2 --ve-heuristic 6 --bdd-ordering 11 -ram 2000 -time 120"
#                "--debug 1 -f -cco 0 -exq -ve 3 --ve-heuristic 6 --bdd-ordering 11 -ram 2000 -time 120"
#                "--debug 1 -f -cco 0 -inf -equ -exq -ve 1 --ve-heuristic 6 --bdd-ordering 11 -ram 2000 -time 120"
#                "--debug 1 -f -cco 0 -inf -equ -exq -ve 2 --ve-heuristic 6 --bdd-ordering 11 -ram 2000 -time 120"
#                "--debug 1 -f -cco 0 -inf -equ -exq -ve 3 --ve-heuristic 6 --bdd-ordering 11 -ram 2000 -time 120"

#             "--debug 1 -f -cco 1 -inf -equ -exq -ve -1 --bdd-ordering 5 -ram 2000 -time 600 --ve-heuristic 0"
#             "--debug 1 -f -cco 1 -inf -equ -exq -ve -1 --bdd-ordering 5 -ram 2000 -time 600 --ve-heuristic 1"
#             "--debug 1 -f -cco 1 -inf -equ -exq -ve -1 --bdd-ordering 5 -ram 2000 -time 600 --ve-heuristic 2"
#             "--debug 1 -f -cco 1 -inf -equ -exq -ve -1 --bdd-ordering 5 -ram 2000 -time 600 --ve-heuristic 3"
#             "--debug 1 -f -cco 1 -inf -equ -exq -ve -1 --bdd-ordering 5 -ram 2000 -time 600 --ve-heuristic 4"
#             "--debug 1 -f -cco 1 -inf -equ -exq -ve -1 --bdd-ordering 5 -ram 2000 -time 600 --ve-heuristic 5"
#             "--debug 1 -f -cco 1 -inf -equ -exq -ve -1 --bdd-ordering 5 -ram 2000 -time 600 --ve-heuristic 6"
#             "--debug 1 -f -cco 1 -exq -ve -1 --bdd-ordering 5 -ram 2000 -time 600 --ve-heuristic 3"
#             "--debug 1 -f -cco 1 -exq -ve -1 --bdd-ordering 5 -ram 2000 -time 600 --ve-heuristic 5"
#             "--debug 1 -exq -ve -1 --bdd-ordering 5 -ram 2000 -time 600 --ve-heuristic 3"
#             "--debug 1 -exq -ve -1 --bdd-ordering 5 -ram 2000 -time 600 --ve-heuristic 5"
#             "--debug 1 -f -cco 1 -inf -equ -exq -ve -1 --bdd-ordering 11 -ram 2000 -time 600 --ve-heuristic 0"
#             "--debug 1 -f -cco 1 -inf -equ -exq -ve -1 --bdd-ordering 11 -ram 2000 -time 600 --ve-heuristic 1"
#             "--debug 1 -f -cco 1 -inf -equ -exq -ve -1 --bdd-ordering 11 -ram 2000 -time 600 --ve-heuristic 2"
#             "--debug 1 -f -cco 1 -inf -equ -exq -ve -1 --bdd-ordering 11 -ram 2000 -time 600 --ve-heuristic 3"
#             "--debug 1 -f -cco 1 -inf -equ -exq -ve -1 --bdd-ordering 11 -ram 2000 -time 600 --ve-heuristic 4"
#             "--debug 1 -f -cco 1 -inf -equ -exq -ve -1 --bdd-ordering 11 -ram 2000 -time 600 --ve-heuristic 5"
#             "--debug 1 -f -cco 1 -inf -equ -exq -ve -1 --bdd-ordering 11 -ram 2000 -time 600 --ve-heuristic 6"
#             "--debug 1 -f -cco 1 -exq -ve -1 --bdd-ordering 11 -ram 2000 -time 600 --ve-heuristic 3"
#             "--debug 1 -f -cco 1 -exq -ve -1 --bdd-ordering 11 -ram 2000 -time 600 --ve-heuristic 5"
#             "--debug 1 -exq -ve -1 --bdd-ordering 11 -ram 2000 -time 600 --ve-heuristic 3"
#             "--debug 1 -exq -ve -1 --bdd-ordering 11 -ram 2000 -time 600 --ve-heuristic 5"
#             "--debug 1 -f -cco 1 -inf -equ -exq -ve -1 --bdd-ordering 17 -ram 2000 -time 600 --ve-heuristic 0"
#             "--debug 1 -f -cco 1 -inf -equ -exq -ve -1 --bdd-ordering 17 -ram 2000 -time 600 --ve-heuristic 1"
#             "--debug 1 -f -cco 1 -inf -equ -exq -ve -1 --bdd-ordering 17 -ram 2000 -time 600 --ve-heuristic 2"
#             "--debug 1 -f -cco 1 -inf -equ -exq -ve -1 --bdd-ordering 17 -ram 2000 -time 600 --ve-heuristic 3"
#             "--debug 1 -f -cco 1 -inf -equ -exq -ve -1 --bdd-ordering 17 -ram 2000 -time 600 --ve-heuristic 4"
#             "--debug 1 -f -cco 1 -inf -equ -exq -ve -1 --bdd-ordering 17 -ram 2000 -time 600 --ve-heuristic 5"
#             "--debug 1 -f -cco 1 -inf -equ -exq -ve -1 --bdd-ordering 17 -ram 2000 -time 600 --ve-heuristic 6"
#             "--debug 1 -f -cco 1 -exq -ve -1 --bdd-ordering 17 -ram 2000 -time 600 --ve-heuristic 3"
#             "--debug 1 -f -cco 1 -exq -ve -1 --bdd-ordering 17 -ram 2000 -time 600 --ve-heuristic 5"
#             "--debug 1 -exq -ve -1 --bdd-ordering 17 -ram 2000 -time 600 --ve-heuristic 3"
#             "--debug 1 -exq -ve -1 --bdd-ordering 17 -ram 2000 -time 600 --ve-heuristic 5"
#             "--debug 1 -exq -ve -1 --bdd-ordering 0 -ram 2000 -time 600 --ve-heuristic 0"
#             "--debug 1 -f -cco 1 -exq -ve -1 --bdd-ordering 0 -ram 2000 -time 600 --ve-heuristic 0"

           )

SBSAT_SHORT_DESCS=(
                   "f_cco2_ccl10_cvl12_In_Eq_VEAll_VHV_o0_CS_2000M_600s"
                   "f_cco2_ccl10_cvl12_In_Eq_VEAll_VHV_o11_CS_2000M_600s"
                   "f_cco2_ccl10_cvl12_In_Eq_VEAll_VHV_o0_2000M_600s"
                   "cco2_ccl10_cvl12_In_Eq_Ex_VEAll_VHV_o0_CS_L0_2000M_600s"
                   "cco2_ccl10_cvl12_In_Eq_Ex_VEAll_VHV_o0_L0_2000M_600s"
                   "f_cco2_ccl10_In_Eq_Ex_VE0_o0_CS_2000M_600s"

#                   "f_cco2_ccl10_cvl12_In_Eq_Ex_VEAll_VHV_o0_CS_DVS_2000M_600s"
#                   "f_cco0_In_Eq_Ex_VE1_VHV_o0_2000M_120s"

#                    "f_cco0_Ex_VE1_VHF_o0_2000M_120s"
#                    "f_cco0_Ex_VE2_VHF_o0_2000M_120s"
#                    "f_cco0_Ex_VE3_VHF_o0_2000M_120s"
#                    "f_cco0_Ex_VE1_VHF_o11_2000M_120s"
#                    "f_cco0_Ex_VE2_VHF_o11_2000M_120s"
#                    "f_cco0_Ex_VE3_VHF_o11_2000M_120s"
#                    "f_cco0_In_Eq_Ex_VE1_VHF_o11_2000M_120s"
#                    "f_cco0_In_Eq_Ex_VE2_VHF_o11_2000M_120s"
#                    "f_cco0_In_Eq_Ex_VE3_VHF_o11_2000M_120s"

#                    "f_cco0_Ex_VE1_VHV_o0_2000M_120s"
#                    "f_cco0_Ex_VE2_VHV_o0_2000M_120s"
#                    "f_cco0_Ex_VE3_VHV_o0_2000M_120s"
#                    "f_cco0_Ex_VE1_VHV_o11_2000M_120s"
#                    "f_cco0_Ex_VE2_VHV_o11_2000M_120s"
#                    "f_cco0_Ex_VE3_VHV_o11_2000M_120s"
#                    "f_cco0_In_Eq_Ex_VE1_VHV_o11_2000M_120s"
#                    "f_cco0_In_Eq_Ex_VE2_VHV_o11_2000M_120s"
#                    "f_cco0_In_Eq_Ex_VE3_VHV_o11_2000M_120s"

#                    "f_cco0_Ex_VE1_VHO_o0_2000M_120s"
#                    "f_cco0_Ex_VE2_VHO_o0_2000M_120s"
#                    "f_cco0_Ex_VE3_VHO_o0_2000M_120s"
#                    "f_cco0_Ex_VE1_VHO_o11_2000M_120s"
#                    "f_cco0_Ex_VE2_VHO_o11_2000M_120s"
#                    "f_cco0_Ex_VE3_VHO_o11_2000M_120s"
#                    "f_cco0_In_Eq_Ex_VE1_VHO_o11_2000M_120s"
#                    "f_cco0_In_Eq_Ex_VE2_VHO_o11_2000M_120s"
#                    "f_cco0_In_Eq_Ex_VE3_VHO_o11_2000M_120s"

#                    "f_cco1_In_Eq_Ex_VEAll_VHS_o5_2000M_600s"
#                    "f_cco1_In_Eq_Ex_VEAll_VHR_o5_2000M_600s"
#                    "f_cco1_In_Eq_Ex_VEAll_VHC_o5_2000M_600s"
#                    "f_cco1_In_Eq_Ex_VEAll_VHF_o5_2000M_600s"
#                    "f_cco1_In_Eq_Ex_VEAll_VHO_o5_2000M_600s"
#                    "f_cco1_In_Eq_Ex_VEAll_VHV_o5_2000M_600s"
#                    "f_cco1_In_Eq_Ex_VEAll_VHP_o5_2000M_600s"
#                    "f_cco1_Ex_VEAll_VHF_o5_2000M_600s"
#                    "f_cco1_Ex_VEAll_VHV_o5_2000M_600s"
#                    "Ex_VEAll_VHF_o5_2000M_600s"
#                    "Ex_VEAll_VHV_o5_2000M_600s"
#                    "f_cco1_In_Eq_Ex_VEAll_VHS_o11_2000M_600s"
#                    "f_cco1_In_Eq_Ex_VEAll_VHR_o11_2000M_600s"
#                    "f_cco1_In_Eq_Ex_VEAll_VHC_o11_2000M_600s"
#                    "f_cco1_In_Eq_Ex_VEAll_VHF_o11_2000M_600s"
#                    "f_cco1_In_Eq_Ex_VEAll_VHO_o11_2000M_600s"
#                    "f_cco1_In_Eq_Ex_VEAll_VHV_o11_2000M_600s"
#                    "f_cco1_In_Eq_Ex_VEAll_VHP_o11_2000M_600s"
#                    "f_cco1_Ex_VEAll_VHF_o11_2000M_600s"
#                    "f_cco1_Ex_VEAll_VHV_o11_2000M_600s"
#                    "Ex_VEAll_VHF_o11_2000M_600s"
#                    "Ex_VEAll_VHV_o11_2000M_600s"
#                    "f_cco1_In_Eq_Ex_VEAll_VHS_o17_2000M_600s"
#                    "f_cco1_In_Eq_Ex_VEAll_VHR_o17_2000M_600s"
#                    "f_cco1_In_Eq_Ex_VEAll_VHC_o17_2000M_600s"
#                    "f_cco1_In_Eq_Ex_VEAll_VHF_o17_2000M_600s"
#                    "f_cco1_In_Eq_Ex_VEAll_VHO_o17_2000M_600s"
#                    "f_cco1_In_Eq_Ex_VEAll_VHV_o17_2000M_600s"
#                    "f_cco1_In_Eq_Ex_VEAll_VHP_o17_2000M_600s"
#                    "f_cco1_Ex_VEAll_VHF_o17_2000M_600s"
#                    "f_cco1_Ex_VEAll_VHV_o17_2000M_600s"
#                    "Ex_VEAll_VHF_o17_2000M_600s"
#                    "Ex_VEAll_VHV_o17_2000M_600s"
#                    "Ex_VEAll_VHS_o0_2000M_600s"
#                    "f_cco1_Ex_VEAll_VHS_o0_2000M_600s"

                  )

numruns=${#SBSAT_SHORT_DESCS[@]}

DATE=`date +%Y%m%d-%H%M`
STATS_DIR="../data/snapshot-$DATE"
mkdir $STATS_DIR


STAT_START=0
BENCH_NAME=$[0+$STAT_START]
CNF_READ_TIME=$[1+$STAT_START]
CNF_PROCESS_TIME=$[2+$STAT_START]
CNF_COMPRESSION=$[3+$STAT_START]
CNF_TOTAL_TIME=$[4+$STAT_START]
CLUSTERING_COMPRESSION=$[5+$STAT_START]
TOTAL_COMPRESSION=$[6+$STAT_START]
SMURF_BUILDING_TIME=$[7+$STAT_START]
TOTAL_TIME=$[8+$STAT_START]

STAT_F_MAX=$[9+$STAT_START]

CNF_NUM_CLAUSES=$[0+$STAT_F_MAX]
CNF_NUM_VARIABLES=$[1+$STAT_F_MAX]
RAM_USAGE_BEFORE_READING_CNF=$[2+$STAT_F_MAX]
RAM_USAGE_AFTER_READING_CNF=$[3+$STAT_F_MAX]
RAM_USAGE_AFTER_PROCESSING_CNF=$[4+$STAT_F_MAX]
BDD_NODES_AFTER_PROCESSING_CNF=$[5+$STAT_F_MAX]
NUM_BDDS_AFTER_PROCESSING_CNF=$[6+$STAT_F_MAX]
ERROR_NUM=$[7+$STAT_F_MAX]
NUM_XORS=$[8+$STAT_F_MAX]
NUM_AND_EQUALS=$[9+$STAT_F_MAX]
NUM_ITE_EQUALS=$[10+$STAT_F_MAX]
NUM_MAJV_EQUALS=$[11+$STAT_F_MAX]
NUM_CARDINALITIES=$[12+$STAT_F_MAX]
NUM_UNKNOWN_CLUSTERS=$[13+$STAT_F_MAX]
NUM_UNITS=$[14+$STAT_F_MAX]
NUM_CLAUSES=$[15+$STAT_F_MAX]
NUM_VARIABLES_AFTER_CLUSTERING=$[16+$STAT_F_MAX]
NUM_BDDS_AFTER_CLUSTERING=$[17+$STAT_F_MAX]
NUM_BDD_NODES_AFTER_CLUSTERING=$[18+$STAT_F_MAX]
MAX_RAM_USAGE=$[19+$STAT_F_MAX]
SAT_RESULT=$[20+$STAT_F_MAX]
DROPPEDVAR_STATES_BUILT=$[21+$STAT_F_MAX]
OR_STATES_BUILT=$[22+$STAT_F_MAX]
XOR_STATES_BUILT=$[23+$STAT_F_MAX]
ANDEQUALS_STATES_BUILT=$[24+$STAT_F_MAX]
MINMAX_STATES_BUILT=$[25+$STAT_F_MAX]
NEGMINMAX_STATES_BUILT=$[26+$STAT_F_MAX]
INFERENCE_STATES_BUILT=$[27+$STAT_F_MAX]
XORFACTOR_STATES_BUILT=$[28+$STAT_F_MAX]
SMURF_STATES_BUILT=$[29+$STAT_F_MAX]
STATES_BUILT=$[30+$STAT_F_MAX]
SMURF_STATES_EXAMINED=$[31+$STAT_F_MAX]

STAT_MAX=$[32+$STAT_F_MAX]

MEM_ERR=105
TIME_ERR=107
UNSAT=113
SAT=114
SAT_UNKNOWN=115

for (( i=0; i<${numruns}; i++ ));
do

  DIR=$STATS_DIR/${SBSAT_SHORT_DESCS[$i]}
  mkdir $DIR
  echo "${SBSAT_DESCS[$i]}"
  echo "${SBSAT_DESCS[$i]}" > $DIR/desc.txt

#  for BENCH in $BENCHES
#  do
#    rm -rf $DIR/$BENCH.csv 
#    find ../../itedata/$BENCH -name "*.cnf*" -exec ./sbsat '{}' ${SBSAT_ARGS[$i]} --stats-file $DIR/$BENCH.csv \;
#  done

  # counting
  cat $DIR/*.csv > $DIR/all.csv
  cat $DIR/all.csv | wc -l > $DIR/total.cnt
  cat $DIR/all.csv | awk -v ERROR_NUM=$ERROR_NUM -F, '{ if ($ERROR_NUM == 0) print $0 }' | wc -l > $DIR/num_passed.cnt
  cat $DIR/all.csv | awk -v ERROR_NUM=$ERROR_NUM -F, '{ if ($ERROR_NUM != 0) print $0 }' | wc -l > $DIR/num_errors.cnt
  cat $DIR/all.csv | awk -v ERROR_NUM=$ERROR_NUM -v MEM_ERR=$MEM_ERR -F, '{ if ($ERROR_NUM == MEM_ERR) print $0 }' | wc -l > $DIR/mem_errors.cnt
  cat $DIR/all.csv | awk -v ERROR_NUM=$ERROR_NUM -v TIME_ERR=$TIME_ERR -F, '{ if ($ERROR_NUM == TIME_ERR) print $0 }' | wc -l > $DIR/time_errors.cnt
  cat $DIR/all.csv | awk -v SAT_RESULT=$SAT_RESULT -v SAT_UNKNOWN=$SAT_UNKNOWN -F, '{ if ($SAT_RESULT != SAT_UNKNOWN) print $0 }' | wc -l > $DIR/num_solved.cnt
  cat $DIR/all.csv | awk -v SAT_RESULT=$SAT_RESULT -v SAT_UNKNOWN=$SAT_UNKNOWN -F, '{ if ($SAT_RESULT == SAT_UNKNOWN) print $0 }' | wc -l > $DIR/num_unsolved.cnt

  # cleanup
  cat $DIR/all.csv | awk -v ERROR_NUM=$ERROR_NUM -F, '{ if ($ERROR_NUM == 0) print $0 }' > $DIR/all_errors_removed.csv
  cat $DIR/all_errors_removed.csv | awk -v SAT_RESULT=$SAT_RESULT -v SAT_UNKNOWN=$SAT_UNKNOWN -F, '{ if ($SAT_RESULT == SAT_UNKNOWN) print $0 }' > $DIR/all_unknown_results.csv
  cat $DIR/all_errors_removed.csv | awk -v SAT_RESULT=$SAT_RESULT -v SAT_UNKNOWN=$SAT_UNKNOWN -F, '{ if ($SAT_RESULT != SAT_UNKNOWN) print $0 }' > $DIR/all_known_results.csv

  # specific column selection, sorting
  cut -d , -f $RAM_USAGE_AFTER_PROCESSING_CNF $DIR/all_errors_removed.csv | sort -n -k 1 > $DIR/all_ram_sorted.csv
  cut -d , -f $CNF_COMPRESSION $DIR/all_errors_removed.csv | sort -n -k 1 > $DIR/all_cnf_compression_sorted.csv
  cut -d , -f $CLUSTERING_COMPRESSION $DIR/all_errors_removed.csv | sort -n -k 1 > $DIR/all_clustering_compression_sorted.csv
  cut -d , -f $TOTAL_COMPRESSION $DIR/all_errors_removed.csv | sort -n -k 1 > $DIR/all_total_compression_sorted.csv
  cut -d , -f $CNF_TOTAL_TIME $DIR/all_errors_removed.csv | sort -n -k 1 > $DIR/all_cnf_time_sorted.csv
  cut -d , -f $TOTAL_TIME $DIR/all_errors_removed.csv | sort -n -k 1 > $DIR/all_total_time_sorted.csv
  cut -d , -f $NUM_BDDS_AFTER_PROCESSING_CNF $DIR/all_errors_removed.csv | sort -n -k 1 > $DIR/all_num_bdds_sorted.csv

  cut -d , -f $CNF_NUM_CLAUSES $DIR/all_unknown_results.csv | sort -n -k 1 > $DIR/unknown_num_clauses.csv
  cut -d , -f $NUM_BDDS_AFTER_PROCESSING_CNF $DIR/all_unknown_results.csv | sort -n -k 1 > $DIR/unknown_num_bdds_after_processing_cnf_sorted.csv
  cut -d , -f $NUM_VARIABLES_AFTER_CLUSTERING $DIR/all_unknown_results.csv | sort -n -k 1 > $DIR/unknown_num_variables_after_clustering_sorted.csv
  cut -d , -f $NUM_BDDS_AFTER_CLUSTERING $DIR/all_unknown_results.csv | sort -n -k 1 > $DIR/unknown_num_bdds_after_clustering_sorted.csv
  cut -d , -f $NUM_BDD_NODES_AFTER_CLUSTERING $DIR/all_unknown_results.csv | sort -n -k 1 > $DIR/unknown_num_bdd_nodes_after_clustering_sorted.csv
  cut -d , -f $SMURF_STATES_BUILT $DIR/all_unknown_results.csv | sort -n -k 1 > $DIR/unknown_smurf_states_built_sorted.csv

  # summing columns
  awk -v CNF_TOTAL_TIME=$CNF_TOTAL_TIME '{sum=sum+$CNF_TOTAL_TIME} END {printf("%4.3f\n", sum)}' < $DIR/all_errors_removed.csv > $DIR/all_cnf_time.cnt
  awk -v TOTAL_TIME=$TOTAL_TIME '{sum=sum+$TOTAL_TIME} END {printf("%4.3f\n", sum)}' < $DIR/all_errors_removed.csv > $DIR/all_total_time.cnt
done
