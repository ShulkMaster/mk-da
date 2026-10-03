#include "hvqm4buf.h"
#include <dolphin/os.h>
#include <mwmem/mwMem.h>

extern void* gHeap;

s32 HVQM4BufvGetDispNums(HVQM4Bufv* buf)
{
  s32 count = 0;
  BOOL enabled = OSDisableInterrupts();
  HVQM4BufvNode* node = buf->dispHead;
  while (node != NULL) {
    node = node->nextDisp;
    count++;
  }
  OSRestoreInterrupts(enabled);
  return count;
}

HVQM4BufvNode* HVQM4BufvChkDisp(HVQM4Bufv* buf)
{
  BOOL enabled = OSDisableInterrupts();
  HVQM4BufvNode* node = buf->dispHead;
  OSRestoreInterrupts(enabled);
  return node;
}

HVQM4BufvNode* HVQM4BufvGetDisp(HVQM4Bufv* buf)
{
  BOOL enabled = OSDisableInterrupts();
  HVQM4BufvNode* node = buf->dispHead;
  if (node != NULL) {
    if (node->nextDisp == NULL) {
      buf->dispHead = NULL;
      buf->dispTail = NULL;
    } else {
      buf->dispHead = node->nextDisp;
    }
    node->nextDisp = NULL;
  }
  OSRestoreInterrupts(enabled);
  return node;
}

void HVQM4BufvSetDisp(HVQM4Bufv* buf, HVQM4BufvNode* node)
{
  BOOL enabled = OSDisableInterrupts();
  if (buf->dispHead == NULL) {
    buf->dispHead = node;
    buf->dispTail = node;
  } else {
    buf->dispTail->nextDisp = node;
    buf->dispTail = node;
  }
  node->nextDisp = NULL;
  OSRestoreInterrupts(enabled);
}

HVQM4BufvNode* HVQM4BufvGetFree(HVQM4Bufv* buf)
{
  HVQM4BufvNode* result = NULL;
  BOOL enabled = OSDisableInterrupts();
  HVQM4BufvNode* first = buf->freeHead;
  HVQM4BufvNode* node = first;
  if (first != NULL) {
    do {
      if (node->refCount == 0) {
        result = node;
        if (node == first) {
          if (node->nextFree == node) {
            buf->freeHead = NULL;
          } else {
            buf->freeHead = node->nextFree;
            node->prevFree->nextFree = node->nextFree;
            node->nextFree->prevFree = node->prevFree;
          }
        } else {
          node->prevFree->nextFree = node->nextFree;
          node->nextFree->prevFree = node->prevFree;
        }
        node->prevFree = NULL;
        node->nextFree = NULL;
        break;
      }
      node = node->nextFree;
    } while (node != first);
  }
  OSRestoreInterrupts(enabled);
  return result;
}

void HVQM4BufvSetFree(HVQM4Bufv* buf, HVQM4BufvNode* node)
{
  BOOL enabled = OSDisableInterrupts();
  HVQM4BufvNode* first = buf->freeHead;
  if (first == NULL) {
    node->nextFree = node;
    node->prevFree = node;
    buf->freeHead = node;
  } else {
    HVQM4BufvNode* last = first->prevFree;
    first->prevFree = node;
    node->nextFree = first;
    node->prevFree = last;
    last->nextFree = node;
  }
  OSRestoreInterrupts(enabled);
}

s32 HVQM4BufvClose(HVQM4Bufv* buf)
{
  _mwMemFree(buf, "HVQM4Bufv.c", 96);
  return 1;
}

HVQM4Bufv* HVQM4BufvCreate(int size, unsigned int count)
{
  int nodesSize = (count * sizeof(HVQM4BufvNode) + 31) & ~31;
  int allocSize;
  HVQM4Bufv* buf;
  HVQM4BufvNode* node;
  HVQM4BufvNode* last;
  HVQM4BufvNode* first;
  int data;
  int i;
  allocSize = ((size + 63) & ~63) * count;
  allocSize += 64;
  allocSize += nodesSize;
  allocSize += 32;
  buf = _mwMemMalloc(gHeap, allocSize, 6, "HVQM4 bufv alloc movie", "HVQM4Bufv.c", 60);
  if (buf == NULL) {
    OSReport("HVQM4 bufv alloc error !\n");
    return NULL;
  }
  first = (HVQM4BufvNode*)((u8*)buf + 32);
  node = first;
  data = ((int)first + nodesSize + 63) & ~63;
  last = &first[count - 1];
  for (i = 0; i < count; i++) {
    if (i == 0) {
      node->prevFree = last;
      node->nextFree = &first[i + 1];
    } else if (i == count - 1) {
      node->prevFree = &first[i - 1];
      node->nextFree = first;
    } else {
      node->prevFree = &first[i - 1];
      node->nextFree = &first[i + 1];
    }
    node->nextDisp = NULL;
    node->refCount = 0;
    node->size = size;
    node->data = (void*)data;
    node++;
    data += size;
    data = (data + 63) & ~63;
  }
  buf->freeHead = first;
  buf->dispHead = NULL;
  buf->dispTail = NULL;
  return buf;
}
