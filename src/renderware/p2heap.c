#include <renderware/project_heap.h>
#include <renderware/project_state.h>
#include <renderware/project_error.h>
#include <string.h>

static void* HeapGrow(ProjectHeapState* unk00, u32 unk04) {
  ProjectHeapRange* unk08;
  ProjectHeapRange* unk14;
  ProjectHeapEntry* unk18;
  u32 unk0C = unk04 + 0x60;
  char unk10[256];
  if (unk0C < unk00->unk00) {
    unk0C = unk00->unk00;
  }
  unk08 = (*(ProjectHeapRange* (**)(u32))(RwEngineInstance + 0x130))(unk0C + 0x8B);
  if (unk08 != 0) {
    unk08->unk00 = (u8*)(((u32)unk08 + 0x8B) & ~0x7F);
    unk08->unk04 = unk0C;
    unk08->unk08 = 0;
  }
  if (unk08 != 0) {
    u32 unk1C;
    u32 unk48;
    s32 unk20;
    unk14 = unk00->unk04;
    unk18 = unk00->unk0C;
    unk1C = unk00->unk14;
    unk48 = unk00->unk10;
    if (unk48 <= unk1C) {
      unk18 = (*(ProjectHeapEntry* (**)(ProjectHeapEntry*, u32))(RwEngineInstance + 0x138))(unk18, (unk00->unk10 = unk48 + 0x20) * 8);
      if (unk18 == 0) {
        BaerrState unk24;
        unk24.unk00 = 1;
        unk24.unk04 = _rwerror(0x80000013, unk00->unk10 * 8);
        RwErrorSet(&unk24);
        unk00->unk10 -= 0x20;
      } else {
        if (unk18 != unk00->unk0C && unk1C != 0) {
          ProjectHeapEntry* unk2C = unk18;
          do {
            unk2C->unk04->unk0C = unk2C;
            --unk1C;
            ++unk2C;
          } while (unk1C != 0);
        }
        unk00->unk0C = unk18;
      }
    }
    if (unk18 != 0) {
      unk18 += unk00->unk14++;
    }
    if (unk18 != 0) {
      ProjectHeapBlock* unk38;
      ProjectHeapBlock* unk34;
      ProjectHeapBlock* unk30;
      unk30 = (ProjectHeapBlock*)unk08->unk00;
      unk34 = (ProjectHeapBlock*)((u8*)unk30 + 0x20);
      unk38 = (ProjectHeapBlock*)((u8*)unk30 + unk08->unk04 - 0x20);
      unk30->unk00 = 0;
      unk30->unk04 = 0;
      unk30->unk08 = 0;
      unk30->unk0C = 0;
      *unk38 = *unk30;
      unk30->unk04 = unk34;
      unk34->unk00 = unk30;
      unk34->unk04 = unk38;
      unk38->unk00 = unk34;
      unk34->unk08 = (u8*)unk38 - ((u8*)unk34 + 0x20);
      unk34->unk0C = unk18;
      unk18->unk04 = unk34;
      unk18->unk00 = unk34->unk08;
      if (unk14 != 0) {
        ProjectHeapBlock* unk3C = (ProjectHeapBlock*)(unk14->unk00 + unk14->unk04 - 0x20);
        unk3C->unk04 = unk30;
        unk30->unk00 = unk3C;
      }
      unk20 = 1;
    } else {
      unk20 = 0;
    }
    if (unk20) {
      u32 unk40 = 0;
      ProjectHeapRange* unk44;
      unk08->unk08 = unk00->unk04;
      unk00->unk04 = unk08;
      unk44 = unk00->unk04;
      while (unk44 != 0) {
        unk40 += unk44->unk04;
        unk44 = unk44->unk08;
      }
      (*(s32 (**)(char*, const char*, ...))(RwEngineInstance + 0xF0))(unk10, "Heap resized from %d to %d bytes", unk40 - unk0C, unk40);
      return unk08->unk00 + 0x20;
    }
    if (unk08 != 0) {
      (*(void (**)(void*))(RwEngineInstance + 0x134))(unk08);
    }
  }
  return 0;
}

