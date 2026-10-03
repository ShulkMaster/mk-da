#ifndef MKDA_RENDERWARE_PROJECT_RASTER_H
#define MKDA_RENDERWARE_PROJECT_RASTER_H

#include <dolphin/types.h>

u8* RwRasterCreate(s32 unk00, s32 unk04, s32 unk08, s32 unk0C);
u8* RwRasterLock(u8* unk00, s32 unk04, s32 unk08);
u8* RwRasterUnlock(u8* unk00);
s32 RwRasterDestroy(u8* unk00);

#endif
