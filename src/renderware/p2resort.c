/* #audit 2026-10-03T08:33Z clean-room PASS (audit) */
#include <dolphin/types.h>
#include <renderware/project_sort.h>

static void _repartition(u8* unk00, u8* unk04, u32 unk08, u32 unk0C, u32 unk10) {
  u32* unk2C;
  struct {
    u8* unk00;
    u8* unk04;
    u32 unk08;
  } unk14[32], *unk18;
  unk14[0].unk00 = unk00;
  unk14[0].unk04 = unk04;
  unk14[0].unk08 = unk10;
  unk18 = unk14 + 1;
  do {
    unk00 = unk18[-1].unk00;
    unk04 = unk18[-1].unk04;
    unk10 = unk18[-1].unk08;
    --unk18;
    for (;;) {
      u8* unk24 = unk00;
      u8* unk28 = unk04;
      do {
        while (!(*(u32*)(unk00 + unk0C) & unk10)) {
          unk00 += unk08;
          if (unk00 > unk04) {
            goto unk48;
          }
        }
        while (*(u32*)(unk04 + unk0C) & unk10) {
          unk04 -= unk08;
          if (unk00 > unk04) {
            goto unk48;
          }
        }
        {
          u32* unk30;
          u32 unk34;
          unk2C = (u32*)unk00;
          unk30 = (u32*)unk04;
          unk34 = unk08;
          while (unk34 >= 4) {
            u32 unk38 = *unk2C;
            u32 unk3C = *unk30;
            *unk30 = unk38;
            *unk2C = unk3C;
            ++unk2C;
            ++unk30;
            unk34 -= 4;
          }
        }
        unk00 += unk08;
        unk04 -= unk08;
      } while (unk00 <= unk04);
unk48:
      unk10 >>= 1;
      if (unk10 == 0) {
        break;
      }
      {
        u32 unk44;
        u8* unk40;
        unk40 = unk04 + unk08;
        unk44 = unk08 * 5;
        if (unk28 >= unk40 + unk44) {
          unk18->unk00 = unk40;
          unk18->unk04 = unk28;
          unk18->unk08 = unk10;
          ++unk18;
        }
        unk04 = unk00 - unk08;
        unk00 = unk24;
        if (unk04 < unk24 + unk44) {
          break;
        }
      }
    }
  } while (unk18 != unk14);
}

void _rx_rxRadixExchangeSort(u8* unk00, u32 unk04, u32 unk08, u32 unk0C, u32 unk10, u32 unk14) {
  if (unk00 != 0 && unk0C + 4 <= unk08 && unk10 < unk14) {
    if (unk04 > 5) {
      s32 unk18;
      if (unk14 != 0) {
        unk18 = 0;
        while ((unk14 >>= 1) != 0) {
          ++unk18;
        }
      } else {
        unk18 = -1;
      }
      _repartition(unk00, unk00 + (unk04 - 1) * unk08, unk08, unk0C, 1U << unk18);
    }
    if (unk04 > 1) {
      u32 unk1C = 4;
      u32 unk20;
      u32 unk24;
      if (unk04 - 1 < 4) {
        unk1C = unk04 - 1;
      }
      unk20 = *(u32*)(unk00 + unk1C * unk08 + unk0C);
      unk24 = unk1C;
      --unk1C;
      do {
        u32 unk28 = *(u32*)(unk00 + unk1C * unk08 + unk0C);
        if (unk28 < unk20) {
          unk20 = unk28;
          unk24 = unk1C;
        }
      } while (unk1C-- != 0);
      if (unk24 != 0) {
        u32* unk2C = (u32*)unk00;
        u32* unk30 = (u32*)(unk00 + unk24 * unk08);
        u32 unk34 = unk08;
        while (unk34 >= 4) {
          u32 unk38 = *unk2C;
          u32 unk3C = *unk30;
          *unk30 = unk38;
          *unk2C = unk3C;
          ++unk2C;
          ++unk30;
          unk34 -= 4;
        }
      }
      {
        u32 unk58;
        u32* unk54;
        u32* unk50;
        u8* unk4C;
        u32 unk48;
        u8* unk44;
        u32 unk40;
        u32 unk60;
        u32 unk5C;
        unk40 = unk04;
        unk44 = unk00;
        while ((unk44 += unk08, --unk40) != 0) {
          unk48 = *(u32*)(unk44 + unk0C);
          unk4C = unk44;
          while (*(u32*)((unk4C -= unk08) + unk0C) > unk48) {
            unk50 = (u32*)unk4C;
            unk54 = (u32*)(unk4C + unk08);
            unk58 = unk08;
            while (unk58 >= 4) {
              unk5C = *unk50++;
              unk60 = *unk54;
              *unk54 = unk5C;
              unk50[-1] = unk60;
              ++unk54;
              unk58 -= 4;
            }
          }
        }
      }
    }
  }
}