static void* HeapReallocLarger(ProjectHeapState* unk00, void* unk04, u32 unk08, ProjectHeapBlock* unk0C, u32 unk10) {
  ProjectHeapBlock* unk14 = unk0C->unk04;
  if (unk14 != 0) {
    ProjectHeapEntry* unk18 = unk14->unk0C;
    if (unk18 != 0) {
      u32 unk1C = unk14->unk08 + 0x20;
      if (unk1C >= unk10) {
        u32 unk20 = unk0C->unk08 + unk1C;
        if (unk20 - unk08 < 0x100) {
          if (unk00->unk0C + (unk00->unk14 - 1) != unk18) {
            *unk18 = (unk00->unk0C + unk00->unk14)[-1];
            unk18->unk04->unk0C = unk18;
          }
          --unk00->unk14;
          unk0C->unk08 = unk20;
          unk0C->unk04 = unk0C->unk04->unk04;
          if (unk0C->unk04 != 0) {
            unk0C->unk04->unk00 = unk0C;
          }
        } else {
          ProjectHeapBlock* unk24 = (ProjectHeapBlock*)((u8*)unk0C + unk08);
          ProjectHeapBlock* unk34;
          unk24[1].unk0C = unk18;
          unk24[1].unk08 = unk0C->unk04->unk08 - unk10;
          unk24[1].unk00 = unk0C;
          unk24[1].unk04 = unk0C->unk04->unk04;
          if ((unk34 = (unk24++)[1].unk04) != 0) {
            unk34->unk00 = unk24;
          }
          unk0C->unk08 = unk08;
          unk0C->unk04 = unk24;
          unk24->unk0C->unk04 = unk24;
          unk24->unk0C->unk00 = unk24->unk08;
        }
        return unk04;
      }
    }
  }
  {
    void* unk2C = RxHeapAlloc(unk00, unk08);
    if (unk2C != 0) {
      memcpy(unk2C, unk04, unk0C->unk08);
      RxHeapFree(unk00, unk04);
      return unk2C;
    } else {
      BaerrState unk30;
      unk30.unk00 = 1;
      unk30.unk04 = _rwerror(0x80000013, unk08);
      RwErrorSet(&unk30);
    }
  }
  return 0;
}

/* TODO: [near miss] 99.74%; initial neighbor-check register allocation remains. */
void RxHeapFree(ProjectHeapState* unk00, void* unk04) {
  ProjectHeapBlock* unk08 = (ProjectHeapBlock*)((u8*)unk04 - 0x20);
  ProjectHeapBlock* unk14;
  ProjectHeapBlock* unk18;
  s32 unk10;
  s32 unk0C;
  unk0C = 0;
  unk14 = unk08->unk00;
  if (unk14 != 0 && unk14->unk0C != 0) {
    unk0C = 1;
  }
  unk10 = 0;
  unk18 = unk08->unk04;
  if (unk18 != 0 && unk18->unk0C != 0) {
    unk10 = 1;
  }
  if (unk0C) {
    if (unk10) {
      ProjectHeapEntry* unk1C = unk18->unk0C;
      if (unk00->unk0C + (unk00->unk14 - 1) != unk1C) {
        *unk1C = (unk00->unk0C + unk00->unk14)[-1];
        unk1C->unk04->unk0C = unk1C;
      }
      --unk00->unk14;
      unk08->unk00->unk08 += unk08->unk08 + unk08->unk04->unk08 + 0x40;
      unk08->unk00->unk0C->unk00 = unk08->unk00->unk08;
      unk08->unk00->unk04 = unk08->unk04->unk04;
      if (unk08->unk04->unk04 != 0) {
        unk08->unk04->unk04->unk00 = unk08->unk00;
      }
    } else {
      unk14->unk08 += unk08->unk08 + 0x20;
      unk08->unk00->unk0C->unk00 = unk08->unk00->unk08;
      unk08->unk00->unk04 = unk08->unk04;
      if (unk08->unk04 != 0) {
        unk08->unk04->unk00 = unk08->unk00;
      }
    }
  } else if (unk10) {
    unk08->unk08 += unk08->unk04->unk08 + 0x20;
    unk08->unk0C = unk08->unk04->unk0C;
    unk08->unk04->unk0C->unk04 = unk08;
    unk08->unk04->unk0C->unk00 = unk08->unk08;
    unk08->unk04 = unk08->unk04->unk04;
    if (unk08->unk04 != 0) {
      unk08->unk04->unk00 = unk08;
    }
  } else {
    u32 unk24;
    ProjectHeapEntry* unk20 = unk00->unk0C;
    u32 unk28;
    unk24 = unk00->unk14;
    unk28 = unk00->unk10;
    if (unk28 <= unk24) {
      unk20 = (*(ProjectHeapEntry* (**)(ProjectHeapEntry*, u32))
          (RwEngineInstance + 0x138))(unk20, (unk00->unk10 = unk28 + 0x20) * 8);
      if (unk20 == 0) {
        BaerrState unk2C;
        unk2C.unk00 = 1;
        unk2C.unk04 = _rwerror(0x80000013, unk00->unk10 * 8);
        RwErrorSet(&unk2C);
        unk00->unk10 -= 0x20;
      } else {
        if (unk20 != unk00->unk0C && unk24 != 0) {
          ProjectHeapEntry* unk34 = unk20;
          do {
            unk34->unk04->unk0C = unk34;
            --unk24;
            ++unk34;
          } while (unk24 != 0);
        }
        unk00->unk0C = unk20;
      }
    }
    if (unk20 != 0) {
      unk20 += unk00->unk14++;
    }
    if (unk20 != 0) {
      unk20->unk04 = unk08;
      unk20->unk00 = unk08->unk08;
      unk08->unk0C = unk20;
    }
  }
}

