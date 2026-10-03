#include <dolphin/gx.h>
#include <renderware/project_driver.h>
#include <renderware/project_raster.h>
#include <renderware/project_state.h>
#include <string.h>

static u32 _RwDlFogConvTable[4] = {0, 2, 4, 5};
static u32 _RwDlAddressConvTable[5] = {0, 1, 2, 0, 0};
static u32 _RwDlFilterModeConvTable[7][2] = {
  {0, 0}, {0, 0}, {1, 1}, {2, 0}, {3, 1}, {4, 0}, {5, 1}
};
static u32 _RwDlBlendConvTable[12] = {0, 0, 1, 2, 3, 4, 5, 6, 7, 2, 3, 0};
static u32 _RwDlCullModeConvTable[4] = {0, 0, 1, 2};
static u32 _RwDlTlutNameConvTable[16] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};
static u32 _RwDlTexMapIDConvTable[8] = {0, 1, 2, 3, 4, 5, 6, 7};


u32 _RwDlStateCache[0x1D];
static u8* _RwDlRasterWhite;

void _rwDlRenderStateOpen(void) {
  GXColor unk00 = {0xFF, 0xFF, 0xFF, 0xFF};
  s32 unk04;
  _RwDlStateCache[6] = 0;
  _RwDlStateCache[7] = 1;
  _RwDlStateCache[8] = -1;
  ((u8*)_RwDlStateCache)[0x24] = 0xFF;
  ((u8*)_RwDlStateCache)[0x25] = 0xFF;
  ((u8*)_RwDlStateCache)[0x26] = 0xFF;
  ((u8*)_RwDlStateCache)[0x27] = 0xFF;
  _RwDlStateCache[0] = 1;
  _RwDlStateCache[1] = 1;
  _RwDlStateCache[2] = 3;
  GXSetZMode(1, 3, 1);
  GXSetZCompLoc(1);
  for (unk04 = 0; unk04 < 8; ++unk04) {
    _RwDlStateCache[unk04 + 0xA] = 0;
  }
  _RwDlStateCache[0x12] = 0;
  _RwDlStateCache[0x13] = 2;
  _RwDlStateCache[0x14] = 1;
  _RwDlStateCache[0x15] = 1;
  _RwDlStateCache[0x16] = 1;
  _RwDlStateCache[0x17] = 5;
  _RwDlStateCache[0x18] = 6;
  _RwDlStateCache[0x19] = 4;
  _RwDlStateCache[0x1A] = 5;
  GXSetBlendMode(1, 4, 5, 0);
  _RwDlStateCache[4] = 2;
  _RwDlStateCache[5] = 1;
  GXSetCullMode(1);
  ((u8*)_RwDlStateCache)[0x70] = 0;
  ((u8*)_RwDlStateCache)[0x71] = 0;
  _RwDlStateCache[0x1B] = 1;
  GXSetAlphaCompare(7, ((u8*)_RwDlStateCache)[0x70], 0, 7,
      ((u8*)_RwDlStateCache)[0x71]);
  GXSetBlendMode(1, 4, 5, 0);
  GXSetChanCtrl(2, 0, 0, 0, 0, 0, 2);
  GXSetChanCtrl(3, 0, 0, 0, 0, 0, 2);
  GXSetChanCtrl(0, 0, 0, 0, 0, 0, 2);
  GXSetChanCtrl(1, 0, 0, 0, 0, 0, 2);
  GXSetChanMatColor(2, unk00);
  GXSetChanMatColor(3, unk00);
  GXSetChanMatColor(0, unk00);
  GXSetChanMatColor(1, unk00);
  GXSetColorUpdate(1);
  GXSetAlphaUpdate(1);
  GXSetCopyClear(unk00, 0xFFFFFF);
  GXSetCurrentMtx(0);
  _RwDlRasterWhite = RwRasterCreate(4, 4, 0x10, 0x204);
  memset(RwRasterLock(_RwDlRasterWhite, 0, 9), 0xFF, 0x20);
  RwRasterUnlock(_RwDlRasterWhite);
}


void _rwDlRenderStateClose(void) {
  RwRasterDestroy(_RwDlRasterWhite);
  _RwDlRasterWhite = 0;
}


