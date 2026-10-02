#ifndef _DOLPHIN_EXI
#define _DOLPHIN_EXI
#include <dolphin/os/OSContext.h>
typedef void (*EXICallback)(s32 chan, OSContext* context);
#define EXI_READ 0
#define EXI_WRITE 1
#define EXI_FREQ_1M 0
#define EXI_FREQ_8M 3
#define EXI_STATE_ATTACHED 8
#define EXI_USB_ADAPTER 0x01010000
u32 EXIClearInterrupts(s32 chan, BOOL exi, BOOL tc, BOOL ext);
EXICallback EXISetExiCallback(s32 chan, EXICallback callback);
BOOL EXIProbe(s32 chan);
s32 EXIProbeEx(s32 chan);
BOOL EXIAttach(s32 chan, EXICallback callback);
BOOL EXIDetach(s32 chan);
s32 EXIGetID(s32 chan, u32 dev, u32* id);
void EXIInit(void);
u32 EXIGetState(s32 chan);
BOOL EXILock(s32 chan, u32 dev, EXICallback unlockedCallback);
BOOL EXIUnlock(s32 chan);
BOOL EXISelect(s32 chan, u32 dev, u32 freq);
BOOL EXIDeselect(s32 chan);
BOOL EXIImm(s32 chan, void* buffer, s32 length, u32 type, EXICallback callback);
BOOL EXIImmEx(s32 chan, void* buffer, s32 length, u32 type);
BOOL EXIDma(s32 chan, void* buffer, s32 length, u32 type, EXICallback callback);
BOOL EXISync(s32 chan);
#endif
