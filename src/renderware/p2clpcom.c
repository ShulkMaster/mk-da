#include <dolphin/types.h>

u32 _rwForAllEdgesWithRxInterpolant(
    u8* unk00, u8* unk04, u8* unk08, u16* unk0C, s32* unk10, s32* unk14,
    u32 unk18, f32 (*unk1C)(u8*, u8*, u8*, u8*, u8*, u8*)) {
  s32 unk20 = *unk10;
  s32 unk24 = *unk14;
  s32 unk28 = unk24;
  u8* unk3C;
  u8* unk40;
  u16 unk2C = unk0C[(unk24 - 1) & 15];
  u16 unk30 = unk0C[unk20];
  u8* unk34 = *(u8**)(unk00 + 4) + *(u16*)(unk00 + 2) * unk2C;
  u8* unk38 = *(u8**)(unk04 + 4) + *(u16*)(unk04 + 2) * unk2C;
  unk3C = *(u8**)(unk00 + 4) + *(u16*)(unk00 + 2) * unk30;
  unk40 = *(u8**)(unk04 + 4) + *(u16*)(unk04 + 2) * unk30;

  do {
    if ((unk34[12] & unk18) == 0) {
      unk0C[unk24] = unk2C;
      unk24 = (unk24 + 1) & 15;
    }
    if (((unk34[12] ^ unk3C[12]) & unk18) != 0) {
      u16 unk48;
      u8* unk44 = *(u8**)(unk08 + 8);
      u8* unk4C;
      u8* unk50;
      unk48 = (*(u32*)(unk00 + 16))++;
      (*(u32*)(unk04 + 16))++;
      unk0C[unk24] = unk48;
      unk24 = (unk24 + 1) & 15;
      unk4C = *(u8**)(unk00 + 4) + *(u16*)(unk00 + 2) * unk48;
      unk50 = *(u8**)(unk04 + 4) + *(u16*)(unk04 + 2) * unk48;
      if ((unk34[12] & unk18) != 0) {
        *(f32*)(unk44 + 8) = unk1C(unk4C, unk34, unk3C, unk50, unk38, unk40);
        *(u16*)(unk44 + 0) = unk48;
        *(u16*)(unk44 + 2) = unk2C;
        *(u16*)(unk44 + 4) = unk30;
      } else {
        *(f32*)(unk44 + 8) = unk1C(unk4C, unk3C, unk34, unk50, unk40, unk38);
        *(u16*)(unk44 + 0) = unk48;
        *(u16*)(unk44 + 2) = unk30;
        *(u16*)(unk44 + 4) = unk2C;
      }
      (*(u32*)(unk08 + 16))++;
      *(u8**)(unk08 + 8) += *(u16*)(unk08 + 2);
    }
    unk20 = (unk20 + 1) & 15;
    unk2C = unk30;
    unk34 = unk3C;
    unk38 = unk40;
    unk30 = unk0C[unk20];
    unk3C = *(u8**)(unk00 + 4) + *(u16*)(unk00 + 2) * unk30;
    unk40 = *(u8**)(unk04 + 4) + *(u16*)(unk04 + 2) * unk30;
  } while (unk20 != unk28);
  *unk10 = unk20;
  *unk14 = unk24;
  return (unk24 - unk20) & 15;
}

