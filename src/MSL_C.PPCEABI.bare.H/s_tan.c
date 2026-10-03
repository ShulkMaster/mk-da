/* @(#)s_tan.c 1.2 95/01/04 */
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
double tan(double x)
#else
double tan(x)
double x;
#endif
{
  double y[2], z = 0.0;
  _INT32 n, ix;

  ix = __HI(x);
  ix &= 0x7fffffff;
  if (ix <= 0x3fe921fb)
    return __kernel_tan(x, z, 1);
  else if (ix >= 0x7ff00000)
    return x - x;
  else {
    n = __ieee754_rem_pio2(x, y);
    return __kernel_tan(y[0], y[1], 1 - ((n & 1) << 1));
  }
}
