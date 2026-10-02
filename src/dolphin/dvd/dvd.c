/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/dvd.h>
#include <dolphin/hw_regs.h>
#include <dolphin/os.h>
#include <dolphin/os/OSThread.h>

extern OSThreadQueue __DVDThreadQueue;
extern void DCInvalidateRange(void *addr, u32 size);
extern void stateReady(void);
extern void cbForStateMotorStopped(u32 cause);
extern BOOL __DVDPushWaitingQueue(s32 prio, DVDCommandBlock *block);
extern BOOL __DVDDequeueWaitingQueue(DVDCommandBlock *block);
extern void __DVDClearWaitingQueue(void);
extern DVDCommandBlock *__DVDPopWaitingQueue(void);
static void cbForCancelSync(s32 result, DVDCommandBlock *block);

#include <dolphin/os/OSBootInfo.h>
#include <dolphin/os/OSAlarm.h>
typedef void (*stateFunc)(DVDCommandBlock *block);
typedef struct DVDBB2 {
  u32 bootFilePosition, FSTPosition, FSTLength, FSTMaxLength;
  void *FSTAddress;
  u32 userPosition, userLength, padding0;
} DVDBB2;
static u8 *tmpBuffer[32] __attribute__((aligned(32)));
static DVDCommandBlock DummyCommandBlock;
static OSAlarm ResetAlarm;
static BOOL autoInvalidation = TRUE;
static DVDCommandBlock *executing;
static DVDDiskID *currID;
static OSBootInfo *bootInfo;
static volatile BOOL PauseFlag;
static volatile BOOL PausingFlag;
static volatile BOOL AutoFinishing;
static volatile BOOL FatalErrorFlag;
static vu32 CurrCommand;
static vu32 Canceling;
static DVDCBCallback CancelCallback;
static vu32 ResumeFromHere;
static vu32 CancelLastError;
static vu32 LastError;
static vs32 NumInternalRetry;
static volatile BOOL ResetRequired;
static volatile BOOL FirstTimeInBootrom;
static BOOL DVDInitialized;
stateFunc LastState;
#define BB2 (*(DVDBB2 *)tmpBuffer)
#define CurrDiskID (*(DVDDiskID *)tmpBuffer)
extern void __fstLoad(void);
extern void __DVDInterruptHandler(__OSInterrupt interrupt, OSContext *context);
extern void __DVDStoreErrorCode(u32 error);
extern BOOL __DVDCheckWaitingQueue(void);
extern void *memcpy(void *dst, const void *src, u32 size);
extern int memcmp(const void *a, const void *b, u32 size);
#define MIN(a, b) (((a) > (b)) ? (b) : (a))
void DVDInit();
static void stateReadingFST();
static void cbForStateReadingFST(u32 intType);
static void cbForStateError(u32 intType);
static void stateTimeout();
static void stateGettingError();
static u32 CategorizeError(u32 error);
static void cbForStateGettingError(u32 intType);
static void cbForUnrecoveredError(u32 intType);
static void cbForUnrecoveredErrorRetry(u32 intType);
static void stateGoToRetry();
static void cbForStateGoToRetry(u32 intType);
static void stateCheckID();
static void stateCheckID3();
static void stateCheckID2();
static void cbForStateCheckID1(u32 intType);
static void cbForStateCheckID2(u32 intType);
static void cbForStateCheckID3(u32 intType);
static void AlarmHandler(OSAlarm *alarm, OSContext *context);
static void stateCoverClosed();
static void stateCoverClosed_CMD(DVDCommandBlock *block);
static void cbForStateCoverClosed(u32 intType);
static void stateMotorStopped(void);
static void cbForStateBusy(u32 intType);
void cbForStateMotorStopped(u32 intType);
void stateReady();
static void stateBusy(DVDCommandBlock *block);
static inline void stateError(u32 error);
static inline BOOL CheckCancel(u32 resume);
static inline void stateError(u32 error) {
  __DVDStoreErrorCode(error);
  DVDLowStopMotor(cbForStateError);
}

static inline BOOL CheckCancel(u32 resume) {
  DVDCommandBlock *finished;

  if (Canceling) {
    ResumeFromHere = resume;
    Canceling = FALSE;

    finished = executing;
    executing = &DummyCommandBlock;

    finished->state = 10;
    if (finished->callback)
      (*finished->callback)(-3, finished);
    if (CancelCallback)
      (CancelCallback)(0, finished);
    stateReady();
    return TRUE;
  }
  return FALSE;
}

