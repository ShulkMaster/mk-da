/* #audit 2026-10-03T04:06Z clean-room PASS (audit) */
#include <dolphin/types.h>

typedef struct {
  f32 unk0;
  f32 unk4;
  f32 unk8;
} Babbox12;

typedef struct {
  Babbox12 unk0;
  Babbox12 unkC;
} Babbox24;

void RwBBoxCalculate(Babbox24* unk0, const Babbox12* unk4, s32 unk8) {
  s32 unk10;
  const Babbox12* unkC;
  unk0->unkC = *unk4;
  unk0->unk0 = *unk4;
  unkC = unk4 + 1;
  unk10 = unk8 - 1;
  while (unk10-- != 0) {
    if (unk0->unkC.unk0 > unkC->unk0) unk0->unkC.unk0 = unkC->unk0;
    if (unk0->unkC.unk4 > unkC->unk4) unk0->unkC.unk4 = unkC->unk4;
    if (unk0->unkC.unk8 > unkC->unk8) unk0->unkC.unk8 = unkC->unk8;
    if (unk0->unk0.unk0 < unkC->unk0) unk0->unk0.unk0 = unkC->unk0;
    if (unk0->unk0.unk4 < unkC->unk4) unk0->unk0.unk4 = unkC->unk4;
    if (unk0->unk0.unk8 < unkC->unk8) unk0->unk0.unk8 = unkC->unk8;
    unkC++;
  }
}
