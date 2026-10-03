#include <dolphin/types.h>
#include <renderware/project_state.h>

static s32 matrixModule[2];

static void MatrixMultiply(u8* unk00, const u8* unk04, const u8* unk08) {
  *(f32*)(unk00) =
      *(const f32*)(unk04 + 0x8) * *(const f32*)(unk08 + 0x20) + (*(const f32*)(unk04) * *(const f32*)(unk08) + *(const f32*)(unk04 + 0x4) * *(const f32*)(unk08 + 0x10));
  *(f32*)(unk00 + 0x4) =
      *(const f32*)(unk04 + 0x8) * *(const f32*)(unk08 + 0x24) + (*(const f32*)(unk04) * *(const f32*)(unk08 + 0x4) + *(const f32*)(unk04 + 0x4) * *(const f32*)(unk08 + 0x14));
  *(f32*)(unk00 + 0x8) =
      *(const f32*)(unk04 + 0x8) * *(const f32*)(unk08 + 0x28) + (*(const f32*)(unk04) * *(const f32*)(unk08 + 0x8) + *(const f32*)(unk04 + 0x4) * *(const f32*)(unk08 + 0x18));
  *(f32*)(unk00 + 0x10) =
      *(const f32*)(unk04 + 0x18) * *(const f32*)(unk08 + 0x20) + (*(const f32*)(unk04 + 0x10) * *(const f32*)(unk08) + *(const f32*)(unk04 + 0x14) * *(const f32*)(unk08 + 0x10));
  *(f32*)(unk00 + 0x14) =
      *(const f32*)(unk04 + 0x18) * *(const f32*)(unk08 + 0x24) + (*(const f32*)(unk04 + 0x10) * *(const f32*)(unk08 + 0x4) + *(const f32*)(unk04 + 0x14) * *(const f32*)(unk08 + 0x14));
  *(f32*)(unk00 + 0x18) =
      *(const f32*)(unk04 + 0x18) * *(const f32*)(unk08 + 0x28) + (*(const f32*)(unk04 + 0x10) * *(const f32*)(unk08 + 0x8) + *(const f32*)(unk04 + 0x14) * *(const f32*)(unk08 + 0x18));
  *(f32*)(unk00 + 0x20) =
      *(const f32*)(unk04 + 0x28) * *(const f32*)(unk08 + 0x20) + (*(const f32*)(unk04 + 0x20) * *(const f32*)(unk08) + *(const f32*)(unk04 + 0x24) * *(const f32*)(unk08 + 0x10));
  *(f32*)(unk00 + 0x24) =
      *(const f32*)(unk04 + 0x28) * *(const f32*)(unk08 + 0x24) + (*(const f32*)(unk04 + 0x20) * *(const f32*)(unk08 + 0x4) + *(const f32*)(unk04 + 0x24) * *(const f32*)(unk08 + 0x14));
  *(f32*)(unk00 + 0x28) =
      *(const f32*)(unk04 + 0x28) * *(const f32*)(unk08 + 0x28) + (*(const f32*)(unk04 + 0x20) * *(const f32*)(unk08 + 0x8) + *(const f32*)(unk04 + 0x24) * *(const f32*)(unk08 + 0x18));
  *(f32*)(unk00 + 0x30) =
      *(const f32*)(unk08 + 0x30) + (*(const f32*)(unk04 + 0x38) * *(const f32*)(unk08 + 0x20) + (*(const f32*)(unk04 + 0x30) * *(const f32*)(unk08) + *(const f32*)(unk04 + 0x34) * *(const f32*)(unk08 + 0x10)));
  *(f32*)(unk00 + 0x34) =
      *(const f32*)(unk08 + 0x34) + (*(const f32*)(unk04 + 0x38) * *(const f32*)(unk08 + 0x24) + (*(const f32*)(unk04 + 0x30) * *(const f32*)(unk08 + 0x4) + *(const f32*)(unk04 + 0x34) * *(const f32*)(unk08 + 0x14)));
  *(f32*)(unk00 + 0x38) =
      *(const f32*)(unk08 + 0x38) + (*(const f32*)(unk04 + 0x38) * *(const f32*)(unk08 + 0x28) + (*(const f32*)(unk04 + 0x30) * *(const f32*)(unk08 + 0x8) + *(const f32*)(unk04 + 0x34) * *(const f32*)(unk08 + 0x18)));
}

#include <renderware/project_vector.h>
typedef struct {
  u32 unk00[3];
} ProjectMatrixSpan;