void DVDInit() {
  if (DVDInitialized == 0) {
    OSInitAlarm();
    DVDInitialized = 1;
    __DVDFSInit();
    __DVDClearWaitingQueue();
    __DVDInitWA();
    bootInfo = (void *)OSPhysicalToCached(0);
    currID = &bootInfo->DVDDiskID;
    __OSSetInterruptHandler(0x15, __DVDInterruptHandler);
    __OSUnmaskInterrupts(0x400U);
    OSInitThreadQueue(&__DVDThreadQueue);
    __DIRegs[0] = 0x2A;
    __DIRegs[1] = 0;
    if (bootInfo->magic == 0xE5207C22) {
      OSReport("app booted via JTAG\n");
      OSReport("load fst\n");
      __fstLoad();
      return;
    }
    if (bootInfo->magic == 0x0D15EA5E) {
      OSReport("app booted from bootrom\n");
      return;
    }
    FirstTimeInBootrom = 1;
    OSReport("bootrom\n");
  }
}

static void stateReadingFST() {
  LastState = stateReadingFST;
  DVDLowRead(bootInfo->FSTLocation, (u32)(tmpBuffer[2] + 0x1F) & 0xFFFFFFE0,
             (u32)tmpBuffer[1], cbForStateReadingFST);
}

static void cbForStateReadingFST(u32 intType) {
  DVDCommandBlock *finished;

  if (intType == 16) {
    executing->state = -1;
    stateTimeout();
    return;
  }

  if (intType & 1) {
    NumInternalRetry = 0;
    finished = executing;
    executing = &DummyCommandBlock;
    finished->state = 0;
    if (finished->callback) {
      (finished->callback)(0, finished);
    }

    stateReady();

  } else {

    stateGettingError();
  }
}

static void cbForStateError(u32 intType) {
  DVDCommandBlock *finished;

  if (intType == 16) {
    executing->state = -1;
    stateTimeout();
    return;
  }

  FatalErrorFlag = TRUE;
  finished = executing;
  executing = &DummyCommandBlock;
  if (finished->callback) {
    (finished->callback)(-1, finished);
  }

  if (Canceling) {
    Canceling = FALSE;
    if (CancelCallback)
      (CancelCallback)(0, finished);
  }

  stateReady();

  return;
}

static void stateTimeout() {
  __DVDStoreErrorCode(0x1234568);
  DVDReset();
  cbForStateError(0);
}

static void stateGettingError() { DVDLowRequestError(cbForStateGettingError); }

static u32 CategorizeError(u32 error) {
  if (error == 0x20400) {
    LastError = error;
    return 1;
  }

  error &= 0xffffff;

  if ((error == 0x62800) || (error == 0x23a00) || (error == 0xb5a01)) {
    return 0;
  }

  ++NumInternalRetry;
  if (NumInternalRetry == 2) {
    if (error == LastError) {
      LastError = error;
      return 1;
    } else {
      LastError = error;
      return 2;
    }
  } else {
    LastError = error;

    if ((error == 0x31100) || (executing->command == 5)) {
      return 2;
    } else {
      return 3;
    }
  }
}

static void cbForStateGettingError(u32 intType) {
  u32 error;
  u32 status;
  u32 errorCategory;
  u32 resume;

  if (intType == 16) {
    executing->state = -1;
    stateTimeout();
    return;
  }

  if (intType & 2) {
    executing->state = -1;
    stateError(0x1234567);
    return;
  }

  error = __DIRegs[8];
  status = error & 0xff000000;

  errorCategory = CategorizeError(error);

  if (errorCategory == 1) {
    executing->state = -1;
    stateError(error);
    return;
  }

  if ((errorCategory == 2) || (errorCategory == 3)) {
    resume = 0;
  } else {
    if (status == 0x01000000)
      resume = 4;
    else if (status == 0x02000000)
      resume = 6;
    else if (status == 0x03000000)
      resume = 3;
    else
      resume = 5;
  }

  if (CheckCancel(resume))
    return;

  if (errorCategory == 2) {
    __DVDStoreErrorCode(error);
    stateGoToRetry();
    return;
  }

  if (errorCategory == 3) {
    if ((error & 0x00ffffff) == 0x00031100) {
      DVDLowSeek(executing->offset, cbForUnrecoveredError);
    } else {
      LastState(executing);
    }
    return;
  }

  if (status == 0x01000000) {
    executing->state = 5;
    stateMotorStopped();
    return;
  } else if (status == 0x02000000) {
    executing->state = 3;
    stateCoverClosed();
    return;
  } else if (status == 0x03000000) {
    executing->state = 4;
    stateMotorStopped();
    return;
  } else {
    executing->state = -1;
    stateError(0x1234567);
    return;
  }
}

