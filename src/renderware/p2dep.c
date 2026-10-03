#include <dolphin/types.h>
#include <string.h>
#include <renderware/project_error.h>
#include <renderware/project_memory.h>
#include <renderware/project_pipeline.h>
#include <renderware/project_node_defs.h>
#include <renderware/project_sort.h>

static u8* _ReqSearch4Cluster(const u8* arg0, u32 arg1) {
  u32 unk00 = *(const u32*)arg0;
  if (unk00 != 0U) {
    u8* unk04 = *(u8* const*)(arg0 + 0xC);
    u32 unk08;
    for (unk08 = 0; unk08 < unk00; unk08++) {
      if (*(u32*)unk04 == arg1) {
        return unk04;
      }
      unk04 += 0x24;
    }
  }
  return 0;
}

typedef struct ProjectDependencyEntry {
  u32 unk00[9];
} ProjectDependencyEntry;

static void _ReqDeleteEntry(u8* unk00, ProjectDependencyEntry* unk04) {
  ProjectDependencyEntry* unk08 = *(ProjectDependencyEntry**)(unk00 + 0xC);
  if (unk04 != &unk08[*(s32*)unk00 - 1]) {
    *unk04 = unk08[*(s32*)unk00 - 1];
  }
  --*(s32*)unk00;
}

static u32 _IoSpecSearch4Cluster(const u8* spec, u32 cluster) {
  u32 index;
  for (index = 0; index < *(const u32*)spec; ++index) {
    if ((*(ProjectNodeCluster* const*)(spec + 4))[index].unk00 == (const ProjectClusterDef*)cluster) {
      return index;
    }
  }
  return ~0U;
}

/* TODO: [near miss] 99.77%; inlined deletion register allocation remains. */
static void _PropDownElimPath(u8* arg0, u8* arg1, u32 arg2) {
  u32 unk08;
  const u8* unk00 = *(u8**)arg1 + 0x20;
  u8* unk04 = _ReqSearch4Cluster(*(u8**)(*(u8**)(arg1 + 0x1C) + 8), arg2);
  if (unk04 != 0) {
    if (--*(s32*)(unk04 + 8) == 0) {
      _ReqDeleteEntry(*(u8**)(*(u8**)(arg1 + 0x1C) + 8),
          (ProjectDependencyEntry*)unk04);
      for (unk08 = 0; unk08 < *(u32*)(arg1 + 4); unk08++) {
        if (~0U != (*(u32**)(arg1 + 8))[unk08]) {
          const u8* unk0C = *(u8**)(*(u8**)arg1 + 0x30) + unk08 * 12;
          u32 unk10 = _IoSpecSearch4Cluster(unk00, arg2);
          if ((unk10 == ~0U ? *(const s32*)(unk0C + 8) :
              (*(s32* const*)(unk0C + 4))[unk10]) == 0) {
            _PropDownElimPath(arg0, *(u8**)(arg0 + 8) +
                (*(u32**)(arg1 + 8))[unk08] * 0x28, arg2);
          }
        }
      }
    }
  }
}

static inline u8* projectDepEntry(const u8* arg0, u32 arg1, u32 arg2) {
  u8* unk00;
  if (arg1 < arg2) {
    unk00 = *(u8* const*)(arg0 + 0xC) + arg1 * 0x24;
  } else {
    unk00 = 0;
  }
  return unk00;
}

static inline u8* projectDepFindClusterInCount(
    const u8* unk00, const ProjectClusterDef* unk04, u32 unk08) {
  if (unk08 != 0) {
    u8* unk0C = *(u8**)(unk00 + 0x0C);
    u32 unk10;
    for (unk10 = 0; unk10 < unk08; ++unk10) {
      if (*(const ProjectClusterDef**)unk0C == unk04) {
        return unk0C;
      }
      unk0C += 0x24;
    }
  }
  return 0;
}

