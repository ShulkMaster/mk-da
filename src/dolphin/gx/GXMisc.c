/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/gx.h>
#include <dolphin/gx/GXPriv.h>
#include <dolphin/os.h>
#include <dolphin/os/OSContext.h>
#include <dolphin/os/OSInterrupt.h>
#include <dolphin/os/OSThread.h>

static GXDrawSyncCallback TokenCB;
static void (*DrawDoneCB)(void);
static GXBool DrawDone;
static OSThreadQueue FinishQueue;
void PPCSync(void);
u32 PPCMfwpar(void);
void PPCMtwpar(u32 value);

static inline void __GXAbortWait(u32 clocks) {
  OSTime time0, time1;
  time0 = OSGetTime();

  do {
    time1 = OSGetTime();
  } while (time1 - time0 <= clocks / 4);
}

static inline void __GXAbort(void) {
  GX_SET_PI_REG(0x18 / 4, 1);
  __GXAbortWait(200);
  GX_SET_PI_REG(0x18 / 4, 0);
  __GXAbortWait(20);
}

static inline void GXSetDrawDone(void) {
  u32 reg;
  BOOL enabled;

  enabled = OSDisableInterrupts();
  reg = 0x45000002;
  GX_WRITE_RAS_REG(reg);
  GXFlush();
  DrawDone = 0;
  OSRestoreInterrupts(enabled);
}

static inline void GXWaitDrawDone(void) {
  BOOL interrupts;
  interrupts = OSDisableInterrupts();
  while (!DrawDone) {
    OSSleepThread(&FinishQueue);
  }

  OSRestoreInterrupts(interrupts);
}

void GXFlush(void) {
  if (__GXData->dirtyState) {
    __GXSetDirtyState();
  }
  GX_WRITE_U32(0);
  GX_WRITE_U32(0);
  GX_WRITE_U32(0);
  GX_WRITE_U32(0);
  GX_WRITE_U32(0);
  GX_WRITE_U32(0);
  GX_WRITE_U32(0);
  GX_WRITE_U32(0);
  PPCSync();
}

void GXResetWriteGatherPipe(void) {
  while (PPCMfwpar() & 1) {
  }
  PPCMtwpar(OSUncachedToPhysical((void*)0xCC008000));
}

void GXAbortFrame(void) {
  __GXAbort();
  __GXCleanGPFifo();
}

void GXSetDrawSync(u16 token) {
  BOOL enabled;
  u32 reg;

  enabled = OSDisableInterrupts();
  reg = token | 0x48000000;
  GX_WRITE_RAS_REG(reg);
  SET_REG_FIELD(reg, 16, 0, token);
  SET_REG_FIELD(reg, 8, 24, 0x47);
  GX_WRITE_RAS_REG(reg);
  GXFlush();
  OSRestoreInterrupts(enabled);
  __GXData->bpSentNot = 0;
}

void GXDrawDone(void) {
  GXSetDrawDone();
  GXWaitDrawDone();
}

void GXPixModeSync(void) {
  GX_WRITE_RAS_REG(__GXData->peCtrl);
  __GXData->bpSentNot = 0;
}

void GXPokeAlphaMode(GXCompare func, u8 threshold) {
  u32 reg;

  reg = (func << 8) | threshold;
  GX_SET_PE_REG(3, reg);
}

void GXPokeAlphaRead(GXAlphaReadMode mode) {
  u32 reg;

  reg = 0;
  SET_REG_FIELD(reg, 2, 0, mode);
  SET_REG_FIELD(reg, 1, 2, 1);
  GX_SET_PE_REG(4, reg);
}

void GXPokeAlphaUpdate(GXBool update_enable) {
  u32 reg;

  reg = GX_GET_PE_REG(1);
  SET_REG_FIELD(reg, 1, 4, update_enable);
  GX_SET_PE_REG(1, reg);
}

