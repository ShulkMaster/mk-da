/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/gx.h>
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

void GXPokeBlendMode(GXBlendMode type, GXBlendFactor src_factor, GXBlendFactor dst_factor,
                     GXLogicOp op) {
  u32 reg;

  reg = GX_GET_PE_REG(1);
  SET_REG_FIELD(reg, 1, 0, (type == GX_BM_BLEND) || (type == GX_BM_SUBTRACT));
  SET_REG_FIELD(reg, 1, 11, (type == GX_BM_SUBTRACT));
  SET_REG_FIELD(reg, 1, 1, (type == GX_BM_LOGIC));
  SET_REG_FIELD(reg, 4, 12, op);
  SET_REG_FIELD(reg, 3, 8, src_factor);
  SET_REG_FIELD(reg, 3, 5, dst_factor);
  SET_REG_FIELD(reg, 8, 24, 0x41);
  GX_SET_PE_REG(1, reg);
}

void GXPokeColorUpdate(GXBool update_enable) {
  u32 reg;

  reg = GX_GET_PE_REG(1);
  SET_REG_FIELD(reg, 1, 3, update_enable);
  GX_SET_PE_REG(1, reg);
}

void GXPokeDstAlpha(GXBool enable, u8 alpha) {
  u32 reg = 0;

  SET_REG_FIELD(reg, 8, 0, alpha);
  SET_REG_FIELD(reg, 1, 8, enable);
  GX_SET_PE_REG(2, reg);
}

void GXPokeDither(GXBool dither) {
  u32 reg;

  reg = GX_GET_PE_REG(1);
  SET_REG_FIELD(reg, 1, 2, dither);
  GX_SET_PE_REG(1, reg);
}

void GXPokeZMode(GXBool compare_enable, GXCompare func, GXBool update_enable) {
  u32 reg = 0;

  SET_REG_FIELD(reg, 1, 0, compare_enable);
  SET_REG_FIELD(reg, 3, 1, func);
  SET_REG_FIELD(reg, 1, 4, update_enable);
  GX_SET_PE_REG(0, reg);
}
