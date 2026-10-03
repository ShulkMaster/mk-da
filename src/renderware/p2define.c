#include <dolphin/types.h>
#include <renderware/project_pipeline.h>
#include <renderware/project_error.h>

static u32 gMemoryLimits[2] = {0, 0};

/* TODO: [near miss] 99.11%; aligned-count and limit-address registers are swapped. */
u8* StalacTiteAlloc(u32 unk00) {
  u32 unk04 = (unk00 + 3) & ~3U;
  gMemoryLimits[0] -= unk04;
  if (gMemoryLimits[0] < gMemoryLimits[1]) {
    BaerrState unk08;
    gMemoryLimits[0] += unk04;
    unk08.unk00 = 1;
    unk08.unk04 = _rwerror(0x80000013);
    RwErrorSet(&unk08);
    return 0;
  }
  return (u8*)gMemoryLimits[0];
}

u8* StalacMiteAlloc(u32 unk00) {
  u32 unk04 = (unk00 + 3) & ~3U;
  gMemoryLimits[1] += unk04;
  if (gMemoryLimits[1] > gMemoryLimits[0]) {
    BaerrState unk08;
    gMemoryLimits[1] -= (unk00 + 3) & ~3U;
    unk08.unk00 = 1;
    unk08.unk04 = _rwerror(0x80000013);
    RwErrorSet(&unk08);
    return 0;
  }
  return (u8*)(gMemoryLimits[1] - ((unk00 + 3) & ~3U));
}

s32 PipelineCalcNumUniqueClusters(u8* unk00) {
  u32 unk0C;
  u32 unk08;
  s32 unk04 = 0;
  u32 unk10;
  u32 unk14;
  unk08 = 0;
  for (;;) {
    unk0C = unk08;
    unk08 = 0xFFFFFFFF;
    for (unk10 = 0; unk10 < *(u32*)(unk00 + 4); ++unk10) {
      u8* unk18 = *(u8**)(*(u8**)(unk00 + 8) + unk10 * 0x28);
      for (unk14 = 0; unk14 < *(u32*)(unk18 + 0x20); ++unk14) {
        u32 unk1C = *(u32*)(*(u8**)(unk18 + 0x24) + unk14 * 12);
        if (unk1C > unk0C && unk1C < unk08) {
          unk08 = unk1C;
        }
      }
    }
    if (unk08 == 0xFFFFFFFF) {
      break;
    }
    ++unk04;
  }
  return unk04;
}

/* TODO: [breakthrough] 80.59%; node-field addressing and loop registers remain. */
#include <string.h>
#include <renderware/project_pipeline.h>
#include <renderware/project_state.h>

