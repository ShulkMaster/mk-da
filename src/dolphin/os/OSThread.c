/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/os.h>

OSThread *__OSCurrentThread AT_ADDRESS(0x800000E4);
OSThreadQueue __OSActiveThreadQueue AT_ADDRESS(0x800000DC);
volatile OSContext *__OSFPUContext AT_ADDRESS(0x800000D8);
extern u8 _stack_addr[];
extern u8 _stack_end[];
static OSThreadQueue RunQueue[32];
static OSThread IdleThread;
static OSThread DefaultThread;
static OSContext IdleContext;
static volatile u32 RunQueueBits;
static volatile BOOL RunQueueHint;
static s32 Reschedule;

void OSLoadContext(OSContext *context);
void OSInitContext(OSContext *context, u32 pc, u32 sp);
void __OSUnlockAllMutex(OSThread *thread);
OSPriority __OSGetEffectivePriority(OSThread *thread);
void __OSReschedule(void);
static void UnsetRun(OSThread *thread);
static OSThread *SetEffectivePriority(OSThread *thread, OSPriority priority);
static OSThread *SelectThread(BOOL yield);

#define AddTail(queue, thread, link)                                                               \
  do {                                                                                             \
    OSThread *prev;                                                                                \
                                                                                                   \
    prev = (queue)->tail;                                                                          \
    if (prev == NULL)                                                                              \
      (queue)->head = (thread);                                                                    \
    else                                                                                           \
      prev->link.next = (thread);                                                                  \
    (thread)->link.prev = prev;                                                                    \
    (thread)->link.next = NULL;                                                                    \
    (queue)->tail = (thread);                                                                      \
  } while (0)

#define AddPrio(queue, thread, link)                                                               \
  do {                                                                                             \
    OSThread *prev, *next;                                                                         \
                                                                                                   \
    for (next = (queue)->head; next && next->priority <= thread->priority; next = next->link.next) \
      ;                                                                                            \
    if (next == NULL)                                                                              \
      AddTail(queue, thread, link);                                                                \
    else {                                                                                         \
      (thread)->link.next = next;                                                                  \
      prev = next->link.prev;                                                                      \
      next->link.prev = (thread);                                                                  \
      (thread)->link.prev = prev;                                                                  \
      if (prev == NULL)                                                                            \
        (queue)->head = (thread);                                                                  \
      else                                                                                         \
        prev->link.next = (thread);                                                                \
    }                                                                                              \
  } while (0)

#define RemoveItem(queue, thread, link)                                                            \
  do {                                                                                             \
    OSThread *next, *prev;                                                                         \
    next = (thread)->link.next;                                                                    \
    prev = (thread)->link.prev;                                                                    \
    if (next == NULL)                                                                              \
      (queue)->tail = prev;                                                                        \
    else                                                                                           \
      next->link.prev = prev;                                                                      \
    if (prev == NULL)                                                                              \
      (queue)->head = next;                                                                        \
    else                                                                                           \
      prev->link.next = next;                                                                      \
  } while (0)

#define RemoveHead(queue, thread, link)                                                            \
  do {                                                                                             \
    OSThread *__next;                                                                              \
    (thread) = (queue)->head;                                                                      \
    __next = (thread)->link.next;                                                                  \
    if (__next == NULL)                                                                            \
      (queue)->tail = NULL;                                                                        \
    else                                                                                           \
      __next->link.prev = NULL;                                                                    \
    (queue)->head = __next;                                                                        \
  } while (0)

static inline void SetRun(OSThread *thread) {
  thread->queue = &RunQueue[thread->priority];
  AddTail(thread->queue, thread, link);
  RunQueueBits |= 1u << (OS_PRIORITY_MAX - thread->priority);
  RunQueueHint = TRUE;
}

static inline void UpdatePriority(OSThread *thread) {
  OSPriority priority;

  do {
    if (0 < thread->suspend) {
      break;
    }
    priority = __OSGetEffectivePriority(thread);
    if (thread->priority == priority) {
      break;
    }
    thread = SetEffectivePriority(thread, priority);
  } while (thread);
}

static inline void __OSSwitchThread(OSThread *nextThread) {
  __OSCurrentThread = nextThread;
  OSSetCurrentContext(&nextThread->context);
  OSLoadContext(&nextThread->context);
}

