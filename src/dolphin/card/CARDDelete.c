/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/CARDPriv.h>
#include <dolphin/os.h>

s32 __CARDFreeBlock(s32 chan, u16 block, CARDCallback callback);
s32 __CARDGetFileNo(CARDControl* card, const char* fileName, s32* fileNo);
BOOL __CARDIsOpened(CARDControl* card, s32 fileNo);
void __CARDDefaultApiCallback(s32 chan, s32 result);
#define CARD_RESULT_BUSY -1

static void DeleteCallback(s32 chan, s32 result) {
  CARDControl* card;
  CARDCallback callback;

  card = &__CARDBlock[chan];
  callback = card->apiCallback;
  card->apiCallback = 0;

  if (result < 0) {
    goto error;
  }

  result = __CARDFreeBlock(chan, card->startBlock, callback);
  if (result < 0) {
    goto error;
  }
  return;

error:
  __CARDPutControlBlock(card, result);
  if (callback) {
    callback(chan, result);
  }
}

s32 CARDDeleteAsync(s32 chan, const char* fileName, CARDCallback callback) {
  CARDControl* card;
  s32 fileNo;
  s32 result;
  CARDDir* dir;
  CARDDir* ent;

  result = __CARDGetControlBlock(chan, &card);
  if (result < 0) {
    return result;
  }
  result = __CARDGetFileNo(card, fileName, &fileNo);
  if (result < 0) {
    return __CARDPutControlBlock(card, result);
  }
  if (__CARDIsOpened(card, fileNo)) {
    return __CARDPutControlBlock(card, CARD_RESULT_BUSY);
  }

  dir = __CARDGetDirBlock(card);
  ent = &dir[fileNo];
  card->startBlock = ent->startBlock;
  memset(ent, 0xff, sizeof(CARDDir));

  card->apiCallback = callback ? callback : __CARDDefaultApiCallback;
  result = __CARDUpdateDir(chan, DeleteCallback);
  if (result < 0) {
    __CARDPutControlBlock(card, result);
  }
  return result;
}

s32 CARDDelete(s32 chan, const char* fileName) {
  s32 result;

  result = CARDDeleteAsync(chan, fileName, __CARDSyncCallback);
  if (result < 0) {
    return result;
  }
  return __CARDSync(chan);
}
