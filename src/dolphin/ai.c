/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/ai.h>
#include <dolphin/hw_regs.h>
#include <dolphin/os.h>
#include <dolphin/os/OSContext.h>
#include <dolphin/os/OSInterrupt.h>
#include <dolphin/asm_sequences.inc>

typedef void (*AISCallback)(u32 sampleCount);

static AISCallback __AIS_Callback;
static AIDCallback __AID_Callback;
static u8* __CallbackStack;
static u8* __OldStack;
static s32 __AI_init_flag;
static OSTime bound_32KHz;
static OSTime bound_48KHz;
static OSTime min_wait;
static OSTime max_wait;
static OSTime buffer;

static void __AI_SRC_INIT(void);
static void __AICallbackStackSwitch(AIDCallback callback);
static void __AISHandler(__OSInterrupt interrupt, OSContext* context);
static void __AIDHandler(__OSInterrupt interrupt, OSContext* context);

AIDCallback AIRegisterDMACallback(AIDCallback callback) {
  s32 oldInts;
  AIDCallback ret;

  ret = __AID_Callback;
  oldInts = OSDisableInterrupts();
  __AID_Callback = callback;
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
    __AI_SRC_INIT();
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
    __AI_SRC_INIT();
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

static void __AI_set_stream_sample_rate(u32 rate) {
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
  __AI_SRC_INIT();
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

void AIInit(u8* stack) {
  if (__AI_init_flag == TRUE) {
    return;
  }

  bound_32KHz = OSNanosecondsToTicks(31524);
  bound_48KHz = OSNanosecondsToTicks(42024);
  min_wait = OSNanosecondsToTicks(42000);
  max_wait = OSNanosecondsToTicks(63000);
  buffer = OSNanosecondsToTicks(3000);
  AISetStreamVolRight(0);
  AISetStreamVolLeft(0);
  __AIRegs[3] = 0;
  __AIRegs[0] = (__AIRegs[0] & ~0x20) | 0x20;
  __AI_set_stream_sample_rate(1);
  AISetDSPSampleRate(0);
  __AIS_Callback = 0;
  __AID_Callback = 0;
  __CallbackStack = stack;
  __OSSetInterruptHandler(5, __AIDHandler);
  __OSUnmaskInterrupts(0x04000000);
  __OSSetInterruptHandler(8, __AISHandler);
  __OSUnmaskInterrupts(0x00800000);
  __AI_init_flag = TRUE;
}

static void __AISHandler(__OSInterrupt interrupt, OSContext* context) {
  OSContext exceptionContext;

  __AIRegs[0] |= 8;
  OSClearContext(&exceptionContext);
  OSSetCurrentContext(&exceptionContext);
  if (__AIS_Callback) {
    __AIS_Callback(__AIRegs[2]);
  }
  OSClearContext(&exceptionContext);
  OSSetCurrentContext(context);
}

static void __AIDHandler(__OSInterrupt interrupt, OSContext* context) {
  OSContext exceptionContext;
  u16 tmp;

  tmp = __DSPRegs[5];
  tmp = (tmp & ~0xA0) | 8;
  __DSPRegs[5] = tmp;
  OSClearContext(&exceptionContext);
  OSSetCurrentContext(&exceptionContext);
  if (__AID_Callback) {
    if (__CallbackStack) {
      __AICallbackStackSwitch(__AID_Callback);
    } else {
      __AID_Callback();
    }
  }
  OSClearContext(&exceptionContext);
  OSSetCurrentContext(context);
}

static asm void __AICallbackStackSwitch(AIDCallback callback) { SEQ___AICallbackStackSwitch(); }

static void __AI_SRC_INIT(void) {
  OSTime rising_32khz = 0;
  OSTime rising_48khz = 0;
  OSTime diff = 0;
  OSTime t1 = 0;
  OSTime temp;
  u32 temp0;
  u32 temp1;
  u32 done = 0;
  u32 volume = 0;
  u32 Init_Cnt = 0;
  u32 walking = 0;

  walking = 0;
  Init_Cnt = 0;
  temp = 0;

  while (!done) {
    __AIRegs[0] = (__AIRegs[0] & ~0x20) | 0x20;
    __AIRegs[0] &= ~2;
    __AIRegs[0] = (__AIRegs[0] & ~1) | 1;
    temp0 = __AIRegs[2];
    while (temp0 == __AIRegs[2]) {
    }
    rising_32khz = OSGetTime();
    __AIRegs[0] = (__AIRegs[0] & ~2) | 2;
    __AIRegs[0] = (__AIRegs[0] & ~1) | 1;
    temp1 = __AIRegs[2];
    while (temp1 == __AIRegs[2]) {
    }
    rising_48khz = OSGetTime();
    diff = rising_48khz - rising_32khz;
    __AIRegs[0] &= ~2;
    __AIRegs[0] &= ~1;
    if (diff < bound_32KHz - buffer) {
      temp = min_wait;
      done = 1;
      Init_Cnt++;
    } else if (diff >= bound_32KHz + buffer && diff < bound_48KHz - buffer) {
      temp = max_wait;
      done = 1;
      Init_Cnt++;
    } else {
      done = 0;
      walking = 1;
      Init_Cnt++;
    }
  }
  while (rising_48khz + temp > OSGetTime()) {
  }
}
