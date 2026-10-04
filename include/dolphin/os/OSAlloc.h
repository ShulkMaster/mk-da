#ifndef _DOLPHIN_OSALLOC
#define _DOLPHIN_OSALLOC

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int OSHeapHandle;

extern volatile OSHeapHandle __OSCurrHeap;

void* OSInitAlloc(void* arenaLo, void* arenaHi, int maxHeaps);
OSHeapHandle OSCreateHeap(void* start, void* end);
OSHeapHandle OSSetCurrentHeap(OSHeapHandle heap);
s32 OSCheckHeap(OSHeapHandle heap);
void* OSAllocFromHeap(OSHeapHandle heap, u32 size);
void OSFreeToHeap(OSHeapHandle heap, void* ptr);

#define OSAlloc(size) OSAllocFromHeap(__OSCurrHeap, (size))
#define OSFree(ptr) OSFreeToHeap(__OSCurrHeap, (ptr))

#ifdef __cplusplus
}
#endif

#endif // _DOLPHIN_OSALLOC
