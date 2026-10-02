#ifndef _DOLPHIN_AI
#define _DOLPHIN_AI

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*AIDCallback)(void);

AIDCallback AIRegisterDMACallback(AIDCallback callback);
void AIInitDMA(u32 addr, u32 length);
void AIStartDMA();
void AIStopDMA(void);
void AISetStreamPlayState(u32 state);
u32 AIGetStreamPlayState();
void AISetDSPSampleRate(u32 rate);
u32 AIGetDSPSampleRate();
u32 AIGetStreamSampleRate();
void AISetStreamVolLeft(u8 volume);
u8 AIGetStreamVolLeft();
void AISetStreamVolRight(u8 volume);
u8 AIGetStreamVolRight();

#ifdef __cplusplus
}
#endif

#endif
