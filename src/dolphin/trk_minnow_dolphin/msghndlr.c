#include "dolphin/trk_msgbuf.h"

typedef unsigned long size_t;

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

enum {
  DS_InvalidMemory = 0x700,
  DS_InvalidRegister = 0x701,
  DS_CWDSException = 0x702,
  DS_UnsupportedError = 0x703,
  DS_InvalidProcessID = 0x704,
  DS_InvalidThreadID = 0x705,
  DS_OSError = 0x706,
};

enum {
  DSREG_Default = 0,
  DSREG_FP = 1,
  DSREG_Extended1 = 2,
  DSREG_Extended2 = 3,
};

enum {
  DSSTEP_IntoCount = 0x0,
  DSSTEP_IntoRange = 0x1,
  DSSTEP_OverCount = 0x10,
  DSSTEP_OverRange = 0x11,
};

enum {
  DSREPLY_NoError = 0x0,
  DSREPLY_Error = 0x1,
  DSREPLY_CWDSError = 0x3,
  DSREPLY_UnsupportedCommandError = 0x10,
  DSREPLY_ParameterError = 0x11,
  DSREPLY_UnsupportedOptionError = 0x12,
  DSREPLY_InvalidMemoryRange = 0x13,
  DSREPLY_InvalidRegisterRange = 0x14,
  DSREPLY_CWDSException = 0x15,
  DSREPLY_NotStopped = 0x16,
  DSREPLY_OSError = 0x20,
  DSREPLY_InvalidProcessID = 0x21,
  DSREPLY_InvalidThreadID = 0x22,
};

enum {
  DSMSGMEMORY_Extended = 0x02,
  DSMSGMEMORY_Userview = 0x08,
};

enum {
  NUBEVENT_Shutdown = 1,
};

enum {
  MEMACCESS_UserMemory = 0,
  MEMACCESS_DebuggerMemory = 1,
};

typedef int DSReplyError;
typedef int DSMessageRegisterOptions;

DSError TRKTargetFlushCache(u8 options, void* start, void* end);
DSError TRKTargetAccessMemory(void* data, u32 start, size_t* length, int access_options, BOOL read);
DSError TRKTargetAccessDefault(u32 first, u32 last, MessageBuffer* b, size_t* length, BOOL read);
DSError TRKTargetAccessFP(u32 first, u32 last, MessageBuffer* b, size_t* length, BOOL read);
DSError TRKTargetAccessExtended1(u32 first, u32 last, MessageBuffer* b, size_t* length, BOOL read);
DSError TRKTargetAccessExtended2(u32 first, u32 last, MessageBuffer* b, size_t* length, BOOL read);
BOOL TRKTargetStopped(void);
DSError TRKTargetContinue(void);
u32 TRKTargetGetPC(void);
DSError TRKTargetSingleStep(u32 count, BOOL step_over);
DSError TRKTargetStepOutOfRange(u32 start, u32 end, BOOL step_over);
u32 TRKTargetStop(void);
DSError TRKTargetVersions(DSVersions* versions);
DSError TRKTargetSupportMask(u8 mask[32]);
DSError TRKTargetCPUType(DSCPUType* cpu_type);
void __TRK_reset(void);
void SetUseSerialIO(u8 enable);

static BOOL IsTRKConnected;

BOOL GetTRKConnected(void) {
  return IsTRKConnected;
}

void SetTRKConnected(BOOL connected) {
  IsTRKConnected = connected;
}

static inline void TRKMessageIntoReply(MessageBuffer* buffer, u8 ackCmd,
                                       DSReplyError errSentInAck) {
  TRKResetBuffer(buffer, 1);

  TRKAppendBuffer1_ui8(buffer, ackCmd);
  TRKAppendBuffer1_ui8(buffer, errSentInAck);
}

static inline DSError TRKSendACK(MessageBuffer* buffer) {
  DSError err;
  int ackTries;

  ackTries = 3;
  do {
    err = TRKMessageSend(buffer);
    --ackTries;
  } while (err != DS_NoError && ackTries > 0);

  return err;
}

DSError TRKStandardACK(MessageBuffer* buffer, MessageCommandID commandID,
                       DSReplyError replyError) {
  TRKMessageIntoReply(buffer, commandID, replyError);
  return TRKSendACK(buffer);
}

