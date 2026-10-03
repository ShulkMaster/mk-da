#include <dolphin/types.h>
#include <renderware/project_image.h>

static void ImageResampleGetSpan(const u8* unk00, s32 unk04, s32 unk08,
    s32 unk0C, f32* unk10) {
  f32 unk14 = (1.0f / 65536.0f) * (f32)(unk08 - unk04);
  f32 unk28;
  f32 unk2C, unk30, unk34, unk38;
  const u8* unk1C = *(u8* const*)(unk00 + 0x14) +
      (unk0C >> 16) * *(const s32*)(unk00 + 0x10) + (unk04 >> 16) * 4;
  s32 unk18 = unk04 >> 16;
  s32 unk20 = unk08 >> 16;
  if (unk18 == unk20) {
    unk10[0] = (1.0f / 255.0f) * (f32)unk1C[0];
    unk10[1] = (1.0f / 255.0f) * (f32)unk1C[1];
    unk10[2] = (1.0f / 255.0f) * (f32)unk1C[2];
    unk10[3] = (1.0f / 255.0f) * (f32)unk1C[3];
    unk10[0] *= unk14;
    unk10[1] *= unk14;
    unk10[2] *= unk14;
    unk10[3] *= unk14;
  } else {
    s32 unk24 = (s32)(65536.0f * (f32)(unk18 + 1));
    unk10[0] = (1.0f / 255.0f) * (f32)unk1C[0];
    unk10[1] = (1.0f / 255.0f) * (f32)unk1C[1];
    unk10[2] = (1.0f / 255.0f) * (f32)unk1C[2];
    unk10[3] = (1.0f / 255.0f) * (f32)unk1C[3];
    unk28 = (1.0f / 65536.0f) * (f32)(unk24 - unk04);
    unk10[0] *= unk28;
    unk10[1] *= unk28;
    unk10[2] *= unk28;
    unk10[3] *= unk28;
    unk1C += 4;
    while ((unk24 >> 16) != unk20) {
      unk2C = (1.0f / 255.0f) * (f32)unk1C[0];
      unk30 = (1.0f / 255.0f) * (f32)unk1C[1];
      unk34 = (1.0f / 255.0f) * (f32)unk1C[2];
      unk38 = (1.0f / 255.0f) * (f32)unk1C[3];
      unk10[0] += unk2C;
      unk10[1] += unk30;
      unk10[2] += unk34;
      unk10[3] += unk38;
      unk24 += 0x10000;
      unk1C += 4;
    }
    {
      unk2C = (1.0f / 255.0f) * (f32)unk1C[0];
      unk30 = (1.0f / 255.0f) * (f32)unk1C[1];
      unk34 = (1.0f / 255.0f) * (f32)unk1C[2];
      unk38 = (1.0f / 255.0f) * (f32)unk1C[3];
      unk28 = (1.0f / 65536.0f) * (f32)(unk08 - unk24);
      unk2C *= unk28;
      unk30 *= unk28;
      unk34 *= unk28;
      unk38 *= unk28;
      unk10[0] += unk2C;
      unk10[1] += unk30;
      unk10[2] += unk34;
      unk10[3] += unk38;
    }
  }
  {
    unk28 = 1.0f / unk14;
    unk10[0] *= unk28;
    unk10[1] *= unk28;
    unk10[2] *= unk28;
    unk10[3] *= unk28;
  }
}

static void ImageResampleGetAvgPixel(const u8* unk00, s32 unk04, s32 unk08,
    s32 unk0C, s32 unk10, f32* unk14) {
  f32 unk18 = (1.0f / 65536.0f) * (f32)(unk10 - unk0C);
  s32 unk1C = unk0C >> 16;
  s32 unk20 = unk10 >> 16;
  f32 unk28;
  f32 unk2C[4];
  if (unk1C == unk20) {
    ImageResampleGetSpan(unk00, unk04, unk08, unk0C, unk14);
    unk14[0] *= unk18;
    unk14[1] *= unk18;
    unk14[2] *= unk18;
    unk14[3] *= unk18;
  } else {
    s32 unk24 = (s32)(65536.0f * (f32)(unk1C + 1));
    ImageResampleGetSpan(unk00, unk04, unk08, unk0C, unk14);
    unk28 = (1.0f / 65536.0f) * (f32)(unk24 - unk0C);
    unk14[0] *= unk28;
    unk14[1] *= unk28;
    unk14[2] *= unk28;
    unk14[3] *= unk28;
    while ((unk24 >> 16) != unk20) {
      ImageResampleGetSpan(unk00, unk04, unk08, unk24, unk2C);
      unk14[0] = unk2C[0] + unk14[0];
      unk14[1] = unk2C[1] + unk14[1];
      unk14[2] = unk2C[2] + unk14[2];
      unk14[3] = unk2C[3] + unk14[3];
      unk24 += 0x10000;
    }
    ImageResampleGetSpan(unk00, unk04, unk08, unk24, unk2C);
    unk28 = (1.0f / 65536.0f) * (f32)(unk10 - unk24);
    unk2C[0] *= unk28;
    unk2C[1] *= unk28;
    unk2C[2] *= unk28;
    unk2C[3] *= unk28;
    unk14[0] += unk2C[0];
    unk14[1] += unk2C[1];
    unk14[2] += unk2C[2];
    unk14[3] += unk2C[3];
  }
  unk28 = 1.0f / unk18;
  unk14[0] *= unk28;
  unk14[1] *= unk28;
  unk14[2] *= unk28;
  unk14[3] *= unk28;
}