static void cbForUnrecoveredError(u32 intType) {
  if (intType == 16) {
    executing->state = -1;
    stateTimeout();
    return;
  }

  if (intType & 1) {
    stateGoToRetry();
    return;
  }

  DVDLowRequestError(cbForUnrecoveredErrorRetry);
}

static void cbForUnrecoveredErrorRetry(u32 intType) {
  if (intType == 16) {
    executing->state = -1;
    stateTimeout();
    return;
  }
  executing->state = -1;

  if (intType & 2) {
    __DVDStoreErrorCode(0x1234567);
    DVDLowStopMotor(cbForStateError);
    return;
  }

  __DVDStoreErrorCode(__DIRegs[8]);
  DVDLowStopMotor(cbForStateError);
}

static void stateGoToRetry() { DVDLowStopMotor(cbForStateGoToRetry); }

static void cbForStateGoToRetry(u32 intType) {
  if (intType == 16) {
    executing->state = -1;
    stateTimeout();
    return;
  }

  if (intType & 2) {
    executing->state = -1;
    stateError(0x1234567);
    return;
  }

  NumInternalRetry = 0;

  if ((CurrCommand == 4) || (CurrCommand == 5) || (CurrCommand == 13) ||
      (CurrCommand == 15)) {
    ResetRequired = TRUE;
  }

  if (!CheckCancel(2)) {
    executing->state = 11;
    stateMotorStopped();
  }
}

static void stateCheckID() {
  switch (CurrCommand) {
  case 3:
    if (memcmp(&CurrDiskID, executing->id, 0x1C)) {
      DVDLowStopMotor(cbForStateCheckID1);
    } else {
      memcpy(currID, &CurrDiskID, sizeof(DVDDiskID));
      executing->state = 1;
      DCInvalidateRange(&BB2, sizeof(DVDBB2));
      LastState = stateCheckID2;
      stateCheckID2(executing);
    }
    break;
  default:
    if (memcmp(&CurrDiskID, currID, sizeof(DVDDiskID))) {
      DVDLowStopMotor(cbForStateCheckID1);
    } else {
      LastState = stateCheckID3;
      stateCheckID3(executing);
    }
    break;
  }
}

static void stateCheckID3() {
  DVDLowAudioBufferConfig(currID->streaming, 10, cbForStateCheckID3);
}

static void stateCheckID2() {
  DVDLowRead(&BB2, OSRoundUp32B(sizeof(BB2)), 0x420, cbForStateCheckID2);
}

static void cbForStateCheckID1(u32 intType) {
  if (intType == 16) {
    executing->state = -1;
    stateTimeout();
    return;
  }

  if (intType & 2) {
    executing->state = -1;
    stateError(0x1234567);
    return;
  }

  NumInternalRetry = 0;

  if (!CheckCancel(1)) {
    executing->state = 6;
    stateMotorStopped();
  }
}

static void cbForStateCheckID2(u32 intType) {
  if (intType == 16) {
    executing->state = -1;
    stateTimeout();
    return;
  }

  if (intType & 1) {

    NumInternalRetry = 0;

    stateReadingFST();

  } else {

    stateGettingError();
  }
}

static void cbForStateCheckID3(u32 intType) {
  if (intType == 16) {
    executing->state = -1;
    stateTimeout();
    return;
  }

  if (intType & 1) {

    NumInternalRetry = 0;

    if (!CheckCancel(0)) {
      executing->state = 1;
      stateBusy(executing);
    }
  } else {
    stateGettingError();
  }
}