static inline void projectMatrixNormalize3(f32* vector) {
  f32 scale = _rwInvSqrt(vector[2] * vector[2] +
      (vector[0] * vector[0] + vector[1] * vector[1]));
  vector[0] *= scale;
  vector[1] *= scale;
  vector[2] *= scale;
}

static inline void projectMatrixCross3(f32* result, const f32* a, const f32* b) {
  result[0] = a[1] * b[2] - a[2] * b[1];
  result[1] = a[2] * b[0] - a[0] * b[2];
  result[2] = a[0] * b[1] - a[1] * b[0];
}

static inline f32 projectMatrixAbsDot3(const f32* a, const f32* b) {
  f32 value = a[2] * b[2] + (a[0] * b[0] + a[1] * b[1]);
  return value >= 0.0f ? value : -value;
}

static f32* MatrixOrthoNormalize(f32* result, const f32* input) {
  f32 scale1;
  f32 scale0;
  f32 scale2;
  f32* a;
  f32* b;
  f32* c;
  *(ProjectMatrixSpan*)(result) = *(const ProjectMatrixSpan*)(input);
  *(ProjectMatrixSpan*)(result + 4) = *(const ProjectMatrixSpan*)(input + 4);
  *(ProjectMatrixSpan*)(result + 8) = *(const ProjectMatrixSpan*)(input + 8);
  *(ProjectMatrixSpan*)(result + 12) = *(const ProjectMatrixSpan*)(input + 12);
  scale0 = _rwInvSqrt(result[2] * result[2] +
      (result[0] * result[0] + result[1] * result[1]));
  result[0] *= scale0;
  result[1] *= scale0;
  result[2] *= scale0;
  scale1 = _rwInvSqrt(result[6] * result[6] +
      (result[4] * result[4] + result[5] * result[5]));
  result[4] *= scale1;
  result[5] *= scale1;
  result[6] *= scale1;
  scale2 = _rwInvSqrt(result[10] * result[10] +
      (result[8] * result[8] + result[9] * result[9]));
  result[8] *= scale2;
  result[9] *= scale2;
  result[10] *= scale2;
  if (scale0 > 0.0f) {
    if (scale1 > 0.0f) {
      if (scale2 > 0.0f) {
        f32 dot12 = projectMatrixAbsDot3(result + 4, result + 8);
        f32 dot20 = projectMatrixAbsDot3(result + 8, result);
        f32 dot01 = projectMatrixAbsDot3(result, result + 4);
        if (dot12 < dot20) {
          if (dot12 < dot01) {
            a = result + 4;
            b = result + 8;
            c = result;
          } else {
            a = result;
            b = result + 4;
            c = result + 8;
          }
        } else if (dot20 < dot01) {
          a = result + 8;
          b = result;
          c = result + 4;
        } else {
          a = result;
          b = result + 4;
          c = result + 8;
        }
      } else {
        a = result;
        b = result + 4;
        c = result + 8;
      }
    } else {
      a = result + 8;
      b = result;
      c = result + 4;
    }
  } else {
    a = result + 4;
    b = result + 8;
    c = result;
  }
  projectMatrixCross3(c, a, b);
  projectMatrixNormalize3(c);
  projectMatrixCross3(b, c, a);
  projectMatrixNormalize3(b);
  ((u32*)result)[3] = (((u32*)result)[3] | 3) & ~0x20000U;
  return result;
}

s32 _rwMatrixSetMultFn(void (*unk00)(u8*, const u8*, const u8*)) {
  s32 unk04;
  u8* unk08;
  if (unk00 == 0) {
    unk00 = MatrixMultiply;
  }
  unk08 = RwEngineInstance;
  unk04 = matrixModule[0] + 8;
  *(void (**)(u8*, const u8*, const u8*))(unk08 + unk04) = unk00;
  return 1;
}

f32 _rwMatrixDeterminant(const u8* arg0) {
  f32 unk00 = *(const f32*)(arg0 + 0x14) * *(const f32*)(arg0 + 0x28) -
      *(const f32*)(arg0 + 0x18) * *(const f32*)(arg0 + 0x24);
  f32 unk04 = *(const f32*)(arg0 + 0x18) * *(const f32*)(arg0 + 0x20) -
      *(const f32*)(arg0 + 0x10) * *(const f32*)(arg0 + 0x28);
  f32 unk08 = *(const f32*)(arg0 + 0x10) * *(const f32*)(arg0 + 0x24) -
      *(const f32*)(arg0 + 0x14) * *(const f32*)(arg0 + 0x20);
  return unk08 * *(const f32*)(arg0 + 8) +
      (unk00 * *(const f32*)arg0 + unk04 * *(const f32*)(arg0 + 4));
}