static inline int __OSIsThreadActive(struct OSThread *thread) {
  struct OSThread *active;

  if (thread->state == 0) {
    return 0;
  }

  for (active = __OSActiveThreadQueue.head; active; active = active->linkActive.next) {
    if (thread == active) {
      return 1;
    }
  }
  return 0;
}

void __OSThreadInit() {
  struct OSThread *thread = &DefaultThread;
  int prio;

  thread->state = 2;
  thread->attr = 1;
  thread->priority = thread->base = 0x10;
  thread->suspend = 0;
  thread->val = (void *)-1;
  thread->mutex = 0;

  OSInitThreadQueue(&thread->queueJoin);
  thread->queueMutex.head = thread->queueMutex.tail = 0;

  __OSFPUContext = &thread->context;
  OSClearContext(&thread->context);
  OSSetCurrentContext(&thread->context);
  thread->stackBase = (unsigned char *)&_stack_addr;
  thread->stackEnd = (unsigned long *)&_stack_end;
  *(u32 *)thread->stackEnd = 0xDEADBABE;
  __OSCurrentThread = thread;
  RunQueueBits = 0;
  RunQueueHint = 0;

  for (prio = 0; prio <= 31; prio++) {
    OSInitThreadQueue(&RunQueue[prio]);
  }
  OSInitThreadQueue(&__OSActiveThreadQueue);

  AddTail(&__OSActiveThreadQueue, thread, linkActive);

  OSClearContext(&IdleContext);
  Reschedule = 0;
}

void OSInitThreadQueue(OSThreadQueue *queue) { queue->head = queue->tail = NULL; }

OSThread *OSGetCurrentThread() { return __OSCurrentThread; }

int OSIsThreadTerminated(struct OSThread *thread) {
  return (thread->state == 8 || thread->state == 0) ? 1 : 0;
}

s32 OSDisableScheduler() {
  BOOL enabled;
  s32 count;

  enabled = OSDisableInterrupts();
  count = Reschedule++;
  OSRestoreInterrupts(enabled);
  return count;
}

s32 OSEnableScheduler() {
  BOOL enabled;
  s32 count;

  enabled = OSDisableInterrupts();
  count = Reschedule--;
  OSRestoreInterrupts(enabled);
  return count;
}

#pragma dont_inline on
static void UnsetRun(OSThread *thread) {
  OSThreadQueue *queue;
  queue = thread->queue;
  RemoveItem(queue, thread, link);
  if (queue->head == 0)
    RunQueueBits &= ~(1u << (OS_PRIORITY_MAX - thread->priority));
  thread->queue = NULL;
}
#pragma dont_inline reset

OSPriority __OSGetEffectivePriority(OSThread *thread) {
  OSPriority priority;
  OSMutex *mutex;
  OSThread *blocked;

  priority = thread->base;
  for (mutex = thread->queueMutex.head; mutex; mutex = mutex->link.next) {
    blocked = mutex->queue.head;
    if (blocked && blocked->priority < priority) {
      priority = blocked->priority;
    }
  }
  return priority;
}

static OSThread *SetEffectivePriority(OSThread *thread, OSPriority priority) {
  switch (thread->state) {
  case OS_THREAD_STATE_READY:
    UnsetRun(thread);
    thread->priority = priority;
    SetRun(thread);
    break;
  case OS_THREAD_STATE_WAITING:
    RemoveItem(thread->queue, thread, link);
    thread->priority = priority;
    AddPrio(thread->queue, thread, link);
    if (thread->mutex) {
      return thread->mutex->thread;
    }
    break;
  case OS_THREAD_STATE_RUNNING:
    RunQueueHint = TRUE;
    thread->priority = priority;
    break;
  }
  return NULL;
}

void __OSPromoteThread(struct OSThread *thread, long priority) {
  while (1) {
    if (thread->suspend > 0 || thread->priority <= priority) {
      break;
    }
    thread = SetEffectivePriority(thread, priority);
    if (thread == 0) {
      break;
    }
  }
}