static void AlarmHandler(OSAlarm *alarm, OSContext *context) {
  DVDReset();
  DCInvalidateRange(&CurrDiskID, sizeof(DVDDiskID));
  LastState = stateCoverClosed_CMD;
  stateCoverClosed_CMD(executing);
}

static void stateCoverClosed() {
  DVDCommandBlock *finished;

  switch (CurrCommand) {
  case 5:
  case 4:
  case 13:
  case 15:
    __DVDClearWaitingQueue();
    finished = executing;
    executing = &DummyCommandBlock;
    if (finished->callback) {
      (finished->callback)(-4, finished);
    }
    stateReady();
    break;

  default:
    DVDReset();
    OSCreateAlarm(&ResetAlarm);
    OSSetAlarm(&ResetAlarm, OSMillisecondsToTicks(1150), AlarmHandler);
    break;
  }
}

static void stateCoverClosed_CMD(DVDCommandBlock *block) {
  DVDLowReadDiskID(&CurrDiskID, cbForStateCoverClosed);
}

static void cbForStateCoverClosed(u32 intType) {
  if (intType == 16) {
    executing->state = -1;
    stateTimeout();
    return;
  }

  if (intType & 1) {
    NumInternalRetry = 0;
    stateCheckID();
  } else {
    stateGettingError();
  }
}

static void stateMotorStopped(void) { DVDLowWaitCoverClose(cbForStateMotorStopped); }

static void cbForStateMotorStopped(u32 intType) {
  __DIRegs[1] = 0;
  executing->state = 3;
  stateCoverClosed();
}

void stateReady() {
  DVDCommandBlock *finished;

  if (!__DVDCheckWaitingQueue()) {
    executing = (DVDCommandBlock *)NULL;
    return;
  }

  if (PauseFlag) {
    PausingFlag = TRUE;
    executing = (DVDCommandBlock *)NULL;
    return;
  }

  executing = __DVDPopWaitingQueue();

  if (FatalErrorFlag) {
    executing->state = -1;
    finished = executing;
    executing = &DummyCommandBlock;
    if (finished->callback) {
      (finished->callback)(-1, finished);
    }
    stateReady();
    return;
  }

  CurrCommand = executing->command;

  if (ResumeFromHere) {
    switch (ResumeFromHere) {
    case 1:
      executing->state = 6;
      stateMotorStopped();
      break;
    case 2:
      executing->state = 11;
      stateMotorStopped();
      break;

    case 3:
      executing->state = 4;
      stateMotorStopped();
      break;

    case 7:
      executing->state = 7;
      stateMotorStopped();
      break;
    case 4:
      executing->state = 5;
      stateMotorStopped();
      break;
    case 6:
      executing->state = 3;
      stateCoverClosed();
      break;

    case 5:
      executing->state = -1;
      stateError(CancelLastError);
      break;
    }

    ResumeFromHere = 0;
  } else {
    executing->state = 1;
    stateBusy(executing);
  }
}

void stateBusy(DVDCommandBlock* block)
{
  LastState = stateBusy;
  switch (block->command) {
  case 5:
    __DIRegs[1]             = __DIRegs[1];
    block->currTransferSize = sizeof(DVDDiskID);
    DVDLowReadDiskID(block->addr, cbForStateBusy);
    break;
  case 1:
  case 4:
    __DIRegs[1] = __DIRegs[1];
    block->currTransferSize
      = block->length - block->transferredSize > 0x80000
                  ? 0x80000
                  : block->length - block->transferredSize;
    DVDLowRead((void*)((u8*)block->addr + block->transferredSize),
                   block->currTransferSize,
                   block->offset + block->transferredSize, cbForStateBusy);
    break;
  case 2:
    __DIRegs[1] = __DIRegs[1];
    DVDLowSeek(block->offset, cbForStateBusy);
    break;
  case 3:
    DVDLowStopMotor(cbForStateBusy);
    break;
  case 15:
    DVDLowStopMotor(cbForStateBusy);
    break;
  case 6:
    __DIRegs[1] = __DIRegs[1];
    if (AutoFinishing) {
      executing->currTransferSize = 0;
      DVDLowRequestAudioStatus(0, cbForStateBusy);
    } else {
      executing->currTransferSize = 1;
      DVDLowAudioStream(0, block->length, block->offset, cbForStateBusy);
    }
    break;
  case 7:
    __DIRegs[1] = __DIRegs[1];
    DVDLowAudioStream(0x10000, 0, 0, cbForStateBusy);
    break;
  case 8:
    __DIRegs[1]   = __DIRegs[1];
    AutoFinishing = TRUE;
    DVDLowAudioStream(0, 0, 0, cbForStateBusy);
    break;
  case 9:
    __DIRegs[1] = __DIRegs[1];
    DVDLowRequestAudioStatus(0, cbForStateBusy);
    break;
  case 10:
    __DIRegs[1] = __DIRegs[1];
    DVDLowRequestAudioStatus(0x10000, cbForStateBusy);
    break;
  case 11:
    __DIRegs[1] = __DIRegs[1];
    DVDLowRequestAudioStatus(0x20000, cbForStateBusy);
    break;
  case 12:
    __DIRegs[1] = __DIRegs[1];
    DVDLowRequestAudioStatus(0x30000, cbForStateBusy);
    break;
  case 13:
    __DIRegs[1] = __DIRegs[1];
    DVDLowAudioBufferConfig(block->offset, block->length, cbForStateBusy);
    break;
  case 14:
    __DIRegs[1]             = __DIRegs[1];
    block->currTransferSize = sizeof(DVDDriveInfo);
    DVDLowInquiry(block->addr, cbForStateBusy);
    break;
  default:
    break;
  }
}

