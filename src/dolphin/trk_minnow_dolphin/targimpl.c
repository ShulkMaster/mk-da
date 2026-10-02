#include <dolphin/asm_sequences.inc>
#include <dolphin/trk.h>

typedef unsigned long size_t;
typedef unsigned char u128[16];
typedef unsigned int uint;

enum {
  DS_InvalidMemory = 0x700,
  DS_InvalidRegister = 0x701,
  DS_CWDSException = 0x702,
  DS_UnsupportedError = 0x703,
};

enum {
  DS_IONoError = 0,
  DS_IOError = 1,
};

enum {
  DSMSG_NotifyStopped = 0x90,
  DSMSG_WriteFile = 0xD0,
  DSMSG_ReadFile = 0xD1,
  DSMSG_OpenFile = 0xD2,
  DSMSG_CloseFile = 0xD3,
  DSMSG_PositionFile = 0xD4,
};

enum {
  DSSTEP_IntoCount = 0x0,
  DSSTEP_IntoRange = 0x1,
  DSSTEP_OverCount = 0x10,
  DSSTEP_OverRange = 0x11,
};

enum {
  NUBEVENT_Breakpoint = 3,
  NUBEVENT_Exception = 4,
  NUBEVENT_Support = 5,
};

enum {
  VALIDMEM_Readable = 0,
  VALIDMEM_Writeable = 1,
};

enum {
  MEMACCESS_UserMemory = 0,
};

#define PPC_Trace 0xD00
#define SPR_GQR0 912
#define SPR_HID2 920
#define SPR_FPECR 1022

typedef struct memRange {
  u8* start;
  u8* end;
  BOOL readable;
  BOOL writeable;
} memRange;

const memRange gTRKMemMap[1] = { { (u8*)0, (u8*)-1, TRUE, TRUE } };

typedef struct DSVersions {
  u8 kernelMajor;
  u8 kernelMinor;
  u8 protocolMajor;
  u8 protocolMinor;
} DSVersions;

typedef struct DSCPUType {
  u8 cpuMajor;
  u8 cpuMinor;
  u8 bigEndian;
  u8 defaultTypeSize;
  u8 fpTypeSize;
  u8 extended1TypeSize;
  u8 extended2TypeSize;
} DSCPUType;

typedef struct StopInfo_PPC {
  u32 PC;
  u32 PCInstruction;
  u16 exceptionID;
} StopInfo_PPC;

typedef struct TRKExceptionStatus {
  StopInfo_PPC exceptionInfo;
  u8 inTRK;
  u8 exceptionDetected;
} TRKExceptionStatus;

typedef struct TRKStepStatus {
  BOOL active;
  u8 type;
  u32 count;
  u32 rangeStart;
  u32 rangeEnd;
} TRKStepStatus;

typedef struct ProcessorRestoreFlags_PPC {
  u8 TBR;
  u8 DEC;
  u8 linker_padding[0x9 - 0x2];
} ProcessorRestoreFlags_PPC;

ProcessorRestoreFlags_PPC gTRKRestoreFlags = { FALSE, FALSE };

static TRKExceptionStatus gTRKExceptionStatus = { { 0, 0, 0 }, TRUE, 0 };

static TRKStepStatus gTRKStepStatus = { FALSE, DSSTEP_IntoCount, 0, 0 };

extern void TRKUARTInterruptHandler(void);
extern void TRKSaveExtended1Block(void);
extern void TRKRestoreExtended1Block(void);
extern u8 TRKTargetCPUMinorType(void);
extern void TRK_flush_cache(void* addr, int nBytes);
extern u32 TRKTargetTranslate(u32 addr);
extern DSError TRKDoNotifyStopped(MessageCommandID cmd);
extern DSError TRKSuppAccessFile(u32 file_handle, u8* data, size_t* count, u8* io_result,
                                 BOOL need_reply, BOOL read);
extern DSError HandleOpenFileSupportRequest(const char* path, u8 replyError, u32* fileHandle,
                                            u8* ioResult);
extern DSError HandleCloseFileSupportRequest(int replyError, u8* ioResult);
extern DSError HandlePositionFileSupportRequest(u32 replyErr, u32* filePosition,
                                                u8 positionType, u8* ioResult);