/* TODO: [near miss] 99.86%; three condition-result registers remain. */
u32 _rwForAllEdgesWithoutRxInterpolant(u8* unk00, u8* unk04, u8* unk08,
    u16* unk0C, s32* unk10, s32* unk14, u32 unk18,
    f32 (*unk1C)(u8*, u8*, u8*, u8*, u8*, u8*)) {
  s32 unk20 = *unk10;
  s32 unk24 = *unk14;
  u8* unk3C;
  u8* unk40;
  u16 unk30;
  s32 unk28 = unk24;
  u16 unk2C = unk0C[(unk24 - 1) & 15];
  u8* unk34;
  u8* unk38;
  unk30 = unk0C[unk20];
  unk34 = *(u8**)(unk00 + 4) + *(u16*)(unk00 + 2) * unk2C;
  unk38 = *(u8**)(unk04 + 4) + *(u16*)(unk04 + 2) * unk2C;
  unk3C = *(u8**)(unk00 + 4) + *(u16*)(unk00 + 2) * unk30;
  unk40 = *(u8**)(unk04 + 4) + *(u16*)(unk04 + 2) * unk30;
  do {
    if ((unk34[0xC] & unk18) == 0) {
      unk0C[unk24] = unk2C;
      unk24 = (unk24 + 1) & 15;
    }
    if (unk18 & (unk34[0xC] ^ unk3C[0xC])) {
      u16 unk44 = (*(u32*)(unk00 + 0x10))++;
      u8* unk48;
      u8* unk4C;
      (*(u32*)(unk04 + 0x10))++;
      unk0C[unk24] = unk44;
      unk24 = (unk24 + 1) & 15;
      unk48 = *(u8**)(unk00 + 4) + *(u16*)(unk00 + 2) * unk44;
      unk4C = *(u8**)(unk04 + 4) + *(u16*)(unk04 + 2) * unk44;
      if (unk34[0xC] & unk18)
        unk1C(unk48, unk34, unk3C, unk4C, unk38, unk40);
      else
        unk1C(unk48, unk3C, unk34, unk4C, unk40, unk38);
    }
    unk20 = (unk20 + 1) & 15;
    unk2C = unk30;
    unk34 = unk3C;
    unk38 = unk40;
    unk30 = unk0C[unk20];
    unk3C = *(u8**)(unk00 + 4) + *(u16*)(unk00 + 2) * unk30;
    unk40 = *(u8**)(unk04 + 4) + *(u16*)(unk04 + 2) * unk30;
  } while (unk20 != unk28);
  *unk10 = unk20;
  *unk14 = unk24;
  return (unk24 - unk20) & 15;
}

u32 _rwForAllEdges(u8* unk0, u8* unk4, u8* unk8, u16* unkC,
    s32* unk10, s32* unk14, u32 unk18,
    f32 (*unk1C)(u8*, u8*, u8*, u8*, u8*, u8*)) {
  return (unk8 != 0 ? _rwForAllEdgesWithRxInterpolant :
      _rwForAllEdgesWithoutRxInterpolant)(unk0, unk4, unk8, unkC, unk10, unk14, unk18, unk1C);
}

u32 _rwForOneEdge(u8* arg0, u8* arg1, u16* arg2, s32* arg3, s32* arg4,
    u32 arg5, f32 (*arg6)(u8*, u8*, u8*, u8*, u8*, u8*)) {
  s32 unk30;
  s32 unk04;
  u8* unk14;
  u16 unk0C;
  s32 unk00;
  u8* unk10;
  u8* unk18;
  u8* unk1C;
  u16 unk08;
  u32 unk2C;
  unk30 = *arg3;
  unk04 = *arg4;
  unk00 = unk04;
  unk08 = arg2[(unk30 + 1) & 15];
  unk0C = arg2[unk30];
  unk10 = *(u8**)(arg0 + 4) + *(u16*)(arg0 + 2) * unk08;
  unk18 = *(u8**)(arg1 + 4) + *(u16*)(arg1 + 2) * unk08;
  unk14 = *(u8**)(arg0 + 4) + *(u16*)(arg0 + 2) * unk0C;
  unk1C = *(u8**)(arg1 + 4) + *(u16*)(arg1 + 2) * unk0C;
  if (!(unk10[0xC] & arg5)) {
    arg2[unk04] = unk08;
    unk04 = (unk04 + 1) & 15;
  }
  unk2C = arg5 & (unk10[0xC] ^ unk14[0xC]);
  if (unk2C) {
    u16 unk20 = (*(u32*)(arg0 + 0x10))++;
    u8* unk24;
    u8* unk28;
    ++*(u32*)(arg1 + 0x10);
    arg2[unk04] = unk20;
    unk04 = (unk04 + 1) & 15;
    unk24 = *(u8**)(arg0 + 4) + *(u16*)(arg0 + 2) * unk20;
    unk28 = *(u8**)(arg1 + 4) + *(u16*)(arg1 + 2) * unk20;
    unk2C = unk10[0xC] & arg5;
    if (unk2C) {
      arg6(unk24, unk10, unk14, unk28, unk18, unk1C);
    } else {
      arg6(unk24, unk14, unk10, unk28, unk1C, unk18);
    }
  }
  unk2C = unk14[0xC] & arg5;
  if (!unk2C) {
    arg2[unk04] = unk0C;
    unk04 = (unk04 + 1) & 15;
  }
  *arg3 = *arg4;
  *arg4 = unk04;
  return (unk04 - unk00) & 15;
}

#include "renderware/project_clip.h"

f32 _rwClipInfoGlobal[10];