s32 _rwDlGetRenderState(s32 unk00, void* unk04) {
  switch (unk00) {
  case 14:
    *(u32*)unk04 = _RwDlStateCache[6];
    return 1;
  case 16:
    *(u32*)unk04 = _RwDlStateCache[7];
    return 1;
  case 15:
    *(u32*)unk04 = _RwDlStateCache[8];
    return 1;
  case 17:
    return 0;
  case 18:
    return 0;
  case 2:
    if (((s32*)_RwDlStateCache)[0x14] == ((s32*)_RwDlStateCache)[0x15]) {
      *(u32*)unk04 = _RwDlStateCache[0x14];
      return 1;
    }
    return 0;
  case 3:
    *(u32*)unk04 = _RwDlStateCache[0x14];
    return 1;
  case 4:
    *(u32*)unk04 = _RwDlStateCache[0x15];
    return 1;
  case 9:
    *(u32*)unk04 = _RwDlStateCache[0x13];
    return 1;
  case 1:
    *(u32*)unk04 = _RwDlStateCache[0x12];
    return 1;
  case 8:
    *(u32*)unk04 = _RwDlStateCache[0];
    return 1;
  case 6:
    *(u32*)unk04 = _RwDlStateCache[1];
    return 1;
  case 10:
    *(u32*)unk04 = _RwDlStateCache[0x17];
    return 1;
  case 11:
    *(u32*)unk04 = _RwDlStateCache[0x18];
    return 1;
  case 7:
    *(u32*)unk04 = 2;
    return 1;
  case 13:
    return 0;
  case 5:
    *(u32*)unk04 = 1;
    return 1;
  case 19:
    return 0;
  case 20:
    *(u32*)unk04 = _RwDlStateCache[4];
    return 1;
  default:
    return 0;
  }
}


static GXTexObj texObj[8];

static GXTexObj* projectTextureSlot(s32 unk00) {
  return texObj + unk00;
}

/* TODO: [near miss] 97.06%; register allocation remains. */
u8* _rwDlRenderStateSetTexture(u8* arg0, s32 arg1) {
  GXTlut unk20;
  u8* unk08;
  static u32 texParamCache[4];
  u32* unk00;
  GXBool unk24;
  _RwDlStateCache[arg1 + 0xA] = (u32)arg0;
  if (arg1 == 0) {
    unk00 = _RwDlStateCache + 0x12;
    _RwDlStateCache[0x16] = 0;
  } else {
    unk00 = texParamCache;
  }
  if (arg0 != 0) {
    unk00[0] = *(u32*)arg0;
    unk00[1] = *(u32*)(arg0 + 0x54);
    unk00[2] = *(u32*)(arg0 + 0x58);
    unk00[3] = *(u32*)(arg0 + 0x5C);
  } else {
    unk00[0] = (u32)_RwDlRasterWhite;
    unk00[1] = 2;
    unk00[2] = 1;
    unk00[3] = 1;
  }
  {
    u8* unk04 = (u8*)unk00[0];
    if (unk04 != 0) {
      u16 unk0C;
      s32 unk10;
      u32 unk14;
      u32 unk18;
      GXTexObj* unk1C;
      unk08 = *(u8**)unk04 + _RwGameCubeRasterExtOffset;
      *(u16*)(unk08 + 0x2C) = _RwDlTokenCurrent;
      unk0C = *(u8*)(unk04 + 0x23) << 8;
      unk10 = unk0C & 0x6000;
      if (unk10 != 0) {
        unk20 = (GXTlut)_RwDlTlutNameConvTable[arg1];
        unk24 = (unk0C >> 15) & 1;
        GXInitTexObjCI(texObj + arg1, *(u8**)(unk08 + 0x1C),
            (u16)*(s32*)(unk04 + 0xC), (u16)*(s32*)(unk04 + 0x10),
            (GXCITexFmt)*(u32*)(unk08 + 0xC),
            (GXTexWrapMode)_RwDlAddressConvTable[unk00[2]],
            (GXTexWrapMode)_RwDlAddressConvTable[unk00[3]],
            unk24, unk20);
        GXLoadTlut((GXTlutObj*)unk08, (GXTlut)unk20);
      } else {
        GXInitTexObj(texObj + arg1, *(u8**)(unk08 + 0x1C),
            (u16)*(s32*)(unk04 + 0xC), (u16)*(s32*)(unk04 + 0x10),
            *(u32*)(unk08 + 0xC),
            (GXTexWrapMode)_RwDlAddressConvTable[unk00[2]],
            (GXTexWrapMode)_RwDlAddressConvTable[unk00[3]],
            (unk0C >> 15) & 1);
      }
      if (unk10 != 0 && ((s32)unk00[1] == 6 || (s32)unk00[1] == 5)) {
        unk14 = _RwDlFilterModeConvTable[4][0];
        unk18 = _RwDlFilterModeConvTable[4][1];
      } else {
        unk14 = _RwDlFilterModeConvTable[unk00[1]][0];
        unk18 = _RwDlFilterModeConvTable[unk00[1]][1];
      }
      unk1C = projectTextureSlot(arg1);
      GXInitTexObjLOD(unk1C, (GXTexFilter)unk14, (GXTexFilter)unk18,
          0.0f, (f32)*(u8*)(unk08 + 0x2E), 0.0f, 1, 1, (GXAnisotropy)0);
      GXLoadTexObj(unk1C, (GXTexMapID)_RwDlTexMapIDConvTable[arg1]);
    }
  }
  return arg0;
}

