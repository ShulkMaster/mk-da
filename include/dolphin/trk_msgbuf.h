#ifndef DOLPHIN_TRK_MSGBUF_H
#define DOLPHIN_TRK_MSGBUF_H

#include <dolphin/trk.h>

static inline DSError TRKAppendBuffer1_ui8(MessageBuffer* buffer, const u8 data) {
  if (buffer->position >= 0x880) {
    return DS_MessageBufferOverflow;
  }

  buffer->data[buffer->position++] = data;
  buffer->length++;
  return DS_NoError;
}

#endif
