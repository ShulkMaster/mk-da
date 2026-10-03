/* #audit 2026-10-03T05:14Z clean-room FIXED (audit) */
#include <dolphin/types.h>

extern void* RwStreamReadInt32(void*, s32*, u32);
extern void* RwStreamWriteInt32(void*, const s32*, u32);

static s32 binWorldModule[2];
static s32 lastSeenWorldRightsPluginId;
static s32 lastSeenWorldExtraData;
static s32 lastSeenSectRightsPluginId;
static s32 lastSeenSectExtraData;

void* _rpReadWorldRights(void* arg0, s32 arg1, void* arg2, s32 arg3, s32 arg4) {
  if (RwStreamReadInt32(arg0, &lastSeenWorldRightsPluginId, 4) == 0) {
    return 0;
  }
  if (arg1 == 8) {
    if (RwStreamReadInt32(arg0, &lastSeenWorldExtraData, 4) == 0) {
      return 0;
    }
  }
  return arg0;
}

void* _rpWriteWorldRights(void* arg0, s32 arg1, const u8* arg2, s32 arg3, s32 arg4) {
  if (RwStreamWriteInt32(arg0, (const s32*)(*(const u8* const*)(arg2 + 0x78) + 0x2C), 4) == 0) {
    return 0;
  }
  if (RwStreamWriteInt32(arg0, (const s32*)(*(const u8* const*)(arg2 + 0x78) + 0x30), 4) != 0) {
    return arg0;
  }
  return 0;
}

s32 _rpSizeWorldRights(u8* unk00, s32 arg1, s32 arg2) {
  u8* unk04 = *(u8**)(unk00 + 0x78);
  if (unk04 != 0 && *(u32*)(unk04 + 0x2C) != 0) {
    return 8;
  }
  return 0;
}

void* _rpReadSectRights(void* unk00, s32 unk04, void* arg2, s32 arg3, s32 arg4) {
  if (RwStreamReadInt32(unk00, &lastSeenSectRightsPluginId, 4) == 0) {
    return 0;
  }
  if (unk04 == 8) {
    if (RwStreamReadInt32(unk00, &lastSeenSectExtraData, 4) == 0) {
      return 0;
    }
  }
  return unk00;
}

void* _rpWriteSectRights(void* arg0, s32 arg1, const u8* arg2, s32 arg3, s32 arg4) {
  if (RwStreamWriteInt32(arg0, (const s32*)(*(const u8* const*)(arg2 + 0x88) + 0x2c), 4) == 0) {
    return 0;
  }
  if (RwStreamWriteInt32(arg0, (const s32*)(*(const u8* const*)(arg2 + 0x88) + 0x30), 4) != 0) {
    return arg0;
  }
  return 0;
}

s32 _rpSizeSectRights(const u8* arg0, s32 arg1, s32 arg2) {
  const u8* unk00 = *(const u8* const*)(arg0 + 0x88);
  if (unk00 != 0 && *(const u32*)(unk00 + 0x2C) != 0) {
    return 8;
  }
  return 0;
}

void* _rpBinaryWorldClose(void* unk00, s32 unk04, s32 unk08) {
  --binWorldModule[1];
  return unk00;
}

void* _rpBinaryWorldOpen(void* arg0, s32 arg1, s32 arg2) {
  binWorldModule[1] += 1;
  return arg0;
}
