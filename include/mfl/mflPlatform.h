#ifndef MFL_PLATFORM_H
#define MFL_PLATFORM_H

#include <dolphin/types.h>
#include <mfl/mflFile.h>

void mflPlatformStatisticsReset(void);
s32 mflGcnDvdGetDriveErrorStatus(void);
s32 mflGcnDvdDriveErrorCheck(s32 status);
u32 mflGcnDvdTell(mflFile* file);
s32 mflGcnDvdEof(mflFile* file);
void mflGcnDvdReadCancel(mflFile* file);
u32 mflGcnDvdReadProgress(mflFile* file);
s32 mflGcnDvdSeek(mflFile* file, s32 offset, u16 origin);
s32 mflGcnDvdExists(const char* filename);
void mflGcnDvdClose(mflFile* file);
mflFile* mflGcnDvdOpen(const char* filename, const char* mode);
s32 mflGcnDvdReadContinue(mflFile* file);
s32 mflGcnDvdReadStart(void* destination, u32 elementSize,
                        u32 elementCount, mflFile* file);
void mflGcnDvdCallback(s32 result, DVDFileInfo* info);
void Trim(char* destination, const char* source);
void InsertUnderscores(char* filename);
void SpoofExtensions(char* filename);

#endif
