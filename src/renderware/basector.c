/* #audit 2026-10-03T04:44Z clean-room FIXED (audit) */
#include <dolphin/types.h>
#include <renderware/project_registry.h>

extern s32 _rwPluginRegistryAddPluginStream(void* arg0, u32 arg1, void* arg2, void* arg3, void* arg4);

u32 sectorTKList[6] = {0x94, 0x94, 0, 0, 0, 0};

static s32 sectorModule[2];

void* _rpSectorOpen(void* arg0) {
  sectorModule[1] += 1;
  return arg0;
}

void* _rpSectorClose(void* arg0) {
  sectorModule[1] -= 1;
  return arg0;
}

u32 RpWorldSectorGetNumVertices(const u8* arg0) {
  return *(const u16*)(arg0 + 0x8E);
}

s32 RpWorldSectorRegisterPlugin(s32 unk00, u32 unk04, ProjectRegistryCall unk08,
    ProjectRegistryCall unk0C, ProjectRegistryCopy unk10) {
  return _rwPluginRegistryAddPlugin((u8*)sectorTKList, unk00, unk04, unk08, unk0C, unk10);
}

s32 RpWorldSectorRegisterPluginStream(u32 arg0, void* arg1, void* arg2, void* arg3) {
  return _rwPluginRegistryAddPluginStream(sectorTKList, arg0, arg1, arg2, arg3);
}
