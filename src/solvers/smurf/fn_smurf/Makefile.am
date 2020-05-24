INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solvers/smurf/include -I$(top_srcdir)/src/solvers/funcsat/src

sources= fn_smurf.c bdd2smurf.c bt_smurf.c hr_lsgb_smurf_init.c \
         hr_lsgb_smurf_update.c

noinst_LTLIBRARIES = libfnsmurf_solver.la
libfnsmurf_solver_la_SOURCES= $(sources)
