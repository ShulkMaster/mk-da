#include <dolphin/gx.h>
#include <dolphin/vi.h>
#include <renderware/project_driver.h>
#include <renderware/project_error.h>

extern void C_MTXOrtho(Mtx44 unk00, f32 unk04, f32 unk08, f32 unk0C,
    f32 unk10, f32 unk14, f32 unk18);
extern void _rwDlTextureRasterFlush(void);

static u32 _rwDlPrimConvTbl[7] = {0, 0xA8, 0xB0, 0x90, 0x98, 0xA0, 0xB8};
static f32 _rwDlProjectionMatrix[7];

void _rw2DRenderPrimitiveInit(void) {
  static f32 posMatrix[3][4] = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, -1.0f, 0.0f}
  };
  Mtx44 unk00;
  const u8* unk04;
  GXClearVtxDesc();
  GXSetVtxDesc(9, 1);
  GXSetVtxAttrFmt(0, 9, 1, 4, 0);
  GXSetVtxDesc(11, 1);
  GXSetVtxAttrFmt(0, 11, 1, 5, 0);
  GXSetNumTevStages(1);
  GXSetNumChans(1);
  GXSetChanCtrl(4, 0, 1, 1, 0, 0, 2);
  GXSetChanCtrl(5, 0, 1, 1, 0, 0, 2);
  if (_RwDlStateCache[0x12] != 0) {
    GXSetVtxDesc(13, 1);
    GXSetVtxAttrFmt(0, 13, 1, 4, 0);
    GXSetTevOp(0, 0);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(0, 1, 4, 0x3C, 0, 0x7D);
    GXSetTevOrder(0, 0, 0, 4);
    _rwDlTextureRasterFlush();
  } else {
    GXSetNumTexGens(0);
    GXSetTevOrder(0, 0xFF, 0xFF, 4);
    GXSetTevOp(0, 4);
  }
  unk04 = _RwDlRenderMode;
  if (unk04[0x18] != 0) {
    GXSetViewportJitter(0.0f, 0.0f, (f32)*(const u16*)(unk04 + 4),
        (f32)*(const u16*)(unk04 + 8), 0.0f, 1.0f, VIGetNextField() ^ 1);
  } else {
    GXSetViewport(0.0f, 0.0f, (f32)*(const u16*)(unk04 + 4),
        (f32)*(const u16*)(unk04 + 8), 0.0f, 1.0f);
  }
  if (_RwDlFSAA == 0) {
    GXSetScissor(0, 0, *(u16*)(_RwDlRenderMode + 4),
        *(u16*)(_RwDlRenderMode + 8));
  } else if (_RwDlFSAATop != 0) {
    GXSetScissor(0, 0, *(u16*)(_RwDlRenderMode + 4), _RwDlHalfHeight + 2);
    GXSetScissorBoxOffset(0, 0);
  } else {
    GXSetScissor(0, _RwDlHalfHeight - 2, *(u16*)(_RwDlRenderMode + 4),
        _RwDlHalfHeight + 2);
    GXSetScissorBoxOffset(0, _RwDlHalfHeight - 2);
  }
  C_MTXOrtho(unk00, 0.0f, (f32)(*(u16*)(_RwDlRenderMode + 8) - 1),
      0.0f, (f32)(*(u16*)(_RwDlRenderMode + 4) - 1), 0.0f, 1.0f);
  GXGetProjectionv(_rwDlProjectionMatrix);
  GXSetProjection(unk00, 1);
  GXLoadPosMtxImm(posMatrix, 0);
}

#include <renderware/project_state.h>

