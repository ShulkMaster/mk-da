#ifndef _DOLPHIN_AR
#define _DOLPHIN_AR

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ARCallback)(void);

ARCallback ARRegisterDMACallback(ARCallback callback);
void ARStartDMA(u32 type, u32 mainmem_addr, u32 aram_addr, u32 length);

#ifdef __cplusplus
}
#endif

#endif
