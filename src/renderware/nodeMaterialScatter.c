/* #audit 2026-10-03T06:01Z clean-room FIXED (audit) */
#include <renderware/project_pipeline.h>
#include <renderware/project_state.h>
#include <renderware/project_node_defs.h>

static s32 _MaterialScatterNode(u8* arg0, u8* arg1) {
  typedef struct {
    u8 unk00[0x1C];
  } Cluster1C;
  typedef struct {
    u8 unk00[0x14];
    Cluster1C unk14[];
  } Packet14;
  u8* unk00;
  u8* unk04;
  s32 unk08;
  u8* unk0C;
  if (*(s32*)(*(u8**)_rxExecCtxGlobal + 0x10) == 3) {
    *(s32*)(*(u8**)_rxExecCtxGlobal + 0x10) = 2;
    unk00 = *(u8**)(*(u8**)_rxExecCtxGlobal + 0x14);
  } else {
    unk00 = 0;
  }
  unk08 = **(s32**)(unk00 + 8);
  if (unk08 == -1) {
    unk04 = 0;
  } else {
    *(void**)(unk00 + unk08 * 0x1C + 0x1C) =
        *(void**)(unk00 + unk08 * 0x1C + 0x18);
    unk04 = (u8*)&((Packet14*)unk00)->unk14[**(s32**)(unk00 + 8)];
  }
  unk0C = *(u8**)(*(u8**)(unk04 + 8) + 0x9C);
  if (unk0C != 0) {
    if (*(s32*)(_rxExecCtxGlobal + 8) != 0) {
      u8* unk10 = *(u8**)_rxExecCtxGlobal;
      _rxEmbeddedPacketBetweenPipelines(unk10, unk0C);
      *(u8**)_rxExecCtxGlobal = unk0C;
      {
        u8* unk14 = *(u8**)(unk0C + 8);
        u8* unk34 = _rxExecCtxGlobal + 0x10;
        ProjectNodeDef* unk18 = *(ProjectNodeDef**)unk14;
        s32 unk1C = unk18->unk04(unk14, unk34);
        if (unk1C == 0U) {
          *(s32*)(_rxExecCtxGlobal + 8) = unk1C;
        }
      }
      *(u8**)_rxExecCtxGlobal = unk10;
    }
    if (*(s32*)(unk0C + 0x10) > 1) {
      *(s32*)(unk0C + 0x10) = 2;
      _rxPacketDestroy(*(u8**)(unk0C + 0x14));
    }
  } else {
    u8* unk24;
    u8* unk20 = *(u8**)(RwEngineInstance + _rxPipelineGlobalsOffset + 0x48);
    if (*(s32*)(_rxExecCtxGlobal + 8) != 0) {
      unk24 = *(u8**)_rxExecCtxGlobal;
      _rxEmbeddedPacketBetweenPipelines(unk24, unk20);
      *(u8**)_rxExecCtxGlobal = unk20;
      {
        u8* unk28 = *(u8**)(unk20 + 8);
        u8* unk38 = _rxExecCtxGlobal + 0x10;
        ProjectNodeDef* unk2C = *(ProjectNodeDef**)unk28;
        s32 unk30 = unk2C->unk04(unk28, unk38);
        if (unk30 == 0U) {
          *(s32*)(_rxExecCtxGlobal + 8) = unk30;
        }
      }
      *(u8**)_rxExecCtxGlobal = unk24;
    }
    if (*(s32*)(unk20 + 0x10) > 1) {
      *(s32*)(unk20 + 0x10) = 2;
      _rxPacketDestroy(*(u8**)(unk20 + 0x14));
    }
  }
  return 1;
}

ProjectNodeDef* RxNodeDefinitionGetMaterialScatter(void) {
  static ProjectNodeCluster N2clofinterest[5] = {
    {&RxClMeshState, 0, 0},
    {&RxClObjSpace3DVertices, 0, 0},
    {&RxClIndices, 0, 0},
    {&RxClRenderState, 0, 0},
    {&RxClLights, 0, 0}
  };
  static u32 N2inputreqs[5] = {1, 2, 2, 2, 2};
  static char _MaterialScatter_csl[] = "MaterialScatter.csl";
  static ProjectNodeDef nodeMaterialScatterCSL = {
    _MaterialScatter_csl, _MaterialScatterNode, 0, 0, 0, 0, 0, 0,
    5, N2clofinterest, N2inputreqs, 0, 0, 0, 0, 0
  };
  return &nodeMaterialScatterCSL;
}