static inline u8* projectDepPropagateCluster(
    u8* unk14, const ProjectClusterDef* unk04, s32 unk08, u32 unk10,
    u8* unk0C) {
  u32 unk18 = *(u32*)unk14;
  u8* unk1C = projectDepFindClusterInCount(unk14, unk04, unk18);
  if (unk1C != 0) {
    if (unk08 == 1) {
      *(s32*)(unk1C + 4) = 1;
      *(u8**)(unk1C + 0x20) = unk0C;
    }
  } else {
    unk1C = *(u8**)(unk14 + 0xC) + unk18 * 0x24;
    ++*(u32*)unk14;
    *(const ProjectClusterDef**)unk1C = unk04;
    *(s32*)(unk1C + 4) = unk08;
    *(u32*)(unk1C + 8) = unk10;
    *(u32*)(unk1C + 0x10) = 0;
    *(u8**)(unk1C + 0x14) = unk14;
    *(u32*)(unk1C + 0xC) = 0;
    *(u32*)(unk1C + 0x18) = 0;
    *(s32*)(unk1C + 0x1C) = -1;
    *(u8**)(unk1C + 0x20) = unk0C;
  }
  return unk1C;
}

static inline u8* projectDepAllocateRequirements(u8* unk00, s32 unk04) {
  u8* unk08 = StalacTiteAlloc(0x14);
  if (unk08 != 0) {
    *(u8**)(unk08 + 0xC) = StalacTiteAlloc(unk04 * 0x24);
    if (*(u8**)(unk08 + 0xC) != 0) {
      *(s32*)(unk08 + 4) = unk04;
      *(u32*)unk08 = 0;
      *(u8**)(unk08 + 0x10) = unk00;
      *(u32*)(unk08 + 8) = 0;
      return unk08;
    }
  }
  return 0;
}

