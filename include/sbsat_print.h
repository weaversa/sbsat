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

#ifndef SBSAT_PRINT_H
#define SBSAT_PRINT_H
/*
 * What level of debugging gets compiled in.
 * Change this to a lower number for better performance
 */
#define _DEBUG_LVL_COMPILE 10

extern FILE *stddbg;
extern uint8_t DEBUG_LVL;

#define D_E(x) { x }
#define D_0(x) { x }

#if _DEBUG_LVL_COMPILE >= 1
# define D_1(x) { if ((DEBUG_LVL&15) >= 1) { x; } }
#else
# define D_1(x) {}
#endif

#if _DEBUG_LVL_COMPILE >= 2
# define D_2(x) if ((DEBUG_LVL&15) >= 2) { x }
# define D_2E(x) if ((DEBUG_LVL&15) == 2) { x }
# define DM_2(x) D_2(if(DEBUG_LVL&16) { x } )
#else
# define D_2(x) {}
# define D_2E(x) {}
# define DM_2(x) {}
#endif

#if _DEBUG_LVL_COMPILE >= 3
# define D_3(x) if ((DEBUG_LVL&15) >= 3) { x }
#else
# define D_3(x) {}
#endif

#if _DEBUG_LVL_COMPILE >= 3
# define D_3E(x) if ((DEBUG_LVL&15) == 3) { x }
#else
# define D_3E(x) {}
#endif

#if _DEBUG_LVL_COMPILE >= 4
# define D_4(x) if ((DEBUG_LVL&15) >= 4) { x }
#else
# define D_4(x) {}
#endif

#if _DEBUG_LVL_COMPILE >= 5
# define D_5(x) if ((DEBUG_LVL&15) >= 5) { x }
#else
# define D_5(x) {}
#endif

#if _DEBUG_LVL_COMPILE >= 6
# define D_6(x) if ((DEBUG_LVL&15) >= 6) { x }
#else
# define D_6(x) {}
#endif

#if _DEBUG_LVL_COMPILE >= 7
# define D_7(x) if ((DEBUG_LVL&15) >= 7) { x }
#else
# define D_7(x) {}
#endif

#if _DEBUG_LVL_COMPILE >= 8
# define D_8(x) if ((DEBUG_LVL&15) >= 8) { x }
#else
# define D_8(x) {}
#endif

#if _DEBUG_LVL_COMPILE >= 9
# define D_9(x) if ((DEBUG_LVL&15) >= 9) { x }
#else
# define D_9(x) {}
#endif

#if _DEBUG_LVL_COMPILE >= 10
# define D_10(x) if ((DEBUG_LVL&15) >= 10) { x }
#else
# define D_10(x) {}
#endif

#define dE_printf1(x)        D_E(fprintf(stddbg, x);)
#define dE_printf2(x1,x2)    D_E(fprintf(stddbg, x1, x2);)
#define dE_printf3(x1,x2,x3) D_E(fprintf(stddbg, x1, x2, x3);)
#define dE_printf4(x1,x2,x3,x4) D_E(fprintf(stddbg, x1, x2, x3,x4);)

#define d0_printf1(x)        D_0(fprintf(stddbg, x);)
#define d0_printf2(x1,x2)    D_0(fprintf(stddbg, x1, x2);)
#define d0_printf3(x1,x2,x3) D_0(fprintf(stddbg, x1, x2, x3);)
#define d0_printf4(x1,x2,x3,x4) D_0(fprintf(stddbg, x1, x2, x3,x4);)
#define d0_printf5(x1,x2,x3,x4,x5) D_0(fprintf(stddbg, x1, x2, x3,x4,x5);)
#define d0_printf6(x1,x2,x3,x4,x5,x6) D_0(fprintf(stddbg, x1, x2, x3,x4,x5,x6);)
#define d0_printf7(x1,x2,x3,x4,x5,x6,x7) D_0(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7);)
#define d0_printf8(x1,x2,x3,x4,x5,x6,x7,x8) D_0(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8);)
#define d0_printf9(x1,x2,x3,x4,x5,x6,x7,x8,x9) D_0(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8,x9);)

