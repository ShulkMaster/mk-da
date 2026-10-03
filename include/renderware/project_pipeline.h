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

#endif