static inline f32 projectMatrixDot(const u8* unk00, const u8* unk04) {
  return *(const f32*)(unk00 + 8) * *(const f32*)(unk04 + 8) +
      (*(const f32*)unk00 * *(const f32*)unk04 +
       *(const f32*)(unk00 + 4) * *(const f32*)(unk04 + 4));
}

f32 _rwMatrixOrthogonalError(const u8* unk00) {
  f32 unk04 = projectMatrixDot(unk00 + 0x10, unk00 + 0x20);
  f32 unk08 = projectMatrixDot(unk00 + 0x20, unk00);
  f32 unk0C = projectMatrixDot(unk00, unk00 + 0x10);
  return unk0C * unk0C + (unk04 * unk04 + unk08 * unk08);
}

f32 _rwMatrixNormalError(const u8* unk00) {
  f32 unk04 = *(const f32*)(unk00 + 8) * *(const f32*)(unk00 + 8) +
      (*(const f32*)(unk00) * *(const f32*)(unk00) +
      *(const f32*)(unk00 + 4) * *(const f32*)(unk00 + 4)) - 1.0f;
  f32 unk08 = *(const f32*)(unk00 + 0x18) * *(const f32*)(unk00 + 0x18) +
      (*(const f32*)(unk00 + 0x10) * *(const f32*)(unk00 + 0x10) +
      *(const f32*)(unk00 + 0x14) * *(const f32*)(unk00 + 0x14)) - 1.0f;
  f32 unk0C = *(const f32*)(unk00 + 0x28) * *(const f32*)(unk00 + 0x28) +
      (*(const f32*)(unk00 + 0x20) * *(const f32*)(unk00 + 0x20) +
      *(const f32*)(unk00 + 0x24) * *(const f32*)(unk00 + 0x24)) - 1.0f;
  return unk0C * unk0C + (unk04 * unk04 + unk08 * unk08);
}

f32 _rwMatrixIdentityError(const f32* arg0) {
  f32 unk00;
  f32 unk04;
  f32 unk08;
  f32 unk0C;
  unk00 = arg0[0] - 1.0f;
  unk00 = arg0[2] * arg0[2] + (unk00 * unk00 + arg0[1] * arg0[1]);
  unk04 = arg0[5] - 1.0f;
  unk04 = arg0[6] * arg0[6] + (arg0[4] * arg0[4] + unk04 * unk04);
  unk08 = arg0[10] - 1.0f;
  unk08 = unk08 * unk08 + (arg0[8] * arg0[8] + arg0[9] * arg0[9]);
  unk0C = arg0[14] * arg0[14] + (arg0[12] * arg0[12] + arg0[13] * arg0[13]);
  return unk0C + (unk08 + (unk00 + unk04));
}


#include <renderware/project_memory.h>

u8* _rwMatrixClose(u8* unk00, s32 unk04, s32 unk08) {
  if (*(u8**)(RwEngineInstance + matrixModule[0]) != 0) {
    RwFreeListDestroy(*(u8**)(RwEngineInstance + matrixModule[0]));
    *(u8**)(RwEngineInstance + matrixModule[0]) = 0;
  }
  --matrixModule[1];
  return unk00;
}

u8* _rwMatrixOpen(u8* arg0, s32 arg1, s32 arg2) {
  extern s32 RwEngineSetMatrixTolerances(const f32*);
  matrixModule[0] = arg1;
  *(u8**)(RwEngineInstance + matrixModule[0]) = RwFreeListCreate(0x40, 0x32, 4);
  if (*(u8**)(RwEngineInstance + matrixModule[0]) == 0) {
    arg0 = 0;
  } else {
    f32 unk00[3] = {0.01f, 0.01f, 0.01f};
    *(u32*)(RwEngineInstance + matrixModule[0] + 4) = 0x20000;
    *(void (**)(u8*, const u8*, const u8*))(RwEngineInstance + matrixModule[0] + 8) = MatrixMultiply;
    RwEngineSetMatrixTolerances(unk00);
    ++matrixModule[1];
  }
  return arg0;
}

s32 RwEngineSetMatrixTolerances(const f32* arg0) {
  u8* unk08 = RwEngineInstance + matrixModule[0] + 0xC;
  *(ProjectMatrixSpan*)unk08 = *(const ProjectMatrixSpan*)arg0;
  return 1;
}