/* TODO: [near miss] 98.88%; register allocation and input load order remain. */
static s32 _PropagateDependenciesAndKillDeadPaths(u8* unk00) {
  u8* unk64;
  const ProjectClusterDef* unk18;
  u8* unk08;
  ProjectNodeDef* unk0C;
  s32 unk04;
  unk04 = *(s32*)(unk00 + 4);
  unk08 = *(u8**)(unk00 + 8) + (*(s32*)(unk00 + 4) - 1) * 0x28;
  do {
    u32 unk10;
    u32 unk14;
    unk0C = *(ProjectNodeDef**)unk08;
    for (unk10 = 0; unk10 < unk0C->unk20; ++unk10) {
      unk18 = unk0C->unk24[unk10].unk00;
      if (unk18 == 0) {
        BaerrState unk1C;
        unk1C.unk00 = 1;
        unk1C.unk04 = _rwerror(0x1F, **(u32**)unk08, unk10);
        RwErrorSet(&unk1C);
        return 0x1F;
      }
      for (unk14 = unk10 + 1; unk14 < unk0C->unk20; ++unk14) {
        const ProjectClusterDef* unk24 =
            unk0C->unk24[unk14].unk00;
        if (unk18 == unk24) {
          BaerrState unk28;
          unk28.unk00 = 1;
          unk28.unk04 = _rwerror(0x1E, **(u32**)unk08, *(const u32*)unk24);
          RwErrorSet(&unk28);
          return 0x1E;
        }
      }
    }
    {
      u8* unk34 = projectDepAllocateRequirements(
          unk08, PipelineCalcNumUniqueClusters(unk00));
      *(u8**)(*(u8**)(unk08 + 0x1C) + 8) = unk34;
      if (unk34 == 0) {
        BaerrState unk38;
        unk38.unk00 = 1;
        unk38.unk04 = _rwerror(0x20);
        RwErrorSet(&unk38);
        return 0x20;
      }
    }
    for (unk10 = 0; unk10 < *(u32*)(unk08 + 4); ++unk10) {
      u32 unk40 = (*(u32**)(unk08 + 8))[unk10];
      if (unk40 != 0xFFFFFFFFU) {
        u8* unk44 = *(u8**)(unk00 + 8) + unk40 * 0x28;
        u8* unk48 = *(u8**)(*(u8**)unk08 + 0x30) + unk10 * 0xC;
        u32 unkA0;
        for (unk14 = 0;
            unk14 < (unkA0 = **(u32**)(*(u8**)(unk44 + 0x1C) + 8));
            ++unk14) {
          u8* unk4C = *(u8**)(*(u8**)(unk44 + 0x1C) + 8);
          u8* unk50 = projectDepEntry(unk4C, unk14, unkA0);
          u32 unk54 = *(u32*)unk50;
          u32 unk58 = _IoSpecSearch4Cluster((u8*)unk0C + 0x20, unk54);
          s32 unk5C = unk58 == 0xFFFFFFFFU ?
              *(s32*)(unk48 + 8) : (*(s32**)(unk48 + 4))[unk58];
          if (unk5C == 0) {
            unk64 = *(u8**)(*(u8**)(unk08 + 0x1C) + 8);
            if (projectDepPropagateCluster(
                unk64,
                (const ProjectClusterDef*)unk54, *(s32*)(unk50 + 4),
                *(u32*)*(u8**)(unk08 + 0x1C),
                *(u8**)(unk50 + 0x20)) == 0) {
              BaerrState unk74;
              unk74.unk00 = 1;
              unk74.unk04 = _rwerror(0x20);
              RwErrorSet(&unk74);
              return 0x20;
            }
          } else {
            if (*(s32*)(unk50 + 4) == 1) {
              if (unk5C != 1) {
                BaerrState unk7C;
                unk7C.unk00 = 1;
                unk7C.unk04 = _rwerror(0x1D, **(u32**)unk50,
                    **(u32**)(*(u8**)(unk50 + 0x20)), **(u32**)unk08,
                    unk10, *(u32*)unk48);
                RwErrorSet(&unk7C);
                return 0x1D;
              }
            } else if (unk5C == 2) {
              _PropDownElimPath(unk00, unk44, unk54);
            }
          }
        }
      }
    }
    for (unk10 = 0; unk10 < unk0C->unk20; ++unk10) {
      s32 unk84 = (s32)unk0C->unk28[unk10];
      if (unk84 != 0) {
        u8* unk88 = *(u8**)(unk08 + 0x1C);
        u8* unk94 = projectDepPropagateCluster(
            *(u8**)(unk88 + 8),
            unk0C->unk24[unk10].unk00,
            unk84, *(u32*)unk88, unk08);
        if (unk94 == 0) {
          BaerrState unk98;
          unk98.unk00 = 1;
          unk98.unk04 = _rwerror(0x20);
          RwErrorSet(&unk98);
          return 0x20;
        }
      }
    }
    --unk04;
    unk08 -= 0x28;
  } while (unk04 != 0);
  return 0;
}

static inline u8* projectDepRequireCluster(
    u8* unk00, const ProjectClusterDef* unk04, u8* unk08) {
  u8* unk0C;
  u32 unk10 = *(u32*)unk00;
  unk0C = projectDepFindClusterInCount(unk00, unk04, unk10);
  if (unk0C != 0) {
    return unk0C;
  }
  unk0C = *(u8**)(unk00 + 0x0C) + unk10 * 0x24;
  ++*(u32*)unk00;
  *(const ProjectClusterDef**)(unk0C + 0x00) = unk04;
  *(s32*)(unk0C + 0x04) = 0;
  *(s32*)(unk0C + 0x08) = 1;
  *(s32*)(unk0C + 0x10) = 0;
  *(u8**)(unk0C + 0x14) = unk00;
  *(s32*)(unk0C + 0x0C) = 0;
  *(u32*)(unk0C + 0x18) = 0;
  *(s32*)(unk0C + 0x1C) = -1;
  *(u8**)(unk0C + 0x20) = unk08;
  return unk0C;
}

static inline u8* projectDepNodeRequirements(const u8* unk00) {
  return *(u8**)(*(u8**)(unk00 + 0x1C) + 8);
}

