/* #audit 2026-10-03T04:06Z clean-room PASS (audit) */
#include <renderware/project_state.h>

u32* RwImageSetFromRaster(u32* unk00, u8* unk04) {
  if ((*(s32 (**)(u32*, u8*, s32))(RwEngineInstance + 0x60))(unk00, unk04, 0)) {
    if (unk04[0x22] & 1) {
      *unk00 |= 2;
    }
    return unk00;
  }
  return 0;
}

u8* RwRasterSetFromImage(u8* unk00, u32* unk04) {
  if ((*(s32 (**)(u8*, u32*, s32))(RwEngineInstance + 0x64))(unk00, unk04, 0)) {
    if (*unk04 & 2) {
      unk00[0x22] |= 1;
    }
    return unk00;
  }
  return 0;
}

void* RwImageFindRasterFormat(void* unk00, s32 unk04, s32* unk08, s32* unk0C, s32* unk10, s32* unk14) {
  u8 unk18[0x34];
  if (!(*(s32 (**)(void*, void*, s32))(RwEngineInstance + 0x6C))(unk18, unk00, unk04)) {
    return 0;
  }
  *unk14 = (unk18[0x23] << 8) | unk18[0x20];
  *unk08 = *(s32*)(unk18 + 0x0C);
  *unk0C = *(s32*)(unk18 + 0x10);
  *unk10 = *(s32*)(unk18 + 0x14);
  return unk00;
}
