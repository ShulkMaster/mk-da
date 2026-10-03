#ifndef MSL_MSLMEM_H
#define MSL_MSLMEM_H

#include <dolphin/types.h>

struct _mwMemHeap;

/* Filled by mwMemHeapGetInfo: retail stores 0x34 bytes. */
typedef struct mwMemHeapInfo {
  u32 unk00;
  u32 unk04;
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

extern struct _mwMemHeap *MSLMFL_HEAP;

int mwMemHeapGetInfo(struct _mwMemHeap *heap, mwMemHeapInfo *info);

u32 mslGetHeapSize(void);
void *mslHeapAlignedAlloc(struct _mwMemHeap *heap, u32 size, const char *name);
void* mslHeapAlloc(struct _mwMemHeap* heap, u32 size, const char* name);
void mlHeapFree(void* ptr);
void* mlHeapRealloc(struct _mwMemHeap* heap, void* ptr, u32 size);
void* mlAlignedHeapAlloc(struct _mwMemHeap* heap, u32 size);
void* mlHeapAlloc(struct _mwMemHeap* heap, u32 size);

void mslHeapFree(void* ptr);
#endif
