#include <dolphin/types.h>

static struct {
  s32 unk00;
  s32 unk04;
} colorModule;

void* _rwColorOpen(void* unk00, s32 unk04, s32 unk08) {
  colorModule.unk04++;
  return unk00;
}

void* _rwColorClose(void* unk00, s32 unk04, s32 unk08) {
  colorModule.unk04--;
  return unk00;
}
