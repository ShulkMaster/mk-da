/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/os/OSInterrupt.h>
#include <dolphin/os/OSException.h>
#include <dolphin/os/OSError.h>
#include <dolphin/hw_regs.h>

BOOL OnReset_801AB500(BOOL final) {
  if (final != FALSE) {
    __MEMRegs[8] = 0xFF;
    __OSMaskInterrupts(0xf0000000);
  }
  return TRUE;
}

void MEMIntrruptHandler_801AB53C(__OSInterrupt interrupt, OSContext* context) {
  u32 addr;
  u32 cause;

  cause = __MEMRegs[0xf];
  addr = (((u32)__MEMRegs[0x12] & 0x3ff) << 16) | __MEMRegs[0x11];
  __MEMRegs[0x10] = 0;

  if (__OSErrorTable[OS_ERROR_PROTECTION]) {
    __OSErrorTable[OS_ERROR_PROTECTION](OS_ERROR_PROTECTION, context, cause, addr);
    return;
  }

  __OSUnhandledException(OS_ERROR_PROTECTION, context, cause, addr);
}
