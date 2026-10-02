/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/os.h>
#include <dolphin/os/OSError.h>
#include <dolphin/asm_sequences.inc>

u32 PPCMfmsr(void);
void PPCMtmsr(u32 value);
u32 PPCMfhid0(void);
u32 PPCMfl2cr(void);
void PPCMtl2cr(u32 value);
u32 PPCMfhid2(void);
void PPCMthid2(u32 value);
void PPCHalt(void);
void ICEnable(void);
void DCEnable(void);
void DBPrintf(const char* format, ...);
OSErrorHandler OSSetErrorHandler(OSError error, OSErrorHandler handler);

#define HID0_ICE 0x00008000
#define HID0_DCE 0x00004000
#define L2CR_L2E 0x80000000
#define L2CR_L2I 0x00200000
#define MSR_IR 0x20
#define MSR_DR 0x10
#define HID2_DCHERR 0x00800000
#define HID2_DNCERR 0x00400000
#define HID2_DCMERR 0x00200000
#define HID2_DQOERR 0x00100000
#define SRR1_DMA_BIT 0x00200000
#define OS_ERROR_MACHINE_CHECK 1

asm void DCEnable(void) { SEQ_DCEnable(); }

asm void DCInvalidateRange(register void* addr, register u32 nBytes) { SEQ_DCInvalidateRange(); }

asm void DCFlushRange(register void* addr, register u32 nBytes) { SEQ_DCFlushRange(); }

asm void DCStoreRange(register void* addr, register u32 nBytes) { SEQ_DCStoreRange(); }

asm void DCFlushRangeNoSync(register void* addr, register u32 nBytes) { SEQ_DCFlushRangeNoSync(); }

asm void DCStoreRangeNoSync(register void* addr, register u32 nBytes) { SEQ_DCStoreRangeNoSync(); }

asm void ICInvalidateRange(register void* addr, register u32 nBytes) { SEQ_ICInvalidateRange(); }

asm void ICFlashInvalidate(void) { SEQ_ICFlashInvalidate(); }

asm void ICEnable(void) { SEQ_ICEnable(); }

asm void LCDisable(void) { SEQ_LCDisable(); }

static inline void L2Disable(void) {
  __sync();
  PPCMtl2cr(PPCMfl2cr() & ~0x80000000);
  __sync();
}

void L2GlobalInvalidate(void) {
  L2Disable();
  PPCMtl2cr(PPCMfl2cr() | 0x00200000);
  while (PPCMfl2cr() & 0x00000001u)
    ;
  PPCMtl2cr(PPCMfl2cr() & ~0x00200000);
  while (PPCMfl2cr() & 0x00000001u) {
    DBPrintf(">>> L2 INVALIDATE : SHOULD NEVER HAPPEN\n");
  }
}

static inline void L2Init(void) {
  u32 oldMSR;
  oldMSR = PPCMfmsr();
  __sync();
  PPCMtmsr(MSR_IR | MSR_DR);
  __sync();
  L2Disable();
  L2GlobalInvalidate();
  PPCMtmsr(oldMSR);
}

static inline void L2Enable(void) { PPCMtl2cr((PPCMfl2cr() | L2CR_L2E) & ~L2CR_L2I); }

void DMAErrorHandler(OSError error, OSContext* context, ...) {
  u32 hid2 = PPCMfhid2();

  OSReport("Machine check received\n");
  OSReport("HID2 = 0x%x   SRR1 = 0x%x\n", hid2, context->srr1);
  if (!(hid2 & (HID2_DCHERR | HID2_DNCERR | HID2_DCMERR | HID2_DQOERR)) ||
      !(context->srr1 & SRR1_DMA_BIT)) {
    OSReport("Machine check was not DMA/locked cache related\n");
    OSDumpContext(context);
    PPCHalt();
  }

  OSReport("DMAErrorHandler(): An error occurred while processing DMA.\n");
  OSReport("The following errors have been detected and cleared :\n");

  if (hid2 & HID2_DCHERR) {
    OSReport("\t- Requested a locked cache tag that was already in the cache\n");
  }

  if (hid2 & HID2_DNCERR) {
    OSReport("\t- DMA attempted to access normal cache\n");
  }

  if (hid2 & HID2_DCMERR) {
    OSReport("\t- DMA missed in data cache\n");
  }

  if (hid2 & HID2_DQOERR) {
    OSReport("\t- DMA queue overflowed\n");
  }

  // write hid2 back to clear the error bits
  PPCMthid2(hid2);
}

void __OSCacheInit() {
  if (!(PPCMfhid0() & HID0_ICE)) {
    ICEnable();
    DBPrintf("L1 i-caches initialized\n");
  }
  if (!(PPCMfhid0() & HID0_DCE)) {
    DCEnable();
    DBPrintf("L1 d-caches initialized\n");
  }

  if (!(PPCMfl2cr() & L2CR_L2E)) {
    L2Init();
    L2Enable();
    DBPrintf("L2 cache initialized\n");
  }

  OSSetErrorHandler(OS_ERROR_MACHINE_CHECK, DMAErrorHandler);
  DBPrintf("Locked cache machine check handler installed\n");
}