static OSThread *SelectThread(BOOL yield) {
  OSContext *currentContext;
  OSThread *currentThread;
  OSThread *nextThread;
  OSPriority priority;
  OSThreadQueue *queue;

  if (0 < Reschedule) {
    return 0;
  }

  currentContext = OSGetCurrentContext();
  currentThread = OSGetCurrentThread();
  if (currentContext != &currentThread->context) {
    return 0;
  }

  if (currentThread) {
    if (currentThread->state == OS_THREAD_STATE_RUNNING) {
      if (!yield) {
        priority = __cntlzw(RunQueueBits);
        if (currentThread->priority <= priority) {
          return 0;
        }
      }
      currentThread->state = OS_THREAD_STATE_READY;
      SetRun(currentThread);
    }

    if (!(currentThread->context.state & OS_CONTEXT_STATE_EXC) &&
        OSSaveContext(&currentThread->context)) {
      return 0;
    }
  }

  __OSCurrentThread = NULL;
  if (RunQueueBits == 0) {
    OSSetCurrentContext(&IdleContext);
    do {
      OSEnableInterrupts();
      while (RunQueueBits == 0)
        ;
      OSDisableInterrupts();
    } while (RunQueueBits == 0);

    OSClearContext(&IdleContext);
  }

  RunQueueHint = FALSE;

  priority = __cntlzw(RunQueueBits);
  queue = &RunQueue[priority];
  RemoveHead(queue, nextThread, link);
  if (queue->head == 0) {
    RunQueueBits &= ~(1u << (OS_PRIORITY_MAX - priority));
  }
  nextThread->queue = NULL;
  nextThread->state = OS_THREAD_STATE_RUNNING;
  __OSSwitchThread(nextThread);
  return nextThread;
}

void __OSReschedule() {
  if (!RunQueueHint) {
    return;
  }

  SelectThread(FALSE);
}

void OSYieldThread(void) {
  BOOL enabled;

  enabled = OSDisableInterrupts();
  SelectThread(TRUE);
  OSRestoreInterrupts(enabled);
}

int OSCreateThread(struct OSThread *thread, void *(*func)(void *), void *param, void *stack,
                   u32 stackSize, OSPriority priority, unsigned short attr) {
  int enabled;
  unsigned long sp;

  if ((priority < 0) || (priority > 0x1F)) {
    return 0;
  }

  thread->state = 1;
  thread->attr = attr & 1U;
  thread->base = priority;
  thread->priority = priority;
  thread->suspend = 1;
  thread->val = (void *)-1;
  thread->mutex = 0;
  OSInitThreadQueue(&thread->queueJoin);
  OSInitThreadQueue((void *)&thread->queueMutex);

  sp = (u32)stack;
  sp &= ~7;
  sp -= 8;
  ((u32 *)sp)[0] = 0;
  ((u32 *)sp)[1] = 0;
  OSInitContext(&thread->context, (u32)func, sp);
  thread->context.lr = (unsigned long)&OSExitThread;
  thread->context.gpr[3] = (unsigned long)param;
  thread->stackBase = stack;
  thread->stackEnd = (void *)((unsigned int)stack - stackSize);
  *thread->stackEnd = 0xDEADBABE;
  enabled = OSDisableInterrupts();

  AddTail(&__OSActiveThreadQueue, thread, linkActive);

  OSRestoreInterrupts(enabled);
  return 1;
}

void OSExitThread(void *val) {
  int enabled = OSDisableInterrupts();
  struct OSThread *currentThread = OSGetCurrentThread();

  OSClearContext(&currentThread->context);
  if (currentThread->attr & 1) {
    RemoveItem(&__OSActiveThreadQueue, currentThread, linkActive);
    currentThread->state = 0;
  } else {
    currentThread->state = 8;
    currentThread->val = val;
  }
  __OSUnlockAllMutex(currentThread);
  OSWakeupThread(&currentThread->queueJoin);
  RunQueueHint = 1;
  if (RunQueueHint != 0) {
    SelectThread(0);
  }

  OSRestoreInterrupts(enabled);
}

