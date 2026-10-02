#ifndef _DOLPHIN_CARDPRIV
#define _DOLPHIN_CARDPRIV
#include <dolphin/card.h>
#include <dolphin/dsp.h>
#include <dolphin/dvd.h>
#include <dolphin/os/OSThread.h>
#include <dolphin/os/OSAlarm.h>
#define CARD_PAGE_SIZE 128u
#define CARD_SEG_SIZE 512u
#define CARD_NUM_SYSTEM_BLOCK 5
#define CARD_SYSTEM_BLOCK_SIZE 8192u
typedef struct CARDDir {
  u8 gameName[4];
  u8 company[2];
  u8 _padding0;
  u8 bannerFormat;
  u8 fileName[CARD_FILENAME_MAX];
  u32 time; // seconds since 01/01/2000 midnight

  u32 iconAddr; // 0xffffffff if not used
  u16 iconFormat;
  u16 iconSpeed;

  u8 permission;
  u8 copyTimes;
  u16 startBlock;
  u16 length;
  u8 _padding1[2];

  u32 commentAddr; // 0xffffffff if not used
} CARDDir;

typedef struct CARDDirCheck {
  u8 padding0[64 - 2 * 4];
  u16 padding1;
  s16 checkCode;
  u16 checkSum;
  u16 checkSumInv;
} CARDDirCheck;

typedef struct CARDControl {
  BOOL attached;
  s32 result;
  u16 size;
  u16 pageSize;
  s32 sectorSize;
  u16 cBlock;
  u16 vendorID;
  s32 latency;
  u8 id[12];
  int mountStep;
  u32 scramble;
  int formatStep;
  DSPTaskInfo task;
  void* workArea;
  CARDDir* currentDir;
  u16* currentFat;
  OSThreadQueue threadQueue;
  u8 cmd[9];
  s32 cmdlen;
  vu32 mode;
  int retry;
  int repeat;
  u32 addr;
  void* buffer;
  s32 xferred;
  u16 freeNo;
  u16 startBlock;
  CARDFileInfo* fileInfo;
  CARDCallback extCallback;
  CARDCallback txCallback;
  CARDCallback exiCallback;
  CARDCallback apiCallback;
  CARDCallback xferCallback;
  CARDCallback eraseCallback;
  CARDCallback unlockCallback;
  OSAlarm alarm;
  u32 cid;
  const DVDDiskID* diskID;
} CARDControl;


#define CARDIsValidBlockNo(card, iBlock) \
  (CARD_NUM_SYSTEM_BLOCK <= (iBlock) && (iBlock) < (card)->cBlock)
#define __CARDGetDirCheck(dir) ((CARDDirCheck*)&(dir)[CARD_MAX_FILE])
extern CARDControl __CARDBlock[2];
u16* __CARDGetFatBlock(CARDControl* card);
CARDDir* __CARDGetDirBlock(CARDControl* card);
s32 __CARDPutControlBlock(CARDControl* card, s32 result);
s32 __CARDReadSegment(s32 chan, CARDCallback callback);
s32 __CARDWritePage(s32 chan, CARDCallback callback);
s32 __CARDWrite(s32 chan, u32 addr, s32 length, void* src, CARDCallback callback);
s32 __CARDEraseSector(s32 chan, u32 addr, CARDCallback callback);
s32 __CARDUpdateFatBlock(s32 chan, u16* fat, CARDCallback callback);
void __CARDCheckSum(void* ptr, int length, u16* checkSum, u16* checkSumInv);
void DCStoreRange(void* buffer, u32 length);
void* memcpy(void* dst, const void* src, unsigned long length);
typedef struct CARDID {
  u8 serial[32]; // flashID[12] + timebase[8] + counterBias[4] + language[4] + XXX[4]
  u16 deviceID;
  u16 size;
  u16 encode; // character set -- 0: S-JIS, 1: ANSI

  u8 padding[512 - 32 - 5 * 2];

  u16 checkSum;
  u16 checkSumInv;
} CARDID;

#define CARD_FAT_AVAIL 0u
#define CARD_FAT_CHECKSUM 0u
#define CARD_FAT_CHECKSUMINV 1u
#define CARD_FAT_CHECKCODE 2u
#define CARD_FAT_FREEBLOCKS 3u
extern DVDDiskID __CARDDiskNone;
s32 __CARDGetControlBlock(s32 chan, CARDControl** card);
s32 __CARDAccess(CARDControl* card, CARDDir* ent);
s32 __CARDIsPublic(CARDDir* ent);
s32 __CARDUpdateDir(s32 chan, CARDCallback callback);
void __CARDSyncCallback(s32 chan, s32 result);
s32 __CARDSync(s32 chan);
int memcmp(const void* a, const void* b, unsigned long length);
void* memset(void* dst, int value, unsigned long length);
#define CARD_MAX_MOUNT_STEP (CARD_NUM_SYSTEM_BLOCK + 2)
#define CARD_FAT_LASTSLOT 4u
extern u16 __CARDVendorID;
void __CARDExiHandler(s32 chan, OSContext* context);
void __CARDExtHandler(s32 chan, OSContext* context);
void __CARDTxHandler(s32 chan, OSContext* context);
void __CARDUnlockedHandler(s32 chan, OSContext* context);
void __CARDDefaultApiCallback(s32 chan, s32 result);
s32 __CARDReadStatus(s32 chan, u8* status);
s32 __CARDClearStatus(s32 chan);
s32 __CARDEnableInterrupt(s32 chan, BOOL enable);
s32 __CARDUnlock(s32 chan, u8* id);
s32 __CARDRead(s32 chan, u32 addr, s32 length, void* dst, CARDCallback callback);
s32 __CARDVerify(CARDControl* card);
void DCInvalidateRange(void* buffer, u32 length);
#endif
