#include <renderware/project_pipeline.h>
#include <renderware/project_heap.h>

/* TODO: [near miss] 99.35%; output-count and strip-loop registers remain. */
static s32 _ImmMangleTriangleIndicesNode(u8* unk00, u8* unk04) {
  typedef struct {
    u8 unk00[0x1C];
  } ProjectTriangleCluster;
  typedef struct {
    u8 unk00[0x14];
    ProjectTriangleCluster unk14[];
  } ProjectTrianglePacket;
  u8* unk08;
  u8* unk10;
  u32 unk48;
  u8* unk0C;
  u32 unk14;
  if (*(s32*)(*(u8**)_rxExecCtxGlobal + 0x10) == 3) {
    *(s32*)(*(u8**)_rxExecCtxGlobal + 0x10) = 2;
    unk08 = *(u8**)(*(u8**)_rxExecCtxGlobal + 0x14);
  } else {
    unk08 = 0;
  }
  unk0C = *(u8**)(RxClusterLockWrite(unk08, 0, unk00) + 8);
  if ((*(s32**)(unk08 + 8))[1] == -1) {
    unk10 = 0;
  } else {
    u8* unk18 = unk08 + (*(s32**)(unk08 + 8))[1] * 0x1C;
    *(u32*)(unk18 + 0x1C) = *(u32*)(unk18 + 0x18);
    unk10 = (u8*)&((ProjectTrianglePacket*)unk08)->unk14[(*(s32**)(unk08 + 8))[1]];
  }
  unk14 = *(u32*)(unk10 + 0x10);
  if (unk14 == 0) {
    u32 unk44;
    u8* unk1C = RxClusterLockWrite(unk08, 1, unk00);
    switch (*(s32*)(unk0C + 0xA0)) {
    case 3: {
      u8* unk24;
      u32 unk2C;
      u16* unk28;
      unk44 = *(u32*)(unk0C + 0xA4) * 3;
      unk24 = RxClusterInitializeData(unk1C, unk44, 2);
      unk28 = *(u16**)(unk24 + 8);
      for (unk2C = 0; unk2C < unk44; ++unk2C) {
        *unk28 = unk2C;
        unk28++;
      }
      *(u32*)(unk24 + 0x10) = unk44;
      break;
    }
    case 5: {
      u8* unk24;
      u32 unk30;
      u16* unk28;
      u32 unk2C;
      unk44 = *(u32*)(unk0C + 0xA4) * 3;
      unk24 = RxClusterInitializeData(unk1C, unk44, 2);
      unk28 = *(u16**)(unk24 + 8);
      for (unk2C = 0; unk2C < *(u32*)(unk0C + 0xA4); ++unk2C) {
        unk30 = unk2C + 1;
        unk28[0] = 0;
        unk28[1] = unk30++;
        unk28[2] = unk30;
        unk28 += 3;
      }
      *(u32*)(unk24 + 0x10) = unk44;
      *(s32*)(unk0C + 0xA0) = 3;
      break;
    }
    case 4: {
      u8* unk24;
      u32 unk30;
      u16* unk28;
      u32 unk2C;
      unk44 = *(u32*)(unk0C + 0xA4) * 3;
      unk24 = RxClusterInitializeData(unk1C, unk44, 2);
      unk28 = *(u16**)(unk24 + 8);
      for (unk2C = 0; unk2C < *(u32*)(unk0C + 0xA4); ++unk2C) {
        unk30 = unk2C & 1;
        unk28[0] = unk2C + unk30;
        unk28[1] = unk2C + (unk30 ^ 1);
        unk28[2] = unk2C + 2;
        unk28 += 3;
      }
      *(u32*)(unk24 + 0x10) = unk44;
      *(s32*)(unk0C + 0xA0) = 3;
      break;
    }
    default: {
      projectPipelineForward(unk00, *(u8**)_rxExecCtxGlobal, 0);
      return 0;
    }
    }
  } else {
    u32 unk44;
    u32 unk38;
    switch (*(s32*)(unk0C + 0xA0)) {
    case 5: {
      u32 unk1C;
      u16 unk24;
      u8* unk20;
      void* unk28;
      u8* unk30;
      u8* unk34;
      unk1C = unk14 - 2;
      unk20 = *(u8**)(unk10 + 4) + *(u16*)(unk10 + 2) * unk1C;
      unk24 = **(u16**)(unk10 + 8);
      unk28 = RxHeapAlloc(*(ProjectHeapState**)(unk04 + 4), unk1C * 6);
      unk30 = RxClusterSetData(unk10, unk28, 2, unk44 = unk1C * 3);
      unk34 = *(u8**)(unk30 + 4) + *(u16*)(unk30 + 2) * ((unk1C - 1) * 3);
      for (unk38 = 0; unk38 < unk1C; ++unk38) {
        *(u16*)(unk34 + 4) = *(u16*)(unk20 + 2);
        *(u16*)(unk34 + 2) = *(u16*)unk20;
        *(u16*)unk34 = unk24;
        unk34 -= 6;
        unk20 -= 2;
      }
      *(u32*)(unk30 + 0x10) = unk44;
      *(s32*)(unk0C + 0xA0) = 3;
      break;
    }
    case 4: {
      u8* unk20;
      void* unk24;
      u8* unk2C;
      u32 unk34;
      u8* unk30;
      unk48 = unk14 - 2;
      unk20 = *(u8**)(unk10 + 4) + *(u16*)(unk10 + 2) * (unk14 - 3);
      unk24 = RxHeapAlloc(*(ProjectHeapState**)(unk04 + 4), unk48 * 6);
      unk2C = RxClusterSetData(unk10, unk24, 2, unk44 = unk48 * 3);
      unk30 = *(u8**)(unk2C + 4) + *(u16*)(unk2C + 2) * ((unk48 - 1) * 3);
      unk34 = unk48 & 1;
      for (unk38 = 0; unk38 < unk48; ++unk38) {
        u16 unk3C = ((u16*)unk20)[unk34];
        u16 unk40 = ((u16*)unk20)[unk34 ^ 1];
        *(u16*)(unk30 + 4) = *(u16*)(unk20 + 4);
        *(u16*)(unk30 + 2) = unk3C;
        *(u16*)unk30 = unk40;
        unk30 -= 6;
        unk20 -= 2;
        unk34 ^= 1;
      }
      *(u32*)(unk2C + 0x10) = unk44;
      *(s32*)(unk0C + 0xA0) = 3;
      break;
    }
    case 3:
      break;
    default: {
      projectPipelineForward(unk00, *(u8**)_rxExecCtxGlobal, 0);
      return 0;
    }
    }
  }
  projectPipelineForward(unk00, *(u8**)_rxExecCtxGlobal, 0);
  return 1;
}

#include <renderware/project_node_defs.h>

ProjectNodeDef* RxNodeDefinitionGetImmMangleTriangleIndices(void) {
  static ProjectNodeCluster N1clofinterest[2] = {
    {&RxClMeshState, 0, 0},
    {&RxClIndices, 0, 0},
  };
  static u32 N1inputreqs[2] = {1, 2};
  static u32 N1outValid[2] = {1, 1};
  static char _ImmRenderSetupOut[] = "ImmRenderSetupOut";
  static ProjectNodeOutput N1outputs[1] = {
    {_ImmRenderSetupOut, N1outValid, 0},
  };
  static char _NodeName[] = "ImmMangleTriangleIndices.csl";
  static ProjectNodeDef NodeDefinition = {
    _NodeName, _ImmMangleTriangleIndicesNode,
    0, 0, 0, 0, 0, 0, 2, N1clofinterest, N1inputreqs, 1, N1outputs, 0, 0, 0,
  };
  return &NodeDefinition;
}