DSError TRKDoUnsupported(MessageBuffer* buffer) {
  return TRKStandardACK(buffer, DSMSG_ReplyACK,
                        DSREPLY_UnsupportedCommandError);
}

DSError TRKDoConnect(MessageBuffer* buffer) {
  SetTRKConnected(TRUE);
  return TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_NoError);
}

DSError TRKDoDisconnect(MessageBuffer* buffer) {
  DSError error;
  TRKEvent event;

  SetTRKConnected(FALSE);
  error = TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_NoError);

  if (error == DS_NoError) {
    TRKConstructEvent(&event, NUBEVENT_Shutdown);
    TRKPostEvent(&event);
  }

  return error;
}

DSError TRKDoReset(MessageBuffer* buffer) {
  TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_NoError);
  __TRK_reset();
  return DS_NoError;
}

DSError TRKDoVersions(MessageBuffer* buffer) {
  DSError error;
  DSVersions versions;

  if (buffer->length != 1) {
    error = TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_PacketSizeError);
  } else {
    TRKMessageIntoReply(buffer, DSMSG_ReplyACK, DSREPLY_NoError);
    error = TRKTargetVersions(&versions);

    if (error == DS_NoError)
      error = TRKAppendBuffer1_ui8(buffer, versions.kernelMajor);
    if (error == DS_NoError)
      error = TRKAppendBuffer1_ui8(buffer, versions.kernelMinor);
    if (error == DS_NoError)
      error = TRKAppendBuffer1_ui8(buffer, versions.protocolMajor);
    if (error == DS_NoError)
      error = TRKAppendBuffer1_ui8(buffer, versions.protocolMinor);

    if (error != DS_NoError)
      error = TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_CWDSError);
    else
      error = TRKSendACK(buffer);
  }
}

DSError TRKDoSupportMask(MessageBuffer* buffer) {
  DSError error;
  u8 mask[32];

  if (buffer->length != 1) {
    TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_PacketSizeError);
  } else {
    TRKMessageIntoReply(buffer, DSMSG_ReplyACK, DSREPLY_NoError);
    error = TRKTargetSupportMask(mask);

    if (error == DS_NoError)
      error = TRKAppendBuffer(buffer, mask, 32);
    if (error == DS_NoError)
      error = TRKAppendBuffer1_ui8(buffer, 2);

    if (error != DS_NoError)
      TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_CWDSError);
    else
      TRKSendACK(buffer);
  }
}

DSError TRKDoCPUType(MessageBuffer* buffer) {
  DSError error;
  DSCPUType cputype;

  if (buffer->length != 1) {
    error = TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_PacketSizeError);
    return;
  }

  TRKMessageIntoReply(buffer, DSMSG_ReplyACK, DSREPLY_NoError);

  error = TRKTargetCPUType(&cputype);

  if (error == DS_NoError)
    error = TRKAppendBuffer1_ui8(buffer, cputype.cpuMajor);
  if (error == DS_NoError)
    error = TRKAppendBuffer1_ui8(buffer, cputype.cpuMinor);
  if (error == DS_NoError)
    error = TRKAppendBuffer1_ui8(buffer, cputype.bigEndian);
  if (error == DS_NoError)
    error = TRKAppendBuffer1_ui8(buffer, cputype.defaultTypeSize);
  if (error == DS_NoError)
    error = TRKAppendBuffer1_ui8(buffer, cputype.fpTypeSize);
  if (error == DS_NoError)
    error = TRKAppendBuffer1_ui8(buffer, cputype.extended1TypeSize);
  if (error == DS_NoError)
    error = TRKAppendBuffer1_ui8(buffer, cputype.extended2TypeSize);

  if (error != DS_NoError)
    error = TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_CWDSError);
  else
    error = TRKSendACK(buffer);
}

