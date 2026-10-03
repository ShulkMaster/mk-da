#include <renderware/project_pipeline.h>

static inline u8* projectCullRead(u8* arg0, u32 arg1) {
  s32 unk00 = (*(s32**)(arg0 + 8))[arg1];
  u8* unk04;
  if (unk00 == -1) {
    unk04 = 0;
  } else {
    *(u32*)(arg0 + unk00 * 0x1C + 0x1C) =
        *(u32*)(arg0 + unk00 * 0x1C + 0x18);
    unk04 = arg0 + (*(s32**)(arg0 + 8))[arg1] * 0x1C + 0x14;
  }
  return unk04;
}

static inline void projectCullReplace(u8* arg0, u16* arg1, u8* arg2, u32 arg3) {
  u16* unk00 = (u16*)(*(u8**)(arg0 + 4) + (arg3 + (arg3 * 2 - 3)) * 2);
  arg1[0] = unk00[0];
  (*(u16**)(arg0 + 8))[1] = unk00[1];
  (*(u16**)(arg0 + 8))[2] = unk00[2];
  --*(u32*)(arg2 + 0xA4);
}

/* TODO: [near miss] 97.32%; register allocation and offset grouping remain. */
static s32 ParallelCullProcessPacket(u8* arg0, u8* arg1) {
  u32 unk0C;
  u32 unk10;
  u32 unk58;
  u8* unk08;
  u8* unk00;
  u8* unk04;
  unk00 = projectCullRead(arg1, 0);
  projectCullRead(arg1, 1);
  unk04 = RxClusterLockWrite(arg1, 2, arg0);
  unk08 = *(u8**)(RxClusterLockWrite(arg1, 3, arg0) + 8);
  unk0C = *(u16*)(unk04 + 2);
  unk10 = 0;
  while (unk10 < (unk58 = *(u32*)(unk08 + 0xA4))) {
    u16 unk28;
    u16* unk14;
    u16 unk20;
    u16 unk1C;
    f32* unk2C;
    f32* unk30;
    f32* unk34;
    f32 unk38;
    f32 unk40;
    f32 unk44;
    f32 unk48;
    f32 unk4C;
    unk14 = *(u16**)(unk04 + 8);
    unk28 = *(u16*)(unk00 + 2);
    unk1C = unk14[1];
    unk20 = unk14[2];
    unk2C = (f32*)(*(u8**)(unk00 + 4) +
        unk28 * unk14[0]);
    unk30 = (f32*)(*(u8**)(unk00 + 4) + unk28 * unk1C);
    unk34 = (f32*)(*(u8**)(unk00 + 4) + unk28 * unk20);
    unk40 = unk2C[0] - unk30[0];
    unk44 = unk2C[1] - unk30[1];
    unk48 = unk34[0] - unk30[0];
    unk4C = unk34[1] - unk30[1];
    unk38 = unk40 * unk4C - unk44 * unk48;
    if (*(s32*)&unk38 < 0) {
      projectCullReplace(unk04, unk14, unk08, unk58);
      --unk10;
    } else {
      *(u8**)(unk04 + 8) += unk0C * 3;
    }
    ++unk10;
  }
  if (unk58 < *(u32*)(unk04 + 0x10) * 3) {
    *(u32*)(unk04 + 0x10) = unk58 * 3;
  }
  if (*(u32*)(unk08 + 0xA4) == 0) {
    u8* unk5C = *(u8**)_rxExecCtxGlobal;
    projectPipelineForward(arg0, unk5C, 1);
  } else {
    u8* unk5C = *(u8**)_rxExecCtxGlobal;
    projectPipelineForward(arg0, unk5C, 0);
  }
  return 1;
}

