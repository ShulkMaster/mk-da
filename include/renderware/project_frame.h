#ifndef MKDA_RENDERWARE_PROJECT_FRAME_H
#define MKDA_RENDERWARE_PROJECT_FRAME_H

#include <dolphin/types.h>

u8* RwFrameForAllObjects(u8* unk00, u8* (*unk04)(u8*, void*), void* unk08);

void _rwFrameSyncHierarchyLTM(u8* unk00);
u8* RwFrameRemoveChild(u8* unk00);

#endif
