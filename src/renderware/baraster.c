/* #audit 2026-10-03T06:10Z clean-room FIXED (audit) */
#include <string.h>
#include <renderware/project_state.h>
#include <renderware/project_registry.h>
#include <renderware/project_resources.h>
#include <renderware/project_memory.h>

static u32 rasterTKList[6] = {0x34, 0x34, 0, 0, 0, 0};
static s32 rasterModule[2];

typedef struct {
  u8 unk00[0x60];
  void* unk60;
} ProjectRasterModule;

u8* RwRasterUnlock(u8* unk00) {
  (*(s32 (**)(void*, u8*, s32))(RwEngineInstance + 0x88))(0, unk00, 0);
  return unk00;
}

u8* RwRasterUnlockPalette(u8* unk00) {
  s32 (*unk04)(void*, u8*, s32) =
      *(s32 (**)(void*, u8*, s32))(RwEngineInstance + 0xA8);
  unk04(0, unk00, 0);
  unk00[0x22] &= ~0x18U;
  return unk00;
}

s32 RwRasterDestroy(u8* unk00) {
  _rwPluginRegistryDeInitObject((u8*)rasterTKList, unk00);
  (*(s32 (**)(void*, u8*, s32))(RwEngineInstance + 0x5C))(0, unk00, 0);
  (*(void* (**)(void*, u8*))(RwEngineInstance + 0x144))(
      ((ProjectRasterModule*)(RwEngineInstance + rasterModule[0]))->unk60, unk00);
  return 1;
}

s32 RwRasterRegisterPlugin(s32 unk00, u32 unk04,
    ProjectRegistryCall unk08, ProjectRegistryCall unk0C,
    ProjectRegistryCopy unk10) {
  return _rwPluginRegistryAddPlugin((u8*)rasterTKList, unk00, unk04,
      unk08, unk0C, unk10);
}

u8* RwRasterLockPalette(u8* unk00, s32 unk04) {
  u8* unk08;
  if ((*(s32 (**)(u8**, u8*, s32))(RwEngineInstance + 0xA4))(
          &unk08, unk00, unk04)) {
    return unk08;
  }
  return 0;
}

s32 RwRasterGetNumLevels(u8* unk00) {
  s32 unk04;
  s32 (*unk08)(s32*, u8*, s32);
  if (!((unk00[0x23] << 8) & 0x8000)) {
    return 1;
  }
  unk08 = *(s32 (**)(s32*, u8*, s32))(RwEngineInstance + 0xB8);
  if (unk08(&unk04, unk00, 0)) {
    return unk04;
  }
  return -1;
}

u8* RwRasterShowRaster(u8* unk00, u32 unk04, s32 unk08) {
  s32 (*unk0C)(u8*, u32, s32) =
      *(s32 (**)(u8*, u32, s32))(RwEngineInstance + 0x98);
  _rwResourcesPurge();
  if (unk0C(unk00, unk04, unk08)) {
    return unk00;
  }
  return 0;
}

u8* RwRasterSubRaster(u8* unk00, u8* unk04, const s32* unk08) {
  if (!(unk00[0x21] & 0x80)) {
    return 0;
  }
  *(s32*)(unk00 + 0xC) = unk08[2];
  *(s32*)(unk00 + 0x10) = unk08[3];
  *(s16*)(unk00 + 0x1C) = *(s16*)(unk04 + 0x1C) + (s16)unk08[0];
  *(s16*)(unk00 + 0x1E) = *(s16*)(unk04 + 0x1E) + (s16)unk08[1];
  if ((*(s32 (**)(u8*, u8*, s32))(RwEngineInstance + 0x78))(
          unk00, unk04, 0)) {
    *(void**)unk00 = *(void**)unk04;
    return unk00;
  }
  return 0;
}

u8* RwRasterCreate(s32 unk00, s32 unk04, s32 unk08, s32 unk0C) {
  u8* unk10 = (*(u8* (**)(void*))(RwEngineInstance + 0x140))(
      ((ProjectRasterModule*)(RwEngineInstance + rasterModule[0]))->unk60);
  if (unk10 != 0) {
    s32 (*unk14)(void*, u8*, s32) =
        *(s32 (**)(void*, u8*, s32))(RwEngineInstance + 0x58);
    unk10[0x22] = 0;
    unk10[0x21] = 0;
    *(s32*)(unk10 + 0xC) = unk00;
    *(s32*)(unk10 + 0x10) = unk04;
    *(s16*)(unk10 + 0x1C) = 0;
    *(s16*)(unk10 + 0x1E) = 0;
    *(s32*)(unk10 + 0x14) = unk08;
    *(u8**)unk10 = unk10;
    *(u32*)(unk10 + 4) = 0;
    *(u32*)(unk10 + 8) = 0;
    if (!unk14(0, unk10, unk0C)) {
      (*(void* (**)(void*, u8*))(RwEngineInstance + 0x144))(
          ((ProjectRasterModule*)(RwEngineInstance + rasterModule[0]))->unk60,
          unk10);
      return 0;
    }
    _rwPluginRegistryInitObject((u8*)rasterTKList, unk10);
    return unk10;
  }
  return 0;
}

u8* RwRasterLock(u8* unk00, s32 unk04, s32 unk08) {
  u8* unk0C;
  if ((*(s32 (**)(u8**, u8*, s32))(RwEngineInstance + 0x84))(
          &unk0C, unk00, unk08 + ((unk04 & 0xFF) << 8))) {
    return unk0C;
  }
  return 0;
}

void* _rwRasterClose(void* unk00, s32 unk04, s32 unk08) {
  if (((ProjectRasterModule*)(RwEngineInstance + rasterModule[0]))->unk60 != 0) {
    RwFreeListDestroy(((ProjectRasterModule*)(RwEngineInstance + rasterModule[0]))->unk60);
    ((ProjectRasterModule*)(RwEngineInstance + rasterModule[0]))->unk60 = 0;
  }
  --rasterModule[1];
  return unk00;
}

void* _rwRasterOpen(void* arg0, s32 arg1, s32 arg2) {
  rasterModule[0] = arg1;
  memset(RwEngineInstance + arg1 + 0x2C, 0, 0x34);
  *(u32*)(RwEngineInstance + rasterModule[0] + 0x38) = 0;
  *(u32*)(RwEngineInstance + rasterModule[0] + 0x3C) = 0;
  *(u32*)(RwEngineInstance + rasterModule[0] + 0x40) = 0;
  *(u8*)(RwEngineInstance + rasterModule[0] + 0x4D) = 0x80;
  *(u32*)(RwEngineInstance + rasterModule[0] + 0x30) = 0;
  *(u32*)(RwEngineInstance + rasterModule[0] + 0x34) = 0;
  *(u8*)(RwEngineInstance + rasterModule[0] + 0x4C) = 0;
  *(u32*)(RwEngineInstance + rasterModule[0] + 0x28) = 0;
  *(u8**)(RwEngineInstance + rasterModule[0]) = RwEngineInstance + rasterModule[0] + 0x2C;
  ((ProjectRasterModule*)(RwEngineInstance + rasterModule[0]))->unk60 =
      RwFreeListCreate((s32)rasterTKList[0], 0x14, 0);
  if (((ProjectRasterModule*)(RwEngineInstance + rasterModule[0]))->unk60 == 0) {
    return 0;
  }
  rasterModule[1]++;
  return arg0;
}