static s32 LockPipelineExpandData(u8* arg0, const u8* arg1) {
  u32 unk00;
  u32 unk04;
  u8* unk08;
  if (arg0 != arg1) {
    for (unk00 = *(const u32*)(arg1 + 4) - 1; (s32)unk00 >= 0; --unk00) {
      unk04 = unk00 * 0x28;
      memcpy(*(u8**)(arg0 + 8) + unk04,
             *(u8* const*)(arg1 + 8) + unk04, 0x28);
      (*(ProjectSortNode**)(arg0 + 8))[unk00].unk0C = 0;
      (*(ProjectSortNode**)(arg0 + 8))[unk00].unk10 = 0;
      (*(ProjectSortNode**)(arg0 + 8))[unk00].unk14 = 0;
      (*(ProjectSortNode**)(arg0 + 8))[unk00].unk18 = 0;
      if ((*(ProjectSortNode**)(arg0 + 8))[unk00].unk24 != 0) {
        (*(ProjectSortNode**)(arg0 + 8))[unk00].unk20 =
            (u32)(*(u8* (**)(u32))(RwEngineInstance + 0x130))(
                (*(ProjectSortNode**)(arg0 + 8))[unk00].unk24);
        if ((*(ProjectSortNode**)(arg0 + 8))[unk00].unk20 == 0) {
          BaerrState unk0C;
          unk0C.unk00 = 1;
          unk0C.unk04 = _rwerror(0x80000013,
              (*(ProjectSortNode**)(arg0 + 8))[unk00].unk24);
          RwErrorSet(&unk0C);
          return 0;
        }
        memcpy((void*)(*(ProjectSortNode**)(arg0 + 8))[unk00].unk20,
               (void*)(*(const ProjectSortNode* const*)(arg1 + 8))[unk00].unk20,
               (*(ProjectSortNode**)(arg0 + 8))[unk00].unk24);
      }
    }
    *(u32*)(arg0 + 4) = *(const u32*)(arg1 + 4);
  }
  unk08 = *(u8**)(arg0 + 8) +
      *(u32*)(RwEngineInstance + _rxPipelineGlobalsOffset + 0x3C) * 0x28;
  for (unk00 = *(const u32*)(arg1 + 4) - 1; (s32)unk00 >= 0; --unk00) {
    (*(ProjectSortNode**)(arg0 + 8))[unk00].unk08 = (u32*)(unk08 + unk00 * 0x80);
    if ((*(const ProjectSortNode* const*)(arg1 + 8))[unk00].unk08 != 0) {
      memcpy((*(ProjectSortNode**)(arg0 + 8))[unk00].unk08,
             (*(const ProjectSortNode* const*)(arg1 + 8))[unk00].unk08, 0x80);
    }
  }
  {
    ProjectSortState* unk10 = (ProjectSortState*)(unk08 +
        *(u32*)(RwEngineInstance + _rxPipelineGlobalsOffset + 0x3C) * 0x80);
    for (unk00 = 0; unk00 < *(const u32*)(arg1 + 4); ++unk00) {
      unk10[unk00].unk00[0] = 0;
      unk10[unk00].unk00[1] = 0;
      unk10[unk00].unk00[2] = 0;
      (*(ProjectSortNode**)(arg0 + 8))[unk00].unk1C = unk10 + unk00;
    }
  }
  return 1;
}

/* TODO: [near miss] 99.81%; loop index register allocation remains. */
static s32 _NodeCreate(u8* unk00, u8* unk04, const u8* unk08) {
  s32 unk0C = 1;
  u32 unk10 = *(const u32*)(unk08 + 0x2C);
  u32 unk24;
  memset(unk04, 0, 0x28);
  if (unk10 > 0x20) {
    BaerrState unk14;
    unk14.unk00 = 1;
    unk14.unk04 = _rwerror(0x29);
    RwErrorSet(&unk14);
    unk0C = 0;
  }
  if (*(const u32*)(unk08 + 0x20) > 0x20) {
    BaerrState unk18;
    unk18.unk00 = 1;
    unk18.unk04 = _rwerror(0x28);
    RwErrorSet(&unk18);
    unk0C = 0;
  }
  if (unk10 >= *(u32*)(RwEngineInstance + _rxPipelineGlobalsOffset + 0x3C)) {
    BaerrState unk1C;
    unk1C.unk00 = 1;
    unk1C.unk04 = _rwerror(0x2A);
    RwErrorSet(&unk1C);
    unk0C = 0;
  }
  if (unk0C != 0) {
    s32* unk20 = (s32*)(*(u8**)(unk00 + 8) +
        *(u32*)(RwEngineInstance + _rxPipelineGlobalsOffset + 0x3C) * 0x28);
    u8* unk28;
    u8* unk2C;
    unk20 = (s32*)((u8*)unk20 + (*(u32*)(unk00 + 4) << 7));
    *(s32**)(unk04 + 8) = unk20;
    *(u32*)(unk04 + 4) = unk10;
    for (unk24 = 0; unk24 < *(u32*)(unk04 + 4); ++unk24) {
      *unk20++ = -1;
    }
    unk28 = *(u8**)(unk00 + 8) +
        *(u32*)(RwEngineInstance + _rxPipelineGlobalsOffset + 0x3C) * 0x28;
    unk2C = unk28 + (*(u32*)(RwEngineInstance + _rxPipelineGlobalsOffset + 0x3C) << 7);
    unk2C += *(u32*)(unk00 + 4) * 0xC;
    *(u32*)(unk2C) = 0;
    *(u32*)(unk2C + 4) = 0;
    *(u32*)(unk2C + 8) = 0;
    *(u8**)(unk04 + 0x1C) = unk2C;
    *(u32*)(unk04 + 0x20) = 0;
    *(u32*)(unk04 + 0x24) = 0;
    *(const u8**)unk04 = unk08;
    ++*(u32*)(unk00 + 4);
  }
  return unk0C;
}