/* TODO: [near miss] 94.08%; register allocation and offset grouping remain. */
static s32 PerspectiveCullProcessPacket(u8* arg0, u8* arg1) {
  u8* unk00;
  u8* unk08;
  u8* unk04;
  u32 unk10;
  u32 unk14;
  u8* unk0C;
  union { f32 unk00; s32 unk04; } unkA0;
  unk00 = projectCullRead(arg1, 0);
  unk04 = projectCullRead(arg1, 1);
  unk08 = RxClusterLockWrite(arg1, 2, arg0);
  unk0C = *(u8**)(RxClusterLockWrite(arg1, 3, arg0) + 8);
  unk10 = *(u16*)(unk08 + 2);
  if (*(s32*)(unk0C + 0xAC) == 0) {
    unk14 = 0;
    while (unk14 < *(u32*)(unk0C + 0xA4)) {
      u8* unk28;
      u16* unk18 = *(u16**)(unk08 + 8);
      u16 unk1C = unk18[0];
      u16 unk20 = unk18[1];
      u16 unk24 = unk18[2];
      u16 unk2C;
      f32* unk38;
      f32* unk34;
      f32* unk30;
      unk28 = *(u8**)(unk04 + 4);
      unk2C = *(u16*)(unk04 + 2);
      unk30 = (f32*)(unk28 + unk2C * unk1C);
      unk34 = (f32*)(unk28 + unk2C * unk20);
      unk38 = (f32*)(unk28 + unk2C * unk24);
      {
        f32 unkA4 = unk30[0] - unk34[0];
        f32 unkA8 = unk30[1] - unk34[1];
        f32 unkAC = unk38[0] - unk34[0];
        f32 unkB0 = unk38[1] - unk34[1];
        unkA0.unk00 = unkB0 * unkA4 - unkAC * unkA8;
      }
      if (unkA0.unk04 <= 0) {
        projectCullReplace(unk08, unk18, unk0C, *(u32*)(unk0C + 0xA4));
        --unk14;
      } else {
        *(u8**)(unk08 + 8) = *(u8**)(unk08 + 8) + unk10 * 3;
      }
      ++unk14;
    }
  } else {
    unk14 = 0;
    while (unk14 < *(u32*)(unk0C + 0xA4)) {
      u16* unk18 = *(u16**)(unk08 + 8);
      u16 unk1C = unk18[0];
      u16 unk2C = *(u16*)(unk00 + 2);
      u16 unk20 = unk18[1];
      u8* unk28 = *(u8**)(unk00 + 4);
      s32 unk24 = unk18[2];
      u8* unk30 = unk28 + unk2C * unk1C;
      u8* unk34 = unk28 + (u32)unk20 * unk2C;
      u8* unk38 = unk28 + unk2C * unk24;
      u8 unk3C = unk30[0xC];
      u8 unk40 = unk34[0xC];
      u8 unk44 = unk38[0xC];
      if ((unk44 & (unk3C & unk40)) == 0) {
        if ((unk44 | (unk3C | unk40)) != 0) {
          f32 unk50;
          f32 unk5C;
          f32 unk68;
          f32 unk6C;
          f32 unk70;
          f32 unk74;
          f32 unk48;
          f32 unk54;
          f32 unk60;
          f32 unk4C;
          f32 unk58;
          f32 unk64;
          f32 unk78;
          f32 unk7C;
          f32 unk80;
          unk48 = *(f32*)(unk28 + unk2C * unk1C);
          unk4C = *(f32*)unk34;
          unk50 = unk48 - unk4C;
          unk54 = *(f32*)(unk30 + 4);
          unk58 = *(f32*)(unk34 + 4);
          unk5C = unk54 - unk58;
          unk60 = *(f32*)(unk30 + 8);
          unk64 = *(f32*)(unk34 + 8);
          unk68 = unk60 - unk64;
          unk6C = *(f32*)unk38 - unk4C;
          unk70 = *(f32*)(unk38 + 4) - unk58;
          unk74 = *(f32*)(unk38 + 8) - unk64;
          unk78 = unk5C * unk74 - unk68 * unk70;
          unk7C = unk68 * unk6C - unk50 * unk74;
          unk80 = unk50 * unk70 - unk5C * unk6C;
          unkA0.unk00 = (unk78 * unk48 + unk7C * unk54) + unk80 * unk60;
          if (unkA0.unk04 <= 0) {
            projectCullReplace(unk08, unk18, unk0C, *(u32*)(unk0C + 0xA4));
            --unk14;
          } else {
            *(u8**)(unk08 + 8) = *(u8**)(unk08 + 8) + unk10 * 3;
          }
        } else {
          u8* unk88 = *(u8**)(unk04 + 4);
          u16 unk8C = *(u16*)(unk04 + 2);
          f32* unk90;
          f32* unk94 = (f32*)(unk88 + unk8C * unk20);
          f32* unk98;
          unk90 = (f32*)(unk88 + unk8C * unk1C);
          unk98 = (f32*)(unk88 + unk8C * unk24);
          {
            f32 unkA4 = unk90[0] - unk94[0];
            f32 unkA8 = unk90[1] - unk94[1];
            f32 unkAC = unk98[0] - unk94[0];
            f32 unkB0 = unk98[1] - unk94[1];
            unkA0.unk00 = unkB0 * unkA4 - unkAC * unkA8;
          }
          if (unkA0.unk04 <= 0) {
            projectCullReplace(unk08, unk18, unk0C, *(u32*)(unk0C + 0xA4));
            --unk14;
          } else {
            *(u8**)(unk08 + 8) = *(u8**)(unk08 + 8) + unk10 * 3;
          }
        }
      } else {
        projectCullReplace(unk08, unk18, unk0C, *(u32*)(unk0C + 0xA4));
        --unk14;
      }
      ++unk14;
    }
  }
  if (*(u32*)(unk0C + 0xA4) < *(u32*)(unk08 + 0x10) * 3) {
    *(u32*)(unk08 + 0x10) = *(u32*)(unk0C + 0xA4) * 3;
  }
  if (*(u32*)(unk0C + 0xA4) == 0) {
    u8* unkC0 = _rxExecCtxGlobal;
    u8* unkC4 = *(u8**)unkC0;
    if (*(s32*)(unkC0 + 8) != 0) {
      u8* unkC8 = _rxEmbeddedPacketBetweenNodes(unkC4, arg0, 1);
      if (unkC8 != 0) {
        u8* unkCC = _rxExecCtxGlobal + 0x10;
        u32 unkD0 = (*(s32 (**)(u8*, u8*))(*(u8**)unkC8 + 4))(unkC8, unkCC);
        if (unkD0 == 0) {
          *(u32*)(unkC0 + 8) = unkD0;
        }
      }
    }
    if (*(s32*)(unkC4 + 0x10) > 1) {
      *(s32*)(unkC4 + 0x10) = 2;
      _rxPacketDestroy(*(u8**)(unkC4 + 0x14));
    }
  } else {
    u8* unkC0 = _rxExecCtxGlobal;
    u8* unkC4 = *(u8**)unkC0;
    if (*(s32*)(unkC0 + 8) != 0) {
      u8* unkC8 = _rxEmbeddedPacketBetweenNodes(unkC4, arg0, 0);
      if (unkC8 != 0) {
        u8* unkCC = _rxExecCtxGlobal + 0x10;
        u32 unkD0 = (*(s32 (**)(u8*, u8*))(*(u8**)unkC8 + 4))(unkC8, unkCC);
        if (unkD0 == 0) {
          *(u32*)(unkC0 + 8) = unkD0;
        }
      }
    }
    if (*(s32*)(unkC4 + 0x10) > 1) {
      *(s32*)(unkC4 + 0x10) = 2;
      _rxPacketDestroy(*(u8**)(unkC4 + 0x14));
    }
  }
  return 1;
}

