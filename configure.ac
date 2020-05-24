# -*- Autoconf -*- # Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
# watch out -- two places to change the version
AC_INIT([sbsat],[3.0],[weaversa@gmail.com])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT
AC_LANG([C])
AM_INIT_AUTOMAKE(sbsat,3.0)
AC_CONFIG_SRCDIR([src/sbsat.c])
AC_CONFIG_HEADER([include/config.h])
AC_CONFIG_MACRO_DIR([m4])

m4_ifndef([AM_SILENT_RULES], [m4_define([AM_SILENT_RULES],[])])
AM_SILENT_RULES([yes])

saved_CXXFLAGS=${CXXFLAGS}
AC_PROG_CC
AC_PROG_CC_C99
if test "x${ac_cv_prog_x99}" = "xno"; then
  AC_MSG_ERROR([compiler does not support C99])
fi
AM_PROG_CC_C_O
CXXFLAGS=${saved_CXXFLAGS}


# compile with optimization and without debugging by default
MYCXXFLAGS=""
MYCCFLAGS=""
MYCFLAGS="-std=c99"
INCL=""

AC_ARG_ENABLE(optimization, 
[  --enable-optimization   Turn on compiler optimization (default=no)],
    [if eval "test x$enable_optimization = xyes"; then
        MYCXXFLAGS="-O3 -DNDEBUG -fno-strict-aliasing -ffast-math -fomit-frame-pointer"
        MYCCFLAGS="-xO6 -KPIC -dalign -native -Wno-unused"
	MYCFLAGS="${MYCXXFLAGS} "
    fi])

AC_ARG_ENABLE(openmp,
[  --enable-openmp	Turn on openmp parallel segemts (default=no)],
    [if eval "test x$enable_openmp = xyes"; then
        MYCXXFLAGS="${MYCXXFLAGS} -fopenmp "
	MYCCFLAGS="${MYCCFLAGS} -fopenmp "
	MYCFLAGS="${MYCFLAGS} -fopenmp "
    fi])

AC_ARG_ENABLE(debug, 
[  --enable-debug          Turn on compiler debugging information (default=no)],
    [if eval "test x$enable_debug = xyes"; then
        MYCXXFLAGS="${MYCXXFLAGS} -g -Wall -O0 "
        MYCCFLAGS="${MYCCFLAGS} -g -Wall -O0 "
        MYCFLAGS="${MYCFLAGS} -g -Wall -O0 "
    fi])

AC_ARG_ENABLE(profile, 
[  --enable-profile        Turn on compiler profiling information (default=no)],
    [if eval "test x$enable_profile = xyes"; then
       if eval "test x$enable_optimization = xyes"; then
	echo "error: can not turn on profiling and extra optimization"
	exit
       else
         MYCXXFLAGS="${MYCXXFLAGS}  -pg -Wall "
         MYCCFLAGS="${MYCCFLAGS}  -pg -Wall "
         MYCFLAGS="${MYCFLAGS}  -pg -Wall "
       fi
    fi])

AC_ARG_ENABLE(extrawarning, 
[  --enable-extrawarning   Turn on compiler extra warning information (default=no)],
    [if eval "test x$enable_extrawarning = xyes"; then
	MYCXXFLAGS="${MYCXXFLAGS} -pedantic -W -Winline -Wpointer-arith -Wcast-qual -Wcast-align -Wsign-compare -Wno-unused -Wconversion -Waggregate-return -Wshadow -Wwrite-strings -Wno-long-long"
    fi])

AC_ARG_ENABLE(coverage, 
[  --enable-coverage       Turn on compiler coverage information (default=no)],
    [if eval "test x$enable_coverage = xyes"; then
       if eval "test x$enable_profile = xyes"; then
	echo "error: can not turn on coverage and profiling"
	exit
       else
         MYCXXFLAGS="${MYCXXFLAGS}  -fprofile-arcs -ftest-coverage "
         MYCCFLAGS="${MYCCFLAGS}  -fprofile-arcs -ftest-coverage "
         MYCFLAGS="${MYCFLAGS}  -fprofile-arcs -ftest-coverage "
       fi
    fi])
 
AC_ARG_ENABLE(32bits,
[AS_HELP_STRING([--enable-32bits],[force build 32 bit version])],
    [if eval "test x$enable_32bits = xyes"; then
        FORCED_BITS=no
        MYCXXFLAGS="${MYCXXFLAGS} -m32 -DSIZEOF_VOID_P=4 -DSIZEOF_LONG=4 -DLIN "
        MYCCFLAGS="${MYCCFLAGS} -m32 -DSIZEOF_VOID_P=4 -DSIZEOF_LONG=4 -DLIN "
        MYCFLAGS="${MYCFLAGS} -m32 -DSIZEOF_VOID_P=4 -DSIZEOF_LONG=4 -DLIN "
    fi])

