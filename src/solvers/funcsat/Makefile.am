EXTRA_DIST = README
SUBDIRS = src .
dist_doc_DATA = README

AM_CFLAGS = $(CODEMODEL) $(SHARED_CFLAGS) $(OPT_CFLAGS) $(DEBUG_CFLAGS) $(WARN_CFLAGS) $(PROF_CFLAGS)
AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)
AM_LDFLAGS = $(CODEMODEL)

ACLOCAL_AMFLAGS = --install -I m4

bin_PROGRAMS = funcsat check_solution

funcsat_SOURCES = main.c
funcsat_LDADD = src/libfuncsat.la
funcsat_CPPFLAGS = -I$(srcdir)/src

check_solution_SOURCES = check_solution.c
check_solution_LDADD = src/libfuncsat.la
check_solution_CPPFLAGS = -I$(srcdir)/src
