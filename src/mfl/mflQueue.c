#include <mfl/mflQueue.h>
#include <mfl/mflPools.h>
#include <msl/mslMusyXUtil.h>

static mflQueue* mflQueueCurrent;

static inline s32 mflQueueContains(mflQueueEntry* entry) {
  mflQueue* queue = mflQueueCurrent;
  mflQueueEntry* current;

  disableIRQ();
  for (current = queue->head; current != 0; current = current->next) {
    if (current == entry) {
      enableIRQ();
      return 1;
    }
  }
  enableIRQ();
  return 0;
}

void mflQueueRemove(mflQueueEntry* entry) {
  mflQueue* queue = mflQueueCurrent;

  disableIRQ();
  if (queue->head == 0 || !mflQueueContains(entry)) {
    enableIRQ();
    return;
  }
  if (queue->head == entry) {
    mflQueue* firstQueue = mflQueueCurrent;
    mflQueueEntry* first;

    if (firstQueue != 0) {
      disableIRQ();
      if (firstQueue->count == 0) {
        enableIRQ();
      } else {
        --firstQueue->count;
        first = firstQueue->head;
        firstQueue->head = first->next;
        if (firstQueue->count == 0) {
          firstQueue->tail = 0;
        } else {
          firstQueue->head->previous = 0;
        }
        first->next = 0;
        first->previous = 0;
        enableIRQ();
      }
    }
  } else {
    --queue->count;
    if (entry->previous != 0) {
      entry->previous->next = entry->next;
    }
    if (entry->next != 0) {
      entry->next->previous = entry->previous;
    }
    if (queue->tail == entry) {
      queue->tail = entry->previous;
    }
  }
  enableIRQ();
}

mflQueueEntry* mflQueueRemoveFirst(void) {
  mflQueue* queue = mflQueueCurrent;
  mflQueueEntry* entry;

  if (queue == 0) {
    return 0;
  }
  disableIRQ();
  if (queue->count == 0) {
    enableIRQ();
    return 0;
  }
  --queue->count;
  entry = queue->head;
  queue->head = entry->next;
  if (queue->count == 0) {
    queue->tail = 0;
  } else {
    queue->head->previous = 0;
  }
  entry->next = 0;
  entry->previous = 0;
  enableIRQ();
  return entry;
}

mflQueueEntry* mflQueueGet(void) {
  mflQueueEntry* entry;

  disableIRQ();
  if (mflQueueCurrent != 0) {
    entry = mflQueueCurrent->head;
  } else {
    entry = 0;
  }
  enableIRQ();
  return entry;
}

s32 mflQueueAdd(mflQueueEntry* entry, s32 priority) {
  mflQueue* queue = mflQueueCurrent;
  mflQueueEntry* current;

  if (queue == 0 || entry == 0) {
    return -1;
  }
  disableIRQ();
  ++queue->count;
  entry->priority = priority;
  if (queue->head == 0) {
    queue->tail = entry;
    queue->head = entry;
    entry->previous = 0;
    entry->next = 0;
  } else {
    current = queue->head->next;
    while (current != 0) {
      if (current->priority < entry->priority) {
        if (current->previous != 0) {
          entry->previous = current->previous;
          entry->previous->next = entry;
        } else {
          queue->head = entry;
          entry->previous = 0;
        }
        entry->next = current;
        current->previous = entry;
        goto done;
      }
      current = current->next;
    }
    entry->previous = queue->tail;
    queue->tail->next = entry;
    queue->tail = entry;
  }
done:
  enableIRQ();
  return 0;
}

s32 mflQueueSize(void) {
  s32 count;

  if (mflQueueCurrent == 0) {
    return -1;
  }
  disableIRQ();
  count = mflQueueCurrent->count;
  enableIRQ();
  return count;
}

mflQueue* mflQueueGetCurrent(void) {
  return mflQueueCurrent;
}

mflQueue* mflQueueSetCurrent(mflQueue* queue) {
  mflQueue* previous = mflQueueCurrent;
  mflQueueCurrent = queue;
  return previous;
}

mflQueue* mflQueueNew(void) {
  mflQueue* queue = mflQueueAlloc();

  if (queue != 0) {
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    queue->allocated = 1;
  }
  return queue;
}
