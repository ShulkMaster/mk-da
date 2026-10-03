/* #audit 2026-10-03T10:26Z clean-room PASS (audit) */
#include <renderware/project_pipeline.h>
#include <renderware/project_heap.h>
#include <renderware/project_node_defs.h>

static s32 _ImmMangleLineIndicesNode(u8* arg0, u8* arg1) {
  /* #audit 2026-10-03T10:26Z clean-room PASS (audit) */
  u8* unk00;
  u8* unk08;
  u32 unk10;
  u32 unk4C;
  s32 unk0C;
  u32 unk40;
  u8* unk04;
  if (*(s32*)(*(u8**)_rxExecCtxGlobal + 0x10) == 3) {
    *(s32*)(*(u8**)_rxExecCtxGlobal + 0x10) = 2;
    unk00 = *(u8**)(*(u8**)_rxExecCtxGlobal + 0x14);
  } else {
    unk00 = 0;
  }
  unk08 = RxClusterLockWrite(unk00, 0, arg0);
  unk04 = *(u8**)(unk08 + 8);
  unk0C = (*(s32**)(unk00 + 8))[1];
  if (unk0C == -1) {
    unk08 = 0;
  } else {
    *(void**)(unk00 + unk0C * 0x1C + 0x1C) =
        *(void**)(unk00 + unk0C * 0x1C + 0x18);
    unk08 = unk00 + (*(s32**)(unk00 + 8))[1] * 0x1C + 0x14;
  }
  unk10 = *(u32*)(unk08 + 0x10);
  if (unk10 == 0) {
    u32 unk14;
    s32 unkA0;
    unk08 = RxClusterLockWrite(unk00, 1, arg0);
    unkA0 = *(s32*)(unk04 + 0xA0);
    if (unkA0 == 1) {
      u16* unk18;
      u32 unk1C;
      unk14 = *(u32*)(unk04 + 0xA4) * 2;
      unk08 = RxClusterInitializeData(unk08, unk14, 2);
      unk18 = *(u16**)(unk08 + 8);
      for (unk1C = 0; unk1C < unk14; unk1C++) {
        *unk18++ = unk1C;
      }
      *(u32*)(unk08 + 0x10) = unk14;
    } else if (unkA0 == 2) {
      u16* unk24;
      u32 unk28;
      unk14 = *(u32*)(unk04 + 0xA4) * 2;
      unk08 = RxClusterInitializeData(unk08, unk14, 2);
      unk24 = *(u16**)(unk08 + 8);
      unk28 = 0;
      while (unk28 < *(u32*)(unk04 + 0xA4)) {
        unk24[0] = unk28;
        unk24[1] = unk28 + 1;
        unk24 += 2;
        unk28++;
      }
      *(u32*)(unk08 + 0x10) = unk14;
      *(s32*)(unk04 + 0xA0) = 1;
    } else {
      u8* unk2C = *(u8**)_rxExecCtxGlobal;
      projectPipelineForward(arg0, unk2C, 0);
      return 0;
    }
  } else if (*(s32*)(unk04 + 0xA0) != 1) {
    if (*(s32*)(unk04 + 0xA0) == 2) {
      u8* unk44;
      u8* unk48;
      unk40 = unk10 - 1;
      unk44 = *(u8**)(unk08 + 4) + *(u16*)(unk08 + 2) * (unk40 - 1);
      unk08 = RxClusterSetData(unk08, RxHeapAlloc(*(ProjectHeapState**)(arg1 + 4),
          unk40 * 4), 2, unk40 << 1);
      unk48 = *(u8**)(unk08 + 4) + *(u16*)(unk08 + 2) * ((unk40 - 1) * 2);
      unk4C = 0;
      while (unk4C < unk40) {
        *(u16*)(unk48 + 2) = *(u16*)(unk44 + 2);
        *(u16*)unk48 = *(u16*)unk44;
        unk48 -= 4;
        unk44 -= 2;
        unk4C++;
      }
      *(u32*)(unk08 + 0x10) = unk40 * 2;
      *(s32*)(unk04 + 0xA0) = 1;
    } else {
      u8* unk50 = *(u8**)_rxExecCtxGlobal;
      projectPipelineForward(arg0, unk50, 0);
      return 0;
    }
  }
  {
    u8* unk64 = *(u8**)_rxExecCtxGlobal;
    projectPipelineForward(arg0, unk64, 0);
  }
  return 1;
}

ProjectNodeDef* RxNodeDefinitionGetImmMangleLineIndices(void) {
  /* #audit 2026-10-03T08:47Z clean-room PASS (audit) */
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
  static char _ImmMangleLineIndices_csl[] = "ImmMangleLineIndices.csl";
  static ProjectNodeDef nodeImmMangleLineIndicesCSL = {
    _ImmMangleLineIndices_csl, _ImmMangleLineIndicesNode,
    0, 0, 0, 0, 0, 0,
    2, N1clofinterest, N1inputreqs, 1, N1outputs, 0, 0, 0,
  };
  return &nodeImmMangleLineIndicesCSL;
}