#define d1_printf1(x)        D_1(fprintf(stddbg, x);)
#define d1_printf2(x1,x2)    D_1(fprintf(stddbg, x1, x2);)
#define d1_printf3(x1,x2,x3) D_1(fprintf(stddbg, x1, x2, x3);)
#define d1_printf4(x1,x2,x3,x4) D_1(fprintf(stddbg, x1, x2, x3,x4);)
#define d1_printf5(x1,x2,x3,x4,x5) D_1(fprintf(stddbg, x1, x2, x3,x4,x5);)
#define d1_printf6(x1,x2,x3,x4,x5,x6) D_1(fprintf(stddbg, x1, x2, x3,x4,x5,x6);)
#define d1_printf7(x1,x2,x3,x4,x5,x6,x7) D_1(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7);)
#define d1_printf8(x1,x2,x3,x4,x5,x6,x7,x8) D_1(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8);)
#define d1_printf9(x1,x2,x3,x4,x5,x6,x7,x8,x9) D_1(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8,x9);)

#define d2_printf1(x)        D_2(fprintf(stddbg, x);)
#define d2_printf2(x1,x2)    D_2(fprintf(stddbg, x1, x2);)
#define d2_printf3(x1,x2,x3) D_2(fprintf(stddbg, x1, x2, x3);)
#define d2_printf4(x1,x2,x3,x4) D_2(fprintf(stddbg, x1, x2, x3,x4);)
#define d2_printf5(x1,x2,x3,x4,x5) D_2(fprintf(stddbg, x1, x2, x3,x4,x5);)
#define d2_printf6(x1,x2,x3,x4,x5,x6) D_2(fprintf(stddbg, x1, x2, x3,x4,x5,x6);)
#define d2_printf7(x1,x2,x3,x4,x5,x6,x7) D_2(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7);)
#define d2_printf8(x1,x2,x3,x4,x5,x6,x7,x8) D_2(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8);)
#define d2_printf9(x1,x2,x3,x4,x5,x6,x7,x8,x9) D_2(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8,x9);)

#define d2e_printf1(x)        D_2E(fprintf(stddbg, x);)
#define d2e_printf2(x1,x2)    D_2E(fprintf(stddbg, x1, x2);)
#define d2e_printf3(x1,x2,x3) D_2E(fprintf(stddbg, x1, x2, x3);)
#define d2e_printf4(x1,x2,x3,x4) D_2E(fprintf(stddbg, x1, x2, x3,x4);)
#define d2e_printf5(x1,x2,x3,x4,x5) D_2E(fprintf(stddbg, x1, x2, x3,x4,x5);)

#define dm2_printf2(x1,x2)    DM_2(fprintf(stddbg, x1, x2);)

#define d3_printf1(x)        D_3(fprintf(stddbg, x);)
#define d3_printf2(x1,x2)    D_3(fprintf(stddbg, x1, x2);)
#define d3_printf3(x1,x2,x3) D_3(fprintf(stddbg, x1, x2, x3);)
#define d3_printf4(x1,x2,x3,x4) D_3(fprintf(stddbg, x1, x2, x3,x4);)
#define d3_printf5(x1,x2,x3,x4,x5) D_3(fprintf(stddbg, x1, x2, x3,x4,x5);)
#define d3_printf6(x1,x2,x3,x4,x5,x6) D_3(fprintf(stddbg, x1, x2, x3,x4,x5,x6);)
#define d3_printf7(x1,x2,x3,x4,x5,x6,x7) D_3(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7);)
#define d3_printf8(x1,x2,x3,x4,x5,x6,x7,x8) D_3(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8);)
#define d3_printf9(x1,x2,x3,x4,x5,x6,x7,x8,x9) D_3(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8,x9);)

