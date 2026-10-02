/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/os.h>

#define ALIGNMENT 32
#define HEADERSIZE 32U
#define MINOBJSIZE 64U

typedef int OSHeapHandle;
typedef struct Cell {
  struct Cell *prev;
  struct Cell *next;
  long size;
} Cell;

typedef struct HeapDesc {
  long size;
  Cell *free;
  Cell *allocated;
} HeapDesc;

volatile OSHeapHandle __OSCurrHeap = -1;
static HeapDesc *HeapArray;
static int NumHeaps;
static void *ArenaStart;
static void *ArenaEnd;

static inline Cell *DLAddFront(Cell *list, Cell *cell) {
  cell->next = list;
  cell->prev = 0;
  if (list) {
    list->prev = cell;
  }
  return cell;
}

static inline Cell *DLExtract(Cell *list, Cell *cell) {
  if (cell->next) {
    cell->next->prev = cell->prev;
  }
  if (cell->prev == NULL) {
    return cell->next;
  }
  cell->prev->next = cell->next;
  return list;
}

static Cell *DLInsert(Cell *list, Cell *cell) {
  Cell *prev;
  Cell *next;

  for (next = list, prev = NULL; next != 0; prev = next, next = next->next) {
    if (cell <= next) {
      break;
    }
  }

  cell->next = next;
  cell->prev = prev;
  if (next) {
    next->prev = cell;
    if ((unsigned char *)cell + cell->size == (unsigned char *)next) {
      cell->size += next->size;
      next = next->next;
      cell->next = next;
      if (next) {
        next->prev = cell;
      }
    }
  }
  if (prev) {
    prev->next = cell;
    if ((unsigned char *)prev + prev->size == (unsigned char *)cell) {
      prev->size += cell->size;
      prev->next = next;
      if (next) {
        next->prev = prev;
      }
    }
    return list;
  }
  return cell;
}

void *OSAllocFromHeap(OSHeapHandle heap, u32 size) {
  HeapDesc *hd;
  Cell *cell;
  Cell *newCell;
  s32 leftoverSize;

  hd = &HeapArray[heap];
  size += HEADERSIZE;
  size = (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);

  for (cell = hd->free; cell != NULL; cell = cell->next) {
    if ((long)size <= cell->size) {
      break;
    }
  }

  if (cell == NULL) {
    return NULL;
  }

  leftoverSize = cell->size - size;
  if (leftoverSize < MINOBJSIZE) {
    hd->free = DLExtract(hd->free, cell);
  } else {
    cell->size = size;
    newCell = (void *)((unsigned char *)cell + size);
    newCell->size = leftoverSize;
    newCell->prev = cell->prev;
    newCell->next = cell->next;
    if (newCell->next != NULL) {
      newCell->next->prev = newCell;
    }
    if (newCell->prev != NULL) {
      newCell->prev->next = newCell;
    } else {
      hd->free = newCell;
    }
  }

  hd->allocated = DLAddFront(hd->allocated, cell);
  return (unsigned char *)cell + HEADERSIZE;
}

void OSFreeToHeap(OSHeapHandle heap, void *ptr) {
  HeapDesc *hd;
  Cell *cell;

  cell = (void *)((unsigned long)ptr - HEADERSIZE);
  hd = &HeapArray[heap];
  hd->allocated = DLExtract(hd->allocated, cell);
  hd->free = DLInsert(hd->free, cell);
}

OSHeapHandle OSSetCurrentHeap(OSHeapHandle heap) {
  OSHeapHandle prev;

  prev = __OSCurrHeap;
  __OSCurrHeap = heap;
  return prev;
}

void *OSInitAlloc(void *arenaStart, void *arenaEnd, int maxHeaps) {
  unsigned long arraySize;
  int i;
  HeapDesc *hd;

  arraySize = maxHeaps * sizeof(HeapDesc);
  HeapArray = arenaStart;
  NumHeaps = maxHeaps;

  for (i = 0; i < NumHeaps; i++) {
    hd = &HeapArray[i];
    hd->size = -1;
    hd->free = hd->allocated = 0;
  }
  __OSCurrHeap = -1;
  arenaStart = (void *)((unsigned long)((char *)HeapArray + arraySize));
  arenaStart = (void *)(((unsigned long)arenaStart + ALIGNMENT - 1) & ~(ALIGNMENT - 1));
  ArenaStart = arenaStart;
  ArenaEnd = (void *)((unsigned long)arenaEnd & ~(ALIGNMENT - 1));
  return arenaStart;
}

