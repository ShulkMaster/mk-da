#include <dolphin/types.h>
#include <string.h>
#include <renderware/project_error.h>
#include <renderware/project_memory.h>

extern void* malloc(size_t arg0);
extern void free(void* arg0);
extern void* realloc(void* arg0, size_t arg1);

static u8* llGAllFreeLists[2];

static void* FakeCalloc(u32 arg0, u32 arg1) {
  u32 unk04 = arg0 * arg1;
  void* unk00 = malloc(unk04);
  if (unk00 != 0) {
    memset(unk00, 0, unk04);
  }
  return unk00;
}

static int FreeListPurgeSort(const void* arg0, const void* arg1) {
  return *(const u32*)arg0 - *(const u32*)arg1;
}

#include <renderware/project_state.h>

/* TODO: [near miss] 95.37%; stride-product reuse and register scheduling remain. */
static s32 FreeContiguous(u8* arg0) {
  u32 unk00 = 0;
  u8* unk04 = 0;
  u32* unk08;
  u8* unk0C = *(u8**)(arg0 + 8);
  unk08 = *(u32**)arg0;
  while (unk0C != 0) {
    s32 unk28;
    u32 unk10 = *(u32*)(arg0 + 0xC);
    u32 unk14 = *(u32*)(arg0 + 0x10);
    u32 unk18 = *(u32*)(arg0 + 0x14);
    u32 unk1C = *(u32*)(arg0 + 0x18);
    s32 unk20 = 0;
    u32 unk24 = ((u32)unk0C + (unk10 - unk14)) & ~unk18;
    unk24 -= (unk1C - 1) * unk14;
    unk28 = 1;
    while (unk08 <= *(u32**)(arg0 + 4)) {
      u32 unk2C = *unk08;
      u32 unk40 = unk2C - unk24;
      unk2C |= ~unk24;
      unk20 = (unk2C - (unk40 >> 1)) >> 31;
      if (unk20 != 0) {
        break;
      }
      ++unk08;
    }
    if (unk20 == 0) {
      unk0C = 0;
    } else if (*unk08 == unk24) {
      u32* unk30 = unk08 + (unk1C - 1);
      if (unk08 + (unk1C - 1) <= *(u32**)(arg0 + 4) && *unk30 == unk24 + ((unk1C - 1) * unk14)) {
        u8* unk34 = *(u8**)unk0C;
        u32* unk38;
        (*(void (**)(void*))(RwEngineInstance + 0x134))(unk0C);
        if (unk04 == 0) {
          *(u8**)(arg0 + 8) = unk34;
        } else {
          *(u8**)unk04 = unk34;
        }
        unk0C = unk34;
        unk28 = 0;
        unk00 += unk10;
        unk38 = unk08;
        while (unk38 + unk1C <= *(u32**)(arg0 + 4)) {
          *unk38 = *(unk38 + unk1C);
          ++unk38;
        }
        *(u32**)(arg0 + 4) -= unk1C;
        *(u32*)(arg0 + 0x1C) -= unk1C;
      }
    }
    if (unk0C != 0 && unk28 != 0) {
      unk04 = unk0C;
      unk0C = *(u8**)unk0C;
    }
  }
  return unk00;
}

#include <renderware/project_state.h>

s32 RwFreeListDestroy(void* unk00) {
  void* unk04 = *(void**)unk00;
  void* unk0C;
  if (unk04 != 0) {
    (*(void (**)(void*))(RwEngineInstance + 0x134))(unk04);
  }
  unk0C = *(void**)((u8*)unk00 + 0x08);
  while (unk0C != 0) {
    void* unk08 = *(void**)unk0C;
    (*(void (**)(void*))(RwEngineInstance + 0x134))(unk0C);
    unk0C = unk08;
  }
  *(void**)*(void**)((u8*)unk00 + 0x24) =
      *(void**)((u8*)unk00 + 0x20);
  *(void**)((u8*)*(void**)((u8*)unk00 + 0x20) + 4) =
      *(void**)((u8*)unk00 + 0x24);
  (*(void (**)(void*))(RwEngineInstance + 0x134))(unk00);
  return 1;
}

