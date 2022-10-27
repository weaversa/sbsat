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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include <unistd.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>
#include <signal.h>
#include <math.h>
#include <termcap.h>
#include <termios.h>
#include <gmp.h>
#include <regex.h>
#include <sys/time.h>
#include <time.h>

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
