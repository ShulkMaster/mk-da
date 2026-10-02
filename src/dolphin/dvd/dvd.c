/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/dvd.h>
#include <dolphin/hw_regs.h>
#include <dolphin/os.h>
#include <dolphin/os/OSThread.h>

extern BOOL autoInvalidation_8041B6C0;
extern DVDCommandBlock *executing_8041CC88;
extern volatile BOOL PauseFlag_8041CC94;
extern volatile BOOL PausingFlag_8041CC98;
extern volatile BOOL FatalErrorFlag_8041CCA0;
extern vu32 Canceling_8041CCA8;
extern DVDCBCallback CancelCallback_8041CCAC;
extern vu32 ResumeFromHere_8041CCB0;
extern volatile BOOL ResetRequired_8041CCC0;
extern DVDCommandBlock DummyCommandBlock_803EADA0;
extern OSThreadQueue __DVDThreadQueue;
extern void DCInvalidateRange(void *addr, u32 size);
extern void stateReady_8019C048(void);
extern void cbForStateMotorStopped_8019BF64(u32 cause);
extern BOOL __DVDPushWaitingQueue(s32 prio, DVDCommandBlock *block);
extern BOOL __DVDDequeueWaitingQueue(DVDCommandBlock *block);
extern void __DVDClearWaitingQueue(void);
extern DVDCommandBlock *__DVDPopWaitingQueue(void);
static void cbForCancelSync(s32 result, DVDCommandBlock *block);

static inline BOOL issueCommand(s32 prio, DVDCommandBlock *block) {
  BOOL level;
  BOOL result;

  if (autoInvalidation_8041B6C0 &&
      (block->command == 1 || block->command == 4 || block->command == 5 || block->command == 14)) {
    DCInvalidateRange(block->addr, block->length);
  }

  level = OSDisableInterrupts();

  block->state = 2;
  result = __DVDPushWaitingQueue(prio, block);

  if ((executing_8041CC88 == (DVDCommandBlock *)NULL) && (PauseFlag_8041CC94 == FALSE)) {
    stateReady_8019C048();
  }

  OSRestoreInterrupts(level);

  return result;
}

static inline void DVDPause(void) {
  BOOL level;
  level = OSDisableInterrupts();
  PauseFlag_8041CC94 = TRUE;
  if (executing_8041CC88 == (DVDCommandBlock *)NULL) {
    PausingFlag_8041CC98 = TRUE;
  }
  OSRestoreInterrupts(level);
}

static inline void DVDResume(void) {
  BOOL level;
  level = OSDisableInterrupts();
  PauseFlag_8041CC94 = FALSE;
  if (PausingFlag_8041CC98) {
    PausingFlag_8041CC98 = FALSE;
    stateReady_8019C048();
  }
  OSRestoreInterrupts(level);
}

static inline BOOL DVDCancelAllAsync(DVDCBCallback callback) {
  BOOL enabled;
  DVDCommandBlock *p;
  BOOL retVal;

  enabled = OSDisableInterrupts();
  DVDPause();

  while ((p = __DVDPopWaitingQueue()) != 0) {
    DVDCancelAsync(p, NULL);
  }

  if (executing_8041CC88)
    retVal = DVDCancelAsync(executing_8041CC88, callback);
  else {
    retVal = TRUE;
    if (callback)
      (*callback)(0, NULL);
  }

  DVDResume();
  OSRestoreInterrupts(enabled);
  return retVal;
}

BOOL DVDReadAbsAsyncPrio(DVDCommandBlock *block, void *addr, s32 length, s32 offset,
                         DVDCBCallback callback, s32 prio) {
  BOOL idle;
  block->command = 1;
  block->addr = addr;
  block->length = length;
  block->offset = offset;
  block->transferredSize = 0;
  block->callback = callback;

  idle = issueCommand(prio, block);
  return idle;
}

BOOL DVDReadAbsAsyncForBS(DVDCommandBlock *block, void *addr, s32 length, s32 offset,
                          DVDCBCallback callback) {
  BOOL idle;
  block->command = 4;
  block->addr = addr;
  block->length = length;
  block->offset = offset;
  block->transferredSize = 0;
  block->callback = callback;

  idle = issueCommand(2, block);
  return idle;
}

BOOL DVDReadDiskID(DVDCommandBlock *block, DVDDiskID *diskID, DVDCBCallback callback) {
  BOOL idle;
  block->command = 5;
  block->addr = diskID;
  block->length = sizeof(DVDDiskID);
  ;
  block->offset = 0;
  block->transferredSize = 0;
  block->callback = callback;

  idle = issueCommand(2, block);
  return idle;
}

void DVDReset(void) {
  DVDLowReset();
  __DIRegs[0] = 0x2a;
  __DIRegs[1] = __DIRegs[1];
  ResetRequired_8041CCC0 = FALSE;
  ResumeFromHere_8041CCB0 = 0;
}

s32 DVDGetCommandBlockStatus(const DVDCommandBlock *block) {
  BOOL enabled;
  s32 retVal;

  enabled = OSDisableInterrupts();

  if (block->state == 3) {
    retVal = 1;
  } else {
    retVal = block->state;
  }

  OSRestoreInterrupts(enabled);

  return retVal;
}

