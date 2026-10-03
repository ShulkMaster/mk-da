#ifndef MKDA_RENDERWARE_PROJECT_NODE_H
#define MKDA_RENDERWARE_PROJECT_NODE_H

#include <renderware/project_pipeline.h>
#include <renderware/project_state.h>

s32 RwRenderStateSet(s32 arg0, u32 arg1);
s32 RwIm2DRenderPrimitive(s32 arg0, void* arg1, s32 arg2);
s32 RwIm2DRenderIndexedPrimitive(s32 arg0, void* arg1, s32 arg2, void* arg3, s32 arg4);
u8* _rxEmbeddedPacketBetweenNodes(u8* arg0, u8* arg1, u32 arg2);

#endif
