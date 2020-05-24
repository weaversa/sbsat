SUBDIRS= include fn_smurf fn_inference fn_droppedvar fn_or fn_orcounter fn_xor fn_xorcounter fn_minmax fn_minmaxcounter fn_negminmax fn_negminmaxcounter fn_xorfactor fn_andequals fn_andequalscounter # bb_gelim bb_lemmas

INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solvers/smurf/include -I$(top_srcdir)/src/solvers/funcsat/src
sources= smurf_cache.c solve_smurf_init.c solve_smurf_core.c smurf_print.c smurf_heuristic.c

noinst_LTLIBRARIES = libsmurf_solver.la
libsmurf_solver_la_SOURCES= $(sources)
