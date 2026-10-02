#ifndef _DOLPHIN_AR
#define _DOLPHIN_AR

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ARCallback)(void);

ARCallback ARRegisterDMACallback(ARCallback callback);
void ARStartDMA(u32 type, u32 mainmem_addr, u32 aram_addr, u32 length);

u32 ARInit(u32* stack_index_addr, u32 num_entries);
u32 ARGetBaseAddress(void);
u32 ARGetSize(void);

#ifdef __cplusplus
}
#endif

#endif
