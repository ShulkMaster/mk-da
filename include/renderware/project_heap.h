#ifndef MKDA_RENDERWARE_PROJECT_HEAP_H
#define MKDA_RENDERWARE_PROJECT_HEAP_H

#include <dolphin/types.h>

typedef struct ProjectHeapBlock ProjectHeapBlock;
typedef struct ProjectHeapEntry ProjectHeapEntry;
typedef struct ProjectHeapRange ProjectHeapRange;

struct ProjectHeapBlock {
  ProjectHeapBlock* unk00;
  ProjectHeapBlock* unk04;
  u32 unk08;
  ProjectHeapEntry* unk0C;
  u32 unk10[4];
};

struct ProjectHeapEntry {
  u32 unk00;
  ProjectHeapBlock* unk04;
};

struct ProjectHeapRange {
  u8* unk00;
  u32 unk04;
  ProjectHeapRange* unk08;
};

typedef struct {
  u32 unk00;
  ProjectHeapRange* unk04;
  ProjectHeapBlock* unk08;
  ProjectHeapEntry* unk0C;
  u32 unk10;
  u32 unk14;
  s32 unk18;
} ProjectHeapState;

ProjectHeapState* RxHeapCreate(u32 arg0);
void RxHeapDestroy(ProjectHeapState* arg0);
s32 _rxHeapReset(ProjectHeapState* unk00);
void* RxHeapAlloc(ProjectHeapState* unk00, u32 unk04);
void RxHeapFree(ProjectHeapState* unk00, void* unk04);
void* RxHeapRealloc(ProjectHeapState* arg0, void* arg1, u32 arg2, s32 arg3);

#endif
