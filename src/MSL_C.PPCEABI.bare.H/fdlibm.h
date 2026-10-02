/* @(#)fdlibm.h 1.2 95/01/04 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
#ifdef __STDC__
#include <errno.h>
#include <math.h>
#endif

#define _IEEE_LIBM

#if __option(little_endian)
#define __HIp(x) *(1 + (_INT32*)x)
#define __LOp(x) *(_INT32*)x
#else
#define __HIp(x) *(_INT32*)x
#define __LOp(x) *(1 + (_INT32*)x)
#endif

#ifdef __STDC__
#define __P(p) p
#else
#define __P(p) ()
#endif

extern int signgam;

#define MAXFLOAT ((float)3.40282346638528860e+38)

enum fdversion { fdlibm_ieee = -1, fdlibm_svid, fdlibm_xopen, fdlibm_posix };

#define _LIB_VERSION_TYPE enum fdversion
#define _LIB_VERSION _fdlib_version

extern _LIB_VERSION_TYPE _LIB_VERSION;

#define _IEEE_ fdlibm_ieee
#define _SVID_ fdlibm_svid
#define _XOPEN_ fdlibm_xopen
#define _POSIX_ fdlibm_posix

struct exception {
  int type;
  char* name;
  double arg1;
  double arg2;
  double retval;
};

#define HUGE MAXFLOAT

#define X_TLOSS 1.41484755040568800000e+16

#define DOMAIN 1
#define SING 2
#define OVERFLOW 3
#define UNDERFLOW 4
#define TLOSS 5
#define PLOSS 6

extern int matherr __P((struct exception*));

extern double significand __P((double));

extern int ilogb __P((double));

#ifdef _REENTRANT
extern double gamma_r __P((double, int*));
extern double lgamma_r __P((double, int*));
#endif

extern double __ieee754_sqrt __P((double));
extern double __ieee754_acos __P((double));
extern double __ieee754_acosh __P((double));
extern double __ieee754_log __P((double));
extern double __ieee754_atanh __P((double));
extern double __ieee754_asin __P((double));
extern double __ieee754_atan2 __P((double, double));
extern double __ieee754_exp __P((double));
extern double __ieee754_cosh __P((double));
extern double __ieee754_fmod __P((double, double));
extern double __ieee754_pow __P((double, double));
extern double __ieee754_lgamma_r __P((double, int*));
extern double __ieee754_gamma_r __P((double, int*));
extern double __ieee754_lgamma __P((double));
extern double __ieee754_gamma __P((double));
extern double __ieee754_log10 __P((double));
extern double __ieee754_sinh __P((double));
extern double __ieee754_hypot __P((double, double));
extern double __ieee754_j0 __P((double));
extern double __ieee754_j1 __P((double));
extern double __ieee754_y0 __P((double));
extern double __ieee754_y1 __P((double));
extern double __ieee754_jn __P((int, double));
extern double __ieee754_yn __P((int, double));
extern double __ieee754_remainder __P((double, double));
extern int __ieee754_rem_pio2 __P((double, double*));

extern double __ieee754_scalb __P((double, int));

extern double __kernel_standard __P((double, double, int));
extern double __kernel_sin __P((double, double, int));
extern double __kernel_cos __P((double, double));
extern double __kernel_tan __P((double, double, int));
extern int __kernel_rem_pio2 __P((double*, double*, int, int, int, const int*));
