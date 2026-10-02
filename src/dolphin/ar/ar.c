/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/ar.h>
#include <dolphin/hw_regs.h>
#include <dolphin/os.h>

extern ARCallback __AR_Callback_8041CBB0;

ARCallback ARRegisterDMACallback(ARCallback callback) {
  ARCallback oldCb;
  BOOL enabled;
  oldCb = __AR_Callback_8041CBB0;
  enabled = OSDisableInterrupts();
  __AR_Callback_8041CBB0 = callback;
  OSRestoreInterrupts(enabled);
  return oldCb;
}

void ARStartDMA(u32 type, u32 mainmem_addr, u32 aram_addr, u32 length) {
  BOOL enabled;

  enabled = OSDisableInterrupts();

  __DSPRegs[16] = (u16)(__DSPRegs[16] & ~0x3ff) | (u16)(mainmem_addr >> 16);
  __DSPRegs[17] = (u16)(__DSPRegs[17] & ~0xffe0) | (u16)(mainmem_addr & 0xffff);
  __DSPRegs[18] = (u16)(__DSPRegs[18] & ~0x3ff) | (u16)(aram_addr >> 16);
  __DSPRegs[19] = (u16)(__DSPRegs[19] & ~0xffe0) | (u16)(aram_addr & 0xffff);
  __DSPRegs[20] = (u16)((__DSPRegs[20] & ~0x8000) | (type << 15));
  __DSPRegs[20] = (u16)(__DSPRegs[20] & ~0x3ff) | (u16)(length >> 16);
  __DSPRegs[21] = (u16)(__DSPRegs[21] & ~0xffe0) | (u16)(length & 0xffff);
  OSRestoreInterrupts(enabled);
}
