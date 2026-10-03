/* #audit 2026-10-03T09:20Z clean-room FIXED (audit) */
#include <dolphin/types.h>
#include <renderware/project_memory.h>
#include <renderware/project_frame.h>
#include <renderware/project_registry.h>
#include <renderware/project_state.h>
#include <renderware/project_matrix.h>

static s32 frameModule[2];
s32 frameTKList[6] = {0xA4, 0xA4, 0, 0, 0, 0};

u8* _rwFrameOpen(u8* arg0, s32 arg1, s32 arg2) {
  /* #audit 2026-10-03T06:42Z clean-room PASS (audit) */
  frameModule[0] = arg1;
  *(u8**)(RwEngineInstance + frameModule[0]) = RwFreeListCreate(frameTKList[0], 0x32, 4);
  if (*(u8**)(RwEngineInstance + frameModule[0]) == 0) {
    return 0;
  }
  *(u8**)(RwEngineInstance + 0xBC) = RwEngineInstance + 0xBC;
  *(u8**)(RwEngineInstance + 0xC0) = RwEngineInstance + 0xBC;
  ++frameModule[1];
  return arg0;
}

u8* _rwFrameClose(u8* unk00, s32 unk04, s32 unk08) {
  /* #audit 2026-10-03T09:17Z clean-room PASS (audit) */
  void* unk0C = *(void**)(RwEngineInstance + frameModule[0]);
  if (unk0C != NULL) {
    RwFreeListDestroy(unk0C);
    *(void**)(RwEngineInstance + frameModule[0]) = NULL;
  }
  --frameModule[1];
  return unk00;
}

static void rwSetHierarchyRoot(u8* arg0, u8* arg1) {
  /* #audit 2026-10-03T06:47Z clean-room PASS (audit) */
  *(u8**)(arg0 + 0xA0) = arg1;
  arg0 = *(u8**)(arg0 + 0x98);
  while (arg0 != 0) {
    rwSetHierarchyRoot(arg0, arg1);
    arg0 = *(u8**)(arg0 + 0x9C);
  }
}

u32 RwFrameDirty(u8* frame) {
  /* #audit 2026-10-03T06:46Z clean-room PASS (audit) */
  return (*(u8**)(frame + 0xA0))[3] & 3;
}

u8* RwFrameCreate(void) {
  /* #audit 2026-10-03T06:43Z clean-room PASS (audit) */
  u8* unk00 = (*(u8* (**)(u8*))(RwEngineInstance + 0x140))(
      *(u8**)(RwEngineInstance + frameModule[0]));
  if (unk00 == 0) {
    return 0;
  }
  unk00[0] = 0;
  unk00[1] = 0;
  unk00[2] = 0;
  unk00[3] = 0;
  *(u8**)(unk00 + 4) = 0;
  *(u8**)(unk00 + 0x90) = unk00 + 0x90;
  *(u8**)(unk00 + 0x94) = unk00 + 0x90;
  *(u32*)(unk00 + 0x1C) = 3;
  *(f32*)(unk00 + 0x38) = 1.0f;
  *(f32*)(unk00 + 0x24) = 1.0f;
  *(f32*)(unk00 + 0x10) = 1.0f;
  *(f32*)(unk00 + 0x20) = 0.0f;
  *(f32*)(unk00 + 0x18) = 0.0f;
  *(f32*)(unk00 + 0x14) = 0.0f;
  *(f32*)(unk00 + 0x34) = 0.0f;
  *(f32*)(unk00 + 0x30) = 0.0f;
  *(f32*)(unk00 + 0x28) = 0.0f;
  *(f32*)(unk00 + 0x48) = 0.0f;
  *(f32*)(unk00 + 0x44) = 0.0f;
  *(f32*)(unk00 + 0x40) = 0.0f;
  *(u32*)(unk00 + 0x1C) |= 0x20000 | 3;
  *(u32*)(unk00 + 0x5C) = 3;
  *(f32*)(unk00 + 0x78) = 1.0f;
  *(f32*)(unk00 + 0x64) = 1.0f;
  *(f32*)(unk00 + 0x50) = 1.0f;
  *(f32*)(unk00 + 0x60) = 0.0f;
  *(f32*)(unk00 + 0x58) = 0.0f;
  *(f32*)(unk00 + 0x54) = 0.0f;
  *(f32*)(unk00 + 0x74) = 0.0f;
  *(f32*)(unk00 + 0x70) = 0.0f;
  *(f32*)(unk00 + 0x68) = 0.0f;
  *(f32*)(unk00 + 0x88) = 0.0f;
  *(f32*)(unk00 + 0x84) = 0.0f;
  *(f32*)(unk00 + 0x80) = 0.0f;
  *(u32*)(unk00 + 0x5C) |= 0x20000 | 3;
  *(u8**)(unk00 + 0x98) = 0;
  *(u8**)(unk00 + 0x9C) = 0;
  *(u8**)(unk00 + 0xA0) = unk00;
  _rwPluginRegistryInitObject((u8*)frameTKList, unk00);
  return unk00;
}

