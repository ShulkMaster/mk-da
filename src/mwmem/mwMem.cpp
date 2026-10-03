#include <mwmem/mwMem.h>
#include <string.h>
#include <dolphin/os.h>
#include <dolphin/os/OSAlloc.h>

struct _memFreeHdr;
struct _memUsedHdr;
struct _mwMemHeap;

_mwMemHeap* HeapList;
_mwMemHeap* SystemHeap;
static _mwMemHeap* newWrapperDefaultHeap;
static mwMemSystemParams systemParams;
static _mwMemHeap* mwMemSystemOverflowHeap;
static OSHeapHandle GameCubeSystemHeap;
static u8 heapIndex;
static void* OsSystemHeap;
static void* systemDebugCallbackFunc[3];
static void privResetHeap(_mwMemHeap*, int);
enum mwMemEnumAlign {
  mwMemEnumAlign_unk0 = 0,
  mwMemEnumAlign_unk4 = 4,
  mwMemEnumAlign_unk5 = 5,
  mwMemEnumAlign_unk6 = 6,
  mwMemEnumAlign_unk7 = 7,
  mwMemEnumAlign_unk8 = 8,
  MW_MEM_ALIGN_FORCE_32BIT = 0x7FFFFFFF
};
enum mwMemHeapType { mwMemHeapType_unk0 };

static void privReturnUsedBlockToFreeList(_mwMemHeap* heap, _memUsedHdr* used);
static _memFreeHdr* privCoalesceFreeBlocksBoundaryTags(_mwMemHeap* heap,
                                                      _memFreeHdr* block);
static inline _memFreeHdr* mwMemFindFirstFit(_mwMemHeap* heap, u32 size);
static inline _memFreeHdr* mwMemFindBestFit(_mwMemHeap* heap, u32 size);
static inline void mwMemInitUsedHeader(_memUsedHdr* block, u32 capacity,
                                      u32 requested, mwMemEnumAlign alignment,
                                      u8 flags);
static void* privMallocMem(unsigned int size, _mwMemHeap* heap,
                           mwMemEnumAlign alignment, unsigned char flags);
static inline void* mwMemTakeFixedBlock(_mwMemHeap* heap);
static void privDeleteBlockFixedBlock(_mwMemHeap* heap, _memUsedHdr* block);
static void privAllocHeap(_mwMemHeap* heap, _mwMemHeap* parent, unsigned int size,
                          const char* name, mwMemHeapType type,
                          unsigned int value60, unsigned int blockSize);
static inline u32 mwMemUsedPayloadSize(_memUsedHdr* block, s32* extra);
static inline u32 mwMemUsedBlockFootprint(_memUsedHdr* block);
static inline u32 mwMemFixedBlockCount(_mwMemHeap* heap);
static void privWipeHeap(_mwMemHeap *);
static inline int mwMemCanAllocateSystemHeap(u32 size);
static int privInitSystemHeap(unsigned int size);
static void privWipeHeapHierarchy(_mwMemHeap *);
static inline mwMemEnumAlign mwMemAlignmentBits(s32 alignment);

/* TODO: [borked] 0.00%; placeholder stub, body not started. */
extern "C" void mwMemSystemCreate(void) {}

/* TODO: [borked] 0.00%; placeholder stub, body not started. */
extern "C" void mwMemHeapWipe(void) {}

/* TODO: [borked] 0.00%; placeholder stub, body not started. */
static void privWipeHeapHierarchy(_mwMemHeap *) {}

extern "C" int mwMemSystemSetHeap(s32 heapKind, _mwMemHeap* heap) {
  if (mwMemIsHeapValid(heap) == 1) {
    switch (heapKind) {
    case 0:
      return 0;
    case 1:
      mwMemSystemOverflowHeap = heap;
      return 1;
    case 2:
      newWrapperDefaultHeap = heap;
      return 1;
    case 3:
    default:
      return 0;
    }
  }
  return 0;
}

