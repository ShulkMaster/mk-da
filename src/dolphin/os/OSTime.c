/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/os.h>
#include <dolphin/asm_sequences.inc>

asm OSTime OSGetTime(void) { SEQ_OSGetTime() }

asm OSTick OSGetTick(void) { SEQ_OSGetTick() }

#define OS_SYSTEMTIME_BASE 0x30D8

OSTime __OSGetSystemTime(void) {
  BOOL enabled;
  OSTime* timeAdjustAddr = (OSTime*)(OS_BASE_CACHED + OS_SYSTEMTIME_BASE);
  OSTime result;

  enabled = OSDisableInterrupts();
  result = *timeAdjustAddr + OSGetTime();
  OSRestoreInterrupts(enabled);

  return result;
}

OSTime __OSTimeToSystemTime(OSTime time) {
  BOOL enabled;
  OSTime* timeAdjustAddr = (OSTime*)(OS_BASE_CACHED + OS_SYSTEMTIME_BASE);
  OSTime result;

  enabled = OSDisableInterrupts();
  result = *timeAdjustAddr + time;
  OSRestoreInterrupts(enabled);

  return result;
}
