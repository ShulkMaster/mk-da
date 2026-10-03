#ifndef MKDA_RENDERWARE_PROJECT_MEMORY_H
#define MKDA_RENDERWARE_PROJECT_MEMORY_H

#include <dolphin/types.h>

u8* RwFreeListCreate(s32 unk00, s32 unk04, s32 unk08);
s32 RwFreeListDestroy(void* unk00);

#endif
