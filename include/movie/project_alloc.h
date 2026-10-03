#ifndef MKDA_MOVIE_PROJECT_ALLOC_H
#define MKDA_MOVIE_PROJECT_ALLOC_H

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void* gHeap;
void* _mwMemMalloc(void*, u32, u32, const char*, const char*, s32);
void _mwMemFree(void*, const char*, s32);
void* _mwMemRealloc(void*, void*, u32, u32, const char*, const char*, s32);

#ifdef __cplusplus
}
#endif

#endif
