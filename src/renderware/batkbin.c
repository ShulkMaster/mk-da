/* #audit 2026-10-03T05:03Z clean-room PASS (audit) */
#include <renderware/project_registry.h>
#include <renderware/project_error.h>

extern s32 RwStreamFindChunk(void* arg0, u32 arg1, u32* arg2, u32* arg3);
extern s32 _rwStreamReadChunkHeader(void* arg0, u32* arg1, u32* arg2, u32* arg3, u32* arg4);
extern void* RwStreamSkip(void* arg0, u32 arg1);
extern void* _rwStreamWriteVersionedChunkHeader(void* arg0, u32 arg1, u32 arg2, u32 arg3, u32 arg4);

s32 _rwPluginRegistryAddPluginStream(u8* arg0, u32 arg1,
    ProjectRegistryRead arg2, ProjectRegistryWrite arg3, ProjectRegistrySize arg4) {
  u8* unk00 = *(u8**)(arg0 + 0x10);
  while (unk00 != 0) {
    if (*(u32*)(unk00 + 8) == arg1) {
      break;
    }
    unk00 = *(u8**)(unk00 + 0x30);
  }
  if (unk00 != 0) {
    *(ProjectRegistryRead*)(unk00 + 0xc) = arg2;
    *(ProjectRegistryWrite*)(unk00 + 0x10) = arg3;
    *(ProjectRegistrySize*)(unk00 + 0x14) = arg4;
    return *(s32*)unk00;
  }
  return -1;
}

s32 _rwPluginRegistryAddPlgnStrmlwysCB(u8* arg0, u32 arg1,
    s32 (*arg2)(void*, s32, s32)) {
  u8* unk00 = *(u8**)(arg0 + 0x10);
  while (unk00 != 0) {
    if (*(u32*)(unk00 + 8) == arg1) {
      break;
    }
    unk00 = *(u8**)(unk00 + 0x30);
  }
  if (unk00 != 0) {
    *(s32 (**)(void*, s32, s32))(unk00 + 0x18) = arg2;
    return *(s32*)unk00;
  }
  return -1;
}

s32 _rwPluginRegistryAddPlgnStrmRightsCB(u8* arg0, u32 arg1, ProjectRegistryRights arg2) {
  u8* unk00 = *(u8**)(arg0 + 0x10);
  while (unk00 != 0) {
    if (*(u32*)(unk00 + 8) == arg1) {
      break;
    }
    unk00 = *(u8**)(unk00 + 0x30);
  }
  if (unk00 != 0) {
    *(ProjectRegistryRights*)(unk00 + 0x1c) = arg2;
    return *(s32*)unk00;
  }
  return -1;
}

u8* _rwPluginRegistryReadDataChunks(u8* arg0, void* arg1, void* arg2) {
  u32 unk00;
  u32 unk04;
  u32 unk08;
  s32 unk0C;
  u8* unk10;
  if (RwStreamFindChunk(arg1, 3, &unk00, &unk04) == 0) {
    return 0;
  }
  if (unk04 >= 0x31000 && unk04 <= 0x32000) {
    while (unk00 != 0) {
      if (_rwStreamReadChunkHeader(arg1, &unk08, (u32*)&unk0C, 0, 0) == 0) {
        return 0;
      }
      unk10 = *(u8**)(arg0 + 0x10);
      while (unk10 != 0) {
        if (*(u32*)(unk10 + 8) == unk08) {
          break;
        }
        unk10 = *(u8**)(unk10 + 0x30);
      }
      if (unk10 != 0 && *(ProjectRegistryRead*)(unk10 + 0xC) != 0) {
        if ((*(ProjectRegistryRead*)(unk10 + 0xC))(arg1, unk0C, arg2,
            *(s32*)unk10, *(s32*)(unk10 + 4)) == 0) {
          return 0;
        }
      } else {
        if (RwStreamSkip(arg1, unk0C) == 0) {
          return 0;
        }
      }
      unk00 -= unk0C + 0xC;
    }
    unk10 = *(u8**)(arg0 + 0x10);
    while (unk10 != 0) {
      if (*(s32 (**)(void*, s32, s32))(unk10 + 0x18) != 0) {
        if ((*(s32 (**)(void*, s32, s32))(unk10 + 0x18))(arg2,
            *(s32*)unk10, *(s32*)(unk10 + 4)) == 0) {
          return 0;
        }
      }
      unk10 = *(u8**)(unk10 + 0x30);
    }
    return arg0;
  } else {
    BaerrState unk14;
    unk14.unk00 = 1;
    unk14.unk04 = _rwerror(0x80000004);
    RwErrorSet(&unk14);
    return 0;
  }
}

