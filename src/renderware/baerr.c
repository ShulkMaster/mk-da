#include <dolphin/types.h>
#include <stdarg.h>

#include "renderware/project_state.h"

#include "renderware/project_error.h"

static BaerrState errorModule;

void* _rwErrorOpen(void* arg0, s32 arg1) {
  errorModule.unk00 = arg1;
  errorModule.unk04++;
  ((BaerrState*)(RwEngineInstance + arg1))->unk00 = 0;
  ((BaerrState*)(RwEngineInstance + errorModule.unk00))->unk04 = 0x80000000;
  return arg0;
}

void* _rwErrorClose(void* unk0) {
  errorModule.unk04--;
  return unk0;
}

BaerrState* RwErrorSet(BaerrState* unk00) {
  BaerrState* unk04 = (BaerrState*)(RwEngineInstance + errorModule.unk00);
  if (unk04->unk00 == 0 && (u32)unk04->unk04 == 0x80000000) {
    if ((u32)unk00->unk04 & 0x80000000) {
      unk04->unk00 = 0;
    } else {
      unk04->unk00 = unk00->unk00;
    }
    ((BaerrState*)(RwEngineInstance + errorModule.unk00))->unk04 = unk00->unk04;
  }
  return unk00;
}

s32 _rwerror(s32 arg0, ...) {
  va_list args;
  va_start(args, arg0);
  va_end(args);
  return arg0;
}