static void _rw2DRenderPrimativeTerm(void) {
  u8* unk00 = *(u8**)(*(u8**)RwEngineInstance + 0x60);
  if (unk00 != *(u8**)unk00) {
    if (_RwDlFSAA == 0) {
      if (*(_RwDlRenderMode + 0x18) != 0) {
        GXSetViewportJitter((f32)*(s16*)(unk00 + 0x1C),
            (f32)*(s16*)(unk00 + 0x1E), (f32)*(s32*)(unk00 + 0xC),
            (f32)*(s32*)(unk00 + 0x10), 0.0f, 1.0f, VIGetNextField() ^ 1);
      } else {
        GXSetViewport((f32)*(s16*)(unk00 + 0x1C),
            (f32)*(s16*)(unk00 + 0x1E), (f32)*(s32*)(unk00 + 0xC),
            (f32)*(s32*)(unk00 + 0x10), 0.0f, 1.0f);
      }
      GXSetScissor(*(s16*)(unk00 + 0x1C), *(s16*)(unk00 + 0x1E),
          *(s32*)(unk00 + 0xC), *(s32*)(unk00 + 0x10));
    } else {
      if (*(_RwDlRenderMode + 0x18) != 0) {
        GXSetViewportJitter((f32)*(s16*)(unk00 + 0x1C),
            (f32)*(s16*)(unk00 + 0x1E), (f32)*(s32*)(unk00 + 0xC),
            (f32)*(s32*)(unk00 + 0x10), 0.0f, 1.0f, VIGetNextField() ^ 1);
      } else {
        GXSetViewport((f32)*(s16*)(unk00 + 0x1C),
            (f32)(*(s16*)(unk00 + 0x1E) * 2), (f32)*(s32*)(unk00 + 0xC),
            (f32)(*(s32*)(unk00 + 0x10) * 2), 0.0f, 1.0f);
      }
      if (_RwDlFSAATop != 0) {
        s32 unk04 = *(s16*)(unk00 + 0x1E);
        s32 unk08 = *(s32*)(unk00 + 0x10);
        s32 unk20 = (unk04 + unk08) * 2;
        s32 unk0C = _RwDlHalfHeight + 2;
        if (unk20 <= unk0C) {
          GXSetScissor(*(s16*)(unk00 + 0x1C), unk04 * 2,
              *(u16*)(_RwDlRenderMode + 4), unk08 * 2);
        } else if (unk04 * 2 > unk0C) {
          GXSetScissor(0, 0, *(u16*)(_RwDlRenderMode + 4), unk0C);
        } else {
          GXSetScissor(*(s16*)(unk00 + 0x1C), unk04 * 2,
              *(u16*)(_RwDlRenderMode + 4), unk0C);
        }
        GXSetScissorBoxOffset(0, 0);
      } else {
        s32 unk10 = *(s16*)(unk00 + 0x1E);
        s32 unk24 = unk10 * 2;
        s32 unk14 = _RwDlHalfHeight - 2;
        if (unk24 >= unk14) {
          GXSetScissor(*(s16*)(unk00 + 0x1C), unk24,
              *(u16*)(_RwDlRenderMode + 4), *(s32*)(unk00 + 0x10) * 2);
        } else {
          s32 unk18 = *(s32*)(unk00 + 0x10);
          s32 unk28 = (unk10 + unk18) * 2;
          s32 unk1C = _RwDlHalfHeight + 2;
          if (unk28 < unk1C) {
            GXSetScissor(0, unk14, *(u16*)(_RwDlRenderMode + 4), unk1C);
          } else {
            GXSetScissor(*(s16*)(unk00 + 0x1C), unk14,
                *(u16*)(_RwDlRenderMode + 4), unk18 * 2);
          }
        }
        GXSetScissorBoxOffset(0, _RwDlHalfHeight - 2);
      }
    }
  }
  GXSetProjectionv(_rwDlProjectionMatrix);
}

static inline void projectDlPosition(f32 unk00, f32 unk04, f32 unk08) {
  *(volatile f32*)0xCC008000 = unk00;
  *(volatile f32*)0xCC008000 = unk04;
  *(volatile f32*)0xCC008000 = unk08;
}

