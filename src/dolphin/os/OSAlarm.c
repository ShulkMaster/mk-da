/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/os/OSAlarm.h>
#include <dolphin/os/OSException.h>

typedef struct OSAlarmQueue {
  OSAlarm *head;
  OSAlarm *tail;
} OSAlarmQueue;

extern OSAlarmQueue AlarmQueue_8041CD60;
void DecrementerExceptionHandler_801A8550(__OSException exception, OSContext *context);
void PPCMtdec(u32 value);
OSTime __OSTimeToSystemTime(OSTime time);
void OSLoadContext(OSContext *context);
void __OSReschedule(void);

static inline void SetTimer(OSAlarm *alarm) {
  OSTime delta;

  delta = alarm->fire - __OSGetSystemTime();
  if (delta < 0) {
    PPCMtdec(0);
  } else if (delta < 0x80000000) {
    PPCMtdec((u32)delta);
  } else {
    PPCMtdec(0x7fffffff);
  }
}

void OSInitAlarm(void) {
  if (__OSGetExceptionHandler(8) != DecrementerExceptionHandler_801A8550) {
    AlarmQueue_8041CD60.head = AlarmQueue_8041CD60.tail = NULL;
    __OSSetExceptionHandler(8, DecrementerExceptionHandler_801A8550);
  }
}

void OSCreateAlarm(OSAlarm *alarm) { alarm->handler = 0; }

static void InsertAlarm(OSAlarm *alarm, OSTime fire, OSAlarmHandler handler) {
  OSAlarm *next;
  OSAlarm *prev;

  if (0 < alarm->period) {
    OSTime time = __OSGetSystemTime();

    fire = alarm->start;
    if (alarm->start < time) {
      fire += alarm->period * ((time - alarm->start) / alarm->period + 1);
    }
  }

  alarm->handler = handler;
  alarm->fire = fire;

  for (next = AlarmQueue_8041CD60.head; next; next = next->next) {
    if (next->fire <= fire) {
      continue;
    }

    alarm->prev = next->prev;
    next->prev = alarm;
    alarm->next = next;
    prev = alarm->prev;
    if (prev) {
      prev->next = alarm;
    } else {
      AlarmQueue_8041CD60.head = alarm;
      SetTimer(alarm);
    }
    return;
  }
  alarm->next = 0;
  prev = AlarmQueue_8041CD60.tail;
  AlarmQueue_8041CD60.tail = alarm;
  alarm->prev = prev;
  if (prev) {
    prev->next = alarm;
  } else {
    AlarmQueue_8041CD60.head = AlarmQueue_8041CD60.tail = alarm;
    SetTimer(alarm);
  }
}

void OSSetAlarm(OSAlarm *alarm, OSTime tick, OSAlarmHandler handler) {
  BOOL enabled;
  enabled = OSDisableInterrupts();
  alarm->period = 0;
  InsertAlarm(alarm, __OSGetSystemTime() + tick, handler);
  OSRestoreInterrupts(enabled);
}

void OSSetPeriodicAlarm(OSAlarm *alarm, OSTime start, OSTime period, OSAlarmHandler handler) {
  BOOL enabled;
  enabled = OSDisableInterrupts();
  alarm->period = period;
  alarm->start = __OSTimeToSystemTime(start);
  InsertAlarm(alarm, 0, handler);
  OSRestoreInterrupts(enabled);
}

void OSCancelAlarm(OSAlarm *alarm) {
  OSAlarm *next;
  BOOL enabled;

  enabled = OSDisableInterrupts();

  if (alarm->handler == 0) {
    OSRestoreInterrupts(enabled);
    return;
  }

  next = alarm->next;
  if (next == 0) {
    AlarmQueue_8041CD60.tail = alarm->prev;
  } else {
    next->prev = alarm->prev;
  }
  if (alarm->prev) {
    alarm->prev->next = next;
  } else {
    AlarmQueue_8041CD60.head = next;
    if (next) {
      SetTimer(next);
    }
  }
  alarm->handler = 0;

  OSRestoreInterrupts(enabled);
}

void DecrementerExceptionCallback_801A8320(__OSException exception, OSContext *context) {
  OSAlarm *alarm;
  OSAlarm *next;
  OSAlarmHandler handler;
  OSTime time;
  OSContext exceptionContext;
  time = __OSGetSystemTime();
  alarm = AlarmQueue_8041CD60.head;
  if (alarm == 0) {
    OSLoadContext(context);
  }

  if (time < alarm->fire) {
    SetTimer(alarm);
    OSLoadContext(context);
  }

  next = alarm->next;
  AlarmQueue_8041CD60.head = next;
  if (next == 0) {
    AlarmQueue_8041CD60.tail = 0;
  } else {
    next->prev = 0;
  }

  handler = alarm->handler;
  alarm->handler = 0;
  if (0 < alarm->period) {
    InsertAlarm(alarm, 0, handler);
  }

  if (AlarmQueue_8041CD60.head) {
    SetTimer(AlarmQueue_8041CD60.head);
  }

  OSDisableScheduler();
  OSClearContext(&exceptionContext);
  OSSetCurrentContext(&exceptionContext);
  handler(alarm, context);
  OSClearContext(&exceptionContext);
  OSSetCurrentContext(context);
  OSEnableScheduler();
  __OSReschedule();
  OSLoadContext(context);
}