static void TRKExceptionHandler(u16);
void TRKInterruptHandlerEnableInterrupts(void);
void TRKPostInterruptEvent(void);
static DSError TRKTargetReadInstruction(void* data, u32 start);
static inline DSError TRKPPCAccessSPR(void* srcDestPtr, u32 spr, BOOL read);
static inline DSError TRKPPCAccessPairedSingleRegister(void* srcDestPtr, u32 psr, BOOL read);
static inline DSError TRKPPCAccessSpecialReg(void* srcDestPtr, u32* instructionData, BOOL read);
DSError TRKPPCAccessFPRegister(void* srcDestPtr, u32 fpr, BOOL read);

static u16 TRK_saved_exceptionID;
extern u128 TRKvalue128_temp;
extern Default_PPC gTRKSaveState;

// Instruction macros
#define INSTR_NOP 0x60000000
#define INSTR_BLR 0x4E800020
#define INSTR_PSQ_ST(psr, offset, rDest, w, gqr) \
  (0xF0000000 | (psr << 21) | (rDest << 16) | (w << 15) | (gqr << 12) | offset)
#define INSTR_PSQ_L(psr, offset, rSrc, w, gqr) \
  (0xE0000000 | (psr << 21) | (rSrc << 16) | (w << 15) | (gqr << 12) | offset)
#define INSTR_STW(rSrc, offset, rDest) (0x90000000 | (rSrc << 21) | (rDest << 16) | offset)
#define INSTR_LWZ(rDest, offset, rSrc) (0x80000000 | (rDest << 21) | (rSrc << 16) | offset)
#define INSTR_STFD(fprSrc, offset, rDest) (0xD8000000 | (fprSrc << 21) | (rDest << 16) | offset)
#define INSTR_LFD(fprDest, offset, rSrc) (0xC8000000 | (fprDest << 21) | (rSrc << 16) | offset)
#define INSTR_MFSPR(rDest, spr) \
  (0x7C000000 | (rDest << 21) | ((spr & 0xFE0) << 6) | ((spr & 0x1F) << 16) | 0x2A6)
#define INSTR_MTSPR(spr, rSrc) \
  (0x7C000000 | (rSrc << 21) | ((spr & 0xFE0) << 6) | ((spr & 0x1F) << 16) | 0x3A6)

#define DSFetch_u32(_p_) (*((u32*)_p_))
#define DSFetch_u64(_p_) (*((u64*)_p_))

static void TRK_ppc_memcpy(register void* dest, register const void* src, register int n,
                           register u32 destMsr, register u32 srcMsr);

asm u32 __TRK_get_MSR(void) { SEQ___TRK_get_MSR(); }

asm void __TRK_set_MSR(register u32 v) { SEQ___TRK_set_MSR(); }

DSError TRKValidMemory32(const void* addr, size_t length, int readWriteable);
DSError TRKTargetAccessMemory(void* data, u32 start, size_t* length, int accessOptions,
                              BOOL read);
DSError TRKTargetAccessDefault(u32 firstRegister, u32 lastRegister, MessageBuffer* b,
                               size_t* registersLengthPtr, BOOL read);
DSError TRKTargetAccessFP(u32 firstRegister, u32 lastRegister, MessageBuffer* b,
                          size_t* registersLengthPtr, BOOL read);
DSError TRKTargetAccessExtended1(u32 firstRegister, u32 lastRegister, MessageBuffer* b,
                                 size_t* registersLengthPtr, BOOL read);
DSError TRKTargetAccessExtended2(u32 firstRegister, u32 lastRegister, MessageBuffer* b,
                                 size_t* registerStorageSize, BOOL read);
DSError TRKTargetVersions(DSVersions* versions);
DSError TRKTargetSupportMask(u8 mask[32]);
DSError TRKTargetCPUType(DSCPUType* cpuType);
DSError TRKTargetSingleStep(u32 count, BOOL stepOver);
DSError TRKTargetStepOutOfRange(u32 rangeStart, u32 rangeEnd, BOOL stepOver);
u32 TRKTargetGetPC(void);
DSError TRKTargetFlushCache(u8 options, void* start, void* end);
BOOL TRKTargetStop();
void TRKTargetSetStopped(uint stopped);

