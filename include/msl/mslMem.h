#ifndef MSL_MSLMEM_H
#define MSL_MSLMEM_H

#include <dolphin/types.h>
#include <mwmem/mwMem.h>

struct _mwMemHeap;

extern struct _mwMemHeap *MSLMFL_HEAP;
extern struct _mwMemHeap* wave_heap;

u32 mslGetHeapSize(void);
void *mslHeapAlignedAlloc(struct _mwMemHeap *heap, u32 size, const char *name);
void* mslHeapAlloc(struct _mwMemHeap* heap, u32 size, const char* name);
void mlHeapFree(void* ptr);
void* mlHeapRealloc(struct _mwMemHeap* heap, void* ptr, u32 size);
void* mlAlignedHeapAlloc(struct _mwMemHeap* heap, u32 size);
void* mlHeapAlloc(struct _mwMemHeap* heap, u32 size);

void mslHeapFree(void* ptr);
#endif
