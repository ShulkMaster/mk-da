/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/os/OSInterrupt.h>
#include <dolphin/os/OSException.h>
#include <dolphin/os/OSError.h>
#include <dolphin/os/OSReset.h>
#include <dolphin/hw_regs.h>
#include <dolphin/asm_sequences.inc>

static BOOL OnReset(BOOL final);

static OSResetFunctionInfo ResetFunctionInfo = {OnReset, 127, NULL, NULL};

static BOOL OnReset(BOOL final) {
  if (final != FALSE) {
    __MEMRegs[8] = 0xFF;
    __OSMaskInterrupts(0xf0000000);
  }
  return TRUE;
}

static void MEMIntrruptHandler(__OSInterrupt interrupt, OSContext* context) {
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

static asm void Config24MB(void) { SEQ_Config24MB(); }

static asm void Config48MB(void) { SEQ_Config48MB(); }

static asm void RealMode(register u32 addr) { SEQ_RealMode(); }

static inline u32 OSGetPhysicalMemSize(void) {
  return *(u32*)0x80000028;
}

static inline u32 OSGetConsoleSimulatedMemSize(void) {
  return *(u32*)0x800000F0;
}

void __OSInitMemoryProtection() {
  u32 padding[8];
  u32 simulatedSize;
  BOOL enabled;
  simulatedSize = OSGetConsoleSimulatedMemSize();
  enabled = OSDisableInterrupts();
  if (simulatedSize <= 0x1800000) {
    RealMode((u32)&Config24MB);
  } else if (simulatedSize <= 0x3000000) {
    RealMode((u32)&Config48MB);
  }

  __MEMRegs[16] = 0;
  __MEMRegs[8] = 0xFF;

  __OSMaskInterrupts(OS_INTERRUPTMASK_MEM_0 | OS_INTERRUPTMASK_MEM_1 | OS_INTERRUPTMASK_MEM_2 |
                     OS_INTERRUPTMASK_MEM_3);
  __OSSetInterruptHandler(__OS_INTERRUPT_MEM_0, MEMIntrruptHandler);
  __OSSetInterruptHandler(__OS_INTERRUPT_MEM_1, MEMIntrruptHandler);
  __OSSetInterruptHandler(__OS_INTERRUPT_MEM_2, MEMIntrruptHandler);
  __OSSetInterruptHandler(__OS_INTERRUPT_MEM_3, MEMIntrruptHandler);
  __OSSetInterruptHandler(__OS_INTERRUPT_MEM_ADDRESS, MEMIntrruptHandler);
  OSRegisterResetFunction(&ResetFunctionInfo);

  if (OSGetConsoleSimulatedMemSize() < OSGetPhysicalMemSize() &&
      OSGetConsoleSimulatedMemSize() == 0x1800000) {
    __MEMRegs[20] = 2;
  }

  __OSUnmaskInterrupts(OS_INTERRUPTMASK_MEM_ADDRESS);
  OSRestoreInterrupts(enabled);
}
