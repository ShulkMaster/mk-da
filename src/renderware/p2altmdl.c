#include <dolphin/types.h>

u8 _rxExecCtxGlobal[0x18];

typedef struct {
  u32 unk00;
  u8* unk04;
  u32 unk08;
  u32 unk0C;
  u32 unk10;
  u32* unk14;
  u32 unk18;
} ProjectPacketWords;

typedef struct {
  u8 unk00[0x14];
  ProjectPacketWords unk14[];
} ProjectPacket;

void _rxPacketDestroy(u8*);

/* TODO: [near miss] 99.89%; source-offset register allocation remains. */
void _rxEmbeddedPacketBetweenPipelines(u8* unk00, u8* unk04) {
  if (*(s32*)(unk00 + 0x10) > 1) {
    u8* unk08 = *(u8**)(unk00 + 0x14);
    u8* unk0C = *(u8**)(unk04 + 0x14);
    u32 unk10;
    for (unk10 = 0; unk10 < *(u32*)(unk04 + 0x18); ++unk10) {
      u32 unk18;
      u32 unk2C;
      u8* unk14 = *(u8**)(unk04 + 0x1C) + unk10 * 0xC;
      for (unk18 = 0; unk18 < (unk2C = *(u16*)(unk08 + 2)); ++unk18) {
        ProjectPacket* unk24 = (ProjectPacket*)(unk08 + unk18 * 0x1C);
        u32* unk1C = unk24->unk14[0].unk14;
        if (unk1C != 0 && *unk1C == *(u32*)unk14) {
          break;
        }
      }
      if (unk18 != unk2C) {
        u32* unk38;
        u32 unk34;
        u8* unk20 = unk08 + unk18 * 0x1C;
        if (((ProjectPacket*)unk20)->unk14[0].unk04 != 0) {
          ProjectPacket* unk30 = (ProjectPacket*)(unk0C + *(u32*)(unk14 + 8) * 0x1C);
          unk30->unk14[0] = ((ProjectPacket*)unk20)->unk14[0];
          {
            unk34 = *(u32*)(unk14 + 8);
            unk38 = ((u32**)*(u8**)(*(u8**)(unk04 + 8) + 0xC))[unk34];
            unk34 *= 0x1C;
            {
              ProjectPacket* unk40 = (ProjectPacket*)(unk0C + unk34);
              unk40->unk14[0].unk14 = unk38;
            }
          }
          ((ProjectPacket*)unk20)->unk14[0].unk04 = 0;
        }
      }
    }
    _rxPacketDestroy(unk08);
    *(u8**)(unk0C + 0x10) = *(u8**)(*(u8**)(unk04 + 8) + 0xC);
    *(u8**)(unk0C + 8) = *(u8**)(*(u8**)(unk04 + 8) + 0x18);
    *(u8**)(unk0C + 0xC) = *(u8**)(*(u8**)(unk04 + 8) + 0x10);
    *(s32*)(unk04 + 0x10) = 3;
  }
}

#include <renderware/project_heap.h>

extern ProjectHeapState* _rxHeapGlobal;

static inline void projectPacketReleaseData(ProjectPacketWords* unk00) {
  if (unk00->unk04 != 0 && (*(u16*)unk00 & 2) == 0) {
    RxHeapFree(_rxHeapGlobal, unk00->unk04);
  }
}

/* TODO: [near miss] 99.18%; cleanup pointer address normalization remains. */
u8* _rxEmbeddedPacketBetweenNodes(u8* unk00, u8* unk04, u32 unk08) {
  u8* unk18;
  u8* unk10;
  u8* unk28;
  u32 unk14;
  u32 unk0C = ((u32*)*(u8**)(unk04 + 8))[unk08];
  if (unk0C != 0xFFFFFFFFU) {
    unk10 = *(u8**)(unk00 + 8) + unk0C * 0x28;
    if (*(s32*)(unk00 + 0x10) > 1) {
      unk14 = 1U << unk08;
      unk18 = *(u8**)(unk00 + 0x14);
      if ((**(u32**)(unk18 + 0xC) & unk14) == 0) {
        u32 unk1C = *(u16*)(unk18 + 2);
        do {
          if ((((u32*)*(u8**)(unk18 + 0xC))[unk1C] & unk14) == 0) {
            u32 unk20 = unk1C * 0x1C;
            u8* unk24 = unk18 + unk20;
            if (*(u8**)(unk24 + 0xC) != 0) {
              unk28 = unk24 - 8;
              projectPacketReleaseData((ProjectPacketWords*)unk28);
              *(u32*)(unk18 + (unk1C - 1) * 0x1C + 0x14) = 0;
              ((ProjectPacketWords*)unk28)->unk04 = 0;
              ((ProjectPacket*)unk18)->unk14[unk1C - 1].unk0C = 0;
              ((ProjectPacket*)unk18)->unk14[unk1C - 1].unk10 = 0;
              ((ProjectPacketWords*)unk28)->unk14 = 0;
            }
          }
        } while (--unk1C != 0);
      }
      *(u8**)(unk18 + 8) = *(u8**)(unk10 + 0x18);
      *(u8**)(unk18 + 0xC) = *(u8**)(unk10 + 0x10);
      *(u8**)(unk18 + 0x10) = *(u8**)(unk10 + 0xC);
      *(s32*)(unk00 + 0x10) = 3;
    }
    return unk10;
  }
  return 0;
}

void _rxPacketDestroy(u8* unk00) {
  u32 unk08;
  u8* unk04;
  u8* unk10 = *(u8**)(unk00 + 4);
  *(s32*)(unk10 + 0x10) = 1;
  unk08 = *(u16*)(unk00 + 2);
  unk04 = unk00 + 0x14;
  do {
    if (*(u32*)(unk04 + 0x14) != 0) {
      projectPacketReleaseData((ProjectPacketWords*)unk04);
      *(u32*)unk04 = 0;
      *(void**)(unk04 + 4) = 0;
      *(u32*)(unk04 + 0x0C) = 0;
      *(u32*)(unk04 + 0x10) = 0;
      *(u32*)(unk04 + 0x14) = 0;
    }
    --unk08;
    unk04 += 0x1C;
  } while (unk08 != 0);
  *(u16*)unk00 = 0;
}
