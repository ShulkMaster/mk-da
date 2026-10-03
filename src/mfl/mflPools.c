#include <mfl/mflPools.h>
#include <msl/mslMem.h>
#include <msl/mslMusyXUtil.h>
#include <string.h>
#include <stdio.h>

static mflFile* mflFileNextFree;
static struct mflZipArchive* mflZipArchiveNextFree;
static mflZFile* mflZFileNextFree;
static mflFileCommand* mflFileCommandNextFree;
static mflQueue* mflQueueNextFree;
static mflQueue* mflQueuePool;
static mflFileCommand* mflFileCommandPool;
static mflZFile* mflZFilePool;
static struct mflZipArchive* mflZipArchivePool;
static DVDFileInfo* DVDFileInfoPool;
static mflFile* mflFilePool;

static inline mflQueue* mflQueueFindFree(mflQueue** nextFree, mflQueue* first,
    mflQueue* last) {
  mflQueue* current = *nextFree;
  mflQueue* result;

  if (current == 0) {
    for (current = first; current <= last; ++current) {
      if (!current->allocated) {
        break;
      }
    }
    if (current > last) {
      return 0;
    }
  }
  result = current;
  do {
    ++current;
    if (current > last) {
      current = first;
    }
  } while (current->allocated && current != result);
  if (current == result) {
    current = 0;
  }
  *nextFree = current;
  return result;
}

mflQueue* mflQueueAlloc(void) {
  mflQueue* result;
  mflQueue* pool;

  disableIRQ();
  pool = mflQueuePool;
  result = mflQueueFindFree(&mflQueueNextFree, pool, pool);
  if (result != 0) {
    memset(result, 0, sizeof(*result));
    result->allocated = 1;
  }
  enableIRQ();
  return result;
}

void mflFileCommandFreePending(void) {
  s32 i;
  mflFileCommand* command;

  disableIRQ();
  command = mflFileCommandPool;
  for (i = 0; i < 16; ++i, ++command) {
    if (command->flags.allocated && command->flags.done &&
        command->flags.pendingFree) {
      command->flags.allocated = 0;
    }
  }
  enableIRQ();
}

void mflFileCommandFree(mflFileCommand* command) {
  disableIRQ();
  command->flags.allocated = 0;
  enableIRQ();
}

static inline mflFileCommand* mflFileCommandFindFree(
    mflFileCommand** nextFree, mflFileCommand* first, mflFileCommand* last) {
  mflFileCommand* current = *nextFree;
  mflFileCommand* result;

  if (current == 0) {
    for (current = first; current <= last; ++current) {
      if (!current->flags.allocated) {
        break;
      }
    }
    if (current > last) {
      return 0;
    }
  }
  result = current;
  do {
    ++current;
    if (current > last) {
      current = first;
    }
  } while (current->flags.allocated && current != result);
  if (current == result) {
    current = 0;
  }
  *nextFree = current;
  return result;
}

mflFileCommand* mflFileCommandAlloc(void) {
  mflFileCommand* last;
  mflFileCommand* pool;
  mflFileCommand* result;

  disableIRQ();
  last = (pool = mflFileCommandPool) + 15;
  result = mflFileCommandFindFree(&mflFileCommandNextFree, pool, last);
  if (result != 0) {
    memset(result, 0, sizeof(*result));
    result->flags.allocated = 1;
  } else {
    printf("mflFileCommandAlloc: Unable to allocate file command, pool empty.\n");
  }
  enableIRQ();
  return result;
}

void mflZFileFree(mflZFile* file) {
  disableIRQ();
  file->file.flags.allocated = 0;
  enableIRQ();
}

static inline mflZFile* mflZFileFindFree(mflZFile** nextFree, mflZFile* first,
    mflZFile* last) {
  mflZFile* current = *nextFree;
  mflZFile* result;

  if (current == 0) {
    for (current = first; current <= last; ++current) {
      if (!current->file.flags.allocated) {
        break;
      }
    }
    if (current > last) {
      return 0;
    }
  }
  result = current;
  do {
    ++current;
    if (current > last) {
      current = first;
    }
  } while (current->file.flags.allocated && current != result);
  if (current == result) {
    current = 0;
  }
  *nextFree = current;
  return result;
}

