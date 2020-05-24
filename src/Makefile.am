SUBDIRS = cudd utils formats bddops solvers #abc

INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solvers/funcsat/src
sources=sbsat.c
sbsat_SOURCES=sbsat.c
bin_PROGRAMS=sbsat

SBSAT_LIBS= \
   formats/libformats.la \
   bddops/libbddops.la \
   utils/libutils.la \
   cudd/cudd/libcudd.la \
   cudd/epd/libepd.la \
   cudd/mtr/libmtr.la \
   cudd/st/libst.la \
   cudd/util/libcuddutil.la \
   solvers/libsolvers.la \
   solvers/funcsat/src/libfuncsat.la \
   solvers/picosat/libpicosat_solver.la \
   solvers/smurf/libsmurf_solver.la \
   solvers/gelim/libgelim_solver.la \
   solvers/smurf/fn_smurf/libfnsmurf_solver.la \
   solvers/smurf/fn_inference/libfninference_solver.la \
   solvers/smurf/fn_droppedvar/libfndroppedvar_solver.la \
   solvers/smurf/fn_or/libfnor_solver.la \
   solvers/smurf/fn_orcounter/libfnorcounter_solver.la \
   solvers/smurf/fn_xor/libfnxor_solver.la \
   solvers/smurf/fn_xorcounter/libfnxorcounter_solver.la \
   solvers/smurf/fn_minmax/libfnminmax_solver.la \
   solvers/smurf/fn_minmaxcounter/libfnminmaxcounter_solver.la \
   solvers/smurf/fn_negminmax/libfnnegminmax_solver.la \
   solvers/smurf/fn_negminmaxcounter/libfnnegminmaxcounter_solver.la \
   solvers/smurf/fn_xorfactor/libfnxorfactor_solver.la \
   solvers/smurf/fn_andequals/libfnandequals_solver.la \
   solvers/smurf/fn_andequalscounter/libfnandequalscounter_solver.la

#  uncomment the following if sbsat requires the math library
#LIBM=-lm

sbsat_LDADD =  $(SBSAT_LIBS) $(LIBM)
if SBSAT_STATIC
sbsat_LDFLAGS =  -all-static
else
#sbsat_LDFLAGS =  -static
sbsat_LDFLAGS =
endif
