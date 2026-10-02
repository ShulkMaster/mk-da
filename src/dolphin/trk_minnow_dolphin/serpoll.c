#include "dolphin/trk_msgbuf.h"

static TRKFramingState gTRKFramingState;
volatile u8* gTRKInputPendingPtr;

static inline BOOL serpoll_inline_00(MessageBuffer* buffer) {
  if (buffer->length < 2) {
    TRKStandardACK(buffer, DSMSG_ReplyNAK, DSREPLY_PacketSizeError);
    if (gTRKFramingState.msg_buffer_id != -1) {
      TRKReleaseBuffer(gTRKFramingState.msg_buffer_id);
      gTRKFramingState.msg_buffer_id = -1;
    }
    gTRKFramingState.buffer = NULL;
    gTRKFramingState.receive_state = DSRECV_Wait;
    return FALSE;
  }
  buffer->position = 0;
  buffer->length--;
  return TRUE;
}

MessageBufferID TRKTestForPacket(void) {
  s32 var_r29;
  s32 var_r3;
  s8 sp8;
  s32 temp_r3;

  var_r29 = 0;
  var_r3 = TRKReadUARTPoll(&sp8);
  while (var_r3 == 0 && var_r29 == 0) {
    if (gTRKFramingState.receive_state != DSRECV_InFrame) {
      gTRKFramingState.is_escape = FALSE;
    }
    switch (gTRKFramingState.receive_state) {
      case DSRECV_Wait:
        if (sp8 == 0x7E) {
          var_r29 = TRKGetFreeBuffer(&gTRKFramingState.msg_buffer_id, &gTRKFramingState.buffer);
          gTRKFramingState.fcs_type = 0;
          gTRKFramingState.receive_state = DSRECV_Found;
        }
        break;
      case DSRECV_Found:
        if (sp8 == 0x7E) {
          break;
        }
        gTRKFramingState.receive_state = DSRECV_InFrame;
      case DSRECV_InFrame:
        if (sp8 == 0x7E) {
          if (gTRKFramingState.is_escape) {
            TRKStandardACK(gTRKFramingState.buffer, DSMSG_ReplyNAK, DSREPLY_EscapeError);
            if (gTRKFramingState.msg_buffer_id != -1) {
              TRKReleaseBuffer(gTRKFramingState.msg_buffer_id);
              gTRKFramingState.msg_buffer_id = -1;
            }
            gTRKFramingState.buffer = NULL;
            gTRKFramingState.receive_state = DSRECV_Wait;
            break;
          }
          if (serpoll_inline_00(gTRKFramingState.buffer)) {
            temp_r3 = gTRKFramingState.msg_buffer_id;
            gTRKFramingState.msg_buffer_id = -1;
            gTRKFramingState.buffer = NULL;
            gTRKFramingState.receive_state = DSRECV_Wait;
            return temp_r3;
          }
          gTRKFramingState.receive_state = DSRECV_Wait;
        } else {
          if (gTRKFramingState.is_escape) {
            sp8 ^= 0x20;
            gTRKFramingState.is_escape = FALSE;
          } else if (sp8 == 0x7D) {
            gTRKFramingState.is_escape = TRUE;
            break;
          }
          var_r29 = TRKAppendBuffer1_ui8(gTRKFramingState.buffer, sp8);
          gTRKFramingState.fcs_type += sp8;
        }
        break;
      case DSRECV_FrameOverflow:
        if (sp8 == 0x7E) {
          if (gTRKFramingState.msg_buffer_id != -1) {
            TRKReleaseBuffer(gTRKFramingState.msg_buffer_id);
            gTRKFramingState.msg_buffer_id = -1;
          }
          gTRKFramingState.buffer = NULL;
          gTRKFramingState.receive_state = DSRECV_Wait;
        }
        break;
    }
    var_r3 = TRKReadUARTPoll(&sp8);
  }
  return -1;
}

void TRKGetInput(void) {
  MessageBuffer* msgBuffer;
  MessageBufferID id;
  u8 command;

  id = TRKTestForPacket();
  if (id == -1) {
    return;
  }

  msgBuffer = TRKGetBuffer(id);
  TRKSetBufferPosition(msgBuffer, 0);
  TRKReadBuffer1_ui8(msgBuffer, &command);
  if (command < DSMSG_ReplyACK) {
    TRKProcessInput(id);
  } else {
    TRKReleaseBuffer(id);
  }
}

void TRKProcessInput(int bufferIdx) {
  TRKEvent event;

  TRKConstructEvent(&event, NUBEVENT_Request);
  gTRKFramingState.msg_buffer_id = -1;
  event.message_buffer_id = bufferIdx;
  TRKPostEvent(&event);
}

DSError TRKInitializeSerialHandler(void) {
  gTRKFramingState.msg_buffer_id = -1;
  gTRKFramingState.receive_state = DSRECV_Wait;
  gTRKFramingState.is_escape = FALSE;

  return DS_NoError;
}

DSError TRKTerminateSerialHandler(void) {
  return DS_NoError;
}
