#include <dolphin/types.h>
#include <renderware/project_state.h>
#include <renderware/project_memory.h>
#include <renderware/project_registry.h>
#include <renderware/project_error.h>
#include <renderware/project_image.h>
#include <string.h>

typedef struct {
  u8* unk00;
  u8* unk04;
  s32 unk08;
  u8 unk0C[0x204];
  u8* unk210;
  s32 unk214;
  u8* unk218;
  u8* unk21C;
} ProjectImageModule;

static u32 imageTKList[6] = {28, 28, 0, 0, 0, 0};
static s32 imageModule[2];

s32 RwImageSetGamma(f32 unk00);

u8* _rwImageOpen(u8* unk00, s32 unk04, s32 unk08) {
  imageModule[0] = unk04;
  ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk00 = RwFreeListCreate(imageTKList[0], 20, 0);
  if (((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk00 == 0) {
    return 0;
  }
  ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk218 = RwFreeListCreate(52, 3, 0);
  if (((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk218 == 0) {
    RwFreeListDestroy(((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk00);
    ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk00 = 0;
    return 0;
  }
  ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk08 = 256;
  ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk04 =
      (*(u8* (**)(s32))(RwEngineInstance + 0x130))(
          ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk08);
  if (((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk04 == 0) {
    RwFreeListDestroy(((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk218);
    ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk218 = 0;
    RwFreeListDestroy(((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk00);
    ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk00 = 0;
    return 0;
  }
  *((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk04 = 0;
  ++imageModule[1];
  RwImageSetGamma(1.0f);
  ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk21C = 0;
  ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk214 = 256;
  ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk210 =
      (*(u8* (**)(s32))(RwEngineInstance + 0x130))(
          ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk214);
  if (((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk210 == 0) {
    (*(void (**)(u8*))(RwEngineInstance + 0x134))(
        ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk04);
    ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk04 = 0;
    ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk08 = 0;
    RwFreeListDestroy(((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk218);
    ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk218 = 0;
    RwFreeListDestroy(((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk00);
    ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk00 = 0;
    return 0;
  }
  return unk00;
}

u8* _rwImageClose(u8* unk00, s32 unk0C, s32 unk10) {
  u8* unk04;
  u8* unk08;
  if (*(void**)(RwEngineInstance + imageModule[0] + 0x210) != 0) {
    (*(void (**)(void*))(RwEngineInstance + 0x134))(
        *(void**)(RwEngineInstance + imageModule[0] + 0x210));
    *(void**)(RwEngineInstance + imageModule[0] + 0x210) = 0;
    *(void**)(RwEngineInstance + imageModule[0] + 0x214) = 0;
  }
  if (*(void**)(RwEngineInstance + imageModule[0] + 4) != 0) {
    (*(void (**)(void*))(RwEngineInstance + 0x134))(
        *(void**)(RwEngineInstance + imageModule[0] + 4));
    *(void**)(RwEngineInstance + imageModule[0] + 4) = 0;
    *(void**)(RwEngineInstance + imageModule[0] + 8) = 0;
  }
  while ((unk08 = *(u8**)((unk04 = RwEngineInstance + imageModule[0]) + 0x21C)) != 0) {
    *(u8**)(unk04 + 0x21C) = *(u8**)(unk08 + 0x30);
    (*(void* (**)(u8*, void*))(RwEngineInstance + 0x144))(
        *(u8**)(RwEngineInstance + imageModule[0] + 0x218), unk08);
  }
  if (*(void**)(unk04 + 0x218) != 0) {
    RwFreeListDestroy(*(void**)(unk04 + 0x218));
    *(void**)(RwEngineInstance + imageModule[0] + 0x218) = 0;
  }
  if (*(void**)(RwEngineInstance + imageModule[0]) != 0) {
    RwFreeListDestroy(*(void**)(RwEngineInstance + imageModule[0]));
    *(void**)(RwEngineInstance + imageModule[0]) = 0;
  }
  --imageModule[1];
  return unk00;
}

u8* RwImageCreate(s32 unk00, s32 unk04, s32 unk08) {
  u8* unk0C = (*(u8* (**)(void*))(RwEngineInstance + 0x140))(
      *(void**)(RwEngineInstance + imageModule[0]));
  if (unk0C == 0) {
    return 0;
  }
  *(s32*)(unk0C + 0x04) = unk00;
  *(s32*)(unk0C + 0x08) = unk04;
  *(s32*)(unk0C + 0x0C) = unk08;
  *(void**)(unk0C + 0x14) = 0;
  *(void**)(unk0C + 0x18) = 0;
  *(u32*)(unk0C + 0x00) = 0;
  _rwPluginRegistryInitObject((u8*)imageTKList, unk0C);
  return unk0C;
}

s32 RwImageDestroy(u8* unk00) {
  if (*(u32*)unk00 & 1) {
    RwImageFreePixels(unk00);
  }
  _rwPluginRegistryDeInitObject((u8*)imageTKList, unk00);
  (*(void* (**)(u8*, void*))(RwEngineInstance + 0x144))(
      *(u8**)(RwEngineInstance + imageModule[0]), unk00);
  return 1;
}

#include <renderware/project_error.h>
#include <renderware/project_state.h>

u8* RwImageAllocatePixels(u8* image) {
  s32 paletteBytes;
  s32 depth = *(s32*)(image + 0xC);
  s32 hasPalette = 0;
  s32 pixelBytes;
  s32 allocationBytes;
  if (depth == 4U || depth == 8U) {
    hasPalette = 1;
  }
  paletteBytes = hasPalette ? (1 << depth) * 4 : 0;
  *(s32*)(image + 0x10) = (*(s32*)(image + 0xC) + 7) >> 3;
  *(s32*)(image + 0x10) *= *(s32*)(image + 4);
  *(s32*)(image + 0x10) = (*(s32*)(image + 0x10) + 3) & ~3;
  pixelBytes = *(s32*)(image + 0x10) * *(s32*)(image + 8);
  allocationBytes = pixelBytes + paletteBytes;
  *(u8**)(image + 0x14) =
      (*(u8* (**)(u32))(RwEngineInstance + 0x130))(allocationBytes);
  if (*(u8**)(image + 0x14) == 0) {
    BaerrState error;
    error.unk00 = 1;
    error.unk04 = _rwerror(0x80000013, allocationBytes);
    RwErrorSet(&error);
    return 0;
  }
  *(u8**)(image + 0x18) =
      hasPalette ? *(u8**)(image + 0x14) + pixelBytes : 0;
  *(u32*)image |= 1;
  return image;
}

#include <renderware/project_state.h>

u8* RwImageFreePixels(u8* arg0) {
  void* unk14 = *(void**)(arg0 + 0x14);
  (*(void (**)(void*))(RwEngineInstance + 0x134))(unk14);
  *(void**)(arg0 + 0x14) = 0;
  *(void**)(arg0 + 0x18) = 0;
  *(u32*)arg0 &= ~1U;
  return arg0;
}

u8* RwImageMakeMask(u8* unk00) {
  s32 unk04 = *(s32*)(unk00 + 0xC);
  switch (unk04) {
  case 4:
  case 8: {
    s32 unk28;
    s32 unk10;
    s32 unk08 = 1 << unk04;
    u8* unk0C = *(u8**)(unk00 + 0x18);
    for (unk10 = 0; unk10 < unk08; ++unk10) {
      s32 unk14 = unk0C[unk10 * 4];
      unk28 = unk0C[unk10 * 4 + 1];
      if (unk28 > unk14) {
        unk14 = unk28;
      }
      unk28 = unk0C[unk10 * 4 + 2];
      if (unk28 > unk14) {
        unk14 = unk28;
      }
      unk0C[unk10 * 4 + 3] = (u8)unk14;
    }
    break;
  }
  case 32: {
    s32 unk2C;
    s32 unk1C;
    u8* unk18 = *(u8**)(unk00 + 0x14);
    s32 unk20;
    for (unk1C = 0; unk1C < *(s32*)(unk00 + 8); ++unk1C) {
      for (unk20 = 0; unk20 < *(s32*)(unk00 + 4); ++unk20) {
        s32 unk24 = unk18[unk20 * 4];
        unk2C = unk18[unk20 * 4 + 1];
        if (unk2C > unk24) {
          unk24 = unk2C;
        }
        unk2C = unk18[unk20 * 4 + 2];
        if (unk2C > unk24) {
          unk24 = unk2C;
        }
        unk18[unk20 * 4 + 3] = (u8)unk24;
      }
      unk18 += *(s32*)(unk00 + 0x10);
    }
    break;
  }
  }
  return unk00;
}

/* TODO: [near miss] 99.09%; register allocation remains. */
u8* RwImageApplyMask(u8* unk00, const u8* unk04) {
  s32 unk0C;
  s32 unk08 = *(s32*)(unk00 + 4);
  if (unk08 != *(const s32*)(unk04 + 4) ||
      (unk0C = *(s32*)(unk00 + 8), unk0C != *(const s32*)(unk04 + 8))) {
    BaerrState unk10;
    unk10.unk00 = 1;
    unk10.unk04 = _rwerror(0x8000000A);
    RwErrorSet(&unk10);
    return 0;
  }
  switch (*(s32*)(unk00 + 0xC)) {
  case 4:
  case 8: {
    u8* unk14;
    if ((unk14 = RwImageCreate(unk08, unk0C, *(s32*)(unk00 + 0xC))) == 0) {
      return 0;
    }
    if (RwImageAllocatePixels(unk14) == 0) {
      RwImageDestroy(unk14);
      return 0;
    }
    RwImageCopy(unk14, unk00);
    if (*(u32*)unk00 & 1) {
      RwImageFreePixels(unk00);
    }
    *(s32*)(unk00 + 0xC) = 32;
    RwImageAllocatePixels(unk00);
    RwImageCopy(unk00, unk14);
    RwImageFreePixels(unk14);
    RwImageDestroy(unk14);
  }
  case 32: {
    s32 unk24;
    s32 unk30;
    const u8* unk18 = *(const u8**)(unk04 + 0x14);
    const u8* unk1C = *(const u8**)(unk04 + 0x18);
    u8* unk20 = *(u8**)(unk00 + 0x14);
    for (unk24 = 0; unk24 < *(s32*)(unk00 + 8); ++unk24) {
      u8* unk28 = unk20;
      switch (*(const s32*)(unk04 + 0xC)) {
      case 4:
      case 8: {
        const u8* unk2C = unk18;
        for (unk30 = 0; unk30 < *(s32*)(unk00 + 4); ++unk30) {
          unk28[3] = unk1C[*unk2C * 4 + 3];
          ++unk2C;
          unk28 += 4;
        }
        break;
      }
      case 32: {
        const u8* unk34 = unk18;
        for (unk30 = 0; unk30 < *(s32*)(unk00 + 4); ++unk30) {
          unk28[3] = unk34[3];
          unk34 += 4;
          unk28 += 4;
        }
        break;
      }
      }
      unk20 += *(s32*)(unk00 + 0x10);
      unk18 += *(const s32*)(unk04 + 0x10);
    }
    break;
  }
  default: {
    BaerrState unk3C;
    unk3C.unk00 = 1;
    unk3C.unk04 = _rwerror(0x80000009);
    RwErrorSet(&unk3C);
    return 0;
  }
  }
  return unk00;
}

u8* RwImageSetPath(u8* unk00) {
  s32 unk04 = (*(s32 (**)(const u8*))(RwEngineInstance + 0x11C))(unk00) + 1;
  if (unk04 > ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk08) {
    u8* unk08 = (*(u8* (**)(u8*, s32))(RwEngineInstance + 0x138))(
        ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk04, unk04);
    if (unk08 == 0) {
      BaerrState unk0C;
      unk0C.unk00 = 1;
      unk0C.unk04 = _rwerror((s32)0x80000013, unk04);
      RwErrorSet(&unk0C);
      return 0;
    }
    ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk04 = unk08;
    ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk08 = unk04;
  }
  memcpy(((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk04, unk00, unk04);
  return unk00;
}

static inline char* projectImagePathBuffer(s32 unk00) {
  if (unk00 > ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk214) {
    char* unk04;
    if (((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk210 != 0) {
      unk04 = (*(char* (**)(void*, s32))(RwEngineInstance + 0x138))(
          ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk210, unk00);
    } else {
      unk04 = (*(char* (**)(s32))(RwEngineInstance + 0x130))(unk00);
    }
    if (unk04 == 0) {
      BaerrState unk08;
      unk08.unk00 = 1;
      unk08.unk04 = _rwerror(0x80000013, unk00);
      RwErrorSet(&unk08);
      return 0;
    }
    ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk210 = (u8*)unk04;
    ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk214 = unk00;
  }
  return (char*)((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk210;
}

s32 _rwpathisabsolute(const char* unk00);

static u8* ImagePathForAllFullNames(u8* unk00, s32 unk04,
    u8* (*unk08)(u8*, u8*), u8* unk0C) {
  s32 unk2C;
  s32 unk30;
  s32 unk14;
  char* unk10 = (char*)((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk04;
  if (_rwpathisabsolute((const char*)unk00) || unk10 == 0 || *unk10 == 0) {
    char* unk18;
    unk14 = unk04 + (*(s32 (**)(const char*))(RwEngineInstance + 0x11C))((const char*)unk00);
    unk18 = projectImagePathBuffer(unk14);
    if (unk18 == 0) {
      return 0;
    }
    (*(char* (**)(char*, const char*))(RwEngineInstance + 0xF8))(unk18, (const char*)unk00);
    unk08((u8*)unk18, unk0C);
  } else {
    while (unk10 != 0 && *unk10 != 0) {
      char* unk28;
      char* unk34;
      (*(char* (**)(const char*, s32))(RwEngineInstance + 0x10C))(unk10, 0x3B);
      unk28 = (*(char* (**)(const char*, s32))(RwEngineInstance + 0x10C))(unk10, 0x3B);
      if (unk28 != 0) {
        unk2C = unk28 - unk10;
        ++unk28;
      } else {
        unk2C = (*(s32 (**)(const char*))(RwEngineInstance + 0x11C))(unk10);
      }
      unk30 = (*(s32 (**)(const char*))(RwEngineInstance + 0x11C))((const char*)unk00);
      unk30 += unk04;
      unk30 = unk2C + unk30;
      unk34 = projectImagePathBuffer(unk30);
      if (unk34 == 0) {
        return 0;
      }
      memcpy(unk34, unk10, unk2C);
      (*(char* (**)(char*, const char*))(RwEngineInstance + 0xF8))(unk34 + unk2C, (const char*)unk00);
      if (unk08((u8*)unk34, unk0C) == 0) {
        return unk00;
      }
      unk10 = unk28;
    }
  }
  return unk00;
}

static u8* ImageAttempRead(u8* unk00, u8* unk04) {
  if ((*(s32 (**)(u8*))(RwEngineInstance + 0xC4))(unk00) != 0) {
    *(u8**)(unk04 + 0x04) = (*(u8* (**)(u8*))unk04)(unk00);
    if (*(u8**)(unk04 + 0x04) != 0) {
      return 0;
    }
  }
  return unk00;
}

#include <stddef.h>

static u8* ImageDetermineExtender(u8* unk00, u8* unk04) {
  u8* entry;
  u8* end = unk00 + (*(size_t (**)(const char*))(RwEngineInstance + 0x11C))((const char*)unk00);
  entry = *(u8**)(RwEngineInstance + imageModule[0] + 0x21C);
  while (entry != 0) {
    (*(char* (**)(char*, const char*))(RwEngineInstance + 0xF8))(
        (char*)end, (const char*)entry);
    if ((*(s32 (**)(const char*))(RwEngineInstance + 0xC4))((const char*)unk00) != 0) {
      *(u8**)unk04 = entry;
      return 0;
    }
    (*(char* (**)(char*, const char*))(RwEngineInstance + 0xF8))(
        (char*)end, (const char*)entry + 0x14);
    if ((*(s32 (**)(const char*))(RwEngineInstance + 0xC4))((const char*)unk00) != 0) {
      *(u8**)unk04 = entry + 0x14;
      return 0;
    }
    entry = *(u8**)(entry + 0x30);
  }
  return unk00;
}

u8* RwImageFindFileType(u8* unk00) {
  u8* unk04 = 0;
  ImagePathForAllFullNames(unk00, 0x14, ImageDetermineExtender, (u8*)&unk04);
  return unk04;
}

s32 RwImageRegisterImageFormat(const u8* unk00, void* unk04, void* unk08) {
  u8* unk14;
  u8* unk10;
  u8 unk0C[20];
  unk0C[0] = '.';
  (*(u8* (**)(u8*, const u8*, s32))(RwEngineInstance + 0xFC))(unk0C + 1, unk00, 18);
  unk0C[19] = 0;
  (*(void (**)(u8*))(RwEngineInstance + 0x124))(unk0C);
  unk10 = ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk21C;
  unk14 = 0;
  while (unk10 != 0 && unk14 == 0) {
    if ((*(s32 (**)(const u8*, const u8*))(RwEngineInstance + 0x114))(unk10, unk0C) == 0) {
      unk14 = unk10;
    } else {
      unk10 = *(u8**)(unk10 + 0x30);
    }
  }
  if (unk14 == 0) {
    unk14 = (*(u8* (**)(u8*))(RwEngineInstance + 0x140))(
        ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk218);
    if (unk14 == 0) {
      return 0;
    }
    memcpy(unk14, unk0C, 20);
    unk14[0x14] = '.';
    (*(u8* (**)(u8*, const u8*, s32))(RwEngineInstance + 0xFC))(unk14 + 0x15, unk00, 18);
    unk14[0x27] = 0;
    (*(void (**)(u8*))(RwEngineInstance + 0x120))(unk14 + 0x14);
    *(void**)(unk14 + 0x28) = 0;
    *(void**)(unk14 + 0x2C) = 0;
    *(u8**)(unk14 + 0x30) = ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk21C;
    ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk21C = unk14;
  }
  if (unk04 != 0) {
    *(void**)(unk14 + 0x28) = unk04;
  }
  if (unk08 != 0) {
    *(void**)(unk14 + 0x2C) = unk08;
  }
  return 1;
}

u8* RwImageWrite(u8* arg0, const char* arg1) {
  char* unk00 = (*(char* (**)(const char*, s32))(RwEngineInstance + 0x108))(arg1, '.');
  if (unk00 != 0) {
    u8* unk04 = ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk21C;
    while (unk04 != 0) {
      if ((*(s32 (**)(const char*, const char*))(RwEngineInstance + 0x114))(
              (const char*)unk04, unk00) == 0 ||
          (*(s32 (**)(const char*, const char*))(RwEngineInstance + 0x114))(
              (const char*)(unk04 + 0x14), unk00) == 0) {
        u8* (*unk08)(u8*, const char*) =
            *(u8* (**)(u8*, const char*))(unk04 + 0x2C);
        if (unk08 != 0) {
          return unk08(arg0, arg1);
        }
      }
      unk04 = *(u8**)(unk04 + 0x30);
    }
    return 0;
  }
  return 0;
}

static inline u8* projectImageReadFile(u8* arg0) {
  char* unk00;
  char* unk04;
  u8* unk08;
  unk00 = (*(char* (**)(const char*, s32))(RwEngineInstance + 0x108))((const char*)arg0, 0x3A);
  if (unk00 != 0) {
    unk04 = unk00;
  } else {
    unk04 = (char*)arg0;
  }
  unk00 = (*(char* (**)(const char*, s32))(RwEngineInstance + 0x108))(unk04, 0x2F);
  if (unk00 != 0) {
    unk04 = unk00;
  }
  unk00 = (*(char* (**)(const char*, s32))(RwEngineInstance + 0x108))(unk04, 0x5C);
  unk04 = (*(char* (**)(const char*, s32))(RwEngineInstance + 0x108))(
      unk00 != 0 ? unk00 : unk04, 0x2E);
  if (unk04 != 0) {
    unk08 = *(u8**)(RwEngineInstance + imageModule[0] + 0x21C);
    while (unk08 != 0) {
      if ((*(s32 (**)(const char*, const char*))(RwEngineInstance + 0x114))((const char*)unk08, unk04) == 0 ||
          (*(s32 (**)(const char*, const char*))(RwEngineInstance + 0x114))((const char*)unk08 + 0x14, unk04) == 0) {
        if (*(u8* (**)(u8*))(unk08 + 0x28) != 0) {
          struct {
            u8* (*unk00)(u8*);
            u8* unk04;
          } unk0C;
          unk0C.unk00 = *(u8* (**)(u8*))(unk08 + 0x28);
          unk0C.unk04 = 0;
          ImagePathForAllFullNames(arg0, 5, ImageAttempRead, (u8*)&unk0C);
          return unk0C.unk04;
        } else {
          return 0;
        }
      }
      unk08 = *(u8**)(unk08 + 0x30);
    }
    return 0;
  }
  return 0;
}

/* TODO: [near miss] 99.92%; read cursor register allocation remains. */
u8* RwImageReadMaskedImage(u8* unk00, u8* unk04) {
  u8* unk08 = projectImageReadFile(unk00);
  if (unk08 != 0) {
    if (unk04 != 0 && *(s8*)unk04 != 0) {
      u8* unk0C = projectImageReadFile(unk04);
      if (unk0C == 0) {
        RwImageDestroy(unk08);
        return 0;
      }
      if (RwImageMakeMask(unk0C) == 0) {
        RwImageDestroy(unk08);
        RwImageDestroy(unk0C);
        return 0;
      }
      if (RwImageApplyMask(unk08, unk0C) == 0) {
        RwImageDestroy(unk08);
        RwImageDestroy(unk0C);
        return 0;
      }
      RwImageDestroy(unk0C);
    }
    return unk08;
  } else {
    return 0;
  }
}

u8* RwRGBASetFromPixel(u8* unk00, u32 unk04, s32 unk08) {
  (*(void (**)(u8*, u32*, s32))(RwEngineInstance + 0x54))(unk00, &unk04, unk08);
  return unk00;
}

#include <string.h>

static s32 ImageConvertDepth(u8* destination, const u8* source) {
  s32 column;
  s32 row;
  s32 width;
  s32 height;
  u8* output;
  const u8* input;
  s32 success = 0;
  const u32* palette;
  width = *(s32*)(destination + 4);
  height = *(s32*)(destination + 8);
  palette = *(u32**)(source + 0x18);
  input = *(u8**)(source + 0x14);
  output = *(u8**)(destination + 0x14);
  switch ((*(s32*)(source + 0xC) << 8) | *(s32*)(destination + 0xC)) {
    case 0x404:
    case 0x808:
    case 0x2020:
      success = 1;
      break;
    case 0x408: {
      for (row = 0; row < height; ++row) {
        memcpy(output, input, width);
        input += *(s32*)(source + 0x10);
        output += *(s32*)(destination + 0x10);
      }
      success = 1;
      break;
    }
    case 0x420:
    case 0x820: {
      for (row = 0; row < height; ++row) {
        for (column = 0; column < width; ++column) {
          u32 paletteOffset = input[column] * 4;
          s32 offset = column * 4;
          u32 value = *(const u32*)((const u8*)palette + paletteOffset);
          *(u32*)(output + offset) = value;
        }
        input += *(s32*)(source + 0x10);
        output += *(s32*)(destination + 0x10);
      }
      success = 1;
      break;
    }
    case 0x804:
    case 0x2004:
    case 0x2008:
    default: {
      BaerrState error;
      error.unk00 = 1;
      error.unk04 = _rwerror(0x80000009);
      RwErrorSet(&error);
      break;
    }
  }
  return success;
}

u8* RwImageCopy(u8* unk00, const u8* unk04) {
  s32 unk08 = *(s32*)(unk00 + 0x0C);
  s32 unk0C = *(s32*)(unk04 + 0x0C);
  if (unk08 == unk0C) {
    u8* unk1C;
    const u8* unk18;
    s32 unk14;
    s32 unk10;
    u8* unk20 = *(u8**)(unk00 + 0x18);
    if (unk20 != 0) {
      const u8* unk24 = *(u8**)(unk04 + 0x18);
      if (unk24 != 0 && unk0C <= 8) {
        memcpy(unk20, unk24, (1 << unk0C) * 4);
      }
    }
    unk14 = ((*(s32*)(unk00 + 0x0C) + 7) >> 3) * *(s32*)(unk00 + 0x04);
    unk18 = *(u8**)(unk04 + 0x14);
    unk1C = *(u8**)(unk00 + 0x14);
    for (unk10 = 0; unk10 < *(s32*)(unk00 + 0x08); ++unk10) {
      memcpy(unk1C, unk18, unk14);
      unk1C += *(s32*)(unk00 + 0x10);
      unk18 += *(s32*)(unk04 + 0x10);
    }
  } else if (ImageConvertDepth(unk00, unk04) == 0) {
    unk00 = 0;
  }
  return unk00;
}

u8* RwImageGammaCorrect(u8* unk00) {
  s32 unk04 = *(s32*)(unk00 + 0xC);
  switch (unk04) {
    case 4:
    case 8: {
      u8* unk08 = *(u8**)(unk00 + 0x18);
      s32 unk0C = 1 << unk04;
      if (unk08 == 0) {
        BaerrState unk10;
        unk10.unk00 = 1;
        unk10.unk04 = _rwerror(0x80000016);
        RwErrorSet(&unk10);
        return 0;
      }
      {
        const u8* unk18;
        const u8* unk1C;
        s32 unk14 = unk0C;
        unk18 = unk08;
        unk1C = RwEngineInstance + imageModule[0] + 0xC;
        while (unk14-- != 0) {
          unk08[0] = unk1C[unk18[0]];
          unk08[1] = unk1C[unk18[1]];
          unk08[2] = unk1C[unk18[2]];
          unk08[3] = unk18[3];
          unk08 += 4;
          unk18 += 4;
        }
      }
      break;
    }
    case 32: {
      u8* unk20;
      const u8* unk1C;
      s32 unk18;
      u8* unk08;
      s32 unk0C;
      s32 unk10;
      s32 unk14;
      unk08 = *(u8**)(unk00 + 0x14);
      unk0C = *(s32*)(unk00 + 4);
      unk10 = *(s32*)(unk00 + 8);
      if (unk08 == 0) {
        BaerrState unk28;
        unk28.unk00 = 1;
        unk28.unk04 = _rwerror(0x80000016);
        RwErrorSet(&unk28);
        return 0;
      }
      for (unk14 = 0; unk14 < unk10; unk14++) {
        const u8* unk24;
        unk18 = unk0C;
        unk1C = unk08;
        unk20 = unk08;
        unk24 = RwEngineInstance + imageModule[0] + 0xC;
        for (; unk18-- != 0; unk1C += 4) {
          unk20[0] = unk24[unk1C[0]];
          unk20[1] = unk24[unk1C[1]];
          unk20[2] = unk24[unk1C[2]];
          unk20[3] = unk1C[3];
          unk20 += 4;
        }
        unk08 += *(s32*)(unk00 + 0x10);
      }
      break;
    }
    default: {
      BaerrState unk08;
      unk08.unk00 = 1;
      unk08.unk04 = _rwerror(0x80000008);
      RwErrorSet(&unk08);
      return 0;
    }
  }
  *(u32*)unk00 |= 2;
  return unk00;
}

#include <math.h>

static inline f32 projectImageScaleChannel(f32 unk00) {
  return 0.5f + 255.0f * unk00;
}

s32 RwImageSetGamma(f32 unk00) {
  f32 unk04;
  s32 unk08;
  *(f32*)(RwEngineInstance + imageModule[0] + 0x20C) = unk00;
  unk04 = 1.0f / unk00;
  ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk0C[0] = 0;
  ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk0C[0x100] = 0;
  for (unk08 = 1; unk08 < 256; ++unk08) {
    f32 unk0C = (f32)unk08 / 255.0f;
    ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk0C[unk08] =
        (u8)projectImageScaleChannel((f32)pow(unk0C, unk04));
    ((ProjectImageModule*)(RwEngineInstance + imageModule[0]))->unk0C[0x100 + unk08] =
        (u8)projectImageScaleChannel((f32)pow(unk0C, unk00));
  }
  return 1;
}