void _rwDlTextureRasterFlush(void) {
  if ((s32)_RwDlStateCache[0x16] == 1) {
    u8* unk00 = (u8*)_RwDlStateCache[0x12];
    if (unk00 != 0) {
      u8* unk04 = *(u8**)unk00 + _RwGameCubeRasterExtOffset;
      u16 unk08;
      s32 unk0C;
      GXTexFilter unk10;
      GXTexFilter unk14;
      *(u16*)(unk04 + 0x2C) = _RwDlTokenCurrent;
      unk08 = (u32)unk00[0x23] << 8;
      unk0C = unk08 & 0x6000;
      if (unk0C != 0) {
        GXTlut unk18 = _RwDlTlutNameConvTable[0];
        GXBool unk20 = (unk08 >> 15) & 1;
        GXInitTexObjCI(texObj, *(void**)(unk04 + 0x1C),
            (u16)*(s32*)(unk00 + 0xC), (u16)*(s32*)(unk00 + 0x10),
            *(u32*)(unk04 + 0xC),
            _RwDlAddressConvTable[_RwDlStateCache[0x14]],
            _RwDlAddressConvTable[_RwDlStateCache[0x15]],
            unk20, unk18);
        GXLoadTlut((GXTlutObj*)unk04, unk18);
      } else {
        GXInitTexObj(texObj, *(void**)(unk04 + 0x1C),
            (u16)*(s32*)(unk00 + 0xC), (u16)*(s32*)(unk00 + 0x10),
            *(u32*)(unk04 + 0xC),
            _RwDlAddressConvTable[_RwDlStateCache[0x14]],
            _RwDlAddressConvTable[_RwDlStateCache[0x15]],
            (unk08 >> 15) & 1);
      }
      if (unk0C != 0 &&
          ((s32)_RwDlStateCache[0x13] == 6 || (s32)_RwDlStateCache[0x13] == 5)) {
        unk10 = _RwDlFilterModeConvTable[4][0];
        unk14 = _RwDlFilterModeConvTable[4][1];
      } else {
        unk10 = _RwDlFilterModeConvTable[_RwDlStateCache[0x13]][0];
        unk14 = _RwDlFilterModeConvTable[_RwDlStateCache[0x13]][1];
      }
      GXInitTexObjLOD(texObj, unk10, unk14, 0.0f, (f32)unk04[0x2E],
          0.0f, 1, 1, 0);
      GXLoadTexObj(texObj, _RwDlTexMapIDConvTable[0]);
    }
    _RwDlStateCache[0x16] = 0;
    {
      s32 unk1C = 1;
      if ((u8*)_RwDlStateCache[0x12] != 0) {
        u8* unk20 = (u8*)_RwDlStateCache[0x12] + _RwGameCubeRasterExtOffset;
        if (*(u32*)(unk20 + 0x14) & 1) {
          unk1C = 3;
        }
      }
      _rwDlRenderStateSetAlphaComp(unk1C);
    }
  }
}