#define d3e_printf1(x)        D_3E(fprintf(stddbg, x);)
#define d3e_printf2(x1,x2)    D_3E(fprintf(stddbg, x1, x2);)
#define d3e_printf3(x1,x2,x3) D_3E(fprintf(stddbg, x1, x2, x3);)
#define d3e_printf4(x1,x2,x3,x4) D_3E(fprintf(stddbg, x1, x2, x3,x4);)
#define d3e_printf5(x1,x2,x3,x4,x5) D_3E(fprintf(stddbg, x1, x2, x3,x4,x5);)

#define d4_printf1(x)        D_4(fprintf(stddbg, x);)
#define d4_printf2(x1,x2)    D_4(fprintf(stddbg, x1, x2);)
#define d4_printf3(x1,x2,x3) D_4(fprintf(stddbg, x1, x2, x3);)
#define d4_printf4(x1,x2,x3,x4) D_4(fprintf(stddbg, x1, x2, x3,x4);)
#define d4_printf5(x1,x2,x3,x4,x5) D_4(fprintf(stddbg, x1, x2, x3,x4,x5);)
#define d4_printf6(x1,x2,x3,x4,x5,x6) D_4(fprintf(stddbg, x1, x2, x3,x4,x5,x6);)
#define d4_printf7(x1,x2,x3,x4,x5,x6,x7) D_4(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7);)
#define d4_printf8(x1,x2,x3,x4,x5,x6,x7,x8) D_4(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8);)
#define d4_printf9(x1,x2,x3,x4,x5,x6,x7,x8,x9) D_4(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8,x9);)

#define d5_printf1(x)        D_5(fprintf(stddbg, x);)
#define d5_printf2(x1,x2)    D_5(fprintf(stddbg, x1, x2);)
#define d5_printf3(x1,x2,x3) D_5(fprintf(stddbg, x1, x2, x3);)
#define d5_printf4(x1,x2,x3,x4) D_5(fprintf(stddbg, x1, x2, x3,x4);)
#define d5_printf5(x1,x2,x3,x4,x5) D_5(fprintf(stddbg, x1, x2, x3,x4,x5);)
#define d5_printf6(x1,x2,x3,x4,x5,x6) D_5(fprintf(stddbg, x1, x2, x3,x4,x5,x6);)
#define d5_printf7(x1,x2,x3,x4,x5,x6,x7) D_5(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7);)
#define d5_printf8(x1,x2,x3,x4,x5,x6,x7,x8) D_5(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8);)
#define d5_printf9(x1,x2,x3,x4,x5,x6,x7,x8,x9) D_5(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8,x9);)

#define d6_printf1(x)        D_6(fprintf(stddbg, x);)
#define d6_printf2(x1,x2)    D_6(fprintf(stddbg, x1, x2);)
#define d6_printf3(x1,x2,x3) D_6(fprintf(stddbg, x1, x2, x3);)
#define d6_printf4(x1,x2,x3,x4) D_6(fprintf(stddbg, x1, x2, x3,x4);)
#define d6_printf5(x1,x2,x3,x4,x5) D_6(fprintf(stddbg, x1, x2, x3,x4,x5);)
#define d6_printf6(x1,x2,x3,x4,x5,x6) D_6(fprintf(stddbg, x1, x2, x3,x4,x5,x6);)
#define d6_printf7(x1,x2,x3,x4,x5,x6,x7) D_6(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7);)
#define d6_printf8(x1,x2,x3,x4,x5,x6,x7,x8) D_6(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8);)
#define d6_printf9(x1,x2,x3,x4,x5,x6,x7,x8,x9) D_6(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8,x9);)