DSError TRKValidMemory32(const void* addr, size_t length, int readWriteable) {
  DSError err = DS_InvalidMemory; /* assume range is invalid */

  const u8* start;
  const u8* end;

  s32 i;

  /*
  ** Get start and end addresses for the memory range and
  ** verify that they are reasonable.
  */

  start = (const u8*)addr;
  end = ((const u8*)addr + (length - 1));

  if (end < start)
    return DS_InvalidMemory;

  /*
  ** Iterate through the gTRKMemMap array to determine if the requested
  ** range falls within the valid ranges in the map.
  */

  for (i = 0; (i < (s32)(sizeof(gTRKMemMap) / sizeof(memRange))); i++) {
    /*
    ** If the requested range is not completely above
    ** the valid range AND it is not completely below
    ** the valid range then it must overlap somewhere.
    ** If the requested range overlaps with one of the
    ** valid ranges, do some additional checking.
    **
    */

    if ((start <= (const u8*)gTRKMemMap[i].end) && (end >= (const u8*)gTRKMemMap[i].start)) {
      /*
      ** First, verify that the read/write attributes are
      ** acceptable.  If so, then recursively check any
      ** part of the requested range that falls before or
      ** after the valid range.
      */

      if ((((u8)readWriteable == VALIDMEM_Readable) && !gTRKMemMap[i].readable) ||
          (((u8)readWriteable == VALIDMEM_Writeable) && !gTRKMemMap[i].writeable)) {
        err = DS_InvalidMemory;
      } else {
        err = DS_NoError;

        /*
        ** If a portion of the requested range falls before
        ** the current valid range, then recursively
        ** check it.
        */

        if (start < (const u8*)gTRKMemMap[i].start)
          err = TRKValidMemory32(start, (u32)((const u8*)gTRKMemMap[i].start - start),
                                 readWriteable);

        /*
        ** If a portion of the requested range falls after
        ** the current valid range, then recursively
        ** check it.
        ** Note: Only do this step if the previous check
        ** did not detect invalid access.
        */

        if ((err == DS_NoError) && (end > (const u8*)gTRKMemMap[i].end))
          err = TRKValidMemory32((const u8*)gTRKMemMap[i].end,
                                 (u32)(end - (const u8*)gTRKMemMap[i].end), readWriteable);
      }

      break;
    }
  }

  return err;
}

asm static void TRK_ppc_memcpy(register void* dest, register const void* src, register int n,
                               register u32 destMsr, register u32 srcMsr) {
  SEQ_TRK_ppc_memcpy();
}

DSError TRKTargetAccessMemory(void* data, u32 start, size_t* length, int accessOptions,
                              BOOL read) {
  DSError error;
  u32 savedMsr;
  void* addr;
  u32 targetMsr;
  TRKExceptionStatus tempExceptionStatus = gTRKExceptionStatus;
  gTRKExceptionStatus.exceptionDetected = FALSE;

  addr = (void*)TRKTargetTranslate(start);
  error = TRKValidMemory32(addr, *length, read == FALSE);

  if (error != DS_NoError) {
    *length = 0;
  } else {
    savedMsr = __TRK_get_MSR();
    targetMsr = savedMsr | gTRKCPUState.Extended1.MSR & 0x10;

    if (read) {
      TRK_ppc_memcpy(data, addr, *length, savedMsr, targetMsr);
    } else {
      TRK_ppc_memcpy(addr, data, *length, targetMsr, savedMsr);
      TRK_flush_cache(addr, *length);
      if ((void*)start != addr) {
        TRK_flush_cache((void*)start, *length);
      }
    }
  }

  if (gTRKExceptionStatus.exceptionDetected) {
    *length = 0;
    error = DS_CWDSException;
  }

  gTRKExceptionStatus = tempExceptionStatus;
  return error;
}

static inline DSError TRKTargetReadInstruction(void* data, u32 start) {
  DSError error;
  size_t registersLength = 4;

  error = TRKTargetAccessMemory(data, start, &registersLength, MEMACCESS_UserMemory, TRUE);

  if (error == DS_NoError && registersLength != 4) {
    error = DS_InvalidMemory;
  }

  return error;
}

