#ifndef MSL_MFLFILE_H
#define MSL_MFLFILE_H

#include <dolphin/types.h>

void* mflOpen(const char* filename, const char* mode);
s32 mflClose(void* file);
s32 mflGetDiscErrorStatus(void);

#endif