AC_ARG_ENABLE(64bits,
[AS_HELP_STRING([--enable-64bits],[force build 64 bit version])],
[ 
   if test "x$enableval" = "xyes" ; then
        orig_ccflags=$CCFLAGS
        orig_cxxflags=$CXXFLAGS
        FORCED_BITS=yes
        AC_COMPILE_IFELSE(
            [ AC_LANG_PROGRAM([], [dnl
                 return sizeof(void*) == 8 ? 0 : 1;
               ])
            ],[
                AC_MSG_CHECKING([force 64 bit target])
	        AC_MSG_RESULT(yes)
                AC_RUN_IFELSE(
                    [ AC_LANG_PROGRAM([], [dnl
                        return sizeof(void*) == 8 ? 0 : 1;
                    ])
                ],[
                    AC_MSG_CHECKING([64 bit pointer])
                    CCFLAGS="-m64 $orig_ccflags -DSIZEOF_VOID_P=8 -DSIZEOF_LONG=8 -DLIN64"
                    CXXFLAGS="-m64 $orig_cxxflags -DSIZEOF_VOID_P=8 -DSIZEOF_LONG=8 -DLIN64"
                    CFLAGS="-m64 -DSIZEOF_VOID_P=8 -DSIZEOF_LONG=8 -DLIN64"
	            AC_DEFINE(SBSAT_64BIT,true,[Do we have 64 bit enabled?])
	            AC_MSG_RESULT(yes)
                ],[
                    AC_MSG_CHECKING([64 bit pointer])
                    CCFLAGS=$orig_ccflags;
                    CXXFLAGS=$orig_cxxflags;
	            AC_MSG_RESULT(no)
                ])
            ],[
               AC_MSG_CHECKING([64 bit compiler])
               CCFLAGS=$orig_ccflags;
               CXXFLAGS=$orig_cxxflags;
	       AC_MSG_RESULT(no)
            ])
        fi ])

 
AC_MSG_CHECKING([autodetect 64 bit compiler ])
if eval "test x$FORCED_BITS = xyes"; then
        AC_MSG_RESULT(skip)
else
        AC_MSG_RESULT(go ahead)
        orig_ccflags=$CCFLAGS
        orig_cxxflags=$CXXFLAGS
        FORCED_BITS=yes
        AC_COMPILE_IFELSE(
            [ AC_LANG_PROGRAM([], [dnl
                 return sizeof(void*) == 8 ? 0 : 1;
               ])
            ],[
                AC_MSG_CHECKING([64 bit compiler])
	        AC_MSG_RESULT(yes)
                AC_RUN_IFELSE(
                    [ AC_LANG_PROGRAM([], [dnl
                        return sizeof(void*) == 8 ? 0 : 1;
                    ])
                ],[
                    AC_MSG_CHECKING([64 bit pointer])
                    CCFLAGS="-m64 $orig_ccflags -DSIZEOF_VOID_P=8 -DSIZEOF_LONG=8 -DLIN64"
                    CXXFLAGS="-m64 $orig_cxxflags -DSIZEOF_VOID_P=8 -DSIZEOF_LONG=8 -DLIN64"
                    CFLAGS="-m64 -DSIZEOF_VOID_P=8 -DSIZEOF_LONG=8 -DLIN64"
	            AC_DEFINE(SBSAT_64BIT,true,[Do we have 64 bit enabled?])
	            AC_MSG_RESULT(yes)
                ],[
                    AC_MSG_CHECKING([64 bit pointer])
                    CCFLAGS=$orig_ccflags;
                    CXXFLAGS=$orig_cxxflags;
	            AC_MSG_RESULT(no)
                ])
            ],[
               AC_MSG_CHECKING([64 bit compiler])
               CCFLAGS=$orig_ccflags;
               CXXFLAGS=$orig_cxxflags;
	       AC_MSG_RESULT(no)
            ])
        fi

sbsat_static=false
AC_ARG_ENABLE(static, 
[  --enable-static         Turn on linking libraries staticly (default=no)],
    [sbsat_static=true])

AM_CONDITIONAL(SBSAT_STATIC, test x$sbsat_static = xtrue)

# Extra libraries 'with'-options
# AC_ARG_WITH(gd-inc,[  --with-gd-inc=DIR       location of the gd includes],
# [INCL="${INCL} -I${withval}"])
   
# AC_ARG_WITH(gd-lib,[  --with-gd-lib=DIR       location of the gd library],
# [LIBS="${LIBS} -L${withval}"])