/* TODO: [near miss] 94.02%; register allocation remains across node swaps. */
static void PipelineTopSort(u8* context, u32 selected) {
  u32 current = *(u32*)(context + 4);
  ProjectSortNode* node;
  if (current != selected) {
    u32 index;
    u32* first = (*(ProjectSortNode**)(*(u8**)context + 8))[current].unk08;
    u32* second = (*(ProjectSortNode**)(*(u8**)context + 8))[selected].unk08;
    ProjectSortState temporaryState;
    ProjectSortNode temporaryNode;
    ProjectSortState* firstState;
    ProjectSortState* secondState;
    for (index = 0; index < 32; ++index) {
      u32 value = first[index];
      first[index] = second[index];
      second[index] = value;
    }
    (*(ProjectSortNode**)(*(u8**)context + 8))[current].unk08 = second;
    (*(ProjectSortNode**)(*(u8**)context + 8))[selected].unk08 = first;
    firstState = (*(ProjectSortNode**)(*(u8**)context + 8))[current].unk1C;
    secondState = (*(ProjectSortNode**)(*(u8**)context + 8))[selected].unk1C;
    temporaryState = *firstState;
    *firstState = *secondState;
    *secondState = temporaryState;
    (*(ProjectSortNode**)(*(u8**)context + 8))[current].unk1C = secondState;
    (*(ProjectSortNode**)(*(u8**)context + 8))[selected].unk1C = firstState;
    temporaryNode = (*(ProjectSortNode**)(*(u8**)context + 8))[current];
    (*(ProjectSortNode**)(*(u8**)context + 8))[current] =
        (*(ProjectSortNode**)(*(u8**)context + 8))[selected];
    (*(ProjectSortNode**)(*(u8**)context + 8))[selected] = temporaryNode;
    for (index = 0; index < *(u32*)(*(u8**)context + 4); ++index) {
      ProjectSortNode* entry = &(*(ProjectSortNode**)(*(u8**)context + 8))[index];
      u32 output;
      for (output = 0; output < entry->unk04; ++output) {
        u32* outputs = entry->unk08;
        u32 value = outputs[output];
        if (current == value) {
          outputs[output] = selected;
        } else if (selected == value) {
          outputs[output] = current;
        }
      }
    }
  }
  node = &(*(ProjectSortNode**)(*(u8**)context + 8))[*(u32*)(context + 4)];
  ++*(u32*)(context + 4);
  if (node->unk04 != 0) {
    u32 index;
    for (index = 0; index < node->unk04; ++index) {
      u32 next = (node->unk08)[index];
      if (next != ~0U) {
        ProjectSortNode* target = &(*(ProjectSortNode**)(*(u8**)context + 8))[next];
        ++(target->unk1C)->unk00[1];
        if ((target->unk1C)->unk00[0] ==
            (target->unk1C)->unk00[1]) {
          PipelineTopSort(context, next);
        }
      }
    }
  }
}

u32* RxPipelineNodeFindOutputByName(const u8* unk00, const char* unk04) {
  s32 unk08;
  s32 unk0C;
  const u8* unk10;
  if (unk00 != 0 && *(u8* const*)unk00 != 0 && unk04 != 0) {
    const u8* unk14 = *(u8* const*)unk00;
    unk0C = *(const s32*)(unk14 + 0x2C);
    unk10 = *(u8* const*)(unk14 + 0x30);
    for (unk08 = 0; unk08 < unk0C; ++unk08, unk10 += 12) {
      if (strcmp(*(const char* const*)unk10, unk04) == 0) {
        return *(u32* const*)(unk00 + 8) + unk08;
      }
    }
  }
  return 0;
}

u8* RxPipelineNodeFindInput(u8* unk00) {
  if (unk00 != 0 && *(u32*)unk00 != 0) {
    return unk00;
  }
  return 0;
}

static inline u8* projectPipelineFixup(u8* unk00, u32 unk04, u32 unk08) {
  return *(u8**)(unk00 + unk04) == 0 ? 0 :
      (*(u8**)(unk00 + unk04) += unk08);
}