/* TODO: [near miss] 99.52%; two inlined register groups remain. */
static s32 _ForAllNodeReqsAddOutputClustersAndBuildContinuityBitfields(u8* unk00) {
  ProjectNodeDef* unk0C;
  s32 unk04 = *(s32*)(unk00 + 4);
  u32 unk10;
  u32 unk34;
  u8* unk08 = *(u8**)(unk00 + 8);
  do {
    unk0C = *(ProjectNodeDef**)unk08;
    for (unk10 = 0; unk10 < unk0C->unk20; ++unk10) {
      if ((s32)unk0C->unk24[unk10].unk04 != 0) {
        const ProjectClusterDef* unk18 = unk0C->unk24[unk10].unk00;
        if (projectDepRequireCluster(projectDepNodeRequirements(unk08), unk18, unk08) == 0) {
          BaerrState unk20;
          unk20.unk00 = 1;
          unk20.unk04 = _rwerror(0x20);
          RwErrorSet(&unk20);
          return 0x20;
        }
      }
    }
    for (unk10 = 0; unk10 < *(u32*)(unk08 + 4); ++unk10) {
      u32 unk28 = ((u32*)*(u8**)(unk08 + 8))[unk10];
      if (unk28 != (u32)-1) {
        u8* unk2C = *(u8**)(unk00 + 8) + unk28 * 0x28;
        u8* unk30 = *(u8**)(*(u8**)unk08 + 0x30) + unk10 * 0x0C;
        for (unk34 = 0;
             unk34 < *(u32*)(*(u8**)(*(u8**)(unk2C + 0x1C) + 8));
             ++unk34) {
          u8* unk38 = *(u8**)(*(u8**)(unk2C + 0x1C) + 8);
          u8* unk3C = projectDepEntry(*(u8**)(*(u8**)(unk2C + 0x1C) + 8), unk34, *(u32*)unk38);
          const ProjectClusterDef* unk40 = *(const ProjectClusterDef**)unk3C;
          u32 unk44 = _IoSpecSearch4Cluster((const u8*)unk0C + 0x20,
              (u32)unk40);
          s32 unk48;
          if (unk44 == (u32)-1) {
            unk48 = *(s32*)(unk30 + 8);
          } else {
            unk48 = ((s32*)*(u8**)(unk30 + 4))[unk44];
          }
          if (unk48 != 2) {
            u8* unk50;
            if (unk48 != 0) {
              u8* unk4C = projectDepNodeRequirements(unk08);
              unk50 = projectDepRequireCluster(unk4C, unk40, unk08);
              if (unk50 == 0) {
                BaerrState unk54;
                unk54.unk00 = 1;
                unk54.unk04 = _rwerror(0x20);
                RwErrorSet(&unk54);
                return 0x20;
              }
            } else {
              u8* unk4C = projectDepNodeRequirements(unk08);
              unk50 = _ReqSearch4Cluster(unk4C, (u32)unk40);
            }
            if (unk50 != 0) {
              *(u32*)(unk50 + 0x18) |= 1U << unk10;
            }
          }
        }
      }
    }
    --unk04;
    unk08 += 0x28;
  } while (unk04 != 0);
  return 0;
}

static inline void projectDepTraceEntry(u8* arg0, u8* arg1, u32 arg2, u8** arg3) {
  if (*(s32*)(arg1 + 4) != 0) {
    u32 unk54 = *(u32*)arg1;
    u8* unk2C = _ReqSearch4Cluster(*(u8**)(*(u8**)(arg0 + 0x1C) + 8), unk54);
    if (unk2C != 0 && (*(u32*)(unk2C + 0x18) & (1U << arg2))) {
      u8* unk38;
      u8* unk30 = *(u8**)(arg1 + 0xC);
      if (unk30 == 0) {
        u8* unk34 = *(u8**)(unk2C + 0xC);
        *(u8**)(arg1 + 0x10) = *(u8**)unk34;
        *(u8**)unk34 = arg1;
        *(u8**)(arg1 + 0xC) = *(u8**)(unk2C + 0xC);
      } else {
        u8** unk3C;
        unk38 = *(u8**)(unk2C + 0xC);
        unk3C = arg3;
        while (*(u8**)(unk30 + 0xC) != 0) {
          unk30 = *(u8**)(unk30 + 0xC);
        }
        while (*(u8**)(unk38 + 0xC) != 0) {
          unk38 = *(u8**)(unk38 + 0xC);
        }
        if (unk30 != unk38) {
          u8* unk40 = unk30;
          while (*(u8**)(unk40 + 4) != 0) {
            unk40 = *(u8**)(unk40 + 4);
          }
          *(u8**)(unk40 + 4) = unk38;
          *(u8**)(unk38 + 0xC) = unk30;
          while (*unk3C != unk38) {
            unk3C = (u8**)(*unk3C + 8);
          }
          *unk3C = *(u8**)(*unk3C + 8);
        }
      }
    }
  }
}