CXXFLAGS="${CXXFLAGS} ${MYCXXFLAGS} ${INCL}"
CFLAGS="${CFLAGS} ${MYCFLAGS} ${INCL} -DHAVE_IEEE_754"
CCFLAGS="${CCFLAGS} ${MYCCFLAGS} ${INCL}"

cvsbuild=false
AC_ARG_ENABLE(cvsbuild,
[  --enable-cvsbuild Create nightly build],
[case "${enableval}" in
  no) cvsbuild=false ;;
  *) cvsbuild=${enableval} ;;
esac],[debug=false])
AM_CONDITIONAL(NO_CVSBUILD, test x$cvsbuild = xfalse)
CVSBUILD=$cvsbuild
AC_SUBST(CVSBUILD)

AC_ARG_WITH(java-os,
[  --with-java-os=OS       Use OS to include machine-dependent java header],[
  if test "$withval" != "no" ; then
    if test "$withval" != "yes"; then
      JAVA_OS_INCL="$withval"
    else
      AC_MSG_ERROR([Missing argument for --with-java-os])
   fi
  fi
])

AC_ARG_WITH(java,
[  --with-java[[=DIR]]       Use java to create shared object use JDK DIR],[
  if test "$withval" != "no" ; then
    if test "$withval" != "yes"; then
      JAVA_DIR=$withval
      CPPFLAGS="${CPPFLAGS} -I$withval/include"
      LDFLAGS="${LDFLAGS} -L$withval/lib"
    else
      AC_CHECK_FILE(
         [$JAVA_HOME/include/jni.h],[
            JAVA_DIR="$JAVA_HOME"
            CPPFLAGS="${CPPFLAGS} -I$JAVA_DIR/include"
            LDFLAGS="${LDFLAGS} -L$JAVA_DIR/lib"
         ],[])
   fi
   if test x$JAVA_OS_INCL != "x" ; then
      JAVA_OS_INCL=" -I$JAVA_DIR/include/$JAVA_OS_INCL "
   fi
   CPPFLAGS="${CPPFLAGS} ${JAVA_OS_INCL}";
   AC_CHECK_HEADERS(jni_md.h, [], 
        AC_MSG_ERROR(
            [Could not find jni_md.h include file please check JAVA_HOME enviroment variable or specify --with-java=DIR and specify --with-java-os=OS or check config.log.]))
   AC_CHECK_HEADERS(jni.h,
      [], AC_MSG_ERROR(
            [Could not find jni.h include file please check JAVA_HOME enviroment variable or specify --with-java=DIR or check config.log.]))
 fi
], [])

AC_C_INLINE

dnl Check for "extern inline".
AC_CACHE_CHECK(
    for extern inline,
    ac_cv_c_extern_inline,
    [
    ac_cv_c_extern_inline=no
    AC_TRY_COMPILE(
        [
        extern $ac_cv_c_inline double foo(double x);
        extern $ac_cv_c_inline double foo(double x) { return x+1.0; };
        double foo (double x) { return x + 1.0; };
        ],
        [foo(1.0)],
        [ac_cv_c_extern_inline="yes"])])
if test "$ac_cv_c_extern_inline" != no ; then
    AC_DEFINE(HAVE_EXTERN_INLINE, 1,
              Define to 1 if your compiler supports extern inline)
fi


#AC_ARG_WITH(zlib,
#[  --with-zlib[[=DIR]]       use libz in DIR],[
  #if test "$withval" != "no" -a "$withval" != "yes"; then
    #Z_DIR=$withval
    #CPPFLAGS="${CPPFLAGS} -I$withval/include"
    #LDFLAGS="${LDFLAGS} -L$withval/lib"
  #fi
#])
#if test "$with_zlib" = "no"; then
    #echo "Disabling compression support"
#else
    #AC_CHECK_HEADERS(zlib.h,
        #AC_CHECK_LIB(z, gzread,[
            #AC_DEFINE(HAVE_LIBZ)
            #if test "x${Z_DIR}" != "x"; then
                #Z_CFLAGS="-I${Z_DIR}/include"
                #Z_LIBS="-L${Z_DIR}/lib -lz"
                #[case ${host} in
                    #*-*-solaris*)
                        #Z_LIBS="-L${Z_DIR}/lib -R${Z_DIR}/lib -lz"
                        #;;
                #esac]
            #else
                #Z_LIBS="-lz"
            #fi]))
#fi
#
#AC_SUBST(Z_CFLAGS)
#AC_SUBST(Z_LIBS)
AC_ARG_ENABLE(alllibtool,
[  --enable-alllibtool    Turn on libtool for the whole tree],
[case "${enableval}" in
  yes) alllibtool=true ;;
  no)  alllibtooldebug=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-alllibtool) ;;
