/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/gx/GXPriv.h>

void GXPokeAlphaMode(GXCompare func, u8 threshold) {
  u32 reg;

  reg = (func << 8) | threshold;
  GX_SET_PE_REG(3, reg);
}

void GXPokeAlphaRead(GXAlphaReadMode mode) {
  u32 reg;

  reg = 0;
  SET_REG_FIELD(reg, 2, 0, mode);
  SET_REG_FIELD(reg, 1, 2, 1);
  GX_SET_PE_REG(4, reg);
}

void GXPokeAlphaUpdate(GXBool update_enable) {
  u32 reg;

  reg = GX_GET_PE_REG(1);
  SET_REG_FIELD(reg, 1, 4, update_enable);
  GX_SET_PE_REG(1, reg);
}
