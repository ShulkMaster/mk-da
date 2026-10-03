#include <dolphin/types.h>

static void InitLeaf(u8* arg0) {
  arg0[0x18] = 0xFF;
  *(f32*)arg0 = 0.0f;
  *(f32*)(arg0 + 4) = 0.0f;
  *(f32*)(arg0 + 8) = 0.0f;
  *(f32*)(arg0 + 0xC) = 0.0f;
  *(f32*)(arg0 + 0x10) = 0.0f;
  *(f32*)(arg0 + 0x14) = 0.0f;
}

static void InitBranch(s32* arg0) {
  s32 unk00;
  for (unk00 = 0; unk00 < 16; unk00++) {
    arg0[unk00] = 0;
  }
}

#include <renderware/project_state.h>

static u8* CreateCube(u8* unk00) {
  return (*(u8* (**)(u8*))(RwEngineInstance + 0x140))(unk00);
}

static u8* AllocateToLeaf(u8* arg0, u8* arg1, u32 arg2, s32 arg3) {
  u32 unk00;
  if (arg3 == 0) {
    return arg1;
  }
  unk00 = arg2 & 0xF;
  if (((u8**)arg1)[unk00] == 0) {
    u8* unk04 = CreateCube(*(u8**)(arg0 + 0xC));
    ((u8**)arg1)[unk00] = unk04;
    if (1 == arg3) {
      InitLeaf(unk04);
    } else {
      InitBranch((s32*)unk04);
    }
  }
  arg1 = AllocateToLeaf(arg0, ((u8**)arg1)[unk00], arg2 >> 4, arg3 - 1);
  return arg1;
}

static u32 splice[256];

void _rwPalQuantAddImage(u8* arg0, const u8* arg1, f32 arg2) {
  s32 unk14;
  s32 unk0C;
  s32 unk00;
  const u8* unk04;
  const u8* unk08;
  unk00 = *(const s32*)(arg1 + 0x10);
  unk04 = *(const u8* const*)(arg1 + 0x14);
  unk08 = *(const u8* const*)(arg1 + 0x18);
  unk0C = *(const s32*)(arg1 + 8);
  switch (*(const s32*)(arg1 + 0xC)) {
  case 4:
  case 8:
    while (unk0C-- != 0) {
      const u8* unk10 = unk04;
      unk14 = *(const s32*)(arg1 + 4);
      while (unk14-- != 0) {
        const u8* unk18 = unk08 + (*unk10 << 2);
        {
          u32 unk20 = splice[unk18[3] >> 3] |
              ((splice[unk18[0] >> 3] << 3) | (splice[unk18[1] >> 3] << 2) |
              (splice[unk18[2] >> 3] << 1));
          f32* unk24 = (f32*)AllocateToLeaf(arg0, *(u8**)(arg0 + 8), unk20, 5);
          f32 unk28 = 0.003921569f * unk18[0];
          f32 unk2C = 0.003921569f * unk18[1];
          f32 unk30 = 0.003921569f * unk18[2];
          f32 unk34 = 0.003921569f * unk18[3];
          unk24[0] += arg2;
          unk28 *= arg2;
          unk2C *= arg2;
          unk30 *= arg2;
          unk34 *= arg2;
          unk24[1] += unk28;
          unk24[2] += unk2C;
          unk24[3] += unk30;
          unk24[4] += unk34;
          unk24[5] += arg2 * (unk34 * unk34 +
              (unk30 * unk30 + (unk28 * unk28 + unk2C * unk2C)));
        }
        unk10++;
      }
      unk04 += unk00;
    }
    break;
  case 32:
    while (unk0C-- != 0) {
      const u8* unk18 = unk04;
      unk14 = *(const s32*)(arg1 + 4);
      while (unk14-- != 0) {
        {
          u32 unk20 = splice[unk18[3] >> 3] |
              ((splice[unk18[0] >> 3] << 3) | (splice[unk18[1] >> 3] << 2) |
              (splice[unk18[2] >> 3] << 1));
          f32* unk24 = (f32*)AllocateToLeaf(arg0, *(u8**)(arg0 + 8), unk20, 5);
          f32 unk28 = 0.003921569f * unk18[0];
          f32 unk2C = 0.003921569f * unk18[1];
          f32 unk30 = 0.003921569f * unk18[2];
          f32 unk34 = 0.003921569f * unk18[3];
          unk24[0] += arg2;
          unk28 *= arg2;
          unk2C *= arg2;
          unk30 *= arg2;
          unk34 *= arg2;
          unk24[1] += unk28;
          unk24[2] += unk2C;
          unk24[3] += unk30;
          unk24[4] += unk34;
          unk24[5] += arg2 * (unk34 * unk34 +
              (unk30 * unk30 + (unk28 * unk28 + unk2C * unk2C)));
        }
        unk18 += 4;
      }
      unk04 += unk00;
    }
    break;
  }
}