extern "C" _mwMemHeap* mwMemSystemGetHeap(s32 heapKind) {
  switch (heapKind) {
  case 0:
    return SystemHeap;
  case 1:
    return mwMemSystemOverflowHeap;
  case 2:
    return newWrapperDefaultHeap;
  case 3:
  default:
    return NULL;
  }
}

extern "C" int mwMemHeapSetParams(_mwMemHeap* heap, const mwMemHeapParams* params) {
  if (mwMemIsHeapValid(heap) == 1) {
    if (params != NULL) {
      heap->allocationCallback = params->allocationCallback;
      heap->unk64 = params->unk04;
      heap->unk68 = params->unk08;
      heap->unk2E = params->unk0C;
      heap->unk2F = params->unk0D;
      heap->unk6C = params->unk0E;
      heap->unk40 = params->unk10;
      heap->unk44 = params->unk14;
      return 1;
    } else {
      mwMemHeapParams defaults;
      mwMemHeapGetDefaultParams(&defaults);
      mwMemHeapSetParams(heap, &defaults);
    }
    return 1;
  }
  return 0;
}

extern "C" int mwMemHeapGetDefaultParams(mwMemHeapParams* params) {
  int result;
  if (params != NULL) {
    params->allocationCallback = NULL;
    params->unk04 = 0;
    params->unk08 = 0;
    params->unk0C = 0xAB;
    params->unk0D = 0xDC;
    params->unk0E = 1;
    params->unk10 = 0;
    params->unk14 = 0;
    result = 1;
  } else {
    result = 0;
  }
  return result;
}

extern "C" int mwMemSystemSetParams(const mwMemSystemParams* params) {
  if (params != NULL) {
    systemParams.unk00 = params->unk00;
    systemParams.unk04 = params->unk04;
  } else {
    mwMemSystemParams defaults;
    mwMemSystemGetDefaultParams(&defaults);
    mwMemSystemSetParams(&defaults);
  }
  return 1;
}

extern "C" int mwMemSystemGetDefaultParams(mwMemSystemParams* params) {
  int result;
  if (params != NULL) {
    params->unk00 = 0;
    params->unk04 = 0;
    result = 1;
  } else {
    result = 0;
  }
  return result;
}

extern "C" int mwMemHeapGetInfo(_mwMemHeap* heap, mwMemHeapInfo* info) {
  int result;
  if (mwMemIsHeapValid(heap) == 1 && info != NULL) {
    info->unk00 = heap->unk30;
    info->unk04 = heap->unk38;
    info->unk08 = (u32)heap->end;
    info->unk0C = heap->unk34;
    info->unk10 = heap->type;
    info->unk14 = heap->unk2D;
    info->unk18 = heap->unk2C;
    info->unk1C = heap->unk48;
    info->unk20 = heap->unk4C;
    info->unk24 = heap->unk50;
    info->unk28 = heap->unk54;
    info->unk2C = heap->unk58;
    info->unk30 = heap->unk5C;
    result = 1;
  } else {
    result = 0;
  }
  return result;
}

/* TODO: [borked] 0.47%; typed placeholder, body not started. */
extern "C" _mwMemHeap* _mwMemHeapCreate(const mwMemHeapConfig* config,
                                      const mwMemHeapParams* params,
                                      const char* file, s32 line) {}

extern "C" void mwMemHeapGetMaxFreeBlock(_mwMemHeap* heap, u32* maxSize, u32* blockCount) {
  _memFreeHdr* block = heap->freeBlocks;
  u32 largest = 0;
  u32 count = 0;
  while (block != NULL) {
    if (block->size > largest) {
      largest = block->size;
    }
    block = block->next;
    count++;
  }
  *blockCount = count;
  *maxSize = largest;
}

extern "C" int mwMemIsHeapValid(_mwMemHeap* heap) {
  _mwMemHeap* entry = HeapList;
  int result = 0;
  if (heap != NULL) {
    while (entry != NULL) {
      if (entry == heap) {
        result = 1;
        break;
      }
      entry = entry->next;
    }
  }
  return result;
}

/* TODO: [borked] 0.00%; placeholder stub, body not started. */
extern "C" void _mwMemMalloc(void) {}

