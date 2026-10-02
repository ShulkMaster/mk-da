#ifndef _DOLPHIN_ARQ
#define _DOLPHIN_ARQ

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ARQ_TYPE_MRAM_TO_ARAM 0

typedef void (*ARQCallback)(u32 pointerToARQRequest);

typedef struct ARQRequest {
  struct ARQRequest* next;
  u32 owner;
  u32 type;
  u32 priority;
  u32 source;
  u32 dest;
  u32 length;
  ARQCallback callback;
} ARQRequest;

#ifdef __cplusplus
}
#endif

#endif
