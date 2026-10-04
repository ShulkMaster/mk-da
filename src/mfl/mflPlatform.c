#include <mfl/mflPlatform.h>
#include <mfl/mflPools.h>
#include <dolphin/dvd.h>
#include <dolphin/os.h>
#include <ctype.h>
#include <string.h>
#include <msl/mslMusyXUtil.h>

static u64 totalDVDReads;
static OSTime totalDVDReadTime;
static OSTime startDVDReadTime;
static s32 GcnDiscErrorStatus;

void Trim(char* destination, const char* source) {
  const char* first;
  const char* last;
  s32 count;
  if (destination == NULL || source == NULL) {
    return;
  }
  first = source;
  while (*first != '\0' && isspace(*first)) {
    first++;
  }
  if (*first == '\0') {
    *destination = '\0';
    return;
  }
  last = first + strlen(first) - 1;
  while (isspace(*last)) {
    last--;
  }
  count = last - first + 1;
  strncpy(destination, source, count);
  destination[count] = '\0';
}

void InsertUnderscores(char* filename) {
  while (*filename != '\0') {
    if (isspace(*filename)) {
      *filename = '_';
    }
    filename++;
  }
}

void SpoofExtensions(char* filename) {
  if (strstr(filename, ".rtd") != NULL) {
    strtok(filename, ".");
    strcat(filename, ".gtd");
  }
  if (strstr(filename, ".pss") != NULL) {
    strtok(filename, ".");
    strcat(filename, ".avi");
  }
}

s32 mflGcnDvdExists(const char* filename) {
  return DVDConvertPathToEntrynum((char*)filename) != -1;
}

mflFile* mflGcnDvdOpen(const char* filename, const char* mode) {
  char normalized[0x100];
  char savedName[0x100];
  mflFile* file;
  /* Retail sets modeU regardless of the requested mode. */
  file = mflFileAlloc();
  if (file != NULL) {
    Trim(normalized, filename);
    InsertUnderscores(normalized);
    _strlwr(normalized);
    SpoofExtensions(normalized);
    strcpy(savedName, normalized);
    if (DVDOpen(normalized, file->dvdInfo)) {
      strncpy(file->filename, savedName, sizeof(file->filename));
      file->filename[sizeof(file->filename) - 1] = '\0';
      file->flags.allocated = 1;
      file->flags.open = 1;
      file->flags.unk08 = 0;
      file->flags.modeU = 1;
      file->position = 0;
      file->size = file->dvdInfo->length;
      file->bufferOffset = -1;
      file->bufferBytes = 0;
      file->lastResult = 0;
    } else {
      mflFileFree(file);
      file = NULL;
    }
  }
  return file;
}

void mflGcnDvdClose(mflFile* file) {
  /* Only an opened DVD file owns a live drive request. */
  if (file->flags.allocated && file->flags.open && !file->flags.zip) {
    DVDClose(file->dvdInfo);
    file->flags.open = 0;
    mflFileFree(file);
  }
}

s32 mflGcnDvdReadStart(void* destination, u32 elementSize,
                        u32 elementCount, mflFile* file) {
  if (!file->flags.eof) {
    totalDVDReads++;
    startDVDReadTime = OSGetTime();
    file->readSize = elementSize * elementCount;
    file->readProgress = 0;
    file->readDestination = destination;
    file->currentDestination = file->readDestination;
    if (file->flags.modeU &&
        (((u32)destination + 31) & ~31U) == (u32)destination &&
        ((file->readSize + 31) & ~31U) == file->readSize &&
        ((file->position + 3) & ~3U) == file->position) {
      file->readFlags.directDMA = 1;
      file->blockIndex = -1;
      file->blockOffset = -1;
      if (!DVDReadAsyncPrio(file->dvdInfo, file->readDestination, file->readSize,
                           file->position, mflGcnDvdCallback, 2)) {
        file->lastResult = 0;
        return 0;
      }
      return 1;
    }
    file->readFlags.directDMA = 0;
    file->blockIndex = (s32)file->position / 0x800;
    file->blockOffset = (s32)file->position % 0x800;
    return mflGcnDvdReadContinue(file);
  }
  return 0;
}