static inline f32 projectMatrixOptimizeNormalError(const u8* unk00) {
  f32 unk08;
  f32 unk04;
  f32 unk0C;
  unk04 = *(const f32*)(unk00 + 8) * *(const f32*)(unk00 + 8) +
      (*(const f32*)(unk00) * *(const f32*)(unk00) +
      *(const f32*)(unk00 + 4) * *(const f32*)(unk00 + 4)) - 1.0f;
  unk08 = *(const f32*)(unk00 + 0x18) * *(const f32*)(unk00 + 0x18) +
      (*(const f32*)(unk00 + 0x10) * *(const f32*)(unk00 + 0x10) +
      *(const f32*)(unk00 + 0x14) * *(const f32*)(unk00 + 0x14)) - 1.0f;
  unk0C = *(const f32*)(unk00 + 0x28) * *(const f32*)(unk00 + 0x28) +
      (*(const f32*)(unk00 + 0x20) * *(const f32*)(unk00 + 0x20) +
      *(const f32*)(unk00 + 0x24) * *(const f32*)(unk00 + 0x24)) - 1.0f;
  return unk0C * unk0C + (unk04 * unk04 + unk08 * unk08);
}

u8* RwMatrixOptimize(u8* arg0, const f32* arg1) {
  u32 unk0C;
  s32 unk08;
  s32 unk00;
  s32 unk04;
  if (arg1 == 0) {
    arg1 = (const f32*)(RwEngineInstance + matrixModule[0] + 0xC);
  }
  unk00 = arg1[0] >= projectMatrixOptimizeNormalError(arg0);
  unk04 = arg1[1] >= _rwMatrixOrthogonalError(arg0);
  {
    s32 unk14;
    unk14 = unk08 = 0;
    if (unk00 && unk04) {
      unk14 = 1;
    }
    if (unk14 != 0) {
      if (arg1[1] >= _rwMatrixIdentityError((const f32*)arg0)) {
        unk08 = 1;
      }
    }
  }
  {
    unk0C = *(u32*)(arg0 + 0xC);
    unk0C = unk00 != 0 ? unk0C | 1U : unk0C & ~1U;
    unk0C = unk04 != 0 ? unk0C | 2U : unk0C & ~2U;
    unk0C = unk08 != 0 ? unk0C | 0x20000U : unk0C & ~0x20000U;
    *(u32*)(arg0 + 0xC) = unk0C;
  }
  return arg0;
}

u8* RwMatrixUpdate(u8* unk00) {
  *(u32*)(unk00 + 0xC) &= ~0x20003U;
  return unk00;
}

#include <renderware/project_matrix.h>

u8* RwMatrixMultiply(u8* unk00, const u8* unk04, const u8* unk08) {
  u32 unk0C = *(const u32*)(unk04 + 0xC);
  u32 unk10 = *(const u32*)(unk08 + 0xC);
  u8* unk14 = RwEngineInstance + matrixModule[0];
  u32 unk18 = *(u32*)(unk14 + 4) & 0x20000;
  if ((unk0C & unk18) != 0) {
    *(ProjectMatrix*)unk00 = *(const ProjectMatrix*)unk08;
  } else if ((unk10 & unk18) != 0) {
    *(ProjectMatrix*)unk00 = *(const ProjectMatrix*)unk04;
  } else {
    u32 unk1C = unk0C & unk10;
    (*(void (**)(u8*, const u8*, const u8*))(unk14 + 8))(unk00, unk04, unk08);
    *(u32*)(unk00 + 0xC) = unk1C;
  }
  return unk00;
}

f32* RwMatrixOrthoNormalize(f32* unk00, const f32* unk04) {
  return MatrixOrthoNormalize(unk00, unk04);
}

#include <renderware/project_matrix.h>
#include <renderware/project_error.h>