OSHeapHandle OSCreateHeap(void *start, void *end) {
  OSHeapHandle heap;
  HeapDesc *hd;
  Cell *cell;

  start = (void *)(((unsigned long)start + ALIGNMENT - 1) & ~(ALIGNMENT - 1));
  end = (void *)((unsigned long)end & ~(ALIGNMENT - 1));

  for (heap = 0; heap < NumHeaps; heap++) {
    hd = &HeapArray[heap];
    if (hd->size < 0) {
      hd->size = (unsigned long)end - (unsigned long)start;
      cell = start;
      cell->prev = 0;
      cell->next = 0;
      cell->size = hd->size;
      hd->free = cell;
      hd->allocated = 0;
      return heap;
    }
  }
  return -1;
}

#define InRange(cell, arenaStart, arenaEnd) \
  ((u32)(arenaStart) <= (u32)(cell) && (u32)(cell) < (u32)(arenaEnd))
#define OFFSET(n, a) ((u32)(n) & ((a) - 1))
#define ASSERTREPORT(line, cond) \
  if (!(cond)) { \
    OSReport("OSCheckHeap: Failed " #cond " in %d", line); \
    return -1; \
  }

long OSCheckHeap(OSHeapHandle heap) {
  HeapDesc* hd;
  Cell* cell;
  long total = 0;
  long free = 0;

  ASSERTREPORT(0x37D, HeapArray);
  ASSERTREPORT(0x37E, 0 <= heap && heap < NumHeaps);
  hd = &HeapArray[heap];
  ASSERTREPORT(0x381, 0 <= hd->size);
  ASSERTREPORT(0x383, hd->allocated == NULL || hd->allocated->prev == NULL);

  for (cell = hd->allocated; cell; cell = cell->next) {
    ASSERTREPORT(0x386, InRange(cell, ArenaStart, ArenaEnd));
    ASSERTREPORT(0x387, OFFSET(cell, ALIGNMENT) == 0);
    ASSERTREPORT(0x388, cell->next == NULL || cell->next->prev == cell);
    ASSERTREPORT(0x389, MINOBJSIZE <= cell->size);
    ASSERTREPORT(0x38A, OFFSET(cell->size, ALIGNMENT) == 0);
    total += cell->size;
    ASSERTREPORT(0x38D, 0 < total && total <= hd->size);
  }

  ASSERTREPORT(0x395, hd->free == NULL || hd->free->prev == NULL);
  for (cell = hd->free; cell; cell = cell->next) {
    ASSERTREPORT(0x398, InRange(cell, ArenaStart, ArenaEnd));
    ASSERTREPORT(0x399, OFFSET(cell, ALIGNMENT) == 0);
    ASSERTREPORT(0x39A, cell->next == NULL || cell->next->prev == cell);
    ASSERTREPORT(0x39B, MINOBJSIZE <= cell->size);
    ASSERTREPORT(0x39C, OFFSET(cell->size, ALIGNMENT) == 0);
    ASSERTREPORT(0x39D, cell->next == NULL || (char*) cell + cell->size < (char*) cell->next);
    total += cell->size;
    free = cell->size + free;
    free -= HEADERSIZE;
    ASSERTREPORT(0x3A1, 0 < total && total <= hd->size);
  }
  ASSERTREPORT(0x3A8, total == hd->size);
  return free;
}

void OSDumpHeap(OSHeapHandle heap) {
  HeapDesc *hd;
  Cell *cell;

  OSReport("\nOSDumpHeap(%d):\n", heap);
  hd = &HeapArray[heap];
  if (hd->size < 0) {
    OSReport("--------Inactive\n");
    return;
  }
  OSReport("addr\tsize\t\tend\tprev\tnext\n");
  OSReport("--------Allocated\n");
  for (cell = hd->allocated; cell; cell = cell->next) {
    OSReport("%x\t%d\t%x\t%x\t%x\n", cell, cell->size, (char *)cell + cell->size, cell->prev,
             cell->next);
  }
  OSReport("--------Free\n");
  for (cell = hd->free; cell; cell = cell->next) {
    OSReport("%x\t%d\t%x\t%x\t%x\n", cell, cell->size, (char *)cell + cell->size, cell->prev,
             cell->next);
  }
}
