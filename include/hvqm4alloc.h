#ifndef HVQM4ALLOC_H
#define HVQM4ALLOC_H

static void* DoMalloc(u32 size)
{
  void* data = _mwMemMalloc(gHeap, size, 5, "DoMalloc movie", "hvqm4play.c", 0x110);
  if (data == NULL) {
    OSPanic("hvqm4play.c", 0x112, "assertion \"data != NULL\" failed");
  }
  return data;
}

#endif
