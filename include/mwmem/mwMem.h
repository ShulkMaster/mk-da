#ifndef MKDA_MWMEM_MWMEM_H
#define MKDA_MWMEM_MWMEM_H

#include <dolphin/types.h>

struct _mwMemHeap;
typedef void* (*mwMemAllocationCallback)(u32 size, struct _mwMemHeap** heap,
                                          s32 alignment, u8 tag);

typedef struct mwMemSystemParams {
  u32 unk00;
  u32 unk04;
} mwMemSystemParams;

typedef struct mwMemHeapParams {
  mwMemAllocationCallback allocationCallback;
  u32 unk04;
  u32 unk08;
  u8 unk0C;
  u8 unk0D;
  u8 unk0E;
  u32 unk10;
  u32 unk14;
} mwMemHeapParams;

/* Filled by mwMemHeapGetInfo: retail stores 0x34 bytes. */
typedef struct mwMemHeapInfo {
  const char* name;
  u8* start;
  u32 unk08;
  u32 unk0C;
  u32 unk10;
  u32 unk14;
  u8 unk18;
  u32 unk1C;
  u32 unk20;
  u32 unk24;
  u32 unk28;
  u32 unk2C;
  u32 unk30;
} mwMemHeapInfo;

typedef struct mwMemFixedHeapConfig {
  u32 unk00;
  u32 count;
  u32 blockSize;
  u32 unk0C;
  s32 alignment;
} mwMemFixedHeapConfig;

typedef struct mwMemHeapConfig {
  struct _mwMemHeap* parent;
  u32 size;
  s32 unk08;
  s32 type;
  mwMemFixedHeapConfig* fixed;
  const char* name;
  u32 unk18;
} mwMemHeapConfig;

#ifdef __cplusplus
extern "C" {
#endif

extern u32 mwMEM_VIRTUAL_HEAP_SIZE;

struct _mwMemHeap* _mwMemHeapCreate(const mwMemHeapConfig* config,
                                   const mwMemHeapParams* params,
                                   const char* file, s32 line);
void MEMPRINT(const char* format, ...);
u32 mwMemSystemGetAvailSize(void);
void* mwMemHeapStrategyCallback(u32 size, struct _mwMemHeap* heap, s32 alignment, u32 flags);
int mwMemHeapGetDefaultParams(mwMemHeapParams* params);
int mwMemHeapSetParams(struct _mwMemHeap* heap, const mwMemHeapParams* params);
int mwMemSystemGetDefaultParams(mwMemSystemParams* params);
int mwMemSystemSetParams(const mwMemSystemParams* params);
int mwMemSystemCreate(u32 size, const mwMemSystemParams* params);
struct _memUsedHdr {
  struct _memUsedHdr* previous;
  struct _memUsedHdr* next;
  u32 size;
  u8 unusedBytes;
  u8 allocationFlags;
  u8 flags;
  u8 alignmentOffset;
};

struct _memFreeHdr {
  struct _memFreeHdr* previous;
  struct _memFreeHdr* next;
  u32 size;
  u8 unk0C[2];
  u8 flags;
  u8 unk0F;
};

struct _mwMemHeap {
  struct _mwMemHeap* next;
  struct _mwMemHeap* previous;
  struct _memUsedHdr* usedBlocks;
  struct _memFreeHdr* freeBlocks;
  struct _memFreeHdr* freeTail;
  s32 type;
  mwMemAllocationCallback allocationCallback;
  u32 marker;
  struct _mwMemHeap* parent;
  struct _mwMemHeap* children;
  struct _mwMemHeap* sibling;
  u8 unk2C;
  u8 unk2D;
  u8 unk2E;
  u8 unk2F;
  const char* name;
  u32 unk34;
  u8* start;
  u8* end;
  u32 unk40;
  u32 unk44;
  u32 unk48;
  u32 unk4C;
  u32 unk50;
  u32 unk54;
  u32 unk58;
  u32 unk5C;
  u32 unk60;
  u32 unk64;
  u32 unk68;
  u8 unk6C;
  u8 unk6D;
};

void* _mwMemMalloc(struct _mwMemHeap* heap, u32 size, s32 alignment,
                    const char* label, const char* file, s32 line);
void _mwMemFree(void* ptr, const char* file, s32 line);
void* _mwMemRealloc(void* ptr, struct _mwMemHeap* heap, u32 size,
                     s32 alignment, const char* label, const char* file,
                     s32 line);

void* _mwMemMallocVirtual(struct _mwMemHeap* heap, u32 size, s32 alignment,
                          const char* label, const char* file, s32 line, u32 flags);
void* _mwMemCalloc(struct _mwMemHeap* heap, u32 count, u32 elementSize,
                   s32 alignment, const char* label, const char* file, s32 line);

int mwMemHeapGetInfo(struct _mwMemHeap* heap, mwMemHeapInfo* info);

void mwMemHeapGetMaxFreeBlock(struct _mwMemHeap* heap, u32* maxSize, u32* blockCount);
int mwMemIsHeapValid(struct _mwMemHeap* heap);
struct _mwMemHeap* mwMemSystemGetHeap(s32 heapKind);
int mwMemSystemSetHeap(s32 heapKind, struct _mwMemHeap* heap);
int mwMemHeapWipe(struct _mwMemHeap* heap);

#ifdef __cplusplus
}
#endif

#endif