u8* RwMatrixRotateOneMinusCosineSine(u8* unk00, const u8* unk04,
    f32 unk08, f32 unk0C, s32 unk10) {
  ProjectMatrix unk14;
  ProjectMatrix unk48;
  f32 unk54;
  f32 unk24;
  f32 unk28;
  f32 unk2C;
  f32 unk30;
  f32 unk34;
  f32 unk38;
  f32 unk18;
  f32 unk1C;
  f32 unk20;
  f32 unk3C;
  f32 unk40;
  f32 unk44;
  unk14.unk00[3] = 3;
  unk54 = 1.0f;
  unk18 = *(const f32*)unk04;
  unk24 = unk54 - unk18 * unk18;
  unk1C = *(const f32*)(unk04 + 4);
  unk28 = unk54 - unk1C * unk1C;
  unk20 = *(const f32*)(unk04 + 8);
  unk2C = unk54 - unk20 * unk20;
  unk24 *= unk08;
  unk28 *= unk08;
  unk2C *= unk08;
  unk30 = unk1C * unk20;
  unk34 = unk20 * unk18;
  unk38 = unk18 * unk1C;
  unk30 *= unk08;
  unk34 *= unk08;
  unk38 *= unk08;
  unk3C = unk18 * unk0C;
  unk40 = unk1C * unk0C;
  unk44 = unk20 * unk0C;
  *(f32*)&unk14.unk00[0] = unk54 - unk24;
  *(f32*)&unk14.unk00[1] = unk38 + unk44;
  *(f32*)&unk14.unk00[2] = unk34 - unk40;
  *(f32*)&unk14.unk00[4] = unk38 - unk44;
  *(f32*)&unk14.unk00[5] = unk54 - unk28;
  *(f32*)&unk14.unk00[6] = unk30 + unk3C;
  *(f32*)&unk14.unk00[8] = unk34 + unk40;
  *(f32*)&unk14.unk00[9] = unk30 - unk3C;
  *(f32*)&unk14.unk00[10] = unk54 - unk2C;
  *(f32*)&unk14.unk00[12] = 0.0f;
  *(f32*)&unk14.unk00[13] = 0.0f;
  *(f32*)&unk14.unk00[14] = 0.0f;
  unk14.unk00[3] &= ~0x20000U;
  switch (unk10) {
  case 0:
    unk14.unk00[3] = *(u32*)(unk00 + 0xC);
    *(ProjectMatrix*)unk00 = unk14;
    *(u32*)(unk00 + 0xC) &= ~0x20000U;
    break;
  case 1: {
    RwMatrixMultiply((u8*)&unk48, (const u8*)&unk14, unk00);
    unk48.unk00[3] = *(u32*)(unk00 + 0xC);
    *(ProjectMatrix*)unk00 = unk48;
    *(u32*)(unk00 + 0xC) &= ~0x20000U;
    break;
  }
  case 2: {
    RwMatrixMultiply((u8*)&unk48, unk00, (const u8*)&unk14);
    unk48.unk00[3] = *(u32*)(unk00 + 0xC);
    *(ProjectMatrix*)unk00 = unk48;
    *(u32*)(unk00 + 0xC) &= ~0x20000U;
    break;
  }
  default: {
    BaerrState unk50;
    unk50.unk00 = 1;
    unk50.unk04 = _rwerror(0x80000003, "Invalid combination type");
    RwErrorSet(&unk50);
    unk00 = 0;
    break;
  }
  }
  return unk00;
}

#include <math.h>
#include <renderware/project_vector.h>

u8* RwMatrixRotate(u8* unk00, const u8* unk04, f32 unk08, s32 unk0C) {
  f32 unk10[3];
  f32 unk1C;
  f32 unk14 = 0.01745329238474369f * unk08;
  f32 unk18 = _rwInvSqrt(*(const f32*)(unk04 + 8) * *(const f32*)(unk04 + 8) +
      (*(const f32*)(unk04) * *(const f32*)(unk04) +
      *(const f32*)(unk04 + 4) * *(const f32*)(unk04 + 4)));
  f32 unk20;
  unk10[0] = *(const f32*)(unk04) * unk18;
  unk10[1] = *(const f32*)(unk04 + 4) * unk18;
  unk10[2] = *(const f32*)(unk04 + 8) * unk18;
  unk1C = (f32)sin(unk14);
  unk20 = 1.0f - (f32)cos(unk14);
  RwMatrixRotateOneMinusCosineSine(unk00, (const u8*)unk10,
      unk20, unk1C, unk0C);
  return unk00;
}

#include <renderware/project_matrix.h>

