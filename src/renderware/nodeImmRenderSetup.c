/* #audit 2026-10-03T05:48Z clean-room PASS (audit) */
#include <renderware/project_pipeline.h>
#include <renderware/project_renderstate.h>

extern ProjectRenderState* RxRenderStateVectorCreate(s32 arg0);
extern u8* RxPacketCreate(const u8* arg0);
extern u8* RxClusterLockWrite(u8* arg0, u32 arg1, u8* arg2);
extern u8* RxClusterSetExternalData(u8* arg0, void* arg1, u32 arg2, u32 arg3);
extern u8* RxClusterInitializeData(u8* arg0, u32 arg1, u16 arg2);
extern u8* _rxEmbeddedPacketBetweenNodes(u8* arg0, u8* arg1, u32 arg2);

static inline void projectSetupForward(u8* unk00, u32 unk04) {
  u8* unk08 = *(u8**)_rxExecCtxGlobal;
  if (*(s32*)(_rxExecCtxGlobal + 8) != 0) {
    u8* unk0C = _rxEmbeddedPacketBetweenNodes(unk08, unk00, unk04);
    if (unk0C != 0) {
      u32 unk10 = (*(s32 (**)(u8*, void*))
          (*(u8**)unk0C + 4))(unk0C, _rxExecCtxGlobal + 0x10);
      if (unk10 == 0) {
        *(u32*)(_rxExecCtxGlobal + 8) = unk10;
      }
    }
  }
  if (*(s32*)(unk08 + 0x10) > 1) {
    *(s32*)(unk08 + 0x10) = 2;
    _rxPacketDestroy(*(u8**)(unk08 + 0x14));
  }
}

static s32 _ImmRenderSetupNode(u8* unk00, u8* const* unk04) {
  u8* unk08 = *unk04;
  ProjectRenderState* unk0C;
  u8* unk10;
  u8* unk14;
  u32 unk18;
  if (*(void**)(unk08 + 0xC) == 0) {
    return 1;
  }
  unk0C = RxRenderStateVectorCreate(1);
  unk10 = RxPacketCreate(unk00);
  unk14 = RxClusterLockWrite(unk10, 0, unk00);
  if (unk14 != 0) {
    RxClusterSetExternalData(unk14, *(void**)(unk08 + 0xC), 0x24, *(u32*)(unk08 + 8));
  }
  if (*(void**)(unk08 + 0x10) != 0) {
    unk14 = RxClusterLockWrite(unk10, 1, unk00);
    RxClusterSetExternalData(unk14, *(void**)(unk08 + 0x10), 0x28, *(u32*)(unk08 + 8));
  }
  if (*(void**)(unk08 + 0x14) != 0) {
    unk14 = RxClusterLockWrite(unk10, 2, unk00);
    RxClusterSetExternalData(unk14, *(void**)(unk08 + 0x14), 0x1C, *(u32*)(unk08 + 8));
  }
  unk14 = RxClusterLockWrite(unk10, 3, unk00);
  RxClusterSetExternalData(unk14, *(void**)(unk08 + 0x18), 0xBC, 1);
  switch (*(s32*)(unk08 + 0x24)) {
  case 4:
    unk18 = *(u32*)(unk08 + 0x2C) - 2;
    break;
  case 3:
    unk18 = *(u32*)(unk08 + 0x2C) / 3;
    break;
  case 6:
    unk18 = *(u32*)(unk08 + 0x2C);
    break;
  case 1:
    unk18 = *(u32*)(unk08 + 0x2C) >> 1;
    break;
  case 2:
    unk18 = *(u32*)(unk08 + 0x2C) - 1;
    break;
  case 5:
    unk18 = *(u32*)(unk08 + 0x2C) - 2;
    break;
  default:
    return 0;
  }
  *(u32*)(*(u8**)(unk08 + 0x18) + 0xA4) = unk18;
  *(s32*)(*(u8**)(unk08 + 0x18) + 0xA0) = *(s32*)(unk08 + 0x24);
  unk14 = RxClusterLockWrite(unk10, 4, unk00);
  if (*(void**)(unk08 + 0x1C) != 0) {
    s32 unk1C = 0;
    RxClusterSetExternalData(unk14, *(void**)(unk08 + 0x1C), 0x30, 1);
    if ((**(u32**)(unk08 + 0x1C) & 8) != 0) {
      unk1C = 1;
    }
    **(ProjectRenderState**)(unk08 + 0x1C) = *unk0C;
    if (unk1C == 0) {
      **(u32**)(unk08 + 0x1C) &= ~8;
    }
  } else {
    unk14 = RxClusterInitializeData(unk14, 1, 0x30);
    **(ProjectRenderState**)(unk14 + 8) = *unk0C;
    ++*(u32*)(unk14 + 0x10);
  }
  RxRenderStateVectorDestroy(unk0C);
  if (*(void**)(unk08 + 0x28) != 0) {
    unk14 = RxClusterLockWrite(unk10, 5, unk00);
    RxClusterSetExternalData(unk14, *(void**)(unk08 + 0x28), 2, *(u32*)(unk08 + 0x2C));
    projectSetupForward(unk00, 0);
  } else {
    projectSetupForward(unk00, 1);
  }
  return 1;
}

#include <renderware/project_node_defs.h>

extern ProjectClusterDef RxClIndices;

void* RxNodeDefinitionGetImmRenderSetup(void) {
  static ProjectNodeCluster N1clofinterest[6] = {
    {&RxClObjSpace3DVertices, 0, 0},
    {&RxClCamSpace3DVertices, 0, 0},
    {&RxClScrSpace2DVertices, 0, 0},
    {&RxClMeshState, 0, 0},
    {&RxClRenderState, 0, 0},
    {&RxClIndices, 0, 0}
  };
  static u32 N1inputreqs[6] = {0, 0, 0, 0, 0, 0};
  static u32 N1outcl1[6] = {1, 1, 1, 1, 1, 1};
  static u32 N1outcl2[6] = {1, 1, 1, 1, 1, 2};
  static char _ImmRenderSetupOut[] = "ImmRenderSetupOut";
  static char _ImmRenderSetupOutUnindexed[] = "ImmRenderSetupOutUnindexed";
  static struct {
    char* unk00;
    u32* unk04;
    u32 unk08;
  } N1outputs[2] = {
    {_ImmRenderSetupOut, N1outcl1, 2},
    {_ImmRenderSetupOutUnindexed, N1outcl2, 2}
  };
  static char _ImmRenderSetup_csl[] = "ImmRenderSetup.csl";
  static struct {
    char* unk00;
    s32 (*unk04)(u8*, u8* const*);
    u32 unk08[6];
    u32 unk20;
    ProjectNodeCluster* unk24;
    u32* unk28;
    u32 unk2C;
    void* unk30;
    u32 unk34;
    u32 unk38;
    u32 unk3C;
  } nodeImmRenderSetupCSL = {
    _ImmRenderSetup_csl, _ImmRenderSetupNode, {0, 0, 0, 0, 0, 0},
    6, N1clofinterest, N1inputreqs, 2, N1outputs, 0, 0, 0
  };
  return &nodeImmRenderSetupCSL;
}
