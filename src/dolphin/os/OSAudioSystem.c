/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/os.h>
#include <dolphin/hw_regs.h>

void __OSStopAudioSystem(void) {
  u32 r28;

#define waitUntil(load, mask) \
  r28 = (load);              \
  while (r28 & (mask)) {     \
    r28 = (load);            \
  }

  __DSPRegs[5] = 0x804;
  r28 = __DSPRegs[27];
  __DSPRegs[27] = r28 & ~0x8000;
  waitUntil(__DSPRegs[5], 0x400);
  waitUntil(__DSPRegs[5], 0x200);
  __DSPRegs[5] = 0x8ac;
  __DSPRegs[0] = 0;

  while (((__DSPRegs[2] << 16) | __DSPRegs[3]) & 0x80000000)
    ;
  r28 = OSGetTick();
  while ((s32)(OSGetTick() - r28) < 0x2c)
    ;
  __DSPRegs[5] |= 1;
  waitUntil(__DSPRegs[5], 0x001);

#undef waitUntil
}