u8* RwMatrixInvert(u8* result, const u8* input) {
  u32 flags = *(const u32*)(input + 0xC);
  if (flags & (*(u32*)(RwEngineInstance + matrixModule[0] + 4) & 0x20000)) {
    *(ProjectMatrix*)result = *(const ProjectMatrix*)input;
    return result;
  }
  if ((flags & 3) == 3) {
    *(f32*)(result + 0x0) = *(const f32*)(input + 0x0);
    *(f32*)(result + 0x4) = *(const f32*)(input + 0x10);
    *(f32*)(result + 0x8) = *(const f32*)(input + 0x20);
    *(f32*)(result + 0x10) = *(const f32*)(input + 0x4);
    *(f32*)(result + 0x14) = *(const f32*)(input + 0x14);
    *(f32*)(result + 0x18) = *(const f32*)(input + 0x24);
    *(f32*)(result + 0x20) = *(const f32*)(input + 0x8);
    *(f32*)(result + 0x24) = *(const f32*)(input + 0x18);
    *(f32*)(result + 0x28) = *(const f32*)(input + 0x28);
    *(f32*)(result + 0x30) = -(*(const f32*)(input + 0x38) * *(const f32*)(input + 0x8) +
        (*(const f32*)(input + 0x30) * *(const f32*)(input + 0x0) + *(const f32*)(input + 0x34) * *(const f32*)(input + 0x4)));
    *(f32*)(result + 0x34) = -(*(const f32*)(input + 0x38) * *(const f32*)(input + 0x18) +
        (*(const f32*)(input + 0x30) * *(const f32*)(input + 0x10) + *(const f32*)(input + 0x34) * *(const f32*)(input + 0x14)));
    *(f32*)(result + 0x38) = -(*(const f32*)(input + 0x38) * *(const f32*)(input + 0x28) +
        (*(const f32*)(input + 0x30) * *(const f32*)(input + 0x20) + *(const f32*)(input + 0x34) * *(const f32*)(input + 0x24)));
    *(u32*)(result + 0xC) = (*(u32*)(result + 0xC) | 3) & ~0x20000U;
    return result;
  }
  {
    f32 determinant;
    f32 inverse;
    *(f32*)(result + 0x0) = *(const f32*)(input + 0x14) * *(const f32*)(input + 0x28) - *(const f32*)(input + 0x18) * *(const f32*)(input + 0x24);
    *(f32*)(result + 0x4) = -(*(const f32*)(input + 0x4) * *(const f32*)(input + 0x28) - *(const f32*)(input + 0x8) * *(const f32*)(input + 0x24));
    *(f32*)(result + 0x8) = *(const f32*)(input + 0x4) * *(const f32*)(input + 0x18) - *(const f32*)(input + 0x8) * *(const f32*)(input + 0x14);
    determinant = *(f32*)(result + 0x8) * *(const f32*)(input + 0x20) +
        (*(f32*)(result + 0x0) * *(const f32*)(input + 0x0) + *(f32*)(result + 0x4) * *(const f32*)(input + 0x10));
    if (determinant != 0.0f) {
      inverse = 1.0f / determinant;
    } else {
      inverse = 1.0f;
    }
    *(f32*)(result + 0x0) *= inverse;
    *(f32*)(result + 0x4) *= inverse;
    *(f32*)(result + 0x8) *= inverse;
    *(f32*)(result + 0x10) = inverse * -(*(const f32*)(input + 0x10) * *(const f32*)(input + 0x28) - *(const f32*)(input + 0x18) * *(const f32*)(input + 0x20));
    *(f32*)(result + 0x14) = inverse * (*(const f32*)(input + 0x0) * *(const f32*)(input + 0x28) - *(const f32*)(input + 0x8) * *(const f32*)(input + 0x20));
    *(f32*)(result + 0x18) = inverse * -(*(const f32*)(input + 0x0) * *(const f32*)(input + 0x18) - *(const f32*)(input + 0x8) * *(const f32*)(input + 0x10));
    *(f32*)(result + 0x20) = inverse * (*(const f32*)(input + 0x10) * *(const f32*)(input + 0x24) - *(const f32*)(input + 0x14) * *(const f32*)(input + 0x20));
    *(f32*)(result + 0x24) = inverse * -(*(const f32*)(input + 0x0) * *(const f32*)(input + 0x24) - *(const f32*)(input + 0x4) * *(const f32*)(input + 0x20));
    *(f32*)(result + 0x28) = inverse * (*(const f32*)(input + 0x0) * *(const f32*)(input + 0x14) - *(const f32*)(input + 0x4) * *(const f32*)(input + 0x10));
    *(f32*)(result + 0x30) = -(*(const f32*)(input + 0x38) * *(f32*)(result + 0x20) +
        (*(const f32*)(input + 0x30) * *(f32*)(result + 0x0) + *(const f32*)(input + 0x34) * *(f32*)(result + 0x10)));
    *(f32*)(result + 0x34) = -(*(const f32*)(input + 0x38) * *(f32*)(result + 0x24) +
        (*(const f32*)(input + 0x30) * *(f32*)(result + 0x4) + *(const f32*)(input + 0x34) * *(f32*)(result + 0x14)));
    *(f32*)(result + 0x38) = -(*(const f32*)(input + 0x38) * *(f32*)(result + 0x28) +
        (*(const f32*)(input + 0x30) * *(f32*)(result + 0x8) + *(const f32*)(input + 0x34) * *(f32*)(result + 0x18)));
  }
  *(u32*)(result + 0xC) &= ~0x20000U;
  return result;
}

