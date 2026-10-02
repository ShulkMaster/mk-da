#include "dolphin/trk.h"

extern void TRKTargetAddStopInfo(MessageBuffer* buffer);
extern void TRKTargetAddExceptionInfo(MessageBuffer* buffer);
extern DSError TRKRequestSend(MessageBuffer* buffer, int* request_id, int retries,
                            int timeout, int blocking);

static inline DSError TRKAppendBuffer1_ui8(MessageBuffer* buffer, u8 value) {
  DSError err;

  if (buffer->position >= 0x880) {
    err = 0x301;
  } else {
    buffer->data[buffer->position++] = value;
    buffer->length += 1;
    err = 0;
  }

  return err;
}

DSError TRKDoNotifyStopped(MessageCommandID cmd) {
  DSError err;
  int reqIdx;
  int bufIdx;
  MessageBuffer* msg;

  err = TRKGetFreeBuffer(&bufIdx, &msg);
  if (err == 0) {
    err = TRKAppendBuffer1_ui8(msg, cmd);

    if (err == 0) {
      if ((u8)cmd == 0x90) {
        TRKTargetAddStopInfo(msg);
      } else {
        TRKTargetAddExceptionInfo(msg);
      }
    }

    err = TRKRequestSend(msg, &reqIdx, 2, 3, 1);
    if (err == 0) {
      TRKReleaseBuffer(reqIdx);
    }

    TRKReleaseBuffer(bufIdx);
  }

  return err;
}