/* TODO: [near miss] 98.86%; footprint arithmetic and incomplete string-pool offsets remain. */
extern "C" void* _mwMemMallocVirtual(_mwMemHeap* heap, u32 size, s32 alignment,
                                    const char* label, const char* file,
                                    s32 line, u32 flags) {
  void* result;
  u32 rounded;
  mwMemEnumAlign mapped = mwMemAlignmentBits(alignment);
  int valid = 1;
  if (mapped < 4) {
    valid = 0;
  }
  if (mapped > 8) {
    valid = 0;
  }
  if (mapped >= 0xFF) {
    valid = 0;
  }
  if (mapped == 0) {
    valid = 1;
  }
  if (valid == 0) {
    mapped = mwMemEnumAlign_unk4;
  }
  if (mwMemIsHeapValid(heap) == 0) {
    MEMPRINT("Out of RAM\n");
    return NULL;
  }
  if (mapped == mwMemEnumAlign_unk4) {
    u32 mask = (1U << mapped) - 1;
    rounded = (size + mask) & ~mask;
  } else if (mapped == mwMemEnumAlign_unk0) {
    rounded = size;
  } else {
    rounded = (size + (1U << mapped) + 15) & ~15U;
  }
  if (heap->allocationCallback != NULL) {
    result = heap->allocationCallback(rounded, &heap, alignment, flags);
    if (heap->type == 2) {
      rounded = heap->unk64;
    }
  } else if (heap->type == 2) {
    if (rounded <= heap->unk64) {
      result = mwMemTakeFixedBlock(heap);
      rounded = heap->unk64;
    } else {
      result = NULL;
    }
  } else {
    result = privMallocMem(rounded, heap, mapped, flags);
  }
  if (result == NULL && heap->unk6C == 1) {
    MEMPRINT(">> OVERFLOW_HEAP: size: %f K heap: %s, file: %s L: %d\n",
             (float)size / 1024.0f, (const char*)heap->unk30, file, line);
    heap->unk2D = 1;
    if (mwMemIsHeapValid(mwMemSystemOverflowHeap) == 1) {
      heap = mwMemSystemOverflowHeap;
      result = privMallocMem(rounded, heap, mapped, 0);
    }
  }
  if (result == NULL) {
    MEMPRINT(">> Out of RAM \n");
    MEMPRINT("      FAILURE:  cannot allocate: %f K  from heap: %s\n",
             (float)size / 1024.0f, (const char*)heap->unk30);
  }
  if (result != NULL) {
    u32 offset = *((u8*)result - 1) + sizeof(_memUsedHdr);
    _memUsedHdr* block = (_memUsedHdr*)((u8*)result - offset);
    u32 footprint = mwMemUsedBlockFootprint(block);
    _mwMemHeap* selectedHeap = heap;
    selectedHeap->unk48 += footprint;
    if (selectedHeap->unk48 > selectedHeap->unk4C) {
      selectedHeap->unk4C = selectedHeap->unk48;
    }
    selectedHeap->unk54++;
    if (selectedHeap->unk54 > selectedHeap->unk58) {
      selectedHeap->unk58 = selectedHeap->unk54;
    }
    selectedHeap->unk5C -= footprint;
    selectedHeap->unk6D = 0;
  }
  return result;
}

extern "C" void* _mwMemCalloc(_mwMemHeap* heap, u32 count, u32 elementSize,
                              s32 alignment, const char* label,
                              const char* file, s32 line) {
  mwMemEnumAlign alignmentBits = mwMemAlignmentBits(alignment);
  u32 size = count * elementSize;
  if (alignmentBits == 4) {
    u32 mask = (1U << alignmentBits) - 1;
    size = (size + mask) & ~mask;
  } else {
    size = (size + (1U << alignmentBits) + 15) & ~15U;
  }
  void* result = _mwMemMallocVirtual(heap, size, alignment, label, file, line, 0);
  if (result != NULL) {
    result = memset(result, 0, size);
  }
  return result;
}

/* TODO: [borked] 0.00%; placeholder stub, body not started. */
extern "C" void _mwMemRealloc(void) {}