static inline s32 projectPipelineResize(u8* unk00, u32 unk08) {
  u8* unk0C = *(u8**)(unk00 + 0x20);
  u8* unk10 = (*(u8* (**)(u8*, u32))(RwEngineInstance + 0x138))(unk0C, unk08);
  if (unk10 != 0) {
    u32 unk14;
    u32 unk18 = *(u32*)(unk00 + 4);
    u32 unk1C = (u32)unk10 - (u32)unk0C;
    *(u8**)(unk00 + 0x20) = unk10;
    *(u32*)(unk00 + 0x24) = unk08;
    *(u8**)(unk00 + 8) = *(u8**)(unk00 + 0x20);
    projectPipelineFixup(unk00, 0x14, unk1C);
    projectPipelineFixup(unk00, 0x1C, unk1C);
    for (unk14 = 0; unk14 < unk18; ++unk14) {
      projectPipelineFixup(*(u8**)(unk00 + 8) + unk14 * 0x28, 0x8, unk1C);
      projectPipelineFixup(*(u8**)(unk00 + 8) + unk14 * 0x28, 0xC, unk1C);
      projectPipelineFixup(*(u8**)(unk00 + 8) + unk14 * 0x28, 0x10, unk1C);
      projectPipelineFixup(*(u8**)(unk00 + 8) + unk14 * 0x28, 0x14, unk1C);
      projectPipelineFixup(*(u8**)(unk00 + 8) + unk14 * 0x28, 0x18, unk1C);
      projectPipelineFixup(*(u8**)(unk00 + 8) + unk14 * 0x28, 0x1C, unk1C);
    }
  } else {
    BaerrState unk20;
    unk20.unk00 = 1;
    unk20.unk04 = _rwerror(0x80000013, unk08);
    RwErrorSet(&unk20);
    return 0;
  }
  return 1;
}