static void assignindex(u8* unk00, const u8* unk04, s32 unk08, const u8* unk0C, u32 unk10) {
  if (unk00 != 0) {
    s32 unk14 = 1 << unk08;
    s32 unk1C = unk04[0] - unk0C[4];
    s32 unk24 = unk04[1] - unk0C[5];
    s32 unk2C = unk04[2] - unk0C[6];
    s32 unk34 = unk04[3] - unk0C[7];
    if (unk1C >= 0 || unk24 >= 0 || unk2C >= 0 || unk34 >= 0) {
      return;
    }
    {
      s32 unk38 = unk0C[0] - unk04[0];
      s32 unk3C = unk0C[1] - unk04[1];
      s32 unk40 = unk0C[2] - unk04[2];
      s32 unk44 = unk0C[3] - unk04[3];
      if (unk38 >= unk14 || unk3C >= unk14 || unk40 >= unk14 || unk44 >= unk14) {
        return;
      }
      if (unk1C <= -unk14 && unk24 <= -unk14 && unk2C <= -unk14 && unk34 <= -unk14 &&
          unk38 <= 0 && unk3C <= 0 && unk40 <= 0 && unk44 <= 0 && unk08 == 0) {
        unk00[0x18] = unk10;
      } else {
        s32 unk48;
        for (unk48 = 0; unk48 < 16; ++unk48) {
          u8 unk4C[4];
          unk4C[0] = unk04[0] + (((unk48 >> 3) & 1) << (unk08 - 1));
          unk4C[1] = unk04[1] + (((unk48 >> 2) & 1) << (unk08 - 1));
          unk4C[2] = unk04[2] + (((unk48 >> 1) & 1) << (unk08 - 1));
          unk4C[3] = unk04[3] + ((unk48 & 1) << (unk08 - 1));
          assignindex(((u8**)unk00)[unk48], unk4C, unk08 - 1, unk0C, unk10);
        }
      }
    }
  }
}

static void addvolume(u8* arg0, const u8* arg1, s32 arg2,
    const u8* arg3, u8* arg4) {
  if (arg0 != 0) {
    s32 unk00 = 1 << arg2;
    s32 unk04 = arg1[0] - arg3[4];
    s32 unk08 = arg1[1] - arg3[5];
    s32 unk0C = arg1[2] - arg3[6];
    s32 unk10 = arg1[3] - arg3[7];
    if (!(unk04 < 0 && unk08 < 0 && unk0C < 0) || unk10 >= 0) {
      return;
    }
    {
      s32 unk14;
      s32 unk18;
      s32 unk1C;
      s32 unk20;
      unk14 = arg3[0] - arg1[0];
      unk18 = arg3[1] - arg1[1];
      unk1C = arg3[2] - arg1[2];
      unk20 = arg3[3] - arg1[3];
      if (!(unk14 < unk00 && unk18 < unk00 && unk1C < unk00) ||
          unk20 >= unk00) {
        return;
      }
      if (unk04 <= -unk00 && unk08 <= -unk00 &&
          unk0C <= -unk00 && unk10 <= -unk00 &&
          unk14 <= 0 && unk18 <= 0 && unk1C <= 0 && unk20 <= 0 &&
          arg2 == 0) {
        ((f32*)arg4)[0] += ((f32*)arg0)[0];
        ((f32*)arg4)[1] += ((f32*)arg0)[1];
        ((f32*)arg4)[2] += ((f32*)arg0)[2];
        ((f32*)arg4)[3] += ((f32*)arg0)[3];
        ((f32*)arg4)[4] += ((f32*)arg0)[4];
        ((f32*)arg4)[5] += ((f32*)arg0)[5];
      } else {
        s32 unk24;
        for (unk24 = 0; unk24 < 16; ++unk24) {
          u8 unk28[4];
          unk28[0] = arg1[0] + ((((u32)unk24 >> 3) & 1) << (arg2 - 1));
          unk28[1] = arg1[1] + ((((u32)unk24 >> 2) & 1) << (arg2 - 1));
          unk28[2] = arg1[2] + ((((u32)unk24 >> 1) & 1) << (arg2 - 1));
          unk28[3] = arg1[3] + ((unk24 & 1) << (arg2 - 1));
          addvolume(*(u8**)(arg0 + unk24 * 4), unk28, arg2 - 1, arg3, arg4);
        }
      }
    }
  }
}