DSError TRKTargetAccessDefault(u32 firstRegister, u32 lastRegister, MessageBuffer* b,
                               size_t* registersLengthPtr, BOOL read) {
  DSError error;
  u32 count;
  u32* data;
  TRKExceptionStatus tempExceptionStatus;

  if (lastRegister > 0x24) {
    return DS_InvalidRegister;
  }

  tempExceptionStatus = gTRKExceptionStatus;
  gTRKExceptionStatus.exceptionDetected = FALSE;

  data = gTRKCPUState.Default.GPR + firstRegister;
  count = (lastRegister - firstRegister) + 1;
  *registersLengthPtr = count * sizeof(u32);

  if (read) {
    error = TRKAppendBuffer_ui32(b, data, count);
  } else {
    error = TRKReadBuffer_ui32(b, data, count);
  }

  if (gTRKExceptionStatus.exceptionDetected) {
    *registersLengthPtr = 0;
    error = DS_CWDSException;
  }

  gTRKExceptionStatus = tempExceptionStatus;
  return error;
}

DSError TRKTargetAccessFP(u32 firstRegister, u32 lastRegister, MessageBuffer* b,
                          size_t* registersLengthPtr, BOOL read) {
  u64 temp;
  DSError error;
  TRKExceptionStatus tempExceptionStatus;
  u32 current;

  if (lastRegister > 0x21) {
    return DS_InvalidRegister;
  }

  tempExceptionStatus = gTRKExceptionStatus;
  gTRKExceptionStatus.exceptionDetected = FALSE;

  __TRK_set_MSR(__TRK_get_MSR() | 0x2000);

  *registersLengthPtr = 0;
  error = DS_NoError;

  for (current = firstRegister; (current <= lastRegister) && (error == DS_NoError);
       current++, *registersLengthPtr += sizeof(f64)) {
    if (read) {
      TRKPPCAccessFPRegister(&temp, current, read);
      error = TRKAppendBuffer1_ui64(b, temp);
    } else {
      TRKReadBuffer1_ui64(b, &temp);
      error = TRKPPCAccessFPRegister(&temp, current, read);
    }
  }

  if (gTRKExceptionStatus.exceptionDetected) {
    *registersLengthPtr = 0;
    error = DS_CWDSException;
  }

  gTRKExceptionStatus = tempExceptionStatus;
  return error;
}

DSError TRKTargetAccessExtended1(u32 firstRegister, u32 lastRegister, MessageBuffer* b,
                                 size_t* registersLengthPtr, BOOL read) {
  TRKExceptionStatus tempExceptionStatus;
  int error;
  u32* data;
  int count;

  if (lastRegister > 0x60) {
    return DS_InvalidRegister;
  }

  tempExceptionStatus = gTRKExceptionStatus;
  gTRKExceptionStatus.exceptionDetected = FALSE;

  *registersLengthPtr = 0;

  if (firstRegister <= lastRegister) {
    data = (u32*)&gTRKCPUState.Extended1 + firstRegister;
    count = lastRegister - firstRegister + 1;
    *registersLengthPtr += count * sizeof(u32);

    if (read) {
      error = TRKAppendBuffer_ui32(b, data, count);
    } else {

      if (data <= &gTRKCPUState.Extended1.TBU && (data + count - 1) >= &gTRKCPUState.Extended1.TBL) {
        gTRKRestoreFlags.TBR = 1;
      }

      if (data <= &gTRKCPUState.Extended1.DEC && (data + count - 1) >= &gTRKCPUState.Extended1.DEC) {
        gTRKRestoreFlags.DEC = 1;
      }
      error = TRKReadBuffer_ui32(b, data, count);
    }
  }
  if (gTRKExceptionStatus.exceptionDetected) {
    *registersLengthPtr = 0;
    error = DS_CWDSException;
  }

  gTRKExceptionStatus = tempExceptionStatus;
  return error;
}

