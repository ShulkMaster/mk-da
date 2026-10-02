/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/hw_regs.h>

u32 DSPCheckMailToDSP(void) { return (__DSPRegs[0] >> 0xF) & 1; }

u32 DSPCheckMailFromDSP(void) { return (__DSPRegs[2] >> 0xF) & 1; }

u32 DSPReadMailFromDSP() {
  u16 reg1;
  u16 reg2;
  reg1 = __DSPRegs[2];
  reg2 = __DSPRegs[3];
  return reg1 << 16 | reg2;
}

void DSPSendMailToDSP(u32 mail) {
  __DSPRegs[0] = mail >> 16;
  __DSPRegs[1] = mail;
}
