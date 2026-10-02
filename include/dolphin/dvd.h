#ifndef _DOLPHIN_DVD
#define _DOLPHIN_DVD

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DVDDiskID DVDDiskID;
typedef struct DVDCommandBlock DVDCommandBlock;
typedef struct DVDFileInfo DVDFileInfo;
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

typedef void (*DVDCallback)(s32 result, DVDFileInfo* fileInfo);

struct DVDFileInfo {
  DVDCommandBlock cb;
  u32 startAddr;
  u32 length;
  DVDCallback callback;
};

BOOL DVDClose(DVDFileInfo* fileInfo);
s32 DVDCancel(DVDCommandBlock* block);
void DVDReset(void);
s32 DVDGetCommandBlockStatus(const DVDCommandBlock* block);
void __DVDLowSetWAType(u32 type, u32 location);

#ifdef __cplusplus
}
#endif

#endif