s32 RwFrameDestroy(u8* unk00) {
  /* #audit 2026-10-03T06:50Z clean-room PASS (audit) */
  u8* unk04;
  _rwPluginRegistryDeInitObject((u8*)frameTKList, unk00);
  if (*(u8**)(unk00 + 4) != 0) {
    RwFrameRemoveChild(unk00);
  }
  if ((unk00[3] & 3) != 0) {
    **(u8***)(unk00 + 0xC) = *(u8**)(unk00 + 8);
    *(u8**)(*(u8**)(unk00 + 8) + 4) = *(u8**)(unk00 + 0xC);
  }
  unk04 = *(u8**)(unk00 + 0x98);
  while (unk04 != 0) {
    *(u32*)(unk04 + 4) = 0;
    unk04 = *(u8**)(unk04 + 0x9C);
  }
  (*(void* (**)(u8*, void*))(RwEngineInstance + 0x144))(
      *(u8**)(RwEngineInstance + frameModule[0]), unk00);
  return 1;
}

static void FrameDestroyRecurseDeInitLeaf(u8* arg0) {
  /* #audit 2026-10-03T06:47Z clean-room PASS (audit) */
  _rwPluginRegistryDeInitObject((u8*)frameTKList, arg0);
  if (arg0[3] & 3) {
    *(u8**)(*(u8**)(arg0 + 0xC)) = *(u8**)(arg0 + 8);
    *(u8**)(*(u8**)(arg0 + 8) + 4) = *(u8**)(arg0 + 0xC);
  }
}

static void rwFrameDestroyRecurseDestroyLeaf(u8* arg0) {
  /* #audit 2026-10-03T06:47Z clean-room PASS (audit) */
  FrameDestroyRecurseDeInitLeaf(arg0);
  (*(void* (**)(u8*, void*))(RwEngineInstance + 0x144))(
      *(u8**)(RwEngineInstance + frameModule[0]), arg0);
}

static void rwFrameDestroyRecurse(u8* unk00) {
  /* #audit 2026-10-03T06:47Z clean-room PASS (audit) */
  if (unk00 != 0) {
    u8* unk04 = *(u8**)(unk00 + 0x98);
    while (unk04 != 0) {
      u8* unk08 = *(u8**)(unk04 + 0x9C);
      rwFrameDestroyRecurse(unk04);
      unk04 = unk08;
    }
    rwFrameDestroyRecurseDestroyLeaf(unk00);
  }
}

s32 RwFrameDestroyHierarchy(u8* unk00) {
  /* #audit 2026-10-03T09:20Z clean-room PASS (audit) */
  rwFrameDestroyRecurse(unk00);
  return 1;
}

u8* RwFrameUpdateObjects(u8* arg0) {
  /* #audit 2026-10-03T06:47Z clean-room PASS (audit) */
  u8* unk00 = *(u8**)(arg0 + 0xA0);
  u32 unk04 = unk00[3];
  if (!(unk04 & 3)) {
    *(u8**)(unk00 + 8) = *(u8**)(RwEngineInstance + 0xBC);
    *(u8**)(*(u8**)(arg0 + 0xA0) + 0xC) = RwEngineInstance + 0xBC;
    *(u8**)(*(u8**)(RwEngineInstance + 0xBC) + 4) = *(u8**)(arg0 + 0xA0) + 8;
    *(u8**)(RwEngineInstance + 0xBC) = *(u8**)(arg0 + 0xA0) + 8;
  }
  (*(u8**)(arg0 + 0xA0))[3] = unk04 | 3;
  arg0[3] |= 0xC;
  return arg0;
}