/* TODO: [breakthrough] 93.35%; size staging, result boundaries and register allocation remain. */
u8* RxLockedPipeUnlock(u8* unk00) {
  if (unk00 != 0 && *(s32*)unk00 != 0) {
    if (*(u32*)(unk00 + 4) != 0) {
      s32 unk04 = 0;
      s32 unk08 = 0;
      u32 unk0C = 0;
      u32 unk10;
      u32 unk14;
      u32 unk18;
      u8* unk1C;
      u8* unk20;
      if (*(u32*)(unk00 + 0x28) >= *(u32*)(unk00 + 4) ||
          *(u32*)(*(u8**)(unk00 + 8) + *(u32*)(unk00 + 0x28) * 0x28) == 0) {
        BaerrState unk24;
        unk24.unk00 = 1;
        unk24.unk04 = _rwerror(0x24);
        RwErrorSet(&unk24);
        return 0;
      }
      {
        u32 unk2C = PipelineCalcNumUniqueClusters(unk00);
        u32 unk30 = *(u32*)(RwEngineInstance + _rxPipelineGlobalsOffset + 0x3C);
        u32 unk32 = unk30 * 0xC;
        u32 unk36 = unk30 * 0x28 + (unk30 * 0x80 + unk32);
        u32 unk34 = *(u32*)(unk00 + 4) * 0x28;
        u32 unk38;
        u32 unk54;
        u32 unk58;
        u32 unk5A;
        u32 unk3A;
        for (unk5A = 0; unk5A < *(u32*)(unk00 + 4); ++unk5A) {
          unk34 += *(u32*)(*(u8**)(unk00 + 8) + 4) * 4;
        }
        unk38 = unk32 + unk34;
        unk54 = *(u32*)(unk00 + 4) * unk2C;
        unk3A = *(u32*)(unk00 + 4) * 0x14;
        unk38 = unk54 * 0x24 + unk54 * 0x10 + unk38;
        unk38 = unk3A + unk38;
        unk58 = 0;
        unk58 += unk2C * 8;
        unk58 += unk2C * 0xC;
        unk58 += unk54 * 4;
        unk58 += *(u32*)(unk00 + 4) * (unk2C + 1) * 4;
        for (unk5A = 0; unk5A < *(u32*)(unk00 + 4); ++unk5A) {
          u8* unk5C = *(u8**)(*(u8**)(unk00 + 8) + unk5A * 0x28);
          if (*(u32*)(unk5C + 0x34) != 0) {
            unk58 += *(u32*)(unk5C + 0x34);
          }
          unk58 += *(u32*)(unk5C + 0x20) * 4;
        }
        unk58 += (unk2C - 1) * 0x1C;
        unk58 += 0x30;
        unk38 += unk58;
        unk18 = unk36;
        if (unk38 > unk18) {
          unk18 = unk38;
        }
      }
      if (unk18 > *(u32*)(unk00 + 0x24)) {
        if (projectPipelineResize(unk00, unk18) == 0) {
          return 0;
        }
      }
      gMemoryLimits[0] = (u32)(*(u8**)(unk00 + 0x20) + unk18);
      gMemoryLimits[1] = 0;
      {
        struct {
          u8* unk00;
          u32 unk04;
        } unk68;
        u8* unk70;
        u32 unk74;
        u32 unk78;
        unk68.unk00 = unk00;
        unk68.unk04 = 0;
        unk70 = *(u8**)(unk00 + 8);
        for (unk74 = 0; unk74 < *(u32*)(unk00 + 4); ++unk74, unk70 += 0x28) {
          if (*(u32*)unk70 != 0) {
            *(u32*)(*(u8**)(unk70 + 0x1C) + 4) = 0;
            **(u32**)(unk70 + 0x1C) = 0;
          }
        }
        unk70 = *(u8**)(unk00 + 8);
        for (unk74 = 0; unk74 < *(u32*)(unk00 + 4); ++unk74, unk70 += 0x28) {
          if (*(u32*)unk70 != 0) {
            unk78 = *(u32*)(unk70 + 4);
            if (unk78 != 0) {
              s32* unk7C = *(s32**)(unk70 + 8);
              do {
                if (*unk7C != -1) {
                  ++(*(ProjectSortNode**)(unk00 + 8))[*unk7C].unk1C->unk00[0];
                }
                --unk78;
                ++unk7C;
              } while (unk78 != 0);
            }
          }
        }
        if ((*(ProjectSortNode**)(unk00 + 8))[*(u32*)(unk00 + 0x28)].unk1C->unk00[0] != 0) {
          BaerrState unk80;
          unk80.unk00 = 1;
          unk80.unk04 = _rwerror(0x24);
          RwErrorSet(&unk80);
          unk00 = 0;
          goto unk560;
        }
        for (unk74 = 0; unk74 < *(u32*)(unk00 + 4); ++unk74) {
          if (unk74 != *(u32*)(unk00 + 0x28) &&
              (*(ProjectSortNode**)(unk00 + 8))[unk74].unk1C->unk00[0] == 0) {
            BaerrState unk88;
            unk88.unk00 = 1;
            unk88.unk04 = _rwerror(0x22);
            RwErrorSet(&unk88);
            unk00 = 0;
            goto unk560;
          }
        }
        PipelineTopSort((u8*)&unk68, *(u32*)(unk00 + 0x28));
        for (unk74 = 0; unk74 < *(u32*)(unk00 + 4); ++unk74) {
          u8* unk90 = (u8*)(*(ProjectSortNode**)(unk00 + 8))[unk74].unk1C;
          if (*(u32*)unk90 != *(u32*)(unk90 + 4)) {
            BaerrState unk94;
            unk94.unk00 = 1;
            unk94.unk04 = _rwerror(0x1C);
            RwErrorSet(&unk94);
            unk00 = 0;
            goto unk560;
          }
        }
        *(u32*)(unk00 + 0x28) = 0;
      }
    unk560:
      if (unk00 == 0) {
        return 0;
      }
      {
        u32 unk9C = *(u32*)(RwEngineInstance + _rxPipelineGlobalsOffset + 0x3C);
        u8* unkA0;
        s32 unkA4;
        unk1C = *(u8**)(unk00 + 8) + unk9C * 0x28;
        unk20 = unk1C + unk9C * 0x80;
        unk20 += (*(u32*)(unk00 + 4) - 1) * 0xC;
        unkA0 = *(u8**)(unk00 + 0x20) + unk18;
        unkA0 -= 0xC;
        for (unkA4 = *(u32*)(unk00 + 4) - 1; unkA4 >= 0; --unkA4) {
          memcpy(unkA0, unk20, 0xC);
          (*(ProjectSortNode**)(unk00 + 8))[unkA4].unk1C = (ProjectSortState*)unkA0;
          unk20 -= 0xC;
          unkA0 -= 0xC;
        }
        unkA0 = *(u8**)(unk00 + 8) + *(u32*)(unk00 + 4) * 0x28;
        for (unk10 = 0; unk10 < *(u32*)(unk00 + 4); ++unk10) {
          u32 unkA8 = (*(ProjectSortNode**)(unk00 + 8))[unk10].unk04;
          if (unkA8 == 0) {
            (*(ProjectSortNode**)(unk00 + 8))[unk10].unk08 = 0;
          } else {
            memcpy(unkA0, unk1C, unkA8 * 4);
            (*(ProjectSortNode**)(unk00 + 8))[unk10].unk08 = (u32*)unkA0;
          }
          unkA8 = (*(ProjectSortNode**)(unk00 + 8))[unk10].unk04;
          unkA0 += unkA8 * 4;
          unk04 += unkA8;
          unk1C += 0x80;
        }
        gMemoryLimits[1] = (u32)(unk1C + unk04 * 4);
        gMemoryLimits[0] = (u32)unk20;
      }
      if (_rxChaseDependencies(unk00) != 0) {
        return 0;
      }
      {
        u32 unkAC = gMemoryLimits[1] - (u32)*(u8**)(unk00 + 0x20);
        if (projectPipelineResize(unk00, unkAC) == 0) {
          LockPipelineExpandData(unk00, unk00);
          return 0;
        }
      }
      for (unk10 = 0; unk10 < *(u32*)(unk00 + 4); ++unk10) {
        (*(ProjectSortNode**)(unk00 + 8))[unk10].unk1C = 0;
      }
      for (unk14 = *(u32*)(unk00 + 4) - 1; (s32)unk14 >= 0; --unk14) {
        u8* unkB8 = *(u8**)(unk00 + 8) + unk14 * 0x28;
        u8* unkBC = *(u8**)unkB8;
        s32 (*unkC0)(u8*);
        if ((*(s32*)(unkBC + 0x3C))++ == 0 &&
            (unkC0 = *(s32 (**)(u8*))(unkBC + 8)) != 0 && unkC0(unkBC) == 0) {
          unk0C = (*(u32*)(unk00 + 4) - 1) - unk14;
          unk08 = 1;
          break;
        }
        {
          s32 (*unkC4)(u8*) = *(s32 (**)(u8*))(unkBC + 0x10);
          if (unkC4 != 0 && unkC4(unkB8) == 0) {
            --*(s32*)(unkBC + 0x3C);
            if (*(s32*)(unkBC + 0x3C) == 0) {
              void (*unkC8)(u8*) = *(void (**)(u8*))(unkBC + 0xC);
              if (unkC8 != 0) {
                unkC8(unkBC);
              }
            }
            unk0C = (*(u32*)(unk00 + 4) - 1) - unk14;
            unk08 = 1;
            break;
          }
        }
      }
      if (unk08 == 0) {
        for (unk14 = *(u32*)(unk00 + 4) - 1; (s32)unk14 >= 0; --unk14) {
          u8* unkCC = *(u8**)(unk00 + 8) + unk14 * 0x28;
          u8* unkD0 = *(u8**)unkCC;
          s32 (*unkD4)(u8*, u8*) = *(s32 (**)(u8*, u8*))(unkD0 + 0x18);
          if (unkD4 != 0 && unkD4(unkCC, unk00) == 0) {
            unk0C = *(u32*)(unk00 + 4);
            unk08 = 1;
            break;
          }
        }
      }
      if (unk08 != 0) {
        for (unk10 = *(u32*)(unk00 + 4) - unk0C;
             unk10 < *(u32*)(unk00 + 4); ++unk10) {
          u8* unkD8 = (u8*)&(*(ProjectSortNode**)(unk00 + 8))[unk10];
          u8* unkDC = *(u8**)unkD8;
          void (*unkE0)(u8*) = *(void (**)(u8*))(unkDC + 0x14);
          if (unkE0 != 0) {
            unkE0(unkD8);
          }
          --*(s32*)(unkDC + 0x3C);
          if (*(s32*)(unkDC + 0x3C) == 0) {
            void (*unkE4)(u8*) = *(void (**)(u8*))(unkDC + 0xC);
            if (unkE4 != 0) {
              unkE4(unkDC);
            }
          }
        }
        LockPipelineExpandData(unk00, unk00);
        return 0;
      }
    }
    *(s32*)unk00 = 0;
    return unk00;
  }
  if (unk00 == 0) {
    BaerrState unkE8;
    unkE8.unk00 = 1;
    unkE8.unk04 = _rwerror(0x80000016);
    RwErrorSet(&unkE8);
  } else {
    BaerrState unkF0;
    unkF0.unk00 = 1;
    unkF0.unk04 = _rwerror(0x34);
    RwErrorSet(&unkF0);
  }
  return 0;
}