void OSCancelThread(OSThread *thread) {
  BOOL enabled;

  enabled = OSDisableInterrupts();

  switch (thread->state) {
  case OS_THREAD_STATE_READY:
    if (!(0 < thread->suspend)) {
      UnsetRun(thread);
    }
    break;
  case OS_THREAD_STATE_RUNNING:
    RunQueueHint = TRUE;
    break;
  case OS_THREAD_STATE_WAITING:
    RemoveItem(thread->queue, thread, link);
    thread->queue = NULL;
    if (!(0 < thread->suspend) && thread->mutex) {
      UpdatePriority(thread->mutex->thread);
    }
    break;
  default:
    OSRestoreInterrupts(enabled);
    return;
  }

  OSClearContext(&thread->context);
  if (thread->attr & OS_THREAD_ATTR_DETACH) {
    RemoveItem(&__OSActiveThreadQueue, thread, linkActive);
    thread->state = 0;
  } else {
    thread->state = OS_THREAD_STATE_MORIBUND;
  }

  __OSUnlockAllMutex(thread);

  OSWakeupThread(&thread->queueJoin);

  __OSReschedule();
  OSRestoreInterrupts(enabled);

  return;
}

int OSJoinThread(struct OSThread *thread, void **val) {
  int enabled = OSDisableInterrupts();

  if (!(thread->attr & 1) && (thread->state != 8) && (thread->queueJoin.head == NULL)) {
    OSSleepThread(&thread->queueJoin);
    if (__OSIsThreadActive(thread) == 0) {
      OSRestoreInterrupts(enabled);
      return 0;
    }
  }
  if (thread->state == 8) {
    if (val) {
      *val = thread->val;
    }
    RemoveItem(&__OSActiveThreadQueue, thread, linkActive);
    thread->state = 0;
    OSRestoreInterrupts(enabled);
    return 1;
  }
  OSRestoreInterrupts(enabled);
  return 0;
}

s32 OSResumeThread(OSThread *thread) {
  BOOL enabled;
  s32 suspendCount;

  enabled = OSDisableInterrupts();
  suspendCount = thread->suspend--;
  if (thread->suspend < 0) {
    thread->suspend = 0;
  } else if (thread->suspend == 0) {
    switch (thread->state) {
    case OS_THREAD_STATE_READY:
      thread->priority = __OSGetEffectivePriority(thread);
      SetRun(thread);
      break;
    case OS_THREAD_STATE_WAITING:
      RemoveItem(thread->queue, thread, link);
      thread->priority = __OSGetEffectivePriority(thread);
      AddPrio(thread->queue, thread, link);
      if (thread->mutex) {
        UpdatePriority(thread->mutex->thread);
      }
      break;
    }
    __OSReschedule();
  }
  OSRestoreInterrupts(enabled);
  return suspendCount;
}

s32 OSSuspendThread(OSThread *thread) {
  BOOL enabled;
  s32 suspendCount;

  enabled = OSDisableInterrupts();
  suspendCount = thread->suspend++;
  if (suspendCount == 0) {
    switch (thread->state) {
    case OS_THREAD_STATE_RUNNING:
      RunQueueHint = TRUE;
      thread->state = OS_THREAD_STATE_READY;
      break;
    case OS_THREAD_STATE_READY:
      UnsetRun(thread);
      break;
    case OS_THREAD_STATE_WAITING:
      RemoveItem(thread->queue, thread, link);
      thread->priority = 32;
      AddTail(thread->queue, thread, link);
      if (thread->mutex) {
        UpdatePriority(thread->mutex->thread);
      }
      break;
    }

    __OSReschedule();
  }
  OSRestoreInterrupts(enabled);
  return suspendCount;
}

void OSSleepThread(OSThreadQueue *queue) {
  BOOL enabled;
  OSThread *currentThread;

  enabled = OSDisableInterrupts();
  currentThread = OSGetCurrentThread();

  currentThread->state = OS_THREAD_STATE_WAITING;
  currentThread->queue = queue;
  AddPrio(queue, currentThread, link);
  RunQueueHint = TRUE;
  __OSReschedule();
  OSRestoreInterrupts(enabled);
}

void OSWakeupThread(OSThreadQueue *queue) {
  BOOL enabled;
  OSThread *thread;

  enabled = OSDisableInterrupts();
  while (queue->head) {
    RemoveHead(queue, thread, link);
    thread->state = OS_THREAD_STATE_READY;
    if (!(0 < thread->suspend)) {
      SetRun(thread);
    }
  }
  __OSReschedule();
  OSRestoreInterrupts(enabled);
}
