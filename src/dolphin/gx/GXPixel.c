/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/gx.h>
#include <dolphin/gx/GXPriv.h>

void GXSetFogRangeAdj(GXBool enable, u16 center, const GXFogAdjTable* table) {
  u32 i;
  u32 range_adj;
  u32 range_c;

  if (enable) {
    for (i = 0; i < 10; i += 2) {
      range_adj = 0;
      SET_REG_FIELD(range_adj, 12, 0, table->r[i]);
      SET_REG_FIELD(range_adj, 12, 12, table->r[i + 1]);
      SET_REG_FIELD(range_adj, 8, 24, (i >> 1) + 0xE9);
      GX_WRITE_RAS_REG(range_adj);
    }
  }
  range_c = 0;
  SET_REG_FIELD(range_c, 10, 0, center + 342);
  SET_REG_FIELD(range_c, 1, 10, enable);
  SET_REG_FIELD(range_c, 8, 24, 0xE8);
  GX_WRITE_RAS_REG(range_c);
  __GXData->bpSentNot = 0;
}

void GXSetBlendMode(GXBlendMode type, GXBlendFactor src_factor,
                    GXBlendFactor dst_factor, GXLogicOp op) {
  SET_REG_FIELD(__GXData->cmode0, 1, 0, (type == GX_BM_BLEND || type == GX_BM_SUBTRACT));
  SET_REG_FIELD(__GXData->cmode0, 1, 11, (type == GX_BM_SUBTRACT));
  SET_REG_FIELD(__GXData->cmode0, 1, 1, (type == GX_BM_LOGIC));
  SET_REG_FIELD(__GXData->cmode0, 4, 12, op);
  SET_REG_FIELD(__GXData->cmode0, 3, 8, src_factor);
  SET_REG_FIELD(__GXData->cmode0, 3, 5, dst_factor);
  SET_REG_FIELD(__GXData->cmode0, 8, 24, 0x41);
  GX_WRITE_RAS_REG(__GXData->cmode0);
  __GXData->bpSentNot = 0;
}

void GXSetColorUpdate(GXBool update_enable) {
  SET_REG_FIELD(__GXData->cmode0, 1, 3, update_enable);
  GX_WRITE_RAS_REG(__GXData->cmode0);

  __GXData->bpSentNot = 0;
}

void GXSetAlphaUpdate(GXBool update_enable) {
  SET_REG_FIELD(__GXData->cmode0, 1, 4, update_enable);
  GX_WRITE_RAS_REG(__GXData->cmode0);

  __GXData->bpSentNot = 0;
}

void GXSetZMode(GXBool compare_enable, GXCompare func, GXBool update_enable) {
  SET_REG_FIELD(__GXData->zmode, 1, 0, compare_enable);
  SET_REG_FIELD(__GXData->zmode, 3, 1, func);
  SET_REG_FIELD(__GXData->zmode, 1, 4, update_enable);
  GX_WRITE_RAS_REG(__GXData->zmode);

  __GXData->bpSentNot = 0;
}

void GXSetZCompLoc(GXBool before_tex) {
  SET_REG_FIELD(__GXData->peCtrl, 1, 6, before_tex);
  GX_WRITE_RAS_REG(__GXData->peCtrl);
  __GXData->bpSentNot = 0;
}

void GXSetPixelFmt(GXPixelFmt pix_fmt, GXZFmt16 z_fmt) {
  u32 oldPeCtrl;
  u8 aa;
  static u32 p2f[8] = {0, 1, 2, 3, 4, 4, 4, 5};

  oldPeCtrl = __GXData->peCtrl;

  SET_REG_FIELD(__GXData->peCtrl, 3, 0, p2f[pix_fmt]);
  SET_REG_FIELD(__GXData->peCtrl, 3, 3, z_fmt);

  if (oldPeCtrl != __GXData->peCtrl) {
    GX_WRITE_RAS_REG(__GXData->peCtrl);
    if (pix_fmt == GX_PF_RGB565_Z16)
      aa = 1;
    else
      aa = 0;
    SET_REG_FIELD(__GXData->genMode, 1, 9, aa);
    __GXData->dirtyState |= 4;
  }

  if (p2f[pix_fmt] == 4) {
    SET_REG_FIELD(__GXData->cmode1, 2, 9, (pix_fmt - 4) & 0x3);
    SET_REG_FIELD(__GXData->cmode1, 8, 24, 0x42);
    GX_WRITE_RAS_REG(__GXData->cmode1);
  }

  __GXData->bpSentNot = 0;
}

void GXSetDither(GXBool dither) {
  SET_REG_FIELD(__GXData->cmode0, 1, 2, dither);
  GX_WRITE_RAS_REG(__GXData->cmode0);

  __GXData->bpSentNot = 0;
}

void GXSetDstAlpha(GXBool enable, u8 alpha) {
  SET_REG_FIELD(__GXData->cmode1, 8, 0, alpha);
  SET_REG_FIELD(__GXData->cmode1, 1, 8, enable);
  GX_WRITE_RAS_REG(__GXData->cmode1);

  __GXData->bpSentNot = 0;
}

void GXSetFieldMask(GXBool odd_mask, GXBool even_mask) {
  u32 reg;

  reg = 0;
  SET_REG_FIELD(reg, 1, 0, even_mask);
  SET_REG_FIELD(reg, 1, 1, odd_mask);
  SET_REG_FIELD(reg, 8, 24, 0x44);
  GX_WRITE_RAS_REG(reg);
  __GXData->bpSentNot = 0;
}

void GXSetFieldMode(GXBool field_mode, GXBool half_aspect_ratio) {
  u32 reg;

  SET_REG_FIELD(__GXData->lpSize, 1, 22, half_aspect_ratio);
  GX_WRITE_RAS_REG(__GXData->lpSize);
  __GXFlushTextureState();
  reg = field_mode | 0x68000000;
  GX_WRITE_RAS_REG(reg);
  __GXFlushTextureState();
}
