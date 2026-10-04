#ifndef MFL_MFLZIP_H
#define MFL_MFLZIP_H

#include <dolphin/types.h>
#include <mfl/mflFile.h>
#include <mfl/mlSysCalls.h>

typedef struct mflZipEntry {
  const char* name;
  u32 size;
  u32 offset;
} mflZipEntry;

typedef struct mflZipArchive {
  struct mflZipArchive* next;
  u32 unk04;
  u32 entryCount;
  mflZipEntry* entries;
  mflFile* file;
  const char* name;
} mflZipArchive;

/* Retail mflZFileAlloc advances by 0x14C; mflZOpen copies the 0x13C
   common file prefix before setting the archive-specific fields. */
typedef struct mflZFile {
  mflFile file;
  struct mflZipArchive* archive;
  struct mflZipEntry* entry;
  s32 position;
  u32 size;
} mflZFile;

mlAsyncRequest* mflZSeekAsync(mflZFile* file, s32 offset, s32 origin,
    s32 priority, mlAsyncCallback callback, void* user);
mlAsyncRequest* mflZReadAsync(void* destination, s32 size, s32 count,
    mflZFile* file, s32 priority, mlAsyncCallback callback, void* user);
s32 mflZRead(void* destination, s32 size, s32 count, mflZFile* file);
s32 mflZSeek(mflZFile* file, s32 offset, s32 origin);
mflZFile* mflZOpen(const char* filename, const char* mode);
u32 mflZSize(const char* filename);
s32 mflZTell(mflZFile* file);
s32 mflZClose(mflZFile* file);

#endif
