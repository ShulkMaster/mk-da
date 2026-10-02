/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/dvd.h>
#include <dolphin/os.h>

extern u32 WorkAroundType_8041CC44;
extern u32 WorkAroundSeekLocation_8041CC48;

void __DVDLowSetWAType(u32 type, u32 location) {
  BOOL enabled;
  enabled = OSDisableInterrupts();
  WorkAroundType_8041CC44 = type;
  WorkAroundSeekLocation_8041CC48 = location;
  OSRestoreInterrupts(enabled);
}
