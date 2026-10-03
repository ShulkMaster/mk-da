#include <renderware/project_clip.h>
#include <renderware/project_pipeline.h>
#include <renderware/project_state.h>
#include <renderware/project_node_defs.h>

static inline u8* projectTransformInputs(u8** unk00, s32* unk04,
    s32* unk08, s32* unk0C) {
  u8* unk10;
  u8* unk14;
  s32 unk18;
  if (*(s32*)(*(u8**)_rxExecCtxGlobal + 0x10) == 3) {
    *(s32*)(*(u8**)_rxExecCtxGlobal + 0x10) = 2;
    unk10 = *(u8**)(*(u8**)_rxExecCtxGlobal + 0x14);
  } else {
    unk10 = 0;
  }
  *unk00 = unk10;
  *unk04 = 0;
  *unk08 = 0;
  *unk0C = 0;
  unk18 = *(s32*)(*(u8**)(unk10 + 8));
  if (unk18 == -1) {
    unk14 = 0;
  } else {
    *(u32*)(unk10 + unk18 * 0x1C + 0x1C) =
        *(u32*)(unk10 + unk18 * 0x1C + 0x18);
    unk14 = unk10 + *(s32*)(*(u8**)(unk10 + 8)) * 0x1C + 0x14;
  }
  return unk14;
}

