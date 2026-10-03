#ifndef MKDA_RENDERWARE_PROJECT_RESOURCES_H
#define MKDA_RENDERWARE_PROJECT_RESOURCES_H

#include <dolphin/types.h>

typedef struct ResmemBlock14 ResmemBlock14;
typedef struct {
  ResmemBlock14* unk0;
  ResmemBlock14* unk4;
} ResmemHeap8;

struct ResmemBlock14 {
  ResmemHeap8* unk0;
  ResmemBlock14* unk4;
  ResmemBlock14* unk8;
  u32 unkC;
  u32 unk10;
};

s32 _rwResHeapInit(ResmemHeap8* unk00, s32 unk04);
s32 _rwResHeapClose(ResmemHeap8* unk00);
void* _rwResHeapAlloc(ResmemHeap8* unk00, u32 unk04);
void _rwResHeapFree(void* unk00);

#endif
