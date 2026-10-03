/* #audit 2026-10-03T04:06Z clean-room FIXED (audit) */
#include <dolphin/types.h>

#include "renderware/project_state.h"
#include "renderware/project_error.h"

f32 RwIm2DGetNearScreenZ(void) {
  return *(f32*)(RwEngineInstance + 0x18);
}

f32 RwIm2DGetFarScreenZ(void) {
  return *(f32*)(RwEngineInstance + 0x1c);
}

s32 RwRenderStateSet(s32 arg0, u32 arg1) {
  BaerrState unk08;
  if (*(u32*)RwEngineInstance == 0U) {
    unk08.unk00 = 1;
    unk08.unk04 = _rwerror(0x10);
    RwErrorSet(&unk08);
    return 0;
  }
  return (*(s32 (**)(s32, u32))(RwEngineInstance + 0x20))(arg0, arg1);
}

s32 RwRenderStateGet(s32 unk00, void* unk04) {
  return (*(s32 (**)(s32, void*))(RwEngineInstance + 0x24))(unk00, unk04);
}

s32 RwIm2DRenderPrimitive(s32 arg0, void* arg1, s32 arg2) {
  return (*(s32 (**)(s32, void*, s32))(RwEngineInstance + 0x30))(arg0, arg1, arg2);
}

s32 RwIm2DRenderIndexedPrimitive(s32 unk00, void* unk04, s32 unk08, void* unk0c, s32 unk10) {
  return (*(s32 (**)(s32, void*, s32, void*, s32))(RwEngineInstance + 0x34))(
      unk00, unk04, unk08, unk0c, unk10);
}