f32 _rwGeneratePerspClippedVertexZLO(u8* arg0, u8* arg1, u8* arg2,
    u8* arg3, u8* arg4, u8* arg5) {
  f32 unk04;
  f32 unk08;
  f32 unk00;
  s32 unk10;

  unk00 = *(const f32*)(arg1 + 8);
  unk04 = (_rwClipInfoGlobal[2] - unk00) / (*(const f32*)(arg2 + 8) - unk00);
  unk00 = *(const f32*)(arg1 + 0);
  unk08 = *(const f32*)(arg2 + 0) - unk00;
  unk08 = unk04 * unk08;
  *(f32*)(arg0 + 0) = unk00 + unk08;
  unk00 = *(const f32*)(arg1 + 4);
  unk08 = *(const f32*)(arg2 + 4) - unk00;
  unk08 = unk04 * unk08;
  *(f32*)(arg0 + 4) = unk00 + unk08;
  *(f32*)(arg0 + 8) = _rwClipInfoGlobal[2];
  unk00 = *(const f32*)(arg1 + 32);
  unk08 = *(const f32*)(arg2 + 32) - unk00;
  unk08 = unk04 * unk08;
  *(f32*)(arg0 + 32) = unk00 + unk08;
  unk00 = *(const f32*)(arg1 + 36);
  unk08 = *(const f32*)(arg2 + 36) - unk00;
  unk08 = unk04 * unk08;
  *(f32*)(arg0 + 36) = unk00 + unk08;
  {
    u8 unk0C = arg4[12];
    arg3[12] = (u8)((f32)unk0C + unk04 * (arg5[12] - unk0C));
  }
  {
    u8 unk0C = arg4[13];
    arg3[13] = (u8)((f32)unk0C + unk04 * (arg5[13] - unk0C));
  }
  {
    u8 unk0C = arg4[14];
    arg3[14] = (u8)((f32)unk0C + unk04 * (arg5[14] - unk0C));
  }
  {
    u8 unk0C = arg4[15];
    arg3[15] = (u8)((f32)unk0C + unk04 * (arg5[15] - unk0C));
  }
  arg0[0xC] = 0x10;
  unk00 = *(f32*)(arg0 + 4);
  if (unk00 < 0.0f) {
    unk10 = 4;
  } else if (unk00 > *(f32*)(arg0 + 8)) {
    unk10 = 8;
  } else {
    unk10 = 0;
  }
  arg0[0xC] |= unk10;
  unk00 = *(f32*)arg0;
  if (unk00 < 0.0f) {
    unk10 = 1;
  } else if (unk00 > *(f32*)(arg0 + 8)) {
    unk10 = 2;
  } else {
    unk10 = 0;
  }
  arg0[0xC] |= unk10;
  return unk04;
}

f32 _rwGeneratePerspClippedVertexZHI(u8* unk00, u8* unk04, u8* unk08,
    u8* unk0C, u8* unk10, u8* unk14) {
  f32 unk18 = *(f32*)(unk04 + 8);
  f32 unk1C = (_rwClipInfoGlobal[3] - unk18) /
      (*(f32*)(unk08 + 8) - unk18);

  unk18 = *(f32*)(unk04 + 0);
  *(f32*)(unk00 + 0) = unk18 + unk1C * (*(f32*)(unk08 + 0) - unk18);
  unk18 = *(f32*)(unk04 + 4);
  *(f32*)(unk00 + 4) = unk18 + unk1C * (*(f32*)(unk08 + 4) - unk18);
  *(f32*)(unk00 + 8) = _rwClipInfoGlobal[3];
  unk18 = *(f32*)(unk04 + 32);
  *(f32*)(unk00 + 32) = unk18 + unk1C * (*(f32*)(unk08 + 32) - unk18);
  unk18 = *(f32*)(unk04 + 36);
  *(f32*)(unk00 + 36) = unk18 + unk1C * (*(f32*)(unk08 + 36) - unk18);
  {
    u8 unk20 = unk10[12];
    unk0C[12] = (u8)((f32)unk20 + unk1C * (unk14[12] - unk20));
  }
  {
    u8 unk20 = unk10[13];
    unk0C[13] = (u8)((f32)unk20 + unk1C * (unk14[13] - unk20));
  }
  {
    u8 unk20 = unk10[14];
    unk0C[14] = (u8)((f32)unk20 + unk1C * (unk14[14] - unk20));
  }
  {
    u8 unk20 = unk10[15];
    unk0C[15] = (u8)((f32)unk20 + unk1C * (unk14[15] - unk20));
  }
  unk00[12] = 32;
  {
    s32 unk24;
    unk18 = *(f32*)(unk00 + 4);
    if (unk18 < 0.0f) {
      unk24 = 4;
    } else if (unk18 > *(f32*)(unk00 + 8)) {
      unk24 = 8;
    } else {
      unk24 = 0;
    }
    unk00[12] |= unk24;
  }
  {
    s32 unk28;
    unk18 = *(f32*)(unk00 + 0);
    if (unk18 < 0.0f) {
      unk28 = 1;
    } else if (unk18 > *(f32*)(unk00 + 8)) {
      unk28 = 2;
    } else {
      unk28 = 0;
    }
    unk00[12] |= unk28;
  }
  return unk1C;
}

