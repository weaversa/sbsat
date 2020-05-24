SUBDIRS= include smurf gelim picosat funcsat

INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solvers/smurf/include -I$(top_srcdir)/src/solvers/funcsat/src
sources= solve_smurf.c solve_picosat.c solve_funcsat.c

noinst_LTLIBRARIES = libsolvers.la
libsolvers_la_SOURCES= $(sources)
