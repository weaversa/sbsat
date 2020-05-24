INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solvers/funcsat/src
sources=bdd_manager.c bdd_ops.c clustering_ve.c type_detection.c \
        bdd_print.c clustering_heuristics.c bdd_inferences.c \
        inference_queue.c early_quantification.c safe_assignments.c

noinst_LTLIBRARIES = libbddops.la
libbddops_la_SOURCES=$(sources)
