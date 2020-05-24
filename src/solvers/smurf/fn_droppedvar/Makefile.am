INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solvers/smurf/include -I$(top_srcdir)/src/solvers/funcsat/src

sources= fn_droppedvar.c bdd2droppedvar.c bt_droppedvar.c # hr_lsgb_droppedvar_init.c hr_lsgb_droppedvar_update.c

noinst_LTLIBRARIES = libfndroppedvar_solver.la
libfndroppedvar_solver_la_SOURCES= $(sources)