s32 mflGcnDvdReadContinue(mflFile* file) {
  s32 available;
  s32 remaining;
  s32 offset;
  u32 bytes;
  u32 alignedBytes;
  if (file->bufferOffset == file->blockIndex) {
    remaining = file->readSize - file->readProgress;
    available = file->bufferBytes - file->blockOffset;
    if (remaining <= available) {
      DCInvalidateRange(file->buffer, 0x800);
      memcpy(file->currentDestination,
             (u8*)file->buffer + file->blockOffset, remaining);
      file->readProgress += remaining;
      file->position += remaining;
      if (file->position >= file->size) {
        file->position = file->size;
        file->flags.eof = 1;
      }
      file->lastResult = file->readProgress;
      DCFlushRange(file->readDestination, file->readProgress);
      totalDVDReadTime += OSGetTime() - startDVDReadTime;
      return 0;
    }
    DCInvalidateRange(file->buffer, 0x800);
    memcpy(file->currentDestination,
           (u8*)file->buffer + file->blockOffset, available);
    file->currentDestination += available;
    file->position += available;
    file->readProgress += available;
    if (file->position >= file->size) {
      file->position = file->size;
      file->flags.eof = 1;
    }
    file->blockIndex++;
    file->blockOffset = 0;
  }
  offset = file->blockIndex * 0x800;
  bytes = file->size - offset;
  if (bytes > 0x800) {
    bytes = 0x800;
  }
  file->skip = 0;
  alignedBytes = (bytes + 31) & ~31;
  if (alignedBytes != bytes) {
    file->skip = alignedBytes - bytes;
    bytes = alignedBytes;
  }
  if (!DVDReadAsyncPrio(file->dvdInfo, file->buffer, bytes,
                       offset, mflGcnDvdCallback, 2)) {
    file->lastResult = 0;
    return 0;
  }
  return 1;
}

u32 mflGcnDvdReadProgress(mflFile* file) {
  union {
    s32 progress;
    u32 byteCount;
  } snapshot;
  disableIRQ();
  snapshot.progress = file->readProgress;
  enableIRQ();
  return snapshot.byteCount;
}

void mflGcnDvdReadCancel(mflFile* file) {
  DVDCancel(&file->dvdInfo->cb);
}

s32 mflGcnDvdSeek(mflFile* file, s32 offset, u16 origin) {
  switch (origin) {
  case 0:
    file->position = offset;
    break;
  case 1:
    file->position += offset;
    break;
  case 2:
    file->position = file->size - offset;
    break;
  }
  file->flags.eof = 0;
  if ((s32)file->position < 0 || file->position > file->size) {
    return -1;
  }
  if (file->position == file->size) {
    file->flags.eof = 1;
  }
  return 0;
}

u32 mflGcnDvdTell(mflFile* file) {
  return file->position;
}

s32 mflGcnDvdEof(mflFile* file) {
  return file->flags.eof;
}

void mflGcnDvdCallback(s32 result, DVDFileInfo* info) {
  mflFileCommand* command;
  mflFile* file;
  s32 status;
  /* The active queue entry owns the file for this DVD completion. */
  command = (mflFileCommand*)mflQueueGet();
  file = command->file;
  status = DVDGetCommandBlockStatus(&file->dvdInfo->cb);
  switch (status) {
  case 0:
    if (file->readFlags.directDMA) {
      file->lastResult = result;
      file->readProgress = result;
      file->position += result;
      if (file->position >= file->size) {
        file->position = file->size;
        file->flags.eof = 1;
      }
      DCInvalidateRange(file->readDestination, file->readProgress);
    } else if (!file->flags.eof && file->readProgress != file->readSize) {
      file->bufferBytes = result;
      file->bufferOffset = file->blockIndex;
      if (file->skip != 0) {
        file->bufferBytes -= file->skip;
        file->skip = 0;
      }
      if (mflGcnDvdReadContinue(file)) {
        return;
      }
    }
    command->readProgress = file->readProgress;
    mflQueueRemoveFirst();
    command->state = 3;
    command->flags.done = 1;
    if (command != NULL && command->callback != NULL) {
      command->callback(command->file->lastResult, command->user);
    }
    break;
  case 10:
    mflQueueRemoveFirst();
    command->state = 3;
    command->readProgress = -1;
    file->lastResult = -1;
    command->flags.done = 1;
    if (command != NULL && command->callback != NULL) {
      command->callback(command->file->lastResult, command->user);
    }
    break;
  default:
    mflGcnDvdDriveErrorCheck(status);
    break;
  }
}

s32 mflGcnDvdDriveErrorCheck(s32 status) {
  if (status == 0) {
    status = DVDGetDriveStatus();
  }
  switch (status) {
  case -1:
  case 4:
  case 5:
  case 6:
  case 11:
    GcnDiscErrorStatus = status;
    return 0;
  default:
    GcnDiscErrorStatus = 0;
    return 1;
  }
}

s32 mflGcnDvdGetDriveErrorStatus(void) {
  return GcnDiscErrorStatus;
}

void mflPlatformStatisticsReset(void) {
  totalDVDReads = 0;
  totalDVDReadTime = 0;
}
