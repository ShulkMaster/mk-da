/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/hw_regs.h>

u32 AIGetStreamSampleRate(void) { return (__AIRegs[0] >> 1) & 1; }

void AISetStreamVolLeft(u8 volume) { __AIRegs[1] = (__AIRegs[1] & ~0xFF) | (volume & 0xFF); }

u8 AIGetStreamVolLeft() { return __AIRegs[1]; }

void AISetStreamVolRight(u8 volume) {
  __AIRegs[1] = (__AIRegs[1] & ~0xFF00) | ((volume & 0xFF) << 8);
}

u8 AIGetStreamVolRight() { return __AIRegs[1] >> 8; }
