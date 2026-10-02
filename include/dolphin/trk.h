#ifndef DOLPHIN_TRK_H
#define DOLPHIN_TRK_H

#include <dolphin/types.h>

typedef int DSError;
typedef int MessageBufferID;
typedef u8 MessageCommandID;

typedef struct MessageBuffer {
  u32 mutex;
  BOOL is_in_use;
  u32 length;
  u32 position;
  u8 data[0x880];
} MessageBuffer;

typedef struct TRKEvent {
  u8 event_type;
  u32 event_id;
  MessageBufferID message_buffer_id;
} TRKEvent;

DSError TRKInitializeMutex(void* mutex);
DSError TRKAcquireMutex(void* mutex);
DSError TRKReleaseMutex(void* mutex);
void* TRK_memcpy(void* destination, const void* source, u32 size);
DSError TRKGetFreeBuffer(MessageBufferID* id, MessageBuffer** buffer);
MessageBuffer* TRKGetBuffer(MessageBufferID id);
void TRKReleaseBuffer(MessageBufferID id);
BOOL TRKGetNextEvent(TRKEvent* event);
DSError TRKInitializeEventQueue(void);
DSError TRKPostEvent(TRKEvent* event);
void TRKConstructEvent(TRKEvent* event, u8 event_type);
void TRKDestructEvent(TRKEvent* event);
DSError TRKDispatchMessage(MessageBuffer* message);
void TRKGetInput(void);
extern volatile u8* gTRKInputPendingPtr;

#endif
