/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/gx.h>
#include <dolphin/gx/GXPriv.h>


void GXSetTevIndirect(GXTevStageID tev_stage, GXIndTexStageID ind_stage, GXIndTexFormat format,
                      GXIndTexBiasSel bias_sel, GXIndTexMtxID matrix_sel, GXIndTexWrap wrap_s,
                      GXIndTexWrap wrap_t, GXBool add_prev, GXBool utc_lod,
                      GXIndTexAlphaSel alpha_sel) {
  u32 field;
  u32 reg;
  reg = 0;
  field = ind_stage;
  SET_REG_FIELD(reg, 2, 0, field);
  field = format;
  SET_REG_FIELD(reg, 2, 2, field);
  field = bias_sel;
  SET_REG_FIELD(reg, 3, 4, field);
  field = alpha_sel;
  SET_REG_FIELD(reg, 2, 7, field);
  field = matrix_sel;
  SET_REG_FIELD(reg, 4, 9, field);
  field = wrap_s;
  SET_REG_FIELD(reg, 3, 13, field);
  field = wrap_t;
  SET_REG_FIELD(reg, 3, 16, field);
  field = utc_lod;
  SET_REG_FIELD(reg, 1, 19, field);
  field = add_prev;
  SET_REG_FIELD(reg, 1, 20, field);
  field = tev_stage + 16;
  SET_REG_FIELD(reg, 8, 24, field);
  GX_WRITE_RAS_REG(reg);
  __GXData->bpSentNot = 0;
}

void GXSetIndTexCoordScale(GXIndTexStageID ind_state, GXIndTexScale scale_s,
                           GXIndTexScale scale_t) {

  switch (ind_state) {
  case GX_INDTEXSTAGE0:
    SET_REG_FIELD(__GXData->IndTexScale0, 4, 0, scale_s);
    SET_REG_FIELD(__GXData->IndTexScale0, 4, 4, scale_t);
    SET_REG_FIELD(__GXData->IndTexScale0, 8, 24, 0x25);
    GX_WRITE_RAS_REG(__GXData->IndTexScale0);
    break;
  case GX_INDTEXSTAGE1:
    SET_REG_FIELD(__GXData->IndTexScale0, 4, 8, scale_s);
    SET_REG_FIELD(__GXData->IndTexScale0, 4, 12, scale_t);
    SET_REG_FIELD(__GXData->IndTexScale0, 8, 24, 0x25);
    GX_WRITE_RAS_REG(__GXData->IndTexScale0);
    break;
  case GX_INDTEXSTAGE2:
    SET_REG_FIELD(__GXData->IndTexScale1, 4, 0, scale_s);
    SET_REG_FIELD(__GXData->IndTexScale1, 4, 4, scale_t);
    SET_REG_FIELD(__GXData->IndTexScale1, 8, 24, 0x26);
    GX_WRITE_RAS_REG(__GXData->IndTexScale1);
    break;
  case GX_INDTEXSTAGE3:
    SET_REG_FIELD(__GXData->IndTexScale1, 4, 8, scale_s);
    SET_REG_FIELD(__GXData->IndTexScale1, 4, 12, scale_t);
    SET_REG_FIELD(__GXData->IndTexScale1, 8, 24, 0x26);
    GX_WRITE_RAS_REG(__GXData->IndTexScale1);
    break;
  default:
    break;
  }
  __GXData->bpSentNot = 0;
}

void GXSetNumIndStages(u8 nIndStages) {
  SET_REG_FIELD(__GXData->genMode, 3, 16, nIndStages);
  __GXData->dirtyState |= 6;
}

void GXSetTevDirect(GXTevStageID tev_stage) {
  GXSetTevIndirect(tev_stage, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_NONE, GX_ITM_OFF, GX_ITW_OFF,
                   GX_ITW_OFF, 0U, 0, 0);
}

void __GXUpdateBPMask(void) {
  u32 nIndStages;
  u32 i;
  u32 tmap;
  u32 new_imask;

  new_imask = 0;
  nIndStages = GET_REG_FIELD(__GXData->genMode, 3, 16);
  for (i = 0; i < nIndStages; i++) {
    switch (i) {
    case 0:
      tmap = GET_REG_FIELD(__GXData->iref, 3, 0);
      break;
    case 1:
      tmap = GET_REG_FIELD(__GXData->iref, 3, 6);
      break;
    case 2:
      tmap = GET_REG_FIELD(__GXData->iref, 3, 12);
      break;
    case 3:
      tmap = GET_REG_FIELD(__GXData->iref, 3, 18);
      break;
    }
    new_imask |= 1 << tmap;
  }

  if ((u8)__GXData->bpMask != new_imask) {
    SET_REG_FIELD(__GXData->bpMask, 8, 0, new_imask);
    GX_WRITE_RAS_REG(__GXData->bpMask);
    __GXData->bpSentNot = 0;
  }
}

void __GXFlushTextureState(void) {
  GX_WRITE_RAS_REG(__GXData->bpMask);
  __GXData->bpSentNot = 0;
}
