#ifndef _DOLPHIN_OSINTERRUPT
#define _DOLPHIN_OSINTERRUPT

#include <dolphin/os.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef u32 OSInterruptMask;
typedef void (*__OSInterruptHandler)(__OSInterrupt interrupt, OSContext* context);

__OSInterruptHandler __OSSetInterruptHandler(__OSInterrupt interrupt, __OSInterruptHandler handler);
__OSInterruptHandler __OSGetInterruptHandler(__OSInterrupt interrupt);
OSInterruptMask __OSMaskInterrupts(OSInterruptMask mask);

#ifdef __cplusplus
}
#endif

#endif