static inline void projectDlColor(u8 unk00, u8 unk04, u8 unk08, u8 unk0C) {
  *(volatile u8*)0xCC008000 = unk00;
  *(volatile u8*)0xCC008000 = unk04;
  *(volatile u8*)0xCC008000 = unk08;
  *(volatile u8*)0xCC008000 = unk0C;
}

static inline void projectDlTexCoord(f32 unk00, f32 unk04) {
  *(volatile f32*)0xCC008000 = unk00;
  *(volatile f32*)0xCC008000 = unk04;
}

s32 _rwDlIm2DRenderTriangle(const u8* unk00, u32 unk04, u32 unk08, u32 unk0C, u32 unk10) {
  const u8* unk14 = unk00 + unk08 * 0x1C;
  const u8* unk18 = unk00 + unk0C * 0x1C;
  const u8* unk1C = unk00 + unk10 * 0x1C;
  _rw2DRenderPrimitiveInit();
  GXBegin(GX_TRIANGLES, GX_VTXFMT0, 3);
  if (_RwDlStateCache[0x12] != 0) {
    projectDlPosition(*(const f32*)unk14, *(const f32*)(unk14 + 4),
        *(const f32*)(unk14 + 8));
    projectDlColor(unk14[0xC], unk14[0xD], unk14[0xE], unk14[0xF]);
    projectDlTexCoord(*(const f32*)(unk14 + 0x10), *(const f32*)(unk14 + 0x14));
    projectDlPosition(*(const f32*)unk18, *(const f32*)(unk18 + 4),
        *(const f32*)(unk18 + 8));
    projectDlColor(unk18[0xC], unk18[0xD], unk18[0xE], unk18[0xF]);
    projectDlTexCoord(*(const f32*)(unk18 + 0x10), *(const f32*)(unk18 + 0x14));
    projectDlPosition(*(const f32*)unk1C, *(const f32*)(unk1C + 4),
        *(const f32*)(unk1C + 8));
    projectDlColor(unk1C[0xC], unk1C[0xD], unk1C[0xE], unk1C[0xF]);
    projectDlTexCoord(*(const f32*)(unk1C + 0x10), *(const f32*)(unk1C + 0x14));
  } else {
    projectDlPosition(*(const f32*)unk14, *(const f32*)(unk14 + 4),
        *(const f32*)(unk14 + 8));
    projectDlColor(unk14[0xC], unk14[0xD], unk14[0xE], unk14[0xF]);
    projectDlPosition(*(const f32*)unk18, *(const f32*)(unk18 + 4),
        *(const f32*)(unk18 + 8));
    projectDlColor(unk18[0xC], unk18[0xD], unk18[0xE], unk18[0xF]);
    projectDlPosition(*(const f32*)unk1C, *(const f32*)(unk1C + 4),
        *(const f32*)(unk1C + 8));
    projectDlColor(unk1C[0xC], unk1C[0xD], unk1C[0xE], unk1C[0xF]);
  }
  _rw2DRenderPrimativeTerm();
  return 1;
}

s32 _rwDlIm2DRenderLine(const u8* arg0, u32 arg1, u32 arg2, u32 arg3) {
  const u8* unk00 = arg0 + arg2 * 0x1C;
  const u8* unk04 = arg0 + arg3 * 0x1C;
  _rw2DRenderPrimitiveInit();
  GXBegin(GX_LINES, GX_VTXFMT0, 2);
  if (_RwDlStateCache[0x12] != 0) {
    projectDlPosition(*(const f32*)unk00, *(const f32*)(unk00 + 4),
        *(const f32*)(unk00 + 8));
    projectDlColor(unk00[0xC], unk00[0xD], unk00[0xE], unk00[0xF]);
    projectDlTexCoord(*(const f32*)(unk00 + 0x10), *(const f32*)(unk00 + 0x14));
    projectDlPosition(*(const f32*)unk04, *(const f32*)(unk04 + 4),
        *(const f32*)(unk04 + 8));
    projectDlColor(unk04[0xC], unk04[0xD], unk04[0xE], unk04[0xF]);
    projectDlTexCoord(*(const f32*)(unk04 + 0x10), *(const f32*)(unk04 + 0x14));
  } else {
    projectDlPosition(*(const f32*)unk00, *(const f32*)(unk00 + 4),
        *(const f32*)(unk00 + 8));
    projectDlColor(unk00[0xC], unk00[0xD], unk00[0xE], unk00[0xF]);
    projectDlPosition(*(const f32*)unk04, *(const f32*)(unk04 + 4),
        *(const f32*)(unk04 + 8));
    projectDlColor(unk04[0xC], unk04[0xD], unk04[0xE], unk04[0xF]);
  }
  _rw2DRenderPrimativeTerm();
  return 1;
}

