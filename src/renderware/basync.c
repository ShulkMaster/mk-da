/* #audit 2026-10-03T09:17Z clean-room FIXED (audit) */
#include <dolphin/types.h>
#include <renderware/project_state.h>
#include <renderware/project_frame.h>
#include <renderware/project_matrix.h>
#include <renderware/project_vector.h>

static u8* SyncObject(u8* arg0, void* arg1) {
  /* #audit 2026-10-03T06:31Z clean-room PASS (audit) */
  (*(u8* (**)(u8*))(arg0 + 0x10))(arg0);
  return arg0;
}

static void FrameSyncHierarchyRecurse(u8* arg0, u32 arg1) {
  /* #audit 2026-10-03T06:38Z clean-room FIXED (audit) */
  while (arg0 != 0) {
    u32 unk00 = arg0[3];
    u32 unk04 = arg1 | unk00;
    if (unk04 & 4) {
      if (unk00 & 0x20) {
        *(ProjectMatrix*)(arg0 + 0x50) = *(const ProjectMatrix*)(arg0 + 0x10);
        RwV3dTransformPoints((f32*)(arg0 + 0x80), (f32*)(arg0 + 0x40), 1,
                            (f32*)(*(u8**)(arg0 + 4) + 0x50));
        RwMatrixUpdate(arg0 + 0x50);
      } else {
        RwMatrixMultiply(arg0 + 0x50, arg0 + 0x10, *(u8**)(arg0 + 4) + 0x50);
      }
    }
    if (*(u8**)(arg0 + 0x90) != arg0 + 0x90) {
      RwFrameForAllObjects(arg0, SyncObject, 0);
    }
    arg0[3] &= ~0xCU;
    FrameSyncHierarchyRecurse(*(u8**)(arg0 + 0x98), unk04);
    arg0 = *(u8**)(arg0 + 0x9C);
  }
}

static void FrameSyncHierarchyRecurseNoLTM(u8* unk00) {
  /* #audit 2026-10-03T06:33Z clean-room PASS (audit) */
  while (unk00 != 0) {
    if (*(u8**)(unk00 + 0x90) != unk00 + 0x90) {
      RwFrameForAllObjects(unk00, SyncObject, 0);
    }
    unk00[3] &= ~8U;
    FrameSyncHierarchyRecurseNoLTM(*(u8**)(unk00 + 0x98));
    unk00 = *(u8**)(unk00 + 0x9C);
  }
}

static void FrameSyncHierarchy(u8* frame) {
  /* #audit 2026-10-03T06:41Z clean-room PASS (audit) */
  u32 changed;
  u32 flags = frame[3];
  if (flags & 1) {
    changed = flags & 4;
    if (changed) {
      *(ProjectMatrix*)(frame + 0x50) =
          *(const ProjectMatrix*)(frame + 0x10);
    }
    if (*(u8**)(frame + 0x90) != frame + 0x90) {
      RwFrameForAllObjects(frame, SyncObject, NULL);
    }
    FrameSyncHierarchyRecurse(*(u8**)(frame + 0x98), changed);
  } else {
    if (*(u8**)(frame + 0x90) != frame + 0x90) {
      RwFrameForAllObjects(frame, SyncObject, NULL);
    }
    FrameSyncHierarchyRecurseNoLTM(*(u8**)(frame + 0x98));
  }
  frame[3] = flags & 0xF0;
}

s32 _rwFrameSyncDirty(void) {
  /* #audit 2026-10-03T09:17Z clean-room PASS (audit) */
  u8* unkBC = RwEngineInstance + 0xBC;
  u8* unk00 = *(u8**)(RwEngineInstance + 0xBC);
  while (unk00 != unkBC) {
    FrameSyncHierarchy(unk00 - 8);
    unk00 = *(u8**)unk00;
  }
  *(u8**)(RwEngineInstance + 0xBC) = RwEngineInstance + 0xBC;
  *(u8**)(RwEngineInstance + 0xC0) = RwEngineInstance + 0xBC;
  return 1;
}

static void FrameSyncHierarchyLTMRecurse(u8* unk00, u32 unk04) {
  /* #audit 2026-10-03T06:38Z clean-room PASS (audit) */
  while (unk00 != 0) {
    u32 unk08 = unk04 | unk00[3];
    if (unk08 & 4) {
      if (unk00[3] & 0x20) {
        *(ProjectMatrix*)(unk00 + 0x50) = *(const ProjectMatrix*)(unk00 + 0x10);
        RwV3dTransformPoints((f32*)(unk00 + 0x80), (const f32*)(unk00 + 0x40),
            1, (const f32*)(*(u8**)(unk00 + 4) + 0x50));
        RwMatrixUpdate(unk00 + 0x50);
      } else {
        RwMatrixMultiply(unk00 + 0x50, unk00 + 0x10, *(u8**)(unk00 + 4) + 0x50);
      }
      unk00[3] &= ~4U;
    }
    FrameSyncHierarchyLTMRecurse(*(u8**)(unk00 + 0x98), unk08);
    unk00 = *(u8**)(unk00 + 0x9C);
  }
}

void _rwFrameSyncHierarchyLTM(u8* arg0) {
  /* #audit 2026-10-03T09:17Z clean-room PASS (audit) */
  u32 unk00 = arg0[3];
  if (unk00 & 4) {
    *(ProjectMatrix*)(arg0 + 0x50) = *(const ProjectMatrix*)(arg0 + 0x10);
  }
  FrameSyncHierarchyLTMRecurse(*(u8**)(arg0 + 0x98), unk00);
  arg0[3] = unk00 & ~5U;
}