/* TODO: [borked] 0.00%; placeholder stub, body not started. */
extern "C" void _mwMemFree(void) {}

/* ELF pool starts with the revision text followed by the heap name. */
static int privInitSystemHeap(unsigned int size) {
  static const char* heapName = "0.90 rev 0\0system heap" + 0xB;
  heapIndex = 0;
  void* hi = OSGetArenaHi();
  OSSetArenaLo(OSInitAlloc(OSGetArenaLo(), hi, 1));
  hi = OSGetArenaHi();
  GameCubeSystemHeap = OSCreateHeap(OSGetArenaLo(), hi);
  OSSetCurrentHeap(GameCubeSystemHeap);
  OSCheckHeap(GameCubeSystemHeap);
  if (mwMemCanAllocateSystemHeap(size) == 0) {
    return 0;
  }
  u32 usable = (size - 0x71) & ~0xFU;
  void* allocation = OSAllocFromHeap(GameCubeSystemHeap, usable + 0x70);
  _mwMemHeap* heap = (_mwMemHeap*)(((u32)allocation + 15) & ~0xFU);
  OsSystemHeap = allocation;
  MEMPRINT("==> allocated SystemHeap size: %6.2f K\n", (float)usable / 1024.0f);
  privAllocHeap(heap, NULL, usable, heapName, mwMemHeapType_unk0, 0, 0);
  SystemHeap = heap;
  mwMemHeapParams params;
  mwMemHeapGetDefaultParams(&params);
  mwMemHeapSetParams(heap, &params);
  mwMemSystemSetHeap(1, heap);
  systemDebugCallbackFunc[0] = NULL;
  systemDebugCallbackFunc[1] = NULL;
  systemDebugCallbackFunc[2] = NULL;
  return 1;
}

/* TODO: [borked] 0.00%; placeholder stub, body not started. */
static void privWipeHeap(_mwMemHeap *) {}

/* TODO: [near miss] 96.89%; register allocation and arithmetic scheduling remain. */
static void privResetHeap(_mwMemHeap* heap, int retainUsed) {
  _memUsedHdr* current;
  if (heap == NULL) {
    return;
  }
  if (heap->type == 2) {
    _memUsedHdr* previous;
    heap->usedBlocks = previous = NULL;
    heap->freeBlocks = (_memFreeHdr*)heap->unk38;
    heap->freeTail = (_memFreeHdr*)heap->unk38;
    heap->unk6D = 1;
    _memFreeHdr* extent = heap->freeBlocks;
    extent->previous = NULL;
    extent->next = NULL;
    extent->size = heap->end - ((u8*)heap->unk38 + 0x10);
    extent->unk0C[0] = 0;
    extent->flags = 0;
    extent->flags &= 0xF0;
    extent->flags |= 4;
    extent->flags &= 0xEF;
    *(_memFreeHdr**)((u8*)extent + extent->size + 0xC) = extent;
    extent->flags |= 0x20;
    extent->unk0C[1] = 0;
    heap->unk48 = 0;
    heap->unk50 = extent->size + 0x10;
    heap->unk54 = 0;
    heap->unk5C = extent->size + 0x10;
    current = (_memUsedHdr*)heap->unk38;
    u32 count = mwMemFixedBlockCount(heap);
    for (u32 i = 0; i < count; i++) {
      current->previous = previous;
      current->size = heap->unk64;
      current->unusedBytes = 0;
      current->allocationFlags = 0;
      current->alignmentOffset = 0;
      current->flags = 0;
      current->flags &= 0xF0;
      current->flags &= 0xEF;
      *(_memUsedHdr**)((u8*)current + current->size + 0xC) = current;
      current->flags |= 0x20;
      _memUsedHdr* next = (_memUsedHdr*)((u8*)current + heap->unk64);
      next = (_memUsedHdr*)((u8*)next + 0x10);
      current->next = next;
      previous = current;
      current = next;
    }
    if (previous != NULL) {
      previous->next = NULL;
    }
  } else {
    if (retainUsed == 0) {
      heap->usedBlocks = NULL;
      heap->freeBlocks = (_memFreeHdr*)heap->unk38;
      heap->freeTail = (_memFreeHdr*)heap->unk38;
      heap->unk6D = 1;
      _memFreeHdr* extent = heap->freeBlocks;
      extent->previous = NULL;
      extent->next = NULL;
      extent->size = heap->end - ((u8*)heap->unk38 + 0x10);
      extent->unk0C[0] = 0;
      extent->flags = 0;
      extent->flags &= 0xF0;
      extent->flags |= 4;
      extent->flags &= 0xEF;
      *(_memFreeHdr**)((u8*)extent + extent->size + 0xC) = extent;
      extent->flags |= 0x20;
      extent->unk0C[1] = 0;
    }
    heap->unk48 = 0;
    heap->unk50 = heap->end - (u8*)heap->unk38;
    heap->unk54 = 0;
    heap->unk5C = heap->end - (u8*)heap->unk38;
    current = heap->usedBlocks;
    while (current != NULL) {
      u32 footprint = mwMemUsedBlockFootprint(current);
      heap->unk48 += footprint;
      if (heap->unk48 > heap->unk4C) {
        heap->unk4C = heap->unk48;
      }
      heap->unk54++;
      if (heap->unk54 > heap->unk58) {
        heap->unk58 = heap->unk54;
      }
      heap->unk5C -= footprint;
      heap->unk6D = 0;
      current = current->next;
    }
  }
}