static inline void* projectHeapAllocEntry(ProjectHeapState* unk00, ProjectHeapEntry* unk0C, u32 unk08) {
  u32 unk14 = unk0C->unk00 - unk08;
  if (unk14 < 0x100) {
    ProjectHeapBlock* unk18 = unk0C->unk04;
    if (unk00->unk0C + (unk00->unk14 - 1) != unk0C) {
      *unk0C = (unk00->unk0C + unk00->unk14)[-1];
      unk0C->unk04->unk0C = unk0C;
    }
    --unk00->unk14;
    unk18->unk0C = 0;
    return unk18 + 1;
  }
  {
    ProjectHeapBlock* unk1C;
    ProjectHeapBlock* unk24;
    ProjectHeapBlock* unk18;
    ProjectHeapBlock* unk20;
    unk18 = unk0C->unk04;
    unk1C = (ProjectHeapBlock*)(unk08 + (u8*)unk18);
    ((unk1C + 1))->unk08 = unk14 - 0x20;
    ((unk1C + 1))->unk00 = unk18;
    ((unk1C + 1))->unk04 = unk18->unk04;
    ((unk1C + 1))->unk0C = unk0C;
    unk20 = (unk1C + 1);
    unk18->unk08 = unk08;
    unk18->unk04 = unk20;
    unk24 = unk20->unk04;
    if (unk24 != 0) {
      unk24->unk00 = unk20;
    }
    unk0C->unk04 = unk20;
    unk0C->unk00 = unk20->unk08;
    unk18->unk0C = 0;
    return unk18 + 1;
  }
}

void* RxHeapAlloc(ProjectHeapState* unk00, u32 unk04) {
  u32 unk10;
  u32 unk08;
  ProjectHeapEntry* unk0C;
  unk08 = (unk04 + 31) & ~31U;
  unk10 = unk00->unk14;
  unk0C = &unk00->unk0C[unk10 - 1];
  do {
    if (unk0C->unk00 >= unk08) {
      unk00->unk18 = 1;
      return projectHeapAllocEntry(unk00, unk0C, unk08);
    }
    --unk10;
    --unk0C;
  } while (unk10 != 0);
  {
    ProjectHeapBlock* unk28 = HeapGrow(unk00, unk08);
    if (unk28 != 0) {
      ProjectHeapEntry* unk30;
      unk00->unk18 = 1;
      unk30 = unk28->unk0C;
      return projectHeapAllocEntry(unk00, unk30, unk08);
    }
  }
  {
    BaerrState unk2C;
    unk2C.unk00 = 1;
    unk2C.unk04 = _rwerror(0x80000013, unk08);
    RwErrorSet(&unk2C);
  }
  return 0;
}

