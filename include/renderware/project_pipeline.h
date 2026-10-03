#ifndef MKDA_RENDERWARE_PROJECT_PIPELINE_H
#define MKDA_RENDERWARE_PROJECT_PIPELINE_H

#include <dolphin/types.h>

extern s32 _rxPipelineGlobalsOffset;
extern u8 _rxExecCtxGlobal[0x18];
s32 _rxPipelineOpen(void);
s32 _rxPipelineClose(void);
void _rxPacketDestroy(u8* unk00);
void _rxEmbeddedPacketBetweenPipelines(u8* arg0, u8* arg1);
u8* _rxEmbeddedPacketBetweenNodes(u8* unk00, u8* unk04, u32 unk08);
u8* RxClusterInitializeData(u8* unk00, u32 unk04, u16 unk08);
u8* RxClusterLockWrite(u8* unk00, u32 unk04, u8* unk08);
u8* RxClusterSetData(u8* unk00, void* unk04, u16 unk08, u32 unk0C);

u32 _rxChaseDependencies(u8* unk00);

typedef struct { u32 unk00[3]; } ProjectSortState;
typedef struct {
  u32 unk00;
  u32 unk04;
  u32* unk08;
  u32 unk0C;
  u32 unk10;
  u32 unk14;
  u32 unk18;
  ProjectSortState* unk1C;
  u32 unk20;
  u32 unk24;
} ProjectSortNode;

u8* PipelineNodeDestroy(u8* unk00, u8* unk04);

u8* RxLockedPipeAddPath(u8* unk00, u32* unk04, u8* unk08);

static inline void projectPipelineForward(u8* unk00, u8* unk04, u32 unk18) {
  if (*(s32*)(_rxExecCtxGlobal + 8) != 0) {
    u8* unk08 = _rxEmbeddedPacketBetweenNodes(unk04, unk00, unk18);
    if (unk08 != 0) {
      u8* unk0C = _rxExecCtxGlobal + 0x10;
      u8* unk10 = *(u8**)unk08;
      s32 unk14 = (*(s32 (**)(u8*, u8*))(unk10 + 4))(unk08, unk0C);
      if (unk14 == 0U) {
        *(u32*)(_rxExecCtxGlobal + 8) = unk14;
      }
    }
  }
  if (*(s32*)(unk04 + 0x10) > 1) {
    *(s32*)(unk04 + 0x10) = 2;
    _rxPacketDestroy(*(u8**)(unk04 + 0x14));
  }
}

#endif
