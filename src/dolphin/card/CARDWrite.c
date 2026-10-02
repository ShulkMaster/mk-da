/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/CARDPriv.h>
#include <dolphin/os.h>

s32 __CARDSeek(CARDFileInfo* fileInfo, s32 length, s32 offset, CARDControl** card);
void __CARDDefaultApiCallback(s32 chan, s32 result);
#define OFFSET(n, a) (((u32)(n)) & ((a)-1))
#define CARD_RESULT_CANCELED -14
static void EraseCallback(s32 chan, s32 result);

static void WriteCallback(s32 chan, s32 result) {
  CARDControl* card;
  CARDCallback callback;
  u16* fat;
  CARDDir* dir;
  CARDDir* ent;
  CARDFileInfo* fileInfo;

  card = &__CARDBlock[chan];
  do {
    if (result < 0) {
      break;
    }

    fileInfo = card->fileInfo;
    if (fileInfo->length < 0) {
      result = CARD_RESULT_CANCELED;
      break;
    }

    fileInfo->length -= card->sectorSize;
    if (fileInfo->length <= 0) {
      dir = __CARDGetDirBlock(card);
      ent = &dir[fileInfo->fileNo];
      ent->time = (u32)OSTicksToSeconds(OSGetTime());
      callback = card->apiCallback;
      card->apiCallback = 0;
      result = __CARDUpdateDir(chan, callback);
    } else {
      fat = __CARDGetFatBlock(card);
      fileInfo->offset += card->sectorSize;
      fileInfo->iBlock = fat[fileInfo->iBlock];
      if (!CARDIsValidBlockNo(card, fileInfo->iBlock)) {
        result = CARD_RESULT_BROKEN;
        break;
      }
      result = __CARDEraseSector(chan, card->sectorSize * (u32)fileInfo->iBlock, EraseCallback);
    }

    if (result < 0) {
      break;
    }
    return;
  } while (0);

  callback = card->apiCallback;
  card->apiCallback = 0;
  __CARDPutControlBlock(card, result);
  callback(chan, result);
}

static void EraseCallback(s32 chan, s32 result) {
  CARDControl* card;
  CARDCallback callback;
  CARDFileInfo* fileInfo;

  card = &__CARDBlock[chan];
  do {
    if (result < 0) {
      break;
    }

    fileInfo = card->fileInfo;
    result = __CARDWrite(chan, card->sectorSize * (u32)fileInfo->iBlock, card->sectorSize,
                         card->buffer, WriteCallback);
    if (result < 0) {
      break;
    }
    return;
  } while (0);

  callback = card->apiCallback;
  card->apiCallback = 0;
  __CARDPutControlBlock(card, result);
  callback(chan, result);
}

s32 CARDWriteAsync(CARDFileInfo* fileInfo, const void* buf, s32 length, s32 offset,
                   CARDCallback callback) {
  CARDControl* card;
  s32 result;
  CARDDir* dir;
  CARDDir* ent;

  result = __CARDSeek(fileInfo, length, offset, &card);
  if (result < 0) {
    return result;
  }

  if (OFFSET(offset, card->sectorSize) != 0 || OFFSET(length, card->sectorSize) != 0) {
    return __CARDPutControlBlock(card, CARD_RESULT_FATAL_ERROR);
  }

  dir = __CARDGetDirBlock(card);
  ent = &dir[fileInfo->fileNo];
  result = __CARDAccess(card, ent);
  if (result < 0) {
    return __CARDPutControlBlock(card, result);
  }

  DCStoreRange((void*)buf, (u32)length);
  card->apiCallback = callback ? callback : __CARDDefaultApiCallback;
  card->buffer = (void*)buf;
  result =
      __CARDEraseSector(fileInfo->chan, card->sectorSize * (u32)fileInfo->iBlock, EraseCallback);
  if (result < 0) {
    __CARDPutControlBlock(card, result);
  }
  return result;
}

s32 CARDWrite(CARDFileInfo* fileInfo, void* buffer, s32 length, s32 offset) {
  s32 result = CARDWriteAsync(fileInfo, buffer, length, offset,
                               __CARDSyncCallback);
  if (result < 0) {
    return result;
  }
  return __CARDSync(fileInfo->chan);
}