void* RxHeapRealloc(ProjectHeapState* arg0, void* arg1, u32 arg2, s32 arg3) {
  s32 unk08;
  ProjectHeapBlock* unk00;
  u32 unk04;
  ProjectHeapEntry* unk0C;
  u32 unk10;
  u32 unk14;
  unk00 = (ProjectHeapBlock*)((u8*)arg1 - 0x20);
  unk04 = (arg2 + 0x1F) & ~0x1F;
  unk08 = unk04 - unk00->unk08;
  if (unk08 > 0) {
    return HeapReallocLarger(arg0, arg1, unk04, unk00, unk08);
  }
  unk08 = -unk08;
  if (unk08 < 0x100) {
    return arg1;
  }
  unk0C = arg0->unk0C;
  unk10 = arg0->unk14;
  unk14 = arg0->unk10;
  if (unk14 <= unk10) {
    unk0C = (*(ProjectHeapEntry* (**)(ProjectHeapEntry*, u32))(RwEngineInstance + 0x138))(unk0C, (arg0->unk10 = unk14 + 0x20) * 8);
    if (unk0C == 0) {
      BaerrState unk18;
      unk18.unk00 = 1;
      unk18.unk04 = _rwerror(0x80000013, arg0->unk10 * 8);
      RwErrorSet(&unk18);
      arg0->unk10 -= 0x20;
    } else {
      if (unk0C != arg0->unk0C && unk10 != 0) {
        ProjectHeapEntry* unk1C = unk0C;
        do {
          unk1C->unk04->unk0C = unk1C;
          --unk10;
          ++unk1C;
        } while (unk10 != 0);
      }
      arg0->unk0C = unk0C;
    }
  }
  if (unk0C != 0) {
    unk0C += arg0->unk14++;
  }
  if (unk0C == 0) {
    return arg1;
  }
  {
    ProjectHeapBlock* unk20 = (ProjectHeapBlock*)((u8*)unk00 + unk04);
    ProjectHeapBlock* unk24;
    unk20[1].unk08 = unk08 - 0x20;
    unk20[1].unk00 = unk00;
    unk20[1].unk04 = unk00->unk04;
    unk20[1].unk0C = unk0C;
    unk20 = (ProjectHeapBlock*)((u8*)unk20 + 0x20);
    unk00->unk08 = unk04;
    unk00->unk04 = unk20;
    unk24 = unk20->unk04;
    if (unk24 != 0) {
      unk24->unk00 = unk20;
    }
    unk0C->unk04 = unk20;
    unk0C->unk00 = unk20->unk08;
  }
  return arg1;
}

static inline s32 projectHeapInitRange(ProjectHeapState* unk00,
    ProjectHeapRange* unk04, ProjectHeapRange* unk08) {
  ProjectHeapEntry* unk0C = unk00->unk0C;
  u32 unk10 = unk00->unk14;
  u32 unk14 = unk00->unk10;
  if (unk14 <= unk10) {
    unk0C = (*(ProjectHeapEntry* (**)(ProjectHeapEntry*, u32))(RwEngineInstance + 0x138))(
        unk0C, (unk00->unk10 = unk14 + 0x20) * 8);
    if (unk0C == 0) {
      BaerrState unk18;
      unk18.unk00 = 1;
      unk18.unk04 = _rwerror(0x80000013, unk00->unk10 * 8);
      RwErrorSet(&unk18);
      unk00->unk10 -= 0x20;
    } else {
      if (unk0C != unk00->unk0C && unk10 != 0) {
        ProjectHeapEntry* unk20 = unk0C;
        do {
          unk20->unk04->unk0C = unk20;
          --unk10;
          ++unk20;
        } while (unk10 != 0);
      }
      unk00->unk0C = unk0C;
    }
  }
  if (unk0C != 0) {
    unk0C += unk00->unk14++;
  }
  if (unk0C != 0) {
    ProjectHeapBlock* unk24;
    ProjectHeapBlock* unk28;
    ProjectHeapBlock* unk2C;
    unk24 = (ProjectHeapBlock*)unk04->unk00;
    unk28 = (ProjectHeapBlock*)((u8*)unk24 + 0x20);
    unk2C = (ProjectHeapBlock*)((u8*)unk24 + unk04->unk04 - 0x20);
    unk24->unk00 = 0;
    unk24->unk04 = 0;
    unk24->unk08 = 0;
    unk24->unk0C = 0;
    *unk2C = *unk24;
    unk24->unk04 = unk28;
    unk28->unk00 = unk24;
    unk28->unk04 = unk2C;
    unk2C->unk00 = unk28;
    unk28->unk08 = (u8*)unk2C - ((u8*)unk28 + 0x20);
    unk28->unk0C = unk0C;
    unk0C->unk04 = unk28;
    unk0C->unk00 = unk28->unk08;
    if (unk08 != 0) {
      ProjectHeapBlock* unk30 = (ProjectHeapBlock*)(unk08->unk00 + unk08->unk04 - 0x20);
      unk30->unk04 = unk24;
      unk24->unk00 = unk30;
    }
    return 1;
  }
  return 0;
}

