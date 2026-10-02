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

extern volatile OSHeapHandle __OSCurrHeap;
extern HeapDesc *HeapArray_8041CD68;
extern int NumHeaps_8041CD6C;
extern void *ArenaStart_8041CD70;
extern void *ArenaEnd_8041CD74;

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

  hd = &HeapArray_8041CD68[heap];
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
  hd = &HeapArray_8041CD68[heap];
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
  HeapArray_8041CD68 = arenaStart;
  NumHeaps_8041CD6C = maxHeaps;

  for (i = 0; i < NumHeaps_8041CD6C; i++) {
    hd = &HeapArray_8041CD68[i];
    hd->size = -1;
    hd->free = hd->allocated = 0;
  }
  __OSCurrHeap = -1;
  arenaStart = (void *)((unsigned long)((char *)HeapArray_8041CD68 + arraySize));
  arenaStart = (void *)(((unsigned long)arenaStart + ALIGNMENT - 1) & ~(ALIGNMENT - 1));
  ArenaStart_8041CD70 = arenaStart;
  ArenaEnd_8041CD74 = (void *)((unsigned long)arenaEnd & ~(ALIGNMENT - 1));
  return arenaStart;
}

OSHeapHandle OSCreateHeap(void *start, void *end) {
  OSHeapHandle heap;
  HeapDesc *hd;
  Cell *cell;

  start = (void *)(((unsigned long)start + ALIGNMENT - 1) & ~(ALIGNMENT - 1));
  end = (void *)((unsigned long)end & ~(ALIGNMENT - 1));

  for (heap = 0; heap < NumHeaps_8041CD6C; heap++) {
    hd = &HeapArray_8041CD68[heap];
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
