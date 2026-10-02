#ifndef _DOLPHIN_OSRTCPRIV
#define _DOLPHIN_OSRTCPRIV

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OSSram {
  u16 checkSum;
  u16 checkSumInv;
  u32 ead0;
  u32 ead1;
  u32 counterBias;
  s8 displayOffsetH;
  u8 ntd;
  u8 language;
  u8 flags;
} OSSram;

OSSram* __OSLockSram(void);
BOOL __OSUnlockSram(BOOL commit);

typedef struct OSSramEx {
  u8 flashID[2][12];
  u32 wirelessKeyboardID;
  u16 wirelessPadID[4];
  u8 dvdErrorCode;
  u8 _padding0;
  u8 flashIDCheckSum[2];
  u16 gbs;
  u8 _padding1[2];
} OSSramEx;

OSSramEx* __OSLockSramEx(void);
BOOL __OSUnlockSramEx(BOOL commit);

#ifdef __cplusplus
}
#endif

#endif
