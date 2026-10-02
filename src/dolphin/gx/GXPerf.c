/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/gx/GXPriv.h>

void GXClearGPMetric(void) {
  u32 reg;

  reg = 4;
  GX_SET_CP_REG(2, reg);
}