static inline u8* projectDepNewScope(u8** arg0) {
  u8* unk00 = StalacTiteAlloc(0x10);
  if (unk00 != 0) {
    *(u8**)(unk00 + 4) = 0;
    *(u8**)unk00 = 0;
    *(u8**)(unk00 + 0xC) = 0;
    *(u8**)(unk00 + 8) = *arg0;
    *arg0 = unk00;
    return unk00;
  }
  return 0;
}

/* TODO: [near miss] 99.54%; scope-search key, count and result registers remain. */
static s32 _TraceClusterScopes(u8* arg0, u8** arg1) {
  s32 unk00 = *(s32*)(arg0 + 4);
  u32 unk08;
  u8* unk04 = *(u8**)(arg0 + 8);
  do {
    u32 unk44;
    u8* unk48;
    for (unk08 = 0; unk08 < (unk44 = *(u32*)(unk48 = *(u8**)(*(u8**)(unk04 + 0x1C) + 8))); ++unk08) {
      u8* unk0C = projectDepEntry(unk48, unk08, unk44);
      if (*(u8**)(unk0C + 0xC) == 0) {
        u8* unk10 = projectDepNewScope(arg1);
        *(u8**)(unk0C + 0xC) = unk10;
        if (unk10 == 0) {
          BaerrState unk14;
          unk14.unk00 = 1;
          unk14.unk04 = _rwerror(0x20);
          RwErrorSet(&unk14);
          return 0x20;
        }
        {
          u8** unk18 = *(u8***)(unk0C + 0xC);
          *(u8**)(unk0C + 0x10) = *unk18;
          *unk18 = unk0C;
        }
      }
    }
    {
      u32 unk1C;
      for (unk1C = 0; unk1C < *(u32*)(unk04 + 4); ++unk1C) {
        u32 unk20 = (*(u32**)(unk04 + 8))[unk1C];
        if (unk20 != 0xFFFFFFFFU) {
          u8* unk24 = *(u8**)(arg0 + 8) + unk20 * 0x28;
          u32 unk4C;
          u8* unk50;
          for (unk08 = 0; unk08 < (unk4C = *(u32*)(unk50 = *(u8**)(*(u8**)(unk24 + 0x1C) + 8))); ++unk08) {
            projectDepTraceEntry(unk04,
                projectDepEntry(unk50, unk08, unk4C), unk1C, arg1);
          }
        }
      }
    }
    --unk00;
    unk04 += 0x28;
  } while (unk00 != 0);
  return 0;
}

static s32 _AssignClusterSlots(u8* arg0, const u8* arg1) {
  u32 unk00 = 0;
  u8* unk04 = *(u8* const*)arg1;
  while (unk04 != 0) {
    u32 unk08 = 0;
    u8* unk0C = unk04;
    u32 unk14;
    do {
      u8* unk10 = *(u8**)unk0C;
      while (unk10 != 0) {
        unk08 |= *(u32*)(*(u8**)(unk10 + 0x14) + 8);
        unk10 = *(u8**)(unk10 + 0x10);
      }
      unk0C = *(u8**)(unk0C + 4);
    } while (unk0C != 0);
    unk14 = 0;
    while (unk08 & 1U) {
      unk08 >>= 1;
      unk14++;
    }
    if (unk14 >= unk00) {
      unk00 = unk14 + 1;
    }
    unk0C = unk04;
    do {
      u8* unk18 = *(u8**)unk0C;
      while (unk18 != 0) {
        *(u32*)(unk18 + 0x1C) = unk14;
        *(u32*)(*(u8**)(unk18 + 0x14) + 8) |= 1U << unk14;
        unk18 = *(u8**)(unk18 + 0x10);
      }
      unk0C = *(u8**)(unk0C + 4);
    } while (unk0C != 0);
    unk04 = *(u8**)(unk04 + 8);
  }
  *(u32*)(arg0 + 0xC) = unk00;
  return 0;
}

