#include <renderware/project_state.h>
#include <renderware/project_binary.h>
#include <renderware/project_error.h>
#include <renderware/project_memory.h>
#include <string.h>
#include <stdio.h>


static s32 streamModule[2];

void* _rwStreamModuleOpen(void* unk00, s32 unk04, s32 unk08) {
  streamModule[0] = unk04;
  *(void**)(RwEngineInstance + streamModule[0]) = RwFreeListCreate(36, 10, 0);
  if (*(void**)(RwEngineInstance + streamModule[0]) == 0) {
    return 0;
  }
  streamModule[1]++;
  return unk00;
}

void* _rwStreamModuleClose(void* arg0, s32 arg1, s32 arg2) {
  void* unk00 = *(void**)(RwEngineInstance + streamModule[0]);
  if (unk00 != 0) {
    RwFreeListDestroy(unk00);
  }
  streamModule[1]--;
  return arg0;
}


u32 RwStreamRead(void* arg0, void* arg1, u32 arg2) {
  switch (*(s32*)arg0) {
    case 1:
    case 2: {
      void* unk00 = *(void**)((u8*)arg0 + 0xc);
      u32 unk04 = (*(u32 (**)(void*, u32, u32, void*))(RwEngineInstance + 0xd0))(
          arg1, 1, arg2, unk00);
      if (unk04 != arg2) {
        if ((*(s32 (**)(void*))(RwEngineInstance + 0xe0))(unk00) != 0) {
          BaerrState unk08;
          unk08.unk00 = 1;
          unk08.unk04 = _rwerror(5);
          RwErrorSet(&unk08);
        } else {
          BaerrState unk10;
          unk10.unk00 = 1;
          unk10.unk04 = _rwerror(0x8000001a);
          RwErrorSet(&unk10);
        }
      }
      return unk04;
    }
    case 3: {
      u32 unk18 = *(u32*)((u8*)arg0 + 0x10) - *(u32*)((u8*)arg0 + 0xc);
      if (arg2 > unk18) {
        BaerrState unk1c;
        arg2 = unk18;
        unk1c.unk00 = 1;
        unk1c.unk04 = _rwerror(5);
        RwErrorSet(&unk1c);
      }
      memcpy(arg1, *(u8**)((u8*)arg0 + 0x14) + *(u32*)((u8*)arg0 + 0xc), arg2);
      *(u32*)((u8*)arg0 + 0xc) += arg2;
      return arg2;
    }
    case 4:
      return (*(u32 (**)(void*, void*, u32))((u8*)arg0 + 0x10))(
          *(void**)((u8*)arg0 + 0x1c), arg1, arg2);
    default: {
      BaerrState unk24;
      unk24.unk00 = 1;
      unk24.unk04 = _rwerror(0xe);
      RwErrorSet(&unk24);
      return 0;
    }
  }
}