s32 _rwDlIm2DRenderPrimitive(s32 unk00, const u8* unk04, s32 unk08) {
  _rw2DRenderPrimitiveInit();
  GXBegin((GXPrimitive)_rwDlPrimConvTbl[unk00], GX_VTXFMT0, (u16)unk08);
  switch (unk00) {
  case 2:
  case 4:
  case 5:
    if (_RwDlStateCache[0x12] != 0) {
      while (unk08--) {
        projectDlPosition(*(const f32*)unk04, *(const f32*)(unk04 + 0x4),
            *(const f32*)(unk04 + 0x8));
        projectDlColor(unk04[0xC], unk04[0xD],
            unk04[0xE], unk04[0xF]);
        projectDlTexCoord(*(const f32*)(unk04 + 0x10), *(const f32*)(unk04 + 0x14));
        unk04 += 0x1C;
      }
    } else {
      while (unk08--) {
        projectDlPosition(*(const f32*)unk04, *(const f32*)(unk04 + 0x4),
            *(const f32*)(unk04 + 0x8));
        projectDlColor(unk04[0xC], unk04[0xD],
            unk04[0xE], unk04[0xF]);
        unk04 += 0x1C;
      }
    }
    break;
  case 1: {
    s32 unk0C = unk08 >> 1;
    if (_RwDlStateCache[0x12] != 0) {
      while (unk0C--) {
        projectDlPosition(*(const f32*)unk04, *(const f32*)(unk04 + 0x4),
            *(const f32*)(unk04 + 0x8));
        projectDlColor(unk04[0xC], unk04[0xD],
            unk04[0xE], unk04[0xF]);
        projectDlTexCoord(*(const f32*)(unk04 + 0x10), *(const f32*)(unk04 + 0x14));
        projectDlPosition(*(const f32*)(unk04 + 0x1C), *(const f32*)(unk04 + 0x20),
            *(const f32*)(unk04 + 0x24));
        projectDlColor(unk04[0x28], unk04[0x29],
            unk04[0x2A], unk04[0x2B]);
        projectDlTexCoord(*(const f32*)(unk04 + 0x2C), *(const f32*)(unk04 + 0x30));
        unk04 += 0x38;
      }
    } else {
      while (unk0C--) {
        projectDlPosition(*(const f32*)unk04, *(const f32*)(unk04 + 0x4),
            *(const f32*)(unk04 + 0x8));
        projectDlColor(unk04[0xC], unk04[0xD],
            unk04[0xE], unk04[0xF]);
        projectDlPosition(*(const f32*)(unk04 + 0x1C), *(const f32*)(unk04 + 0x20),
            *(const f32*)(unk04 + 0x24));
        projectDlColor(unk04[0x28], unk04[0x29],
            unk04[0x2A], unk04[0x2B]);
        unk04 += 0x38;
      }
    }
    break;
  }
  case 3: {
    s32 unk10 = unk08 / 3;
    if (_RwDlStateCache[0x12] != 0) {
      while (unk10--) {
        projectDlPosition(*(const f32*)unk04, *(const f32*)(unk04 + 0x4),
            *(const f32*)(unk04 + 0x8));
        projectDlColor(unk04[0xC], unk04[0xD],
            unk04[0xE], unk04[0xF]);
        projectDlTexCoord(*(const f32*)(unk04 + 0x10), *(const f32*)(unk04 + 0x14));
        projectDlPosition(*(const f32*)(unk04 + 0x1C), *(const f32*)(unk04 + 0x20),
            *(const f32*)(unk04 + 0x24));
        projectDlColor(unk04[0x28], unk04[0x29],
            unk04[0x2A], unk04[0x2B]);
        projectDlTexCoord(*(const f32*)(unk04 + 0x2C), *(const f32*)(unk04 + 0x30));
        projectDlPosition(*(const f32*)(unk04 + 0x38), *(const f32*)(unk04 + 0x3C),
            *(const f32*)(unk04 + 0x40));
        projectDlColor(unk04[0x44], unk04[0x45],
            unk04[0x46], unk04[0x47]);
        projectDlTexCoord(*(const f32*)(unk04 + 0x48), *(const f32*)(unk04 + 0x4C));
        unk04 += 0x54;
      }
    } else {
      while (unk10--) {
        projectDlPosition(*(const f32*)unk04, *(const f32*)(unk04 + 0x4),
            *(const f32*)(unk04 + 0x8));
        projectDlColor(unk04[0xC], unk04[0xD],
            unk04[0xE], unk04[0xF]);
        projectDlPosition(*(const f32*)(unk04 + 0x1C), *(const f32*)(unk04 + 0x20),
            *(const f32*)(unk04 + 0x24));
        projectDlColor(unk04[0x28], unk04[0x29],
            unk04[0x2A], unk04[0x2B]);
        projectDlPosition(*(const f32*)(unk04 + 0x38), *(const f32*)(unk04 + 0x3C),
            *(const f32*)(unk04 + 0x40));
        projectDlColor(unk04[0x44], unk04[0x45],
            unk04[0x46], unk04[0x47]);
        unk04 += 0x54;
      }
    }
    break;
  }
  default: {
    BaerrState unk14;
    unk14.unk00 = 1;
    unk14.unk04 = _rwerror(0x25);
    RwErrorSet(&unk14);
    break;
  }
  }
  _rw2DRenderPrimativeTerm();
  return 1;
}