static u8* FreeListCreate(s32 arg0, s32 arg1, s32 arg2) {
  u8* unk00 = (*(void* (**)(u32))(RwEngineInstance + 0x130))(0x28);
  if (unk00 != 0) {
    s32 unk04 = arg2 > 1 ? arg2 : 1;
    *(s32*)(unk00 + 0x14) = unk04 - 1;
    arg0 += *(s32*)(unk00 + 0x14);
    arg0 &= ~*(s32*)(unk00 + 0x14);
    *(s32*)(unk00 + 0x10) = arg0;
    *(s32*)(unk00 + 0x18) = arg1;
    *(s32*)(unk00 + 0xC) = arg0 * arg1 + 4;
    *(s32*)(unk00 + 0xC) += *(s32*)(unk00 + 0x14);
    *(void**)unk00 = (*(void* (**)(u32))(RwEngineInstance + 0x130))(4);
    if (*(void**)unk00 != 0) {
      *(u8**)(unk00 + 4) = *(u8**)unk00 - 4;
      *(s32*)(unk00 + 0x1C) = 0;
      *(void**)(unk00 + 8) = 0;
      *(u8**)(unk00 + 0x20) = llGAllFreeLists[0];
      *(u8**)(unk00 + 0x24) = (u8*)llGAllFreeLists;
      *(u8**)(llGAllFreeLists[0] + 4) = unk00 + 0x20;
      llGAllFreeLists[0] = unk00 + 0x20;
    } else {
      BaerrState unk08;
      unk08.unk00 = 1;
      unk08.unk04 = _rwerror(0x80000013, 4);
      RwErrorSet(&unk08);
      (*(void (**)(void*))(RwEngineInstance + 0x134))(unk00);
      unk00 = 0;
    }
  } else {
    BaerrState unk0C;
    unk0C.unk00 = 1;
    unk0C.unk04 = _rwerror(0x80000013, 0x28);
    RwErrorSet(&unk0C);
    unk00 = 0;
  }
  return unk00;
}

u8* RwFreeListCreate(s32 unk00, s32 unk04, s32 unk08) {
  return FreeListCreate(unk00, unk04, unk08);
}

void* _rwFreeListFreeReal(u8* unk00, void* unk04) {
  *++*(void***)(unk00 + 4) = unk04;
  return unk00;
}

void* _rwFreeListAllocReal(u8* arg0) {
  if (*(void***)(arg0 + 4) < *(void***)arg0) {
    u8* unk00 = (*(void* (**)(u32))(RwEngineInstance + 0x130))(
        *(u32*)(arg0 + 0xc));
    void** unk08;
    s32 unk04;
    u8* unk0c;
    s32 unk10;
    if (unk00 == 0) {
      BaerrState unk14;
      unk14.unk00 = 1;
      unk14.unk04 = _rwerror(0x80000013, *(s32*)(arg0 + 0xc));
      RwErrorSet(&unk14);
      return 0;
    }
    unk04 = (*(s32*)(arg0 + 0x1c) + *(s32*)(arg0 + 0x18)) * 4;
    unk08 = (*(void* (**)(u32))(RwEngineInstance + 0x130))(unk04);
    if (unk08 == 0) {
      BaerrState unk1c;
      unk1c.unk00 = 1;
      unk1c.unk04 = _rwerror(0x80000013, unk04);
      RwErrorSet(&unk1c);
      (*(void (**)(void*))(RwEngineInstance + 0x134))(unk00);
      return 0;
    }
    (*(void (**)(void*))(RwEngineInstance + 0x134))(*(void**)arg0);
    *(void***)arg0 = unk08;
    *(s32*)(arg0 + 0x1c) += *(s32*)(arg0 + 0x18);
    *(u8**)unk00 = *(u8**)(arg0 + 8);
    *(u8**)(arg0 + 8) = unk00;
    unk0c = (u8*)(((u32)unk00 +
        (*(u32*)(arg0 + 0xc) - *(u32*)(arg0 + 0x10))) &
        ~*(u32*)(arg0 + 0x14));
    unk10 = *(s32*)(arg0 + 0x18);
    while (unk10-- != 0) {
      *unk08 = unk0c;
      unk0c -= *(u32*)(arg0 + 0x10);
      ++unk08;
    }
    *(void***)(arg0 + 4) = *(void***)arg0 + (*(s32*)(arg0 + 0x18) - 1);
  }
  {
    void** unk24 = *(void***)(arg0 + 4);
    *(void***)(arg0 + 4) = unk24 - 1;
    return *unk24;
  }
}