DSError TRKTargetAccessExtended2(u32 firstRegister, u32 lastRegister, MessageBuffer* b,
                                 size_t* registerStorageSize, BOOL read) {
  TRKExceptionStatus savedException;
  u32 i;
  u32 value_buf0[1];
  u32 value_buf[2];
  DSError err;

  if (lastRegister > 0x1f)
    return DS_InvalidRegister;

  /*
  ** Save any existing exception status and clear the exception flag.
  ** This allows detection of exceptions that occur ONLY within this
  ** function.
  */

  savedException = gTRKExceptionStatus;
  gTRKExceptionStatus.exceptionDetected = FALSE;

  TRKPPCAccessSPR(value_buf0, SPR_HID2, TRUE);

  value_buf0[0] |= 0xA0000000;
  TRKPPCAccessSPR(value_buf0, SPR_HID2, FALSE);

  value_buf0[0] = 0;
  TRKPPCAccessSPR(value_buf0, SPR_GQR0, FALSE);

  *registerStorageSize = 0;
  err = DS_NoError;

  for (i = firstRegister; (i <= lastRegister) && (err == DS_NoError); i++) {

    if (read) {
      err = TRKPPCAccessPairedSingleRegister((u64*)value_buf, i, read);
      err = TRKAppendBuffer1_ui64(b, *(u64*)value_buf);
    } else {
      err = TRKReadBuffer1_ui64(b, (u64*)value_buf);
      err = TRKPPCAccessPairedSingleRegister((u64*)value_buf, i, read);
    }

    *registerStorageSize += sizeof(u64);
  }

  if (gTRKExceptionStatus.exceptionDetected) {
    *registerStorageSize = 0;
    err = DS_CWDSException;
  }

  gTRKExceptionStatus = savedException;

  return err;
}

DSError TRKTargetVersions(DSVersions* versions) {
  versions->kernelMajor = 0;
  versions->kernelMinor = 10;
  versions->protocolMajor = 1;
  versions->protocolMinor = 10;
  return DS_NoError;
}

DSError TRKTargetSupportMask(u8 mask[32]) {
  mask[0] = 0x7A;
  mask[1] = 0;
  mask[2] = 0x4F;
  mask[3] = 0x07;
  mask[4] = 0;
  mask[5] = 0;
  mask[6] = 0;
  mask[7] = 0;
  mask[8] = 0;
  mask[9] = 0;
  mask[10] = 0;
  mask[11] = 0;
  mask[12] = 0;
  mask[13] = 0;
  mask[14] = 0;
  mask[15] = 0;
  mask[16] = 0x01;
  mask[17] = 0;
  mask[18] = 0x03;
  mask[19] = 0;
  mask[20] = 0;
  mask[21] = 0;
  mask[22] = 0;
  mask[23] = 0;
  mask[24] = 0;
  mask[25] = 0;
  mask[26] = 0x03;
  mask[27] = 0;
  mask[28] = 0;
  mask[29] = 0;
  mask[30] = 0;
  mask[31] = 0x80;
  return DS_NoError;
}

DSError TRKTargetCPUType(DSCPUType* cpuType) {
  cpuType->cpuMajor = 0;
  cpuType->cpuMinor = TRKTargetCPUMinorType();
  cpuType->bigEndian = gTRKBigEndian;
  cpuType->defaultTypeSize = 4;
  cpuType->fpTypeSize = 8;
  cpuType->extended1TypeSize = 4;
  cpuType->extended2TypeSize = 8;
  return DS_NoError;
}

asm void TRKInterruptHandler(void) { SEQ_TRKInterruptHandler(); }

static asm void TRKExceptionHandler(u16) { SEQ_TRKExceptionHandler(); }

asm void TRKSwapAndGo(void) { SEQ_TRKSwapAndGo(); }

asm void TRKInterruptHandlerEnableInterrupts(void) { SEQ_TRKInterruptHandlerEnableInterrupts(); }

void TRKPostInterruptEvent(void) {
  int eventType;
  int instruction;
  size_t registerSize;
  TRKEvent event;

  if (gTRKState.inputActivated) {
    gTRKState.inputActivated = FALSE;
  } else {
    switch (gTRKCPUState.Extended1.exceptionID & 0xFFFF) {
    case 0xd00:
    case 0x700:
      registerSize = 4;
      TRKTargetReadInstruction(&instruction, gTRKCPUState.Default.PC);

      if (instruction == 0xfe00000) {
        eventType = NUBEVENT_Support;
      } else {
        eventType = NUBEVENT_Breakpoint;
      }
      break;
    default:
      eventType = NUBEVENT_Exception;
      break;
    }

    TRKConstructEvent(&event, eventType);
    TRKPostEvent(&event);
  }
}