s32 DVDGetDriveStatus() {
  BOOL enabled;
  s32 retVal;

  enabled = OSDisableInterrupts();

  if (FatalErrorFlag_8041CCA0) {
    retVal = -1;
  } else if (PausingFlag_8041CC98) {
    retVal = 8;
  } else {
    if (executing_8041CC88 == (DVDCommandBlock *)NULL) {
      retVal = 0;
    } else if (executing_8041CC88 == &DummyCommandBlock_803EADA0) {
      retVal = 0;
    } else {
      retVal = DVDGetCommandBlockStatus(executing_8041CC88);
    }
  }

  OSRestoreInterrupts(enabled);

  return retVal;
}

BOOL DVDSetAutoInvalidation(BOOL autoInval) {
  BOOL prev;
  prev = autoInvalidation_8041B6C0;
  autoInvalidation_8041B6C0 = autoInval;
  return prev;
}

BOOL DVDCancelAsync(DVDCommandBlock *block, DVDCBCallback callback) {
  BOOL enabled;
  DVDLowCallback old;

  enabled = OSDisableInterrupts();

  switch (block->state) {
  case -1:
  case 0:
  case 10:
    if (callback)
      (*callback)(0, block);
    break;

  case 1:
    if (Canceling_8041CCA8) {
      OSRestoreInterrupts(enabled);
      return FALSE;
    }

    Canceling_8041CCA8 = TRUE;
    CancelCallback_8041CCAC = callback;
    if (block->command == 4 || block->command == 1) {
      DVDLowBreak();
    }
    break;

  case 2:
    __DVDDequeueWaitingQueue(block);
    block->state = 10;
    if (block->callback)
      (block->callback)(-3, block);
    if (callback)
      (*callback)(0, block);
    break;

  case 3:
    switch (block->command) {
    case 5:
    case 4:
    case 13:
    case 15:
      if (callback)
        (*callback)(0, block);
      break;

    default:
      if (Canceling_8041CCA8) {
        OSRestoreInterrupts(enabled);
        return FALSE;
      }
      Canceling_8041CCA8 = TRUE;
      CancelCallback_8041CCAC = callback;
      break;
    }
    break;

  case 4:
  case 5:
  case 6:
  case 7:
  case 11:
    old = DVDLowClearCallback();
    if (old != cbForStateMotorStopped_8019BF64) {
      OSRestoreInterrupts(enabled);
      return FALSE;
    }

    if (block->state == 4)
      ResumeFromHere_8041CCB0 = 3;
    if (block->state == 5)
      ResumeFromHere_8041CCB0 = 4;
    if (block->state == 6)
      ResumeFromHere_8041CCB0 = 1;
    if (block->state == 11)
      ResumeFromHere_8041CCB0 = 2;
    if (block->state == 7)
      ResumeFromHere_8041CCB0 = 7;
    block->state = 10;
    if (block->callback) {
      (block->callback)(-3, block);
    }
    if (callback) {
      (callback)(0, block);
    }
    stateReady_8019C048();
    break;
  }

  OSRestoreInterrupts(enabled);
  return TRUE;
}

s32 DVDCancel(DVDCommandBlock *block) {
  BOOL result;
  s32 state;
  u32 command;
  BOOL enabled;

  result = DVDCancelAsync(block, cbForCancelSync);

  if (result == FALSE) {
    return -1;
  }

  enabled = OSDisableInterrupts();

  for (;;) {
    state = ((volatile DVDCommandBlock *)block)->state;

    if ((state == 0) || (state == -1) || (state == 10)) {
      break;
    }

    if (state == 3) {
      command = ((volatile DVDCommandBlock *)block)->command;

      if ((command == 4) || (command == 5) || (command == 13) || (command == 15)) {
        break;
      }
    }

    OSSleepThread(&__DVDThreadQueue);
  }

  OSRestoreInterrupts(enabled);
  return 0;
}

static void cbForCancelSync(s32 result, DVDCommandBlock *block) {
  OSWakeupThread(&__DVDThreadQueue);
}

DVDDiskID *DVDGetCurrentDiskID(void) { return (DVDDiskID *)OSPhysicalToCached(0); }

BOOL DVDCheckDisk(void) {
  BOOL enabled;
  s32 retVal;
  s32 state;
  u32 coverReg;

  enabled = OSDisableInterrupts();

  if (FatalErrorFlag_8041CCA0) {
    state = -1;
  } else if (PausingFlag_8041CC98) {
    state = 8;
  } else {
    if (executing_8041CC88 == (DVDCommandBlock *)NULL) {
      state = 0;
    } else if (executing_8041CC88 == &DummyCommandBlock_803EADA0) {
      state = 0;
    } else {
      state = executing_8041CC88->state;
    }
  }

  switch (state) {
  case 1:
  case 9:
  case 10:
  case 2:
    retVal = TRUE;
    break;

  case -1:
  case 11:
  case 7:
  case 3:
  case 4:
  case 5:
  case 6:
    retVal = FALSE;
    break;

  case 0:
  case 8:
    coverReg = __DIRegs[1];
    if (((coverReg >> 2) & 1) || (coverReg & 1)) {
      retVal = FALSE;
    } else {
      retVal = TRUE;
    }
  }

  OSRestoreInterrupts(enabled);

  return retVal;
}

void __DVDPrepareResetAsync(DVDCBCallback callback) {
  BOOL enabled;

  enabled = OSDisableInterrupts();

  __DVDClearWaitingQueue();

  if (Canceling_8041CCA8) {
    CancelCallback_8041CCAC = callback;
  } else {
    if (executing_8041CC88) {
      executing_8041CC88->callback = NULL;
    }

    DVDCancelAllAsync(callback);
  }

  OSRestoreInterrupts(enabled);
}
