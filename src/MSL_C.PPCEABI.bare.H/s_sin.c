/* @(#)s_sin.c 1.2 95/01/04 */
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

#include "fdlibm.h"

#ifdef __STDC__
double sin(double x)
#else
double sin(x)
double x;
#endif
{
  double y[2], z = 0.0;
  int n, ix;

  ix = __HI(x);
  ix &= 0x7fffffff;
  if (ix <= 0x3fe921fb)
    return __kernel_sin(x, z, 0);
  else if (ix >= 0x7ff00000)
    return x - x;
  else {
    n = __ieee754_rem_pio2(x, y);
    switch (n & 3) {
    case 0:
      return __kernel_sin(y[0], y[1], 1);
    case 1:
      return __kernel_cos(y[0], y[1]);
    case 2:
      return -__kernel_sin(y[0], y[1], 1);
    default:
      return -__kernel_cos(y[0], y[1]);
    }
  }
}
