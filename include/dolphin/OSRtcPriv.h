/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#ifndef _DOLPHIN_OSRTCPRIV
#define _DOLPHIN_OSRTCPRIV

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

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
