#include <mwmem/mwMem.h>

struct _mwMemHeap* overflow_heap;
struct _mwMemHeap* permanent_heap;
struct _mwMemHeap* section_heap;
struct _mwMemHeap* wave_heap;
struct _mwMemHeap* MSLMFL_HEAP;
struct _mwMemHeap* cpp_heap;
struct _mwMemHeap* movie_heap;
struct _mwMemHeap* mkobj_heap;
struct _mwMemHeap* mksobj_heap;
struct _mwMemHeap* mkmaterial_heap;
struct _mwMemHeap* mkproc_heap;
struct _mwMemHeap* tinystack_heap;
struct _mwMemHeap* bigstack_heap;
struct _mwMemHeap* fixed_block_16_heap;
struct _mwMemHeap* fixed_block_32_heap;
struct _mwMemHeap* fixed_block_64_heap;
struct _mwMemHeap* fixed_block_128_heap;
struct _mwMemHeap* fixed_block_1024_heap;

static void reportHeapAllocationFailure(void) {
  MEMPRINT("allocaion Failure callback()\n");
}

static void reportHeapFree(const char* filename, const char* comment) {
  MEMPRINT("free callback()\n");
  MEMPRINT("filename: %s  comment: %s \n", filename, comment);
}

static void reportHeapAllocation(const char* filename, const char* comment) {
  MEMPRINT("allocaion callback()\n");
  MEMPRINT("filename: %s  comment: %s \n", filename, comment);
}

static void* movie_strategy(u32 size, struct _mwMemHeap** heap,
    s32 alignment, u8 requestFlags) {
  mwMemHeapInfo info;
  struct _mwMemHeap* fallback;
  u32 heapTag;
  void* memory;

  mwMemHeapGetInfo(*heap, &info);
  heapTag = info.unk18;
  fallback = mwMemSystemGetHeap(1);
  memory = mwMemHeapStrategyCallback(size, wave_heap, alignment, heapTag);
  if (memory != 0) {
    *heap = wave_heap;
    return memory;
  }
  memory = mwMemHeapStrategyCallback(size, section_heap, alignment, heapTag);
  if (memory != 0) {
    *heap = section_heap;
    return memory;
  }
  memory = mwMemHeapStrategyCallback(size, permanent_heap, alignment, heapTag);
  if (memory != 0) {
    *heap = permanent_heap;
    return memory;
  }
  memory = mwMemHeapStrategyCallback(size, fallback, alignment, heapTag);
  if (memory != 0) {
    *heap = fallback;
  }
  return memory;
}

static void* fixed1024_strategy(u32 size, struct _mwMemHeap** heap,
    s32 alignment, u8 requestFlags) {
  mwMemHeapInfo info;
  struct _mwMemHeap* fallback;
  u32 heapTag;
  void* memory;

  mwMemHeapGetInfo(*heap, &info);
  heapTag = info.unk18;
  fallback = mwMemSystemGetHeap(1);
  memory = mwMemHeapStrategyCallback(size, fixed_block_1024_heap, alignment, heapTag);
  if (memory != 0) {
    *heap = fixed_block_1024_heap;
    return memory;
  }
  memory = mwMemHeapStrategyCallback(size, wave_heap, alignment, heapTag);
  if (memory != 0) {
    *heap = wave_heap;
    return memory;
  }
  memory = mwMemHeapStrategyCallback(size, fallback, alignment, heapTag);
  if (memory != 0) {
    *heap = fallback;
  }
  return memory;
}

static void* fixed128_strategy(u32 size, struct _mwMemHeap** heap,
    s32 alignment, u8 requestFlags) {
  mwMemHeapInfo info;
  struct _mwMemHeap* fallback;
  u32 heapTag;
  void* memory;

  mwMemHeapGetInfo(*heap, &info);
  heapTag = info.unk18;
  fallback = mwMemSystemGetHeap(1);
  memory = mwMemHeapStrategyCallback(size, fixed_block_128_heap, alignment, heapTag);
  if (memory != 0) {
    *heap = fixed_block_128_heap;
    return memory;
  }
  memory = mwMemHeapStrategyCallback(size, wave_heap, alignment, heapTag);
  if (memory != 0) {
    *heap = wave_heap;
    return memory;
  }
  memory = mwMemHeapStrategyCallback(size, fallback, alignment, heapTag);
  if (memory != 0) {
    *heap = fallback;
  }
  return memory;
}

