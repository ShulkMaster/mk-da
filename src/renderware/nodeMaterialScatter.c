/* #audit 2026-10-03T05:37Z clean-room PASS (audit) */
#include <renderware/project_pipeline.h>
#include <renderware/project_state.h>

extern void _rxEmbeddedPacketBetweenPipelines(u8* arg0, u8* arg1);
extern void _rxPacketDestroy(u8* arg0);

static u32 _MaterialScatterNode(u8* arg0, void* arg1) {
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
    unk04 = (u8*)(**(s32**)(unk00 + 8) * 0x1C + 0x14);
    unk04 = (u8*)((u32)unk00 + (u32)unk04);
  }
  unk0C = *(u8**)(*(u8**)(unk04 + 8) + 0x9C);
  if (unk0C != 0) {
    if (*(s32*)(_rxExecCtxGlobal + 8) != 0) {
      u8* unk10 = *(u8**)_rxExecCtxGlobal;
      _rxEmbeddedPacketBetweenPipelines(unk10, unk0C);
      *(u8**)_rxExecCtxGlobal = unk0C;
      {
        u8* unk14 = *(u8**)(unk0C + 8);
        void* unk34 = _rxExecCtxGlobal + 0x10;
        u8* unk18 = *(u8**)unk14;
        u32 unk1C = (*(u32 (**)(u8*, void*))(unk18 + 4))(unk14, unk34);
        if (unk1C == 0) {
          *(u32*)(_rxExecCtxGlobal + 8) = unk1C;
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
        void* unk38 = _rxExecCtxGlobal + 0x10;
        u8* unk2C = *(u8**)unk28;
        u32 unk30 = (*(u32 (**)(u8*, void*))(unk2C + 4))(unk28, unk38);
        if (unk30 == 0) {
          *(u32*)(_rxExecCtxGlobal + 8) = unk30;
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

extern u8 RxClMeshState[0x10];
extern u8 RxClObjSpace3DVertices[0x10];
extern u8 RxClIndices[0x10];
extern u8 RxClRenderState[0x10];
extern u8 RxClLights[0x10];

void* RxNodeDefinitionGetMaterialScatter(void) {
  static struct {
    u8* unk00;
    u32 unk04;
    u32 unk08;
  } N2clofinterest[5] = {
    {RxClMeshState, 0, 0},
    {RxClObjSpace3DVertices, 0, 0},
    {RxClIndices, 0, 0},
    {RxClRenderState, 0, 0},
    {RxClLights, 0, 0}
  };
  static u32 N2inputreqs[5] = {1, 2, 2, 2, 2};
  static char _MaterialScatter_csl[] = "MaterialScatter.csl";
  static struct {
    char* unk00;
    u32 (*unk04)(u8*, void*);
    u32 unk08[6];
    u32 unk20;
    void* unk24;
    u32* unk28;
    u32 unk2C[5];
  } nodeMaterialScatterCSL = {
    _MaterialScatter_csl, _MaterialScatterNode, {0, 0, 0, 0, 0, 0},
    5, N2clofinterest, N2inputreqs, {0, 0, 0, 0, 0}
  };
  return &nodeMaterialScatterCSL;
}