typedef struct {
  u8 unk00[8];
} ProjectPalBounds;

typedef struct {
  f32 unk00[6];
  u8 unk18;
} ProjectPalLeaf;

static inline f32 projectPalSquaredMoments(f32 unk00, f32 unk04,
    f32 unk08, f32 unk0C, f32 unk10) {
  return (unk0C * unk0C + (unk08 * unk08 +
      (unk00 * unk00 + unk04 * unk04))) / unk10;
}

/* TODO: [breakthrough] 95.86%; floating-point register allocation remains. */
static f32 nMaximize(u8* unk00, const u8* unk04, s32 unk08,
    s32* unk0C, const f32* unk10) {
  f32 unk14 = 0.0f;
  f32 unk18 = 0.0f;
  ProjectPalLeaf unk1C;
  ProjectPalBounds unk20;
  s32 unk24;
  *unk0C = -1;
  unk20 = *(const ProjectPalBounds*)unk04;
  switch (unk08) {
  case 1: {
    u8 unk28[4];
    for (unk24 = unk04[0]; unk24 < unk04[4]; ++unk24) {
      f32 unk2C;
      f32 unk30;
      f32 unk34;
      f32 unk38;
      f32 unk3C;
      f32 unk40;
      unk20.unk00[4] = unk24;
      unk28[0] = 0;
      unk28[1] = 0;
      unk28[2] = 0;
      unk28[3] = 0;
      InitLeaf((u8*)&unk1C);
      addvolume(unk00, unk28, 5, unk20.unk00, (u8*)&unk1C);
      unk2C = unk10[1] - unk1C.unk00[1];
      unk30 = unk10[2] - unk1C.unk00[2];
      unk34 = unk10[3] - unk1C.unk00[3];
      unk38 = unk10[4] - unk1C.unk00[4];
      unk3C = unk10[0] - unk1C.unk00[0];
      if (unk1C.unk00[0] > 0.0f && unk3C > 0.0f) {
        unk40 = projectPalSquaredMoments(unk1C.unk00[1], unk1C.unk00[2],
            unk1C.unk00[3], unk1C.unk00[4], unk1C.unk00[0]) +
            projectPalSquaredMoments(unk2C, unk30, unk34, unk38, unk3C);
        if (unk40 > unk14) {
          unk14 = unk40;
          *unk0C = unk24;
        } else if (unk40 < unk18) {
          break;
        }
        unk18 = unk40;
      }
    }
    break;
  }
  case 2: {
    u8 unk28[4];
    for (unk24 = unk04[1]; unk24 < unk04[5]; ++unk24) {
      f32 unk2C;
      f32 unk30;
      f32 unk34;
      f32 unk38;
      f32 unk3C;
      f32 unk40;
      unk20.unk00[5] = unk24;
      unk28[0] = 0;
      unk28[1] = 0;
      unk28[2] = 0;
      unk28[3] = 0;
      InitLeaf((u8*)&unk1C);
      addvolume(unk00, unk28, 5, unk20.unk00, (u8*)&unk1C);
      unk2C = unk10[1] - unk1C.unk00[1];
      unk30 = unk10[2] - unk1C.unk00[2];
      unk34 = unk10[3] - unk1C.unk00[3];
      unk38 = unk10[4] - unk1C.unk00[4];
      unk3C = unk10[0] - unk1C.unk00[0];
      if (unk1C.unk00[0] > 0.0f && unk3C > 0.0f) {
        unk40 = projectPalSquaredMoments(unk1C.unk00[1], unk1C.unk00[2],
            unk1C.unk00[3], unk1C.unk00[4], unk1C.unk00[0]) +
            projectPalSquaredMoments(unk2C, unk30, unk34, unk38, unk3C);
        if (unk40 > unk14) {
          unk14 = unk40;
          *unk0C = unk24;
        } else if (unk40 < unk18) {
          break;
        }
        unk18 = unk40;
      }
    }
    break;
  }
  case 3: {
    u8 unk28[4];
    for (unk24 = unk04[2]; unk24 < unk04[6]; ++unk24) {
      f32 unk2C;
      f32 unk30;
      f32 unk34;
      f32 unk38;
      f32 unk3C;
      f32 unk40;
      unk20.unk00[6] = unk24;
      unk28[0] = 0;
      unk28[1] = 0;
      unk28[2] = 0;
      unk28[3] = 0;
      InitLeaf((u8*)&unk1C);
      addvolume(unk00, unk28, 5, unk20.unk00, (u8*)&unk1C);
      unk2C = unk10[1] - unk1C.unk00[1];
      unk30 = unk10[2] - unk1C.unk00[2];
      unk34 = unk10[3] - unk1C.unk00[3];
      unk38 = unk10[4] - unk1C.unk00[4];
      unk3C = unk10[0] - unk1C.unk00[0];
      if (unk1C.unk00[0] > 0.0f && unk3C > 0.0f) {
        unk40 = projectPalSquaredMoments(unk1C.unk00[1], unk1C.unk00[2],
            unk1C.unk00[3], unk1C.unk00[4], unk1C.unk00[0]) +
            projectPalSquaredMoments(unk2C, unk30, unk34, unk38, unk3C);
        if (unk40 > unk14) {
          unk14 = unk40;
          *unk0C = unk24;
        } else if (unk40 < unk18) {
          break;
        }
        unk18 = unk40;
      }
    }
    break;
  }
  case 4: {
    u8 unk28[4];
    for (unk24 = unk04[3]; unk24 < unk04[7]; ++unk24) {
      f32 unk2C;
      f32 unk30;
      f32 unk34;
      f32 unk38;
      f32 unk3C;
      f32 unk40;
      unk20.unk00[7] = unk24;
      unk28[0] = 0;
      unk28[1] = 0;
      unk28[2] = 0;
      unk28[3] = 0;
      InitLeaf((u8*)&unk1C);
      addvolume(unk00, unk28, 5, unk20.unk00, (u8*)&unk1C);
      unk2C = unk10[1] - unk1C.unk00[1];
      unk30 = unk10[2] - unk1C.unk00[2];
      unk34 = unk10[3] - unk1C.unk00[3];
      unk38 = unk10[4] - unk1C.unk00[4];
      unk3C = unk10[0] - unk1C.unk00[0];
      if (unk1C.unk00[0] > 0.0f && unk3C > 0.0f) {
        unk40 = projectPalSquaredMoments(unk1C.unk00[1], unk1C.unk00[2],
            unk1C.unk00[3], unk1C.unk00[4], unk1C.unk00[0]) +
            projectPalSquaredMoments(unk2C, unk30, unk34, unk38, unk3C);
        if (unk40 > unk14) {
          unk14 = unk40;
          *unk0C = unk24;
        } else if (unk40 < unk18) {
          break;
        }
        unk18 = unk40;
      }
    }
    break;
  }
  }
  return unk14;
}