void _rwDlRenderStateSetAlphaComp(s32 arg0) {
  if ((s32)_RwDlStateCache[0x1B] != arg0) {
    switch (arg0) {
    case 1:
      GXSetAlphaCompare(7, ((u8*)_RwDlStateCache)[0x70], 0, 7,
          ((u8*)_RwDlStateCache)[0x71]);
      GXSetZCompLoc(1);
      break;
    case 2:
      GXSetAlphaCompare(4, ((u8*)_RwDlStateCache)[0x70], 0, 4,
          ((u8*)_RwDlStateCache)[0x71]);
      GXSetZCompLoc(1);
      break;
    case 3:
      GXSetAlphaCompare(4, ((u8*)_RwDlStateCache)[0x70], 0, 4,
          ((u8*)_RwDlStateCache)[0x71]);
      GXSetZCompLoc(0);
      break;
    case 4:
    default:
      break;
    }
    _RwDlStateCache[0x1B] = arg0;
  }
}


static inline s32 projectSetAddress(s32 unk00, u32 unk04) {
  if (unk00 == 4) {
    return 0;
  }
  if (unk00 != (s32)_RwDlStateCache[unk04]) {
    _RwDlStateCache[unk04] = unk00;
    _RwDlStateCache[0x16] = 1;
    _RwDlStateCache[0xA] = 0;
  }
  return 1;
}

static inline s32 projectSetBlendDestination(s32 unk00) {
  if (unk00 != (s32)_RwDlStateCache[0x18]) {
    switch (unk00) {
    default:
      return 0;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8: {
      u32 unk04 = _RwDlBlendConvTable[unk00];
      GXSetBlendMode(1, _RwDlStateCache[0x19], unk04, 0);
      _RwDlStateCache[0x18] = unk00;
      _RwDlStateCache[0x1A] = unk04;
      return 1;
    }
    }
  }
  return 1;
}

static inline s32 projectSetFogState(s32 unk00) {
  if (unk00 != (s32)_RwDlStateCache[7]) {
    if (unk00 != 1) {
      return 0;
    } else {
      u8* unk04 = *(u8**)RwEngineInstance;
      GXSetFog(_RwDlFogConvTable[unk00], *(f32*)(unk04 + 0x88),
          *(f32*)(unk04 + 0x84), *(f32*)(unk04 + 0x80),
          *(f32*)(unk04 + 0x84), *(GXColor*)&_RwDlStateCache[9]);
      _RwDlStateCache[7] = unk00;
    }
  }
  return 1;
}

