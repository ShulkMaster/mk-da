#include <msl/mslMem.h>
#include <movie/project_alloc.h>

u32 mslGetHeapSize(void) {
  mwMemHeapInfo info;

  mwMemHeapGetInfo(MSLMFL_HEAP, &info);
  return info.unk24;
}

/* Absent from the retail ELF (linker-stripped): its use of "mslMem.c" is
   what puts that string first in retail .rodata. */
void mslHeapFree(void* ptr) {
  _mwMemFree(ptr, "mslMem.c", 110);
}

void* mslAlignedAlloc(u32 size) {
  return _mwMemMalloc(MSLMFL_HEAP, size, 5, "MSL heap aligned alloc", "mslMem.c", 104);
}

void *mslHeapAlignedAlloc(struct _mwMemHeap *heap, u32 size, const char *name) {
  return _mwMemMalloc(heap, size, 5, name, "mslMem.c", 96);
}

void* mslHeapAlloc(struct _mwMemHeap* heap, u32 size, const char* name) {
  return _mwMemMalloc(heap, size, 3, name, "mslMem.c", 88);
}

void mlHeapFree(void* ptr) {
  _mwMemFree(ptr, "mslMem.c", 82);
}

void* mlHeapRealloc(struct _mwMemHeap* heap, void* ptr, u32 size) {
  return _mwMemRealloc(ptr, heap, size, 3, "mslRealloc", "mslMem.c", 75);
}

void* mlAlignedHeapAlloc(struct _mwMemHeap* heap, u32 size) {
  return _mwMemMalloc(heap, size, 5, "mslAlignedAlloc", "mslMem.c", 66);
}

void* mlHeapAlloc(struct _mwMemHeap* heap, u32 size) {
  return _mwMemMalloc(heap, size, 3, "mslAlloc", "mslMem.c", 58);
}
