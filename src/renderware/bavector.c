#include <dolphin/types.h>
#include <renderware/project_error.h>
#include <renderware/project_state.h>
#include <renderware/project_vector.h>

typedef f32* (*BavectorMult)(f32*, const f32*, s32, const f32*);

static s32 vectorModule[2];

static f32* VectorMultPoint(f32* unk00, const f32* unk04, s32 unk08, const f32* unk0C) {
  f32* unk10 = unk00;
  while (--unk08 >= 0) {
    f32 unk20 = unk04[0];
    f32 unk14 = unk20 * unk0C[0];
    f32 unk18 = unk20 * unk0C[1];
    f32 unk1C = unk20 * unk0C[2];
    f32 unk24;
    unk20 = unk04[1];
    unk24 = unk20 * unk0C[4];
    unk14 += unk24;
    unk24 = unk20 * unk0C[5];
    unk18 += unk24;
    unk24 = unk20 * unk0C[6];
    unk1C += unk24;
    unk20 = unk04[2];
    unk24 = unk20 * unk0C[8];
    unk14 += unk24;
    unk24 = unk20 * unk0C[9];
    unk18 += unk24;
    unk24 = unk20 * unk0C[10];
    unk1C += unk24;
    unk00[0] = unk14 + unk0C[12];
    unk00[1] = unk18 + unk0C[13];
    unk00[2] = unk1C + unk0C[14];
    unk04 += 3;
    unk00 += 3;
  }
  return unk10;
}

static f32* VectorMultVector(f32* unk0, const f32* unk4, s32 unk8, const f32* unkC) {
  f32* unk10 = unk0;
  while (--unk8 >= 0) {
    f32 unk14;
    f32 unk18, unk1C, unk20;
    f32 unk24;
    unk14 = unk4[0];
    unk18 = unk14 * unkC[0];
    unk1C = unk14 * unkC[1];
    unk20 = unk14 * unkC[2];
    unk14 = unk4[1];
    unk24 = unk14 * unkC[4];
    unk18 += unk24;
    unk24 = unk14 * unkC[5];
    unk1C += unk24;
    unk24 = unk14 * unkC[6];
    unk20 += unk24;
    unk14 = unk4[2];
    unk24 = unk14 * unkC[8];
    unk0[0] = unk18 + unk24;
    unk24 = unk14 * unkC[9];
    unk0[1] = unk1C + unk24;
    unk24 = unk14 * unkC[10];
    unk0[2] = unk20 + unk24;
    unk4 += 3;
    unk0 += 3;
  }
  return unk10;
}

s32 _rwVectorSetMultFn(BavectorMult unk00, BavectorMult unk04) {
  if (unk00 == 0) {
    unk00 = VectorMultPoint;
  }
  *(BavectorMult*)(RwEngineInstance + vectorModule[0] + 8) = unk00;
  if (unk04 == 0) {
    unk04 = VectorMultVector;
  }
  *(BavectorMult*)(RwEngineInstance + vectorModule[0] + 12) = unk04;
  return 1;
}

void _rwV3dNormalize(f32* arg0, const f32* arg1) {
  f32 unk00 = _rwInvSqrt(arg1[2] * arg1[2] +
      (arg1[0] * arg1[0] + arg1[1] * arg1[1]));
  arg0[0] = arg1[0] * unk00;
  arg0[1] = arg1[1] * unk00;
  arg0[2] = arg1[2] * unk00;
}

/* TODO: [near miss] 100.00%; zero-literal relocation awaits unit pool layout. */
f32 RwV3dNormalize(f32* unk0, const f32* unk4) {
  f32 unkC;
  f32 unk8 = unk4[2] * unk4[2] + (unk4[0] * unk4[0] + unk4[1] * unk4[1]);
  f32 unk10;
  unkC = _rwSqrt(unk8);
  unk10 = _rwInvSqrt(unk8);
  unk0[0] = unk4[0] * unk10;
  unk0[1] = unk4[1] * unk10;
  unk0[2] = unk4[2] * unk10;
  if (0.0f == unkC) {
    BaerrState unk14;
    unk14.unk00 = 1;
    unk14.unk04 = _rwerror(0x19);
    RwErrorSet(&unk14);
  }
  return unkC;
}

/* TODO: [breakthrough needed] 86.43%; raw-word reloads and an intermediate store differ. */
f32 _rwSqrt(f32 arg0) {
  if (*(u32*)&arg0 != 0) {
    u32* unk00 = *(u32**)(RwEngineInstance + vectorModule[0]);
    *(u32*)&arg0 += 0x800;
    {
      u32 unk04 = (*(u32*)&arg0 >> 12) & 0xFFF;
      *(u32*)&arg0 = (*(u32*)&arg0 >> 1) & 0x3FC00000;
      *(u32*)&arg0 += unk00[unk04];
    }
  }
  return arg0;
}