f32 _rwGeneratePerspClippedVertexYLO(u8* unk00, u8* unk04, u8* unk08,
    u8* unk0C, u8* unk10, u8* unk14) {
  f32 unk18 = *(f32*)(unk04 + 4) /
      (*(f32*)(unk04 + 4) - *(f32*)(unk08 + 4));
  u32 unk1C;
  *(f32*)(unk00 + 0) = *(f32*)(unk04 + 0) +
      unk18 * (*(f32*)(unk08 + 0) - *(f32*)(unk04 + 0));
  *(f32*)(unk00 + 8) = *(f32*)(unk04 + 8) +
      unk18 * (*(f32*)(unk08 + 8) - *(f32*)(unk04 + 8));
  *(f32*)(unk00 + 4) = 0.0f;
  *(f32*)(unk00 + 0x20) = *(f32*)(unk04 + 0x20) +
      unk18 * (*(f32*)(unk08 + 0x20) - *(f32*)(unk04 + 0x20));
  *(f32*)(unk00 + 0x24) = *(f32*)(unk04 + 0x24) +
      unk18 * (*(f32*)(unk08 + 0x24) - *(f32*)(unk04 + 0x24));
  unk0C[0xC] = (u8)((f32)unk10[0xC] +
      unk18 * (f32)(unk14[0xC] - unk10[0xC]));
  unk0C[0xD] = (u8)((f32)unk10[0xD] +
      unk18 * (f32)(unk14[0xD] - unk10[0xD]));
  unk0C[0xE] = (u8)((f32)unk10[0xE] +
      unk18 * (f32)(unk14[0xE] - unk10[0xE]));
  unk0C[0xF] = (u8)((f32)unk10[0xF] +
      unk18 * (f32)(unk14[0xF] - unk10[0xF]));
  unk00[0xC] = (unk04[0xC] & ~0xFU) | 4;
  if (*(f32*)(unk00 + 0) < 0.0f)
    unk1C = 1;
  else if (*(f32*)(unk00 + 0) > *(f32*)(unk00 + 8))
    unk1C = 2;
  else
    unk1C = 0;
  unk00[0xC] |= unk1C;
  return unk18;
}

f32 _rwGeneratePerspClippedVertexYHI(u8* unk00, u8* unk04, u8* unk08,
    u8* unk0C, u8* unk10, u8* unk14) {
  f32 unk18 = (*(f32*)(unk04 + 4) - *(f32*)(unk04 + 8)) /
      ((*(f32*)(unk04 + 4) - *(f32*)(unk04 + 8)) -
       (*(f32*)(unk08 + 4) - *(f32*)(unk08 + 8)));
  u32 unk1C;
  *(f32*)(unk00 + 0) = *(f32*)(unk04 + 0) +
      unk18 * (*(f32*)(unk08 + 0) - *(f32*)(unk04 + 0));
  *(f32*)(unk00 + 8) = *(f32*)(unk04 + 8) +
      unk18 * (*(f32*)(unk08 + 8) - *(f32*)(unk04 + 8));
  *(f32*)(unk00 + 4) = *(f32*)(unk00 + 8);
  *(f32*)(unk00 + 0x20) = *(f32*)(unk04 + 0x20) +
      unk18 * (*(f32*)(unk08 + 0x20) - *(f32*)(unk04 + 0x20));
  *(f32*)(unk00 + 0x24) = *(f32*)(unk04 + 0x24) +
      unk18 * (*(f32*)(unk08 + 0x24) - *(f32*)(unk04 + 0x24));
  unk0C[0xC] = (u8)((f32)unk10[0xC] +
      unk18 * (f32)(unk14[0xC] - unk10[0xC]));
  unk0C[0xD] = (u8)((f32)unk10[0xD] +
      unk18 * (f32)(unk14[0xD] - unk10[0xD]));
  unk0C[0xE] = (u8)((f32)unk10[0xE] +
      unk18 * (f32)(unk14[0xE] - unk10[0xE]));
  unk0C[0xF] = (u8)((f32)unk10[0xF] +
      unk18 * (f32)(unk14[0xF] - unk10[0xF]));
  unk00[0xC] = (unk04[0xC] & ~0xFU) | 8;
  if (*(f32*)(unk00 + 0) < 0.0f)
    unk1C = 1;
  else if (*(f32*)(unk00 + 0) > *(f32*)(unk00 + 8))
    unk1C = 2;
  else
    unk1C = 0;
  unk00[0xC] |= unk1C;
  return unk18;
}

