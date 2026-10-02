/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/ar.h>
#include <dolphin/arq.h>
#include <dolphin/os.h>

extern ARQRequest *__ARQRequestQueueHi_8041CBD0;
extern ARQRequest *__ARQRequestQueueLo_8041CBD8;
extern ARQRequest *__ARQRequestPendingHi_8041CBE0;
extern ARQRequest *__ARQRequestPendingLo_8041CBE4;
extern ARQCallback __ARQCallbackHi_8041CBE8;
extern ARQCallback __ARQCallbackLo_8041CBEC;
extern u32 __ARQChunkSize_8041CBF0;

extern ARQRequest *__ARQRequestTailHi_8041CBD4;
extern ARQRequest *__ARQRequestTailLo_8041CBDC;
extern BOOL __ARQ_init_flag_8041CBF4;

void __ARQServiceQueueLo(void);

static inline void __ARQPopTaskQueueHi(void) {

  if (__ARQRequestQueueHi_8041CBD0) {
    if (__ARQRequestQueueHi_8041CBD0->type == ARQ_TYPE_MRAM_TO_ARAM) {
      ARStartDMA(__ARQRequestQueueHi_8041CBD0->type, __ARQRequestQueueHi_8041CBD0->source,
                 __ARQRequestQueueHi_8041CBD0->dest, __ARQRequestQueueHi_8041CBD0->length);
    } else {
      ARStartDMA(__ARQRequestQueueHi_8041CBD0->type, __ARQRequestQueueHi_8041CBD0->dest,
                 __ARQRequestQueueHi_8041CBD0->source, __ARQRequestQueueHi_8041CBD0->length);
    }

    __ARQCallbackHi_8041CBE8 = __ARQRequestQueueHi_8041CBD0->callback;

    __ARQRequestPendingHi_8041CBE0 = __ARQRequestQueueHi_8041CBD0;

    __ARQRequestQueueHi_8041CBD0 = __ARQRequestQueueHi_8041CBD0->next;
  }
}

void __ARQServiceQueueLo(void) {

  if ((__ARQRequestPendingLo_8041CBE4 == NULL) && (__ARQRequestQueueLo_8041CBD8)) {
    __ARQRequestPendingLo_8041CBE4 = __ARQRequestQueueLo_8041CBD8;

    __ARQRequestQueueLo_8041CBD8 = __ARQRequestQueueLo_8041CBD8->next;
  }

  if (__ARQRequestPendingLo_8041CBE4) {
    if (__ARQRequestPendingLo_8041CBE4->length <= __ARQChunkSize_8041CBF0) {
      if (__ARQRequestPendingLo_8041CBE4->type == ARQ_TYPE_MRAM_TO_ARAM)
        ARStartDMA(__ARQRequestPendingLo_8041CBE4->type, __ARQRequestPendingLo_8041CBE4->source,
                   __ARQRequestPendingLo_8041CBE4->dest, __ARQRequestPendingLo_8041CBE4->length);
      else
        ARStartDMA(__ARQRequestPendingLo_8041CBE4->type, __ARQRequestPendingLo_8041CBE4->dest,
                   __ARQRequestPendingLo_8041CBE4->source, __ARQRequestPendingLo_8041CBE4->length);

      __ARQCallbackLo_8041CBEC = __ARQRequestPendingLo_8041CBE4->callback;
    } else {
      if (__ARQRequestPendingLo_8041CBE4->type == ARQ_TYPE_MRAM_TO_ARAM)
        ARStartDMA(__ARQRequestPendingLo_8041CBE4->type, __ARQRequestPendingLo_8041CBE4->source,
                   __ARQRequestPendingLo_8041CBE4->dest, __ARQChunkSize_8041CBF0);
      else
        ARStartDMA(__ARQRequestPendingLo_8041CBE4->type, __ARQRequestPendingLo_8041CBE4->dest,
                   __ARQRequestPendingLo_8041CBE4->source, __ARQChunkSize_8041CBF0);
    }

    __ARQRequestPendingLo_8041CBE4->length -= __ARQChunkSize_8041CBF0;
    __ARQRequestPendingLo_8041CBE4->source += __ARQChunkSize_8041CBF0;
    __ARQRequestPendingLo_8041CBE4->dest += __ARQChunkSize_8041CBF0;
  }
}

void __ARQCallbackHack(u32 pointerToARQRequest) { return; }

void __ARQInterruptServiceRoutine(void) {

  if (__ARQCallbackHi_8041CBE8) {
    (*__ARQCallbackHi_8041CBE8)((u32)__ARQRequestPendingHi_8041CBE0);
    __ARQRequestPendingHi_8041CBE0 = NULL;
    __ARQCallbackHi_8041CBE8 = NULL;
  }

  else if (__ARQCallbackLo_8041CBEC) {
    (*__ARQCallbackLo_8041CBEC)((u32)__ARQRequestPendingLo_8041CBE4);
    __ARQRequestPendingLo_8041CBE4 = NULL;
    __ARQCallbackLo_8041CBEC = NULL;
  }

  __ARQPopTaskQueueHi();

  if (__ARQRequestPendingHi_8041CBE0 == NULL)
    __ARQServiceQueueLo();
}

void ARQInit(void) {

  if (__ARQ_init_flag_8041CBF4 == TRUE) {
    return;
  }

  __ARQRequestQueueHi_8041CBD0 = __ARQRequestQueueLo_8041CBD8 = NULL;
  __ARQChunkSize_8041CBF0 = 4096;
  ARRegisterDMACallback(&__ARQInterruptServiceRoutine);
  __ARQRequestPendingHi_8041CBE0 = NULL;
  __ARQRequestPendingLo_8041CBE4 = NULL;
  __ARQCallbackHi_8041CBE8 = NULL;
  __ARQCallbackLo_8041CBEC = NULL;
  __ARQ_init_flag_8041CBF4 = TRUE;
}

void ARQPostRequest(ARQRequest *request, u32 owner, u32 type, u32 priority, u32 source, u32 dest,
                    u32 length, ARQCallback callback) {

  BOOL enabled;

  request->next = NULL;
  request->owner = owner;
  request->type = type;
  request->source = source;
  request->dest = dest;
  request->length = length;

  if (callback) {
    request->callback = callback;
  } else {
    request->callback = (ARQCallback)&__ARQCallbackHack;
  }

  enabled = OSDisableInterrupts();

  switch (priority) {
  case ARQ_PRIORITY_LOW:

    if (__ARQRequestQueueLo_8041CBD8) {
      __ARQRequestTailLo_8041CBDC->next = request;
    } else {
      __ARQRequestQueueLo_8041CBD8 = request;
    }
    __ARQRequestTailLo_8041CBDC = request;

    break;

  case ARQ_PRIORITY_HIGH:

    if (__ARQRequestQueueHi_8041CBD0) {
      __ARQRequestTailHi_8041CBD4->next = request;
    } else {
      __ARQRequestQueueHi_8041CBD0 = request;
    }

    __ARQRequestTailHi_8041CBD4 = request;

    break;
  }

  if ((__ARQRequestPendingHi_8041CBE0 == NULL) && (__ARQRequestPendingLo_8041CBE4 == NULL)) {
    __ARQPopTaskQueueHi();

    if (__ARQRequestPendingHi_8041CBE0 == NULL) {
      __ARQServiceQueueLo();
    }
  }

  OSRestoreInterrupts(enabled);
}