/* TODO: [near miss] 85.87%; repeated local bit loads remain. */
f32 _rwInvSqrt(f32 arg0) {
  union {
    f32 unk00f;
    u32 unk00u;
  } unk04;
  unk04.unk00f = arg0;
  if (unk04.unk00u != 0) {
    u32* unk08 = *(u32**)(RwEngineInstance + vectorModule[0] + 4);
    unk04.unk00u += 0x800;
    {
      u32 unk10 = ~unk04.unk00u;
      unk10 >>= 1;
      unk10 &= 0x3FC00000U;
      unk04.unk00u = unk10 + unk08[(unk04.unk00u >> 12) & 0xFFF];
    }
  }
  return unk04.unk00f;
}

f32* RwV3dTransformPoints(f32* arg0, const f32* arg1, s32 arg2, const f32* arg3) {
  s32 unk00;
  u8* unk04;
  unk04 = RwEngineInstance;
  unk00 = vectorModule[0] + 8;
  return (*(BavectorMult*)(unk04 + unk00))(arg0, arg1, arg2, arg3);
}

f32 *RwV3dTransformVectors(f32 *unk00, const f32 *unk04, s32 unk08, const f32 *unk0C)
{
  return (*(BavectorMult *)(RwEngineInstance + vectorModule[0] + 12))
      (unk00, unk04, unk08, unk0C);
}

void* _rwVectorClose(void* unk00, s32 unk04, s32 unk08) {
  if (*(void **)(RwEngineInstance + vectorModule[0] + 4) != NULL) {
    (*(void (**)(void *))(RwEngineInstance + 0x134))
        (*(void **)(RwEngineInstance + vectorModule[0] + 4));
    *(void **)(RwEngineInstance + vectorModule[0] + 4) = NULL;
  }
  if (*(void **)(RwEngineInstance + vectorModule[0]) != NULL) {
    (*(void (**)(void *))(RwEngineInstance + 0x134))
        (*(void **)(RwEngineInstance + vectorModule[0]));
    *(void **)(RwEngineInstance + vectorModule[0]) = NULL;
  }
  vectorModule[1]--;
  return unk00;
}

#include <math.h>
#include "renderware/project_error.h"

/* TODO: [near miss] 98.59%; four bit-word reloads and their register staging remain. */
void* _rwVectorOpen(void* arg0, s32 arg1, s32 arg2) {
  s32 unk10;
  vectorModule[0] = arg1;
  *(BavectorMult*)(RwEngineInstance + arg1 + 8) = VectorMultPoint;
  *(BavectorMult*)(RwEngineInstance + vectorModule[0] + 12) = VectorMultVector;
  {
    u32* unk00 = (*(u32* (**)(u32))(RwEngineInstance + 0x130))(0x4000);
    if (unk00 == 0) {
      BaerrState unk04;
      unk04.unk00 = 1;
      unk04.unk04 = _rwerror(0x80000013, 0x4000);
      RwErrorSet(&unk04);
      unk10 = 0;
    } else {
      u32* unk18 = unk00 + 0x800;
      union { f32 unk00; u32 unk04; } unk0C, unk08;
      u32 unk14;
      unk08.unk00 = 1.0f;
      for (unk14 = 0; unk14 < 0x800; unk14++) {
        unk0C.unk00 = sqrt(unk08.unk00);
        unk0C.unk04 -= 0x1FC00000;
        unk18[unk14] = unk0C.unk04;
        unk08.unk04 += 0x1000;
      }
      for (unk14 = 0; unk14 < 0x800; unk14++) {
        unk0C.unk00 = sqrt(unk08.unk00);
        unk0C.unk04 -= 0x20000000;
        unk00[unk14] = unk0C.unk04;
        unk08.unk04 += 0x1000;
      }
      *(u32**)(RwEngineInstance + vectorModule[0]) = unk00;
      unk10 = 1;
    }
  }
  if (!unk10) {
    return 0;
  }
  {
    u32* unk00 = (*(u32* (**)(u32))(RwEngineInstance + 0x130))(0x4000);
    if (unk00 == 0) {
      BaerrState unk04;
      unk04.unk00 = 1;
      unk04.unk04 = _rwerror(0x80000013, 0x4000);
      RwErrorSet(&unk04);
      unk10 = 0;
    } else {
      u32* unk18 = unk00 + 0x800;
      union { f32 unk00; u32 unk04; } unk0C, unk08;
      u32 unk14;
      unk08.unk00 = 1.0f;
      for (unk14 = 0; unk14 < 0x800; unk14++) {
        unk0C.unk00 = 1.0 / sqrt(unk08.unk00);
        unk0C.unk04 -= 0x20000000;
        unk18[unk14] = unk0C.unk04;
        unk08.unk04 += 0x1000;
      }
      for (unk14 = 0; unk14 < 0x800; unk14++) {
        unk0C.unk00 = 1.0 / sqrt(unk08.unk00);
        unk0C.unk04 -= 0x1FC00000;
        unk00[unk14] = unk0C.unk04;
        unk08.unk04 += 0x1000;
      }
      *(u32**)(RwEngineInstance + vectorModule[0] + 4) = unk00;
      unk10 = 1;
    }
  }
  if (!unk10) {
    return 0;
  }
  vectorModule[1]++;
  return arg0;
}
