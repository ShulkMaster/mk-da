#ifndef _DOLPHIN_VI
#define _DOLPHIN_VI
#include <dolphin/gx.h>
#ifdef __cplusplus
extern "C" {
#endif
VIRetraceCallback VISetPreRetraceCallback(VIRetraceCallback cb);
VIRetraceCallback VISetPostRetraceCallback(VIRetraceCallback cb);
void __VIInit(VITVMode mode);
void VIInit(void);
void VIWaitForRetrace(void);
void VIConfigure(const GXRenderModeObj *rm);
void VIFlush(void);
void VISetNextFrameBuffer(void *fb);
void VISetBlack(BOOL black);
u32 VIGetNextField(void);
u32 VIGetCurrentLine(void);
u32 VIGetTvFormat(void);
#ifdef __cplusplus
}
#endif
#endif