DSError TRKDoReadMemory(MessageBuffer* buffer) {
  DSError error;
  DSReplyError replyError;
  u8 tmpBuffer[0x800];
  u32 msg_start;
  u32 length;
  u16 msg_length;
  u8 msg_command;
  u8 msg_options;

  if (buffer->length != 8) {
    error = TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_PacketSizeError);
    return error;
  }

  TRKSetBufferPosition(buffer, DSREPLY_NoError);
  error = TRKReadBuffer1_ui8(buffer, &msg_command);
  if (error == DS_NoError)
    error = TRKReadBuffer1_ui8(buffer, &msg_options);

  if (error == DS_NoError)
    error = TRKReadBuffer1_ui16(buffer, &msg_length);

  if (error == DS_NoError)
    error = TRKReadBuffer1_ui32(buffer, &msg_start);

  if (msg_options & DSMSGMEMORY_Extended) {
    error = TRKStandardACK(buffer, DSMSG_ReplyACK,
                           DSREPLY_UnsupportedOptionError);
    return error;
  }

  if (msg_length > 0x800) {
    error = TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_ParameterError);
    return error;
  }

  TRKMessageIntoReply(buffer, DSMSG_ReplyACK, DSREPLY_NoError);

  if (error == DS_NoError) {
    length = msg_length;
    error = TRKTargetAccessMemory(tmpBuffer, msg_start, &length,
                                  (msg_options & DSMSGMEMORY_Userview)
                                      ? MEMACCESS_UserMemory
                                      : MEMACCESS_DebuggerMemory,
                                  TRUE);
    msg_length = length;
    if (error == DS_NoError)
      error = TRKAppendBuffer1_ui16(buffer, msg_length);
    if (error == DS_NoError)
      error = TRKAppendBuffer(buffer, tmpBuffer, length);
  }

  if (error != DS_NoError) {
    switch (error) {
    case DS_CWDSException:
      replyError = DSREPLY_CWDSException;
      break;
    case DS_InvalidMemory:
      replyError = DSREPLY_InvalidMemoryRange;
      break;
    case DS_InvalidProcessID:
      replyError = DSREPLY_InvalidProcessID;
      break;
    case DS_InvalidThreadID:
      replyError = DSREPLY_InvalidThreadID;
      break;
    case DS_OSError:
      replyError = DSREPLY_OSError;
      break;
    default:
      replyError = DSREPLY_CWDSError;
      break;
    }
    error = TRKStandardACK(buffer, DSMSG_ReplyACK, replyError);
  } else {
    error = TRKSendACK(buffer);
  }

  return error;
}

DSError TRKDoWriteMemory(MessageBuffer* buffer) {
  DSError error;
  DSReplyError replyError;
  u8 tmpBuffer[0x800];
  u32 msg_start;
  u32 length;
  u16 msg_length;
  u8 msg_command;
  u8 msg_options;

  if (buffer->length <= 8) {
    error = TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_PacketSizeError);
    return error;
  }

  TRKSetBufferPosition(buffer, DSREPLY_NoError);
  error = TRKReadBuffer1_ui8(buffer, &msg_command);
  if (error == DS_NoError)
    error = TRKReadBuffer1_ui8(buffer, &msg_options);

  if (error == DS_NoError)
    error = TRKReadBuffer1_ui16(buffer, &msg_length);

  if (error == DS_NoError)
    error = TRKReadBuffer1_ui32(buffer, &msg_start);

  if (msg_options & 2) {
    error = TRKStandardACK(buffer, DSMSG_ReplyACK,
                           DSREPLY_UnsupportedOptionError);
    return error;
  }

  if ((buffer->length != msg_length + 8) || (msg_length > 0x800)) {
    error = TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_ParameterError);
  } else {
    if (error == DS_NoError) {
      length = msg_length;
      error  = TRKReadBuffer(buffer, tmpBuffer, length);
      if (error == DS_NoError) {
        error = TRKTargetAccessMemory(tmpBuffer, msg_start, &length,
                                      (msg_options & 8)
                                          ? MEMACCESS_UserMemory
                                          : MEMACCESS_DebuggerMemory,
                                      FALSE);
      }
      msg_length = length;
    }

    if (error == DS_NoError)
      TRKMessageIntoReply(buffer, DSMSG_ReplyACK, DSREPLY_NoError);

    if (error == DS_NoError)
      error = TRKAppendBuffer1_ui16(buffer, msg_length);

    if (error != DS_NoError) {
      switch (error) {
      case DS_CWDSException:
        replyError = DSREPLY_CWDSException;
        break;
      case DS_InvalidMemory:
        replyError = DSREPLY_InvalidMemoryRange;
        break;
      case DS_InvalidProcessID:
        replyError = DSREPLY_InvalidProcessID;
        break;
      case DS_InvalidThreadID:
        replyError = DSREPLY_InvalidThreadID;
        break;
      case DS_OSError:
        replyError = DSREPLY_OSError;
        break;
      default:
        replyError = DSREPLY_CWDSError;
        break;
      }
      error = TRKStandardACK(buffer, DSMSG_ReplyACK, replyError);
    } else {
      error = TRKSendACK(buffer);
    }
  }

  return error;
}