/* TODO: [near miss] 94.74%; register allocation and call argument staging remain. */
u8* RwImageResample(u8* unk00, const u8* unk04) {
  s32 unk08 = *(s32*)(unk00 + 4);
  s32 unk0C = *(s32*)(unk00 + 8);
  s32 unk10 = *(const s32*)(unk04 + 4);
  s32 unk14 = *(const s32*)(unk04 + 8);
  s32 unk24;
  s32 unk20;
  s32 unk1C;
  s32 unk18;
  s32 unk3C;
  s32 unk40;
  f32 unk28[4];
  *(u32*)unk00 |= *(const u32*)unk04 & 2;
  unk18 = (s32)(65536.0f * ((f32)unk10 / (f32)unk08));
  unk1C = (s32)(65536.0f * ((f32)unk14 / (f32)unk0C));
  unk20 = 0;
  for (unk24 = 0; unk24 < unk0C; ++unk24) {
    u8* unk38 = *(u8**)(unk00 + 0x14) + *(s32*)(unk00 + 0x10) * unk24;
    unk3C = 0;
    for (unk40 = 0; unk40 < unk08; ++unk40) {
      s32 unk44 = unk18 - 1 + unk3C;
      s32 unk48 = unk1C - 1 + unk20;
      ImageResampleGetAvgPixel(unk04, unk3C, unk44, unk20, unk48, unk28);
      unk38[unk40 * 4] = (u8)(0.5f + 255.0f * unk28[0]);
      unk38[unk40 * 4 + 1] = (u8)(0.5f + 255.0f * unk28[1]);
      unk38[unk40 * 4 + 2] = (u8)(0.5f + 255.0f * unk28[2]);
      unk38[unk40 * 4 + 3] = (u8)(0.5f + 255.0f * unk28[3]);
      unk3C += unk18;
    }
    unk20 += unk1C;
  }
  return unk00;
}

/* TODO: [near miss] 95.28%; inlined register allocation and endpoint staging remain. */
u8* RwImageCreateResample(const u8* unk00, s32 unk04, s32 unk08) {
  u8* unk0C = RwImageCreate(unk04, unk08, 32);
  if (unk0C == 0) {
    return 0;
  }
  if (RwImageAllocatePixels(unk0C) == 0) {
    RwImageDestroy(unk0C);
    return 0;
  }
  if (*(const s32*)(unk00 + 0xC) != 32) {
    u8* unk10 = RwImageCreate(*(const s32*)(unk00 + 4),
        *(const s32*)(unk00 + 8), 32);
    if (unk10 == 0) {
      RwImageFreePixels(unk0C);
      RwImageDestroy(unk0C);
      return 0;
    }
    if (RwImageAllocatePixels(unk10) == 0) {
      RwImageDestroy(unk10);
      RwImageFreePixels(unk0C);
      RwImageDestroy(unk0C);
      return 0;
    }
    RwImageCopy(unk10, unk00);
    if (RwImageResample(unk0C, unk10) == 0) {
      RwImageFreePixels(unk10);
      RwImageDestroy(unk10);
      RwImageFreePixels(unk0C);
      RwImageDestroy(unk0C);
      return 0;
    }
    RwImageFreePixels(unk10);
    RwImageDestroy(unk10);
  } else if (RwImageResample(unk0C, unk00) == 0) {
    RwImageFreePixels(unk0C);
    RwImageDestroy(unk0C);
    return 0;
  }
  return unk0C;
}
