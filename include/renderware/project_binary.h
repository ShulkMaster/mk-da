#ifndef MKDA_RENDERWARE_PROJECT_BINARY_H
#define MKDA_RENDERWARE_PROJECT_BINARY_H

#include <dolphin/types.h>

s32 _rwStreamReadChunkHeader(void* arg0, u32* arg1, u32* arg2, u32* arg3, u32* arg4);
void* RwMemNative32(void* arg0, u32 arg1);
void* RwMemLittleEndian32(void* arg0, u32 arg1);

void* RwMemFloat32ToReal(void* arg0, u32 arg1);
void* RwStreamWrite(void* arg0, const void* arg1, u32 arg2);
void* RwStreamSkip(u8* arg0, u32 arg1);
s32 RwStreamFindChunk(void* arg0, u32 arg1, u32* arg2, u32* arg3);

#endif