DSError TRKDoReadRegisters(MessageBuffer* buffer) {
  DSError error;
  DSReplyError replyError;
  DSMessageRegisterOptions options;
  u32 registerDataLength;
  u16 msg_firstRegister;
  u16 msg_lastRegister;
  u8 msg_command;
  u8 msg_options;

  if (buffer->length != 6) {
    error = TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_PacketSizeError);
    return;
  }
  TRKSetBufferPosition(buffer, DSREPLY_NoError);
  error = TRKReadBuffer1_ui8(buffer, &msg_command);
  if (error == DS_NoError)
    error = TRKReadBuffer1_ui8(buffer, &msg_options);

  if (error == DS_NoError)
    error = TRKReadBuffer1_ui16(buffer, &msg_firstRegister);

  if (error == DS_NoError)
    error = TRKReadBuffer1_ui16(buffer, &msg_lastRegister);

  if (msg_firstRegister > msg_lastRegister) {
    error = TRKStandardACK(buffer, DSMSG_ReplyACK,
                           DSREPLY_InvalidRegisterRange);
    return;
  }

  if (error == DS_NoError)
    TRKMessageIntoReply(buffer, DSMSG_ReplyACK, DSREPLY_NoError);

  options = (DSMessageRegisterOptions)(msg_options & 7);
  switch (options) {
  case DSREG_Default:
    error = TRKTargetAccessDefault(msg_firstRegister, msg_lastRegister,
                                   buffer, &registerDataLength, TRUE);
    break;
  case DSREG_FP:
    error = TRKTargetAccessFP(msg_firstRegister, msg_lastRegister, buffer,
                              &registerDataLength, TRUE);
    break;
  case DSREG_Extended1:
    error = TRKTargetAccessExtended1(msg_firstRegister, msg_lastRegister,
                                     buffer, &registerDataLength, TRUE);
    break;
  case DSREG_Extended2:
    error = TRKTargetAccessExtended2(msg_firstRegister, msg_lastRegister,
                                     buffer, &registerDataLength, TRUE);
    break;
  default:
    error = DS_UnsupportedError;
    break;
  }

  if (error != DS_NoError) {
    switch (error) {
    case DS_UnsupportedError:
      replyError = DSREPLY_UnsupportedOptionError;
      break;
    case DS_InvalidRegister:
      replyError = DSREPLY_InvalidRegisterRange;
      break;
    case DS_CWDSException:
      replyError = DSREPLY_CWDSException;
      break;
    case DS_InvalidProcessID:
      replyError = DSREPLY_InvalidProcessID;
      break;
    case DS_InvalidThreadID:
      replyError = DSREPLY_InvalidThreadID;
      break;
    case DS_OSError:
      replyError = DSREPLY_OSError;
      break;
    default:
      replyError = DSREPLY_CWDSError;
    }

    error = TRKStandardACK(buffer, DSMSG_ReplyACK, replyError);
  } else {
    error = TRKSendACK(buffer);
  }
}