#include <renderware/project_error.h>

u8* RwMatrixScale(u8* unk00, const u8* unk04, s32 unk08) {
  switch (unk08) {
  case 0:
    *(f32*)unk00 =
        *(f32*)(unk00 + 0x14) =
        *(f32*)(unk00 + 0x28) = 1.0f;
    *(f32*)(unk00 + 4) =
        *(f32*)(unk00 + 8) =
        *(f32*)(unk00 + 0x10) = 0.0f;
    *(f32*)(unk00 + 0x18) =
        *(f32*)(unk00 + 0x20) =
        *(f32*)(unk00 + 0x24) = 0.0f;
    *(f32*)(unk00 + 0x30) =
        *(f32*)(unk00 + 0x34) =
        *(f32*)(unk00 + 0x38) = 0.0f;
    *(u32*)(unk00 + 0xC) |= 0x20000 | 3;
    *(f32*)unk00 = *(const f32*)unk04;
    *(f32*)(unk00 + 0x14) = *(const f32*)(unk04 + 4);
    *(f32*)(unk00 + 0x28) = *(const f32*)(unk04 + 8);
    break;
  case 1:
    *(f32*)unk00 *= *(const f32*)unk04;
    *(f32*)(unk00 + 0x4) *= *(const f32*)unk04;
    *(f32*)(unk00 + 0x8) *= *(const f32*)unk04;
    *(f32*)(unk00 + 0x10) *= *(const f32*)(unk04 + 4);
    *(f32*)(unk00 + 0x14) *= *(const f32*)(unk04 + 4);
    *(f32*)(unk00 + 0x18) *= *(const f32*)(unk04 + 4);
    *(f32*)(unk00 + 0x20) *= *(const f32*)(unk04 + 8);
    *(f32*)(unk00 + 0x24) *= *(const f32*)(unk04 + 8);
    *(f32*)(unk00 + 0x28) *= *(const f32*)(unk04 + 8);
    break;
  case 2:
    *(f32*)unk00 *= *(const f32*)unk04;
    *(f32*)(unk00 + 0x4) *= *(const f32*)(unk04 + 4);
    *(f32*)(unk00 + 0x8) *= *(const f32*)(unk04 + 8);
    *(f32*)(unk00 + 0x10) *= *(const f32*)unk04;
    *(f32*)(unk00 + 0x14) *= *(const f32*)(unk04 + 4);
    *(f32*)(unk00 + 0x18) *= *(const f32*)(unk04 + 8);
    *(f32*)(unk00 + 0x20) *= *(const f32*)unk04;
    *(f32*)(unk00 + 0x24) *= *(const f32*)(unk04 + 4);
    *(f32*)(unk00 + 0x28) *= *(const f32*)(unk04 + 8);
    *(f32*)(unk00 + 0x30) *= *(const f32*)unk04;
    *(f32*)(unk00 + 0x34) *= *(const f32*)(unk04 + 4);
    *(f32*)(unk00 + 0x38) *= *(const f32*)(unk04 + 8);
    break;
  default: {
    BaerrState unk0C;
    unk0C.unk00 = 1;
    unk0C.unk04 = _rwerror((s32)0x80000003, "Invalid combination type");
    RwErrorSet(&unk0C);
    unk00 = 0;
    break;
  }
  }
  *(u32*)(unk00 + 0xC) &= ~0x20003U;
  return unk00;
}

#include <renderware/project_error.h>

