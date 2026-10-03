#ifndef MKDA_RENDERWARE_PROJECT_CLIP_H
#define MKDA_RENDERWARE_PROJECT_CLIP_H

#include <dolphin/types.h>

extern f32 _rwClipInfoGlobal[10];

typedef f32 (*ProjectClipFn)(u8*, u8*, u8*, u8*, u8*, u8*);
u32 _rwForAllEdgesWithRxInterpolant(u8*, u8*, u8*, u16*, s32*, s32*, u32, ProjectClipFn);
u32 _rwForAllEdgesWithoutRxInterpolant(u8*, u8*, u8*, u16*, s32*, s32*, u32, ProjectClipFn);
f32 _rwGeneratePerspClippedVertexZLO(u8*, u8*, u8*, u8*, u8*, u8*);

#endif