static void* fixed64_strategy(u32 size, struct _mwMemHeap** heap,
    s32 alignment, u8 requestFlags) {
  mwMemHeapInfo info;
  struct _mwMemHeap* fallback;
  u32 heapTag;
  void* memory;

  mwMemHeapGetInfo(*heap, &info);
  heapTag = info.unk18;
  fallback = mwMemSystemGetHeap(1);
  memory = mwMemHeapStrategyCallback(size, fixed_block_64_heap, alignment, heapTag);
  if (memory != 0) {
    *heap = fixed_block_64_heap;
    return memory;
  }
  memory = mwMemHeapStrategyCallback(size, fixed_block_128_heap, alignment, heapTag);
  if (memory != 0) {
    *heap = fixed_block_128_heap;
    return memory;
  }
  memory = mwMemHeapStrategyCallback(size, wave_heap, alignment, heapTag);
  if (memory != 0) {
    *heap = wave_heap;
    return memory;
  }
  memory = mwMemHeapStrategyCallback(size, fallback, alignment, heapTag);
  if (memory != 0) {
    *heap = fallback;
  }
  return memory;
}

static void* fixed32_strategy(u32 size, struct _mwMemHeap** heap,
    s32 alignment, u8 requestFlags) {
  mwMemHeapInfo info;
  struct _mwMemHeap* fallback;
  u32 heapTag;
  void* memory;

  mwMemHeapGetInfo(*heap, &info);
  heapTag = info.unk18;
  fallback = mwMemSystemGetHeap(1);
  memory = mwMemHeapStrategyCallback(size, fixed_block_32_heap, alignment, heapTag);
  if (memory != 0) {
    *heap = fixed_block_32_heap;
    return memory;
  }
  memory = mwMemHeapStrategyCallback(size, fixed_block_64_heap, alignment, heapTag);
  if (memory != 0) {
    *heap = fixed_block_64_heap;
    return memory;
  }
  memory = mwMemHeapStrategyCallback(size, wave_heap, alignment, heapTag);
  if (memory != 0) {
    *heap = wave_heap;
    return memory;
  }
  memory = mwMemHeapStrategyCallback(size, fallback, alignment, heapTag);
  if (memory != 0) {
    *heap = fallback;
  }
  return memory;
}

static void* fixed16_strategy(u32 size, struct _mwMemHeap** heap,
    s32 alignment, u8 requestFlags) {
  mwMemHeapInfo info;
  struct _mwMemHeap* fallback;
  u32 heapTag;
  void* memory;

  mwMemHeapGetInfo(*heap, &info);
  heapTag = info.unk18;
  fallback = mwMemSystemGetHeap(1);
  memory = mwMemHeapStrategyCallback(size, fixed_block_16_heap, alignment, heapTag);
  if (memory != 0) {
    *heap = fixed_block_16_heap;
    return memory;
  }
  memory = mwMemHeapStrategyCallback(size, fixed_block_32_heap, alignment, heapTag);
  if (memory != 0) {
    *heap = fixed_block_32_heap;
    return memory;
  }
  memory = mwMemHeapStrategyCallback(size, wave_heap, alignment, heapTag);
  if (memory != 0) {
    *heap = wave_heap;
    return memory;
  }
  memory = mwMemHeapStrategyCallback(size, fallback, alignment, heapTag);
  if (memory != 0) {
    *heap = fallback;
  }
  return memory;
}

static inline void setHeapAllocationCallback(mwMemHeapParams* params,
    const mwMemAllocationCallback* callback) {
  params->allocationCallback = *callback;
}