#include <renderware/project_error.h>

s32 _rwDlIm2DRenderIndexedPrimitive(s32 mode, const u8* vertices,
    u32 unk08, const u16* indices, s32 count) {
  _rw2DRenderPrimitiveInit();
  GXBegin(_rwDlPrimConvTbl[mode], GX_VTXFMT0, (u16)count);
  switch (mode) {
    case 2:
    case 4:
    case 5:
      if (_RwDlStateCache[0x12] != 0) {
        while (count--) {
          const u8* vertex = vertices + indices[0] * 0x1C;
          projectDlPosition(*(const f32*)vertex, *(const f32*)(vertex + 4),
              *(const f32*)(vertex + 8));
          projectDlColor(vertex[0xC], vertex[0xD], vertex[0xE], vertex[0xF]);
          projectDlTexCoord(*(const f32*)(vertex + 0x10),
              *(const f32*)(vertex + 0x14));
          indices += 1;
        }
      } else {
        while (count--) {
          const u8* vertex = vertices + indices[0] * 0x1C;
          projectDlPosition(*(const f32*)vertex, *(const f32*)(vertex + 4),
              *(const f32*)(vertex + 8));
          projectDlColor(vertex[0xC], vertex[0xD], vertex[0xE], vertex[0xF]);
          indices += 1;
        }
      }
      break;
    case 1: {
      s32 groups = count >> 1;
      if (_RwDlStateCache[0x12] != 0) {
        while (groups--) {
          const u8* vertex = vertices + indices[0] * 0x1C;
          projectDlPosition(*(const f32*)vertex, *(const f32*)(vertex + 4),
              *(const f32*)(vertex + 8));
          projectDlColor(vertex[0xC], vertex[0xD], vertex[0xE], vertex[0xF]);
          projectDlTexCoord(*(const f32*)(vertex + 0x10),
              *(const f32*)(vertex + 0x14));
          vertex = vertices + indices[1] * 0x1C;
          projectDlPosition(*(const f32*)vertex, *(const f32*)(vertex + 4),
              *(const f32*)(vertex + 8));
          projectDlColor(vertex[0xC], vertex[0xD], vertex[0xE], vertex[0xF]);
          projectDlTexCoord(*(const f32*)(vertex + 0x10),
              *(const f32*)(vertex + 0x14));
          indices += 2;
        }
      } else {
        while (groups--) {
          const u8* vertex = vertices + indices[0] * 0x1C;
          projectDlPosition(*(const f32*)vertex, *(const f32*)(vertex + 4),
              *(const f32*)(vertex + 8));
          projectDlColor(vertex[0xC], vertex[0xD], vertex[0xE], vertex[0xF]);
          vertex = vertices + indices[1] * 0x1C;
          projectDlPosition(*(const f32*)vertex, *(const f32*)(vertex + 4),
              *(const f32*)(vertex + 8));
          projectDlColor(vertex[0xC], vertex[0xD], vertex[0xE], vertex[0xF]);
          indices += 2;
        }
      }
      break;
    }
    case 3: {
      s32 groups = count / 3;
      if (_RwDlStateCache[0x12] != 0) {
        while (groups--) {
          const u8* vertex = vertices + indices[0] * 0x1C;
          projectDlPosition(*(const f32*)vertex, *(const f32*)(vertex + 4),
              *(const f32*)(vertex + 8));
          projectDlColor(vertex[0xC], vertex[0xD], vertex[0xE], vertex[0xF]);
          projectDlTexCoord(*(const f32*)(vertex + 0x10),
              *(const f32*)(vertex + 0x14));
          vertex = vertices + indices[1] * 0x1C;
          projectDlPosition(*(const f32*)vertex, *(const f32*)(vertex + 4),
              *(const f32*)(vertex + 8));
          projectDlColor(vertex[0xC], vertex[0xD], vertex[0xE], vertex[0xF]);
          projectDlTexCoord(*(const f32*)(vertex + 0x10),
              *(const f32*)(vertex + 0x14));
          vertex = vertices + indices[2] * 0x1C;
          projectDlPosition(*(const f32*)vertex, *(const f32*)(vertex + 4),
              *(const f32*)(vertex + 8));
          projectDlColor(vertex[0xC], vertex[0xD], vertex[0xE], vertex[0xF]);
          projectDlTexCoord(*(const f32*)(vertex + 0x10),
              *(const f32*)(vertex + 0x14));
          indices += 3;
        }
      } else {
        while (groups--) {
          const u8* vertex = vertices + indices[0] * 0x1C;
          projectDlPosition(*(const f32*)vertex, *(const f32*)(vertex + 4),
              *(const f32*)(vertex + 8));
          projectDlColor(vertex[0xC], vertex[0xD], vertex[0xE], vertex[0xF]);
          vertex = vertices + indices[1] * 0x1C;
          projectDlPosition(*(const f32*)vertex, *(const f32*)(vertex + 4),
              *(const f32*)(vertex + 8));
          projectDlColor(vertex[0xC], vertex[0xD], vertex[0xE], vertex[0xF]);
          vertex = vertices + indices[2] * 0x1C;
          projectDlPosition(*(const f32*)vertex, *(const f32*)(vertex + 4),
              *(const f32*)(vertex + 8));
          projectDlColor(vertex[0xC], vertex[0xD], vertex[0xE], vertex[0xF]);
          indices += 3;
        }
      }
      break;
    }
    default: {
      BaerrState error;
      error.unk00 = 1;
      error.unk04 = _rwerror(0x25);
      RwErrorSet(&error);
      break;
    }
  }
  _rw2DRenderPrimativeTerm();
  return 1;
}