s32 _rxHeapReset(ProjectHeapState* unk00) {
  ProjectHeapRange* unk08 = 0;
  ProjectHeapRange* unk04;
  unk00->unk14 = 0;
  unk04 = unk00->unk04->unk08;
  while (unk04 != 0) {
    if (!projectHeapInitRange(unk00, unk04, unk08)) {
      return 0;
    }
    if (unk08 == 0) {
      unk00->unk08 = (ProjectHeapBlock*)unk04->unk00;
    }
    unk08 = unk04;
    unk04 = unk04->unk08;
  }
  unk04 = unk00->unk04;
  if (!projectHeapInitRange(unk00, unk04, unk08)) {
    return 0;
  }
  if (unk08 == 0) {
    unk00->unk08 = (ProjectHeapBlock*)unk04->unk00;
  }
  unk00->unk18 = 0;
  return 1;
}

void RxHeapDestroy(ProjectHeapState* arg0) {
  if (arg0 != 0) {
    ProjectHeapRange* unk00;
    if (arg0->unk0C != 0) {
      (*(void (**)(void*))(RwEngineInstance + 0x134))(arg0->unk0C);
      arg0->unk0C = 0;
    }
    unk00 = arg0->unk04;
    while (unk00 != 0) {
      ProjectHeapRange* unk04 = unk00->unk08;
      if (unk00 != 0) {
        (*(void (**)(void*))(RwEngineInstance + 0x134))(unk00);
      }
      unk00 = unk04;
    }
    (*(void (**)(void*))(RwEngineInstance + 0x134))(arg0);
  }
}

ProjectHeapState* RxHeapCreate(u32 arg0) {
  ProjectHeapRange* unk04;
  ProjectHeapState* unk00;
  if (arg0 < 0x400) {
    arg0 = 0x400;
  }
  unk00 = (*(ProjectHeapState* (**)(u32))(RwEngineInstance + 0x130))(0x1C);
  if (unk00 != 0) {
    arg0 = (arg0 + 31) & ~31;
    if (arg0 < 0x80) {
      arg0 = 0x80;
    }
    unk04 = (*(ProjectHeapRange* (**)(u32))(RwEngineInstance + 0x130))(arg0 + 0x8B);
    if (unk04 != 0) {
      unk04->unk00 = (u8*)(((u32)unk04 + 0x8B) & ~0x7F);
      unk04->unk04 = arg0;
      unk04->unk08 = 0;
    }
    if (unk04 != 0) {
      s32 unk08;
      unk00->unk00 = arg0;
      unk00->unk04 = unk04;
      unk00->unk0C = 0;
      unk00->unk10 = 0;
      unk00->unk14 = 0;
      unk00->unk18 = 1;
      if (unk00->unk18 == 0) {
        unk08 = 1;
      } else {
        unk08 = _rxHeapReset(unk00);
      }
      if (unk08) {
        return unk00;
      }
      if (unk04 != 0) {
        (*(void (**)(void*))(RwEngineInstance + 0x134))(unk04);
      }
    }
    (*(void (**)(void*))(RwEngineInstance + 0x134))(unk00);
  }
  return 0;
}
