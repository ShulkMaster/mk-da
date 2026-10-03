/* #audit 2026-10-03T05:46Z clean-room PASS (audit) */
#include <string.h>
#include <renderware/project_pipeline.h>

extern void _rxPacketDestroy(u8* arg0);

static inline u8* projectStashAcquireWord(u8* unk00, u32 unk04) {
  s32 unk08 = *(s32*)(*(u8**)(unk00 + 8) + unk04);
  if (unk08 == -1) {
    return 0;
  }
  *(u32*)(unk00 + unk08 * 0x1C + 0x1C) =
      *(u32*)(unk00 + unk08 * 0x1C + 0x18);
  return unk00 + *(s32*)(*(u8**)(unk00 + 8) + unk04) * 0x1C + 0x14;
}

static s32 PL2ImmStashNodeBody(const void* unk00, u8* const* unk04) {
  u8* unk08 = *unk04 + 0xC;
  u8* unk0C;
  u8* unk10;
  if (*(s32*)(*(u8**)_rxExecCtxGlobal + 0x10) == 3) {
    *(s32*)(*(u8**)_rxExecCtxGlobal + 0x10) = 2;
    unk0C = *(u8**)(*(u8**)_rxExecCtxGlobal + 0x14);
  } else {
    unk0C = 0;
  }
  memset(unk08, 0, 0x30);
  unk10 = projectStashAcquireWord(unk0C, 0);
  *(u32*)(unk08 + 0xC) = *(u32*)(unk10 + 8);
  *(u16*)unk10 |= 2;
  unk10 = projectStashAcquireWord(unk0C, 4);
  *(u32*)(unk08 + 0x10) = *(u32*)(unk10 + 8);
  *(u16*)unk10 |= 2;
  unk10 = projectStashAcquireWord(unk0C, 8);
  *(u32*)(unk08 + 0x14) = *(u32*)(unk10 + 8);
  *(u16*)unk10 |= 2;
  unk10 = projectStashAcquireWord(unk0C, 0xC);
  *(u8**)(unk08 + 0x18) = *(u8**)(unk10 + 8);
  *(u32*)(unk08 + 8) = *(u32*)(*(u8**)(unk08 + 0x18) + 0xA8);
  *(u16*)unk10 |= 2;
  unk10 = projectStashAcquireWord(unk0C, 0x10);
  *(u32*)(unk08 + 0x1C) = *(u32*)(unk10 + 8);
  *(u16*)unk10 |= 2;
  _rxPacketDestroy(unk0C);
  return 1;
}

#include <renderware/project_node_defs.h>

ProjectNodeDef* RxNodeDefinitionGetImmStash(void) {
  static ProjectNodeCluster nodeClusters[5] = {
    {&RxClObjSpace3DVertices, 0, 0},
    {&RxClCamSpace3DVertices, 0, 0},
    {&RxClScrSpace2DVertices, 0, 0},
    {&RxClMeshState, 0, 0},
    {&RxClRenderState, 0, 0},
  };
  static u32 nodeReqs[5] = {2, 2, 2, 2, 2};
  static char _ImmStash_csl[] = "ImmStash.csl";
  static ProjectNodeDef nodeImmStashCSL = {
    _ImmStash_csl, PL2ImmStashNodeBody,
    0, 0, 0, 0, 0, 0,
    5, nodeClusters, nodeReqs, 0, 0, 0, 0, 0,
  };
  return &nodeImmStashCSL;
}
