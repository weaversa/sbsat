INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solvers/smurf/include -I$(top_srcdir)/src/solvers/funcsat/src

sources= fn_inference.c bdd2inference.c bt_inference.c hr_lsgb_inference_init.c \
         hr_lsgb_inference_update.c

noinst_LTLIBRARIES = libfninference_solver.la
libfninference_solver_la_SOURCES= $(sources)
