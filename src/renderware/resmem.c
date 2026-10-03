/* #audit 2026-10-03T04:06Z clean-room PASS (audit) */
#include <dolphin/types.h>

typedef struct ResmemBlock14 ResmemBlock14;
typedef struct {
  ResmemBlock14* unk0;
  ResmemBlock14* unk4;
} ResmemHeap8;

struct ResmemBlock14 {
  ResmemHeap8* unk0;
  ResmemBlock14* unk4;
  ResmemBlock14* unk8;
  u32 unkC;
  u32 unk10;
};

s32 _rwResHeapInit(ResmemHeap8* unk0, s32 unk4) {
  s32 unkC;
  ResmemBlock14* unk8;
  unk8 = (ResmemBlock14*)(((u32)unk0 + 0x27) & ~31U);
  unkC = (((u32)unk0 + unk4) & ~31U) - (u32)unk8 - 0x20;
  if (unkC < 0x20) return 0;
  unk8->unk0 = unk0;
  unk8->unk4 = 0;
  unk8->unk8 = 0;
  unk8->unk10 = 0;
  unk8->unkC = unkC;
  unk0->unk0 = unk8;
  unk0->unk4 = unk8;
  return 1;
}

s32 _rwResHeapClose(ResmemHeap8* unk0) {
  return 1;
}

void _rwResHeapFree(void* arg0) {
  ResmemBlock14* unk04;
  ResmemBlock14* unk08;
  ResmemBlock14* unk0C;
  ResmemHeap8* unk10;

  unk04 = (ResmemBlock14*)((u8*)arg0 - 0x20);
  unk04->unk10 = 0;
  unk08 = unk04->unk8;
  unk0C = unk04->unk4;
  unk10 = unk04->unk0;
  if (unk10->unk4 == 0 || unk04 < unk10->unk4) {
    unk10->unk4 = unk04;
  }
  if (unk08 != 0 && (~unk08->unk10 & 1)) {
    unk08->unk4 = unk0C;
    if (unk0C != 0) {
      unk0C->unk8 = unk08;
    }
    unk08->unkC += unk04->unkC + 0x20;
    unk04 = unk08;
  }
  if (unk0C != 0 && (~unk0C->unk10 & 1)) {
    unk04->unk4 = unk0C->unk4;
    if (unk0C->unk4 != 0) {
      unk0C->unk4->unk8 = unk04;
    }
    unk04->unkC += unk0C->unkC + 0x20;
  }
}

void *_rwResHeapAlloc(ResmemHeap8 *unk0, u32 unk4)
{
  ResmemBlock14 *unk8 = NULL;
  u32 unk10;
  ResmemBlock14 *unkC = unk0->unk4;

  while (unkC != NULL) {
    if ((~unkC->unk10 & 1) != 0) {
      unk10 = unkC->unkC;
      if (unk10 >= ((unk4 + 31) & ~31U) &&
          (unk8 == NULL || unk10 < unk8->unkC)) {
        unk8 = unkC;
      }
    }
    unkC = unkC->unk4;
  }
  if (unk8 == NULL) {
    return NULL;
  }
  if (unk8->unkC > ((unk4 + 31) & ~31U) + 0x40) {
    u32 unk14 = (unk4 + 31) & ~31U;
    ResmemBlock14 *unk1C;
    ResmemBlock14 *unk18 = (ResmemBlock14 *)((u8 *)unk8 + unk14 + 0x20);
    unk1C = unk8->unk4;
    if (unk1C != NULL && (~unk1C->unk10 & 1) != 0) {
      unk18->unk4 = unk1C->unk4;
      unk18->unkC = unk8->unk4->unkC + (unk8->unkC - unk14);
    } else {
      unk18->unk4 = unk1C;
      unk18->unkC = unk8->unkC - ((unk4 + 31) / 32 * 32) - 0x20;
    }
    unk8->unk4 = unk18;
    unk18->unk10 = 0;
    unk18->unk8 = unk8;
    if (unk18->unk4 != NULL) {
      unk18->unk4->unk8 = unk18;
    }
    unk8->unkC = (unk4 + 31) / 32 * 32;
    unk18->unk0 = unk8->unk0;
  }
  if (unk8 == unk0->unk4) {
    do {
      unk0->unk4 = unk0->unk4->unk4;
    } while (unk0->unk4 != NULL && (unk0->unk4->unk10 & 1) != 0);
  }
  unk8->unk10 = 1;
  return (u8 *)unk8 + 0x20;
}