static s32 nCut(u8* unk00, u8* unk04, u8* unk08) {
  typedef struct {
    u32 unk00;
    u32 unk04;
  } ProjectQuantWords;
  s32 unk10;
  s32 unk14;
  s32 unk18;
  s32 unk1C;
  u8 unk0C[4];
  ProjectPalLeaf unk20;
  f32 unk24;
  f32 unk28;
  f32 unk2C;
  f32 unk30;
  unk0C[0] = 0;
  unk0C[1] = 0;
  unk0C[2] = 0;
  unk0C[3] = 0;
  InitLeaf((u8*)&unk20);
  addvolume(unk00, unk0C, 5, unk04, (u8*)&unk20);
  unk24 = nMaximize(unk00, unk04, 1, &unk10, unk20.unk00);
  unk28 = nMaximize(unk00, unk04, 2, &unk14, unk20.unk00);
  unk2C = nMaximize(unk00, unk04, 3, &unk18, unk20.unk00);
  unk30 = nMaximize(unk00, unk04, 4, &unk1C, unk20.unk00);
  if (unk24 == 0.0f && unk28 == 0.0f && unk2C == 0.0f && unk30 == 0.0f) {
    return 0;
  }
  *(ProjectQuantWords*)unk08 = *(ProjectQuantWords*)unk04;
  if (unk24 >= unk28) {
    if (unk24 >= unk2C) {
      if (unk24 >= unk30) {
        unk04[4] = unk08[0] = unk10;
      } else {
        unk04[7] = unk08[3] = unk1C;
      }
    } else if (unk2C >= unk30) {
      unk04[6] = unk08[2] = unk18;
    } else {
      unk04[7] = unk08[3] = unk1C;
    }
  } else if (unk28 >= unk2C) {
    if (unk28 >= unk30) {
      unk04[5] = unk08[1] = unk14;
    } else {
      unk04[7] = unk08[3] = unk1C;
    }
  } else if (unk2C >= unk30) {
    unk04[6] = unk08[2] = unk18;
  } else {
    unk04[7] = unk08[3] = unk1C;
  }
  return 1;
}