static void cbForStateBusy(u32 intType) {
  DVDCommandBlock *finished;

  if (intType == 16) {
    executing->state = -1;
    stateTimeout();
    return;
  }

  if ((CurrCommand == 3) || (CurrCommand == 15)) {
    if (intType & 2) {
      executing->state = -1;
      stateError(0x1234567);
      return;
    }

    NumInternalRetry = 0;

    if (CurrCommand == 15) {
      ResetRequired = TRUE;
    }

    if (CheckCancel(7)) {
      return;
    }

    executing->state = 7;
    stateMotorStopped();
    return;
  }

  if ((CurrCommand == 1 || CurrCommand == 4 || CurrCommand == 5 ||
       CurrCommand == 14)) {
    executing->transferredSize += executing->currTransferSize - __DIRegs[6];
  }

  if (intType & 8) {
    Canceling = FALSE;
    finished = executing;
    executing = &DummyCommandBlock;

    finished->state = 10;
    if (finished->callback)
      (*finished->callback)(-3, finished);
    if (CancelCallback)
      (CancelCallback)(0, finished);
    stateReady();

    return;
  }

  if (intType & 1) {
    NumInternalRetry = 0;

    if (CheckCancel(0))
      return;

    if ((CurrCommand == 1 || CurrCommand == 4 || CurrCommand == 5 ||
         CurrCommand == 14)) {
      if (executing->transferredSize != executing->length) {
        stateBusy(executing);
        return;
      }

      finished = executing;
      executing = &DummyCommandBlock;

      finished->state = 0;
      if (finished->callback) {
        (finished->callback)((s32)finished->transferredSize, finished);
      }
      stateReady();
    } else if ((CurrCommand == 9 || CurrCommand == 10 ||
                CurrCommand == 11 || CurrCommand == 12)) {
      s32 result;

      if ((CurrCommand == 11) || (CurrCommand == 10)) {
        result = (s32)(__DIRegs[8] << 2);
      } else {
        result = (s32)__DIRegs[8];
      }
      finished = executing;
      executing = &DummyCommandBlock;

      finished->state = 0;
      if (finished->callback) {
        (finished->callback)(result, finished);
      }
      stateReady();
    } else if (CurrCommand == 6) {
      if (executing->currTransferSize == 0) {
        if (__DIRegs[8] & 1) {
          finished = executing;
          executing = &DummyCommandBlock;

          finished->state = 9;
          if (finished->callback) {
            (finished->callback)(-2, finished);
          }
          stateReady();
        } else {
          AutoFinishing = FALSE;
          executing->currTransferSize = 1;
          DVDLowAudioStream(0, executing->length, executing->offset,
                            cbForStateBusy);
        }
      } else {
        finished = executing;
        executing = &DummyCommandBlock;

        finished->state = 0;
        if (finished->callback) {
          (finished->callback)(0, finished);
        }
        stateReady();
      }
    } else {
      finished = executing;
      executing = &DummyCommandBlock;

      finished->state = 0;
      if (finished->callback) {
        (finished->callback)(0, finished);
      }
      stateReady();
    }
  } else {
    if (CurrCommand == 14) {
      executing->state = -1;
      stateError(0x01234567);
      return;
    }

    if ((CurrCommand == 1 || CurrCommand == 4 || CurrCommand == 5 ||
         CurrCommand == 14) &&
        (executing->transferredSize == executing->length)) {
      if (CheckCancel(0)) {
        return;
      }
      finished = executing;
      executing = &DummyCommandBlock;

      finished->state = 0;
      if (finished->callback) {
        (finished->callback)((s32)finished->transferredSize, finished);
      }
      stateReady();
      return;
    }

    stateGettingError();
  }
}

