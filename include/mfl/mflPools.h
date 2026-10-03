#ifndef MFL_POOLS_H
#define MFL_POOLS_H

#include <mfl/mflQueue.h>
#include <mfl/mflFile.h>
#include <mfl/mflZip.h>

/* FileSystemInit copies this configuration as one 32-bit word. */
typedef union mflPoolConfig {
  u32 value;
  struct {
    u8 files : 1;
    u8 zip : 1;
    u8 commands : 1;
    u8 unk10 : 1;
    u8 queues : 1;
    u8 unk07 : 3;
  } flags;
} mflPoolConfig;

void mflInitializePools(mflPoolConfig config);
mflFileCommand* mflFileCommandAlloc(void);
void mflFileCommandFree(mflFileCommand* command);
void mflFileCommandFreePending(void);
mflZFile* mflZFileAlloc(void);
void mflZFileFree(mflZFile* file);
mflFile* mflFileAlloc(void);
void mflFileFree(mflFile* file);

#endif
