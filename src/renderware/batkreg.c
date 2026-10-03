/* #audit 2026-10-03T05:05Z clean-room FIXED (audit) */
#include <dolphin/types.h>
#include <renderware/project_state.h>
#include <renderware/project_registry.h>
#include <renderware/project_error.h>

extern u32 _rwGetNumEngineInstances(void);
extern void* RwFreeListForAllUsed(void* arg0, void (*arg1)(u8*, void*), void* arg2);
extern s32 RwFreeListDestroy(void* arg0);

static void* toolkitRegEntries;
extern void* RwFreeListCreate(s32 arg0, s32 arg1, s32 arg2);

s32 _rwPluginRegistryOpen(void) {
  toolkitRegEntries = RwFreeListCreate(0x3C, 0x14, 0);
  return toolkitRegEntries != 0;
}

static void rwDestroyEntry(u8* arg0, void* arg1) {
  u8* unk00 = *(u8**)(arg0 + 0x38);
  if (*(u32*)(unk00 + 0x10) != 0) {
    *(u32*)unk00 = *(u32*)(unk00 + 4);
    *(u32*)(*(u8**)(arg0 + 0x38) + 0x10) = 0;
    *(u32*)(*(u8**)(arg0 + 0x38) + 0x14) = 0;
  }
  (*(void* (**)(void*, u8*))(RwEngineInstance + 0x144))(arg1, arg0);
}

s32 _rwPluginRegistryClose(void) {
  if (toolkitRegEntries != 0) {
    RwFreeListForAllUsed(toolkitRegEntries, rwDestroyEntry, toolkitRegEntries);
    RwFreeListDestroy(toolkitRegEntries);
    toolkitRegEntries = 0;
  }
  return 1;
}

static void* PluginDefaultConstructor(void* unk00, s32 unk04, s32 unk08) {
  return unk00;
}

static void* PluginDefaultDestructor(void* unk00, s32 unk04, s32 unk08) {
  return unk00;
}

static void* PluginDefaultCopy(void* unk00, const void* unk04, s32 unk08, s32 unk0C) {
  return unk00;
}

s32 _rwPluginRegistryGetPluginOffset(u8* unk00, u32 unk04) {
  u8* unk08 = *(u8**)(unk00 + 0x10);
  while (unk08 != 0) {
    if (*(u32*)(unk08 + 0x08) == unk04) {
      return *(s32*)unk08;
    }
    unk08 = *(u8**)(unk08 + 0x30);
  }
  return -1;
}

s32 _rwPluginRegistryAddPlugin(u8* unk00, s32 unk04, u32 unk08,
    ProjectRegistryCall unk0C, ProjectRegistryCall unk10,
    ProjectRegistryCopy unk14) {
  u8* unk18;
  s32 unk1C;

  if (toolkitRegEntries == 0) {
    return -1;
  }
  if (_rwGetNumEngineInstances() != 0) {
    BaerrState unk20;
    unk20.unk00 = 1;
    unk20.unk04 = _rwerror(0x80000017);
    RwErrorSet(&unk20);
    return -1;
  }
  unk18 = *(u8**)(unk00 + 0x10);
  while (unk18 != 0) {
    if (*(u32*)(unk18 + 0x08) == unk08) {
      return *(s32*)unk18;
    }
    unk18 = *(u8**)(unk18 + 0x30);
  }
  unk1C = *(s32*)unk00 + ((unk04 + 3) & ~3);
  if (*(s32*)(unk00 + 0x08) != 0 &&
      unk1C > *(s32*)(unk00 + 0x08)) {
    return -1;
  }
  unk18 = (*(u8* (**)(void*))(RwEngineInstance + 0x140))(toolkitRegEntries);
  if (unk18 != 0) {
    *(s32*)unk18 = *(s32*)unk00;
    *(s32*)unk00 = unk1C;
    *(s32*)(unk18 + 0x04) = unk04;
    *(u32*)(unk18 + 0x08) = unk08;
    *(u32*)(unk18 + 0x0C) = 0;
    *(u32*)(unk18 + 0x10) = 0;
    *(u32*)(unk18 + 0x14) = 0;
    *(u32*)(unk18 + 0x18) = 0;
    *(u32*)(unk18 + 0x1C) = 0;
    *(ProjectRegistryCall*)(unk18 + 0x20) =
        unk0C != 0 ? unk0C : PluginDefaultConstructor;
    *(ProjectRegistryCall*)(unk18 + 0x24) =
        unk10 != 0 ? unk10 : PluginDefaultDestructor;
    *(ProjectRegistryCopy*)(unk18 + 0x28) =
        unk14 != 0 ? unk14 : PluginDefaultCopy;
    *(u32*)(unk18 + 0x2C) = 0;
    *(u8**)(unk18 + 0x30) = 0;
    *(u8**)(unk18 + 0x34) = 0;
    *(u8**)(unk18 + 0x38) = unk00;
    if (*(u8**)(unk00 + 0x10) == 0) {
      *(u8**)(unk00 + 0x10) = unk18;
      *(u8**)(unk00 + 0x14) = unk18;
    } else {
      *(u8**)(*(u8**)(unk00 + 0x14) + 0x30) = unk18;
      *(u8**)(unk18 + 0x34) = *(u8**)(unk00 + 0x14);
      *(u8**)(unk00 + 0x14) = unk18;
    }
    return *(s32*)unk18;
  }
  return -1;
}

u8* _rwPluginRegistryInitObject(u8* arg0, void* arg1) {
  u8* unk00 = *(u8**)(arg0 + 0x10);
  while (unk00 != 0) {
    if ((*(void* (**)(void*, s32, s32))(unk00 + 0x20))(arg1, *(s32*)unk00, *(s32*)(unk00 + 4)) == 0) {
      unk00 = *(u8**)(unk00 + 0x34);
      while (unk00 != 0) {
        (*(void* (**)(void*, s32, s32))(unk00 + 0x24))(arg1, *(s32*)unk00, *(s32*)(unk00 + 4));
        unk00 = *(u8**)(unk00 + 0x34);
      }
      return 0;
    }
    unk00 = *(u8**)(unk00 + 0x30);
  }
  return arg0;
}

u8* _rwPluginRegistryDeInitObject(u8* arg0, void* arg1) {
  u8* unk00 = *(u8**)(arg0 + 0x14);
  while (unk00 != 0) {
    void* (*unk04)(void*, s32, s32) = *(void* (**)(void*, s32, s32))(unk00 + 0x24);
    s32 unk08 = *(s32*)unk00;
    s32 unk0C = *(s32*)(unk00 + 4);
    unk04(arg1, unk08, unk0C);
    unk00 = *(u8**)(unk00 + 0x34);
  }
  return arg0;
}

u8* _rwPluginRegistryCopyObject(u8* arg0, void* arg1, const void* arg2) {
  u8* unk00 = *(u8**)(arg0 + 0x10);
  while (unk00 != 0) {
    (*(void* (**)(void*, const void*, s32, s32))(unk00 + 0x28))(
        arg1, arg2, *(s32*)unk00, *(s32*)(unk00 + 4));
    unk00 = *(u8**)(unk00 + 0x30);
  }
  return arg0;
}