/* TODO: [breakthrough needed] 95.72%; inline value staging and switch returns remain. */
s32 _rwDlSetRenderState(u32 unk00, void* unk04) {
  s32 unk08 = 0;
  switch (unk00) {
  case 14: {
    u8* unk0C = *(u8**)RwEngineInstance;
    if ((s32)unk04 != 0) {
      if ((s32)_RwDlStateCache[6] == 0) {
        GXSetFog(_RwDlFogConvTable[_RwDlStateCache[7]],
            *(f32*)(unk0C + 0x88), *(f32*)(unk0C + 0x84),
            *(f32*)(unk0C + 0x80), *(f32*)(unk0C + 0x84), *(GXColor*)&_RwDlStateCache[9]);
        _RwDlStateCache[6] = 1;
      }
    } else if ((s32)_RwDlStateCache[6] != 0) {
      GXSetFog(0, *(f32*)(unk0C + 0x88), *(f32*)(unk0C + 0x84),
          *(f32*)(unk0C + 0x80), *(f32*)(unk0C + 0x84), *(GXColor*)&_RwDlStateCache[9]);
      _RwDlStateCache[6] = 0;
    }
    unk08 = 1;
    break;
  }
  case 15:
    if ((u32)unk04 != _RwDlStateCache[8]) {
      u8* unk18 = *(u8**)RwEngineInstance;
      ((u8*)_RwDlStateCache)[0x27] = (u32)unk04 >> 24;
      ((u8*)_RwDlStateCache)[0x24] = (u32)unk04 >> 16;
      ((u8*)_RwDlStateCache)[0x25] = (u32)unk04 >> 8;
      ((u8*)_RwDlStateCache)[0x26] = (u32)unk04;
      GXSetFog(_RwDlFogConvTable[_RwDlStateCache[7]],
          *(f32*)(unk18 + 0x88), *(f32*)(unk18 + 0x84),
          *(f32*)(unk18 + 0x80), *(f32*)(unk18 + 0x84), *(GXColor*)&_RwDlStateCache[9]);
      _RwDlStateCache[8] = (u32)unk04;
    }
    unk08 = 1;
    break;
  case 16:
    unk08 = projectSetFogState((s32)unk04);
    break;
  case 17:
    unk08 = 0;
    break;
  case 18:
    unk08 = 0;
    break;
  case 2: {
    if ((s32)unk04 == 4) {
      unk08 = 0;
    } else {
      if ((s32)unk04 != (s32)_RwDlStateCache[0x14]) {
        _RwDlStateCache[0x14] = (s32)unk04;
        _RwDlStateCache[0x16] = 1;
        _RwDlStateCache[0xA] = 0;
      }
      if ((s32)unk04 != (s32)_RwDlStateCache[0x15]) {
        _RwDlStateCache[0x15] = (s32)unk04;
        _RwDlStateCache[0x16] = 1;
        _RwDlStateCache[0xA] = 0;
      }
      unk08 = 1;
    }
    break;
  }
  case 3:
    unk08 = projectSetAddress((s32)unk04, 0x14);
    break;
  case 4:
    unk08 = projectSetAddress((s32)unk04, 0x15);
    break;
  case 9:
    if ((s32)unk04 != (s32)_RwDlStateCache[0x13]) {
      _RwDlStateCache[0x13] = (u32)unk04;
      _RwDlStateCache[0x16] = 1;
      _RwDlStateCache[0xA] = 0;
    }
    unk08 = 1;
    break;
  case 1:
    if ((u32)unk04 != _RwDlStateCache[0x12]) {
      _RwDlStateCache[0x12] = (u32)unk04;
      _RwDlStateCache[0x16] = 1;
      _RwDlStateCache[0xA] = 0;
    }
    unk08 = 1;
    break;
  case 8:
    if ((s32)unk04 != 0) {
      if ((s32)_RwDlStateCache[0] == 0) {
        GXSetZMode(1, _RwDlStateCache[2], 1);
        _RwDlStateCache[0] = 1;
      }
    } else if ((s32)_RwDlStateCache[0] != 0) {
      GXSetZMode(1, _RwDlStateCache[2], 0);
      _RwDlStateCache[0] = 0;
    }
    unk08 = 1;
    break;
  case 6:
    if ((s32)unk04 != 0) {
      if ((s32)_RwDlStateCache[1] == 0) {
        GXSetZMode(1, 3, (u8)_RwDlStateCache[0]);
        _RwDlStateCache[1] = 1;
        _RwDlStateCache[2] = 3;
      }
    } else if ((s32)_RwDlStateCache[1] != 0) {
      GXSetZMode(1, 7, (u8)_RwDlStateCache[0]);
      _RwDlStateCache[1] = 0;
      _RwDlStateCache[2] = 7;
    }
    unk08 = 1;
    break;
  case 10: {
    if ((s32)unk04 != (s32)_RwDlStateCache[0x17]) {
      switch ((s32)unk04) {
      case 1:
      case 2:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 10: {
        u32 unk3C = _RwDlBlendConvTable[(s32)unk04];
        GXSetBlendMode(1, unk3C, _RwDlStateCache[0x1A], 0);
        _RwDlStateCache[0x17] = (s32)unk04;
        _RwDlStateCache[0x19] = unk3C;
        unk08 = 1;
        break;
      }
      default:
        unk08 = 0;
        break;
      }
    } else {
      unk08 = 1;
    }
    break;
  }
  case 11:
    unk08 = projectSetBlendDestination((s32)unk04);
    break;
  case 7:
    unk08 = (s32)unk04 == 2;
    break;
  case 13:
    unk08 = 0;
    break;
  case 5:
    unk08 = (s32)unk04;
    break;
  case 19:
    unk08 = !unk04;
    break;
  case 20: {
    if ((s32)unk04 != (s32)_RwDlStateCache[4]) {
      u32 unk4C = _RwDlCullModeConvTable[(s32)unk04];
      GXSetCullMode(unk4C);
      _RwDlStateCache[4] = (s32)unk04;
      _RwDlStateCache[5] = unk4C;
    }
    unk08 = 1;
    break;
  }
  }
  return unk08;
}
