#include <mfl/mflZip.h>
#include <mfl/mflPools.h>
#include <mfl/mflPlatform.h>
#include <string.h>

static mflZipArchive mflZipArchiveHead;
static mlSysCalls mflZipSysCalls;




static mflZipEntry* mflZipArchiveEntrySearch(mflZipArchive* archive,
    const char* name) {
  s32 result;
  u32 low = 0;
  u32 middle;
  s32 found = 0;
  u32 high = archive->entryCount;
  mflZipEntry* entry;
  u32 i;

  while (!found && high - low > 2) {
    middle = low + ((high - low) >> 1);
    result = _stricmp(archive->entries[middle].name, name);
    if (result == 0) {
      low = middle;
      found = 1;
    } else if (result > 0) {
      high = middle;
    } else {
      low = middle;
    }
  }
  for (i = low, entry = &archive->entries[low]; i < high; ++i, ++entry) {
    if (_stricmp(entry->name, name) == 0) {
      return entry;
    }
  }
  for (i = 0; i < archive->entryCount; ++i) {
    if (_stricmp(archive->entries[i].name, name) == 0) {
      return &archive->entries[i];
    }
  }
  return NULL;
}

static inline mflZipArchive* findArchiveByName(const char* name) {
  mflZipArchive* archive;
  if (name == NULL || strlen(name) == 0) {
    return NULL;
  }
  archive = mflZipArchiveHead.next;
  while (archive != NULL) {
    if (_stricmp(archive->name, name) == 0) {
      return archive;
    }
    archive = archive->next;
  }
  return NULL;
}

static inline void findEntryInArchives(mflZipArchive* node,
    const char* name, mflZipArchive** owner, mflZipEntry** result) {
  mflZipEntry* entry;
  while (node != NULL) {
    entry = mflZipArchiveEntrySearch(node, name);
    if (entry != NULL) {
      *owner = node;
      *result = entry;
      return;
    }
    node = node->next;
  }
  *result = NULL;
}

mflZFile* mflZOpen(const char* filename, const char* mode) {
  char logName[256];
  char archiveName[64];
  char entryName[64];
  const char* path = filename;
  const char* separator;
  u32 length;
  mflZipArchive* archive = NULL;
  mflZipArchive* node;
  const char* suffix;
  mflZipEntry* entry;
  mflZFile* file;

  archiveName[0] = 0;
  entryName[0] = 0;
  separator = strchr(filename, ':');
  if (separator != NULL) {
    length = separator - filename;
    strncpy(archiveName, filename, length);
    archiveName[length] = 0;
    path = separator + 1;
  }
  Trim(entryName, path);
  InsertUnderscores(entryName);
  _strlwr(entryName);
  SpoofExtensions(entryName);
  if (archiveName[0] != 0) {
    archive = findArchiveByName(archiveName);
  }
  if (archive != NULL) {
    entry = mflZipArchiveEntrySearch(archive, entryName);
  } else {
    node = mflZipArchiveHead.next;
    length = strlen(entryName);
    if (length > 4) {
      suffix = &entryName[length - 4];
      if (strcmp(suffix, ".gsb") != 0 &&
          strcmp(suffix, ".dsp") != 0 &&
          strcmp(suffix, ".adp") != 0) {
        node = node->next;
      }
    }
    findEntryInArchives(node, entryName, &archive, &entry);
  }
  if (entry == NULL) {
    return NULL;
  }
  file = mflZFileAlloc();
  if (file == NULL) {
    return NULL;
  }
  file->entry = entry;
  file->archive = archive;
  file->position = 0;
  file->size = file->entry->size;
  memcpy(file, archive->file, sizeof(mflFile));
  strncpy(file->file.filename, entryName, sizeof(file->file.filename));
  file->file.filename[sizeof(file->file.filename) - 1] = 0;
  file->file.flags.zip = 1;
  file->file.flags.modeU = mode != NULL && strchr(mode, 'u') != NULL;
  file->file.size = file->entry->offset + file->size;
  if (mflFileLogging) {
    strcpy(logName, archive->name);
    strcat(logName, ":");
    strcat(logName, entryName);
    mflFileLog(logName);
  }
  return file;
}