static void privAllocHeap(_mwMemHeap* heap, _mwMemHeap* parent, unsigned int size,
                          const char* name, mwMemHeapType type,
                          unsigned int value60, unsigned int blockSize) {
  if (heap != NULL) {
    u8* start = (u8*)heap + 0x70;
    heap->unk38 = (u32)start;
    heap->end = start + size;
    heap->unk30 = (u32)name;
    heap->marker = 0xBEABBEAB;
    heap->unk2C = ++heapIndex;
    heap->unk34 = size;
    heap->unk2D = 0;
    heap->type = type;
    heap->usedBlocks = NULL;
    heap->allocationCallback = 0;
    heap->unk60 = value60;
    heap->unk64 = blockSize;
    heap->freeBlocks = (_memFreeHdr*)heap->unk38;
    heap->freeTail = (_memFreeHdr*)heap->unk38;
    privResetHeap(heap, 0);
    heap->unk4C = 0;
    heap->unk58 = 0;
    if (HeapList == NULL) {
      heap->next = NULL;
      heap->previous = NULL;
      heap->parent = NULL;
      heap->children = NULL;
      heap->sibling = NULL;
      HeapList = heap;
    } else if (parent != NULL) {
      heap->next = HeapList;
      heap->previous = NULL;
      HeapList->previous = heap;
      HeapList = heap;
      if (parent->children == NULL) {
        parent->children = heap;
        heap->children = NULL;
        heap->sibling = NULL;
        heap->parent = parent;
      } else {
        heap->parent = parent;
        heap->children = NULL;
        heap->sibling = parent->children;
        parent->children = heap;
      }
    }
  }
}

static void privDeleteBlockFixedBlock(_mwMemHeap* heap, _memUsedHdr* block) {
  if (heap == NULL) {
    return;
  }
  if (block == NULL) {
    return;
  }
  _memUsedHdr* previous = block->previous;
  if (previous == NULL && block->next == NULL) {
    heap->usedBlocks = NULL;
  } else if (previous == NULL && block->next != NULL) {
    heap->usedBlocks = block->next;
    heap->usedBlocks->previous = NULL;
  } else if (previous != NULL && block->next == NULL) {
    previous->next = NULL;
  } else {
    _memUsedHdr* next = block->next;
    previous->next = next;
    next->previous = previous;
  }
  _memFreeHdr* free = heap->freeBlocks;
  if (free == NULL) {
    heap->freeBlocks = (_memFreeHdr*)block;
    block->next = NULL;
    block->previous = NULL;
  } else {
    block->next = (_memUsedHdr*)free;
    block->previous = NULL;
    free->previous = (_memFreeHdr*)block;
    heap->freeBlocks = (_memFreeHdr*)block;
  }
}

