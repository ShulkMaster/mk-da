/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/ai.h>
#include <dolphin/hw_regs.h>
#include <dolphin/os.h>

extern AIDCallback __AID_Callback_8041CB74;
extern void __AI_SRC_INIT_801913D4(void);

AIDCallback AIRegisterDMACallback(AIDCallback callback) {
  s32 oldInts;
  AIDCallback ret;

  ret = __AID_Callback_8041CB74;
  oldInts = OSDisableInterrupts();
  __AID_Callback_8041CB74 = callback;
  OSRestoreInterrupts(oldInts);
  return ret;
}

void AIInitDMA(u32 addr, u32 length) {
  s32 oldInts;
  oldInts = OSDisableInterrupts();
  __DSPRegs[24] = (u16)((__DSPRegs[24] & ~0x3FF) | (addr >> 16));
  __DSPRegs[25] = (u16)((__DSPRegs[25] & ~0xFFE0) | (0xffff & addr));
  __DSPRegs[27] = (u16)((__DSPRegs[27] & ~0x7FFF) | (u16)((length >> 5) & 0xFFFF));
  OSRestoreInterrupts(oldInts);
}

void AIStartDMA() { __DSPRegs[27] |= 0x8000; }

void AIStopDMA(void) { __DSPRegs[27] &= ~0x8000; }

void AISetStreamPlayState(u32 state) {
  s32 oldInts;
  u8 volRight;
  u8 volLeft;

  if (state == AIGetStreamPlayState()) {
    return;
  }
  if ((AIGetStreamSampleRate() == 0U) && (state == 1)) {
    volRight = AIGetStreamVolRight();
    volLeft = AIGetStreamVolLeft();
    AISetStreamVolRight(0);
    AISetStreamVolLeft(0);
    oldInts = OSDisableInterrupts();
    __AI_SRC_INIT_801913D4();
    __AIRegs[0] = (__AIRegs[0] & ~0x20) | 0x20;
    __AIRegs[0] = (__AIRegs[0] & ~1) | 1;
    OSRestoreInterrupts(oldInts);
    AISetStreamVolLeft(volRight);
    AISetStreamVolRight(volLeft);
  } else {
    __AIRegs[0] = (__AIRegs[0] & ~1) | state;
  }
}

u32 AIGetStreamPlayState() { return __AIRegs[0] & 1; }

void AISetDSPSampleRate(u32 rate) {
  u32 state;
  s32 oldInts;
  u8 left;
  u8 right;
  u32 sampleRate;

  if (rate == AIGetDSPSampleRate()) {
    return;
  }

  __AIRegs[0] &= ~0x40;
  if (rate == 0) {
    left = AIGetStreamVolLeft();
    right = AIGetStreamVolRight();
    state = AIGetStreamPlayState();
    sampleRate = AIGetStreamSampleRate();
    AISetStreamVolLeft(0);
    AISetStreamVolRight(0);
    oldInts = OSDisableInterrupts();
    __AI_SRC_INIT_801913D4();
    __AIRegs[0] = (__AIRegs[0] & ~0x20) | 0x20;
    __AIRegs[0] = (__AIRegs[0] & ~2) | (sampleRate * 2);
    __AIRegs[0] = (__AIRegs[0] & ~1) | state;
    __AIRegs[0] |= 0x40;
    OSRestoreInterrupts(oldInts);
    AISetStreamVolLeft(left);
    AISetStreamVolRight(right);
  }
}

u32 AIGetDSPSampleRate() { return ((__AIRegs[0] >> 6) & 1) ^ 1; }

void __AI_set_stream_sample_rate_80190FD0(u32 rate) {
  s32 oldInts;
  s32 state;
  u8 left;
  u8 right;
  s32 temp_r26;

  if (rate == AIGetStreamSampleRate()) {
    return;
  }
  state = AIGetStreamPlayState();
  left = AIGetStreamVolLeft();
  right = AIGetStreamVolRight();
  AISetStreamVolRight(0);
  AISetStreamVolLeft(0);
  temp_r26 = __AIRegs[0] & 0x40;
  __AIRegs[0] &= ~0x40;
  oldInts = OSDisableInterrupts();
  __AI_SRC_INIT_801913D4();
  __AIRegs[0] |= temp_r26;
  __AIRegs[0] = (__AIRegs[0] & ~0x20) | 0x20;
  __AIRegs[0] = (__AIRegs[0] & ~2) | (rate * 2);
  OSRestoreInterrupts(oldInts);
  AISetStreamPlayState(state);
  AISetStreamVolLeft(left);
  AISetStreamVolRight(right);
}

u32 AIGetStreamSampleRate() { return (__AIRegs[0] >> 1) & 1; }

void AISetStreamVolLeft(u8 volume) { __AIRegs[1] = (__AIRegs[1] & ~0xFF) | (volume & 0xFF); }

u8 AIGetStreamVolLeft() { return __AIRegs[1]; }

void AISetStreamVolRight(u8 volume) {
  __AIRegs[1] = (__AIRegs[1] & ~0xFF00) | ((volume & 0xFF) << 8);
}

u8 AIGetStreamVolRight() { return __AIRegs[1] >> 8; }