static inline BOOL TRKTargetCheckStep();

DSError TRKTargetInterrupt(TRKEvent* event) {
  DSError error = DS_NoError;
  switch (event->event_type) {
  case NUBEVENT_Breakpoint:
  case NUBEVENT_Exception:
    if (TRKTargetCheckStep() == FALSE) {
      TRKTargetSetStopped(TRUE);
      error = TRKDoNotifyStopped(DSMSG_NotifyStopped);
    }
    break;
  default:
    break;
  }

  return error;
}

DSError TRKTargetAddStopInfo(MessageBuffer* b) {
  DSError error;
  u32 instruction;
  size_t instructionLength;
  s32 i;

  error = TRKAppendBuffer1_ui32(b, gTRKCPUState.Default.PC);

  if (error == DS_NoError) {
    instructionLength = sizeof(instruction);
    error = TRKTargetAccessMemory(&instruction, gTRKCPUState.Default.PC, &instructionLength,
                                  MEMACCESS_UserMemory, TRUE);

    if (error == DS_NoError && instructionLength != sizeof(instruction)) {
      error = DS_InvalidMemory;
    }
  }

  if (error == DS_NoError) {
    error = TRKAppendBuffer1_ui32(b, instruction);
  }

  if (error == DS_NoError) {
    error = TRKAppendBuffer1_ui16(b, gTRKCPUState.Extended1.exceptionID);
  }

  if (error == DS_NoError) {
    for (i = 0; i < 32; i++) {
      TRKAppendBuffer1_ui32(b, (u16)gTRKCPUState.Default.GPR[i]);
    }

    for (i = 0; i < 32; i++) {
      error = TRKAppendBuffer1_ui64(b, (u16)gTRKCPUState.Float.FPR[i]);
    }
  }

  return error;
}

DSError TRKTargetAddExceptionInfo(MessageBuffer* b) {
  DSError error;
  u32 instruction;
  size_t instructionLength;

  error = TRKAppendBuffer1_ui32(b, gTRKExceptionStatus.exceptionInfo.PC);

  if (error == DS_NoError) {
    instructionLength = sizeof(instruction);
    error = TRKTargetAccessMemory(&instruction, gTRKExceptionStatus.exceptionInfo.PC,
                                  &instructionLength, MEMACCESS_UserMemory, TRUE);

    if (error == DS_NoError && instructionLength != sizeof(instruction)) {
      error = DS_InvalidMemory;
    }
  }

  if (error == DS_NoError) {
    error = TRKAppendBuffer1_ui32(b, instruction);
  }

  if (error == DS_NoError) {
    error = TRKAppendBuffer1_ui16(b, gTRKExceptionStatus.exceptionInfo.exceptionID);
  }

  return error;
}

inline DSError TRKTargetEnableTrace(BOOL val) {
  if (val) {
    gTRKCPUState.Extended1.MSR = (gTRKCPUState.Extended1.MSR | 0x400);
  } else {
    gTRKCPUState.Extended1.MSR = (gTRKCPUState.Extended1.MSR & ~0x400);
  }
  return DS_NoError;
}

static inline BOOL TRKTargetStepDone() {
  BOOL result = TRUE;

  if (gTRKStepStatus.active && ((u16)gTRKCPUState.Extended1.exceptionID) == PPC_Trace) {
    switch (gTRKStepStatus.type) {
    case DSSTEP_IntoCount:
      if (gTRKStepStatus.count > 0) {
        result = FALSE;
      }
      break;
    case DSSTEP_IntoRange:
      if (gTRKCPUState.Default.PC >= gTRKStepStatus.rangeStart &&
          gTRKCPUState.Default.PC <= gTRKStepStatus.rangeEnd) {
        result = FALSE;
      }
      break;
    default:
      break;
    }
  }

  return result;
}

inline DSError TRKTargetDoStep(void) {
  gTRKStepStatus.active = TRUE;
  TRKTargetEnableTrace(TRUE);

  if (gTRKStepStatus.type == DSSTEP_IntoCount || gTRKStepStatus.type == DSSTEP_OverCount) {
    gTRKStepStatus.count--;
  }

  TRKTargetSetStopped(FALSE);
  return DS_NoError;
}

