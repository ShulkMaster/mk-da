#include <renderware/project_error.h>
#include <renderware/project_binary.h>

extern u32 RwStreamRead(void* arg0, void* arg1, u32 arg2);

s32 _rwStreamReadChunkHeader(void* unk00, u32* unk04, u32* unk08,
    u32* unk0C, u32* unk10) {
  u32 unk14[3];
  BaerrState unk18;
  s32 unk1C = RwStreamRead(unk00, unk14, 12) == 12;
  if (!unk1C) {
    unk18.unk00 = 1;
    unk18.unk04 = _rwerror((s32)0x8000001A);
    RwErrorSet(&unk18);
    return 0;
  }
  RwMemNative32(unk14, 12);
  if (unk04 != 0) {
    *unk04 = unk14[0];
  }
  if (unk08 != 0) {
    *unk08 = unk14[1];
  }
  if ((unk14[2] & 0xFFFF0000) == 0) {
    if (unk0C != 0) {
      *unk0C = unk14[2] << 8;
    }
    if (unk10 != 0) {
      *unk10 = 0;
    }
  } else {
    if (unk0C != 0) {
      *unk0C = (((unk14[2] >> 14) & 0x3FF00) + 0x30000) |
          ((unk14[2] >> 16) & 0x3F);
    }
    if (unk10 != 0) {
      *unk10 = unk14[2] & 0xFFFF;
    }
  }
  return 1;
}

void* _rwStreamWriteVersionedChunkHeader(void* arg0, u32 arg1, u32 arg2, u32 arg3, u32 arg4) {
  u32 unk00[3];
  unk00[0] = arg1;
  unk00[1] = arg2;
  unk00[2] = (((arg3 - 0x30000) << 14) & 0xFFC00000) |
      ((arg3 & 0x3F) << 16) | (arg4 & 0xFFFF);
  RwMemLittleEndian32(unk00, sizeof(unk00));
  return RwStreamWrite(arg0, unk00, sizeof(unk00));
}

/* TODO: [near miss] 96.51%; two inlined version-pointer null checks remain. */
s32 RwStreamFindChunk(void* arg0, u32 arg1, u32* arg2, u32* arg3) {
  u32 unk00;
  u32 unk04;
  u32 unk08;
  while (_rwStreamReadChunkHeader(arg0, &unk00, &unk04, &unk08, 0)) {
    if (unk00 == arg1) {
      if (unk08 < 0x31000U) {
        BaerrState unk0C;
        unk0C.unk00 = 1;
        unk0C.unk04 = _rwerror((s32)0x80000004, unk08);
        RwErrorSet(&unk0C);
        return 0;
      }
      if (unk08 > 0x32000U) {
        BaerrState unk14;
        unk14.unk00 = 1;
        unk14.unk04 = _rwerror((s32)0x80000004, unk08);
        RwErrorSet(&unk14);
        return 0;
      }
      if (arg2 != 0) {
        *arg2 = unk04;
      }
      if (arg3 != 0) {
        *arg3 = unk08;
      }
      return 1;
    }
    if (RwStreamSkip(arg0, unk04) == 0) {
      return 0;
    }
  }
  return 0;
}

void* RwMemLittleEndian32(void* arg0, u32 arg1) {
  u32* unk00 = arg0;
  u32 unk08;
  u32 unk04 = arg1 >> 2;
  while (unk04 != 0) {
    unk08 = *unk00;
    *unk00 = (unk08 >> 24) | ((unk08 >> 8) & 0xff00) |
        ((unk08 << 8) & 0xff0000) | (unk08 << 24);
    ++unk00;
    --unk04;
  }
  return arg0;
}

#include <dolphin/types.h>

void* RwMemLittleEndian16(void* unk00, u32 unk04) {
  u16* unk0C = unk00;
  s32 unk10;
  u32 unk08 = unk04 >> 1;
  while (unk08 != 0) {
    unk10 = *unk0C;
    *unk0C = (unk10 >> 8) | (unk10 << 8);
    ++unk0C;
    --unk08;
  }
  return unk00;
}