extern "C" void* mwMemHeapStrategyCallback(u32 size, _mwMemHeap* heap,
                                           s32 alignment, u32 flags) {
  void* result;
  if (heap->type == 2) {
    if (size <= heap->unk64) {
      result = mwMemTakeFixedBlock(heap);
    } else {
      result = NULL;
    }
  } else {
    mwMemEnumAlign mapped;
    switch (alignment) {
    case 1:
    case 4:
      mapped = mwMemEnumAlign_unk4;
      break;
    case 5:
      mapped = mwMemEnumAlign_unk5;
      break;
    case 6:
      mapped = mwMemEnumAlign_unk6;
      break;
    case 7:
      mapped = mwMemEnumAlign_unk7;
      break;
    case 2:
    case 8:
      mapped = mwMemEnumAlign_unk8;
      break;
    case 128:
      mapped = mwMemEnumAlign_unk0;
      break;
    case MW_MEM_ALIGN_FORCE_32BIT:
    case 0:
    case 3:
    case 16:
    case 32:
    case 64:
    default:
      mapped = mwMemEnumAlign_unk4;
      break;
    }
  
    result = privMallocMem(size, heap, mapped, flags);
  }
  return result;
}

static void* privMallocMem(unsigned int size, _mwMemHeap* heap,
                           mwMemEnumAlign alignment, unsigned char flags) {
  _memUsedHdr* used;
  _memFreeHdr* next;
  if (size == 0) {
    size = 0x10;
  }
  if (heap->freeBlocks != NULL) {
    _memFreeHdr* block;
    switch (heap->type) {
    case 0:
    case 1:
    case 4:
      block = mwMemFindFirstFit(heap, size);
      break;
    case 2:
      block = NULL;
      break;
    case 3:
      block = mwMemFindBestFit(heap, size);
      break;
    case 0x7FFFFFFF:
    default:
      block = mwMemFindFirstFit(heap, size);
      break;
    }
    if (block == NULL) {
      return NULL;
    }
    u32 oldSize = block->size;
    u32 capacity;
    if (oldSize <= size + 0x20) {
      _memFreeHdr* previous = block->previous;
      if (previous == NULL && block->next == NULL) {
        heap->freeBlocks = NULL;
        heap->freeTail = NULL;
      } else if (previous == NULL && block->next != NULL) {
        heap->freeBlocks = block->next;
        heap->freeBlocks->previous = NULL;
      } else if (previous != NULL && block->next == NULL) {
        previous->next = NULL;
        heap->freeTail = previous;
      } else {
        next = block->next;
        previous->next = next;
        next->previous = previous;
      }
      capacity = block->size;
      used = (_memUsedHdr*)block;
      used->flags = 0;
      _memUsedHdr* following = (_memUsedHdr*)((u8*)block + capacity + 0x10);
      used->flags &= 0xEF;
      used->flags &= 0xDF;
      if (heap->end != (u8*)following) {
        following->flags &= 0xEF;
      }
    } else {
      used = (_memUsedHdr*)((u8*)block + oldSize - size);
      block->size = oldSize - (size + 0x10);
      _memUsedHdr* following = (_memUsedHdr*)((u8*)used + size + 0x10);
      capacity = size;
      block->flags &= 0xEF;
      *(_memFreeHdr**)((u8*)block + block->size + 0xC) = block;
      block->flags |= 0x20;
      used->flags = 0;
      used->flags |= 0x10;
      used->flags &= 0xDF;
      if ((u8*)following != heap->end) {
        following->flags &= 0xEF;
      }
    }
    if (heap->usedBlocks == NULL) {
      mwMemInitUsedHeader(used, capacity, size, alignment, flags);
      used->next = NULL;
      used->previous = NULL;
      heap->usedBlocks = used;
    } else {
      mwMemInitUsedHeader(used, capacity, size, alignment, flags);
      used->next = heap->usedBlocks;
      used->previous = NULL;
      heap->usedBlocks->previous = used;
      heap->usedBlocks = used;
    }
    u32 mask = (1u << alignment) - 1;
    u8* data = (u8*)used + 0x10;
    u8* aligned = (u8*)(((u32)data + mask) & ~mask);
    u8 displacement = aligned - data;
    used->alignmentOffset = displacement;
    aligned[-1] = displacement;
    return aligned;
  }
  return NULL;
}

