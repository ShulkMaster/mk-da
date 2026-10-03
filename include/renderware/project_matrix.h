#ifndef MKDA_RENDERWARE_PROJECT_MATRIX_H
#define MKDA_RENDERWARE_PROJECT_MATRIX_H

#include <dolphin/types.h>

typedef struct {
  u32 unk00[16];
} ProjectMatrix;

u8* RwMatrixUpdate(u8* unk00);
u8* RwMatrixMultiply(u8* unk00, const u8* unk04, const u8* unk08);
u8* RwMatrixTransform(u8* unk00, const u8* unk04, s32 unk08);
u8* RwMatrixRotateOneMinusCosineSine(u8* unk00, const u8* unk04,
    f32 unk08, f32 unk0C, s32 unk10);

#endif