void* RwMemNative32(void* unk00, u32 unk04) {
  u32* unk08 = unk00;
  u32 unk10;
  u32 unk0C = unk04 >> 2;
  while (unk0C != 0) {
    unk10 = *unk08;
    *unk08 = (unk10 >> 24) | ((unk10 >> 8) & 0xFF00) |
        ((unk10 << 8) & 0xFF0000) | (unk10 << 24);
    unk08++;
    unk0C--;
  }
  return unk00;
}

void* RwMemFloat32ToReal(void* unk00, u32 unk04) {
  f32* unk08 = unk00;
  unk04 >>= 2;
  while (unk04 != 0) {
    *unk08 = (f32)*unk08;
    ++unk08;
    --unk04;
  }
  return unk00;
}

#include <string.h>

void* RwStreamWriteReal(void* unk00, const u8* unk04, u32 unk08) {
  f32 unk0C[64];
  u32 unk14;
  while (unk08 != 0) {
    unk14 = unk08 < 256 ? unk08 : 256;
    memcpy(unk0C, unk04, unk14);
    RwMemFloat32ToReal(unk0C, unk14);
    {
      u32* unk20 = (u32*)unk0C;
      u32 unk24 = unk14 >> 2;
      while (unk24 != 0) {
        u32 unk28 = *unk20;
        *unk20 = (unk28 >> 24) | ((unk28 >> 8) & 0xFF00) |
            ((unk28 << 8) & 0xFF0000) | (unk28 << 24);
        unk20++;
        unk24--;
      }
    }
    if (RwStreamWrite(unk00, unk0C, unk14) == 0) {
      return 0;
    }
    unk08 -= unk14;
    unk04 += unk14;
  }
  return unk00;
}

#include <string.h>
extern void* RwStreamWrite(void* arg0, const void* arg1, u32 arg2);

void* RwStreamWriteInt32(void* arg0, const s32* arg1, u32 arg2) {
  u32 unk00[64];
  while (arg2 != 0) {
    u32 unk0C;
    if (arg2 < 0x100) {
      unk0C = arg2;
    } else {
      unk0C = 0x100;
    }
    memcpy(unk00, arg1, unk0C);
    {
      u32* unk10 = unk00;
      u32 unk14 = unk0C >> 2;
      while (unk14 != 0) {
        u32 unk18 = *unk10;
        *unk10 = (unk18 >> 24) | ((unk18 >> 8) & 0xFF00) |
            ((unk18 << 8) & 0xFF0000) | (unk18 << 24);
        unk10++;
        unk14--;
      }
    }
    if (RwStreamWrite(arg0, unk00, unk0C) == 0) {
      return 0;
    }
    arg2 -= unk0C;
    arg1 = (const s32*)((const u8*)arg1 + unk0C);
  }
  return arg0;
}

void* RwStreamReadReal(void* unk00, f32* unk04, u32 unk08) {
  if (RwStreamRead(unk00, unk04, unk08) == 0) {
    BaerrState unk0C;
    unk0C.unk00 = 1;
    unk0C.unk04 = _rwerror(0x8000001A);
    RwErrorSet(&unk0C);
    return 0;
  }
  {
    u32* unk10 = (u32*)unk04;
    u32 unk14 = unk08 >> 2;
    while (unk14 != 0) {
      u32 unk18 = *unk10;
      *unk10 = (unk18 >> 24) | ((unk18 >> 8) & 0xFF00) |
          ((unk18 << 8) & 0xFF0000) | (unk18 << 24);
      ++unk10;
      --unk14;
    }
  }
  RwMemFloat32ToReal(unk04, unk08);
  return unk00;
}

void* RwStreamReadInt32(void* unk00, void* unk04, u32 unk08) {
  if (RwStreamRead(unk00, unk04, unk08) == 0) {
    BaerrState unk0C;
    unk0C.unk00 = 1;
    unk0C.unk04 = _rwerror((s32)0x8000001A);
    RwErrorSet(&unk0C);
    return NULL;
  }
  {
    u32* unk10 = unk04;
    u32 unk18 = unk08 >> 2;
    u32 unk14;
    while (unk18 != 0) {
      unk14 = *unk10;
      *unk10 = (unk14 >> 24) | ((unk14 >> 8) & 0xFF00) |
          ((unk14 << 8) & 0xFF0000) | (unk14 << 24);
      ++unk10;
      --unk18;
    }
  }
  return unk00;
}
