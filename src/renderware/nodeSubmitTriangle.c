/* #audit 2026-10-03T06:02Z clean-room FIXED (audit) */
#include <renderware/project_state.h>
#include <renderware/project_pipeline.h>
#include <renderware/project_renderstate.h>
#include <renderware/project_node_defs.h>

extern s32 RwRenderStateSet(s32 arg0, u32 arg1);
extern s32 RwIm2DRenderPrimitive(s32 arg0, void* arg1, s32 arg2);
extern s32 RwIm2DRenderIndexedPrimitive(s32 arg0, void* arg1, s32 arg2, void* arg3, s32 arg4);
extern u8* _rxEmbeddedPacketBetweenNodes(u8* arg0, u8* arg1, u32 arg2);
extern void _rxPacketDestroy(u8* arg0);

static inline u8* projectTriangleSlotAt(u8* unk00, u32 unk04) {
  s32 unk08 = *(s32*)(*(u8**)(unk00 + 8) + unk04);
  if (unk08 == -1) {
    return 0;
  }
  *(u32*)(unk00 + unk08 * 0x1C + 0x1C) =
      *(u32*)(unk00 + unk08 * 0x1C + 0x18);
  return unk00 + *(s32*)(*(u8**)(unk00 + 8) + unk04) * 0x1C + 0x14;
}

static s32 SubmitTriangleNode(u8* unk00, u8* unk04) {
  u8* unk08;
  u8* unk0C;
  u8* unk10;
  u8* unk14;
  u8* unk18;
  u8* unk1C;
  u8* unk20;
  if (*(s32*)(*(u8**)_rxExecCtxGlobal + 0x10) == 3) {
    *(s32*)(*(u8**)_rxExecCtxGlobal + 0x10) = 2;
    unk08 = *(u8**)(*(u8**)_rxExecCtxGlobal + 0x14);
  } else {
    unk08 = 0;
  }
  unk0C = projectTriangleSlotAt(unk08, 0);
  unk10 = projectTriangleSlotAt(unk08, 4);
  unk14 = projectTriangleSlotAt(unk08, 8);
  unk18 = projectTriangleSlotAt(unk08, 0xC);
  unk1C = *(u8**)(unk14 + 8);
  if (unk18 != 0 && *(void**)(unk18 + 4) != 0) {
    unk20 = *(u8**)(unk18 + 8);
  } else {
    unk20 = (u8*)&((ProjectRenderModule*)(RwEngineInstance + _rxPipelineGlobalsOffset))->unk04;
  }
  if (*(u32*)(unk20 + 0x10) != 0) {
    RwRenderStateSet(1, *(u32*)(unk20 + 0x10));
    RwRenderStateSet(9, *(u32*)(unk20 + 0x1C));
    if (*(s32*)(unk20 + 0x14) == *(s32*)(unk20 + 0x18)) {
      RwRenderStateSet(2, *(u32*)(unk20 + 0x14));
    } else {
      RwRenderStateSet(3, *(u32*)(unk20 + 0x14));
      RwRenderStateSet(4, *(u32*)(unk20 + 0x18));
    }
  } else {
    RwRenderStateSet(1, 0);
  }
  RwRenderStateSet(12, (*(u32*)unk20 >> 3) & 1);
  if (unk10 != 0 && *(u32*)(unk10 + 0xC) != 0) {
    void* unk34 = *(void**)(unk10 + 8);
    void* unk38 = *(void**)(unk0C + 8);
    RwIm2DRenderIndexedPrimitive(3, unk38, *(s32*)(unk1C + 0xA8),
        unk34, *(s32*)(unk10 + 0x10));
  } else {
    s32 unk44 = *(s32*)(unk1C + 0xA8);
    void* unk48 = *(void**)(unk0C + 8);
    RwIm2DRenderPrimitive(3, unk48, unk44);
  }
  {
    u8* unk24 = *(u8**)_rxExecCtxGlobal;
    if (*(s32*)(_rxExecCtxGlobal + 8) != 0) {
      u8* unk28 = _rxEmbeddedPacketBetweenNodes(unk24, unk00, 0);
      if (unk28 != 0) {
        u32 unk2C = (*(s32 (**)(u8*, u8*))
            (*(u8**)unk28 + 4))(unk28, _rxExecCtxGlobal + 0x10);
        if (unk2C == 0) {
          *(u32*)(_rxExecCtxGlobal + 8) = unk2C;
        }
      }
    }
    if (*(s32*)(unk24 + 0x10) > 1) {
      *(s32*)(unk24 + 0x10) = 2;
      _rxPacketDestroy(*(u8**)(unk24 + 0x14));
    }
  }
  return 1;
}

ProjectNodeDef* RxNodeDefinitionGetSubmitTriangle(void) {
  static ProjectNodeCluster N1clofinterest[4] = {
    {&RxClScrSpace2DVertices, 0, 0},
    {&RxClIndices, 0, 0},
    {&RxClMeshState, 0, 0},
    {&RxClRenderState, 0, 0}
  };
  static u32 N1inputreqs[4] = {1, 2, 1, 2};
  static u32 N1outcl1[4] = {1, 0, 1, 0};
  static char _SubmitOut[] = "SubmitOut";
  static ProjectNodeOutput N1outputs[1] = {
    {_SubmitOut, N1outcl1, 0}
  };
  static char _SubmitTriangle_csl[] = "SubmitTriangle.csl";
  static ProjectNodeDef nodeSubmitTriangleCSL = {
    _SubmitTriangle_csl, SubmitTriangleNode, 0, 0, 0, 0, 0, 0,
    4, N1clofinterest, N1inputreqs, 1, N1outputs, 0, 0, 0
  };
  return &nodeSubmitTriangleCSL;
}