static s32 CountLeafs(const u8* unk00, s32 unk04) {
  s32 unk0C;
  s32 unk08 = 0;
  if (unk00 != 0) {
    if (unk04 > 0) {
      for (unk0C = 0; unk0C < 16; ++unk0C) {
        unk08 += CountLeafs(*(u8* const*)(unk00 + unk0C * 4), unk04 - 1);
      }
    } else {
      unk08 = 1;
    }
  }
  return unk08;
}

/* TODO: [near miss] 97.66%; recursive register allocation remains. */
static s32 ExtractNodes(u8* unk00, u8* unk04, s32 unk08, s32 unk0C) {
  s32 unk10;
  if (unk00 != 0) {
    if (unk0C > 0) {
      for (unk10 = 0; unk10 < 16; ++unk10) {
        u8* unk1C = ((u8**)unk00)[unk10];
        unk08 = ExtractNodes(unk1C, unk04, unk08, unk0C - 1);
      }
    } else {
      f32 unk14 = *(f32*)unk00;
      f32 unk18 = 0.0f;
      if (unk14 > 0.0f) {
        unk18 = 255.9999f / unk14;
      }
      unk04[unk08 * 4] = (u8)(*(f32*)(unk00 + 0x04) * unk18);
      unk04[unk08 * 4 + 1] = (u8)(*(f32*)(unk00 + 0x08) * unk18);
      unk04[unk08 * 4 + 2] = (u8)(*(f32*)(unk00 + 0x0C) * unk18);
      unk04[unk08 * 4 + 3] = (u8)(*(f32*)(unk00 + 0x10) * unk18);
      unk00[0x18] = unk08;
      ++unk08;
    }
  }
  return unk08;
}

static inline void projectPalInitializeVolume(ProjectPalLeaf* arg0, const u8* arg1,
    u8* arg2, const u8* arg3) {
  InitLeaf((u8*)arg0);
  addvolume(arg2, arg1, 5, arg3, (u8*)arg0);
}

