#ifndef MFL_QUEUE_H
#define MFL_QUEUE_H

#include <dolphin/types.h>

typedef struct mflQueueEntry {
  struct mflQueueEntry* previous;
  struct mflQueueEntry* next;
  s32 priority;
} mflQueueEntry;

typedef struct mflQueue {
  mflQueueEntry* head;
  mflQueueEntry* tail;
  u16 count;
  u8 allocated : 1;
  u8 unk0A : 7;
} mflQueue;

mflQueue* mflQueueAlloc(void);
mflQueue* mflQueueGetCurrent(void);
mflQueue* mflQueueSetCurrent(mflQueue* queue);
mflQueue* mflQueueNew(void);
mflQueueEntry* mflQueueGet(void);
mflQueueEntry* mflQueueRemoveFirst(void);
void mflQueueRemove(mflQueueEntry* entry);
s32 mflQueueAdd(mflQueueEntry* entry, s32 priority);
s32 mflQueueSize(void);

#endif
