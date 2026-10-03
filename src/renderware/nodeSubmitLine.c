#include <renderware/project_node.h>

static s32 SubmitLineNode(u8* arg0, u8* arg1) {
  u8* unk00;
  u8* unk04;
  u8* unk10;
  u8* unk14;
  u8* unk18;
  if (*(s32*)(*(u8**)_rxExecCtxGlobal + 0x10) == 3) {
    *(s32*)(*(u8**)_rxExecCtxGlobal + 0x10) = 2;
    unk10 = *(u8**)(*(u8**)_rxExecCtxGlobal + 0x14);
  } else {
    unk10 = 0;
  }
  if (((s32*)*(u8**)(unk10 + 8))[0] == -1) {
    unk00 = 0;
  } else {
    s32 unk1C = ((s32*)*(u8**)(unk10 + 8))[0];
    *(u32*)(unk10 + unk1C * 0x1C + 0x1C) = *(u32*)(unk10 + unk1C * 0x1C + 0x18);
    unk00 = unk10 + ((s32*)*(u8**)(unk10 + 8))[0] * 0x1C + 0x14;
  }
  if (((s32*)*(u8**)(unk10 + 8))[1] == -1) {
    unk04 = 0;
  } else {
    s32 unk1C = ((s32*)*(u8**)(unk10 + 8))[1];
    *(u32*)(unk10 + unk1C * 0x1C + 0x1C) = *(u32*)(unk10 + unk1C * 0x1C + 0x18);
    unk04 = unk10 + ((s32*)*(u8**)(unk10 + 8))[1] * 0x1C + 0x14;
  }
  if (((s32*)*(u8**)(unk10 + 8))[2] == -1) {
    unk14 = 0;
  } else {
    s32 unk1C = ((s32*)*(u8**)(unk10 + 8))[2];
    *(u32*)(unk10 + unk1C * 0x1C + 0x1C) = *(u32*)(unk10 + unk1C * 0x1C + 0x18);
    unk14 = unk10 + ((s32*)*(u8**)(unk10 + 8))[2] * 0x1C + 0x14;
  }
  if (((s32*)*(u8**)(unk10 + 8))[3] == -1) {
    unk18 = 0;
  } else {
    s32 unk1C = ((s32*)*(u8**)(unk10 + 8))[3];
    *(u32*)(unk10 + unk1C * 0x1C + 0x1C) = *(u32*)(unk10 + unk1C * 0x1C + 0x18);
    unk18 = unk10 + ((s32*)*(u8**)(unk10 + 8))[3] * 0x1C + 0x14;
  }
  {
    u8* unk0C;
    u32* unk08 = *(u32**)(unk14 + 8);
    if (unk18 != 0 && *(u32*)(unk18 + 4) != 0) {
      unk0C = *(u8**)(unk18 + 8);
    } else {
      unk0C = RwEngineInstance + _rxPipelineGlobalsOffset + 4;
    }
    if (*(u32*)(unk0C + 0x10) != 0) {
      RwRenderStateSet(1, *(u32*)(unk0C + 0x10));
      RwRenderStateSet(9, *(u32*)(unk0C + 0x1C));
      if (*(s32*)(unk0C + 0x14) == *(s32*)(unk0C + 0x18)) {
        RwRenderStateSet(2, *(u32*)(unk0C + 0x14));
      } else {
        RwRenderStateSet(3, *(u32*)(unk0C + 0x14));
        RwRenderStateSet(4, *(u32*)(unk0C + 0x18));
      }
    } else {
      RwRenderStateSet(1, 0);
    }
    RwRenderStateSet(7, *(u32*)(unk0C + 4));
    RwRenderStateSet(0xC, (*(u32*)unk0C >> 3) & 1);
    RwRenderStateSet(0xA, *(u32*)(unk0C + 8));
    RwRenderStateSet(0xB, *(u32*)(unk0C + 0xC));
    RwRenderStateSet(6, (*(u32*)unk0C >> 1) & 1);
    RwRenderStateSet(8, (*(u32*)unk0C >> 2) & 1);
    if (unk04 != 0 && *(u32*)(unk04 + 0x10) != 0) {
      void* unk30 = *(void**)(unk04 + 8);
      void* unk34 = *(void**)(unk00 + 8);
      RwIm2DRenderIndexedPrimitive(1, unk34, (s32)unk08[0x2A],
          unk30, (s32)unk08[0x29] * 2);
    } else {
      void* unk34 = *(void**)(unk00 + 8);
      RwIm2DRenderPrimitive(1, unk34, (s32)unk08[0x2A]);
    }
  }
  {
    u8* unk20 = *(u8**)_rxExecCtxGlobal;
    if (*(s32*)(_rxExecCtxGlobal + 8) != 0) {
      u8* unk24 = _rxEmbeddedPacketBetweenNodes(unk20, arg0, 0);
      if (unk24 != 0) {
        s32 unk28 = (*(s32 (**)(u8*, u8*))(*(u8**)unk24 + 4))(unk24, _rxExecCtxGlobal + 0x10);
        if (unk28 == 0U) {
          *(s32*)(_rxExecCtxGlobal + 8) = unk28;
        }
      }
    }
    if (*(s32*)(unk20 + 0x10) > 1) {
      *(s32*)(unk20 + 0x10) = 2;
      _rxPacketDestroy(*(u8**)(unk20 + 0x14));
    }
  }
  return 1;
}


#include <renderware/project_node_defs.h>

ProjectNodeDef* RxNodeDefinitionGetSubmitLine(void) {
  static ProjectNodeCluster N1clofinterest[4] = {
    {&RxClScrSpace2DVertices, 0, 0},
    {&RxClIndices, 0, 0},
    {&RxClMeshState, 0, 0},
    {&RxClRenderState, 0, 0}
  };
  static u32 N1inputreqs[4] = {1, 2, 1, 2};
  static u32 N1outcl1[4] = {0, 0, 0, 0};
  static char _SubmitOut[] = "SubmitOut";
  static ProjectNodeOutput N1outputs = {
    _SubmitOut, N1outcl1, 0
  };
  static char _SubmitLine_csl[] = "SubmitLine.csl";
  static ProjectNodeDef nodeSubmitLineCSL = {
    _SubmitLine_csl, SubmitLineNode,
    0, 0, 0, 0, 0, 0, 4, N1clofinterest, N1inputreqs, 1,
    &N1outputs, 0, 0, 0
  };

  return &nodeSubmitLineCSL;
}