u8* RwFrameGetLTM(u8* frame) {
  /* #audit 2026-10-03T06:48Z clean-room PASS (audit) */
  u8* root = *(u8**)(frame + 0xA0);
  if (root[3] & 1) {
    _rwFrameSyncHierarchyLTM(root);
  }
  return frame + 0x50;
}

u8* RwFrameAddChild(u8* arg0, u8* arg1) {
  /* #audit 2026-10-03T06:50Z clean-room PASS (audit) */
  if (*(u8**)(arg1 + 4) != 0) {
    RwFrameRemoveChild(arg1);
  }
  *(u8**)(arg1 + 0x9C) = *(u8**)(arg0 + 0x98);
  *(u8**)(arg0 + 0x98) = arg1;
  *(u8**)(arg1 + 4) = arg0;
  rwSetHierarchyRoot(arg1, *(u8**)(arg0 + 0xA0));
  if (arg1[3] & 3) {
    **(u8***)(arg1 + 0xC) = *(u8**)(arg1 + 8);
    *(u8**)(*(u8**)(arg1 + 8) + 4) = *(u8**)(arg1 + 0xC);
    arg1[3] &= ~3U;
  }
  RwFrameUpdateObjects(arg1);
  return arg0;
}

u8* RwFrameRemoveChild(u8* unk00) {
  /* #audit 2026-10-03T06:48Z clean-room PASS (audit) */
  u8* unk04 = *(u8**)(unk00 + 4);
  u8* unk08 = *(u8**)(unk04 + 0x98);
  if (unk08 == unk00) {
    *(u8**)(unk04 + 0x98) = *(u8**)(unk00 + 0x9C);
  } else {
    while (*(u8**)(unk08 + 0x9C) != unk00) {
      unk08 = *(u8**)(unk08 + 0x9C);
    }
    *(u8**)(unk08 + 0x9C) = *(u8**)(unk00 + 0x9C);
  }
  *(u8**)(unk00 + 4) = 0;
  *(u8**)(unk00 + 0x9C) = 0;
  rwSetHierarchyRoot(unk00, unk00);
  RwFrameUpdateObjects(unk00);
  return unk00;
}

u8* RwFrameForAllChildren(u8* unk00, u8* (*unk04)(u8*, void*), void* unk08) {
  /* #audit 2026-10-03T06:48Z clean-room PASS (audit) */
  u8* unk0C = *(u8**)(unk00 + 0x98);
  while (unk0C != 0) {
    u8* unk10 = *(u8**)(unk0C + 0x9C);
    if (unk04(unk0C, unk08) == 0) {
      return unk00;
    }
    unk0C = unk10;
  }
  return unk00;
}

u8* RwFrameTransform(u8* unk00, const u8* unk04, s32 unk08) {
  /* #audit 2026-10-03T06:49Z clean-room PASS (audit) */
  RwMatrixTransform(unk00 + 0x10, unk04, unk08);
  RwFrameUpdateObjects(unk00);
  return unk00;
}

u8* RwFrameForAllObjects(u8* unk00, u8* (*unk04)(u8*, void*), void* unk08) {
  /* #audit 2026-10-03T06:49Z clean-room PASS (audit) */
  u8* next;
  u8* end;
  u8* link = *(u8**)(unk00 + 0x90);
  end = unk00 + 0x90;
  while (link != end) {
    next = *(u8**)link;
    if (unk04(link - 8, unk08) == 0) {
      return unk00;
    }
    link = next;
  }
  return unk00;
}

s32 RwFrameRegisterPlugin(s32 arg0, u32 arg1, ProjectRegistryCall arg2,
    ProjectRegistryCall arg3, ProjectRegistryCopy arg4) {
  /* #audit 2026-10-03T09:20Z clean-room PASS (audit) */
  return _rwPluginRegistryAddPlugin((u8*)frameTKList, arg0, arg1,
      arg2, arg3, arg4);
}