f32 _rwGeneratePerspClippedVertexXLO(u8* unk00, u8* unk04, u8* unk08,
    u8* unk0C, u8* unk10, u8* unk14) {
  f32 unk18 = *(f32*)(unk04 + 0) /
      (*(f32*)(unk04 + 0) - *(f32*)(unk08 + 0));
  *(f32*)(unk00 + 4) = *(f32*)(unk04 + 4) +
      unk18 * (*(f32*)(unk08 + 4) - *(f32*)(unk04 + 4));
  *(f32*)(unk00 + 8) = *(f32*)(unk04 + 8) +
      unk18 * (*(f32*)(unk08 + 8) - *(f32*)(unk04 + 8));
  *(f32*)(unk00 + 0) = 0.0f;
  *(f32*)(unk00 + 0x20) = *(f32*)(unk04 + 0x20) +
      unk18 * (*(f32*)(unk08 + 0x20) - *(f32*)(unk04 + 0x20));
  *(f32*)(unk00 + 0x24) = *(f32*)(unk04 + 0x24) +
      unk18 * (*(f32*)(unk08 + 0x24) - *(f32*)(unk04 + 0x24));
  unk0C[0xC] = (u8)((f32)unk10[0xC] +
      unk18 * (f32)(unk14[0xC] - unk10[0xC]));
  unk0C[0xD] = (u8)((f32)unk10[0xD] +
      unk18 * (f32)(unk14[0xD] - unk10[0xD]));
  unk0C[0xE] = (u8)((f32)unk10[0xE] +
      unk18 * (f32)(unk14[0xE] - unk10[0xE]));
  unk0C[0xF] = (u8)((f32)unk10[0xF] +
      unk18 * (f32)(unk14[0xF] - unk10[0xF]));
  unk00[0xC] = (unk04[0xC] & ~3U) | 1;
  return unk18;
}

f32 _rwGeneratePerspClippedVertexXHI(u8* unk0, u8* unk4, u8* unk8,
    u8* unkC, u8* unk10, u8* unk14) {
  f32 unk18 = *(f32*)(unk4 + 0) - *(f32*)(unk4 + 8);
  f32 unk1C = unk18 / (unk18 - (*(f32*)(unk8 + 0) - *(f32*)(unk8 + 8)));
  *(f32*)(unk0 + 4) = *(f32*)(unk4 + 4) + unk1C * (*(f32*)(unk8 + 4) - *(f32*)(unk4 + 4));
  *(f32*)(unk0 + 8) = *(f32*)(unk4 + 8) + unk1C * (*(f32*)(unk8 + 8) - *(f32*)(unk4 + 8));
  *(f32*)(unk0 + 0) = *(f32*)(unk0 + 8);
  *(f32*)(unk0 + 0x20) = *(f32*)(unk4 + 0x20) + unk1C * (*(f32*)(unk8 + 0x20) - *(f32*)(unk4 + 0x20));
  *(f32*)(unk0 + 0x24) = *(f32*)(unk4 + 0x24) + unk1C * (*(f32*)(unk8 + 0x24) - *(f32*)(unk4 + 0x24));
  {
    u8 unk20 = unk10[0xC];
    unkC[0xC] = (u8)((f32)unk20 + unk1C * (f32)((s32)unk14[0xC] - (s32)unk20));
  }
  {
    u8 unk20 = unk10[0xD];
    unkC[0xD] = (u8)((f32)unk20 + unk1C * (f32)((s32)unk14[0xD] - (s32)unk20));
  }
  {
    u8 unk20 = unk10[0xE];
    unkC[0xE] = (u8)((f32)unk20 + unk1C * (f32)((s32)unk14[0xE] - (s32)unk20));
  }
  {
    u8 unk20 = unk10[0xF];
    unkC[0xF] = (u8)((f32)unk20 + unk1C * (f32)((s32)unk14[0xF] - (s32)unk20));
  }
  unk0[0xC] = (unk4[0xC] & ~3U) | 2;
  return unk1C;
}

