#ifndef MFL_FILE_H
#define MFL_FILE_H

#include <dolphin/types.h>
#include <dolphin/dvd.h>
#include <mfl/mflQueue.h>

typedef struct mflFile {
  struct {
    u8 allocated : 1;
    u8 zip : 1;
    u8 open : 1;
    u8 eof : 1;
    u8 unk08 : 1;
    u8 modeU : 1;
    u8 unk03 : 2;
  } flags;
  u8 unk01[3];
  char filename[0x100];
  DVDFileInfo* dvdInfo;
  u32 size;
  u32 position;
  s32 bufferOffset;
  u32 bufferBytes;
  s32 readSize;
  s32 readProgress;
  s32 blockIndex;
  s32 blockOffset;
  s16 skip;
  struct {
    u8 directDMA : 1;
    u8 unk7F : 7;
  } readFlags;
  u8 unk12B;
  void* readDestination;
  u8* currentDestination;
  s32 lastResult;
  void* buffer;
} mflFile;
typedef void (*mflAsyncCallback)(s32 result, void* user);

typedef struct mflFileCommand {
  mflQueueEntry queueEntry;
  struct {
    u8 allocated : 1;
    u8 done : 1;
    u8 pendingFree : 1;
    u8 unk1F : 5;
  } flags;
  u8 unk0D[3];
  s32 operation;
  s32 state;
  mflFile* file;
  mflAsyncCallback callback;
  void* user;
  union {
    const char* filename;
    void* buffer;
    s32 value;
  } argument0;
  union {
    const char* mode;
    u32 value;
  } argument1;
  char filename[0x100];
  char mode[4];
  u32 readProgress;
  u32 elementSize;
  u32 elementCount;
} mflFileCommand;

extern s32 mflFileLogging;
void mflFileLog(const char* message);
void mflSetTickErrorCallback(void (*callback)(void));
s32 mflGetDiscErrorStatus(void);
s32 mflCheckQ(mflFileCommand* request);
u32 mflCheckQRead(mflFileCommand* request);
void mflCancelQ(mflFileCommand* request);
void mflFreeQ(mflFileCommand* request);
s32 mflTick(void);
s32 mflTell(void* file);
void* mflOpen(const char* filename, const char* mode);
s32 mflSize(const char* filename);
s32 mflExist(const char* filename);
s32 mflEof(void* file);
char* mflGetS(char* buffer, s32 bytes, void* file);
s32 mflClose(void* file);
s32 mflSeek(void* file, s32 offset, s32 origin);
s32 mflSeekEx(void* file, s32 offset, s32 origin, s32 priority);
s32 mflReadEx(void* buffer, s32 elementSize, s32 elementCount, void* file,
              s32 priority);
mflFileCommand* mflReadAsync(void* destination, u32 elementSize,
                             u32 elementCount, void* file, s32 priority,
                             mflAsyncCallback callback, void* user);
s32 mflFileSystemInit(void);
mflFileCommand* mflOpenAsync(const char* filename, const char* mode,
                             mflAsyncCallback callback, void* user);
mflFileCommand* mflSeekAsync(void* file, u32 offset, s32 origin,
                             s32 priority, mflAsyncCallback callback, void* user);

#endif