void mwMemHeapInit(void) {
  mwMemHeapConfig config;
  mwMemHeapParams params;
  mwMemFixedHeapConfig fixed;
  u32 maxSize;
  u32 blockCount;
  mwMemAllocationCallback strategy;
  struct _mwMemHeap* system;

  system = mwMemSystemGetHeap(0);
  mwMemHeapGetDefaultParams(&params);
  params.unk0C = 0xAB;
  params.unk0D = 0;
  config.parent = system;
  config.size = 0x580000;
  config.type = 0;
  config.unk08 = 16;
  config.name = "Permanent heap";
  config.unk18 = 0;
  permanent_heap = _mwMemHeapCreate(&config, &params, 0, 0);
  config.parent = system;
  config.size = 0xB00000;
  config.type = 0;
  config.unk08 = 16;
  config.name = "Section heap";
  config.unk18 = 0;
  section_heap = _mwMemHeapCreate(&config, &params, 0, 0);
  config.parent = system;
  config.size = 0xA2800;
  config.type = 0;
  config.unk08 = 16;
  config.name = "MSLMFL heap";
  config.unk18 = 0;
  MSLMFL_HEAP = _mwMemHeapCreate(&config, &params, 0, 0);
  config.parent = system;
  config.size = 0x280000;
  config.type = 0;
  config.unk08 = 16;
  config.name = "Wave heap";
  config.unk18 = 0;
  wave_heap = _mwMemHeapCreate(&config, &params, 0, 0);
  config.parent = system;
  config.size = 0;
  config.type = 0;
  config.unk08 = 16;
  config.name = "CPP heap";
  config.unk18 = 0;
  cpp_heap = _mwMemHeapCreate(&config, &params, 0, 0);
  if (cpp_heap != 0) {
    mwMemSystemSetHeap(2, cpp_heap);
  }
  config.parent = system;
  config.size = mwMEM_VIRTUAL_HEAP_SIZE;
  config.type = 1;
  config.unk08 = 16;
  config.name = "MPEG heap";
  config.unk18 = 0;
  strategy = movie_strategy;
  setHeapAllocationCallback(&params, &strategy);
  movie_heap = _mwMemHeapCreate(&config, &params, 0, 0);
  mwMemHeapGetDefaultParams(&params);
  params.unk0C = 0xAB;
  params.unk0D = 0;
  fixed.unk00 = 2;
  fixed.unk0C = 8;
  config.parent = wave_heap;
  config.size = 1;
  config.type = 2;
  config.unk18 = 0;
  config.fixed = &fixed;
  config.unk08 = 4;
  config.name = "MKOBJ fixed block heap";
  fixed.blockSize = 212;
  fixed.count = 100;
  fixed.alignment = 4;
  mkobj_heap = _mwMemHeapCreate(&config, &params, 0, 0);
  config.name = "MKSOBJ fixed block heap";
  fixed.blockSize = 128;
  fixed.count = 120;
  fixed.alignment = 4;
  mksobj_heap = _mwMemHeapCreate(&config, &params, 0, 0);
  config.name = "MKMATERIAL fixed block heap";
  fixed.blockSize = 28;
  fixed.count = 100;
  fixed.alignment = 4;
  mkmaterial_heap = _mwMemHeapCreate(&config, &params, 0, 0);
  config.name = "MKPROC fixed block heap";
  fixed.blockSize = 208;
  fixed.count = 160;
  fixed.alignment = 3;
  mkproc_heap = _mwMemHeapCreate(&config, &params, 0, 0);
  config.name = "fixed block 16 heap";
  fixed.blockSize = 64;
  fixed.count = 256;
  fixed.alignment = 4;
  strategy = fixed16_strategy;
  setHeapAllocationCallback(&params, &strategy);
  fixed_block_16_heap = _mwMemHeapCreate(&config, &params, 0, 0);
  config.name = "fixed block 32 heap";
  fixed.blockSize = 128;
  fixed.count = 128;
  fixed.alignment = 4;
  strategy = fixed32_strategy;
  setHeapAllocationCallback(&params, &strategy);
  fixed_block_32_heap = _mwMemHeapCreate(&config, &params, 0, 0);
  config.name = "fixed block 64 heap";
  fixed.blockSize = 256;
  fixed.count = 64;
  fixed.alignment = 4;
  strategy = fixed64_strategy;
  setHeapAllocationCallback(&params, &strategy);
  fixed_block_64_heap = _mwMemHeapCreate(&config, &params, 0, 0);
  config.name = "fixed block 128 heap";
  fixed.blockSize = 512;
  fixed.count = 64;
  fixed.alignment = 4;
  strategy = fixed128_strategy;
  setHeapAllocationCallback(&params, &strategy);
  fixed_block_128_heap = _mwMemHeapCreate(&config, &params, 0, 0);
  config.name = "fixed block 1024 heap";
  fixed.blockSize = 4096;
  fixed.count = 96;
  fixed.alignment = 4;
  strategy = fixed1024_strategy;
  setHeapAllocationCallback(&params, &strategy);
  fixed_block_1024_heap = _mwMemHeapCreate(&config, &params, 0, 0);
  fixed.unk0C = 0;
  config.name = "TINYSTACK fixed block heap";
  fixed.blockSize = 512;
  fixed.count = 144;
  fixed.alignment = 3;
  params.allocationCallback = 0;
  tinystack_heap = _mwMemHeapCreate(&config, &params, 0, 0);
  config.name = "BIGSTACK fixed block heap";
  fixed.blockSize = 16384;
  fixed.count = 16;
  fixed.alignment = 3;
  params.allocationCallback = 0;
  bigstack_heap = _mwMemHeapCreate(&config, &params, 0, 0);
  mwMemHeapGetMaxFreeBlock(system, &maxSize, &blockCount);
  MEMPRINT("==>> allocated OVERFLOW_HEAP size: %6.2f K\n", maxSize / 1024.0f);
  config.parent = system;
  config.size = maxSize;
  config.type = 4;
  config.unk08 = 16;
  config.name = "OVERFLOW Heap";
  config.unk18 = 0;
  overflow_heap = _mwMemHeapCreate(&config, &params, 0, 0);
  mwMemSystemSetHeap(1, overflow_heap);
}