static inline f32 projectPalVolumeVariance(u8* arg0, const u8* arg1) {
  u8 unk00[4];
  ProjectPalLeaf unk04;
  unk00[0] = 0;
  unk00[1] = 0;
  unk00[2] = 0;
  unk00[3] = 0;
  projectPalInitializeVolume(&unk04, unk00, arg0, arg1);
  return unk04.unk00[5] - (unk04.unk00[4] * unk04.unk00[4] +
      (unk04.unk00[3] * unk04.unk00[3] +
      (unk04.unk00[1] * unk04.unk00[1] + unk04.unk00[2] * unk04.unk00[2]))) /
      unk04.unk00[0];
}

/* TODO: [breakthrough] 92.59%; register allocation and call argument staging remain. */
s32 _rwPalQuantResolvePalette(u8* arg0, s32 arg1, u8* arg2) {
  u8 unk38[4];
  u8 unk3C[4];
  ProjectPalLeaf unk40;
  s32 unk00 = arg1;
  s32 unk04 = CountLeafs(*(u8**)(arg2 + 8), 5);
  if (arg1 >= unk04) {
    s32 unk08;
    unk00 = unk04;
    unk08 = ExtractNodes(*(u8**)(arg2 + 8), arg0, 0, 5);
    for (; unk08 < arg1; ++unk08) {
      arg0[unk08 * 4] = 0;
      arg0[unk08 * 4 + 1] = 0;
      arg0[unk08 * 4 + 2] = 0;
      arg0[unk08 * 4 + 3] = 0;
    }
  } else {
    s32 unk0C;
    s32 unk10;
    (*(u8**)arg2)[0] = 0;
    (*(u8**)arg2)[1] = 0;
    (*(u8**)arg2)[2] = 0;
    (*(u8**)arg2)[3] = 0;
    (*(u8**)arg2)[4] = 32;
    (*(u8**)arg2)[5] = 32;
    (*(u8**)arg2)[6] = 32;
    (*(u8**)arg2)[7] = 32;
    (*(f32**)(arg2 + 4))[0] = projectPalVolumeVariance(
        *(u8**)(arg2 + 8), *(u8**)arg2);
    for (unk0C = 1; unk0C < arg1; ++unk0C) {
      s32 unk1C = -1;
      f32 unk20 = 0.0f;
      for (unk10 = 0; unk10 < unk0C; ++unk10) {
        if ((*(f32**)(arg2 + 4))[unk10] > unk20) {
          unk20 = (*(f32**)(arg2 + 4))[unk10];
          unk1C = unk10;
        }
      }
      if (unk1C == -1) {
        break;
      }
      if (nCut(*(u8**)(arg2 + 8), *(u8**)arg2 + unk1C * 8,
          *(u8**)arg2 + unk0C * 8) != 0) {
        (*(f32**)(arg2 + 4))[unk1C] = projectPalVolumeVariance(
            *(u8**)(arg2 + 8), *(u8**)arg2 + unk1C * 8);
        (*(f32**)(arg2 + 4))[unk0C] = projectPalVolumeVariance(
            *(u8**)(arg2 + 8), *(u8**)arg2 + unk0C * 8);
      } else {
        (*(f32**)(arg2 + 4))[unk1C] = 0.0f;
        --unk0C;
      }
    }
    for (unk10 = 0; unk10 < arg1; ++unk10) {
      if (unk10 < arg1) {
        f32 unk44;
        unk38[0] = 0;
        unk38[1] = 0;
        unk38[2] = 0;
        unk38[3] = 0;
        assignindex(*(u8**)(arg2 + 8), unk38, 5,
            *(u8**)arg2 + unk10 * 8, unk10);
        unk3C[0] = 0;
        unk3C[1] = 0;
        unk3C[2] = 0;
        unk3C[3] = 0;
        projectPalInitializeVolume(&unk40, unk3C, *(u8**)(arg2 + 8),
            *(u8**)arg2 + unk10 * 8);
        unk44 = unk40.unk00[0] > 0.0f ?
            255.9999f / unk40.unk00[0] : 0.0f;
        arg0[unk10 * 4] = (u8)(unk40.unk00[1] * unk44);
        arg0[unk10 * 4 + 1] = (u8)(unk40.unk00[2] * unk44);
        arg0[unk10 * 4 + 2] = (u8)(unk40.unk00[3] * unk44);
        arg0[unk10 * 4 + 3] = (u8)(unk40.unk00[4] * unk44);
      } else {
        arg0[unk10 * 4] = 0;
        arg0[unk10 * 4 + 1] = 0;
        arg0[unk10 * 4 + 2] = 0;
        arg0[unk10 * 4 + 3] = 0;
      }
    }
  }
  return unk00;
}