#include <renderware/project_state.h>

static s32 _CullTriangleNode(u8* arg0, u8* arg1) {
  static s32 (*CullFunc[2])(u8*, u8*) = {
    ParallelCullProcessPacket, PerspectiveCullProcessPacket
  };
  s32 (*unk00)(u8*, u8*) = CullFunc[*(s32*)(*(u8**)RwEngineInstance + 0x14) == 1];
  u8* unk04;
  if (*(s32*)(*(u8**)_rxExecCtxGlobal + 0x10) == 3) {
    *(s32*)(*(u8**)_rxExecCtxGlobal + 0x10) = 2;
    unk04 = *(u8**)(*(u8**)_rxExecCtxGlobal + 0x14);
  } else {
    unk04 = 0;
  }
  return unk00(arg0, unk04);
}


#include <renderware/project_node_defs.h>

ProjectNodeDef* RxNodeDefinitionGetCullTriangle(void) {
  static ProjectNodeCluster N1clofinterest[4] = {
    {&RxClCamSpace3DVertices, 0, 0},
    {&RxClScrSpace2DVertices, 0, 0},
    {&RxClIndices, 0, 0},
    {&RxClMeshState, 0, 0}
  };
  static u32 N1inputreqs[4] = {1, 1, 1, 1};
  static u32 N1outcl1[4] = {0, 0, 1, 1};
  static u32 N1outcl2[4] = {0, 0, 2, 1};
  static char _CullTriangleOut[] = "CullTriangleOut";
  static char _CullTriangleAllCulled[] = "CullTriangleAllCulled";
  static ProjectNodeOutput N1outputs[2] = {
    {_CullTriangleOut, N1outcl1, 0},
    {_CullTriangleAllCulled, N1outcl2, 0}
  };
  static char _CullTriangle_csl[] = "CullTriangle.csl";
  static ProjectNodeDef nodeCullTriangleCSL = {
    _CullTriangle_csl, _CullTriangleNode, 0, 0, 0, 0, 0, 0,
    4, N1clofinterest, N1inputreqs, 2, N1outputs, 0, 0, 0
  };
  return &nodeCullTriangleCSL;
}