f32 _rwGenerateParallelZLOClippedVertex(u8* unk00, u8* unk04, u8* unk08,
    u8* unk0C, u8* unk10, u8* unk14) {
  f32 unk18 = (_rwClipInfoGlobal[2] - *(f32*)(unk04 + 8)) /
      (*(f32*)(unk08 + 8) - *(f32*)(unk04 + 8));
  u32 unk1C;
  u32 unk20;
  *(f32*)(unk00 + 0) = *(f32*)(unk04 + 0) +
      unk18 * (*(f32*)(unk08 + 0) - *(f32*)(unk04 + 0));
  *(f32*)(unk00 + 4) = *(f32*)(unk04 + 4) +
      unk18 * (*(f32*)(unk08 + 4) - *(f32*)(unk04 + 4));
  *(f32*)(unk00 + 8) = _rwClipInfoGlobal[2];
  *(f32*)(unk00 + 0x20) = *(f32*)(unk04 + 0x20) +
      unk18 * (*(f32*)(unk08 + 0x20) - *(f32*)(unk04 + 0x20));
  *(f32*)(unk00 + 0x24) = *(f32*)(unk04 + 0x24) +
      unk18 * (*(f32*)(unk08 + 0x24) - *(f32*)(unk04 + 0x24));
  unk0C[0xC] = (u8)((f32)unk10[0xC] +
      unk18 * (f32)(unk14[0xC] - unk10[0xC]));
  unk0C[0xD] = (u8)((f32)unk10[0xD] +
      unk18 * (f32)(unk14[0xD] - unk10[0xD]));
  unk0C[0xE] = (u8)((f32)unk10[0xE] +
      unk18 * (f32)(unk14[0xE] - unk10[0xE]));
  unk0C[0xF] = (u8)((f32)unk10[0xF] +
      unk18 * (f32)(unk14[0xF] - unk10[0xF]));
  unk00[0xC] = 0x10;
  if (*(f32*)(unk00 + 4) < 0.0f)
    unk1C = 4;
  else if (*(f32*)(unk00 + 4) > 1.0f)
    unk1C = 8;
  else
    unk1C = 0;
  unk00[0xC] |= unk1C;
  if (*(f32*)(unk00 + 0) < 0.0f)
    unk20 = 1;
  else if (*(f32*)(unk00 + 0) > 1.0f)
    unk20 = 2;
  else
    unk20 = 0;
  unk00[0xC] |= unk20;
  return unk18;
}

f32 _rwGenerateParallelZHIClippedVertex(u8* unk00, u8* unk04, u8* unk08,
    u8* unk0C, u8* unk10, u8* unk14) {
  f32 unk18 = *(f32*)(unk04 + 8);
  f32 unk1C = (_rwClipInfoGlobal[3] - unk18) /
      (*(f32*)(unk08 + 8) - unk18);

  unk18 = *(f32*)(unk04 + 0);
  *(f32*)(unk00 + 0) = unk18 + unk1C * (*(f32*)(unk08 + 0) - unk18);
  unk18 = *(f32*)(unk04 + 4);
  *(f32*)(unk00 + 4) = unk18 + unk1C * (*(f32*)(unk08 + 4) - unk18);
  *(f32*)(unk00 + 8) = _rwClipInfoGlobal[3];
  unk18 = *(f32*)(unk04 + 32);
  *(f32*)(unk00 + 32) = unk18 + unk1C * (*(f32*)(unk08 + 32) - unk18);
  unk18 = *(f32*)(unk04 + 36);
  *(f32*)(unk00 + 36) = unk18 + unk1C * (*(f32*)(unk08 + 36) - unk18);
  {
    u8 unk20 = unk10[12];
    unk0C[12] = (u8)((f32)unk20 + unk1C * (unk14[12] - unk20));
  }
  {
    u8 unk20 = unk10[13];
    unk0C[13] = (u8)((f32)unk20 + unk1C * (unk14[13] - unk20));
  }
  {
    u8 unk20 = unk10[14];
    unk0C[14] = (u8)((f32)unk20 + unk1C * (unk14[14] - unk20));
  }
  {
    u8 unk20 = unk10[15];
    unk0C[15] = (u8)((f32)unk20 + unk1C * (unk14[15] - unk20));
  }
  unk00[12] = 32;
  {
    s32 unk24;
    unk18 = *(f32*)(unk00 + 4);
    if (unk18 < 0.0f) {
      unk24 = 4;
    } else if (unk18 > 1.0f) {
      unk24 = 8;
    } else {
      unk24 = 0;
    }
    unk00[12] |= unk24;
  }
  {
    s32 unk28;
    unk18 = *(f32*)(unk00 + 0);
    if (unk18 < 0.0f) {
      unk28 = 1;
    } else if (unk18 > 1.0f) {
      unk28 = 2;
    } else {
      unk28 = 0;
    }
    unk00[12] |= unk28;
  }
  return unk1C;
}