static s32 GetIndex(const u8* unk00, u32 unk04, s32 unk08) {
  if (unk08 == 0) {
    return unk00[0x18];
  }
  return GetIndex(*(u8* const*)(unk00 + (unk04 & 15) * 4),
      unk04 >> 4, unk08 - 1);
}

void _rwPalQuantMatchImage(u8* unk00, s32 unk04, s32 unk08, s32 unk0C,
    const u8* unk10, const u8* unk14) {
  u32 unk1C;
  u32 unk2C;
  u32 unk30;
  s32 unk16 = *(const s32*)(unk14 + 0x10);
  const u8* unk18 = *(const u8* const*)(unk14 + 0x14);
  u8* unk28;
  const u8* unk40;
  const u8* unk44;
  u32 unk48;
  if (unk08 == 4 && unk0C != 0) {
    switch (*(const s32*)(unk14 + 0xC)) {
    case 4:
    case 8: {
        const u8* unk20 = *(const u8* const*)(unk14 + 0x18);
        unk1C = *(const u32*)(unk14 + 8);
        while (unk1C-- != 0) {
          const u8* unk24 = unk18;
          unk28 = unk00;
          unk2C = *(const u32*)(unk14 + 4);
          for (unk30 = 0; unk30 < unk2C; ++unk30) {
            const u8* unk34 = unk20 + ((u8)*unk24 << 2);
            s32 unk3C;
            u32 unk38 = splice[(s32)unk34[3] >> 3] |
                ((splice[(s32)unk34[2] >> 3] << 1) |
                ((splice[(s32)unk34[0] >> 3] << 3) |
                (splice[(s32)unk34[1] >> 3] << 2)));
            unk24 += 1;
            unk3C = GetIndex(*(u8* const*)(unk10 + 8), unk38, 5);
            if (unk30 & 1) {
              *unk28 &= 0x0F;
              *unk28 |= (unk3C << 4) & 0xF0;
              ++unk28;
            } else {
              *unk28 &= 0xF0;
              *unk28 |= unk3C & 0x0F;
            }
          }
          unk18 += unk16;
          unk00 += unk04;
        }
        break;
      }
    case 32: {
        unk1C = *(const u32*)(unk14 + 8);
        while (unk1C-- != 0) {
          const u8* unk24 = unk18;
          unk28 = unk00;
          unk48 = *(const u32*)(unk14 + 4);
          for (unk30 = 0; unk30 < unk48; ++unk30) {
            s32 unk3C;
            u32 unk38 = splice[(s32)unk24[3] >> 3] |
                ((splice[(s32)unk24[2] >> 3] << 1) |
                ((splice[(s32)unk24[0] >> 3] << 3) |
                (splice[(s32)unk24[1] >> 3] << 2)));
            unk24 += 4;
            unk3C = GetIndex(*(u8* const*)(unk10 + 8), unk38, 5);
            if (unk30 & 1) {
              *unk28 &= 0x0F;
              *unk28 |= (unk3C << 4) & 0xF0;
              ++unk28;
            } else {
              *unk28 &= 0xF0;
              *unk28 |= unk3C & 0x0F;
            }
          }
          unk18 += unk16;
          unk00 += unk04;
        }
        break;
      }
    }
  } else {
    switch (*(const s32*)(unk14 + 0xC)) {
    case 4:
    case 8: {
        const u8* unk20 = *(const u8* const*)(unk14 + 0x18);
        unk1C = *(const u32*)(unk14 + 8);
        while (unk1C-- != 0) {
          const u8* unk24 = unk18;
          unk28 = unk00;
          unk2C = *(const u32*)(unk14 + 4);
          while (unk2C-- != 0) {
            const u8* unk34 = unk20 + ((u8)*unk24 << 2);
            s32 unk3C;
            u32 unk38 = splice[(s32)unk34[3] >> 3] |
                ((splice[(s32)unk34[2] >> 3] << 1) |
                ((splice[(s32)unk34[0] >> 3] << 3) |
                (splice[(s32)unk34[1] >> 3] << 2)));
            unk24 += 1;
            unk40 = *(u8* const*)(unk10 + 8);
            unk3C = GetIndex(unk40, unk38, 5);
            *unk28++ = unk3C;
          }
          unk18 += unk16;
          unk00 += unk04;
        }
        break;
      }
    case 32: {
        unk1C = *(const u32*)(unk14 + 8);
        while (unk1C-- != 0) {
          const u8* unk24 = unk18;
          unk28 = unk00;
          unk2C = *(const u32*)(unk14 + 4);
          while (unk2C-- != 0) {
            s32 unk3C;
            u32 unk38 = splice[(s32)unk24[3] >> 3] |
                ((splice[(s32)unk24[2] >> 3] << 1) |
                ((splice[(s32)unk24[0] >> 3] << 3) |
                (splice[(s32)unk24[1] >> 3] << 2)));
            unk24 += 4;
            unk44 = *(u8* const*)(unk10 + 8);
            unk3C = GetIndex(unk44, unk38, 5);
            *unk28++ = unk3C;
          }
          unk18 += unk16;
          unk00 += unk04;
        }
        break;
      }
    }
  }
}

