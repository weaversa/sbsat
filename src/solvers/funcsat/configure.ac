dnl Copyright 2012 Sandia Corporation. Under the terms of Contract
dnl DE-AC04-94AL85000, there is a non-exclusive license for use of this work by
dnl or on behalf of the U.S. Government. Export of this program may require a
dnl license from the United States Government.

AC_INIT([funcsat], [2.14], [denis.bueno@sandia.gov])
dnl version is <bitfunc-version.funcsat-revision>
m4_pattern_forbid([^AX_])
:${CFLAGS=""}
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT
AM_INIT_AUTOMAKE([-Wall foreign])
AC_CONFIG_SRCDIR(main.c)
AC_CONFIG_MACRO_DIR([m4])
dnl BASIC_CFLAGS="-Wall -std=c99"
dnl DEFAULT_CFLAGS="$BASIC_CFLAGS -O3 -fexpensive-optimizations -fomit-frame-pointer -fno-strict-aliasing"
dnl CHOSEN_CFLAGS="$DEFAULT_CFLAGS"
dnl SHARED_CFLAGS=
dnl SHARED_CXXFLAGS=

dnl -Wsign-conversion only works in more-recent-than 4.2.1 -Wno-deprecated gcc?
WARN_CFLAGS="-Wall -Wextra -Wno-unused "
OPT_CFLAGS="-O3 -DNDEBUG -fexpensive-optimizations -fomit-frame-pointer -fno-strict-aliasing"
#OPT_CFLAGS="-DNDEBUG"
DEBUG_CFLAGS=
SHARED_CFLAGS=
PROF_CFLAGS=

AC_ARG_ENABLE([codemodel],
  AC_HELP_STRING([--enable-codemodel],[32, 64, or native (default native)]),
  [if test "x${enableval}" = "x32"; then CODEMODEL="-m32";
   elif test "x${enableval}" = "x64"; then CODEMODEL="-m64";
   elif test "x${enableval}" = "xnative"; then CODEMODEL=""; fi],
  [CODEMODEL=""]
  )

AC_ARG_ENABLE([debug],
  AC_HELP_STRING([--enable-debug],[turn on debugging]),
  [if test "x${enableval}" = "xyes"; then
     DEBUG_CFLAGS="-g"; enable_debug="yes"
   elif test "x${enableval}" = "xno"; then :
   else AC_MSG_ERROR([Invalid value ${enableval} to --enable-debug]); fi])

AC_ARG_ENABLE([shared],
  AC_HELP_STRING([--enable-shared],[enable -fPIC]),
  [if test "x${enableval}" = "xyes"; then SHARED_CFLAGS="-fPIC";
   elif test "x${enableval}" = "xno"; then :
   else AC_MSG_ERROR([Invalid value ${enableval} to --enable-shared]); fi])

AC_ARG_ENABLE([optimize],
  AC_HELP_STRING([--disable-optimize],[turn off optimizations]),
  [if test "x${enableval}" = "xno"; then OPT_CFLAGS="-O0";
   elif test "x${enableval}" = "xyes"; then :
   else OPT_CFLAGS="${enableval}"; fi])

dnl some optimizations (-fomit-frame-pointer) are incompatible with profiling,
dnl so disable them
AC_ARG_ENABLE([profiling],
  AC_HELP_STRING([--enable-profiling],[turn on profiling with -pg]),
  [if test "x${enableval}" = "xyes"; then PROF_CFLAGS="-pg -g -O3"; OPT_CFLAGS=""
   elif test "x${enableval}" = "xno"; then :
   else AC_MSG_ERROR([Invalid value ${enableval} to --enable-profiling]); fi])

AC_ARG_ENABLE([sharkprof],
  AC_HELP_STRING([--enable-sharkprof],[best options for profiling with Shark (OS X)]),
  [if test "x${enableval}" = "xyes"; then PROF_CFLAGS="-O3 -g"; OPT_CFLAGS=""
   elif test "x${enableval}" = "xno"; then :
   else AC_MSG_ERROR([Invalid value ${enableval} to --enable-sharkprof]); fi])

AC_ARG_ENABLE([log], AC_HELP_STRING([--enable-log],[enable level-based logging (off by default)]))
AS_IF([test "x$enable_log" = "xyes"],[AC_DEFINE([FUNCSAT_LOG],[1],[Define to 1 to compile in level-based logging.])])

AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_LIBTOOL
if test "x${ac_cv_prog_x99}" = "xno"; then
  AC_MSG_ERROR([compiler does not support C99])
fi
AM_PROG_CC_C_O
AC_CONFIG_HEADERS([src/funcsat/config.h])
AC_CONFIG_FILES([
 Makefile
 src/Makefile
 test/Makefile
])

AC_ARG_VAR([CTANGLE], [ctangle executable])
AC_CHECK_PROGS([CTANGLE], [ctangle])
AS_IF([test "x$CTANGLE" = "x"], [AC_MSG_ERROR([ctangle not found, ensure cweb system is available])])
AC_ARG_VAR([CWEAVE], [cweave executable])
AC_CHECK_PROGS([CWEAVE], [cweave])
AS_IF([test "x$CWEAVE" = "x"], [AC_MSG_ERROR([cweave not found, ensure cweb system is available])])
AC_ARG_VAR([PDFTEX], [pdftex executable])
AC_CHECK_PROGS([PDFTEX], [pdftex])
AS_IF([test "x$PDFTEX" = "x"], [AC_MSG_ERROR([pdftex not found, ensure cweb system is available])])


dnl AC_ARG_ENABLE([proof],AC_HELP_STRING([--enable-proof],[enable resolution proof generation]),AC_DEFINE([PROOFS],[],[enable resolution proof generation]))

dnl CFLAGS="$SHARED_CFLAGS $OPT_CFLAGS $DEBUG_CFLAGS $WARN_CFLAGS $PROF_CFLAGS $CFLAGS"


AC_SUBST([CODEMODEL])
AC_SUBST([FUNCSAT_CPPFLAGS])
AC_SUBST([FUNCSAT_LDFLAGS])
AC_SUBST([DEBUG_CFLAGS])
AC_SUBST([OPT_CFLAGS])
AC_SUBST([WARN_CFLAGS])
AC_SUBST([SHARED_CFLAGS])
AC_SUBST([PROF_CFLAGS])
AC_HEADER_STDC()
AC_HEADER_ASSERT()
AC_HEADER_STAT()
AC_HEADER_STDBOOL()
AC_CHECK_HEADERS([limits.h])
AC_CHECK_HEADERS([sys/resource.h])
AC_CHECK_HEADERS([zlib.h],AC_CHECK_LIB(z,gzopen),AC_MSG_WARN([reading .cnf.gz disabled]))
AC_SEARCH_LIBS(log10,m,,AC_MSG_ERROR([need log10]))
dnl CFLAGS="$CFLAGS -Werror"
AC_OUTPUT