static inline mflZFile* openArchiveFile(const char* filename, const char* mode) {
  char logName[256];
  char archiveName[64];
  char entryName[64];
  const char* path = filename;
  const char* separator;
  u32 length;
  mflZipArchive* archive = NULL;
  mflZipArchive* node;
  mflZipEntry* entry;
  mflZFile* file;

  archiveName[0] = 0;
  entryName[0] = 0;
  separator = strchr(filename, ':');
  if (separator != NULL) {
    length = separator - filename;
    strncpy(archiveName, filename, length);
    archiveName[length] = 0;
    path = separator + 1;
  }
  Trim(entryName, path);
  InsertUnderscores(entryName);
  _strlwr(entryName);
  SpoofExtensions(entryName);
  if (archiveName[0] != 0) {
    archive = findArchiveByName(archiveName);
  }
  if (archive != NULL) {
    entry = mflZipArchiveEntrySearch(archive, entryName);
  } else {
    node = mflZipArchiveHead.next;
    length = strlen(entryName);
    if (length > 4) {
      path = &entryName[length - 4];
      if (strcmp(path, ".gsb") != 0 &&
          strcmp(path, ".dsp") != 0 &&
          strcmp(path, ".adp") != 0) {
        node = node->next;
      }
    }
    findEntryInArchives(node, entryName, &archive, &entry);
  }
  if (entry == NULL) {
    return NULL;
  }
  file = mflZFileAlloc();
  if (file == NULL) {
    return NULL;
  }
  file->entry = entry;
  file->archive = archive;
  file->position = 0;
  file->size = file->entry->size;
  memcpy(file, archive->file, sizeof(mflFile));
  strncpy(file->file.filename, entryName, sizeof(file->file.filename));
  file->file.filename[sizeof(file->file.filename) - 1] = 0;
  file->file.flags.zip = 1;
  file->file.flags.modeU = mode != NULL && strchr(mode, 'u') != NULL;
  file->file.size = file->entry->offset + file->size;
  if (mflFileLogging) {
    strcpy(logName, archive->name);
    strcat(logName, ":");
    strcat(logName, entryName);
    mflFileLog(logName);
  }
  return file;
}

static inline s32 closeArchiveFile(mflZFile* file) {
  if (file != NULL) {
    mflZFileFree(file);
  }
  return 0;
}

s32 mflZClose(mflZFile* file) {
  return closeArchiveFile(file);
}

s32 mflZRead(void* destination, s32 size, s32 count, mflZFile* file) {
  s32 bytes = size * count;
  if (bytes + file->position > file->size) {
    s32 remaining = file->size - file->position;
    bytes = (remaining / size) * size;
  }
  if (bytes == 0) {
    return bytes;
  }
  mflZipSysCalls.seek(file, file->entry->offset + file->position, 0);
  bytes = mflZipSysCalls.read(destination, 1, bytes, file);
  file->position += bytes;
  return bytes / size;
}

static inline s32 seekArchiveFile(mflZFile* file, s32 offset, s32 origin) {
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
  default:
    return -1;
  }
  if (file->position < 0 || (u32)file->position > file->size) {
    return -1;
  }
  if ((u32)file->position == file->size) {
    file->file.flags.eof = 1;
  } else {
    file->file.flags.eof = 0;
  }
  return 0;
}

s32 mflZSeek(mflZFile* file, s32 offset, s32 origin) {
  return seekArchiveFile(file, offset, origin);
}

s32 mflZTell(mflZFile* file) {
  return file->position;
}

u32 mflZSize(const char* filename) {
  mflZFile* file = openArchiveFile(filename, "rb");
  u32 size = 0;
  if (file != NULL) {
    size = file->size;
    closeArchiveFile(file);
  }
  return size;
}

/* The earlier open-body owner is stripped; only its literal order is proven. */
static mflZFile* mflZipOpenScaffold(const char* filename, const char* mode) {
  return openArchiveFile(filename, mode);
}

mlAsyncRequest* mflZReadAsync(void* destination, s32 size, s32 count,
    mflZFile* file, s32 priority, mlAsyncCallback callback, void* user) {
  file->file.position = file->entry->offset + file->position;
  file->position += size * count;
  return mflZipSysCalls.readAsync(destination, size, count, file, priority,
      callback, user);
}

mlAsyncRequest* mflZSeekAsync(mflZFile* file, s32 offset, s32 origin,
    s32 priority, mlAsyncCallback callback, void* user) {
  mflFileCommand* request;
  if (mflQueueGetCurrent() == NULL) {
    return NULL;
  }
  request = mflFileCommandAlloc();
  if (request == NULL) {
    return NULL;
  }
  request->file = &file->file;
  request->argument0.value = seekArchiveFile(file, offset, origin);
  request->callback = callback;
  request->user = user;
  request->operation = 6;
  request->state = 2;
  mflQueueAdd(&request->queueEntry, priority);
  return request;
}
