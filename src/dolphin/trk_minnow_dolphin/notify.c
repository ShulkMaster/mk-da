#include "dolphin/trk_msgbuf.h"

extern void TRKTargetAddStopInfo(MessageBuffer* buffer);
extern void TRKTargetAddExceptionInfo(MessageBuffer* buffer);
extern DSError TRKRequestSend(MessageBuffer* buffer, int* request_id, int retries,
                            int timeout, int blocking);

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