#define d7_printf1(x)        D_7(fprintf(stddbg, x);)
#define d7_printf2(x1,x2)    D_7(fprintf(stddbg, x1, x2);)
#define d7_printf3(x1,x2,x3) D_7(fprintf(stddbg, x1, x2, x3);)
#define d7_printf4(x1,x2,x3,x4) D_7(fprintf(stddbg, x1, x2, x3,x4);)
#define d7_printf5(x1,x2,x3,x4,x5) D_7(fprintf(stddbg, x1, x2, x3,x4,x5);)
#define d7_printf6(x1,x2,x3,x4,x5,x6) D_7(fprintf(stddbg, x1, x2, x3,x4,x5,x6);)
#define d7_printf7(x1,x2,x3,x4,x5,x6,x7) D_7(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7);)
#define d7_printf8(x1,x2,x3,x4,x5,x6,x7,x8) D_7(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8);)
#define d7_printf9(x1,x2,x3,x4,x5,x6,x7,x8,x9) D_7(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8,x9);)

#define d8_printf1(x)        D_8(fprintf(stddbg, x);)
#define d8_printf2(x1,x2)    D_8(fprintf(stddbg, x1, x2);)
#define d8_printf3(x1,x2,x3) D_8(fprintf(stddbg, x1, x2, x3);)
#define d8_printf4(x1,x2,x3,x4) D_8(fprintf(stddbg, x1, x2, x3,x4);)
#define d8_printf5(x1,x2,x3,x4,x5) D_8(fprintf(stddbg, x1, x2, x3,x4,x5);)
#define d8_printf6(x1,x2,x3,x4,x5,x6) D_8(fprintf(stddbg, x1, x2, x3,x4,x5,x6);)
#define d8_printf7(x1,x2,x3,x4,x5,x6,x7) D_8(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7);)
#define d8_printf8(x1,x2,x3,x4,x5,x6,x7,x8) D_8(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8);)
#define d8_printf9(x1,x2,x3,x4,x5,x6,x7,x8,x9) D_8(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8,x9);)

#define d9_printf1(x)        D_9(fprintf(stddbg, x);)
#define d9_printf2(x1,x2)    D_9(fprintf(stddbg, x1, x2);)
#define d9_printf3(x1,x2,x3) D_9(fprintf(stddbg, x1, x2, x3);)
#define d9_printf4(x1,x2,x3,x4) D_9(fprintf(stddbg, x1, x2, x3,x4);)
#define d9_printf5(x1,x2,x3,x4,x5) D_9(fprintf(stddbg, x1, x2, x3,x4,x5);)
#define d9_printf6(x1,x2,x3,x4,x5,x6) D_9(fprintf(stddbg, x1, x2, x3,x4,x5,x6);)
#define d9_printf7(x1,x2,x3,x4,x5,x6,x7) D_9(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7);)
#define d9_printf8(x1,x2,x3,x4,x5,x6,x7,x8) D_9(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8);)
#define d9_printf9(x1,x2,x3,x4,x5,x6,x7,x8,x9) D_9(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8,x9);)

#define d10_printf1(x)        D_10(fprintf(stddbg, x);)
#define d10_printf2(x1,x2)    D_10(fprintf(stddbg, x1, x2);)
#define d10_printf3(x1,x2,x3) D_10(fprintf(stddbg, x1, x2, x3);)
#define d10_printf4(x1,x2,x3,x4) D_10(fprintf(stddbg, x1, x2, x3,x4);)
#define d10_printf5(x1,x2,x3,x4,x5) D_10(fprintf(stddbg, x1, x2, x3,x4,x5);)
#define d10_printf6(x1,x2,x3,x4,x5,x6) D_10(fprintf(stddbg, x1, x2, x3,x4,x5,x6);)
#define d10_printf7(x1,x2,x3,x4,x5,x6,x7) D_10(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7);)
#define d10_printf8(x1,x2,x3,x4,x5,x6,x7,x8) D_10(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8);)
#define d10_printf9(x1,x2,x3,x4,x5,x6,x7,x8,x9) D_10(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8,x9);)

#endif
