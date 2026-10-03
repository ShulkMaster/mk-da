#ifndef MKDA_RENDERWARE_PROJECT_PIPELINE_H
#define MKDA_RENDERWARE_PROJECT_PIPELINE_H

#include <dolphin/types.h>

extern s32 _rxPipelineGlobalsOffset;
extern u8 _rxExecCtxGlobal[0x18];
s32 _rxPipelineOpen(void);
s32 _rxPipelineClose(void);
void _rxPacketDestroy(u8* unk00);

#endif