static inline void mwMemUnlinkFreeBlock(_mwMemHeap* heap, _memFreeHdr* block) {
  _memFreeHdr* previous = block->previous;
  if (previous == NULL && block->next == NULL) {
    heap->freeBlocks = NULL;
    heap->freeTail = NULL;
  } else if (previous == NULL && block->next != NULL) {
    heap->freeBlocks = block->next;
    heap->freeBlocks->previous = NULL;
  } else if (previous != NULL && block->next == NULL) {
    previous->next = NULL;
    heap->freeTail = previous;
  } else {
    _memFreeHdr* next = block->next;
    previous->next = next;
    next->previous = previous;
  }
}

/* TODO: [near miss] 99.24%; register allocation remains. */
static _memFreeHdr* privCoalesceFreeBlocksBoundaryTags(_mwMemHeap* heap,
                                                      _memFreeHdr* block) {
  _memFreeHdr* following;
  u32 size;
  u32 previousFree;
  _memFreeHdr* previous;
  u32 nextFree;
  size = block->size;
  nextFree = 0;
  following = (_memFreeHdr*)((u8*)block + size + sizeof(*block));
  previousFree = (block->flags >> 4) & 1;
  if (heap->end != (u8*)following) {
    nextFree = (following->flags >> 5) & 1;
  }
  if (block->next == NULL && block->previous == NULL) {
  } else if (block->next != NULL && block->previous == NULL) {
    if (following == block->next) {
      block->size = (size + 16) + block->next->size;
      mwMemUnlinkFreeBlock(heap, block->next);
    }
  } else if (block->next == NULL && block->previous != NULL) {
    previous = block->previous;
    if ((u8*)previous + previous->size + sizeof(*previous) == (u8*)block) {
      previous->size = (size + 16) + previous->size;
      mwMemUnlinkFreeBlock(heap, block);
      block = block->previous;
    }
  } else {
    if (nextFree == 1) {
      block->size = (size + 16) + following->size;
      mwMemUnlinkFreeBlock(heap, following);
    }
    if (previousFree == 1) {
      previous = block->previous;
      previous->size = (block->size + 16) + previous->size;
      mwMemUnlinkFreeBlock(heap, block);
      block = previous;
    }
  }
  return block;
}

static void privReturnUsedBlockToFreeList(_mwMemHeap* heap, _memUsedHdr* used) {
  _memFreeHdr* block = (_memFreeHdr*)used;
  _memFreeHdr* entry = heap->freeBlocks;
  if (entry == NULL) {
    block->next = NULL;
    block->previous = NULL;
    block->unk0C[0] = 0;
    heap->freeBlocks = block;
    heap->freeTail = block;
    return;
  }
  if (block < entry) {
    block->previous = NULL;
    block->next = entry;
    block->unk0C[0] = 0;
    entry->previous = block;
    heap->freeBlocks = block;
    return;
  }
  _memFreeHdr* previous;
  _memFreeHdr* following = NULL;
  previous = NULL;
  while (entry != NULL) {
    if (block < entry) {
      following = entry;
      break;
    }
    previous = entry;
    entry = entry->next;
  }
  if (following != NULL) {
    block->previous = following->previous;
    block->next = following;
    block->unk0C[0] = 0;
    following->previous->next = block;
    following->previous = block;
  } else if (previous != NULL) {
    block->previous = previous;
    block->next = NULL;
    block->unk0C[0] = 0;
    previous->next = block;
    heap->freeTail = block;
  }
}

extern "C" u32 mwMemSystemGetAvailSize(void) {
  OSInit();
  u8* arenaHi = (u8*)OSGetArenaHi();
  return arenaHi - (u8*)OSGetArenaLo() - 0x140;
}