f32 _rwGenerateParallelYLOClippedVertex(u8* unk00, u8* unk04, u8* unk08,
    u8* unk0C, u8* unk10, u8* unk14) {
  f32 unk1C = *(f32*)(unk04 + 4) /
      (*(f32*)(unk04 + 4) - *(f32*)(unk08 + 4));
  f32 unk18;

  *(f32*)(unk00 + 0) = *(f32*)(unk04 + 0) +
      unk1C * (*(f32*)(unk08 + 0) - *(f32*)(unk04 + 0));
  *(f32*)(unk00 + 8) = *(f32*)(unk04 + 8) +
      unk1C * (*(f32*)(unk08 + 8) - *(f32*)(unk04 + 8));
  *(f32*)(unk00 + 4) = 0.0f;
  unk18 = *(f32*)(unk04 + 0x20);
  *(f32*)(unk00 + 0x20) = unk18 + unk1C * (*(f32*)(unk08 + 0x20) - unk18);
  unk18 = *(f32*)(unk04 + 0x24);
  *(f32*)(unk00 + 0x24) = unk18 + unk1C * (*(f32*)(unk08 + 0x24) - unk18);
  {
    u8 unk20 = unk10[0xC];
    unk0C[0xC] = (u8)((f32)unk20 + unk1C * (unk14[0xC] - unk20));
  }
  {
    u8 unk20 = unk10[0xD];
    unk0C[0xD] = (u8)((f32)unk20 + unk1C * (unk14[0xD] - unk20));
  }
  {
    u8 unk20 = unk10[0xE];
    unk0C[0xE] = (u8)((f32)unk20 + unk1C * (unk14[0xE] - unk20));
  }
  {
    u8 unk20 = unk10[0xF];
    unk0C[0xF] = (u8)((f32)unk20 + unk1C * (unk14[0xF] - unk20));
  }
  unk00[0xC] = (unk04[0xC] & ~0xFU) | 4;
  {
    s32 unk24;
    f32 unk28 = *(f32*)(unk00 + 0);
    if (unk28 < 0.0f) {
      unk24 = 1;
    } else if (unk28 > 1.0f) {
      unk24 = 2;
    } else {
      unk24 = 0;
    }
    unk00[0xC] |= unk24;
  }
  return unk1C;
}

f32 _rwGenerateParallelYHIClippedVertex(u8* unk00, u8* unk04, u8* unk08,
    u8* unk0C, u8* unk10, u8* unk14) {
  f32 unk18 = *(f32*)(unk04 + 4);
  f32 unk1C = (1.0f - unk18) / (*(f32*)(unk08 + 4) - unk18);

  unk18 = *(f32*)(unk04 + 0);
  *(f32*)(unk00 + 0) = unk18 + unk1C * (*(f32*)(unk08 + 0) - unk18);
  unk18 = *(f32*)(unk04 + 8);
  *(f32*)(unk00 + 8) = unk18 + unk1C * (*(f32*)(unk08 + 8) - unk18);
  *(f32*)(unk00 + 4) = 1.0f;
  unk18 = *(f32*)(unk04 + 0x20);
  *(f32*)(unk00 + 0x20) = unk18 + unk1C * (*(f32*)(unk08 + 0x20) - unk18);
  unk18 = *(f32*)(unk04 + 0x24);
  *(f32*)(unk00 + 0x24) = unk18 + unk1C * (*(f32*)(unk08 + 0x24) - unk18);
  {
    u8 unk20 = unk10[0xC];
    unk0C[0xC] = (u8)((f32)unk20 + unk1C * (unk14[0xC] - unk20));
  }
  {
    u8 unk20 = unk10[0xD];
    unk0C[0xD] = (u8)((f32)unk20 + unk1C * (unk14[0xD] - unk20));
  }
  {
    u8 unk20 = unk10[0xE];
    unk0C[0xE] = (u8)((f32)unk20 + unk1C * (unk14[0xE] - unk20));
  }
  {
    u8 unk20 = unk10[0xF];
    unk0C[0xF] = (u8)((f32)unk20 + unk1C * (unk14[0xF] - unk20));
  }
  unk00[0xC] = (unk04[0xC] & ~0xFU) | 8;
  {
    s32 unk24;
    unk18 = *(f32*)(unk00 + 0);
    if (unk18 < 0.0f) {
      unk24 = 1;
    } else if (unk18 > 1.0f) {
      unk24 = 2;
    } else {
      unk24 = 0;
    }
    unk00[0xC] |= unk24;
  }
  return unk1C;
}

