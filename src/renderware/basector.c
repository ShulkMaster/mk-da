/* #audit 2026-10-03T04:51Z clean-room PASS (audit) */
#include <dolphin/types.h>
#include <renderware/project_registry.h>

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

s32 RpWorldSectorRegisterPluginStream(u32 arg0, ProjectRegistryRead arg1,
    ProjectRegistryWrite arg2, ProjectRegistrySize arg3) {
  return _rwPluginRegistryAddPluginStream((u8*)sectorTKList, arg0, arg1, arg2, arg3);
}