u8* RxPipelineLock(u8* unk00) {
  typedef struct {
    u8 unk00[0xC];
    u8* unk0C;
    u8 unk10[0x18];
  } ProjectLockNode;
  if (*(s32*)unk00 == 0) {
    u32 unk04 = *(u32*)(RwEngineInstance + _rxPipelineGlobalsOffset + 0x3C);
    u32 unk08 = unk04 * 0x28 + unk04 * 0x80 + unk04 * 12;
    if (*(u8**)(unk00 + 8) != 0) {
      if (unk08 > *(u32*)(unk00 + 0x24)) {
        if (projectPipelineResize(unk00, unk08) == 0) {
          return 0;
        }
      }
      if (LockPipelineExpandData(unk00, unk00) == 0) {
        return 0;
      }
    } else {
      *(u8**)(unk00 + 0x20) =
      (*(u8* (**)(u32))(RwEngineInstance + 0x130))(unk08);
      if (*(u8**)(unk00 + 0x20) == 0) {
        BaerrState unk24;
        unk24.unk00 = 1;
        unk24.unk04 = _rwerror(0x80000013, unk08);
        RwErrorSet(&unk24);
        return 0;
      }
      *(u32*)(unk00 + 0x24) = unk08;
      *(u8**)(unk00 + 8) = *(u8**)(unk00 + 0x20);
    }
    *(s32*)unk00 = 1;
    if (*(u8**)(unk00 + 8) != 0) {
      u32 unk28;
      for (unk28 = 0; unk28 < *(u32*)(unk00 + 4); ++unk28) {
        u8* unk30 = *(u8**)(*(u8**)(unk00 + 8) + unk28 * 0x28);
        void (*unk34)(u8*) = *(void (**)(u8*))(*(u8**)(*(u8**)(unk00 + 8) + unk28 * 0x28) + 0x14);
        u8* unk38;
        u8* unk54;
        if (unk34 != 0) {
          u8* unk50 = *(u8**)(unk00 + 8) + unk28 * 0x28;
          unk34(unk50);
        }
        unk38 = *(u8**)(unk00 + 8);
        unk54 = *(u8**)(unk38 + unk28 * 0x28);
        if (--*(s32*)(unk54 + 0x3C) == 0) {
          void (*unk3C)(u8*) = *(void (**)(u8*))(unk30 + 0xC);
          if (unk3C != 0) {
            unk3C(*(u8**)(*(u8**)(unk00 + 8) + unk28 * 0x28));
          }
        }
        (*(ProjectLockNode**)(unk00 + 8) + unk28)->unk0C = 0;
      }
    }
  }
  return unk00;
}