static inline _memFreeHdr* mwMemFindFirstFit(_mwMemHeap* heap, u32 size) {
  _memFreeHdr* node = heap->freeTail;
  _memFreeHdr* result = NULL;
  while (node != NULL) {
    if (node->size >= size) {
      result = node;
      break;
    }
    node = node->previous;
  }
  return result;
}

static inline _memFreeHdr* mwMemFindBestFit(_mwMemHeap* heap, u32 size) {
  _memFreeHdr* node = heap->freeTail;
  _memFreeHdr* result = NULL;
  u32 smallest = heap->unk34;
  while (node != NULL) {
    if (node->size >= size) {
      if (node->size == size) {
        result = node;
        break;
      }
      if (node->size < smallest) {
        smallest = node->size;
        result = node;
      }
    }
    node = node->previous;
  }
  return result;
}

static inline void mwMemInitUsedHeader(_memUsedHdr* block, u32 capacity,
                                      u32 requested, mwMemEnumAlign alignment,
                                      u8 flags) {
  block->size = capacity;
  block->unusedBytes = capacity - requested;
  block->flags &= 0xF0;
  block->flags |= alignment & 0xF;
  block->allocationFlags = flags;
}

static inline void* mwMemTakeFixedBlock(_mwMemHeap* heap) {
  _memFreeHdr* block = heap->freeBlocks;
  void* result;
  if (block == NULL) {
    result = NULL;
  } else {
    result = (u8*)block + 0x10;
    _memFreeHdr* nextFree = block->next;
    if (nextFree != NULL) {
      nextFree->previous = NULL;
      heap->freeBlocks = nextFree;
    } else {
      heap->freeBlocks = NULL;
    }
    _memUsedHdr* used = (_memUsedHdr*)block;
    _memUsedHdr* head = heap->usedBlocks;
    if (head == NULL) {
      heap->usedBlocks = used;
      used->next = NULL;
      used->previous = NULL;
    } else {
      heap->usedBlocks = used;
      used->previous = NULL;
      used->next = head;
      head->previous = used;
    }
  }
  return result;
}

static inline u32 mwMemUsedPayloadSize(_memUsedHdr* block, s32* extra) {
  s32 alignment = block->flags & 0xF;
  *extra = alignment == 4 ? 0 : (1 << alignment);
  return block->size - block->unusedBytes - *extra;
}

static inline u32 mwMemUsedBlockFootprint(_memUsedHdr* block) {
  if (block != NULL) {
    u32 size;
    s32 extra;
    size = mwMemUsedPayloadSize(block, &extra);
    u32 overhead = extra + 0x10;
    return size + overhead;
  }
  return 0;
}

static inline u32 mwMemFixedBlockCount(_mwMemHeap* heap) {
  u32 bytes = heap->end - (u8*)heap->unk38;
  return bytes / (heap->unk64 + sizeof(_memUsedHdr));
}

static inline int mwMemCanAllocateSystemHeap(u32 size) {
  void* allocation = OSAllocFromHeap(GameCubeSystemHeap, size);
  if (allocation == NULL) {
    return 0;
  }
  OSFreeToHeap(GameCubeSystemHeap, allocation);
  return 1;
}

#include <string.h>

static inline mwMemEnumAlign mwMemAlignmentBits(s32 alignment) {
  mwMemEnumAlign result;
  switch (alignment) {
  case 1:
  case 4: result = mwMemEnumAlign_unk4; break;
  case 5: result = mwMemEnumAlign_unk5; break;
  case 6: result = mwMemEnumAlign_unk6; break;
  case 7: result = mwMemEnumAlign_unk7; break;
  case 2:
  case 8: result = mwMemEnumAlign_unk8; break;
  case 128: result = mwMemEnumAlign_unk0; break;
  case 0:
  case 3:
  case 16:
  case 32:
  case 64:
  case MW_MEM_ALIGN_FORCE_32BIT:
  default: result = mwMemEnumAlign_unk4; break;
  }
  return result;
}
