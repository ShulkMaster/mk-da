#include "movie/project_movie.h"

extern "C" void _mwMemFree(void*, const char*, s32);

extern "C" void* _mwMemMalloc(void*, u32, u32, const char*, const char*, s32);

void operator delete(void* ptr, _mwMemHeap*, mwMemFlags, const char*) {
  if (ptr != 0) {
    _mwMemFree(ptr, "mwMemNewDelete.cpp", 0x173);
  }
}

void operator delete(void* ptr) throw() {
  if (ptr != 0) {
    _mwMemFree(ptr, "mwMemNewDelete.cpp", 342);
  }
}

void* operator new(unsigned long size, _mwMemHeap* heap, mwMemFlags flags, const char* name) {
  return _mwMemMalloc(heap, size, flags, name, "mwMemNewDelete.cpp", 0x72);
}