u8* RxPipelineFindNodeByName(u8* unk00, const u8* unk04,
    const u8* unk08, s32* unk0C) {
  s32 unk10 = 0;
  if (unk00 != 0 && unk04 != 0 && *(u32*)(unk00 + 4) != 0) {
    unk10 = 1;
  }
  if (unk10 != 0) {
    u8* unk14 = *(u8**)(unk00 + 8);
    s32 unk18 = *(s32*)(unk00 + 4);
    if (unk08 != 0) {
      while (unk14 != unk08 && unk18 > 0) {
        unk14 += 0x28;
        unk18--;
      }
      unk14 += 0x28;
      unk18--;
    }
    while (unk18 > 0) {
      u8* unk1C = *(u8**)unk14;
      if (unk1C != 0 &&
          (*(s32 (**)(const u8*, const u8*))(RwEngineInstance + 0x114))(
              *(u8**)unk1C, unk04) == 0) {
        if (unk0C != 0) {
          *unk0C = unk18;
        }
        return unk14;
      }
      unk14 += 0x28;
      unk18--;
    }
  }
  if (unk0C != 0) {
    *unk0C = -1;
  }
  return 0;
}

#include <stdarg.h>

u8* RxLockedPipeAddFragment(u8* arg0, u32* arg1, const u8* arg2, ...) {
  u32 unk04;
  va_list unk08;
  if (arg0 != 0 && *(s32*)arg0 != 0) {
    u32 unk0C = 0;
    const u8* unk10;
    va_start(unk08, arg2);
    for (unk10 = arg2; unk10 != 0;
         unk10 = va_arg(unk08, const u8*)) {
      ++unk0C;
    }
    va_end(unk08);
    if (unk0C != 0) {
      u32 unk14;
      u8* unk18 = 0;
      unk04 = *(u32*)(arg0 + 4);
      if (unk04 + unk0C > *(u32*)(RwEngineInstance + _rxPipelineGlobalsOffset + 0x3C)) {
        BaerrState unk24;
        unk24.unk00 = 1;
        unk24.unk04 = _rwerror(0x2A);
        RwErrorSet(&unk24);
        return 0;
      }
      unk14 = 0;
      va_start(unk08, arg2);
      for (unk10 = arg2; unk10 != 0;
           unk10 = va_arg(unk08, const u8*)) {
        u8* unk1C = *(u8**)(arg0 + 8) + (unk04 + unk14) * 0x28;
        if (_NodeCreate(arg0, unk1C, unk10) == 0) {
          break;
        }
        if (unk18 != 0) {
          u32 unk00 = 0;
          u32* unk20;
          if (unk18 != 0 && *(u32*)unk18 != 0 && *(u32*)(unk18 + 4) > unk00) {
            unk20 = *(u32**)(unk18 + 8) + unk00;
          } else {
            unk20 = 0;
          }
          if (RxLockedPipeAddPath(arg0, unk20, RxPipelineNodeFindInput(unk1C)) == 0) {
            PipelineNodeDestroy(unk1C, arg0);
            break;
          }
        }
        unk18 = unk1C;
        ++unk14;
      }
      va_end(unk08);
      if (unk14 == unk0C) {
        if (arg1 != 0) {
          *arg1 = unk04;
        }
        return arg0;
      }
      while (unk14-- != 0) {
        PipelineNodeDestroy(*(u8**)(arg0 + 8) + (unk14 + unk04) * 0x28, arg0);
      }
    }
  } else if (arg0 == 0) {
    BaerrState unk24;
    unk24.unk00 = 1;
    unk24.unk04 = _rwerror(0x80000016);
    RwErrorSet(&unk24);
  } else {
    BaerrState unk24;
    unk24.unk00 = 1;
    unk24.unk04 = _rwerror(0x34);
    RwErrorSet(&unk24);
  }
  return 0;
}

static inline u32 projectPipelineNodeIndex(u8* arg0, u8* arg1) {
  u8* unk00 = *(u8**)(arg0 + 8);
  u32 unk04 = (arg1 - unk00) / 0x28;
  if (unk00 + unk04 * 0x28 == arg1 && unk04 < *(u32*)(arg0 + 4)) {
    return unk04;
  }
  return ~0U;
}

u8* RxLockedPipeAddPath(u8* arg0, u32* arg1, u8* arg2) {
  if (arg0 != 0 && *(s32*)arg0 != 0 &&
      arg1 != 0 && *arg1 == ~0U &&
      arg2 != 0 && *(u8**)arg2 != 0) {
    u32 unk00 = projectPipelineNodeIndex(arg0, arg2);
    if (unk00 != ~0U) {
      *arg1 = unk00;
      return arg0;
    }
  }
  return 0;
}
