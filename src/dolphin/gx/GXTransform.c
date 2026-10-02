/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/gx.h>
#include <dolphin/gx/GXPriv.h>
#include <dolphin/asm_sequences.inc>

inline void __GXSetProjection(void) {
  u32 reg = 0x00061020;

  GX_WRITE_U8(0x10);
  GX_WRITE_U32(reg);
  GX_WRITE_F32(__GXData->projMtx[0]);
  GX_WRITE_F32(__GXData->projMtx[1]);
  GX_WRITE_F32(__GXData->projMtx[2]);
  GX_WRITE_F32(__GXData->projMtx[3]);
  GX_WRITE_F32(__GXData->projMtx[4]);
  GX_WRITE_F32(__GXData->projMtx[5]);
  GX_WRITE_U32(__GXData->projType);
}

void GXSetProjection(const Mtx44 proj, GXProjectionType type) {
  __GXData->projType = type;

  __GXData->projMtx[0] = proj[0][0];
  __GXData->projMtx[2] = proj[1][1];
  __GXData->projMtx[4] = proj[2][2];
  __GXData->projMtx[5] = proj[2][3];

  if (type == GX_ORTHOGRAPHIC) {
  __GXData->projMtx[1] = proj[0][3];
  __GXData->projMtx[3] = proj[1][3];
  } else {
  __GXData->projMtx[1] = proj[0][2];
  __GXData->projMtx[3] = proj[1][2];
  }

  __GXSetProjection();

  __GXData->bpSentNot = 1;
}

/* TODO: [near miss] 99.91%; input register selection remains after 15 passes. */
void GXSetProjectionv(const f32 *proj) {
  __GXData->projType = proj[0];
  __GXData->projMtx[0] = proj[1];
  __GXData->projMtx[1] = proj[2];
  __GXData->projMtx[2] = proj[3];
  __GXData->projMtx[3] = proj[4];
  __GXData->projMtx[4] = proj[5];
  __GXData->projMtx[5] = proj[6];

  __GXSetProjection();
  __GXData->bpSentNot = 1;
}

/* TODO: [near miss] 93.33%; prologue load scheduling remains after 15 passes. */
void GXGetProjectionv(f32 *ptr)
{


  ptr[0] = __GXData->projType;
  ptr[1] = __GXData->projMtx[0];
  ptr[2] = __GXData->projMtx[1];
  ptr[3] = __GXData->projMtx[2];
  ptr[4] = __GXData->projMtx[3];
  ptr[5] = __GXData->projMtx[4];
  ptr[6] = __GXData->projMtx[5];
}

asm void WriteMTXPS4x3(const f32 mtx[3][4], volatile f32* dest) { SEQ_WriteMTXPS4x3() }

asm void WriteMTXPS3x3from3x4(const f32 mtx[3][4], volatile f32* dest) {
  SEQ_WriteMTXPS3x3from3x4()
}

asm void WriteMTXPS4x2(const f32 mtx[2][4], volatile f32* dest) { SEQ_WriteMTXPS4x2() }

void GXSetCurrentMtx(u32 id) {

  SET_REG_FIELD(__GXData->matIdxA, 6, 0, id);
  __GXSetMatrixIndex(GX_VA_PNMTXIDX);
}

void GXSetViewportJitter(f32 left, f32 top, f32 wd, f32 ht, f32 nearz, f32 farz, u32 field) {
  f32 sx;
  f32 sy;
  f32 sz;
  f32 ox;
  f32 oy;
  f32 oz;
  f32 zmin;
  f32 zmax;
  u32 reg;

  if (field == 0) {
  top -= 0.5f;
  }

  sx = wd / 2.0f;
  sy = -ht / 2.0f;
  ox = 342.0f + (left + (wd / 2.0f));
  oy = 342.0f + (top + (ht / 2.0f));
  zmin = 1.6777215e7f * nearz;
  zmax = 1.6777215e7f * farz;
  sz = zmax - zmin;
  oz = zmax;

  __GXData->vpLeft = left;
  __GXData->vpTop = top;
  __GXData->vpWd = wd;
  __GXData->vpHt = ht;
  __GXData->vpNearz = nearz;
  __GXData->vpFarz = farz;

  if (__GXData->fgRange != 0) {
  __GXSetRange(nearz, __GXData->fgSideX);
  }

  reg = 0x5101A;
  GX_WRITE_U8(0x10);
  GX_WRITE_U32(reg);
  GX_WRITE_F32(sx);
  GX_WRITE_F32(sy);
  GX_WRITE_F32(sz);
  GX_WRITE_F32(ox);
  GX_WRITE_F32(oy);
  GX_WRITE_F32(oz);

  __GXData->bpSentNot = 1;
}

void GXSetViewport(f32 left, f32 top, f32 width, f32 height, f32 nearZ, f32 farZ) {
  GXSetViewportJitter(left, top, width, height, nearZ, farZ, 1);
}

void GXGetViewportv(f32 *vp)
{


  vp[0] = __GXData->vpLeft;
  vp[1] = __GXData->vpTop;
  vp[2] = __GXData->vpWd;
  vp[3] = __GXData->vpHt;
  vp[4] = __GXData->vpNearz;
  vp[5] = __GXData->vpFarz;
}

void GXSetScissor(u32 left, u32 top, u32 wd, u32 ht) {
  u32 tp;
  u32 lf;
  u32 bm;
  u32 rt;

  tp = top + 342;
  lf = left + 342;
  bm = tp + ht - 1;
  rt = lf + wd - 1;

  SET_REG_FIELD(__GXData->suScis0, 11, 0, tp);
  SET_REG_FIELD(__GXData->suScis0, 11, 12, lf);
  SET_REG_FIELD(__GXData->suScis1, 11, 0, bm);
  SET_REG_FIELD(__GXData->suScis1, 11, 12, rt);

  GX_WRITE_RAS_REG(__GXData->suScis0);
  GX_WRITE_RAS_REG(__GXData->suScis1);
  __GXData->bpSentNot = 0;
}

void GXSetScissorBoxOffset(s32 x_off, s32 y_off) {
  u32 reg = 0;
  u32 hx;
  u32 hy;

  hx = (u32)(x_off + 342) >> 1;
  hy = (u32)(y_off + 342) >> 1;

  SET_REG_FIELD(reg, 10, 0, hx);
  SET_REG_FIELD(reg, 10, 10, hy);
  SET_REG_FIELD(reg, 8, 24, 0x59);
  GX_WRITE_RAS_REG(reg);
  __GXData->bpSentNot = 0;
}

void GXSetClipMode(GXClipMode mode) {

  GX_WRITE_XF_REG(5, mode);
  __GXData->bpSentNot = 1;
}

void __GXSetMatrixIndex(GXAttr matIdxAttr) {
  if (matIdxAttr < GX_VA_TEX4MTXIDX) {
  GX_WRITE_SOME_REG4(8, 0x30, __GXData->matIdxA, -12);
  GX_WRITE_XF_REG(24, __GXData->matIdxA);
  } else {
  GX_WRITE_SOME_REG4(8, 0x40, __GXData->matIdxB, -12);
  GX_WRITE_XF_REG(25, __GXData->matIdxB);
  }
  __GXData->bpSentNot = 1;
}
