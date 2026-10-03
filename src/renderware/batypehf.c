/* #audit 2026-10-03T04:35Z clean-room PASS (audit) */
#include <dolphin/types.h>

extern u8* RwFrameUpdateObjects(u8*);

void _rwObjectHasFrameSetFrame(u8* unk00, u8* unk04) {
  if (*(u8**)(unk00 + 4) != 0) {
    **(u8***)(unk00 + 0xC) = *(u8**)(unk00 + 8);
    *(u8**)(*(u8**)(unk00 + 8) + 4) = *(u8**)(unk00 + 0xC);
  }
  *(u8**)(unk00 + 4) = unk04;
  if (unk04 != 0) {
    *(u8**)(unk00 + 8) = *(u8**)(unk04 + 0x90);
    *(u8**)(unk00 + 0xC) = unk04 + 0x90;
    *(u8**)(*(u8**)(unk04 + 0x90) + 4) = unk00 + 8;
    *(u8**)(unk04 + 0x90) = unk00 + 8;
    RwFrameUpdateObjects(unk04);
  }
}

void _rwObjectHasFrameReleaseFrame(u8* arg0) {
  if (*(u8**)(arg0 + 4) != 0) {
    **(u8***)(arg0 + 0xC) = *(u8**)(arg0 + 8);
    *(u8**)(*(u8**)(arg0 + 8) + 4) = *(u8**)(arg0 + 0xC);
  }
}