static inline BOOL TRKTargetCheckStep() {
  if (gTRKStepStatus.active) {
    TRKTargetEnableTrace(FALSE);

    if (TRKTargetStepDone()) {
      gTRKStepStatus.active = FALSE;
    } else {
      TRKTargetDoStep();
    }
  }

  return gTRKStepStatus.active;
}

DSError TRKTargetSingleStep(u32 count, BOOL stepOver) {
  DSError error = DS_NoError;

  if (stepOver) {
    error = DS_UnsupportedError;
  } else {
    gTRKStepStatus.type = DSSTEP_IntoCount;
    gTRKStepStatus.count = count;
    error = TRKTargetDoStep();
  }

  return error;
}

DSError TRKTargetStepOutOfRange(u32 rangeStart, u32 rangeEnd, BOOL stepOver) {
  DSError error = DS_NoError;

  if (stepOver) {
    error = DS_UnsupportedError;
  } else {
    gTRKStepStatus.type = DSSTEP_IntoRange;
    gTRKStepStatus.rangeStart = rangeStart;
    gTRKStepStatus.rangeEnd = rangeEnd;
    error = TRKTargetDoStep();
  }

  return error;
}

u32 TRKTargetGetPC(void) { return gTRKCPUState.Default.PC; }

DSError TRKTargetSupportRequest() {
  u8 ioResult;
  MessageCommandID commandId;
  size_t* length;
  DSError error;
  u32 local_28;
  TRKEvent event;

  commandId = gTRKCPUState.Default.GPR[3];
  if (commandId != DSMSG_ReadFile && commandId != DSMSG_WriteFile &&
      commandId != DSMSG_OpenFile && commandId != DSMSG_CloseFile &&
      commandId != DSMSG_PositionFile) {
    TRKConstructEvent(&event, 4);
    TRKPostEvent(&event);
    return DS_NoError;
  } else if (commandId == DSMSG_OpenFile) {
    error = HandleOpenFileSupportRequest((const char*)gTRKCPUState.Default.GPR[4],
                                         (u8)gTRKCPUState.Default.GPR[5],
                                         (u32*)gTRKCPUState.Default.GPR[6], &ioResult);

    if (ioResult == DS_IONoError && error != DS_NoError) {
      ioResult = DS_IOError;
    }

    gTRKCPUState.Default.GPR[3] = ioResult;
  } else if (commandId == DSMSG_CloseFile) {
    error = HandleCloseFileSupportRequest(gTRKCPUState.Default.GPR[4], &ioResult);

    if (ioResult == DS_IONoError && error != DS_NoError) {
      ioResult = DS_IOError;
    }

    gTRKCPUState.Default.GPR[3] = ioResult;
  } else if (commandId == DSMSG_PositionFile) {
    local_28 = *(u32*)gTRKCPUState.Default.GPR[5];
    error = HandlePositionFileSupportRequest(gTRKCPUState.Default.GPR[4], &local_28,
                                             (u8)gTRKCPUState.Default.GPR[6], &ioResult);

    if (ioResult == DS_IONoError && error != DS_NoError) {
      ioResult = DS_IOError;
    }

    gTRKCPUState.Default.GPR[3] = ioResult;
    *(u32*)gTRKCPUState.Default.GPR[5] = local_28;
  } else {
    length = (size_t*)gTRKCPUState.Default.GPR[5];
    error = TRKSuppAccessFile((u8)gTRKCPUState.Default.GPR[4],
                              (u8*)gTRKCPUState.Default.GPR[6], length, &ioResult, TRUE,
                              commandId == DSMSG_ReadFile);

    if (ioResult == DS_IONoError && error != DS_NoError) {
      ioResult = DS_IOError;
    }

    gTRKCPUState.Default.GPR[3] = ioResult;

    if (commandId == DSMSG_ReadFile) {
      TRK_flush_cache((void*)gTRKCPUState.Default.GPR[6], *length);
    }
  }

  gTRKCPUState.Default.PC += 4;
  return error;
}