static void _MyEnumPipelineClustersCallBack(u8* arg0, u8* arg1, u8** arg2) {
  u8* unk00 = StalacMiteAlloc(8);
  if (*arg2 == 0) {
    *arg2 = unk00;
  }
  *(u8**)unk00 = arg0;
  *(u32*)(unk00 + 4) = *(u32*)(arg0 + 8);
}

static inline u32 projectEnumerateUniqueClusters(
    u8* arg0, void (*arg1)(u8*, u8*, u8**), void* arg2) {
  u32 unk00 = 0;
  u8* unk04;
  for (unk04 = arg0; unk04 != 0; unk04 = *(u8**)(unk04 + 8)) {
    u8* unk08 = arg0;
    while (unk08 != unk04) {
      if (*(u8**)*(u8**)unk08 == *(u8**)*(u8**)unk04) {
        break;
      }
      unk08 = *(u8**)(unk08 + 8);
    }
    if (unk08 == unk04) {
      if (arg1 != 0) {
        arg1(*(u8**)*(u8**)unk04, unk04, arg2);
      }
      ++unk00;
    }
  }
  return unk00;
}

/* TODO: [near miss] 96.35%; register allocation and temporary staging remain. */
static s32 _ForAllNodesWriteClusterAllocations(u8* arg0, u8* arg1) {
  u32 unk04;
  u8* unk08;
  u32 unk0C;
  u32 unk10;
  u32 unk00;
  {
    u8* unk14 = *(u8**)(arg0 + 8);
    u32 unk4C;
    unk00 = 0;
    for (unk4C = 0;
         unk4C < (unk10 = *(u32*)(*(u8**)(*(u8**)(unk14 + 0x1C) + 8)));
         ++unk4C) {
      u8* unk30 = projectDepEntry(
          *(u8**)(*(u8**)(unk14 + 0x1C) + 8), unk4C, unk10);
      if (*(s32*)(unk30 + 4) == 1 || *(s32*)(unk30 + 4) == 2) {
        ++unk00;
      }
    }
  }
  unk04 = projectEnumerateUniqueClusters(arg1, 0, 0);
  {
    u8* unk18 = StalacTiteAlloc(0);
    u8* unk1C = StalacMiteAlloc(0);
    s32 unk48 = unk18 - unk1C;
    memset(unk1C, 0, unk48);
  }
  *(u8**)(arg0 + 0x14) =
      StalacMiteAlloc((*(u32*)(arg0 + 0xC) - 1) * 0x1C + 0x30);
  *(u16*)(*(u8**)(arg0 + 0x14) + 2) = *(u32*)(arg0 + 0xC);
  *(u8**)(*(u8**)(arg0 + 0x14) + 4) = arg0;
  *(u32*)(arg0 + 0x10) = 0;
  unk08 = 0;
  projectEnumerateUniqueClusters(arg1, _MyEnumPipelineClustersCallBack, &unk08);
  for (unk0C = 0; unk0C < *(u32*)(arg0 + 4); ++unk0C) {
    u8* unk20 = *(u8**)(arg0 + 8) + unk0C * 0x28;
    u32 unk24;
    u32 unk28;
    if (*(u32*)(arg0 + 0xC) != 0) {
      *(u8**)(unk20 + 0xC) = StalacMiteAlloc(*(u32*)(arg0 + 0xC) * 4);
    }
    *(u8**)(unk20 + 0x10) = StalacMiteAlloc((*(u32*)(arg0 + 0xC) + 1) * 4);
    if (*(u32*)(*(u8**)unk20 + 0x20) != 0) {
      *(u8**)(unk20 + 0x18) = StalacMiteAlloc(*(u32*)(*(u8**)unk20 + 0x20) * 4);
    }
    if (*(u32*)(*(u8**)unk20 + 0x34) != 0) {
      *(u8**)(unk20 + 0x14) =
          StalacMiteAlloc((*(u32*)(*(u8**)unk20 + 0x34) + 3) & ~3U);
    }
    unk28 = ~0U;
    for (unk24 = 0;
         unk24 < (unk10 = *(u32*)(*(u8**)(*(u8**)(unk20 + 0x1C) + 8)));
         ++unk24) {
      u8* unk2C = 0;
      u8* unk30 = projectDepEntry(
          *(u8**)(*(u8**)(unk20 + 0x1C) + 8), unk24, unk10);
      u32 unk34;
      for (unk34 = 0; unk34 < unk04; ++unk34) {
        u8* unk38 = unk08 + unk34 * 8;
        if (*(u8**)unk38 == *(u8**)unk30) {
          unk2C = unk38;
          break;
        }
      }
      (*(u8***)(unk20 + 0xC))[*(u32*)(unk30 + 0x1C)] = unk2C;
      (*(u32**)(unk20 + 0x10))[*(u32*)(unk30 + 0x1C) + 1] = *(u32*)(unk30 + 0x18);
      unk28 = *(u32*)(unk30 + 0x18) & unk28;
    }
    **(u32**)(unk20 + 0x10) = unk28;
    for (unk24 = 0; unk24 < *(u32*)(*(u8**)unk20 + 0x20); ++unk24) {
      u32 unk3C;
      (*(u32**)(unk20 + 0x18))[unk24] = ~0U;
      for (unk3C = 0; unk3C < *(u32*)(arg0 + 0xC); ++unk3C) {
        u8* unk38 = (*(u8***)(unk20 + 0xC))[unk3C];
        if (unk38 != 0 &&
            *(ProjectClusterDef**)unk38 ==
                (*(ProjectNodeCluster**)(*(u8**)unk20 + 0x24))[unk24].unk00) {
          (*(u32**)(unk20 + 0x18))[unk24] = unk3C;
          break;
        }
      }
    }
  }
  if (unk00 != 0) {
    *(u8**)(arg0 + 0x1C) = StalacMiteAlloc(unk00 * 12);
  }
  {
    u32 unk40 = 0;
    u8* unk20 = *(u8**)(arg0 + 8);
    u32 unk50;
    for (unk50 = 0;
         unk50 < (unk10 = *(u32*)(*(u8**)(*(u8**)(unk20 + 0x1C) + 8)));
         ++unk50) {
      u8* unk30 = projectDepEntry(
          *(u8**)(*(u8**)(unk20 + 0x1C) + 8), unk50, unk10);
      if (*(s32*)(unk30 + 4) == 1 || *(s32*)(unk30 + 4) == 2) {
        u8* unk44 = *(u8**)(arg0 + 0x1C) + unk40 * 12;
        *(u32*)(unk44 + 0) = *(u32*)unk30;
        *(u32*)(unk44 + 4) = *(u32*)(unk30 + 4);
        *(u32*)(unk44 + 8) = *(u32*)(unk30 + 0x1C);
        ++unk40;
      }
    }
    *(u32*)(arg0 + 0x18) = unk40;
  }
  _rx_rxRadixExchangeSort(*(u8**)(arg0 + 0x1C),
                         *(u32*)(arg0 + 0x18), 12, 0, 0, ~0U);
  return 0;
}

u32 _rxChaseDependencies(u8* unk00) {
  u8* unk04 = 0;
  u32 unk08 = _PropagateDependenciesAndKillDeadPaths(unk00);
  if (unk08 == 0) {
    unk08 = _ForAllNodeReqsAddOutputClustersAndBuildContinuityBitfields(unk00);
    if (unk08 == 0) {
      unk08 = _TraceClusterScopes(unk00, &unk04);
      if (unk08 == 0) {
        unk08 = _AssignClusterSlots(unk00, (const u8*)&unk04);
        if (unk08 == 0) {
          unk08 = _ForAllNodesWriteClusterAllocations(unk00, unk04);
        }
      }
    }
  }
  return unk08;
}
