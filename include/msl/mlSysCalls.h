#ifndef MSL_MLSYSCALLS_H
#define MSL_MLSYSCALLS_H

#include <dolphin/types.h>
#include <mfl/mflFile.h>

/* Retail g_SysCalls occupies 0x54 bytes. */
struct _mwMemHeap;
typedef mflFileCommand mlAsyncRequest;
typedef mflAsyncCallback mlAsyncCallback;

typedef struct mlSysCalls {
  s32 kind;
  s32 (*exist)(const char* filename);
  void* (*open)(const char* filename, const char* mode);
  s32 (*close)(void* file);
  s32 (*read)(void* buffer, s32 elementSize, s32 elementCount, void* file);
  s32 (*seek)(void* file, s32 offset, s32 origin);
  s32 (*tell)(void* file);
  char* (*gets)(char* buffer, s32 bytes, void* file);
  s32 (*eof)(void* file);
  s32 (*size)(const char* filename);
  mlAsyncRequest* (*readAsync)(void* destination, u32 elementSize, u32 elementCount,
                              void* file, s32 priority,
                              mlAsyncCallback callback, void* user);
  mlAsyncRequest* (*seekAsync)(void* file, u32 offset, s32 origin,
                              s32 priority, mlAsyncCallback callback, void* user);
  void (*cancelQ)(mflFileCommand* q);
  s32 (*checkQ)(mflFileCommand* q);
  u32 (*checkQRead)(mflFileCommand* q);
  void (*freeQ)(mflFileCommand* q);
  void* (*alignedAlloc)(struct _mwMemHeap* heap, u32 size);
  void* (*realloc)(struct _mwMemHeap* heap, void* ptr, u32 size);
  void (*free)(void* ptr);
  u32 unk4C;
  u32 unk50;
} mlSysCalls;

extern mlSysCalls g_SysCalls;
void mlSetSysCalls(s32 kind);

#endif