DSError TRKDoWriteRegisters(MessageBuffer* buffer) {
  DSError error;
  DSReplyError replyError;
  DSMessageRegisterOptions options;
  u32 registerDataLength;
  u16 msg_firstRegister;
  u16 msg_lastRegister;
  u8 msg_command;
  u8 msg_options;

  if (buffer->length <= 6) {
    error = TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_PacketSizeError);
    return;
  }
  TRKSetBufferPosition(buffer, DSREPLY_NoError);
  error = TRKReadBuffer1_ui8(buffer, &msg_command);
  if (error == DS_NoError)
    error = TRKReadBuffer1_ui8(buffer, &msg_options);

  if (error == DS_NoError)
    error = TRKReadBuffer1_ui16(buffer, &msg_firstRegister);

  if (error == DS_NoError)
    error = TRKReadBuffer1_ui16(buffer, &msg_lastRegister);

  if (msg_firstRegister > msg_lastRegister) {
    error = TRKStandardACK(buffer, DSMSG_ReplyACK,
                           DSREPLY_InvalidRegisterRange);
    return;
  }

  options = (DSMessageRegisterOptions)msg_options;
  switch (options) {
  case DSREG_Default:
    error = TRKTargetAccessDefault(msg_firstRegister, msg_lastRegister,
                                   buffer, &registerDataLength, FALSE);
    break;
  case DSREG_FP:
    error = TRKTargetAccessFP(msg_firstRegister, msg_lastRegister, buffer,
                              &registerDataLength, FALSE);
    break;
  case DSREG_Extended1:
    error = TRKTargetAccessExtended1(msg_firstRegister, msg_lastRegister,
                                     buffer, &registerDataLength, FALSE);
    break;
  case DSREG_Extended2:
    error = TRKTargetAccessExtended2(msg_firstRegister, msg_lastRegister,
                                     buffer, &registerDataLength, FALSE);
    break;
  default:
    error = DS_UnsupportedError;
    break;
  }

  if (error == DS_NoError)
    TRKMessageIntoReply(buffer, DSMSG_ReplyACK, DSREPLY_NoError);

  if (error != DS_NoError) {
    switch (error) {
    case DS_UnsupportedError:
      replyError = DSREPLY_UnsupportedOptionError;
      break;
    case DS_InvalidRegister:
      replyError = DSREPLY_InvalidRegisterRange;
      break;
    case DS_MessageBufferReadError:
      replyError = DSREPLY_PacketSizeError;
      break;
    case DS_CWDSException:
      replyError = DSREPLY_CWDSException;
      break;
    case DS_InvalidProcessID:
      replyError = DSREPLY_InvalidProcessID;
      break;
    case DS_InvalidThreadID:
      replyError = DSREPLY_InvalidThreadID;
      break;
    case DS_OSError:
      replyError = DSREPLY_OSError;
      break;
    default:
      replyError = DSREPLY_CWDSError;
    }

    error = TRKStandardACK(buffer, DSMSG_ReplyACK, replyError);
  } else {
    error = TRKSendACK(buffer);
  }
}

DSError TRKDoFlushCache(MessageBuffer* buffer) {
  DSError error;
  DSReplyError replyErr;
  u32 msg_start;
  u32 msg_end;
  u8 msg_command;
  u8 msg_options;

  if (buffer->length != 10) {
    error = TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_PacketSizeError);
    return;
  }

  TRKSetBufferPosition(buffer, DSREPLY_NoError);
  error = TRKReadBuffer1_ui8(buffer, &msg_command);
  if (error == DS_NoError)
    error = TRKReadBuffer1_ui8(buffer, &msg_options);
  if (error == DS_NoError)
    error = TRKReadBuffer1_ui32(buffer, &msg_start);
  if (error == DS_NoError)
    error = TRKReadBuffer1_ui32(buffer, &msg_end);

  if (msg_start > msg_end) {
    error = TRKStandardACK(buffer, DSMSG_ReplyACK,
                           DSREPLY_InvalidMemoryRange);
    return;
  }

  if (error == DS_NoError)
    error = TRKTargetFlushCache(msg_options, (void*)msg_start,
                                (void*)msg_end);

  if (error == DS_NoError)
    TRKMessageIntoReply(buffer, DSMSG_ReplyACK, DSREPLY_NoError);

  if (error != DS_NoError) {
    switch (error) {
    case DS_UnsupportedError:
      replyErr = DSREPLY_UnsupportedOptionError;
      break;
    default:
      replyErr = DSREPLY_CWDSError;
      break;
    }

    error = TRKStandardACK(buffer, DSMSG_ReplyACK, replyErr);
  } else {
    error = TRKSendACK(buffer);
  }
}

DSError TRKDoContinue(MessageBuffer* buffer) {
  DSError error;

  error = TRKTargetStopped();
  if (error == DS_NoError) {
    error = TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_NotStopped);
    return;
  }

  error = TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_NoError);
  if (error == DS_NoError)
    error = TRKTargetContinue();
}