static inline BOOL issueCommand(s32 prio, DVDCommandBlock *block) {
  BOOL level;
  BOOL result;

  if (autoInvalidation &&
      (block->command == 1 || block->command == 4 || block->command == 5 || block->command == 14)) {
    DCInvalidateRange(block->addr, block->length);
  }

  level = OSDisableInterrupts();

  block->state = 2;
  result = __DVDPushWaitingQueue(prio, block);

  if ((executing == (DVDCommandBlock *)NULL) && (PauseFlag == FALSE)) {
    stateReady();
  }

  OSRestoreInterrupts(level);

  return result;
}

static inline void DVDPause(void) {
  BOOL level;
  level = OSDisableInterrupts();
  PauseFlag = TRUE;
  if (executing == (DVDCommandBlock *)NULL) {
    PausingFlag = TRUE;
  }
  OSRestoreInterrupts(level);
}

static inline void DVDResume(void) {
  BOOL level;
  level = OSDisableInterrupts();
  PauseFlag = FALSE;
  if (PausingFlag) {
    PausingFlag = FALSE;
    stateReady();
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

  if (executing)
    retVal = DVDCancelAsync(executing, callback);
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
  ResetRequired = FALSE;
  ResumeFromHere = 0;
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

  if (FatalErrorFlag) {
    retVal = -1;
  } else if (PausingFlag) {
    retVal = 8;
  } else {
    if (executing == (DVDCommandBlock *)NULL) {
      retVal = 0;
    } else if (executing == &DummyCommandBlock) {
      retVal = 0;
    } else {
      retVal = DVDGetCommandBlockStatus(executing);
    }
  }

  OSRestoreInterrupts(enabled);

  return retVal;
}

BOOL DVDSetAutoInvalidation(BOOL autoInval) {
  BOOL prev;
  prev = autoInvalidation;
  autoInvalidation = autoInval;
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
    if (Canceling) {
      OSRestoreInterrupts(enabled);
      return FALSE;
    }

    Canceling = TRUE;
    CancelCallback = callback;
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
      if (Canceling) {
        OSRestoreInterrupts(enabled);
        return FALSE;
      }
      Canceling = TRUE;
      CancelCallback = callback;
      break;
    }
    break;

  case 4:
  case 5:
  case 6:
  case 7:
  case 11:
    old = DVDLowClearCallback();
    if (old != cbForStateMotorStopped) {
      OSRestoreInterrupts(enabled);
      return FALSE;
    }

    if (block->state == 4)
      ResumeFromHere = 3;
    if (block->state == 5)
      ResumeFromHere = 4;
    if (block->state == 6)
      ResumeFromHere = 1;
    if (block->state == 11)
      ResumeFromHere = 2;
    if (block->state == 7)
      ResumeFromHere = 7;
    block->state = 10;
    if (block->callback) {
      (block->callback)(-3, block);
    }
    if (callback) {
      (callback)(0, block);
    }
    stateReady();
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

  if (FatalErrorFlag) {
    state = -1;
  } else if (PausingFlag) {
    state = 8;
  } else {
    if (executing == (DVDCommandBlock *)NULL) {
      state = 0;
    } else if (executing == &DummyCommandBlock) {
      state = 0;
    } else {
      state = executing->state;
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

  if (Canceling) {
    CancelCallback = callback;
  } else {
    if (executing) {
      executing->callback = NULL;
    }

    DVDCancelAllAsync(callback);
  }

  OSRestoreInterrupts(enabled);
}
