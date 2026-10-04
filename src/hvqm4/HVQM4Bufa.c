#include "hvqm4buf.h"
#include <dolphin/os.h>
#include <mwmem/mwMem.h>

extern void* gHeap;

s32 HVQM4BufaGetSendNums(HVQM4Bufa* buf)
{
  s32 count = 0;
  BOOL enabled = OSDisableInterrupts();
  HVQM4BufaNode* node = buf->sendHead;
  while (node != NULL) {
    node = node->nextSend;
    count++;
  }
  OSRestoreInterrupts(enabled);
  return count;
}

HVQM4BufaNode* HVQM4BufaChkSend(HVQM4Bufa* buf)
{
  BOOL enabled = OSDisableInterrupts();
  HVQM4BufaNode* node = buf->sendHead;
  OSRestoreInterrupts(enabled);
  return node;
}

HVQM4BufaNode* HVQM4BufaGetSend(HVQM4Bufa* buf)
{
  BOOL enabled = OSDisableInterrupts();
  HVQM4BufaNode* node = buf->sendHead;
  if (node != NULL) {
    if (node->nextSend == NULL) {
      buf->sendHead = NULL;
      buf->sendTail = NULL;
    } else {
      buf->sendHead = node->nextSend;
    }
    node->nextSend = NULL;
  }
  OSRestoreInterrupts(enabled);
  return node;
}

void HVQM4BufaSetSend(HVQM4Bufa* buf, HVQM4BufaNode* node)
{
  BOOL enabled = OSDisableInterrupts();
  if (buf->sendHead == NULL) {
    buf->sendHead = node;
    buf->sendTail = node;
  } else {
    buf->sendTail->nextSend = node;
    buf->sendTail = node;
  }
  node->nextSend = NULL;
  OSRestoreInterrupts(enabled);
}

HVQM4BufaNode* HVQM4BufaGetFree(HVQM4Bufa* buf)
{
  BOOL enabled = OSDisableInterrupts();
  HVQM4BufaNode* node = buf->freeHead;
  if (node != NULL) {
    if (node->nextFree == NULL) {
      buf->freeHead = NULL;
      buf->freeTail = NULL;
    } else {
      buf->freeHead = node->nextFree;
    }
    node->nextFree = NULL;
  }
  OSRestoreInterrupts(enabled);
  return node;
}

void HVQM4BufaSetFree(HVQM4Bufa* buf, HVQM4BufaNode* node)
{
  BOOL enabled = OSDisableInterrupts();
  if (buf->freeHead == NULL) {
    buf->freeHead = node;
    buf->freeTail = node;
  } else {
    buf->freeTail->nextFree = node;
    buf->freeTail = node;
  }
  node->nextFree = NULL;
  OSRestoreInterrupts(enabled);
}

s32 HVQM4BufaClose(HVQM4Bufa* buf)
{
  _mwMemFree(buf, "HVQM4Bufa.c", 90);
  return 1;
}

HVQM4Bufa* HVQM4BufaCreate(int size, unsigned int count)
{
  int nodesSize = (count * sizeof(HVQM4BufaNode) + 31) & ~31;
  int allocSize;
  HVQM4Bufa* buf;
  HVQM4BufaNode* node;
  HVQM4BufaNode* first;
  int data;
  unsigned int i;
  allocSize = ((size + 63) & ~63) * count + 64;
  allocSize += nodesSize;
  allocSize += 32;
  buf = _mwMemMalloc(gHeap, allocSize, 6, "HVQM4 buffa movie", "HVQM4Bufa.c", 59);
  if (buf == NULL) {
    OSReport("HVQM4 bufa alloc error !\n");
    return NULL;
  }
  first = (HVQM4BufaNode*)((u8*)buf + 32);
  node = first;
  data = ((int)first + nodesSize + 63) & ~63;
  for (i = 0; i < count; i++) {
    if (i == count - 1) {
      node->nextFree = NULL;
    } else {
      node->nextFree = &first[i + 1];
    }
    node->nextSend = NULL;
    node->size = 0;
    node->data = (void*)data;
    node++;
    data += size;
    data = (data + 63) & ~63;
  }
  buf->freeHead = first;
  buf->freeTail = &first[count - 1];
  buf->sendHead = NULL;
  buf->sendTail = NULL;
  return buf;
}