DSError TRKTargetFlushCache(u8 options, void* start, void* end) {
  (void)options;

  if ((u32)start < (u32)end) {
    TRK_flush_cache(start, (u32)end - (u32)start);
    return DS_NoError;
  }

  return DS_InvalidMemory;
}

BOOL TRKTargetStopped(void) { return gTRKState.isStopped; }

void TRKTargetSetStopped(uint stopped) { gTRKState.isStopped = stopped; }

BOOL TRKTargetStop() {
  gTRKState.isStopped = 1;
  return FALSE;
}

static inline DSError TRKPPCAccessSPR(void* value, u32 spr_register_num, BOOL read) {

  u32 access_func[10] = { INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP,
                          INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP };
  /*
  ** Construct a small assembly function to perform the
  ** requested access and call it.  The read/write function
  ** is in the form:
  **
  ** read:
  **        mfspr    r4, spr_register_num
  **        stw      r4, 0(r3)
  **        blr
  **
  ** write:
  **        lwz      r4, 0(r3)
  **        mtspr    spr_register_num, r4
  **        blr
  **
  */

  if (read) {
    access_func[0] = INSTR_MFSPR(4, spr_register_num);
    access_func[1] = INSTR_STW(4, 0, 3);
  } else {
    access_func[0] = INSTR_LWZ(4, 0, 3);
    access_func[1] = INSTR_MTSPR(spr_register_num, 4);
  }

  return TRKPPCAccessSpecialReg(value, access_func, read);
}

static inline DSError TRKPPCAccessPairedSingleRegister(void* srcDestPtr, u32 psr, BOOL read) {
  // all nop by default
  u32 instructionData[] = { INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP,
                            INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP };

  if (read) {
    instructionData[0] = INSTR_PSQ_ST(psr, 0, 3, 0, 0);
  } else {
    instructionData[0] = INSTR_PSQ_L(psr, 0, 3, 0, 0);
  }

  return TRKPPCAccessSpecialReg(srcDestPtr, instructionData, read);
}

DSError TRKPPCAccessFPRegister(void* srcDestPtr, u32 fpr, BOOL read) {
  DSError error = DS_NoError;
  // all nop by default
  u32 instructionData1[] = { INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP,
                             INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP };

  if (fpr < 0x20) {
    if (read) {
      instructionData1[0] = INSTR_STFD(fpr, 0, 3);
    } else {
      instructionData1[0] = INSTR_LFD(fpr, 0, 3);
    }

    error = TRKPPCAccessSpecialReg(srcDestPtr, instructionData1, read);
  } else if (fpr == 0x20) {
    *(u64*)srcDestPtr &= 0xFFFFFFFF;
  } else if (fpr == 0x21) {
    if (!read) {
      *(u32*)srcDestPtr = *((u32*)(srcDestPtr) + 1);
    }

    error = TRKPPCAccessSPR(srcDestPtr, SPR_FPECR, read);
    if (read) {
      DSFetch_u64(srcDestPtr) = DSFetch_u32(srcDestPtr) & 0xffffffffLL;
    }
  }

  return error;
}

#define DEBUG_VECTORREG_ACCESS 0

static inline DSError TRKPPCAccessSpecialReg(void* value, u32* access_func, BOOL read) {
#pragma unused(read)

  typedef void (*asm_access_type)(void*, void*);

  asm_access_type asm_access;

  /*
  ** Construct a small assembly function to perform the
  ** requested access and call it.  The read/write function
  ** is in the form:
  **
  **        <access_func>
  **        blr
  */

  /*
  ** Put blr instruction at the end of access function (it should be
  ** a 5-instruction array w/the last one empty).
  */

  access_func[9] = INSTR_BLR;

  /*
  ** Now that the instruction array is built, get a function pointer to it.
  */

  asm_access = (asm_access_type)access_func;

  // Flush cache
  TRK_flush_cache((void*)(u32)access_func, (sizeof(access_func) * 10));
  (*asm_access)((u32*)value, &TRKvalue128_temp);

  return DS_NoError;
}

void TRKTargetSetInputPendingPtr(void* ptr) { gTRKState.inputPendingPtr = ptr; }

u128 TRKvalue128_temp;
Default_PPC gTRKSaveState;
ProcessorState_PPC gTRKCPUState;
TRKState gTRKState;