mflZFile* mflZFileAlloc(void) {
  mflZFile* last;
  mflZFile* pool;
  mflZFile* result;

  disableIRQ();
  last = (pool = mflZFilePool) + 9;
  result = mflZFileFindFree(&mflZFileNextFree, pool, last);
  if (result != 0) {
    memset(result, 0, sizeof(*result));
    result->file.flags.allocated = 1;
  }
  enableIRQ();
  return result;
}

void mflFileFree(mflFile* file) {
  disableIRQ();
  mlHeapFree(file->buffer);
  file->flags.allocated = 0;
  enableIRQ();
}

static inline mflFile* mflFileFindFree(mflFile** nextFree, mflFile* first,
    mflFile* last) {
  mflFile* current = *nextFree;
  mflFile* result;

  if (current == 0) {
    for (current = first; current <= last; ++current) {
      if (!current->flags.allocated) {
        break;
      }
    }
    if (current > last) {
      return 0;
    }
  }
  result = current;
  do {
    ++current;
    if (current > last) {
      current = first;
    }
  } while (current->flags.allocated && current != result);
  if (current == result) {
    current = 0;
  }
  *nextFree = current;
  return result;
}

mflFile* mflFileAlloc(void) {
  mflFile* last;
  mflFile* pool;
  mflFile* result;

  disableIRQ();
  last = (pool = mflFilePool) + 15;
  result = mflFileFindFree(&mflFileNextFree, pool, last);
  if (result != 0) {
    memset(result, 0, sizeof(*result));
    result->flags.allocated = 1;
    result->buffer = mlAlignedHeapAlloc(MSLMFL_HEAP, 0x800);
    result->dvdInfo = DVDFileInfoPool +
        ((u32)result - (u32)mflFilePool) / sizeof(*result);
  }
  enableIRQ();
  return result;
}

void mflInitializePools(mflPoolConfig config) {
  if (config.flags.files) {
    if (mflFilePool == 0) {
      mflFilePool = mflFileNextFree = mlAlignedHeapAlloc(MSLMFL_HEAP, 0x13C0);
      memset(mflFilePool, 0, 0x13C0);
      DVDFileInfoPool = mlAlignedHeapAlloc(MSLMFL_HEAP, 0x3C0);
      memset(DVDFileInfoPool, 0, 0x3C0);
    }
  } else if (mflFilePool != 0) {
    mlHeapFree(mflFilePool);
    mflFileNextFree = 0;
    mflFilePool = 0;
    if (DVDFileInfoPool != 0) {
      mlHeapFree(DVDFileInfoPool);
      DVDFileInfoPool = 0;
    }
  }
  if (config.flags.zip) {
    if (mflZipArchivePool == 0) {
      mflZipArchivePool = mflZipArchiveNextFree = mlHeapAlloc(MSLMFL_HEAP, 0xC0);
      memset(mflZipArchivePool, 0, 0xC0);
    }
    if (mflZFilePool == 0) {
      mflZFilePool = mflZFileNextFree = mlAlignedHeapAlloc(MSLMFL_HEAP, 0xCF8);
      memset(mflZFilePool, 0, 0xCF8);
    }
  } else {
    if (mflZipArchivePool != 0) {
      mlHeapFree(mflZipArchivePool);
      mflZipArchiveNextFree = 0;
      mflZipArchivePool = 0;
    }
    if (mflZFilePool != 0) {
      mlHeapFree(mflZFilePool);
      mflZFileNextFree = 0;
      mflZFilePool = 0;
    }
  }
  if (config.flags.commands) {
    if (mflFileCommandPool == 0) {
      mflFileCommandPool = mflFileCommandNextFree = mlHeapAlloc(MSLMFL_HEAP, 0x13C0);
      memset(mflFileCommandPool, 0, 0x13C0);
    }
  } else if (mflFileCommandPool != 0) {
    mlHeapFree(mflFileCommandPool);
    mflFileCommandNextFree = 0;
    mflFileCommandPool = 0;
  }
  if (config.flags.queues) {
    if (mflQueuePool == 0) {
      mflQueuePool = mflQueueNextFree = mlHeapAlloc(MSLMFL_HEAP, 0xC);
      memset(mflQueuePool, 0, 0xC);
    }
  } else if (mflQueuePool != 0) {
    mlHeapFree(mflQueuePool);
    mflQueueNextFree = 0;
    mflQueuePool = 0;
  }
}
