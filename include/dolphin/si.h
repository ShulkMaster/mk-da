#ifndef _DOLPHIN_SI
#define _DOLPHIN_SI

#include <dolphin/os/OSInterrupt.h>

#define SI_MAX_CHAN 4
#define SI_CHAN0_BIT 0x80000000
#define SI_MAX_COMCSR_INLNGTH 128
#define SI_MAX_COMCSR_OUTLNGTH 128
#define SI_ERROR_UNDER_RUN 0x0001
#define SI_ERROR_OVER_RUN 0x0002
#define SI_ERROR_COLLISION 0x0004
#define SI_ERROR_NO_RESPONSE 0x0008
#define SI_ERROR_RDST 0x0020
#define SI_ERROR_UNKNOWN 0x0040
#define SI_ERROR_BUSY 0x0080
#define SI_TYPE_MASK 0x18000000u
#define SI_TYPE_N64 0x00000000u
#define SI_TYPE_DOLPHIN 0x08000000u
#define SI_TYPE_GC SI_TYPE_DOLPHIN
#define SI_GC_WIRELESS 0x80000000u
#define SI_GC_STANDARD 0x01000000u       // dolphin standard controller
#define SI_WIRELESS_RECEIVED 0x40000000u // 0: no wireless unit
#define SI_WIRELESS_IR 0x04000000u       // 0: IR  1: RF
#define SI_WIRELESS_STATE 0x02000000u    // 0: variable  1: fixed
#define SI_WIRELESS_FIX_ID 0x00100000u   // 0: not fixed  1: fixed
#define SI_WIRELESS_TYPE 0x000f0000u
#define SI_WIRELESS_ID 0x00c0ff00u
#define SI_WIRELESS_TYPE_ID (SI_WIRELESS_TYPE | SI_WIRELESS_ID)
#define SI_N64_CONTROLLER (SI_TYPE_N64 | 0x05000000)
#define SI_N64_MIC (SI_TYPE_N64 | 0x00010000)
#define SI_N64_KEYBOARD (SI_TYPE_N64 | 0x00020000)
#define SI_N64_MOUSE (SI_TYPE_N64 | 0x02000000)
#define SI_GBA (SI_TYPE_N64 | 0x00040000)
#define SI_GC_CONTROLLER (SI_TYPE_GC | SI_GC_STANDARD)
#define SI_GC_RECEIVER (SI_TYPE_GC | SI_GC_WIRELESS)
#define SI_GC_WAVEBIRD                                                                             \
  (SI_TYPE_GC | SI_GC_WIRELESS | SI_GC_STANDARD | SI_WIRELESS_STATE | SI_WIRELESS_FIX_ID)
#define SI_GC_KEYBOARD (SI_TYPE_GC | 0x00200000)
#define SI_GC_STEERING (SI_TYPE_GC | 0x00000000)

#define SI_GC_NOMOTOR 0x20000000u  // no rumble motor

#define SI_WIRELESS_LITE 0x00040000u      // 0: normal 1: lite controller

#define SI_WIRELESS_CONT_MASK 0x00080000u // 0: non-controller 1: non-controller

#define SI_WIRELESS_CONT 0x00000000u

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*SICallback)(s32 chan, u32 sr, OSContext *context);
typedef void (*SITypeAndStatusCallback)(s32 chan, u32 type);

typedef struct SIPacket {
  s32 chan;
  void *output;
  u32 outputBytes;
  void *input;
  u32 inputBytes;
  SICallback callback;
  OSTime fire;
} SIPacket;

BOOL SIBusy(void);
BOOL SIIsChanBusy(s32 chan);
BOOL SIRegisterPollingHandler(__OSInterruptHandler handler);
BOOL SIUnregisterPollingHandler(__OSInterruptHandler handler);
void SIInit(void);
u32 SIGetStatus(s32 chan);
void SISetCommand(s32 chan, u32 command);
void SITransferCommands(void);
u32 SISetXY(u32 x, u32 y);
u32 SIEnablePolling(u32 poll);
u32 SIDisablePolling(u32 poll);
BOOL SIGetResponse(s32 chan, void *data);
BOOL SITransfer(s32 chan, void *output, u32 outputBytes, void *input, u32 inputBytes,
                SICallback callback, OSTime delay);
u32 SIGetType(s32 chan);
u32 SIGetTypeAsync(s32 chan, SITypeAndStatusCallback callback);
u32 SIDecodeType(u32 type);
u32 SIProbe(s32 chan);
void SISetSamplingRate(u32 msec);

#ifdef __cplusplus
}
#endif

#endif