u8* RwMatrixTranslate(u8* arg0, const f32* arg1, s32 arg2) {
  switch (arg2) {
  case 0:
    *(f32*)(arg0) = *(f32*)(arg0 + 0x14) = *(f32*)(arg0 + 0x28) = 1.0f;
    *(f32*)(arg0 + 4) = *(f32*)(arg0 + 8) = *(f32*)(arg0 + 0x10) = 0.0f;
    *(f32*)(arg0 + 0x18) = *(f32*)(arg0 + 0x20) = *(f32*)(arg0 + 0x24) = 0.0f;
    *(f32*)(arg0 + 0x30) = *(f32*)(arg0 + 0x34) = *(f32*)(arg0 + 0x38) = 0.0f;
    *(u32*)(arg0 + 0xC) |= 0x20003;
    *(f32*)(arg0 + 0x30) = arg1[0];
    *(f32*)(arg0 + 0x34) = arg1[1];
    *(f32*)(arg0 + 0x38) = arg1[2];
    break;
  case 1:
    *(f32*)(arg0 + 0x30) +=
        arg1[2] * *(f32*)(arg0 + 0x20) +
        (arg1[0] * *(f32*)arg0 + arg1[1] * *(f32*)(arg0 + 0x10));
    *(f32*)(arg0 + 0x34) +=
        arg1[2] * *(f32*)(arg0 + 0x24) +
        (arg1[0] * *(f32*)(arg0 + 4) + arg1[1] * *(f32*)(arg0 + 0x14));
    *(f32*)(arg0 + 0x38) +=
        arg1[2] * *(f32*)(arg0 + 0x28) +
        (arg1[0] * *(f32*)(arg0 + 8) + arg1[1] * *(f32*)(arg0 + 0x18));
    break;
  case 2:
    *(f32*)(arg0 + 0x30) += arg1[0];
    *(f32*)(arg0 + 0x34) += arg1[1];
    *(f32*)(arg0 + 0x38) += arg1[2];
    break;
  default: {
    BaerrState unk00;
    unk00.unk00 = 1;
    unk00.unk04 = _rwerror((s32)0x80000003, "Invalid combination type");
    RwErrorSet(&unk00);
    arg0 = 0;
    break;
  }
  }
  *(u32*)(arg0 + 0xC) &= ~0x20000U;
  return arg0;
}


#include <renderware/project_error.h>

u8* RwMatrixTransform(u8* unk00, const u8* unk04, s32 unk08) {
  ProjectMatrix temporary;
  temporary.unk00[3] = 1;
  switch (unk08) {
    case 0:
      *(ProjectMatrix*)unk00 = *(const ProjectMatrix*)unk04;
      *(u32*)(unk00 + 0xC) = *(const u32*)(unk04 + 0xC);
      break;
    case 1: {
      u32 copyBit;
      u32 outputFlags;
      u32 inputFlags;
      u8* module;
      inputFlags = *(const u32*)(unk04 + 0xC);
      outputFlags = *(u32*)(unk00 + 0xC);
      module = RwEngineInstance + matrixModule[0];
      copyBit = *(u32*)(module + 4) & 0x20000;
      if (inputFlags & copyBit) {
        temporary = *(ProjectMatrix*)unk00;
      } else if (outputFlags & copyBit) {
        temporary = *(const ProjectMatrix*)unk04;
      } else {
        u32 flags = inputFlags & outputFlags;
        (*(void (**)(u8*, const u8*, const u8*))(module + 8))(
            (u8*)&temporary, unk04, unk00);
        temporary.unk00[3] = flags;
      }
      temporary.unk00[3] = *(u32*)(unk00 + 0xC);
      *(ProjectMatrix*)unk00 = temporary;
      *(u32*)(unk00 + 0xC) &= ~0x20003U;
      break;
    }
    case 2:
      RwMatrixMultiply((u8*)&temporary, unk00, unk04);
      temporary.unk00[3] = *(u32*)(unk00 + 0xC);
      *(ProjectMatrix*)unk00 = temporary;
      *(u32*)(unk00 + 0xC) &= ~0x20003U;
      break;
    default: {
      BaerrState error;
      error.unk00 = 1;
      error.unk04 = _rwerror(0x80000003, "Invalid combination type");
      RwErrorSet(&error);
      unk00 = 0;
      break;
    }
  }
  return unk00;
}

s32 RwMatrixDestroy(u8* unk00) {
  (*(void* (**)(u8*, void*))(RwEngineInstance + 0x144))(
      *(u8**)(RwEngineInstance + matrixModule[0]), unk00);
  return 1;
}

u8* RwMatrixCreate(void) {
  u8* unk00 = (*(u8* (**)(u8*))(RwEngineInstance + 0x140))(
      *(u8**)(RwEngineInstance + matrixModule[0]));
  if (unk00 != 0) {
    *(u32*)(unk00 + 0xC) = 3;
    *(f32*)unk00 =
        *(f32*)(unk00 + 0x14) =
        *(f32*)(unk00 + 0x28) = 1.0f;
    *(f32*)(unk00 + 4) =
        *(f32*)(unk00 + 8) =
        *(f32*)(unk00 + 0x10) = 0.0f;
    *(f32*)(unk00 + 0x18) =
        *(f32*)(unk00 + 0x20) =
        *(f32*)(unk00 + 0x24) = 0.0f;
    *(f32*)(unk00 + 0x30) =
        *(f32*)(unk00 + 0x34) =
        *(f32*)(unk00 + 0x38) = 0.0f;
    *(u32*)(unk00 + 0xC) |= 0x20000 | 3;
  }
  return unk00;
}