static s32 TransformNodeParallel(u8* unk00, const u8* unk04) {
  u8* unk08;
  u8* unk10;
  u8* unk14;
  u8* unk18;
  u8* unk20;
  u32 unk30;
  u32 unk34;
  s32 unk38;
  u32 unk3C;
  u32 unk3E;
  u32 unk40;
  union {
    u32 unk00;
    u8 unk04[4];
  } unk44;
  unk08 = *(u8**)(unk04 + 0x60);
  _rwClipInfoGlobal[6] = (f32)*(s16*)(unk08 + 0x1C);
  _rwClipInfoGlobal[7] = (f32)*(s16*)(unk08 + 0x1E);
  _rwClipInfoGlobal[4] = (f32)*(s32*)(unk08 + 0xC);
  _rwClipInfoGlobal[5] = (f32)*(s32*)(unk08 + 0x10);
  _rwClipInfoGlobal[3] = *(f32*)(unk04 + 0x84);
  _rwClipInfoGlobal[2] = *(f32*)(unk04 + 0x80);
  _rwClipInfoGlobal[0] = *(f32*)(unk04 + 0x8C);
  _rwClipInfoGlobal[1] = *(f32*)(unk04 + 0x90);
  {
    u8* unk9C;
    u8* unk0C;
    {
      s32 unk24;
      s32 unk28;
      s32 unk2C;
      u8* unk1C;
      unk10 = projectTransformInputs(&unk0C, &unk24, &unk28, &unk2C);
      unk14 = RxClusterLockWrite(unk0C, 1, unk00);
      unk18 = RxClusterLockWrite(unk0C, 2, unk00);
      unk1C = *(u8**)(RxClusterLockWrite(unk0C, 3, unk00) + 8);
      if (*(u32*)(unk1C + 0xA8) == 0) {
        return 1;
      }
      unk9C = RxClusterInitializeData(unk14, *(u32*)(unk1C + 0xA8), 0x28);
      unk20 = RxClusterInitializeData(unk18, *(u32*)(unk1C + 0xA8), 0x1C);
      unk30 = *(u32*)(unk1C + 0xA8);
      if (*(u32*)unk1C & 0x100) {
        unk24 = 1;
      } else if (*(u32*)unk1C & 8) {
        unk28 = 1;
      } else {
        unk44.unk00 = *(u32*)(unk1C + 0x98);
      }
      if (*(u32*)unk1C & 4) {
        unk2C = 1;
      }
      unk34 = 0;
      unk38 = -1;
      unk3C = *(u16*)(unk10 + 2);
      unk3E = *(u16*)(unk9C + 2);
      unk40 = *(u16*)(unk20 + 2);
      while (unk30-- != 0) {
        f32 unk4C = *(f32*)(*(u8**)(unk10 + 8));
        f32 unk50 = *(f32*)(*(u8**)(unk10 + 8) + 4);
        f32 unk54 = *(f32*)(*(u8**)(unk10 + 8) + 8);
        f32 unk58 = *(f32*)(unk1C + 0x78) +
        ((unk54 * *(f32*)(unk1C + 0x68)) +
        ((unk4C * *(f32*)(unk1C + 0x48)) + (unk50 * *(f32*)(unk1C + 0x58))));
        f32 unk5C = *(f32*)(unk1C + 0x7C) +
        ((unk54 * *(f32*)(unk1C + 0x6C)) +
        ((unk4C * *(f32*)(unk1C + 0x4C)) + (unk50 * *(f32*)(unk1C + 0x5C))));
        f32 unk60 = *(f32*)(unk1C + 0x80) +
        ((unk54 * *(f32*)(unk1C + 0x70)) +
        ((unk4C * *(f32*)(unk1C + 0x50)) + (unk50 * *(f32*)(unk1C + 0x60))));
        *(f32*)(*(u8**)(unk9C + 8)) = unk58;
        *(f32*)(*(u8**)(unk9C + 8) + 4) = unk5C;
        *(f32*)(*(u8**)(unk9C + 8) + 8) = unk60;
        if (unk28 != 0) {
          unk44.unk04[0] = *(u8*)(*(u8**)(unk10 + 8) + 0x18);
          unk44.unk04[1] = *(u8*)(*(u8**)(unk10 + 8) + 0x19);
          unk44.unk04[2] = *(u8*)(*(u8**)(unk10 + 8) + 0x1A);
          unk44.unk04[3] = *(u8*)(*(u8**)(unk10 + 8) + 0x1B);
          *(f32*)(*(u8**)(unk9C + 8) + 0x10) = (f32)unk44.unk04[0];
          *(f32*)(*(u8**)(unk9C + 8) + 0x14) = (f32)unk44.unk04[1];
          *(f32*)(*(u8**)(unk9C + 8) + 0x18) = (f32)unk44.unk04[2];
          *(f32*)(*(u8**)(unk9C + 8) + 0x1C) = (f32)unk44.unk04[3];
        } else if (unk24 != 0) {
          unk44.unk04[0] = *(u8*)(*(u8**)(unk10 + 8) + 0x18);
          unk44.unk04[1] = *(u8*)(*(u8**)(unk10 + 8) + 0x19);
          unk44.unk04[2] = *(u8*)(*(u8**)(unk10 + 8) + 0x1A);
          unk44.unk04[3] = *(u8*)(*(u8**)(unk10 + 8) + 0x1B);
          *(f32*)(*(u8**)(unk9C + 8) + 0x10) = (f32)unk44.unk04[0];
          *(f32*)(*(u8**)(unk9C + 8) + 0x14) = (f32)unk44.unk04[1];
          *(f32*)(*(u8**)(unk9C + 8) + 0x18) = (f32)unk44.unk04[2];
          *(f32*)(*(u8**)(unk9C + 8) + 0x1C) = (f32)unk44.unk04[3];
        } else {
          *(f32*)(*(u8**)(unk9C + 8) + 0x10) = 0.0f;
          *(f32*)(*(u8**)(unk9C + 8) + 0x14) = 0.0f;
          *(f32*)(*(u8**)(unk9C + 8) + 0x18) = 0.0f;
          *(f32*)(*(u8**)(unk9C + 8) + 0x1C) = 255.0f;
        }
        *(u8*)(*(u8**)(unk20 + 8) + 0xC) = unk44.unk04[0];
        *(u8*)(*(u8**)(unk20 + 8) + 0xD) = unk44.unk04[1];
        *(u8*)(*(u8**)(unk20 + 8) + 0xE) = unk44.unk04[2];
        *(u8*)(*(u8**)(unk20 + 8) + 0xF) = unk44.unk04[3];
        {
          s32 unk64;
          s32 unk6C;
          s32 unk68;
          if (unk60 < _rwClipInfoGlobal[2]) {
            unk64 = 0x10;
          } else if (unk60 > _rwClipInfoGlobal[3]) {
            unk64 = 0x20;
          } else {
            unk64 = 0;
          }
          if (unk5C < 0.0f) {
            unk68 = 4;
          } else if (unk5C > 1.0f) {
            unk68 = 8;
          } else {
            unk68 = 0;
          }
          if (unk58 < 0.0f) {
            unk6C = 1;
          } else if (unk58 > 1.0f) {
            unk6C = 2;
          } else {
            unk6C = 0;
          }
          *(u8*)(*(u8**)(unk9C + 8) + 0xC) = unk6C | (unk68 | unk64);
        }
        {
          u8 unk70 = *(u8*)(*(u8**)(unk9C + 8) + 0xC);
          unk34 |= unk70;
          unk38 &= unk70;
          if (unk70 == 0) {
            *(f32*)(*(u8**)(unk20 + 8)) =
            _rwClipInfoGlobal[6] + (unk58 * _rwClipInfoGlobal[4]);
            *(f32*)(*(u8**)(unk20 + 8) + 4) =
            _rwClipInfoGlobal[7] + (unk5C * _rwClipInfoGlobal[5]);
            *(f32*)(*(u8**)(unk20 + 8) + 8) =
            _rwClipInfoGlobal[1] + (unk60 * _rwClipInfoGlobal[0]);
            if (unk2C != 0) {
              f32 unk74 = *(f32*)(*(u8**)(unk10 + 8) + 0x1C);
              f32 unk78 = *(f32*)(*(u8**)(unk10 + 8) + 0x20);
              *(f32*)(*(u8**)(unk9C + 8) + 0x20) = unk74;
              *(f32*)(*(u8**)(unk9C + 8) + 0x24) = unk78;
              *(f32*)(*(u8**)(unk20 + 8) + 0x10) = unk74;
              *(f32*)(*(u8**)(unk20 + 8) + 0x14) = unk78;
            }
          } else {
            if (unk2C != 0) {
              f32 unk7C = *(f32*)(*(u8**)(unk10 + 8) + 0x1C);
              f32 unk80 = *(f32*)(*(u8**)(unk10 + 8) + 0x20);
              *(f32*)(*(u8**)(unk9C + 8) + 0x20) = unk7C;
              *(f32*)(*(u8**)(unk9C + 8) + 0x24) = unk80;
            }
            *(f32*)(*(u8**)(unk20 + 8) + 8) = 0.0f;
          }
        }
        *(u8**)(unk10 + 8) += unk3C;
        *(u8**)(unk9C + 8) += unk3E;
        *(u8**)(unk20 + 8) += unk40;
      }
      *(u32*)(unk20 + 0x10) = *(u32*)(unk1C + 0xA8);
      *(u32*)(unk9C + 0x10) = *(u32*)(unk1C + 0xA8);
      *(u32*)(unk1C + 0xAC) = unk34;
      *(s32*)(unk1C + 0xB0) = unk38;
      if (unk38 != 0) {
        u8* unk84 = *(u8**)_rxExecCtxGlobal;
        if (*(s32*)(_rxExecCtxGlobal + 8) != 0) {
          u8* unk88 = _rxEmbeddedPacketBetweenNodes(unk84, unk00, 1);
          if (unk88 != 0) {
            u32 unk8C = (*(s32 (**)(u8*, u8*))(*(u8**)unk88 + 4))(
            unk88, _rxExecCtxGlobal + 0x10);
            if (unk8C == 0) {
              *(u32*)(_rxExecCtxGlobal + 8) = unk8C;
            }
          }
        }
        if (*(s32*)(unk84 + 0x10) > 1) {
          *(s32*)(unk84 + 0x10) = 2;
          _rxPacketDestroy(*(u8**)(unk84 + 0x14));
        }
      } else {
        u8* unk90 = *(u8**)_rxExecCtxGlobal;
        if (*(s32*)(_rxExecCtxGlobal + 8) != 0) {
          u8* unk94 = _rxEmbeddedPacketBetweenNodes(unk90, unk00, 0);
          if (unk94 != 0) {
            u32 unk98 = (*(s32 (**)(u8*, u8*))(*(u8**)unk94 + 4))(
            unk94, _rxExecCtxGlobal + 0x10);
            if (unk98 == 0) {
              *(u32*)(_rxExecCtxGlobal + 8) = unk98;
            }
          }
        }
        if (*(s32*)(unk90 + 0x10) > 1) {
          *(s32*)(unk90 + 0x10) = 2;
          _rxPacketDestroy(*(u8**)(unk90 + 0x14));
        }
      }
    }
  }
  return 1;
}