void GXPokeBlendMode(GXBlendMode type, GXBlendFactor src_factor, GXBlendFactor dst_factor,
                     GXLogicOp op) {
  u32 reg;

  reg = GX_GET_PE_REG(1);
  SET_REG_FIELD(reg, 1, 0, (type == GX_BM_BLEND) || (type == GX_BM_SUBTRACT));
  SET_REG_FIELD(reg, 1, 11, (type == GX_BM_SUBTRACT));
  SET_REG_FIELD(reg, 1, 1, (type == GX_BM_LOGIC));
  SET_REG_FIELD(reg, 4, 12, op);
  SET_REG_FIELD(reg, 3, 8, src_factor);
  SET_REG_FIELD(reg, 3, 5, dst_factor);
  SET_REG_FIELD(reg, 8, 24, 0x41);
  GX_SET_PE_REG(1, reg);
}

void GXPokeColorUpdate(GXBool update_enable) {
  u32 reg;

  reg = GX_GET_PE_REG(1);
  SET_REG_FIELD(reg, 1, 3, update_enable);
  GX_SET_PE_REG(1, reg);
}

void GXPokeDstAlpha(GXBool enable, u8 alpha) {
  u32 reg = 0;

  SET_REG_FIELD(reg, 8, 0, alpha);
  SET_REG_FIELD(reg, 1, 8, enable);
  GX_SET_PE_REG(2, reg);
}

void GXPokeDither(GXBool dither) {
  u32 reg;

  reg = GX_GET_PE_REG(1);
  SET_REG_FIELD(reg, 1, 2, dither);
  GX_SET_PE_REG(1, reg);
}

void GXPokeZMode(GXBool compare_enable, GXCompare func, GXBool update_enable) {
  u32 reg = 0;

  SET_REG_FIELD(reg, 1, 0, compare_enable);
  SET_REG_FIELD(reg, 3, 1, func);
  SET_REG_FIELD(reg, 1, 4, update_enable);
  GX_SET_PE_REG(0, reg);
}

GXDrawSyncCallback GXSetDrawSyncCallback(GXDrawSyncCallback callback) {
  GXDrawSyncCallback prevCB;
  BOOL interrupts;

  prevCB = TokenCB;
  interrupts = OSDisableInterrupts();
  TokenCB = callback;
  OSRestoreInterrupts(interrupts);
  return prevCB;
}

static void GXTokenInterruptHandler(__OSInterrupt interrupt, OSContext *context) {
  u16 token;
  OSContext exceptionContext;
  u32 reg;

  token = GX_GET_PE_REG(7);
  if (TokenCB != NULL) {
    OSClearContext(&exceptionContext);
    OSSetCurrentContext(&exceptionContext);
    TokenCB(token);
    OSClearContext(&exceptionContext);
    OSSetCurrentContext(context);
  }
  reg = GX_GET_PE_REG(5);
  SET_REG_FIELD(reg, 1, 2, 1);
  GX_SET_PE_REG(5, reg);
}

static void GXFinishInterruptHandler(__OSInterrupt interrupt, OSContext *context) {
  OSContext exceptionContext;
  u32 reg;

  reg = GX_GET_PE_REG(5);
  SET_REG_FIELD(reg, 1, 3, 1);
  GX_SET_PE_REG(5, reg);
  DrawDone = 1;
  if (DrawDoneCB != NULL) {
    OSClearContext(&exceptionContext);
    OSSetCurrentContext(&exceptionContext);
    DrawDoneCB();
    OSClearContext(&exceptionContext);
    OSSetCurrentContext(context);
  }
  OSWakeupThread(&FinishQueue);
}

void __GXPEInit(void) {
  u32 reg;
  __OSSetInterruptHandler(0x12, GXTokenInterruptHandler);
  __OSSetInterruptHandler(0x13, GXFinishInterruptHandler);
  OSInitThreadQueue(&FinishQueue);
  __OSUnmaskInterrupts(0x2000);
  __OSUnmaskInterrupts(0x1000);
  reg = GX_GET_PE_REG(5);
  SET_REG_FIELD(reg, 1, 2, 1);
  SET_REG_FIELD(reg, 1, 3, 1);
  SET_REG_FIELD(reg, 1, 0, 1);
  SET_REG_FIELD(reg, 1, 1, 1);
  GX_SET_PE_REG(5, reg);
}