f32 _rwGenerateParallelXLOClippedVertex(u8* unk00, u8* unk04, u8* unk08,
    u8* unk0C, u8* unk10, u8* unk14) {
  f32 unk1C = *(f32*)(unk04 + 0) /
      (*(f32*)(unk04 + 0) - *(f32*)(unk08 + 0));

  *(f32*)(unk00 + 4) = *(f32*)(unk04 + 4) +
      unk1C * (*(f32*)(unk08 + 4) - *(f32*)(unk04 + 4));
  *(f32*)(unk00 + 8) = *(f32*)(unk04 + 8) +
      unk1C * (*(f32*)(unk08 + 8) - *(f32*)(unk04 + 8));
  *(f32*)(unk00 + 0) = 0.0f;
  *(f32*)(unk00 + 0x20) = *(f32*)(unk04 + 0x20) +
      unk1C * (*(f32*)(unk08 + 0x20) - *(f32*)(unk04 + 0x20));
  *(f32*)(unk00 + 0x24) = *(f32*)(unk04 + 0x24) +
      unk1C * (*(f32*)(unk08 + 0x24) - *(f32*)(unk04 + 0x24));
  {
    u8 unk20 = unk10[0xC];
    unk0C[0xC] = (u8)((f32)unk20 + unk1C * (unk14[0xC] - unk20));
  }
  {
    u8 unk20 = unk10[0xD];
    unk0C[0xD] = (u8)((f32)unk20 + unk1C * (unk14[0xD] - unk20));
  }
  {
    u8 unk20 = unk10[0xE];
    unk0C[0xE] = (u8)((f32)unk20 + unk1C * (unk14[0xE] - unk20));
  }
  {
    u8 unk20 = unk10[0xF];
    unk0C[0xF] = (u8)((f32)unk20 + unk1C * (unk14[0xF] - unk20));
  }
  unk00[0xC] = (unk04[0xC] & ~3U) | 1;
  return unk1C;
}

f32 _rwGenerateParallelXHIClippedVertex(u8* unk00, u8* unk04, u8* unk08,
    u8* unk0C, u8* unk10, u8* unk14) {
  f32 unk24;
  f32 unk18 = *(f32*)unk04;
  f32 unk1C = (1.0f - unk18) / (*(f32*)unk08 - unk18);
  unk18 = *(f32*)(unk04 + 4);
  *(f32*)(unk00 + 4) = unk18 + unk1C * (*(f32*)(unk08 + 4) - unk18);
  unk18 = *(f32*)(unk04 + 8);
  *(f32*)(unk00 + 8) = unk18 + unk1C * (*(f32*)(unk08 + 8) - unk18);
  *(f32*)unk00 = 1.0f;
  unk24 = *(f32*)(unk04 + 0x20);
  *(f32*)(unk00 + 0x20) = unk24 + unk1C * (*(f32*)(unk08 + 0x20) - unk24);
  unk24 = *(f32*)(unk04 + 0x24);
  *(f32*)(unk00 + 0x24) = unk24 + unk1C * (*(f32*)(unk08 + 0x24) - unk24);
  {
    u8 unk20 = unk10[12];
    unk0C[12] = (u8)((f32)unk20 + unk1C * (unk14[12] - unk20));
  }
  {
    u8 unk20 = unk10[13];
    unk0C[13] = (u8)((f32)unk20 + unk1C * (unk14[13] - unk20));
  }
  {
    u8 unk20 = unk10[14];
    unk0C[14] = (u8)((f32)unk20 + unk1C * (unk14[14] - unk20));
  }
  {
    u8 unk20 = unk10[15];
    unk0C[15] = (u8)((f32)unk20 + unk1C * (unk14[15] - unk20));
  }
  unk00[0xC] = (unk04[0xC] & ~3U) | 2;
  return unk1C;
}
