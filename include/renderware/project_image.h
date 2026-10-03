#ifndef MKDA_RENDERWARE_PROJECT_IMAGE_H
#define MKDA_RENDERWARE_PROJECT_IMAGE_H

#include <dolphin/types.h>

u8* RwImageCreate(s32 unk00, s32 unk04, s32 unk08);
s32 RwImageDestroy(u8* unk00);
u8* RwImageAllocatePixels(u8* unk00);
u8* RwImageFreePixels(u8* unk00);
u8* RwImageCopy(u8* unk00, const u8* unk04);

#endif