static s32 TransformNodePerspective(u8* unk00, const u8* unk04) {
  u8* unk08;
  u8* unk10;
  u8* unk14;
  u8* unk18;
  u8* unk20;
  u32 unk30;
  u32 unk34;
  s32 unk38;
  u32 unk3C;
  u32 unk3E;
  u32 unk40;
  union {
    u32 unk00;
    u8 unk04[4];
  } unk44;
  unk08 = *(u8**)(unk04 + 0x60);
  _rwClipInfoGlobal[6] = (f32)*(s16*)(unk08 + 0x1C);
  _rwClipInfoGlobal[7] = (f32)*(s16*)(unk08 + 0x1E);
  _rwClipInfoGlobal[4] = (f32)*(s32*)(unk08 + 0xC);
  _rwClipInfoGlobal[5] = (f32)*(s32*)(unk08 + 0x10);
  _rwClipInfoGlobal[3] = *(f32*)(unk04 + 0x84);
  _rwClipInfoGlobal[2] = *(f32*)(unk04 + 0x80);
  _rwClipInfoGlobal[0] = *(f32*)(unk04 + 0x8C);
  _rwClipInfoGlobal[1] = *(f32*)(unk04 + 0x90);
  {
    u8* unk9C;
    u8* unk0C;
    {
      s32 unk24;
      s32 unk28;
      s32 unk2C;
      u8* unk1C;
      unk10 = projectTransformInputs(&unk0C, &unk24, &unk28, &unk2C);
      unk14 = RxClusterLockWrite(unk0C, 1, unk00);
      unk18 = RxClusterLockWrite(unk0C, 2, unk00);
      unk1C = *(u8**)(RxClusterLockWrite(unk0C, 3, unk00) + 8);
      if (*(u32*)(unk1C + 0xA8) == 0) {
        return 1;
      }
      unk9C = RxClusterInitializeData(unk14, *(u32*)(unk1C + 0xA8), 0x28);
      unk20 = RxClusterInitializeData(unk18, *(u32*)(unk1C + 0xA8), 0x1C);
      unk30 = *(u32*)(unk1C + 0xA8);
      if (*(u32*)unk1C & 0x100) {
        unk24 = 1;
      } else if (*(u32*)unk1C & 8) {
        unk28 = 1;
      } else {
        unk44.unk00 = *(u32*)(unk1C + 0x98);
      }
      if (*(u32*)unk1C & 4) {
        unk2C = 1;
      }
      unk34 = 0;
      unk38 = -1;
      unk3C = *(u16*)(unk10 + 2);
      unk3E = *(u16*)(unk9C + 2);
      unk40 = *(u16*)(unk20 + 2);
      while (unk30-- != 0) {
        f32 unk4C = *(f32*)(*(u8**)(unk10 + 8));
        f32 unk50 = *(f32*)(*(u8**)(unk10 + 8) + 4);
        f32 unk54 = *(f32*)(*(u8**)(unk10 + 8) + 8);
        f32 unk58 = *(f32*)(unk1C + 0x78) +
            ((unk54 * *(f32*)(unk1C + 0x68)) +
            ((unk4C * *(f32*)(unk1C + 0x48)) + (unk50 * *(f32*)(unk1C + 0x58))));
        f32 unk5C = *(f32*)(unk1C + 0x7C) +
            ((unk54 * *(f32*)(unk1C + 0x6C)) +
            ((unk4C * *(f32*)(unk1C + 0x4C)) + (unk50 * *(f32*)(unk1C + 0x5C))));
        f32 unk60 = *(f32*)(unk1C + 0x80) +
            ((unk54 * *(f32*)(unk1C + 0x70)) +
            ((unk4C * *(f32*)(unk1C + 0x50)) + (unk50 * *(f32*)(unk1C + 0x60))));
        *(f32*)(*(u8**)(unk9C + 8)) = unk58;
        *(f32*)(*(u8**)(unk9C + 8) + 4) = unk5C;
        *(f32*)(*(u8**)(unk9C + 8) + 8) = unk60;
        if (unk28 != 0) {
          unk44.unk04[0] = *(u8*)(*(u8**)(unk10 + 8) + 0x18);
          unk44.unk04[1] = *(u8*)(*(u8**)(unk10 + 8) + 0x19);
          unk44.unk04[2] = *(u8*)(*(u8**)(unk10 + 8) + 0x1A);
          unk44.unk04[3] = *(u8*)(*(u8**)(unk10 + 8) + 0x1B);
          *(f32*)(*(u8**)(unk9C + 8) + 0x10) = (f32)unk44.unk04[0];
          *(f32*)(*(u8**)(unk9C + 8) + 0x14) = (f32)unk44.unk04[1];
          *(f32*)(*(u8**)(unk9C + 8) + 0x18) = (f32)unk44.unk04[2];
          *(f32*)(*(u8**)(unk9C + 8) + 0x1C) = (f32)unk44.unk04[3];
        } else if (unk24 != 0) {
          unk44.unk04[0] = *(u8*)(*(u8**)(unk10 + 8) + 0x18);
          unk44.unk04[1] = *(u8*)(*(u8**)(unk10 + 8) + 0x19);
          unk44.unk04[2] = *(u8*)(*(u8**)(unk10 + 8) + 0x1A);
          unk44.unk04[3] = *(u8*)(*(u8**)(unk10 + 8) + 0x1B);
          *(f32*)(*(u8**)(unk9C + 8) + 0x10) = (f32)unk44.unk04[0];
          *(f32*)(*(u8**)(unk9C + 8) + 0x14) = (f32)unk44.unk04[1];
          *(f32*)(*(u8**)(unk9C + 8) + 0x18) = (f32)unk44.unk04[2];
          *(f32*)(*(u8**)(unk9C + 8) + 0x1C) = (f32)unk44.unk04[3];
        } else {
          *(f32*)(*(u8**)(unk9C + 8) + 0x10) = 0.0f;
          *(f32*)(*(u8**)(unk9C + 8) + 0x14) = 0.0f;
          *(f32*)(*(u8**)(unk9C + 8) + 0x18) = 0.0f;
          *(f32*)(*(u8**)(unk9C + 8) + 0x1C) = 255.0f;
        }
        *(u8*)(*(u8**)(unk20 + 8) + 0xC) = unk44.unk04[0];
        *(u8*)(*(u8**)(unk20 + 8) + 0xD) = unk44.unk04[1];
        *(u8*)(*(u8**)(unk20 + 8) + 0xE) = unk44.unk04[2];
        *(u8*)(*(u8**)(unk20 + 8) + 0xF) = unk44.unk04[3];
        {
          s32 unk64;
          s32 unk6C;
          s32 unk68;
          if (unk60 < _rwClipInfoGlobal[2]) {
            unk64 = 0x10;
          } else if (unk60 > _rwClipInfoGlobal[3]) {
            unk64 = 0x20;
          } else {
            unk64 = 0;
          }
          if (unk5C < 0.0f) {
            unk68 = 4;
          } else if (unk5C > unk60) {
            unk68 = 8;
          } else {
            unk68 = 0;
          }
          if (unk58 < 0.0f) {
            unk6C = 1;
          } else if (unk58 > unk60) {
            unk6C = 2;
          } else {
            unk6C = 0;
          }
          *(u8*)(*(u8**)(unk9C + 8) + 0xC) = unk6C | (unk68 | unk64);
        }
        {
          u8 unk70 = *(u8*)(*(u8**)(unk9C + 8) + 0xC);
          unk34 |= unk70;
          unk38 &= unk70;
          if (unk70 == 0) {
            f32 unkA0 = 1.0f / unk60;
            *(f32*)(*(u8**)(unk20 + 8)) =
                _rwClipInfoGlobal[6] + (_rwClipInfoGlobal[4] * (unk58 * unkA0));
            *(f32*)(*(u8**)(unk20 + 8) + 4) =
                _rwClipInfoGlobal[7] + (_rwClipInfoGlobal[5] * (unk5C * unkA0));
            *(f32*)(*(u8**)(unk20 + 8) + 8) =
                _rwClipInfoGlobal[1] + (unkA0 * _rwClipInfoGlobal[0]);
            if (unk2C != 0) {
              f32 unk74 = *(f32*)(*(u8**)(unk10 + 8) + 0x1C);
              f32 unk78 = *(f32*)(*(u8**)(unk10 + 8) + 0x20);
              *(f32*)(*(u8**)(unk9C + 8) + 0x20) = unk74;
              *(f32*)(*(u8**)(unk9C + 8) + 0x24) = unk78;
              *(f32*)(*(u8**)(unk20 + 8) + 0x10) = unk74;
              *(f32*)(*(u8**)(unk20 + 8) + 0x14) = unk78;
            }
          } else {
            if (unk2C != 0) {
              f32 unk7C = *(f32*)(*(u8**)(unk10 + 8) + 0x1C);
              f32 unk80 = *(f32*)(*(u8**)(unk10 + 8) + 0x20);
              *(f32*)(*(u8**)(unk9C + 8) + 0x20) = unk7C;
              *(f32*)(*(u8**)(unk9C + 8) + 0x24) = unk80;
            }
            *(f32*)(*(u8**)(unk20 + 8) + 8) = 0.0f;
          }
        }
        *(u8**)(unk10 + 8) += unk3C;
        *(u8**)(unk9C + 8) += unk3E;
        *(u8**)(unk20 + 8) += unk40;
      }
      *(u32*)(unk20 + 0x10) = *(u32*)(unk1C + 0xA8);
      *(u32*)(unk9C + 0x10) = *(u32*)(unk1C + 0xA8);
      *(u32*)(unk1C + 0xAC) = unk34;
      *(s32*)(unk1C + 0xB0) = unk38;
      if (unk38 != 0) {
        u8* unk84 = *(u8**)_rxExecCtxGlobal;
        if (*(s32*)(_rxExecCtxGlobal + 8) != 0) {
          u8* unk88 = _rxEmbeddedPacketBetweenNodes(unk84, unk00, 1);
          if (unk88 != 0) {
            u32 unk8C = (*(s32 (**)(u8*, u8*))(*(u8**)unk88 + 4))(
                unk88, _rxExecCtxGlobal + 0x10);
            if (unk8C == 0) {
              *(u32*)(_rxExecCtxGlobal + 8) = unk8C;
            }
          }
        }
        if (*(s32*)(unk84 + 0x10) > 1) {
          *(s32*)(unk84 + 0x10) = 2;
          _rxPacketDestroy(*(u8**)(unk84 + 0x14));
        }
      } else {
        u8* unk90 = *(u8**)_rxExecCtxGlobal;
        if (*(s32*)(_rxExecCtxGlobal + 8) != 0) {
          u8* unk94 = _rxEmbeddedPacketBetweenNodes(unk90, unk00, 0);
          if (unk94 != 0) {
            u32 unk98 = (*(s32 (**)(u8*, u8*))(*(u8**)unk94 + 4))(
                unk94, _rxExecCtxGlobal + 0x10);
            if (unk98 == 0) {
              *(u32*)(_rxExecCtxGlobal + 8) = unk98;
            }
          }
        }
        if (*(s32*)(unk90 + 0x10) > 1) {
          *(s32*)(unk90 + 0x10) = 2;
          _rxPacketDestroy(*(u8**)(unk90 + 0x14));
        }
      }
    }
  }
  return 1;
}

