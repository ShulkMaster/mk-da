#ifndef _DOLPHIN_OSBOOTINFO
#define _DOLPHIN_OSBOOTINFO

#include <dolphin/dvd.h>

typedef struct OSBootInfo {
  DVDDiskID DVDDiskID;
  u32 magic;
  u32 version;
  u32 memorySize;
  u32 consoleType;
  void* arenaLo;
  void* arenaHi;
  void* FSTLocation;
  u32 FSTMaxLength;
} OSBootInfo;

#endif