void* RwStreamWrite(void* unk00, const void* unk04, u32 unk08) {
  u32 unk1C;
  switch (*(s32*)unk00) {
  case 1:
  case 2: {
    void* unk10 = *(void**)((u8*)unk00 + 0x0C);
    u32 unk30 = (*(u32 (**)(const void*, u32, u32, void*))(RwEngineInstance + 0xD4))(
        unk04, 1, unk08, unk10);
    if (unk30 != unk08) {
      BaerrState unk0C;
      unk0C.unk00 = 1;
      unk0C.unk04 = _rwerror((s32)0x8000001C);
      RwErrorSet(&unk0C);
      return NULL;
    }
    return unk00;
  }
  case 3:
    if (*(void**)((u8*)unk00 + 0x14) == NULL) {
      *(void**)((u8*)unk00 + 0x14) =
          (*(void* (**)(u32))(RwEngineInstance + 0x130))(0x200);
      if (*(void**)((u8*)unk00 + 0x14) == NULL) {
        BaerrState unk14;
        unk14.unk00 = 1;
        unk14.unk04 = _rwerror((s32)0x80000013, 0x200);
        RwErrorSet(&unk14);
        return NULL;
      }
      *(u32*)((u8*)unk00 + 0x10) = 0x200;
    }
    if (*(u32*)((u8*)unk00 + 0x10) - *(u32*)((u8*)unk00 + 0x0C) < unk08) {
      void* unk20;
      if (unk08 < 0x200) {
        unk1C = *(u32*)((u8*)unk00 + 0x10) + 0x200;
      } else {
        unk1C = unk08 + *(u32*)((u8*)unk00 + 0x10);
      }
      unk20 = (*(void* (**)(void*, u32))(RwEngineInstance + 0x138))(
          *(void**)((u8*)unk00 + 0x14), unk1C);
      if (unk20 == NULL) {
        BaerrState unk24;
        unk24.unk00 = 1;
        unk24.unk04 = _rwerror((s32)0x80000013,
            unk1C - *(u32*)((u8*)unk00 + 0x10));
        RwErrorSet(&unk24);
        return NULL;
      }
      *(void**)((u8*)unk00 + 0x14) = unk20;
      *(u32*)((u8*)unk00 + 0x10) = unk1C;
    }
    memcpy(*(u8**)((u8*)unk00 + 0x14) + *(u32*)((u8*)unk00 + 0x0C), unk04, unk08);
    *(u32*)((u8*)unk00 + 0x0C) += unk08;
    return unk00;
  case 4:
    if ((*(s32 (**)(void*, const void*, u32))((u8*)unk00 + 0x14))(
            *(void**)((u8*)unk00 + 0x1C), unk04, unk08) != 0) {
      return unk00;
    }
    return NULL;
  default:
    {
      BaerrState unk2C;
      unk2C.unk00 = 1;
      unk2C.unk04 = _rwerror(0x0E);
      RwErrorSet(&unk2C);
    }
    return NULL;
  }
}


void* RwStreamSkip(u8* unk00, u32 unk04) {
  if (unk04 == 0) {
    return unk00;
  }
  switch (*(s32*)unk00) {
    case 1:
    case 2: {
      void* unk08 = *(void**)(unk00 + 0x0C);
      if ((*(s32 (**)(void*, u32, s32))(RwEngineInstance + 0xE4))(
          unk08, unk04, 1) != 0) {
        if ((*(s32 (**)(void*))(RwEngineInstance + 0xE0))(unk08) != 0) {
          BaerrState unk0C;
          unk0C.unk00 = 1;
          unk0C.unk04 = _rwerror(5);
          RwErrorSet(&unk0C);
        }
        return 0;
      }
      return unk00;
    }
    case 3: {
      u32 unk14 = *(u32*)(unk00 + 0x0C) + unk04;
      u32 unk24 = *(u32*)(unk00 + 0x10);
      if (unk14 > unk24) {
        BaerrState unk18;
        *(u32*)(unk00 + 0x0C) = unk24;
        unk18.unk00 = 1;
        unk18.unk04 = _rwerror(5);
        RwErrorSet(&unk18);
        return 0;
      }
      *(u32*)(unk00 + 0x0C) = unk14;
      return unk00;
    }
    case 4:
      if ((*(s32 (**)(void*, u32))(unk00 + 0x18))(
          *(void**)(unk00 + 0x1C), unk04) != 0) {
        return unk00;
      }
      return 0;
    default: {
      BaerrState unk20;
      unk20.unk00 = 1;
      unk20.unk04 = _rwerror(0x0E);
      RwErrorSet(&unk20);
      return 0;
    }
  }
}

s32 RwStreamClose(u8* unk00, u8* unk04) {
  s32 unk08;
  switch (*(s32*)(unk00 + 0)) {
  case 1:
    unk08 = 1;
    break;
  case 2:
    unk08 = !(*(int (**)(FILE*))(RwEngineInstance + 0xCC))(
        *(FILE**)(unk00 + 0x0C));
    break;
  case 3:
    if (*(s32*)(unk00 + 4) != 1 && unk04 != 0) {
      *(u32*)(unk04 + 0) = *(u32*)(unk00 + 0x14);
      *(void**)(unk04 + 4) = *(void**)(unk00 + 0x0C);
    }
    unk08 = 1;
    break;
  case 4: {
    void (*unk0C)(void*) = *(void (**)(void*))(unk00 + 0x0C);
    if (unk0C != 0) {
      unk0C(*(void**)(unk00 + 0x1C));
    }
    unk08 = 1;
    break;
  }
  default: {
    BaerrState unk10;
    unk10.unk00 = 1;
    unk10.unk04 = _rwerror(0x0E);
    RwErrorSet(&unk10);
    return 0;
  }
  }
  if (*(s32*)(unk00 + 0x20) != 0) {
    (*(void* (**)(void*, void*))(RwEngineInstance + 0x144))(
        *(void**)(RwEngineInstance + streamModule[0]), unk00);
  }
  return unk08;
}


