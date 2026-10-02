/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/os.h>
#include <dolphin/db.h>
#include <dolphin/os/OSContext.h>
#include <dolphin/os/OSException.h>
#include <dolphin/asm_sequences.inc>

volatile OSContext* __OSCurrentContext AT_ADDRESS(0x800000D4);
volatile OSContext* __OSFPUContext AT_ADDRESS(0x800000D8);

extern __OSExceptionHandler __OSSetExceptionHandler(__OSException exception,
                                                  __OSExceptionHandler handler);

static asm void __OSLoadFPUContext(u32 dummy, OSContext* fpuContext) {
  SEQ___OSLoadFPUContext();
}

static asm void __OSSaveFPUContext(u32 dummy1, u32 dummy2,
                                   OSContext* fpuContext) {
  SEQ___OSSaveFPUContext();
}

asm void OSSetCurrentContext(OSContext* context) { SEQ_OSSetCurrentContext(); }

OSContext* OSGetCurrentContext(void) {
  return (OSContext*)__OSCurrentContext;
}

asm u32 OSSaveContext(OSContext* context) { SEQ_OSSaveContext(); }

extern void __RAS_OSDisableInterrupts_begin();
extern void __RAS_OSDisableInterrupts_end();

asm void OSLoadContext(OSContext* context) { SEQ_OSLoadContext(); }

asm u32 OSGetStackPointer(void) { SEQ_OSGetStackPointer(); }

asm int OSSwitchFiber(u32 pc, u32 newsp) { SEQ_OSSwitchFiber(); }

void OSClearContext(OSContext* context) {
  context->mode = 0;
  context->state = 0;
  if (context == __OSFPUContext)
    __OSFPUContext = NULL;
}

asm void OSInitContext(OSContext* context, u32 pc, u32 newsp) {
  SEQ_OSInitContext();
}

void OSDumpContext(OSContext* context) {
  u32 i;
  u32* p;

  OSReport("------------------------- Context 0x%08x -------------------------\n", context);

  for (i = 0; i < 16; ++i) {
    OSReport("r%-2d  = 0x%08x (%14d)  r%-2d  = 0x%08x (%14d)\n", i, context->gpr[i],
             context->gpr[i], i + 16, context->gpr[i + 16], context->gpr[i + 16]);
  }

  OSReport("LR   = 0x%08x                   CR   = 0x%08x\n", context->lr, context->cr);
  OSReport("SRR0 = 0x%08x                   SRR1 = 0x%08x\n", context->srr0, context->srr1);

  OSReport("\nGQRs----------\n");
  for (i = 0; i < 4; ++i) {
    OSReport("gqr%d = 0x%08x \t gqr%d = 0x%08x\n", i, context->gqr[i], i + 4, context->gqr[i + 4]);
  }

  if (context->state & OS_CONTEXT_STATE_FPSAVED) {
    OSContext* currentContext;
    OSContext fpuContext;
    BOOL enabled;

    enabled = OSDisableInterrupts();
    currentContext = OSGetCurrentContext();
    OSClearContext(&fpuContext);
    OSSetCurrentContext(&fpuContext);

    OSReport("\n\nFPRs----------\n");
    for (i = 0; i < 32; i += 2) {
      OSReport("fr%d \t= %d \t fr%d \t= %d\n", i, (u32)context->fpr[i], i + 1,
               (u32)context->fpr[i + 1]);
    }
    OSReport("\n\nPSFs----------\n");
    for (i = 0; i < 32; i += 2) {
      OSReport("ps%d \t= 0x%x \t ps%d \t= 0x%x\n", i, (u32)context->psf[i], i + 1,
               (u32)context->psf[i + 1]);
    }

    OSClearContext(&fpuContext);
    OSSetCurrentContext(currentContext);
    OSRestoreInterrupts(enabled);
  }

  OSReport("\nAddress:      Back Chain    LR Save\n");
  for (i = 0, p = (u32*)context->gpr[1]; p && (u32)p != 0xffffffff && i++ < 16; p = (u32*)*p) {
    OSReport("0x%08x:   0x%08x    0x%08x\n", p, p[0], p[1]);
  }
}

static asm void OSSwitchFPUContext(__OSException exception, OSContext* context) {
  SEQ_OSSwitchFPUContext();
}

void __OSContextInit(void) {
  __OSSetExceptionHandler(7, OSSwitchFPUContext);
  __OSFPUContext = NULL;
  DBPrintf("FPU-unavailable handler installed\n");
}