u8* _rwPluginRegistryInvokeRights(u8* arg0, u32 arg1, void* arg2, u32 arg3) {
  u8* unk00 = *(u8**)(arg0 + 0x10);
  while (unk00 != 0) {
    if (*(u32*)(unk00 + 8) == arg1) {
      break;
    }
    unk00 = *(u8**)(unk00 + 0x30);
  }
  if (unk00 != 0) {
    ProjectRegistryRights unk04 = *(ProjectRegistryRights*)(unk00 + 0x1C);
    if (unk04 != 0) {
      if (unk04(arg2, *(s32*)unk00, *(s32*)(unk00 + 4), arg3) != 0) {
        return arg0;
      }
      return 0;
    }
  }
  return 0;
}

u32 _rwPluginRegistryGetSize(u8* unk00, const void* unk04) {
  u32 unk08 = 0;
  u8* unk0C = *(u8**)(unk00 + 0x10);
  while (unk0C != 0) {
    ProjectRegistrySize unk10 = *(ProjectRegistrySize*)(unk0C + 0x14);
    if (unk10 != 0) {
      s32 unk14 = unk10(unk04, *(s32*)unk0C, *(s32*)(unk0C + 4));
      if (unk14 > 0) {
        unk08 += unk14 + 0xC;
      }
    }
    unk0C = *(u8**)(unk0C + 0x30);
  }
  return unk08;
}

u8* _rwPluginRegistryWriteDataChunks(u8* arg0, void* arg1, const void* arg2) {
  {
    u8* unk04;
    s32 unk00 = 0;
    unk04 = *(u8**)(arg0 + 0x10);
    while (unk04 != 0) {
      if (*(ProjectRegistrySize*)(unk04 + 0x14) != 0) {
        s32 unk08 = (*(ProjectRegistrySize*)(unk04 + 0x14))(arg2,
            *(s32*)unk04, *(s32*)(unk04 + 4));
        if (unk08 > 0) {
          unk00 = unk08 + unk00;
          unk00 += 0xC;
        }
      }
      unk04 = *(u8**)(unk04 + 0x30);
    }
    if (_rwStreamWriteVersionedChunkHeader(arg1, 3, unk00, 0x32000, 0xFFFF) == 0) {
      return 0;
    }
  }
  {
    u8* unk0C = *(u8**)(arg0 + 0x10);
    while (unk0C != 0) {
      if (*(ProjectRegistrySize*)(unk0C + 0x14) != 0 &&
          *(ProjectRegistryWrite*)(unk0C + 0x10) != 0) {
        s32 unk10 = (*(ProjectRegistrySize*)(unk0C + 0x14))(arg2,
            *(s32*)unk0C, *(s32*)(unk0C + 4));
        if (unk10 > 0) {
          if (_rwStreamWriteVersionedChunkHeader(arg1, *(u32*)(unk0C + 8),
              unk10, 0x32000, 0xFFFF) == 0) {
            return 0;
          }
          if ((*(ProjectRegistryWrite*)(unk0C + 0x10))(arg1, unk10,
              arg2, *(s32*)unk0C, *(s32*)(unk0C + 4)) == 0) {
            return 0;
          }
        }
      }
      unk0C = *(u8**)(unk0C + 0x30);
    }
  }
  return arg0;
}

u8* _rwPluginRegistrySkipDataChunks(u8* arg0, void* arg1) {
  u32 unk04;
  u32 unk00;
  if (RwStreamFindChunk(arg1, 3, &unk04, 0) == 0) {
    return 0;
  }
  while (unk04 != 0) {
    if (_rwStreamReadChunkHeader(arg1, 0, &unk00, 0, 0) == 0) {
      return 0;
    }
    if (RwStreamSkip(arg1, unk00) == 0) {
      return 0;
    }
    unk04 -= unk00 + 0xc;
  }
  return arg0;
}