#include <renderware/project_sort.h>

/* TODO: [breakthrough needed] 83.32%; outer accumulator source shape remains. */
s32 RwFreeListPurgeAllFreeLists(void) {
  s32 unk00 = 0;
  u8* unk0C;
  u8* unk08;
  u8* unk04 = llGAllFreeLists[0];
  unk08 = (u8*)llGAllFreeLists;
  while (unk04 != unk08) {
    s32 unk10;
    u32* unk14;
    u32* unk18;
    unk0C = unk04 - 0x20;
    unk10 = 0;
    unk14 = *(u32**)(unk0C + 4);
    unk18 = *(u32**)unk0C;
    if (unk14 >= unk18) {
      qsort(unk18, (((u32)unk14 - (u32)unk18) >> 2) + 1,
          sizeof(*unk18), FreeListPurgeSort);
      {
        u8* unk28;
        u8* unk24;
        s32 unk1C;
        u8** unk20;
        do {
          unk20 = (u8**)(unk0C + 8);
          unk24 = *(u8**)(unk0C + 8);
          unk1C = 0;
          while ((unk28 = *(u8**)unk24) != 0) {
            if (unk28 < unk24) {
              unk1C = 1;
              *unk20 = unk28;
              *(u8**)unk24 = *(u8**)unk28;
              *(u8**)unk28 = unk24;
              unk20 = (u8**)unk28;
            } else {
              unk20 = (u8**)unk24;
              unk24 = unk28;
            }
          }
        } while (unk1C != 0);
      }
      unk10 = FreeContiguous(unk0C);
    }
    if (unk10 > 0) {
      unk00 += unk10;
    }
    unk04 = *(u8**)unk04;
  }
  return unk00;
}


void* RwFreeListForAllUsed(void* unk00, void (*unk04)(void*, void*), void* unk08) {
  u32 unk14;
  u8* unk10;
  s32 unk0C;
  unk10 = *(u8**)((u8*)unk00 + 0x08);
  while (unk10 != NULL) {
    unk14 = (u32)unk10 + *(u32*)((u8*)unk00 + 0x14);
    unk14 += 4;
    unk14 &= ~*(u32*)((u8*)unk00 + 0x14);
    unk0C = *(s32*)((u8*)unk00 + 0x18);
    while (unk0C-- != 0) {
      void** unk18 = *(void***)unk00;
      while (unk18 <= *(void***)((u8*)unk00 + 0x04)) {
        if (*unk18 == (void*)unk14) {
          break;
        }
        ++unk18;
      }
      if (unk18 > *(void***)((u8*)unk00 + 0x04)) {
        unk04((void*)unk14, unk08);
      }
      unk14 += *(u32*)((u8*)unk00 + 0x10);
    }
    unk10 = *(u8**)unk10;
  }
  return unk00;
}

void _rwMemoryClose(void) {
}

s32 _rwMemoryOpen(const void* unk00) {
  typedef struct {
    void* (*unk00)(size_t);
    void (*unk04)(void*);
    void* (*unk08)(void*, size_t);
    void* (*unk0C)(u32, u32);
  } ProjectMemoryCallbacks;
  llGAllFreeLists[0] = (u8*)llGAllFreeLists;
  llGAllFreeLists[1] = (u8*)llGAllFreeLists;
  if (unk00 != 0) {
    *(ProjectMemoryCallbacks*)(RwEngineInstance + 0x130) =
        *(const ProjectMemoryCallbacks*)unk00;
  } else {
    *(void* (**)(size_t))(RwEngineInstance + 0x130) = malloc;
    *(void (**)(void*))(RwEngineInstance + 0x134) = free;
    *(void* (**)(void*, size_t))(RwEngineInstance + 0x138) = realloc;
    *(void* (**)(u32, u32))(RwEngineInstance + 0x13C) = FakeCalloc;
  }
  return 1;
}
