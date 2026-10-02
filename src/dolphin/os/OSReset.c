/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/os.h>
#include <dolphin/os/OSReset.h>
#include <dolphin/os/OSThread.h>
#include <dolphin/OSRtcPriv.h>
#include <dolphin/hw_regs.h>
#include <dolphin/asm_sequences.inc>

extern void* memset(void* ptr, int value, u32 size);
extern void __OSStopAudioSystem(void);
extern BOOL __PADDisableRecalibration(BOOL disable);
extern void __OSReboot(u32 resetCode, BOOL forceMenu);
extern void ICFlashInvalidate(void);
extern void LCDisable(void);
extern OSThreadQueue __OSActiveThreadQueue : (OS_BASE_CACHED | 0x00DC);
volatile u8 DAT_800030e2 : 0x800030e2;
#define OS_RESET_RESTART 0
#define OS_RESET_HOTRESET 1
#define OS_RESET_SHUTDOWN 2

typedef struct OSResetQueue {
  OSResetFunctionInfo* first;
  OSResetFunctionInfo* last;
} OSResetQueue;

static OSResetQueue ResetFunctionQueue;

void OSRegisterResetFunction(OSResetFunctionInfo* func) {
  OSResetFunctionInfo* tmp;
  OSResetFunctionInfo* iter;

  for (iter = ResetFunctionQueue.first; iter && iter->priority <= func->priority; iter = iter->next)
    ;

  if (iter == NULL) {
    tmp = ResetFunctionQueue.last;
    if (tmp == NULL) {
      ResetFunctionQueue.first = func;
    } else {
      tmp->next = func;
    }
    func->prev = tmp;
    func->next = NULL;
    ResetFunctionQueue.last = func;
    return;
  }

  func->next = iter;
  tmp = iter->prev;
  iter->prev = func;
  func->prev = tmp;
  if (tmp == NULL) {
    ResetFunctionQueue.first = func;
    return;
  }
  tmp->next = func;
}

static asm void Reset(s32 resetCode) { SEQ_Reset(); }

static inline BOOL __OSCallResetFunctions(u32 arg0) {
  OSResetFunctionInfo* iter;
  s32 retCode = 0;

  for (iter = ResetFunctionQueue.first; iter != NULL; iter = iter->next) {
    retCode |= !iter->func(arg0);
  }
  retCode |= !__OSSyncSram();
  if (retCode) {
    return 0;
  }
  return 1;
}

static inline void KillThreads(void) {
  OSThread* active[2];

  for (active[0] = __OSActiveThreadQueue.head; active[0]; active[0] = active[1]) {
    active[1] = active[0]->linkActive.next;
    switch (active[0]->state) {
    case 1:
    case 4:
      OSCancelThread(active[0]);
      break;
    default:
      break;
    }
  }
}

void __OSDoHotReset(s32 arg0) {
  OSDisableInterrupts();
  __VIRegs[1] = 0;
  ICFlashInvalidate();
  Reset(arg0 * 8);
}

void OSResetSystem(int reset, u32 resetCode, BOOL forceMenu) {
  BOOL disableRecalibration;
  OSDisableScheduler();
  __OSStopAudioSystem();

  if (reset == OS_RESET_SHUTDOWN) {
    disableRecalibration = __PADDisableRecalibration(TRUE);
  }

  while (!__OSCallResetFunctions(FALSE))
    ;

  if (reset == OS_RESET_HOTRESET && forceMenu) {
    OSSram* sram;

    sram = __OSLockSram();
    sram->flags |= 0x40;
    __OSUnlockSram(TRUE);

    while (!__OSSyncSram())
      ;
  }
  OSDisableInterrupts();
  __OSCallResetFunctions(TRUE);
  LCDisable();
  if (reset == OS_RESET_HOTRESET) {
    __OSDoHotReset(resetCode);
  } else if (reset == OS_RESET_RESTART) {
    KillThreads();
    OSEnableScheduler();
    __OSReboot(resetCode, forceMenu);
  }
  KillThreads();
  memset(OSPhysicalToCached(0x40), 0, 0xcc - 0x40);
  memset(OSPhysicalToCached(0xd4), 0, 0xe8 - 0xd4);
  memset(OSPhysicalToCached(0xf4), 0, 0xf8 - 0xf4);
  memset(OSPhysicalToCached(0x3000), 0, 0xc0);
  memset(OSPhysicalToCached(0x30c8), 0, 0xd4 - 0xc8);

  __PADDisableRecalibration(disableRecalibration);
}

u32 OSGetResetCode(void) {
  if (DAT_800030e2 != 0) {
    return 0x80000000;
  }
  return ((__PIRegs[9] & ~7) >> 3);
}