esac],[alllibtool=true])
AM_CONDITIONAL(MYLIBTOOL, test x$alllibtool = xtrue)

# Checks for programs.
AC_PROG_AWK
AC_PROG_YACC("bison -y")
AC_PROG_LIBTOOL

AC_SUBST(ECHO_N)
AC_SUBST(ECHO_C)

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h inttypes.h limits.h memory.h stddef.h stdint.h stdlib.h string.h strings.h sys/param.h sys/time.h termios.h unistd.h])

# Also check for these headers
AC_CHECK_HEADERS([string])
AC_CHECK_HEADERS([signal.h assert.h])
# AC_CHECK_HEADERS([iostream.h fstream.h iostream fstream ])
AC_CHECK_HEADERS([sys/types.h sys/times.h sys/resource.h sys/wait.h])
AC_CHECK_HEADERS([sys/stat.h ctype.h math.h regex.h])
AC_CHECK_HEADERS([ncurses/termcap.h])
AC_CHECK_HEADERS([termcap.h])
AC_CHECK_HEADERS([proc/readproc.h])
AC_CHECK_HEADERS([gmp.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
# AC_FUNC_MALLOC
AC_FUNC_MMAP
# AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_CHECK_FUNCS([popen getpagesize gettimeofday memmove memset munmap select sqrt strcasecmp strchr strncasecmp strrchr])
AC_SUBST(LIBM)

AC_CHECK_LIB(termcap,tgetnum,[
	AC_DEFINE([HAVE_TERMCAP_LIB], [], [Have termcap support])
	LIBS="$LIBS -ltermcap"	
])

AC_CHECK_LIB(readline,tgetnum,[
	AC_DEFINE([HAVE_READLINE_LIB], [], [Have readline support])
	LIBS="$LIBS -lreadline"	
])

AC_CHECK_LIB(ncurses,tgetnum,[
	AC_DEFINE([HAVE_NCURSES_LIB], [], [Have ncurses support])
	LIBS="$LIBS -lncurses"	
])

AC_CHECK_LIB(gmp, __gmpz_init,[
	AC_DEFINE([HAVE_GMP_LIB], [], [Have gmp support])
	LIBS="$LIBS -lgmp"	
])

##                 src/abc/Makefile
##                 src/abc/misc/Makefile
##                 src/abc/misc/extra/Makefile
##                 src/abc/misc/util/Makefile
##                 src/abc/misc/vec/Makefile
##                 src/abc/aig/Makefile
##                 src/abc/aig/aig/Makefile
##                 src/abc/aig/bdc/Makefile
##                 src/abc/aig/hop/Makefile
##                 src/abc/aig/ioa/Makefile
##                 src/abc/aig/kit/Makefile
##                 src/abc/aig/mem/Makefile
##                 src/abc/aig/tim/Makefile


AC_CONFIG_FILES([Makefile
                 include/Makefile
                 src/cudd/Makefile
                 src/cudd/cudd/Makefile
                 src/cudd/epd/Makefile
                 src/cudd/include/Makefile
                 src/cudd/mtr/Makefile
                 src/cudd/st/Makefile
                 src/cudd/util/Makefile
		 src/utils/Makefile
                 src/formats/Makefile
                 src/bddops/Makefile
                 src/solvers/include/Makefile
                 src/solvers/gelim/Makefile
                 src/solvers/picosat/Makefile
                 src/solvers/smurf/Makefile
                 src/solvers/smurf/include/Makefile
                 src/solvers/smurf/fn_smurf/Makefile
                 src/solvers/smurf/fn_inference/Makefile
                 src/solvers/smurf/fn_droppedvar/Makefile
                 src/solvers/smurf/fn_or/Makefile
                 src/solvers/smurf/fn_orcounter/Makefile
                 src/solvers/smurf/fn_xor/Makefile
                 src/solvers/smurf/fn_xorcounter/Makefile
                 src/solvers/smurf/fn_minmax/Makefile
                 src/solvers/smurf/fn_minmaxcounter/Makefile
                 src/solvers/smurf/fn_negminmax/Makefile
                 src/solvers/smurf/fn_negminmaxcounter/Makefile
                 src/solvers/smurf/fn_xorfactor/Makefile
                 src/solvers/smurf/fn_andequals/Makefile
                 src/solvers/smurf/fn_andequalscounter/Makefile
                 src/solvers/Makefile
                 src/Makefile])
AC_CONFIG_SUBDIRS([src/solvers/funcsat])
AC_OUTPUT
