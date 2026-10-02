/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#ifndef _DOLPHIN_DVD
#define _DOLPHIN_DVD

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DVDDiskID DVDDiskID;
typedef struct DVDCommandBlock DVDCommandBlock;
typedef void (*DVDCBCallback)(s32 result, DVDCommandBlock* block);

struct DVDCommandBlock {
  DVDCommandBlock* next;
  DVDCommandBlock* prev;
  u32 command;
  s32 state;
  u32 offset;
  u32 length;
  void* addr;
  u32 currTransferSize;
  u32 transferredSize;
  DVDDiskID* id;
  DVDCBCallback callback;
  void* userData;
};

#ifdef __cplusplus
}
#endif

#endif