DSError TRKDoStep(MessageBuffer* buffer) {
  DSError error;
  u8 msg_command;
  u8 msg_options;
  u8 msg_count;
  u32 msg_rangeStart;
  u32 msg_rangeEnd;
  u32 pc;

  if (buffer->length < 3) {
    TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_PacketSizeError);
    return;
  }

  TRKSetBufferPosition(buffer, DSREPLY_NoError);

  error = TRKReadBuffer1_ui8(buffer, &msg_command);
  if (error == DS_NoError)
    error = TRKReadBuffer1_ui8(buffer, &msg_options);

  switch (msg_options) {
  case DSSTEP_IntoCount:
  case DSSTEP_OverCount:
    if (error == DS_NoError)
      TRKReadBuffer1_ui8(buffer, &msg_count);
    if (msg_count >= 1) {
      break;
    }
    TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_ParameterError);
    return;
  case DSSTEP_IntoRange:
  case DSSTEP_OverRange:
    if (buffer->length != 10) {
      TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_PacketSizeError);
      return;
    }

    if (error == DS_NoError)
      error = TRKReadBuffer1_ui32(buffer, &msg_rangeStart);
    if (error == DS_NoError)
      error = TRKReadBuffer1_ui32(buffer, &msg_rangeEnd);

    pc = TRKTargetGetPC();
    if (pc >= msg_rangeStart && pc <= msg_rangeEnd) {
      break;
    }
    TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_ParameterError);
    return;
  default:
    TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_UnsupportedOptionError);
    return;
  }

  if (!TRKTargetStopped()) {
    TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_NotStopped);
    return;
  }

  error = TRKStandardACK(buffer, DSMSG_ReplyACK, DSREPLY_NoError);
  if (error == DS_NoError)
    switch (msg_options) {
    case DSSTEP_IntoCount:
    case DSSTEP_OverCount:
      error = TRKTargetSingleStep(msg_count,
                    (msg_options == DSSTEP_OverCount));
      break;
    case DSSTEP_IntoRange:
    case DSSTEP_OverRange:
      error = TRKTargetStepOutOfRange(
        msg_rangeStart, msg_rangeEnd,
        (msg_options == DSSTEP_OverRange));
      break;
    }
}

DSError TRKDoStop(MessageBuffer* b) {
  DSReplyError replyError;

  switch (TRKTargetStop()) {
  case DS_NoError:
    replyError = DSREPLY_NoError;
    break;
  case DS_InvalidProcessID:
    replyError = DSREPLY_InvalidProcessID;
    break;
  case DS_InvalidThreadID:
    replyError = DSREPLY_InvalidThreadID;
    break;
  case DS_OSError:
    replyError = DSREPLY_OSError;
    break;
  default:
    replyError = DSREPLY_Error;
    break;
  }

  return TRKStandardACK(b, DSMSG_ReplyACK, replyError);
}

DSError TRKDoSetOption(MessageBuffer* buffer) {
    DSError error;
    u8 spA;
    u8 sp9;
    u8 sp8;

    spA = 0;
    sp9 = 0;
    sp8 = 0;
    TRKSetBufferPosition(buffer, DSREPLY_NoError);
    error = TRKReadBuffer1_ui8(buffer, &spA);
    if (error == DS_NoError) {
        error = TRKReadBuffer1_ui8(buffer, &sp9);
    }
    if (error == DS_NoError) {
        error = TRKReadBuffer1_ui8(buffer, &sp8);
    }
    if (error != DS_NoError) {
        TRKResetBuffer(buffer, 1);
        if (buffer->position < 0x880) {
            buffer->data[buffer->position++] = 0x80;
            buffer->length++;
        }
        if (buffer->position < 0x880) {
            buffer->data[buffer->position++] = 1;
            buffer->length++;
        }
        TRKSendACK(buffer);
    } else if (sp9 == 1) {
        SetUseSerialIO(sp8);
    }
    TRKResetBuffer(buffer, 1);
    if (buffer->position < 0x880) {
        buffer->data[buffer->position++] = 0x80;
        buffer->length++;
    }
    if (buffer->position < 0x880) {
        buffer->data[buffer->position++] = 0;
        buffer->length++;
    }
    return TRKSendACK(buffer);
}
