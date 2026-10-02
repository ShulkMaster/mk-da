/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/os.h>
#include <stdarg.h>
#include <dolphin/os/OSException.h>
#include <dolphin/os/OSContext.h>
#include <dolphin/os/OSThread.h>
#include <dolphin/os/OSInterrupt.h>
#include <dolphin/hw_regs.h>

typedef u16 OSError;
typedef void (*OSErrorHandler)(OSError error, OSContext* context, ...);
extern OSErrorHandler __OSErrorTable[16];
extern volatile OSTime __OSLastInterruptTime;
extern volatile __OSInterrupt __OSLastInterrupt;
extern volatile u32 __OSLastInterruptSrr0;
extern void OSLoadContext(OSContext* context);
extern void __OSReschedule(void);
extern void PPCHalt(void);
#define MSR_RI 2
#define OS_ERROR_DECREMENTER 8
#define OS_ERROR_PROTECTION 15

int vprintf(const char* format, va_list args);

__declspec(weak) void OSReport(const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  vprintf(msg, args);
  va_end(args);
}

__declspec(weak) void OSPanic(const char* file, int line, const char* msg, ...) {
  va_list marker;
  u32 i;
  u32* p;

  OSDisableInterrupts();
  va_start(marker, msg);
  vprintf(msg, marker);
  va_end(marker);
  OSReport(" in \"%s\" on line %d.\n", file, line);

  OSReport("\nAddress:      Back Chain    LR Save\n");
  for (i = 0, p = (u32*)OSGetStackPointer(); p && (u32)p != 0xffffffff && i++ < 16; p = (u32*)*p) {
    OSReport("0x%08x:   0x%08x    0x%08x\n", p, p[0], p[1]);
  }

  PPCHalt();
}

OSErrorHandler OSSetErrorHandler(OSError error, OSErrorHandler handler) {
  OSErrorHandler oldHandler;
  oldHandler = __OSErrorTable[error];
  __OSErrorTable[error] = handler;
  return oldHandler;
}

void __OSUnhandledException(__OSException exception, OSContext* context, u32 dsisr, u32 dar) {
  if (!(context->srr1 & MSR_RI)) {
    OSReport("Non-recoverable Exception %d", exception);
  } else {
    if (__OSErrorTable[exception]) {
      OSDisableScheduler();
      __OSErrorTable[exception](exception, context, dsisr, dar);
      OSEnableScheduler();
      __OSReschedule();
      OSLoadContext(context);
    }

    if (exception == OS_ERROR_DECREMENTER) {
      OSLoadContext(context);
    }

    OSReport("Unhandled Exception %d", exception);
  }

  OSReport("\n");
  OSDumpContext(context);
  OSReport("\nDSISR = 0x%08x                   DAR  = 0x%08x\n", dsisr, dar);
  OSReport("TB = 0x%016llx\n", OSGetTime());

  switch (exception) {
  case __OS_EXCEPTION_DSI:
    OSReport("\nInstruction at 0x%x (read from SRR0) attempted to access "
             "invalid address 0x%x (read from DAR)\n",
             context->srr0, dar);
    break;
  case __OS_EXCEPTION_ISI:
    OSReport("\nAttempted to fetch instruction from invalid address 0x%x "
             "(read from SRR0)\n",
             context->srr0);
    break;
  case __OS_EXCEPTION_ALIGNMENT:
    OSReport("\nInstruction at 0x%x (read from SRR0) attempted to access "
             "unaligned address 0x%x (read from DAR)\n",
             context->srr0, dar);
    break;
  case __OS_EXCEPTION_PROGRAM:
    OSReport("\nProgram exception : Possible illegal instruction/operation "
             "at or around 0x%x (read from SRR0)\n",
             context->srr0, dar);
    break;
  case OS_ERROR_PROTECTION:
    OSReport("\n");
    OSReport("AI DMA Address =   0x%04x%04x\n", __DSPRegs[0x00000018], __DSPRegs[0x00000018 + 1]);
    OSReport("ARAM DMA Address = 0x%04x%04x\n", __DSPRegs[0x00000010], __DSPRegs[0x00000010 + 1]);
    OSReport("DI DMA Address =   0x%08x\n", __DIRegs[0x00000005]);
    break;
  }

  OSReport("\nLast interrupt (%d): SRR0 = 0x%08x  TB = 0x%016llx\n", __OSLastInterrupt,
           __OSLastInterruptSrr0, __OSLastInterruptTime);

  PPCHalt();
}
