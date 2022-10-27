/*
   sbsat - SBSAT is a state-based Satisfiability solver.

   Copyright (C) 2006 Sean Weaver

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef SBSAT_HEADERS_H
#define SBSAT_HEADERS_H

#if HAVE_CONFIG_H
#include "config.h"
#endif


# include <stdio.h>

#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#if HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#if HAVE_TIME_H
# include <time.h>
#endif
#if HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif
#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#if HAVE_STRING_H
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif
#if HAVE_STRINGS_H
# include <strings.h>
#endif
#if HAVE_INTTYPES_H
# include <inttypes.h>
#else
# if HAVE_STDINT_H
#  include <stdint.h>
# endif
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#if HAVE_CTYPE_H
# include <ctype.h>
#endif
#if HAVE_LIMITS_H
# include <limits.h>
#endif
#if HAVE_ASSERT_H
# include <assert.h>
#endif
#if HAVE_SIGNAL_H
# include <signal.h>
#endif
#if HAVE_MATH_H
# include <math.h>
#endif
#if HAVE_TERMCAP_H
# include <termcap.h>
#endif
#if HAVE_TERMIOS_H
# include <termios.h>
#endif

#include <gmp.h>
#include <regex.h>

#ifndef INT_MAX
#define INT_MAX 0x7FFFFFFF
#endif

#ifndef UINT_MAX
#define UINT_MAX 0xFFFFFFFE
#endif

/* system */
#ifdef HAVE_USING_NAMESPACE_STD
using namespace std;
#endif

#endif
