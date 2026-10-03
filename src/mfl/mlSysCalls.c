#include <mfl/mlSysCalls.h>
#include <mfl/mflZip.h>
#include <msl/mslMem.h>
#include <string.h>

mlSysCalls g_SysCalls;

static void SetSysCalls(s32 kind, mlSysCalls* calls);
static s32 mflSeekFromMSL(void* file, s32 offset, s32 origin);
static s32 mflReadFromMSL(void* buffer, s32 elementSize, s32 elementCount, void* file);


/* Install callbacks for the selected file-system kind. */
void mlSetSysCalls(s32 kind) {
  SetSysCalls(kind, &g_SysCalls);
}

static void SetSysCalls(s32 kind, mlSysCalls* calls) {
  switch (kind) {
  case 0:
    memset(calls, 0, sizeof(calls));
    break;
  case 1:
    calls->exist = mflExist;
    calls->open = mflOpen;
    calls->close = mflClose;
    calls->read = mflReadFromMSL;
    calls->seek = mflSeekFromMSL;
    calls->tell = mflTell;
    calls->gets = mflGetS;
    calls->eof = mflEof;
    calls->size = mflSize;
    calls->readAsync = mflReadAsync;
    calls->seekAsync = mflSeekAsync;
    calls->cancelQ = mflCancelQ;
    calls->checkQ = mflCheckQ;
    calls->checkQRead = mflCheckQRead;
    calls->freeQ = mflFreeQ;
    calls->alignedAlloc = mlAlignedHeapAlloc;
    calls->realloc = mlHeapRealloc;
    calls->free = mlHeapFree;
    calls->unk4C = 0;
    calls->unk50 = 0;
    break;
  case 2:
    if (calls->kind != 1) {
      SetSysCalls(1, calls);
    }
    calls->open = (void* (*)(const char*, const char*))mflZOpen;
    calls->close = (s32 (*)(void*))mflZClose;
    calls->read = (s32 (*)(void*, s32, s32, void*))mflZRead;
    calls->seek = (s32 (*)(void*, s32, s32))mflZSeek;
    calls->tell = (s32 (*)(void*))mflZTell;
    calls->size = (s32 (*)(const char*))mflZSize;
    calls->readAsync = (mlAsyncRequest* (*)(void*, u32, u32, void*, s32, mlAsyncCallback, void*))mflZReadAsync;
    calls->seekAsync = (mlAsyncRequest* (*)(void*, u32, s32, s32, mlAsyncCallback, void*))mflZSeekAsync;
    break;
  }
  calls->kind = kind;
}

static s32 mflSeekFromMSL(void* file, s32 offset, s32 origin) {
  return mflSeekEx(file, offset, origin, 20);
}

static s32 mflReadFromMSL(void* buffer, s32 elementSize, s32 elementCount, void* file) {
  return mflReadEx(buffer, elementSize, elementCount, file, 20);
}