static s32 TransformNode(u8* arg0, u8* arg1) {
  u8* unk00 = *(u8**)RwEngineInstance;
  if (*(s32*)(unk00 + 0x14) != 1) {
    return TransformNodeParallel(arg0, unk00);
  } else {
    return TransformNodePerspective(arg0, unk00);
  }
}

ProjectNodeDef* RxNodeDefinitionGetTransform(void) {
  static ProjectNodeCluster N3clofinterest[4] = {
    {&RxClObjSpace3DVertices, 0, 0},
    {&RxClCamSpace3DVertices, 0, 0},
    {&RxClScrSpace2DVertices, 0, 0},
    {&RxClMeshState, 0, 0}
  };
  static u32 N3inputreqs[4] = {1, 0, 0, 1};
  static u32 N3outcl1[4] = {0, 1, 1, 1};
  static u32 N3outcl2[4] = {0, 1, 1, 1};
  static char _TransformOut[] = "TransformOut";
  static char _TransformOut2[] = "TransformOut2";
  static ProjectNodeOutput N3outputs[2] = {
    {_TransformOut, N3outcl1, 0},
    {_TransformOut2, N3outcl2, 0}
  };
  static char _Transform_csl[] = "Transform.csl";
  static ProjectNodeDef nodeTransformCSL = {
    _Transform_csl, TransformNode, 0, 0, 0, 0, 0, 0,
    4, N3clofinterest, N3inputreqs, 2, N3outputs, 0, 0, 0
  };
  return &nodeTransformCSL;
}