#include <renderware/project_memory.h>

s32 _rwPalQuantInit(u8* arg0) {
  s32 unk00;
  s32 unk08;
  for (unk00 = 0; unk00 < 32; unk00++) {
    u32 unk04 = 0;
    for (unk08 = 0; unk08 < 5; unk08++) {
      unk04 |= (unk00 & (1 << unk08)) != 0 ?
          1U << ((4 - unk08) * 4) : 0U;
    }
    splice[unk00] = unk04;
  }
  *(u8**)(arg0) = (*(u8* (**)(u32, u32))(RwEngineInstance + 0x13C))(8, 256);
  *(u8**)(arg0 + 4) = (*(u8* (**)(u32, u32))(RwEngineInstance + 0x13C))(4, 256);
  *(u8**)(arg0 + 0xC) = RwFreeListCreate(64, 64, 0);
  *(u8**)(arg0 + 8) = (*(u8* (**)(u8*))(RwEngineInstance + 0x140))(*(u8**)(arg0 + 0xC));
  InitBranch(*(s32**)(arg0 + 8));
  return 1;
}

static void DeleteOctTree(u8* unk00, u8* unk04, s32 unk08) {
  if (unk04 != 0) {
    if (unk08 > 0) {
      s32 unk0C;
      for (unk0C = 0; unk0C < 16; unk0C++) {
        DeleteOctTree(unk00, ((u8**)unk04)[unk0C], unk08 - 1);
      }
    }
    (*(void* (**)(u8*, void*))(RwEngineInstance + 0x144))(
        *(u8**)(unk00 + 0xC), unk04);
  }
}

#include <renderware/project_memory.h>

void _rwPalQuantTerm(u8* unk00) {
  DeleteOctTree(unk00, *(u8**)(unk00 + 8), 5);
  *(u8**)(unk00 + 8) = 0;
  RwFreeListDestroy(*(u8**)(unk00 + 0xC));
  (*(void (**)(void*))(RwEngineInstance + 0x134))(*(void**)(unk00 + 4));
  (*(void (**)(void*))(RwEngineInstance + 0x134))(*(void**)unk00);
}