static inline u8* projectStreamOpenFill(s32 unk00, s32 unk04, const void* unk08, u8* unk0C) {
  typedef void* (*ProjectStreamOpen)(const void*, const char*);
  BaerrState unk20;
  BaerrState unk18;
  BaerrState unk1C;
  BaerrState unk24;
  u8* unk10;
  void* unk14;
  unk10 = 0;
  if (unk0C == 0) {
    return unk10;
  }
  {
    *(s32*)unk0C = unk00;
    *(s32*)(unk0C + 4) = unk04;
    *(s32*)(unk0C + 0x20) = 1;
    switch (unk00) {
    case 1:
      *(const void**)(unk0C + 0xC) = unk08;
      unk10 = unk0C;
      break;
    case 2: {
      unk14 = unk10;
      switch (unk04) {
      case 1:
        unk14 = (*(ProjectStreamOpen*)(RwEngineInstance + 0xC8))(
            unk08, "rb");
        break;
      case 2:
        unk14 = (*(ProjectStreamOpen*)(RwEngineInstance + 0xC8))(
            unk08, "wb");
        break;
      case 3:
        unk14 = (*(ProjectStreamOpen*)(RwEngineInstance + 0xC8))(
            unk08, "ab");
        break;
      default: {
        unk18.unk00 = 1;
        unk18.unk04 = _rwerror(13);
        RwErrorSet(&unk18);
        break;
      }
      }
      if (unk14 != 0) {
        *(void**)(unk0C + 0xC) = unk14;
        unk10 = unk0C;
      } else {
        unk1C.unk00 = 1;
        unk1C.unk04 = _rwerror((s32)0x80000002, unk08);
        RwErrorSet(&unk1C);
      }
      break;
    }
    case 3:
      switch (unk04) {
      case 1:
        *(u32*)(unk0C + 0xC) = 0;
        *(u32*)(unk0C + 0x10) = *(const u32*)((const u8*)unk08 + 4);
        *(const void**)(unk0C + 0x14) = *(const void**)unk08;
        unk10 = unk0C;
        break;
      case 2:
        *(u32*)(unk0C + 0xC) = 0;
        *(u32*)(unk0C + 0x10) = 0;
        *(void**)(unk0C + 0x14) = 0;
        unk10 = unk0C;
        break;
      case 3:
        *(u32*)(unk0C + 0xC) = *(const u32*)((const u8*)unk08 + 4);
        *(u32*)(unk0C + 0x10) = *(const u32*)((const u8*)unk08 + 4);
        *(const void**)(unk0C + 0x14) = *(const void**)unk08;
        unk10 = unk0C;
        break;
      default: {
        unk20.unk00 = 1;
        unk20.unk04 = _rwerror(13);
        RwErrorSet(&unk20);
        break;
      }
      }
      break;
    case 4:
      memcpy(unk0C + 0xC, unk08, 20);
      unk10 = unk0C;
      break;
    default: {
      unk24.unk00 = 1;
      unk24.unk04 = _rwerror(14);
      RwErrorSet(&unk24);
      break;
    }
    }
  }
  return unk10;
}

void* RwStreamOpen(s32 unk00, s32 unk04, const void* unk08) {
  typedef u8* (*ProjectStreamAlloc)(void*);
  typedef void* (*ProjectStreamFree)(void*, void*);
  u8* unk10;
  u8* unk0C = (*(ProjectStreamAlloc*)(RwEngineInstance + 0x140))(
      *(void**)(RwEngineInstance + streamModule[0]));
  unk10 = projectStreamOpenFill(unk00, unk04, unk08, unk0C);
  if (unk10 == 0) {
    (*(ProjectStreamFree*)(RwEngineInstance + 0x144))(
        *(void**)(RwEngineInstance + streamModule[0]), unk0C);
    unk0C = 0;
  }
  return unk0C;
}
